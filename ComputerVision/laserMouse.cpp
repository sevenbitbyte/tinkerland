/**
 *	@author	Alan Meekins
 *	@date	9/8/09
**/

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <math.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/time.h>
#include <list>
#include <vector>
#include <stdio.h>

#define COLOR_SPACE 256
#define MAX_COUNT 255
#define STEP_SIZE 1
#define INC_VAL 1
#define S_MATRIX true

#define GetPixelPtrD8(i,r,c,chan) i->imageData[((r*i->widthStep)+(c*i->nChannels)+chan)]
#define GetPixelPtrD16(i,r,c,chan) i->imageData[((r*(i->widthStep/sizeof(uint16_t)))+(c*i->nChannels)+chan)]
#define GetPixelPtrD32(i,r,c,chan) ((float*)i->imageData)[((r*768)+(c*i->nChannels)+chan)]

#define STARTUP_DELAY 5

#define LASER_R 80
#define LASER_G 90
#define LASER_B 100

#define LASER_STD_R 50
#define LASER_STD_G 15
#define LASER_STD_B 30

using namespace std;

uint32_t minRed=255;
uint32_t minGreen=255;
uint32_t minBlue=255;

uint32_t maxRed=0;
uint32_t maxGreen=0;
uint32_t maxBlue=0;

float avgRed=-1;
float avgGreen=-1;
float avgBlue=-1;

CvPoint roiStart;
bool roiMod=false;
bool roiSlide=false;
bool updateNeeded=true;
//unsigned char drawState=0;

char waitPause();
void printUsage(char*);
IplImage* getFrame(CvCapture *);

list<CvPoint> getBrightPixels(IplImage* img, int pixels, int threshold);
CvPoint getCentriod(list<CvPoint> points);
void drawGesture(IplImage* img, list<CvPoint> points);

//void roiHandler(int event, int x, int y, int flags, void* i);


enum MediaType {Unknown=-1, Video=1, Photo=2, Camera=3};

int main(int argc, char** argv){
	string fileName="";
	int camIndex=-1;
    char flag;
    char options[]="hp:m:c::";
	char inputMedia=Unknown;
    CvCapture* capture=NULL;
	IplImage* frame=NULL;
	IplImage* greyFrame=NULL;
	IplImage* motionFrame=NULL;
	IplImage* lastFrame=NULL;
	IplImage* gestureImage=NULL;

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
		gestureImage=cvCreateImage(cvSize(frame->width, frame->height), IPL_DEPTH_8U, 3);
    }

	if(inputMedia==Photo){
		frame=cvLoadImage(fileName.c_str());
		if(frame == NULL){
			cout<<"Error: Could not load photo "<<fileName<<endl;
			exit(-1);
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
		gestureImage=cvCreateImage(cvSize(frame->width, frame->height), IPL_DEPTH_8U, 3);
    }

	if(inputMedia==Unknown){
		cout<<"Option error!"<<endl;
		printUsage(argv[0]);
		exit(-1);
	}

	cvNamedWindow("InputFrame", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("ProcessedFrame", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("CapturedGesture", CV_WINDOW_AUTOSIZE);

	bool isPaused=false;
	bool stepMode=false;
	char keyVal='p';
	float milSecs=0;
	CvPoint pointer=cvPoint(100,100);
	uint32_t frameCount=0;
	list<CvPoint> gesture;
	uint32_t gestureFrames=0;
	bool laserFound=false;
	uint32_t frameGap=0;

	while(keyVal!='q'){
		if(keyVal=='r'){	//RESET
			//cvResetImageROI(grey1);
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
					stepMode=false;
					cout<<"Exit program"<<endl;
					break;
				}
				else if(keyVal=='n'){	//Next frame
					cout<<"Step"<<endl;
					updateNeeded=true;
					break;
				}
			}
		}

		if(updateNeeded || !isPaused){
			if(inputMedia != Photo){
				frame=getFrame(capture);
				//cvCvtColor(frame, frame, CV_RGB2HLS);
			}

			if(lastFrame==NULL){
				lastFrame=cvCloneImage(frame);
			}

			if(motionFrame==NULL){
				motionFrame=cvCloneImage(frame);
			}

			cvAbsDiff(frame, lastFrame, motionFrame);
			if(lastFrame!=NULL){
				cvReleaseImage(&lastFrame);
			}
			lastFrame=cvCloneImage(frame);

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

				list<CvPoint> points = getBrightPixels(motionFrame, 5, 80);

				if(points.size() < 5){
					pointer=cvPoint(0,0);
					frameGap++;
					if(frameGap >= 5 && laserFound){
						drawGesture(gestureImage, gesture);
						laserFound=false;
						frameGap=0;
						cout<<"Gesture found with "<<gesture.size();
						cout<<" points in "<<gestureFrames<<" frames."<<endl;
						gesture.clear();
						gestureFrames=0;
						//isPaused=true;
					}
				}
				else{
					pointer = getCentriod(points);
					list<CvPoint>::iterator iter=points.begin();
					for(iter; iter!=points.end(); iter++){
						gesture.push_back(*iter);
					}
					laserFound=true;
					frameGap=0;
					gestureFrames++;
					drawGesture(gestureImage, gesture);
				}

				//cout<<"Found "<<points.size()<< " pixels centered at (" << pointer.x<<", " << pointer.y<<")";

				gettimeofday(&timeDelta, NULL);
				timeDelta.tv_sec-=timeTemp.tv_sec;
				timeDelta.tv_usec-=timeTemp.tv_usec;
				milSecs=timeDelta.tv_sec*1000 + timeDelta.tv_usec/1000;
				//cout<<" - Processed frame in "<<milSecs<<"ms."<<endl;

				cvLine(frame, cvPoint(pointer.x, 0), cvPoint(pointer.x, frame->height), cvScalar(0,255,0), 1);
				cvLine(frame, cvPoint(0, pointer.y), cvPoint(frame->width, pointer.y), cvScalar(0,255,0), 1);
			}

			cvShowImage("InputFrame", frame);
			cvShowImage("ProcessedFrame", motionFrame);
			cvShowImage("CapturedGesture", gestureImage);
			updateNeeded=false;
		}

		if(keyVal=='p'){
			isPaused=!isPaused;
			if(isPaused){
				frameCount=0;
			}
		}
		if(keyVal=='q'){
			break;
		}
		keyVal=cvWaitKey(10);
	}

	cout<<"Red "<<minRed << " - " << maxRed << " Avg = " << avgRed << endl;
	cout<<"Green "<<minGreen << " - " << maxGreen << " Avg = " << avgGreen << endl;
	cout<<"Blue "<<minBlue << " - " << maxBlue << " Avg = " << avgBlue << endl;

	printf("Cleaning up\n");

	cvDestroyAllWindows();

	if(lastFrame!=NULL){
		cvReleaseImage(&lastFrame);
	}
	if(motionFrame!=NULL){
		cvReleaseImage(&motionFrame);
	}

	if(inputMedia==Photo){
		cvReleaseImage(&frame);
	}
	else{
		cvReleaseCapture(&capture);
	}

	if(gestureImage != NULL){
		cvReleaseImage(&gestureImage);
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


list<CvPoint> getBrightPixels(IplImage* img, int pixels, int threshold){
	int height=img->height;
	int width=img->width;
	int rowStart=0;
	int rowEnd=height;
	int colStart=0;
	int colEnd=width;
	CvRect roi=cvGetImageROI(img);
	list<CvPoint> brightPoints;

	if(!(roi.x==0 && roi.y==0 && roi.width==width && roi.height==height)){
		rowStart=(int) fmax(fmin(roi.y, height-1), 0);
		rowEnd=(int) fmax(fmin(rowStart+roi.height, height), rowStart+1);
		colStart=(int) fmax(fmin(roi.x, width-1), 0);
		colEnd=(int) fmax(fmin(colStart+roi.width, width), colStart+1);
		cout<<"Using ROI"<<endl;
	}
	uint8_t value =(uint8_t) GetPixelPtrD8(img, rowStart, colStart, 1);



	for(int row=rowStart; row<rowEnd; row++){
		for(int col=colStart; col<colEnd; col++){

			uint8_t red=(uint8_t) GetPixelPtrD8(img, row, col, 0);
			uint8_t green=(uint8_t) GetPixelPtrD8(img, row, col, 1);
			uint8_t blue=(uint8_t) GetPixelPtrD8(img, row, col, 2);


			if(green < LASER_G){
				continue;
			}

			/*if(red > LASER_R+LASER_STD_R || red < LASER_R - LASER_STD_R){
				continue;
			}

			if(green > LASER_G+LASER_STD_G || green < LASER_G - LASER_STD_G){
				continue;
			}


			if(blue > LASER_B+LASER_STD_B || blue < LASER_B - LASER_STD_B){
				continue;
			}*/

			brightPoints.push_front(cvPoint(col, row));

			if(red < minRed){minRed = red;}
			if(green < minGreen){minGreen = green;}
			if(blue < minBlue){minBlue = blue;}

			if(red > maxRed){maxRed = red;}
			if(green > maxGreen){maxGreen = green;}
			if(blue > maxBlue){maxBlue = blue;}

			if(avgRed == -1){avgRed=red;}
			if(avgGreen == -1){avgGreen = green;}
			if(avgBlue == -1){avgBlue = blue;}

			avgRed=(avgRed+(float)red)/2;
			avgGreen=(avgGreen+(float)green)/2;
			avgBlue=(avgBlue+(float)blue)/2;

			/*value=(uint8_t) GetPixelPtrD8(img, row, col, 1);

			if(value < threshold){
				continue;
			}

			cout<<(uint8_t) GetPixelPtrD8(img, row, col, 0) <<" ";
			cout<<(uint8_t) GetPixelPtrD8(img, row, col, 1) <<" ";
			cout<<(uint8_t) GetPixelPtrD8(img, row, col, 2) << endl;

			if(brightPoints.size() > 0){
				CvPoint highPoint=brightPoints.front();
				CvPoint lowPoint=brightPoints.back();



				uint8_t high=GetPixelPtrD8(img, highPoint.y, highPoint.x, 1);
				uint8_t low=GetPixelPtrD8(img, lowPoint.y, lowPoint.x, 1);

				//cout<<"value="<<(int) value <<" lowest="<<(int) low <<" highest="<<(int)high<<endl;

				if(value > high){
					brightPoints.push_front(cvPoint(col, row));
					//cout<<"highest"<<endl;
				}
				else if(value > low){
					list<CvPoint>::iterator iter=brightPoints.begin();
					for(iter; iter!=brightPoints.end(); iter++){
						CvPoint p=*iter;
						uint8_t pointVal=GetPixelPtrD8(img, p.y, p.x, 1);
						if(pointVal < value){
							iter++;
							if(iter==brightPoints.end()){
								brightPoints.push_back(cvPoint(col, row));
								//cout<<"lowest"<<endl;
							}
							else{
								brightPoints.insert(iter, cvPoint(col, row));
								//cout<<"inserted"<<endl;
							}
							break;
						}
					}
				}
			}
			else{
				//cout<<"First point"<<endl;
				brightPoints.push_back(cvPoint(col, row));
			}

			if(brightPoints.size() > pixels){
				brightPoints.pop_back();
			}*/
		}
	}

	return brightPoints;
}

CvPoint getCentriod(list<CvPoint> points){
	/*CvPoint leftMost=points.front();
	CvPoint rightMost=points.front();
	CvPoint topMost*/

	CvPoint2D32f stdDev=cvPoint2D32f(0,0);
	CvPoint centriod=cvPoint(0,0);
	list<CvPoint>::iterator iter=points.begin();
	for(iter; iter!=points.end(); iter++){
		centriod.x += iter->x;
		centriod.y += iter->y;
	}

	if(points.size() > 0){
		centriod.x=centriod.x/points.size();
		centriod.y=centriod.y/points.size();
	}

	for(iter=points.begin(); iter!=points.end(); iter++){
		float xDiff=iter->x - centriod.x;
		xDiff=powf(xDiff, 2);
		float yDiff=iter->y - centriod.y;
		yDiff=powf(yDiff, 2);

		stdDev.x+=xDiff;
		stdDev.y+=yDiff;
	}

	stdDev.x=sqrtf(stdDev.x);
	stdDev.y=sqrtf(stdDev.y);


	return centriod;
}

void drawGesture(IplImage* img, list<CvPoint> points){
	cvSetZero(img);
	cout<<"Drawing gesture with "<<points.size()<<"points"<<endl;
	list<CvPoint>::iterator iter=points.begin();
	for(iter; iter!=points.end(); iter++){
		GetPixelPtrD8(img, iter->y, iter->x, 0) = (uint8_t) 255;
		GetPixelPtrD8(img, iter->y, iter->x, 1) = (uint8_t) 255;
		GetPixelPtrD8(img, iter->y, iter->x, 2) = (uint8_t) 255;
	}
}
