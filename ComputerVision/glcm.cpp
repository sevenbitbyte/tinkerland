/**
 *	@author	Alan Meekins
 *	@date	3/7/09
 *
 *	This program generates a Grey Level Co-occurance Matrix in the horizontal,
 *	vertical, and diagonal directions. The genererated GLCM is used to search a
 *	target image for a similar texture. The program takes two arguments and a
 *	number of switches. The first argument is always the source image of a
 *	texture to search for with in the target image, which is the second argument.
 *	The second argument may be an image file, video file, or a capture device
 *	like a webcam. To change between media types for the target image look
 *	at the usage. If only the first argument is provided then the program will
 *	display useful texture information about the source image and allow the
 *	GLCMData to be saved to disk for use in a search program.
**/

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <math.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>

#define COLOR_SPACE 256
#define MAX_COUNT 255
#define STEP_SIZE 1
#define INC_VAL 1
#define S_MATRIX true

#define GetPixelPtrD8(i,r,c,chan) i->imageData[((r*i->widthStep)+(c*i->nChannels)+chan)]
#define GetPixelPtrD16(i,r,c,chan) i->imageData[((r*(i->widthStep/sizeof(uint16_t)))+(c*i->nChannels)+chan)]
#define GetPixelPtrD32(i,r,c,chan) ((float*)i->imageData)[((r*768)+(c*i->nChannels)+chan)]

using namespace std;

struct GLCMData{
	CvPoint centroid[3];	//Centroids for all channels
	double centroidS[6];	//Variance for each axis of centroids]
	double energy;
	double entropy;
	double contrast;
	double homogeneity;
	double correlation;
	IplImage* coMat;	//Cooccurance matrix
	IplImage* nMat;		//Normalized Matrix
	uint32_t totalCount[4];
};

CvPoint roiStart;
bool roiMod=false;
bool roiSlide=false;
bool updateNeeded=true;
//unsigned char drawState=0;

char waitPause();
bool initGLCMData(GLCMData*, int, int);
void releaseGLCMData(GLCMData*);
void buildGLCMData(GLCMData*);
void createGLCM(IplImage*,GLCMData*);
void roiHandler(int event, int x, int y, int flags, void* i);
//vector<CvPoint> findExtrema(IplImage*, int h, int w);

int main(int argc, char** argv){

	if(argc<2){
		printf("Usage: %s [referenceImage] <targetImage>\n", argv[0]);
		exit(1);
	}

	IplImage* input1=cvLoadImage(argv[1]);
	if(input1 == NULL){
		cout<<"Error: Could not load "<<argv[1]<<endl;
		exit(-1);
	}
	cout<<"Image1: "<<argv[1]<<" Width="<<input1->width<<" Height="<<input1->height<<endl;

	if(argc>=3){
		IplImage* input2=cvLoadImage(argv[2]);
		if(input2 == NULL){
			cout<<"Error: Could not load "<<argv[2]<<endl;
			exit(-1);
		}
	}

	IplImage* grey1=cvCreateImage(cvSize(input1->width, input1->height), IPL_DEPTH_8U, 1);
	IplImage* img1Copy=cvCreateImage(cvSize(input1->width, input1->height), input1->depth, input1->nChannels);
	GLCMData* gdata1=new GLCMData;
    gdata1->coMat = NULL;
    gdata1->nMat = NULL;

	cvCvtColor(input1, grey1, CV_BGR2GRAY);
	cvCopy(input1, img1Copy);

	cvNamedWindow("Input1", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("GLCM-1", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Grey1", CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback("Input1", roiHandler, (void*)grey1);

	cvShowImage("Grey1", grey1);
	bool isPaused=false;
	bool stepMode=false;
	char keyVal='p';
	float milSecs=0;

	while(keyVal!='q'){
		if(keyVal=='r'){
			//Reset ROI
			cvResetImageROI(grey1);
			updateNeeded=true;
		}
		if(keyVal=='s'){
			cout<<"Enter, step mode"<<endl;
			cout<<"\tn - Next frame"<<endl;
			cout<<"\tq - Quit"<<endl;
			cout<<"\ts - Exit step mode"<<endl;
			stepMode=true;
		}
		if(stepMode){
			while(1){
				keyVal=cvWaitKey(50);
				if(keyVal=='s'){		//Stop step mode
					stepMode=false;
					keyVal=0;
					cout<<"Exit, step mode"<<endl;
					break;
				}
				else if(keyVal=='q'){	//Exit program
					break;
				}
				else if(keyVal=='n'){	//Next frame
					cout<<"Step"<<endl;
					updateNeeded=true;
					break;
				}
			}
		}

		//cout<<"Entering init"<<endl;
		if(!initGLCMData(gdata1, COLOR_SPACE, 3)){
			cout<<"Error: Could not initialize GLCMData!"<<endl;
			exit(-2);
		}
		//cout<<"Init complete!"<<endl;

		if(updateNeeded || !isPaused){
			//Time how long it takes to generate the GLCM
			timeval timeTemp;
			timeval timeDelta;
			gettimeofday(&timeTemp, NULL);
			createGLCM(grey1, gdata1);
			gettimeofday(&timeDelta, NULL);
			timeDelta.tv_sec-=timeTemp.tv_sec;
			timeDelta.tv_usec-=timeTemp.tv_usec;
			milSecs=timeDelta.tv_sec*1000 + timeDelta.tv_usec/1000;
			cout<<"Created glcm in "<<milSecs<<"ms.";

			gettimeofday(&timeTemp, NULL);
			buildGLCMData(gdata1);
			gettimeofday(&timeDelta, NULL);
			timeDelta.tv_sec-=timeTemp.tv_sec;
			timeDelta.tv_usec-=timeTemp.tv_usec;
			milSecs=timeDelta.tv_sec*1000 + timeDelta.tv_usec/1000;
			cout<<" Analysized glcm in "<<milSecs<<"ms."<<endl;

			cout<<"Energy: "<<gdata1->energy<<" Entropy: "<<gdata1->entropy<<" Contrast: "<<gdata1->contrast<<" Homogeneity: "<<gdata1->homogeneity<<endl;

			cvCopy(input1, img1Copy);
			CvRect grey1ROI=cvGetImageROI(grey1);
			if(!(grey1ROI.x==0 && grey1ROI.y==0 && grey1ROI.width==grey1->width && grey1ROI.height==grey1->height)){
				printf("Draw rectangle\n");
				cvRectangle(img1Copy, cvPoint(grey1ROI.x, grey1ROI.y), cvPoint(grey1ROI.x+grey1ROI.width, grey1ROI.y+grey1ROI.height), cvScalar(0.0, 0.0, 254.0));
			}

			cvShowImage("Input1", img1Copy);
			cvShowImage("GLCM-1", gdata1->coMat);
			updateNeeded=false;
		}

		if(keyVal=='p'){
			isPaused=!isPaused;
		}
		keyVal=cvWaitKey(10);
	}

	printf("Cleaning up\n");

	cvDestroyAllWindows();
	cvReleaseImage(&input1);
	cvReleaseImage(&grey1);
	releaseGLCMData(gdata1);
}


/**
 *	Creates a grey level co-occurance matrix
 *	@param	img	Image to analyze
 *	@param	glcm	GLCMData struct to use as output
 */
void createGLCM(IplImage* img, GLCMData* glcm){
	int height=img->height;
	int width=img->width;
	int rowStart=0;
	int rowEnd=width;
	int colStart=0;
	int colEnd=height;
	CvRect roi=cvGetImageROI(img);
	IplImage* coMat=glcm->coMat;

	glcm->totalCount[0]=0;
	glcm->totalCount[1]=0;
	glcm->totalCount[2]=0;
	glcm->totalCount[3]=0;

	if(!(roi.x==0 && roi.y==0 && roi.width==width && roi.height==height)){
		rowStart=(int) fmax(fmin(roi.y, height-1), 0);
		rowEnd=(int) fmax(fmin(rowStart+roi.height, height), rowStart+1);
		colStart=(int) fmax(fmin(roi.x, width-1), 0);
		colEnd=(int) fmax(fmin(colStart+roi.width, width), colStart+1);
		cout<<"Using ROI"<<endl;
	}
	uint8_t px1 =(uint8_t) GetPixelPtrD8(img, rowStart, colStart, 0);
	uint8_t px2;
	uint8_t* out;

	for(int row=rowStart; row<rowEnd; row++){
		for(int col=colStart; col<colEnd; col++){
			if(row<(height-STEP_SIZE) && coMat->nChannels>=2){
				//Populate vertical matrix
				px2 = (uint8_t) GetPixelPtrD8(img, row+STEP_SIZE, col, 0);
				out = (uint8_t*)&GetPixelPtrD8(coMat, px1, px2, 1);
				/*if(*out < MAX_COUNT){
                                        *out=*out+INC_VAL;
				}*/
				*out=(uint8_t) fmin(*out+INC_VAL, MAX_COUNT);
				glcm->totalCount[1]++;
				#if S_MATRIX
				out = (uint8_t*)&GetPixelPtrD8(coMat, px2, px1, 1);
				*out=(uint8_t) fmin(*out+INC_VAL, MAX_COUNT);
				glcm->totalCount[1]++;
				#endif
			}
			if(col<(width-STEP_SIZE) && row<(height-STEP_SIZE) && coMat->nChannels>=3){
				//Populate diagonal matrix
                                px2 = (uint8_t) GetPixelPtrD8(img, row+STEP_SIZE, col+STEP_SIZE, 0);
                                out = (uint8_t*)&GetPixelPtrD8(coMat, px1, px2, 2);
				*out=(uint8_t) fmin(*out+INC_VAL, MAX_COUNT);
				glcm->totalCount[2]++;
				#if S_MATRIX
				out = (uint8_t*)&GetPixelPtrD8(coMat, px2, px1, 2);
				*out=(uint8_t) fmin(*out+INC_VAL, MAX_COUNT);
				glcm->totalCount[2]++;
				#endif
			}
			if(col<(width-STEP_SIZE) ){
				//Populate horizontal matrix
				px2 = (uint8_t) GetPixelPtrD8(img, row, col+STEP_SIZE, 0);
				out = (uint8_t*)&GetPixelPtrD8(coMat, px1, px2, 0);
				*out=(uint8_t) fmin(*out+INC_VAL, MAX_COUNT);
				glcm->totalCount[0]++;
				#if S_MATRIX
				out = (uint8_t*)&GetPixelPtrD8(coMat, px2, px1, 0);
				*out=(uint8_t) fmin(*out+INC_VAL, MAX_COUNT);
				glcm->totalCount[0]++;
				#endif
				px1=px2;
			}
			else{
				uint8_t px1 =(uint8_t) GetPixelPtrD8(img, row, col, 0);
			}
		}
	}
}

void buildGLCMData(GLCMData* d){
	IplImage* coMat=d->coMat;
	int height=coMat->height;
	int width=coMat->width;
	IplImage* nMat=d->nMat;
	int directions=coMat->nChannels;	//Directions

	d->energy=0;
	d->entropy=0;
	d->contrast=0;
	d->homogeneity=0;
	d->correlation=0;

	//Normalize matrix
	float* norm;
	for(int row=0; row<height; row++){
		for(int col=0; col<width; col++){
			for(int dir=0; dir<directions; dir++){
				norm = &GetPixelPtrD32(nMat, row, col, dir);
				*norm=GetPixelPtrD8(coMat, row, col, dir);
				if(*norm > 0 && d->totalCount[dir] > 0){
					*norm /= d->totalCount[dir];
					d->energy+=(*norm )*(*norm);
					d->entropy+=*norm*log2f(*norm);
					d->contrast+=*norm*(row-col)*(row-col);
					d->homogeneity+=*norm / (1+ fabs(row-col));
				}/*
				else{
					*norm = 0.0;
				}*/
			}
		}
	}
	d->entropy*=-1;
}

/**
 *	Initializes a GLCMData struct
 *	@param	d	GLCMData struct to init
 *	@param	colorSpace	Size of the color space we are working with
 *	@param	directions	Number of directions to analyze, 3 is the max and default
 *	@return	Returns false if allocation failed
*/
bool initGLCMData(GLCMData* d, int colorSpace, int directions=3){
	if(d != NULL){
		for(int i=0; i<6; i++){
			d->centroidS[i]=0;
			if(i<3){
				d->centroid[i].x=0;
				d->centroid[i].y=0;
			}
		}

        if(d->coMat != NULL){
            cvReleaseImage(&d->coMat);	//Release in case this is a reinitialization
        }
        
        if(d->nMat != NULL){
            cvReleaseImage(&d->nMat);
        }
        
		d->coMat=cvCreateImage(cvSize(colorSpace, colorSpace), IPL_DEPTH_8U, directions);
		d->nMat=cvCreateImage(cvSize(colorSpace, colorSpace), IPL_DEPTH_32F, directions);
		if(d->coMat != NULL){
			cvSetZero(d->coMat);
			return true;
		}
	}
	return false;
}

/**
*	Releases a GLCMData struct and frees the IplImage stored within it
*	@param d	GLCMData struct to release
*/
void releaseGLCMData(GLCMData* d){
	if(d != NULL){
		cvReleaseImage(&d->coMat);
		cvReleaseImage(&d->nMat);
		delete d;
	}
}

char waitPause(){
	char keyVal=0;
	while(keyVal!='p' && keyVal!='q'){
		keyVal=cvWaitKey(50);
		if(keyVal=='q'){
			return 'q';
		}
		if(updateNeeded){
			return 'p';
		}
	}
	return 0;
}

void roiHandler(int event, int x, int y, int flags, void* i){
	IplImage* image=(IplImage*)i;
	switch(event){
		case CV_EVENT_LBUTTONDOWN:
			roiStart.x=x;
			roiStart.y=y;
			roiMod=true;
			//printf("Start X=%i Y=%i\n", x, y);
			//updateNeeded=true;
			break;
		case CV_EVENT_LBUTTONUP:
			roiMod=false;
		case CV_EVENT_MOUSEMOVE:
			if(roiMod){
				int minX=(int)fmin(roiStart.x, x);
				int minY=(int)fmin(roiStart.y, y);
				int width=((int)fmax(roiStart.x, x)) - minX;
				int height=((int)fmax(roiStart.y, y)) - minY;
				//printf("minX=%i minY=%i width=%i height=%i\n", minX, minY, width, height);
				if(width > 1 && height > 1){
					cvSetImageROI(image, cvRect( minX, minY, width, height));
					//printf("ROI Set\n");
				}
				updateNeeded=true;
			}
			else if(roiSlide){
				CvRect imgROI=cvGetImageROI(image);
				if(imgROI.width > 1 && imgROI.height > 1){
					cvSetImageROI(image, cvRect( x, y, imgROI.width, imgROI.height));
				}
				updateNeeded=true;
			}
			break;
		case CV_EVENT_RBUTTONDOWN:
			roiSlide=true;
			roiMod=false;
			break;
		case CV_EVENT_RBUTTONUP:
			roiSlide=false;
			break;
	}
}

