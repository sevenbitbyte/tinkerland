/**
 *	@author	Alan Meekins
 *	@date	9/8/09
**/

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/time.h>
#include <list>
#include <vector>
#include <sstream>
#include "kdnode.h"
#include "routines.h"

uint32_t targetHist[256][3];

/*#define GetPixelPtrD8(i,r,c,chan) i->imageData[((r*i->widthStep)+(c*i->nChannels)+chan)]
#define GetPixelPtrD16(i,r,c,chan) i->imageData[((r*(i->widthStep/sizeof(uint16_t)))+(c*i->nChannels)+chan)]
#define GetPixelPtrD32(i,r,c,chan) ((float*)i->imageData)[((r*i->widthStep)/4+(c*i->nChannels)+chan)]*/

#define STARTUP_DELAY 0
#define DELTA_PX 2

using namespace std;

CvPoint roiStart;
bool roiMod=false;
bool roiSlide=false;
bool updateNeeded=true;
bool insertSeedPoints=false;
bool resetSeedPoints=true;
bool showSeedPoints=true;

//Functions
char waitPause();
void printUsage(char*);
IplImage* getFrame(CvCapture *);
void mouseHandler(int event, int x, int y, int flags, void* i);
void forceUpdate(int i);
string cvScalarToString(CvScalar val, int channels);

enum MediaType {Unknown=-1, Video=1, Photo=2, Camera=3};

ofstream dataFile;

int redTarget = 50;
int greenTarget = 50;
int blueTarget = 50;
int maxDistance = 100;

vector<CvPoint*> seedPoints;

int main(int argc, char** argv){
	string fileName="";
	int camIndex=-1;
    char flag;
    char options[]="hp:m:c::";
	char inputMedia=Unknown;
    CvCapture* capture=NULL;
	IplImage* original=NULL;
	IplImage* frame=NULL;


    while((flag=getopt(argc, argv, options)) != -1){
		switch(flag){
			case 'h':
				printUsage(argv[0]);
				exit(EXIT_SUCCESS);
			case 'c':
				if(optarg == 0){
					if(optind < argc){
						camIndex=atoi(argv[optind]);
					}
				}
				else{
					camIndex=atoi(optarg);
				}
				inputMedia=Camera;
				break;
			case 'p':
				fileName=optarg;
				inputMedia=Photo;
				break;
			case 'm':
				fileName=optarg;
				inputMedia=Video;
				break;
			default:
				printUsage(argv[0]);
				exit(-1);
		}
    }

    if(inputMedia==Video){
		capture = cvCaptureFromAVI(fileName.c_str());
		if(!capture){
			cerr<<"Error: Could not open video "<<fileName<<endl;
			exit(-1);
		}
		cout<<"Reading video "<<fileName<<endl;
		frame=getFrame(capture);
    }

	if(inputMedia==Photo){
		original=cvLoadImage(fileName.c_str());
		if(original == NULL){
			cout<<"Error: Could not load photo "<<fileName<<endl;
			exit(-1);
		}

		cout<<"Input image depth="<<original->depth<<endl;

		int width = original->width;
		int height = original->height;
		if(original->width > 512 || original->height > 512){
			width = width/2;
			height = height/2;
			frame = cvCreateImage( cvSize(width, height), original->depth, original->nChannels );
			cvResize(original, frame);
		}
		else{
			frame=cvCreateImage( cvGetSize(original), original->depth, original->nChannels );
			cvCopy(original, frame);
		}
		cout<<"Loaded photo "<<fileName<<endl;
	}

    if(inputMedia==Camera){
		capture = cvCaptureFromCAM(camIndex);
		if(!capture){
			cerr<<"Error: Could not open camera "<<camIndex<<endl;
			exit(-1);
		}
		cout<<"Reading from camera "<<camIndex<<endl;
		frame=getFrame(capture);
    }

	if(inputMedia==Unknown){
		cout<<"Option error!"<<endl;
		printUsage(argv[0]);
		exit(-1);
	}


	IplImage* edgeImg=NULL;
	IplImage* normImg=NULL;
	IplImage* maskImg=NULL;
	IplImage* distImg=NULL;
	IplImage* mahalaDistImg=NULL;
	IplImage* mahalaMaskImg=NULL;
	IplImage* input = frame;
	IplImage* colorSpace =NULL;
	IplConvKernel* structElem=cvCreateStructuringElementEx(3, 3, 1, 1, CV_SHAPE_ELLIPSE);
	IplConvKernel* structElem1=cvCreateStructuringElementEx(9, 9, 5, 5, CV_SHAPE_ELLIPSE);

	if(frame->width > 512 || frame->height > 512){
		int width = frame->width/2;
		int height = frame->height/2;
		colorSpace=cvCreateImage(cvSize(width, height), frame->depth, frame->nChannels);
		cvResize(frame, colorSpace);
	}
	else{
		colorSpace=cvCreateImage(cvGetSize(frame), frame->depth, frame->nChannels);
	}

	cvNamedWindow("InputFrame1", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("ColorSpace", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("MaskedImage", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("DistanceFrame", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("MahalanobisDistanceFrame", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("MahalanobisMaskedImage", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Controls", CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback("InputFrame1", mouseHandler, (void*)colorSpace);
	cvSetMouseCallback("ColorSpace", mouseHandler, (void*)colorSpace);

	int colorIndex=3;
	int connectivity=0;
	int floodRange=1;
	int mahalaMaxDistance=7;

	cvCreateTrackbar("Max Distance", "Controls", &maxDistance, 100, forceUpdate);
	cvCreateTrackbar("Mahalanobis Max Distance", "Controls", &mahalaMaxDistance, 100, forceUpdate);
	cvCreateTrackbar("Color Space", "Controls", &colorIndex, 3, forceUpdate);
	cvCreateTrackbar("Connectivity 4 or 8", "Controls", &connectivity, 1, forceUpdate);
	cvCreateTrackbar("Floating or Fixed  Fload", "Controls", &floodRange, 1, forceUpdate);

	//IplConvKernel* closeElement = cvCreateStructuringElementEx(5, 5, 3, 3, CV_SHAPE_RECT);

	bool isPaused=false;
	bool stepMode=false;
	char keyVal='p';
	float milSecs=0;
	uint32_t frameCount=0;

	while(keyVal!='q'){
		if(keyVal=='r'){	//RESET
			updateNeeded=true;
		}
		if(keyVal=='s'){
			cout<<"Enter, step mode"<<endl;
			cout<<"\tn - Next frame"<<endl;
			cout<<"\tq - Quit"<<endl;
			cout<<"\ts - Exit step mode"<<endl;
			stepMode=true;
		}
		if(keyVal=='j'){
			int jump=frameCount;
			cout<<"Enter, jump mode @ frame="<<frameCount<<endl;
			cout<<"\tWhat frame to jump to? ";
			cin>>jump;

			while(frameCount < jump){
				frameCount++;
				frame=getFrame(capture);
				if(frame==NULL){
					cout<<"At end of stream, read "<<frameCount<<" frames"<<endl;
					exit(1);
				}

				if(frameCount%10==0){
					int width = frame->width;
					int height = frame->height;
					input = frame;
					if(input->width > 512 || input->height > 512){
						width = width/2;
						height = height/2;
						//input = cvCreateImage( cvSize(width, height), frame->depth, frame->nChannels );
						cvResize(frame, input);
					}
					cvShowImage("InputFrame1", input);
					cvWaitKey(10);
				}
				//cvWaitKey(2);
				cout<<"frame="<<frameCount<<endl;
			}

			cout<<"Jump complete!"<<endl;
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
					stepMode=false;
					cout<<"Exit program"<<endl;
					break;
				}
				else if(keyVal=='n'){	//Next frame
					cout<<"Step, frame="<<frameCount<<endl;
					updateNeeded=true;
					break;
				}
			}
		}

		if(updateNeeded || !isPaused){
			if(inputMedia != Photo){
				frameCount++;
				frame=getFrame(capture);
				if(frame==NULL){
					cout<<"At end of stream, read "<<frameCount<<" frames"<<endl;
					exit(1);
				}
				int width = frame->width;
				int height = frame->height;

				if(frame->width > 512 || frame->height > 512){
					width = width/2;
					height = height/2;
					if(input==frame){
						//cvReleaseImage(&input);
						input = cvCreateImage( cvSize(width, height), frame->depth, frame->nChannels );
					}
					cvResize(frame, input);
				}
				else{
					input=frame;
				}
			}
			else{
				int width = original->width;
				int height = original->height;
				if(original->width > 512 || original->height > 512){
					width = width/2;
					height = height/2;
					//input = cvCreateImage( cvSize(width, height), orginal->depth, orginal->nChannels );
					cvResize(original, input);
				}
				else{
					cvCopy(original, input);
				}
			}

			if(frameCount < STARTUP_DELAY){
				cout<<"Starting up"<< STARTUP_DELAY-frameCount <<endl;
				frameCount++;
			}
			else{
				//Time how long it takes to process
				timeval timeTemp;
				timeval timeDelta;
				gettimeofday(&timeTemp, NULL);

				/********* PROCESS IMAGE *******/


				if(maskImg==NULL){
					maskImg=cvCreateImage( cvSize(input->width, input->height), IPL_DEPTH_8U, 1 );
				}
				if(normImg==NULL){
					normImg=cvCreateImage( cvSize(input->width, input->height), IPL_DEPTH_8U, 3);
				}
				if(edgeImg==NULL){
					edgeImg=cvCreateImage( cvGetSize(input), IPL_DEPTH_8U, 1 );
				}
				if(distImg==NULL){
					distImg=cvCreateImage( cvGetSize(input), IPL_DEPTH_8U, 1 );
				}
				if(mahalaDistImg==NULL){
					mahalaDistImg=cvCreateImage( cvGetSize(input), IPL_DEPTH_8U, 1 );
				}
				if(mahalaMaskImg==NULL){
					mahalaMaskImg=cvCreateImage( cvGetSize(input), IPL_DEPTH_8U, 1 );
				}

				//cvSet(maskImg, cvScalarAll(255));
				cvZero(maskImg);

				if(colorIndex==1){
					cvCvtColor(input, colorSpace, CV_RGB2HSV);
				}
				else if(colorIndex==2){
					for(int row=0; row<input->height; row++){
						for(int col=0; col<input->width; col++){
							float red=(uint8_t)GetPixelPtrD8(input, row, col, 0);
							float green=(uint8_t)GetPixelPtrD8(input, row, col, 1);
							float blue=(uint8_t)GetPixelPtrD8(input, row, col, 2);

							float sum=red+green+blue;

							float r=red/sum;
							float g=green/sum;
							float b=blue/sum;

							GetPixelPtrD8(colorSpace, row, col, 0) = (uint8_t) (r * 255);
							GetPixelPtrD8(colorSpace, row, col, 1) = (uint8_t) (g * 255);
							GetPixelPtrD8(colorSpace, row, col, 2) = (uint8_t) (b * 255);

						}
					}
				}
				else if(colorIndex==3){
					cvCvtColor(input, colorSpace, CV_RGB2HLS);

					for(int row=0; row<input->height; row++){
						for(int col=0; col<input->width; col++){
							float red=(uint8_t)GetPixelPtrD8(input, row, col, 0);
							float green=(uint8_t)GetPixelPtrD8(input, row, col, 1);
							float blue=(uint8_t)GetPixelPtrD8(input, row, col, 2);

							float sum=red+green+blue;

							float r=red/sum;
							float g=green/sum;
							float b=blue/sum;

							GetPixelPtrD8(normImg, row, col, 0) = (uint8_t) (r * 255);
							GetPixelPtrD8(normImg, row, col, 1) = (uint8_t) (g * 255);
							GetPixelPtrD8(normImg, row, col, 2) = (uint8_t) (b * 255);

						}
					}
				}
				else{
					cvCopy(input, colorSpace);
				}

				CvScalar avgVal=cvScalarAll(0);
				CvScalar variance=cvScalarAll(0);
				CvScalar stdDev=cvScalarAll(0);
				double coVar[3][3]={{0,0,0},
									{0,0,0},
									{0,0,0}};
				double minDistanceVal=100;
				double maxDistanceVal=0;
				int minDistanceIndex=0;
				int minDistanceIndex2=0;
				int chans=colorSpace->nChannels;

				//Calaculate average
				for(int i=0; i<seedPoints.size(); i++){
						CvPoint* pt=seedPoints.at(i);

						for(int c=0; c<chans; c++){
							avgVal.val[c]+=(uint8_t)GetPixelPtrD8(colorSpace, pt->y, pt->x, c);
						}
				}

				for(int c=0; c<chans; c++){
					avgVal.val[c]/=seedPoints.size();
				}

				//Calculate Variance
				for(int i=0; i<seedPoints.size(); i++){
					CvPoint* pt=seedPoints.at(i);
					double distance=0.0;
					CvScalar distTemp=cvScalarAll(0);

					for(int c=0; c<chans; c++){
						distTemp.val[c]=((uint8_t)GetPixelPtrD8(colorSpace, pt->y, pt->x, c))-avgVal.val[c];
						distTemp.val[c]*=distTemp.val[c];
						variance.val[c]+=distTemp.val[c];
						distance+=distTemp.val[c];
					}

					coVar[0][1]+=(((uint8_t)GetPixelPtrD8(colorSpace,pt->y,pt->x,0))/255) * (((uint8_t)GetPixelPtrD8(colorSpace,pt->y,pt->x,1))/255);

					coVar[0][2]+=(((uint8_t)GetPixelPtrD8(colorSpace,pt->y,pt->x,0))/255) * (((uint8_t)GetPixelPtrD8(colorSpace,pt->y,pt->x,2))/255);

					coVar[1][2]+=(((uint8_t)GetPixelPtrD8(colorSpace,pt->y,pt->x,1))/255) * (((uint8_t)GetPixelPtrD8(colorSpace,pt->y,pt->x,2))/255);

					coVar[0][0]+=(((uint8_t)GetPixelPtrD8(colorSpace,pt->y,pt->x,0))/255) * (((uint8_t)GetPixelPtrD8(colorSpace,pt->y,pt->x,0))/255);

					coVar[1][1]+=(((uint8_t)GetPixelPtrD8(colorSpace,pt->y,pt->x,1))/255) * (((uint8_t)GetPixelPtrD8(colorSpace,pt->y,pt->x,1))/255);

					coVar[2][2]+=(((uint8_t)GetPixelPtrD8(colorSpace,pt->y,pt->x,2))/255) * (((uint8_t)GetPixelPtrD8(colorSpace,pt->y,pt->x,2))/255);

					distance=sqrt(distance);
					if(distance<minDistanceVal){
						minDistanceIndex=i;
						minDistanceVal=distance;
					}
					else if(distance<minDistanceIndex2){
						minDistanceIndex2=i;
					}

					if(distance>maxDistanceVal){
						maxDistanceVal=distance;
					}
				}

				coVar[0][1]=(coVar[0][1]/seedPoints.size())-(avgVal.val[0]/255)*(avgVal.val[1]/255);
				coVar[1][0]=coVar[0][1];

				coVar[0][2]=(coVar[0][2]/seedPoints.size())-(avgVal.val[0]/255)*(avgVal.val[2]/255);
				coVar[2][0]=coVar[0][2];

				coVar[1][2]=(coVar[1][2]/seedPoints.size())-(avgVal.val[1]/255)*(avgVal.val[2]/255);
				coVar[2][1]=coVar[1][2];

				coVar[0][0]=(coVar[0][0]/seedPoints.size())-(avgVal.val[0]/255)*(avgVal.val[0]/255);

				coVar[1][1]=(coVar[1][1]/seedPoints.size())-(avgVal.val[1]/255)*(avgVal.val[1]/255);

				coVar[2][2]=(coVar[2][2]/seedPoints.size())-(avgVal.val[2]/255)*(avgVal.val[2]/255);

				for(int c=0; c<chans; c++){
					variance.val[c]/=seedPoints.size();
					stdDev.val[c]=sqrt(variance.val[c])/2;

					//coVar[c][c]=variance.val[c];
				}


				cout<<"\tAvg="<<cvScalarToString(avgVal,chans)<<endl;
				cout<<"\tVariance="<<cvScalarToString(variance,chans)<<endl;
				cout<<"\tStdDev="<<cvScalarToString(stdDev,chans)<<endl;
				cout<<"\tMin Distance="<<minDistanceVal<<endl;
				cout<<"\tMax Distance="<<maxDistanceVal<<endl;
				cout<<"\tTarget index="<<minDistanceIndex<<endl;



				CvMat* invCoVarMat=cvCreateMat(3, 3, CV_32FC1);
				for(int i=0; i<3; i++){
					for(int j=0; j<3; j++){
						CvScalar v;
						v.val[0]=coVar[i][j];
						cvSet2D(invCoVarMat, i, j, v);
					}
				}

				cvInvert(invCoVarMat, invCoVarMat, CV_SVD);

				for(int i=0; i<3; i++){
					for(int j=0; j<3; j++){
						CvScalar v=cvGet2D(invCoVarMat, i, j);
						coVar[i][j]=v.val[0];
					}
				}

				cvReleaseMat(&invCoVarMat);

				cout<<"\tCoVar="<<endl;

				for(int i=0; i<3; i++){
					cout<<"\t\t[";
					for(int j=0; j<3; j++){
						cout<<coVar[i][j];
						if(j!=2){
							cout<<",";
						}
					}
					cout<<"]"<<endl;
				}

				for(int row=0; row<colorSpace->height; row++){
					for(int col=0; col<colorSpace->width; col++){
						float red=(uint8_t)GetPixelPtrD8(colorSpace, row, col, 0);
						float green=(uint8_t)GetPixelPtrD8(colorSpace, row, col, 1);
						float blue=(uint8_t)GetPixelPtrD8(colorSpace, row, col, 2);

						float rDelta=red- ((float)avgVal.val[0]);
						float gDelta=green- ((float)avgVal.val[1]);
						float bDelta=blue- ((float)avgVal.val[2]);
						float dist=fminf(sqrtf( (rDelta*rDelta) + (gDelta*gDelta) + (bDelta*bDelta) ), 255);

						GetPixelPtrD8(distImg, row, col, 0) = (uint8_t)(dist);

						//Computer Mahalanobis Distance

						rDelta/=255;
						gDelta/=255;
						bDelta/=255;

						float mahalDist=rDelta*(rDelta*coVar[0][0]+gDelta*coVar[0][1]+bDelta*coVar[0][2]);
						mahalDist+=gDelta*(rDelta*coVar[1][0]+gDelta*coVar[1][1]+bDelta*coVar[1][2]);
						mahalDist+=bDelta*(rDelta*coVar[2][0]+gDelta*coVar[2][1]+bDelta*coVar[2][2]);

						mahalDist=sqrtf(fabsf(mahalDist))*256;

						GetPixelPtrD8(mahalaDistImg, row, col, 0) = (uint8_t)(mahalDist);
						//cout<<"mahalDist="<<mahalDist<<endl;
					}
				}
				cvSmooth(maskImg, maskImg, CV_GAUSSIAN, 5);

				cvThreshold(distImg, maskImg, (double) maxDistance, 255, CV_THRESH_BINARY);


				cvThreshold(mahalaDistImg, mahalaMaskImg, (double) mahalaMaxDistance, 255, CV_THRESH_BINARY);

				cvErode(maskImg, maskImg, structElem);

				cvCvtColor(normImg, edgeImg, CV_BGR2GRAY);
				//cvSobel(edgeImg, edgeImg, 1, 0, 3);
				cvAdaptiveThreshold(edgeImg, edgeImg, (double)8);
				//cvSmooth(edgeImg, edgeImg, CV_GAUSSIAN, 3);
				//cvThreshold(edgeImg, edgeImg, (double) 8, 255, CV_THRESH_BINARY_INV);

				cvAnd(maskImg, edgeImg, maskImg);

				//cvErode(maskImg, maskImg, structElem);
				//cvMorphologyEx(maskImg, maskImg, NULL, structElem, CV_MOP_CLOSE);
				//cvMorphologyEx(maskImg, maskImg, NULL, structElem1, CV_MOP_OPEN);

				colorMaximization(colorSpace,colorSpace);

				if(seedPoints.size()>0){
					int flags=4+4*connectivity;
					flags|=CV_FLOODFILL_FIXED_RANGE*floodRange;
					//flags|=CV_FLOODFILL_MASK_ONLY;
					cout<<"Flooding"<<flags<<endl;

					CvScalar diff=cvScalarAll(maxDistance);
					if(maxDistance==0){
						diff=stdDev;
					}

					//cvFloodFill(colorSpace, *seedPoints.at(minDistanceIndex), cvScalarAll(255), diff, diff, NULL, flags, NULL);
				}


				/********* END PROCESSING *******/

				gettimeofday(&timeDelta, NULL);
				timeDelta.tv_sec-=timeTemp.tv_sec;
				timeDelta.tv_usec-=timeTemp.tv_usec;
				milSecs=timeDelta.tv_sec*1000 + timeDelta.tv_usec/1000;
				cout<<" - Processed frame in "<<milSecs<<"ms."<<endl;
			}

			for(int i=0; i<seedPoints.size(); i++){
					CvPoint* pt=seedPoints.at(i);
					cvCircle(input, *pt, 2, cvScalar(0,0,255), 1);
			}

			cvShowImage("InputFrame1", input);
			cvShowImage("ColorSpace", colorSpace);
			cvShowImage("MaskedImage", maskImg);
			cvShowImage("DistanceFrame", distImg);
			cvShowImage("MahalanobisDistanceFrame", mahalaDistImg);
			cvShowImage("MahalanobisMaskedImage", mahalaMaskImg);
			updateNeeded=false;
		}
		else if(showSeedPoints){
			if(inputMedia==Photo){
				int width = original->width;
				int height = original->height;
				if(original->width > 512 || original->height > 512){
					width = width/2;
					height = height/2;
					cvResize(original, input);
				}
				else{
					cvCopy(original, input);
				}
			}

			for(int i=0; i<seedPoints.size(); i++){
					CvPoint* pt=seedPoints.at(i);
					cvCircle(input, *pt, 2, cvScalar(0,0,255), 1);
			}

			showSeedPoints=false;
			cvShowImage("InputFrame1", input);
		}

		if(keyVal=='p'){
			isPaused=!isPaused;
			if(isPaused){
				cout<<"Paused @ frame="<<frameCount<<endl;
			}
			else{
				cout<<"Unpaused"<<endl;
			}
		}
		if(keyVal=='q'){
			break;
		}
		keyVal=cvWaitKey(10);
	}

	//dataFile.close();

	printf("Cleaning up\n");

	cvDestroyAllWindows();

	//cvReleaseStructuringElement(&closeElement);

	if(inputMedia==Photo){
		//cvReleaseImage(&frame);
		cvReleaseImage(&original);
	}
	else{
		cvReleaseCapture(&capture);
		cvReleaseImage(&input);
	}

	cvReleaseStructuringElement(&structElem);

	if(colorSpace!=NULL){
		cvReleaseImage(&colorSpace);
	}

	if(maskImg!=NULL){
		cvReleaseImage(&maskImg);
	}

	if(normImg!=NULL){
		cvReleaseImage(&normImg);
	}

	if(edgeImg!=NULL){
		cvReleaseImage(&edgeImg);
	}

	if(distImg!=NULL){
		cvReleaseImage(&distImg);
	}

	if(mahalaDistImg!=NULL){
		cvReleaseImage(&mahalaDistImg);
	}

	if(mahalaMaskImg!=NULL){
		cvReleaseImage(&mahalaMaskImg);
	}

	while(!seedPoints.empty()){
		delete seedPoints.back();
		seedPoints.pop_back();
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

void mouseHandler(int event, int col, int row, int flags, void* i){
	if(i==NULL){
		return;
	}

	/*if(flags&CV_EVENT_FLAG_SHIFTKEY){
		resetSeedPoints=false;
	}*/
	if(event==CV_EVENT_LBUTTONDOWN){
		if(!(flags&CV_EVENT_FLAG_SHIFTKEY)){
			seedPoints.clear();
		}
		insertSeedPoints=true;

		CvPoint* pt=new CvPoint;
		pt->x=col;
		pt->y=row;
		seedPoints.push_back(pt);
	}
	else if(event==CV_EVENT_LBUTTONUP){
		cout<<"INFO: Have "<<seedPoints.size()<<" seed points"<<endl;
		updateNeeded=true;
		resetSeedPoints=true;
		insertSeedPoints=false;
	}
	else if(event==CV_EVENT_RBUTTONDOWN){
		roiStart=cvPoint(col, row);
	}
	else if(event==CV_EVENT_RBUTTONUP){
		roiStart=cvPoint(0, 0);
		updateNeeded=true;
	}
	else if(event==CV_EVENT_MOUSEMOVE){
		if(insertSeedPoints){
			CvPoint* pt=new CvPoint;
			pt->x=col;
			pt->y=row;
			seedPoints.push_back(pt);
			showSeedPoints=true;
		}

		if(roiStart.x!=0 && roiStart.y!=0){
			CvPoint delta=cvPoint(col-roiStart.x, row-roiStart.y);
			for(int i=0; i<seedPoints.size(); i++){
				CvPoint* pt=seedPoints.at(i);

				pt->x+=delta.x;
				pt->y+=delta.y;
			}
			showSeedPoints=true;
			roiStart=cvPoint(col, row);
		}
	}
}

IplImage* getFrame(CvCapture *cvCpt){
    if(!cvGrabFrame(cvCpt)){
        cerr<<"Warning: Could not grab frame"<<endl;
        return NULL;
    }
    return cvRetrieveFrame(cvCpt);
}

void forceUpdate(int i){
	updateNeeded=true;
}


void printUsage(char *name){
	cout << "Usage: " << name << " [options]" << endl << endl;
	cout << "\t-c\t[camId]\tRead from camera" << endl;
	cout << "\t-m\t[movie]\tRead video from file" << endl;
	cout << "\t-p\t[photo]\tRead photo from file" << endl;
}


string cvScalarToString(CvScalar val, int channels=3){
	stringstream stream;

	stream<<"[";
	for(int i=0; i<channels; i++){
		stream<<val.val[i];
		if(i+1!=channels){
			stream<<", ";
		}
	}
	stream<<"]";

	return stream.str();
}
