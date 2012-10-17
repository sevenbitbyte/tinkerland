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

#define GetPixelPtrD8(i,r,c,chan) i->imageData[((r*i->widthStep)+(c*i->nChannels)+chan)]
#define GetPixelPtrD16(i,r,c,chan) i->imageData[((r*(i->widthStep/sizeof(uint16_t)))+(c*i->nChannels)+chan)]
#define GetPixelPtrD32(i,r,c,chan) ((float*)i->imageData)[((r*i->widthStep)/4+(c*i->nChannels)+chan)]

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
int maxDistance = 7;

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
	IplImage* laplaceImg=NULL;
	//IplImage* distImg=NULL;
	IplImage* input = frame;
	IplImage* colorSpace =NULL;

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
	//cvNamedWindow("MaskedImage", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Edges", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Controls", CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback("InputFrame1", mouseHandler, (void*)colorSpace);
	cvSetMouseCallback("ColorSpace", mouseHandler, (void*)colorSpace);

	int colorIndex=2;
	int apertureSize=1;
	int edgeDetector=0;
	int cannyThresh1=10;
	int cannyThresh2=5;
	int maxDistance=8;
	//int connectivity=0;
	//int floodRange=1;

	//cvCreateTrackbar("Max Distance", "Controls", &maxDistance, 100, forceUpdate);
	cvCreateTrackbar("Color Space", "Controls", &colorIndex, 3, forceUpdate);
	cvCreateTrackbar("Aperture Size", "Controls", &apertureSize, 3, forceUpdate);
	cvCreateTrackbar("Edge Detector", "Controls", &edgeDetector, 3, forceUpdate);
	cvCreateTrackbar("Canny Threshold 1", "Controls", &cannyThresh1, 1000, forceUpdate);
	cvCreateTrackbar("Canny Threshold 2", "Controls", &cannyThresh2, 1000, forceUpdate);
	cvCreateTrackbar("Threshold", "Controls", &maxDistance, 255, forceUpdate);
	//cvCreateTrackbar("Floating or Fixed  Fload", "Controls", &floodRange, 1, forceUpdate);

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


				if(edgeImg==NULL){
					edgeImg=cvCreateImage( cvSize(input->width, input->height), IPL_DEPTH_8U, 1 );
				}

				if(laplaceImg==NULL){
					laplaceImg=cvCreateImage( cvSize(input->width, input->height), IPL_DEPTH_16S, 1 );
				}

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
							//float b=blue/sum;
							float b=1-(r+g);

							GetPixelPtrD8(colorSpace, row, col, 0) = (uint8_t) (r * 255);
							GetPixelPtrD8(colorSpace, row, col, 1) = (uint8_t) (g * 255);
							GetPixelPtrD8(colorSpace, row, col, 2) = (uint8_t) (b * 255);

						}
					}
				}
				else if(colorIndex==3){
					cvCvtColor(input, colorSpace, CV_RGB2HLS);
				}
				else{
					cvCopy(input, colorSpace);
				}

				//cvSmooth(colorSpace, colorSpace, CV_GAUSSIAN, 3);
				cvCvtColor(colorSpace, edgeImg, CV_BGR2GRAY);

				if(edgeDetector==0){
					cvSobel(edgeImg, edgeImg, 1, 0, (2*apertureSize)+1);

					//cvAdaptiveThreshold(edgeImg, edgeImg, (double)255);

					//cvSmooth(edgeImg, edgeImg, CV_GAUSSIAN, 3);
					//cvThreshold(edgeImg, edgeImg, (double) maxDistance, 255, CV_THRESH_BINARY);

					//IplConvKernel* structElem=cvCreateStructuringElementEx(5, 5, 3, 3, CV_SHAPE_ELLIPSE);
					//cvErode(edgeImg, edgeImg, structElem);
					//cvMorphologyEx(edgeImg, edgeImg, NULL, structElem, CV_MOP_CLOSE);
					//cvReleaseStructuringElement(&structElem);
				}
				else if(edgeDetector==2){
					cvLaplace(edgeImg, laplaceImg, (2*apertureSize)+1);
					cvCmpS(laplaceImg, (double)maxDistance, edgeImg, CV_CMP_GT);
				}
				else if(edgeDetector==1){
					cvCanny(edgeImg, edgeImg, (double)cannyThresh1, (double)cannyThresh2, (2*apertureSize)+1);

					cvSmooth(edgeImg, edgeImg, CV_GAUSSIAN, 3);
					cvThreshold(edgeImg, edgeImg, (double) maxDistance, 255, CV_THRESH_BINARY);
				}

				//cvSmooth(edgeImg, edgeImg, CV_GAUSSIAN, 3);
				//cvThreshold(edgeImg, edgeImg, (double) maxDistance, 255, CV_THRESH_BINARY);


				/********* END PROCESSING *******/

				gettimeofday(&timeDelta, NULL);
				timeDelta.tv_sec-=timeTemp.tv_sec;
				timeDelta.tv_usec-=timeTemp.tv_usec;
				milSecs=timeDelta.tv_sec*1000 + timeDelta.tv_usec/1000;
				cout<<" - Processed frame in "<<milSecs<<"ms."<<endl;
			}


			cvShowImage("InputFrame1", input);
			cvShowImage("ColorSpace", colorSpace);
			//cvShowImage("MaskedImage", maskImg);
			//if(edgeDetector!=2){
				cvShowImage("Edges", edgeImg);
			/*}
			else{
				cvShowImage("Edges", laplaceImg);
			}*/
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


	if(colorSpace!=NULL){
		cvReleaseImage(&colorSpace);
	}

	if(edgeImg!=NULL){
		cvReleaseImage(&edgeImg);
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
