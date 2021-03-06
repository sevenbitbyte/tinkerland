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

char waitPause();
void printUsage(char*);
IplImage* getFrame(CvCapture *);


void mouseHandler(int event, int x, int y, int flags, void* i);
// void minThreshUpdate(int i);
// void maxThreshUpdate(int i);

void forceUpdate(int i);

enum MediaType {Unknown=-1, Video=1, Photo=2, Camera=3};

ofstream dataFile;

int redTarget = 50;
int greenTarget = 50;
int blueTarget = 50;
int maxDistance = 5;

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


	IplImage* tempImg=NULL;
	IplImage* maskImg=NULL;
	IplImage* distImg=NULL;
	IplImage* input = frame;
	IplImage* colorSpace = cvCreateImage(cvGetSize(frame), frame->depth, frame->nChannels);
	IplImage* colorChannel=cvCreateImage(cvGetSize(frame), frame->depth, 1);

	cvNamedWindow("InputFrame1", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("ColorSpace", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("MaskedImage", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("DistanceFrame", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Controls", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("ColorChannel", CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback("ColorSpace", mouseHandler, (void*)colorSpace);


	int level=1;
	int colorIndex=1;
	int targetChan=1;

	cvCreateTrackbar("Red Target", "Controls", &redTarget, 255, forceUpdate);
	cvCreateTrackbar("Green Target", "Controls", &greenTarget, 255, forceUpdate);
	cvCreateTrackbar("Blue Target", "Controls", &blueTarget, 255, forceUpdate);
	cvCreateTrackbar("Max Distance", "Controls", &maxDistance, 100, forceUpdate);
	cvCreateTrackbar("Tree Level", "Controls", &level, 100, forceUpdate);
	cvCreateTrackbar("Color Space", "Controls", &colorIndex, 3, forceUpdate);
	cvCreateTrackbar("Target Color Channel", "Controls", &targetChan, 2, forceUpdate);

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
				input = frame;
				if(input->width > 512 || input->height > 512){
					width = width/2;
					height = height/2;
					//input = cvCreateImage( cvSize(width, height), frame->depth, frame->nChannels );
					cvResize(frame, input);
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
					maskImg=cvCreateImage( cvGetSize(input), IPL_DEPTH_8U, 1 );
				}
				if(tempImg==NULL){
					tempImg=cvCreateImage( cvGetSize(input), IPL_DEPTH_8U, 1 );
				}
				if(distImg==NULL){
					distImg=cvCreateImage( cvGetSize(input), IPL_DEPTH_8U, 1 );
				}

				cvSetZero(maskImg);
				if(colorIndex==1){
					cvCvtColor(input, colorSpace, CV_RGB2HSV);
				}
				else if(colorIndex==2){
					cvCvtColor(input, colorSpace, CV_RGB2HLS);
				}
				else{
					cvCopy(input, colorSpace);
				}

				cvSmooth(colorSpace, colorSpace, CV_GAUSSIAN, 3);

				IplImage* channels[3]={NULL, NULL, NULL};
				channels[targetChan]=colorChannel;
				cvSplit(colorSpace, channels[0], channels[1], channels[2], NULL);
				//cvSobel(colorChannel, colorChannel, 1, 1, 3);

				//vector<CvPoint*> points;
				int targetVal=redTarget;
				if(targetChan==1){
					targetVal=greenTarget;
				}
				else if(targetChan==2){
					targetVal=blueTarget;
				}

				for(int row=0; row<colorSpace->height; row++){
					for(int col=0; col<colorSpace->width; col++){
						float red=(uint8_t)GetPixelPtrD8(colorSpace, row, col, targetChan);

						float rDelta=red- ((float)targetVal);
						float dist=fminf(sqrtf( (rDelta*rDelta)  ), 255);

						/*if(dist < maxDistance){
							CvPoint* p=new CvPoint;
							p->x=col;
							p->y=row;
							points.push_back(p);
						}*/

						GetPixelPtrD8(distImg, row, col, 0) = (uint8_t)(dist);
					}
				}

				//cvMorphologyEx(maskImg, maskImg, tempImg, closeElement, CV_MOP_CLOSE, 1);

				cvThreshold(distImg, maskImg, (double) maxDistance, 255, CV_THRESH_BINARY);

				/*cout<<"Have "<<points.size()<<" points"<<endl;
				nodeCount=0;
				killCount=0;
				KDNode* node=KDNode::buildTree(points, level+1);
				if(node!=NULL){
					cout<<"Tree has "<<nodeCount<<" nodes and aborted "<<killCount<<endl;
					drawNodes(input, node, level);
					delete node;
				}*/

				/********* END PROCESSING *******/

				gettimeofday(&timeDelta, NULL);
				timeDelta.tv_sec-=timeTemp.tv_sec;
				timeDelta.tv_usec-=timeTemp.tv_usec;
				milSecs=timeDelta.tv_sec*1000 + timeDelta.tv_usec/1000;
				cout<<" - Processed frame in "<<milSecs<<"ms."<<endl;
			}

			cvShowImage("InputFrame1", input);
			cvShowImage("ColorSpace", colorSpace);
			cvShowImage("MaskedImage", maskImg);
			cvShowImage("DistanceFrame", distImg);
			cvShowImage("ColorChannel", colorChannel);
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

	if(maskImg!=NULL){
		cvReleaseImage(&maskImg);
	}

	if(tempImg!=NULL){
		cvReleaseImage(&tempImg);
	}

	if(distImg!=NULL){
		cvReleaseImage(&distImg);
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

	if(event==CV_EVENT_LBUTTONDOWN){
		IplImage* input=(IplImage*)i;
		float red=(uint8_t)GetPixelPtrD8(input, row, col, 0);
		float green=(uint8_t)GetPixelPtrD8(input, row, col, 1);
		float blue=(uint8_t)GetPixelPtrD8(input, row, col, 2);


		cout<<endl<<"Red="<<red<<" Green="<<green<<" Blue="<<blue<<endl;

		cout<<"redTarget="<<redTarget<<" greenTarget="<<greenTarget<<" blueTarget="<<blueTarget<<endl;

		float rDelta=red- ((float)redTarget);
		float gDelta=green- ((float)greenTarget);
		float bDelta=blue- ((float)blueTarget);
		float dist=sqrtf( (rDelta*rDelta) + (gDelta*gDelta) + (bDelta*bDelta) );

		cout<<"rDelta="<<rDelta<<" gDelta="<<gDelta<<" bDelta="<<bDelta<<" dist="<<dist<<endl<<endl;
	}
	else if(event==CV_EVENT_RBUTTONDOWN){
		IplImage* input=(IplImage*)i;
		float red=(uint8_t)GetPixelPtrD8(input, row, col, 0);
		float green=(uint8_t)GetPixelPtrD8(input, row, col, 1);
		float blue=(uint8_t)GetPixelPtrD8(input, row, col, 2);

		float rDelta=red- ((float)redTarget);
		float gDelta=green- ((float)greenTarget);
		float bDelta=blue- ((float)blueTarget);
		float dist=sqrtf( (rDelta*rDelta) + (gDelta*gDelta) + (bDelta*bDelta) );

		cout<<endl<<"OldTarget: redTarget="<<redTarget<<" greenTarget="<<greenTarget<<" blueTarget="<<blueTarget<<endl;
		cout<<"Updating to R="<<red<<" G="<<green<<" B="<<blue<<endl;

		cvSetTrackbarPos("Red Target", "Controls", (int)(red));
		cvSetTrackbarPos("Green Target", "Controls", (int)(green));
		cvSetTrackbarPos("Blue Target", "Controls", (int)(blue));

		cout<<"NewTarget: redTarget="<<redTarget<<" greenTarget="<<greenTarget<<" blueTarget="<<blueTarget<<endl<<endl;
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
