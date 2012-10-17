 
/**
 *	@author	Alan Meekins
 *	@date	5/30/2010
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
#include <cstdio>
#include <sstream>
#include "routines.h"

uint32_t targetHist[256][3];

#define STARTUP_DELAY 0
#define DELTA_PX 2

#define DegToRad(x) x*(M_PI/180.0)

#define HFIELD_OF_VIEW DegToRad(62.7495)
#define VFIELD_OF_VIEW DegToRad(48.5677)

#define CAM_HEIGHT 1.3335	//Meters
#define CAM_PITCH DegToRad(50.52058266)	//

using namespace std;

CvPoint roiStart;
bool roiMod=false;
bool roiSlide=false;
bool updateNeeded=true;
bool getNewFrame=true;
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


	IplImage* input=frame;
	IplImage* flatImg=flatImg=cvCreateImage(cvGetSize(input), input->depth, input->nChannels);
	double radPerPxH=HFIELD_OF_VIEW/((double)frame->width);
	double radPerPxV=VFIELD_OF_VIEW/((double)frame->height);

	double maxPitch=CAM_PITCH+(VFIELD_OF_VIEW/2);
	double minPitch=CAM_PITCH-(VFIELD_OF_VIEW/2);
	double maxYDist=CAM_HEIGHT*tan(maxPitch);
	double minYDist=CAM_HEIGHT*tan(minPitch);
	double yDelta=fabs(maxYDist-minYDist);

	double maxYaw=HFIELD_OF_VIEW/2;
	double maxXDist=maxYDist*tan(maxYaw);
	double xDelta=2*maxXDist;

	double pixelsPerMeter=(double)flatImg->width/xDelta;

	cout<<"radians per pixel Hori = "<<radPerPxH<<endl;
	cout<<"radians per pixel Vert = "<<radPerPxV<<endl;
	cout<<"Max Visable Pitch = "<<maxPitch<<endl;
	cout<<"Min Visable Pitch = "<<minPitch<<endl;
	cout<<"Max Y Distance = "<<maxYDist<<endl;
	cout<<"Min Y Distance = "<<minYDist<<endl;
	cout<<"Y Delta = "<<yDelta<<endl;

	cout<<"Max Visable Yaw = "<<maxYaw<<endl;
	cout<<"Max X Distance = "<<maxXDist<<endl;
	cout<<"X Delta = "<<xDelta<<endl;
	cout<<endl;

	//exit(1);

	/*if(frame->width > 512 || frame->height > 512){
		int width = frame->width/2;
		int height = frame->height/2;
		colorSpace=cvCreateImage(cvSize(width, height), frame->depth, frame->nChannels);
		cvResize(frame, colorSpace);
	}
	else{
		colorSpace=cvCreateImage(cvGetSize(frame), frame->depth, frame->nChannels);
	}*/

	cvNamedWindow("Input", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("FlatImage", CV_WINDOW_AUTOSIZE);

	bool isPaused=false;
	bool stepMode=false;
	char keyVal='p';
	float milSecs=0;
	uint32_t frameCount=0;

	while(keyVal!='q'){
		if(keyVal=='r'){	//RESET
			seedPoints.clear();
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
					cvShowImage("Input", input);
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
					getNewFrame=true;
					break;
				}
			}
		}

		if(updateNeeded || !isPaused || getNewFrame){
			if(inputMedia != Photo){
				if(!isPaused || getNewFrame){
					frameCount++;
					frame=getFrame(capture);
					if(frame==NULL){
						cout<<"At end of stream, read "<<frameCount<<" frames"<<endl;
						exit(1);
					}
					getNewFrame=false;
				}
				int width = frame->width;
				int height = frame->height;

				if(frame->width > 700 || frame->height > 512){
					width = width/2;
					height = height/2;
					if(input==frame){
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
					cvResize(original, input);
				}
				else{
					cvCopy(original, input);
				}
			}

			if(flatImg==NULL){
				flatImg=cvCreateImage(cvGetSize(input), input->depth, input->nChannels);
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

				for(int row=0; row<input->height; row++){
					double rowPitch=CAM_PITCH+(VFIELD_OF_VIEW/2.0 - ((double)row*radPerPxV));
					double yDist=CAM_HEIGHT*tan(rowPitch);
					for(int col=0; col<input->width; col++){
						double colYaw=(radPerPxH*(double)col)-(HFIELD_OF_VIEW/2.0);
						double xDist=yDist*tan(colYaw);

						int pxCol=(int)(xDist*pixelsPerMeter)+(flatImg->width/2);

						int pxRow=flatImg->height-(int)((yDist-minYDist)*pixelsPerMeter);


						GetPixelPtrD8(flatImg,pxRow,pxCol,0)=(uint8_t)GetPixelPtrD8(input,row,col,0);
						GetPixelPtrD8(flatImg,pxRow,pxCol,1)=(uint8_t)GetPixelPtrD8(input,row,col,1);
						GetPixelPtrD8(flatImg,pxRow,pxCol,2)=(uint8_t)GetPixelPtrD8(input,row,col,2);
					}
				}


				/********* END PROCESSING *******/

				gettimeofday(&timeDelta, NULL);
				timeDelta.tv_sec-=timeTemp.tv_sec;
				timeDelta.tv_usec-=timeTemp.tv_usec;
				milSecs=((float)timeDelta.tv_sec)*1000 + ((float)timeDelta.tv_usec)/1000;

				cout<<" - Processed frame in "<<milSecs<<"ms."<<endl;
			}

			cvShowImage("Input", input);
			cvShowImage("FlatImage", flatImg);
			updateNeeded=false;
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


	printf("Cleaning up\n");

	cvDestroyAllWindows();


	if(inputMedia==Photo){
		//cvReleaseImage(&frame);
		cvReleaseImage(&original);
	}
	else{
		cvReleaseCapture(&capture);
		cvReleaseImage(&input);
	}


	if(flatImg!=NULL){
		cvReleaseImage(&flatImg);
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
