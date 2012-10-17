#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <math.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/time.h>

#define COLOR_SPACE 256
#define MAX_COUNT 65535

using namespace std;

void createGLCM(IplImage*,IplImage*,uint16_t*, uint16_t*);
void mouseHandler(int event, int x, int y, int flags, void* i);

int main(int argc, char** argv){

	if(argc<2){
		printf("Usage: %s [file1] <file2>\n", argv[0]);
		exit(1);
	}

	IplImage* input1=cvLoadImage(argv[1]);
	if(input1 == NULL){
		cout<<"Error: Could not load "<<argv[1]<<endl;
		exit(-1);
	}

	IplImage* input2=cvLoadImage(argv[2]);
	if(input2 == NULL){
		cout<<"Error: Could not load "<<argv[2]<<endl;
		exit(-1);
	}

	IplImage* grey1=cvCreateImage(cvSize(input1->width, input1->height), IPL_DEPTH_8U, 1);
	IplImage* glcm1H=cvCreateImage(cvSize(COLOR_SPACE, COLOR_SPACE), IPL_DEPTH_16U, 1);

	//uint16_t* hMatrix=(uint16_t*)glcm1H->imageData; //new uint16_t[COLOR_SPACE*COLOR_SPACE];
	uint16_t* vMatrix=new uint16_t[COLOR_SPACE*COLOR_SPACE];
	uint16_t* dMatrix=new uint16_t[COLOR_SPACE*COLOR_SPACE];

	unsigned int hCount=0;
	unsigned int vCount=0;
	unsigned int dCount=0;	

	cvCvtColor(input1, grey1, CV_BGR2GRAY);

        timeval timeTemp;
        timeval timeDelta;
        gettimeofday(&timeTemp, NULL);
	createGLCM(grey1, glcm1H, vMatrix, dMatrix);
	gettimeofday(&timeDelta, NULL);
        timeDelta.tv_sec-=timeTemp.tv_sec;
        timeDelta.tv_usec-=timeTemp.tv_usec;

        float milSecs=timeDelta.tv_sec*1000 + timeDelta.tv_usec/1000;

        cout<<"Created glcm in "<<milSecs<<"ms."<<endl;


	cvNamedWindow("Input1", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Grey1", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("GLCM-1 Horizontal", CV_WINDOW_AUTOSIZE);

	cvShowImage("Input1", input1);
	cvShowImage("Grey1", grey1);
	cvShowImage("GLCM-1 Horizontal", glcm1H);

	cvSetMouseCallback("GLCM-1 Horizontal",mouseHandler, (void*)glcm1H);

	cvWaitKey(0);

	printf("Cleaning up\n");	

	//delete[] hMatrix;
	delete[] vMatrix;
	delete[] dMatrix;

	cvReleaseImage(&input1);
	cvReleaseImage(&grey1);
	cvReleaseImage(&glcm1H);
	cvDestroyAllWindows();
}


/**
 *	Creates a grey level co-occurance matrix
 *	@param	img	Image to analyze
 *	@param	hMat	Horizontal matrix
 *	@param	vMat	Vertical Matrix
 *	@param	dMat	Diagonal Matrix
 */
void createGLCM(IplImage* img, IplImage* hMat, uint16_t* vMat, uint16_t* dMat){
	int height=img->height;
	int width=img->width;
	/*int wStep=img->widthStep;
	char* data=img->imageData;*/

	for(int row=0; row<height; row++){
		for(int col=0; col<width; col++){
			CvScalar px1=cvGet2D(img, row, col);
			CvScalar px2;
			CvScalar out;
			if(col<(width-1) && hMat!=NULL){
				//Populate horizontal matrix
				px2=cvGet2D(img, row, col+1);
				out=cvGet2D(hMat, (int)px1.val[0], (int)px2.val[0]);
				if(out.val[0] < MAX_COUNT){
					out.val[0]++;
					cvSet2D(hMat, (int)px1.val[0], (int)px2.val[0], out);
				}
			}
			if(row<height && vMat!=NULL){
				//Populate vertical matrix
			}
			if(col<width && row<height && dMat!=NULL){
				//Populate diagnal matrix
			}
		}
	}
}


void mouseHandler(int event, int x, int y, int flags, void* i){
	IplImage* image=(IplImage*)i;
	switch(event){
		case CV_EVENT_LBUTTONDOWN:
			/*if(flags & CV_EVENT_FLAG_CTRLKEY){
				printf("Left button down with CTRL pressed\n");
			}*/
			CvScalar s;
			for(int i=y; i<image->height; i++){
				for(int j=x; j<image->width; j++){
					s=cvGet2D(image, i,j);
					if(s.val[0] > 0){
						cout<<"X="<<j<<" Y="<<i<<" Value="<<s.val[0]<<endl;
					}
				}
			}
			break;
		case CV_EVENT_MOUSEMOVE:
			//CvScalar s=cvGet2D(image, y, x);
			//printf("Click X = %i  Y = %i Value=%i\n", x, y, s.val[0]);
			break;
		//case CV_EVENT_MOUSEMOVE:
			//printf("Move\n");
			//break;
	}
}

