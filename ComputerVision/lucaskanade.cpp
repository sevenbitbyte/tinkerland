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

#include "lkt.h"


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

double quality = 1.0f;
double minDistance = 5.0f;

char waitPause();
void printUsage(char*);
IplImage* getFrame(CvCapture *);


void mouseHandler(int event, int x, int y, int flags, void* i);
void qualityUpdate(int i);
void minDistanceUpdate(int i);

enum MediaType {Unknown=-1, Video=1, Photo=2, Camera=3};



int main(int argc, char** argv){
	string fileName="";
	int camIndex=-1;
    char flag;
    char options[]="hp:m:c::";
	char inputMedia=Unknown;
    CvCapture* capture=NULL;
	IplImage* orginal=NULL;
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
		orginal=cvLoadImage(fileName.c_str());
		if(orginal == NULL){
			cout<<"Error: Could not load photo "<<fileName<<endl;
			exit(-1);
		}
		frame=cvCreateImage( cvGetSize(orginal), orginal->depth, orginal->nChannels );
		cvCopy(orginal, frame);
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










	IplImage* input = frame;
	LKImage* previous=NULL;
	LKImage* current=NULL;
	IplImage* prevImg=NULL;
	IplImage* eigImage = NULL;
	IplImage* tempImage = NULL;

	cvNamedWindow("InputFrame1", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("InputFrame2", 0);
	cvNamedWindow("Controls", CV_WINDOW_AUTOSIZE);

	/*cvNamedWindow("GrayFrame1", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("GrayFrame2", CV_WINDOW_AUTOSIZE);*/
	cvNamedWindow("EdgeX2", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("EdgeY2", CV_WINDOW_AUTOSIZE);

	int qualityInt = 5;
	int minDistInt = 5;
	cvCreateTrackbar("Quality", "Controls", &qualityInt, 100, qualityUpdate);
	cvCreateTrackbar("Min Distance", "Controls", &minDistInt, 100, minDistanceUpdate);

	qualityUpdate(cvGetTrackbarPos("Quality", "Controls"));
	minDistanceUpdate(cvGetTrackbarPos("Min Distance", "Controls"));

	bool isPaused=false;
	bool stepMode=false;
	char keyVal='p';
	float milSecs=0;
	uint32_t frameCount=0;

	while(keyVal!='q'){
		if(keyVal=='r'){	//RESET
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
					exit(1);
				}

				if(frameCount%100==0){
					int width = frame->width;
					int height = frame->height;
					input = frame;
					if(input->width > 512 || input->height > 512){
						width = width/2;
						height = height/2;
						input = cvCreateImage( cvSize(width, height), frame->depth, frame->nChannels );
						cvResize(frame, input);
					}
					cvShowImage("InputFrame1", input);
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
				/*for(int i=0; i<5; i++){
					frameCount++;
					getFrame(capture);
				}*/
			if(inputMedia != Photo){
				frameCount++;
				frame=getFrame(capture);
				int width = frame->width;
				int height = frame->height;
				input = frame;
				if(input->width > 512 || input->height > 512){
					width = width/2;
					height = height/2;
					input = cvCreateImage( cvSize(width, height), frame->depth, frame->nChannels );
					cvResize(frame, input);
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
				//Time how long it takes to process
				timeval timeTemp;
				timeval timeDelta;
				gettimeofday(&timeTemp, NULL);

				if(eigImage==NULL){
					eigImage=cvCreateImage( cvGetSize(input), IPL_DEPTH_32F, 1 );
				}
				if(tempImage==NULL){
					tempImage=cvCreateImage( cvGetSize(input), IPL_DEPTH_32F, 1 );
				}

				/********* PROCESS IMAGE *******/
				if(previous!=NULL){
					delete previous;
				}

				previous=current;
				current=new LKImage(input, true);

				if(previous!=NULL){
					vector<LKFeature*> features;
					CvPoint2D32f corners[200];
					int cornerCount=0;

					//cvGoodFeaturesToTrack(current->gray, eigImage, tempImage, corners,
									  //&cornerCount, quality, minDistance, NULL, 3, 1);
									  
					for(int r=input->height-10; r>70; r--){
						for(int c=input->width-20; c>10; c--){
							if(r%30==0 && c%30==0){
								CvPoint p = cvPoint( c, r );
								LKFeature* feature=new LKFeature;
								feature->estimate=p;
								feature->points.push_back(p);
								features.push_back(feature);
								cornerCount++;
							}
						}
					}

					

					/*for(int i=0; i<cornerCount; i++){
						CvPoint p = cvPoint( (int)corners[i].x, (int)corners[i].y );
						LKFeature* feature=new LKFeature;
						feature->estimate=p;
						//feature->estimate.x+=2;
						//feature->estimate.y+=2;
						feature->points.push_back(p);
						features.push_back(feature);



						//cvLine(prevImg, cvPoint(p.x-2, p.y), cvPoint(p.x+2, p.y), cvScalar(0,255,0), 1);
						//cvLine(prevImg, cvPoint(p.x, p.y-2), cvPoint(p.x, p.y+2), cvScalar(0,255,0), 1);
					}*/

					calculateLKFlow(previous, current, features, 25);

					cout<<"Found "<<cornerCount<<" corners ";
					/*cvShowImage("EdgeX", current->edgeX);
					cvShowImage("EdgeY", current->edgeY);*/

					CvPoint avgDelta=cvPoint(0,0);
					int matchCount=0;
					for(int i=0; i<features.size(); i++){
						LKFeature* feature=features.at(i);

						if(feature->points.size()==2){
							CvPoint pt1=feature->points.front();
							CvPoint pt2=feature->points.back();
							avgDelta.x+=pt1.x-pt2.x;
							avgDelta.y+=pt1.y-pt2.y;
							matchCount++;		
							//cout<<"Found match at ("<<pt1.x<<", "<<pt1.y<<") -> ";
							//cout<<"("<<pt2.x<<", "<<pt2.y<<")"<<endl;

							
							
							cvLine(prevImg, cvPoint(pt1.x-2, pt1.y), cvPoint(pt1.x+2, pt1.y), cvScalar(0,255,0), 1);
							cvLine(prevImg, cvPoint(pt1.x, pt1.y-2), cvPoint(pt1.x, pt1.y+2), cvScalar(0,255,0), 1);
							
							cvLine(prevImg, pt1, pt2, cvScalar(0,0,255), 1);
							delete feature;
						}
						else{
							//cout<<"No match for feature index="<<i<<endl;
						}
					}
					
					if(matchCount>0){
						CvPoint center=cvPoint(prevImg->width/2, prevImg->height/2);
						avgDelta.x=(avgDelta.x/matchCount)*2 + center.x;
						avgDelta.y=(avgDelta.y/matchCount)*2 + center.y;
						
						cvLine(prevImg, center, avgDelta, cvScalar(255,0,0), 1);
					}
				}

				/********* END PROCESSING *******/

				gettimeofday(&timeDelta, NULL);
				timeDelta.tv_sec-=timeTemp.tv_sec;
				timeDelta.tv_usec-=timeTemp.tv_usec;
				milSecs=timeDelta.tv_sec*1000 + timeDelta.tv_usec/1000;
				
				cout<<" - Processed frame in "<<milSecs<<"ms."<<endl;
			}

			cvShowImage("InputFrame1", input);
			cvShowImage("InputFrame2", prevImg);
			if(prevImg!=NULL){
				cvReleaseImage(&prevImg);
			}
			/*if(previous!=NULL){
				cout<<"Showing gray2"<<endl;
				cvShowImage("GrayFrame2", previous->gray);
			}
			if(current!=NULL){
				cout<<"Showing gray1"<<endl;
				cvShowImage("GrayFrame1", current->gray);
			}*/

			prevImg=cvCloneImage(input);
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
	if(eigImage!=NULL){
		cvReleaseImage(&eigImage);
	}
	if(tempImage!=NULL){
		cvReleaseImage(&tempImage);
	}
	if(prevImg!=NULL){
		cvReleaseImage(&prevImg);
	}

	if(previous!=NULL){
		delete previous;
	}

	if(current!=NULL){
		delete current;
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

	}
	else if(event==CV_EVENT_RBUTTONDOWN){

	}
}

IplImage* getFrame(CvCapture *cvCpt){
    if(!cvGrabFrame(cvCpt)){
        cerr<<"Warning: Could not grab frame"<<endl;
        return NULL;
    }
    return cvRetrieveFrame(cvCpt);
}

void qualityUpdate(int i){
	quality = ((double) i)/100;
	cout<<"Quality = "<<quality<<endl;
}

void minDistanceUpdate(int i){
	minDistance=i;
	cout<<"MinDistance = "<<minDistance<<endl;
}


void printUsage(char *name){
	cout << "Usage: " << name << " [options]" << endl << endl;
	cout << "\t-c\t[camId]\tRead from camera" << endl;
	cout << "\t-m\t[movie]\tRead video from file" << endl;
	cout << "\t-p\t[photo]\tRead photo from file" << endl;
}
