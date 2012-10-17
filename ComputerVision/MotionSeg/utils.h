#ifndef UTILS_H
#define UTILS_H

/*
  *	This header implements all frame grabbing and playback control
  *	image processing algorithms are implemented on top of this functionality
  *	by implementing the processImage() function in a source file which includes
  *	this header. The preProcess() and postProcess() functions are used for 
  *	routines that are not part of the core algorithm which will not be timed.
  *	These functions are intended for any image display, resizing or allocation
  *	that may be neeeded but could take along time.
*/

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/time.h>

using namespace std;

#define STARTUP_DELAY 0

enum MediaType {Unknown=-1, Video=1, Photo=2, Camera=3};

bool allocateUserData(IplImage* input);
void deallocateUserData();
bool processImage(IplImage* input);
bool preProcess(IplImage* input);
bool postProcess(IplImage* input);

char waitPause();
void printUsage(char*);
IplImage* getFrame(CvCapture* frame);


CvCapture* capture=NULL;
IplImage* orginal=NULL;
IplImage* frame=NULL;
char inputMedia=Unknown;
bool updateNeeded=true;
bool resetRequest=false;
uint32_t frameCount=1;

bool parseArgs(int argc, char** argv){
	string fileName="";
	int camIndex=-1;
    char flag;
    char options[]="hp:m:c::";
	
    while((flag=getopt(argc, argv, options)) != -1){
		switch(flag){
			case 'h':
				printUsage(argv[0]);
				return false;
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
				return false;
		}
    }

    if(inputMedia==Video){
		capture = cvCaptureFromAVI(fileName.c_str());
		if(!capture){
			cerr<<"Error: Could not open video "<<fileName<<endl;
			return false;
		}
		cout<<"Reading video "<<fileName<<endl;
		frame=getFrame(capture);
    }

	if(inputMedia==Photo){
		orginal=cvLoadImage(fileName.c_str());
		if(orginal == NULL){
			cout<<"Error: Could not load photo "<<fileName<<endl;
			return false;
		}
		frame=cvCreateImage( cvGetSize(orginal), orginal->depth, orginal->nChannels );
		cvCopy(orginal, frame);
		cout<<"Loaded photo "<<fileName<<endl;
	}

    if(inputMedia==Camera){
		capture = cvCaptureFromCAM(camIndex);
		if(!capture){
			cerr<<"Error: Could not open camera "<<camIndex<<endl;
			return false;
		}
		cout<<"Reading from camera "<<camIndex<<endl;
		frame=getFrame(capture);
    }

	if(inputMedia==Unknown){
		cout<<"Option error!"<<endl;
		printUsage(argv[0]);
		return false;
	}
	
	return true;
}




void runLoop(){
	cvNamedWindow("InputFrame", 0);

	bool isPaused=false;
	bool enabled=allocateUserData(frame);
	bool stepMode=false;
	char keyVal='p';
	float milSecs=0;
	
	

	while(keyVal!='q' && enabled){
		if(keyVal=='r'){	//RESET
			resetRequest=true;
			updateNeeded=true;
		}
		if(keyVal=='j'){
			int jump=frameCount+1;
			cout<<"Enter, jump mode @ frame="<<frameCount<<endl;
			cout<<"\tWhat frame to jump to? ";
			cin>>jump;

			while(frameCount < jump){
				frameCount++;
				frame=getFrame(capture);
				if(frame==NULL){
					cout<<"At end of stream, read "<<frameCount<<" frames"<<endl;
					enabled=false;
					break;
				}

				if(frameCount%100==0){
					/*int width = frame->width;
					int height = frame->height;
					input = frame;
					if(input->width > 512 || input->height > 512){
						width = width/2;
						height = height/2;
						input = cvCreateImage( cvSize(width, height), frame->depth, frame->nChannels );
						cvResize(frame, input);
					}*/
					cvShowImage("InputFrame", frame);
					cvWaitKey(2);
				}
				cout<<"frame="<<frameCount<<endl;
			}

			cout<<"Jump complete!"<<endl;
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
					stepMode=false;
					enabled=false;
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
					enabled=false;
					break;
				}
			}
			else{
				cvCopy(orginal, frame);
			}

			if(frameCount < STARTUP_DELAY){
				cout<<"Starting up"<< STARTUP_DELAY-frameCount <<endl;
				frameCount++;
			}
			else{
			
				enabled&=preProcess(frame);
			
				//Time how long it takes to process
				timeval timeTemp;
				timeval timeDelta;
				gettimeofday(&timeTemp, NULL);

				/********* PROCESS IMAGE *******/
				enabled&=processImage(frame);
				/********* END PROCESSING *******/

				gettimeofday(&timeDelta, NULL);
				timeDelta.tv_sec-=timeTemp.tv_sec;
				timeDelta.tv_usec-=timeTemp.tv_usec;
				milSecs=(float)timeDelta.tv_sec*1000 + (float)timeDelta.tv_usec/1000.0;
				
				enabled&=postProcess(frame);
				
				cout<<" - Processed frame in "<<milSecs<<"ms."<<endl;
			}
			

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
		keyVal=cvWaitKey(5);
	}

	printf("Cleaning up\n");

	cvDestroyAllWindows();

	if(inputMedia==Photo){
		cvReleaseImage(&frame);
		cvReleaseImage(&orginal);
	}
	else{
		cvReleaseCapture(&capture);
	}
	
	deallocateUserData();
}

IplImage* getFrame(CvCapture *cvCpt){
    if(!cvGrabFrame(cvCpt)){
        cerr<<"Warning: Could not grab frame"<<endl;
        return NULL;
    }
    return cvRetrieveFrame(cvCpt);
}

void printUsage(char *name){
	cout << "Usage: " << name << " [options]" << endl << endl;
	cout << "\t-c\t[camId]\tRead from camera" << endl;
	cout << "\t-m\t[movie]\tRead video from file" << endl;
	cout << "\t-p\t[photo]\tRead photo from file" << endl;
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

#endif //UTILS_H
