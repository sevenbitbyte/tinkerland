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


#define STARTUP_DELAY 0
#define DELTA_PX 2

using namespace std;

CvPoint roiStart;
bool roiMod=false;
bool roiSlide=false;
bool updateNeeded=true;
bool getNewFrame=true;
bool insertSeedPoints=false;
bool resetSeedPoints=true;
bool showSeedPoints=true;
bool saveData=false;

//Functions
char waitPause();
void printUsage(char*);
IplImage* getFrame(CvCapture *);
void mouseHandler(int event, int x, int y, int flags, void* i);
void forceUpdate(int i);
string cvScalarToString(CvScalar val, int channels);

enum MediaType {Unknown=-1, Video=1, Photo=2, Camera=3};

ofstream dataFile1;
ofstream dataFile2;
ofstream dataFile3;

uint32_t targetHist[256][3];
uint32_t acceptedHist[256][3];
uint32_t rejectedHist[256][3];
uint32_t tempHist[256][3];

int stageRemoved[5]={0,0,0,0,0};
int totalAccepted=0;
int totalChecked=0;

vector<CvPoint*> seedPoints;

void clearHist(uint32_t hist[256][3]){
	for(int i=0; i<3; i++){
		for(int j=0; j<256; j++){
			hist[j][i]=0;
		}
	}
}

void addHist(uint32_t src1[256][3], uint32_t src2[256][3], uint32_t dest[256][3]){
	for(int i=0; i<3; i++){
		for(int j=0; j<256; j++){
			dest[j][i]=src1[j][i]+src2[j][i];
		}
	}
}

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


	IplImage* greyImg=NULL;
	IplImage* edgeImg=NULL;
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

	cvNamedWindow("Input", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("ColorSpace", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Grey", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Lines", CV_WINDOW_AUTOSIZE);
	//cvNamedWindow("Controls", 0);

	cvSetMouseCallback("Input", mouseHandler, (void*)colorSpace);
	cvSetMouseCallback("ColorSpace", mouseHandler, (void*)colorSpace);

	int colorIndex=2;
	int colorChannel=0;
	int lineThresh=20;
	int minLength=30;
	int maxGap=10;
	int cannyThresh1=10;
	int cannyThresh2=200;
	int cannyAperture=1;


	cvCreateTrackbar("Color Space", "Input", &colorIndex, 3, forceUpdate);
	cvCreateTrackbar("Color Channel", "Input", &colorChannel, 3, forceUpdate);
	cvCreateTrackbar("Canny Aperture", "Input", &cannyAperture, 3, forceUpdate);
	cvCreateTrackbar("Canny Threshold1", "Input", &cannyThresh1, 300, forceUpdate);
	cvCreateTrackbar("Canny Threshold2", "Input", &cannyThresh2, 300, forceUpdate);
	cvCreateTrackbar("Line Threshold", "Input", &lineThresh, 300, forceUpdate);
	cvCreateTrackbar("Minimum Line Length", "Input", &minLength, 300, forceUpdate);
	cvCreateTrackbar("Maximum Segment Gap", "Input", &maxGap, 300, forceUpdate);

	//cvResizeWindow("Controls", 400, 300);

	bool isPaused=false;
	bool stepMode=false;
	char keyVal='p';
	float milSecs=0;
	uint32_t frameCount=0;

	while(keyVal!='q'){
		if(keyVal=='o'){	//Output data
			saveData=true;
		}
		if(keyVal=='r'){	//RESET
			while(!seedPoints.empty()){
				delete seedPoints.back();
				seedPoints.pop_back();
			}
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
				/*if(input!=frame && input!=NULL){
					cvReleaseImage(&input);
					input=NULL;
					cout<<"Release"<<endl;
				}*/

				frame=getFrame(capture);
				if(frame==NULL){
					cout<<"At end of stream, read "<<frameCount<<" frames"<<endl;
					exit(1);
				}

				if(frameCount%10==0){
					int width = frame->width;
					int height = frame->height;
					//input = frame;
					/*if(width > 512 || height > 512){
						width = width/2;
						height = height/2;
						if(input==frame || input==NULL){
							cout<<"Alloc"<<endl;
							input = cvCreateImage( cvSize(width, height), frame->depth, frame->nChannels );
						}
						cvResize(frame, input);
					}
					cvShowImage("Input", input);*/
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

					if(input!=frame && input!=NULL){
						cvReleaseImage(&input);
						input=NULL;
					}
					frame=getFrame(capture);
					if(frame==NULL){
						cout<<"At end of stream, read "<<frameCount<<" frames"<<endl;
						break;
						//exit(1);
					}
					getNewFrame=false;
				}
				int width = frame->width;
				int height = frame->height;

				if(frame->width > 512 || frame->height > 512){
					width = width/2;
					height = height/2;
					if(input==frame || input==NULL){
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


				if(greyImg==NULL){
					greyImg=cvCreateImage( cvGetSize(input), IPL_DEPTH_8U, 1 );
				}
				if(edgeImg==NULL){
					edgeImg=cvCreateImage( cvGetSize(input), IPL_DEPTH_8U, 1 );
				}


				if(colorIndex==1){	//Normalized RGB
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
				else if(colorIndex==2){	//HSV
					cvCvtColor(input, colorSpace, CV_RGB2HSV);
				}
				else if(colorIndex==3){	//HLS
					cvCvtColor(input, colorSpace, CV_RGB2HLS);
				}
				else{
					cvCopy(input, colorSpace);
				}

				if(colorChannel>=3){
					cvCvtColor(colorSpace, greyImg, CV_RGB2GRAY);
				}
				else{
					//Copy target color channel into buffer
					IplImage* channels[3]={NULL, NULL, NULL};
					channels[colorChannel]=greyImg;

					cvSplit(colorSpace, channels[0], channels[1], channels[2], NULL);
				}

				CvMemStorage* storage = cvCreateMemStorage(0);
				CvSeq* lines = 0;


				cvCanny( greyImg, edgeImg, cannyThresh1, cannyThresh2, (2*cannyAperture)+1 );

				clearHist(targetHist);
				clearHist(rejectedHist);
				clearHist(acceptedHist);


				lines = cvHoughLines2( edgeImg,
									   storage,
									   CV_HOUGH_PROBABILISTIC,
									   1,
									   CV_PI/180,
									   lineThresh+1,
									   minLength,
									   maxGap );

                                //Delete old points
                                while(!seedPoints.empty()){
                                        delete seedPoints.back();
                                        seedPoints.pop_back();
                                }

                                for(int row=colorSpace->height/2; row<colorSpace->height; row+=10){
                                        for(int col=0; col < colorSpace->width; col+=10){
                                                CvPoint* pt=new CvPoint;
                                                pt->x=col;
                                                pt->y=row;
                                                seedPoints.push_back(pt);
                                        }
                                }

                                PixelStats stats;
                                calcStats(colorSpace, seedPoints, &stats);
                                vector<CvPoint*>::iterator iter=seedPoints.begin();
                                while(iter!=seedPoints.end()){
                                        CvPoint* pt=*iter;
                                        double chan0Delta=fabs(((uint8_t)GetPixelPtrD8(colorSpace, pt->y, pt->x, 0)) - stats.avgVal.val[0]);
                                        double chan1Delta=fabs(((uint8_t)GetPixelPtrD8(colorSpace, pt->y, pt->x, 1)) - stats.avgVal.val[1]);
                                        double chan2Delta=fabs(((uint8_t)GetPixelPtrD8(colorSpace, pt->y, pt->x, 2)) - stats.avgVal.val[2]);
                                        if(chan0Delta > stats.stdDev.val[0]*1 /*||
                                                chan1Delta > stats.stdDev.val[1]*1.5 ||
                                                chan2Delta > stats.stdDev.val[2]*1.5*/){
                                                delete (*iter);
                                                iter=seedPoints.erase(iter);
                                                continue;
                                        }
                                        iter++;
                                }

                                //PixelStats stats;
				calcStats(colorSpace, seedPoints, &stats);

				int removed=0;
				int total=lines->total;

				totalChecked+=total;

				for(int i=0; i<lines->total; i++){
					CvPoint* line = (CvPoint*)cvGetSeqElem(lines,i);

					float slope=(float)(line[0].y - line[1].y)/(float)(line[0].x - line[1].x);

					/*if(line[0].y < 50 || line[1].y<50){
						removed++;
						continue;
					}

					if(fabsf(slope-1) < 0.1){
						removed++;
						continue;
					}*/

					CvLineIterator iterator;
					int count=cvInitLineIterator( colorSpace, line[0], line[1], &iterator, 8, 0 );

					CvScalar avgVal=cvScalarAll(0);
					CvScalar delta=cvScalarAll(0);
					CvScalar variance=cvScalarAll(0);
					CvScalar stdDev=cvScalarAll(0);

					clearHist(tempHist);

					for(int p=0; p<count; p++){	//Loop over pixels in line
						tempHist[iterator.ptr[0]][0]++;
						tempHist[iterator.ptr[1]][1]++;
						tempHist[iterator.ptr[2]][2]++;

						avgVal.val[0]+=iterator.ptr[0];
						avgVal.val[1]+=iterator.ptr[1];
						avgVal.val[2]+=iterator.ptr[2];
						CV_NEXT_LINE_POINT(iterator);
					}

					avgVal.val[0]=avgVal.val[0]/count;
					avgVal.val[1]=avgVal.val[1]/count;
					avgVal.val[2]=avgVal.val[2]/count;

					count=cvInitLineIterator( colorSpace, line[0], line[1], &iterator, 8, 0 );

					for(int p=0; p<count; p++){	//Loop over pixels in line
						variance.val[0]+=(iterator.ptr[0]-avgVal.val[0])*(iterator.ptr[0]-avgVal.val[0]);
						variance.val[1]+=(iterator.ptr[1]-avgVal.val[1])*(iterator.ptr[1]-avgVal.val[1]);
						variance.val[2]+=(iterator.ptr[2]-avgVal.val[2])*(iterator.ptr[2]-avgVal.val[2]);
						CV_NEXT_LINE_POINT(iterator);
					}

					variance.val[0]/=count;
					variance.val[1]/=count;
					variance.val[2]/=count;

					delta.val[0]=fabs(avgVal.val[0]-stats.avgVal.val[0]);
					delta.val[1]=fabs(avgVal.val[1]-stats.avgVal.val[1]);
					delta.val[2]=fabs(avgVal.val[2]-stats.avgVal.val[2]);

					stdDev.val[0]=sqrt(variance.val[0]);
					stdDev.val[1]=sqrt(variance.val[1]);
					stdDev.val[2]=sqrt(variance.val[2]);

					//cout<<"line m="<<slope<<" stdDev="<<cvScalarToString(stdDev, 3)<<", avg="<<cvScalarToString(avgVal, 3);


					if(delta.val[0] < 10*stats.stdDev.val[0] &&
					   delta.val[1] < 2*stats.stdDev.val[1] &&
					   delta.val[2] < 4*stats.stdDev.val[2]){

						cout<<" REMOVED deltaAvg="<<cvScalarToString(delta,3)<<" stdDev="<<cvScalarToString(stdDev,3)<<endl;

						removed++;
						cvLine( input, line[0], line[1], CV_RGB(255,0,0), 1);
						stageRemoved[0]++;

						addHist(tempHist, rejectedHist, rejectedHist);
						continue;
					}

					//Dark grass Checking for HSV
					if(avgVal.val[0] > stats.avgVal.val[0]){
						cout<<" REMOVED chan0 AVG deltaAvg="<<cvScalarToString(delta,3)<<" stdDev="<<cvScalarToString(stdDev,3)<<endl;
						removed++;
						cvLine( input, line[0], line[1], CV_RGB(255,255,0), 1);
						stageRemoved[1]++;
						addHist(tempHist, rejectedHist, rejectedHist);
						continue;
					}
					else if(avgVal.val[1] > stats.avgVal.val[1]){
						cout<<" REMOVED chan1 AVG deltaAvg="<<cvScalarToString(delta,3)<<" stdDev="<<cvScalarToString(stdDev,3)<<endl;
						removed++;
						cvLine( input, line[0], line[1], CV_RGB(0,127,127), 1);
						stageRemoved[2]++;
						addHist(tempHist, rejectedHist, rejectedHist);
						continue;
					}
					else if(avgVal.val[2] > 200){
						//cout<<" REMOVED chan2 AVG deltaAvg="<<cvScalarToString(delta,3)<<" stdDev="<<cvScalarToString(stdDev,3)<<endl;
						//removed++;
						//cvLine( input, line[0], line[1], CV_RGB(255,157,0), 1);
						//continue;
					}

					if(15*stats.stdDev.val[0] < stdDev.val[0]){
						cout<<" REMOVED hue deltaAvg="<<cvScalarToString(delta,3)<<" stdDev="<<cvScalarToString(stdDev,3)<<endl;
						removed++;
						cvLine( input, line[0], line[1], CV_RGB(255,83,83), 1);
						stageRemoved[3]++;
						addHist(tempHist, rejectedHist, rejectedHist);
						continue;
					}

					cout<<"Keeping deltaAvg="<<cvScalarToString(delta,3)<<" stdDev="<<cvScalarToString(stdDev,3)<<endl;
					cvLine( input, line[0], line[1], CV_RGB(0, 255, 0), 1);
					addHist(tempHist, acceptedHist, acceptedHist);
				}

				if(saveData==true){
					dataFile1.open("targetHist.dat", ios_base::trunc);
					dataFile2.open("acceptedHist.dat", ios_base::trunc);
					dataFile3.open("rejectedHist.dat", ios_base::trunc);

					cout<<"Writing Data to file..."<<endl;

					for(int j=0; j<256; j++){
						dataFile1<<targetHist[j][0]<<" "<<targetHist[j][1]<<" "<<targetHist[j][2]<<endl;
						dataFile2<<acceptedHist[j][0]<<" "<<acceptedHist[j][1]<<" "<<acceptedHist[j][2]<<endl;
						dataFile3<<rejectedHist[j][0]<<" "<<rejectedHist[j][1]<<" "<<rejectedHist[j][2]<<endl;
					}

					cout<<"Writing complete"<<endl;

					dataFile1.close();
					dataFile2.close();
					dataFile3.close();

					saveData=false;
					isPaused=true;
				}

				totalAccepted+=total-removed;

				cvReleaseMemStorage(&storage);

				/********* END PROCESSING *******/

				gettimeofday(&timeDelta, NULL);
				timeDelta.tv_sec-=timeTemp.tv_sec;
				timeDelta.tv_usec-=timeTemp.tv_usec;
				milSecs=((float)timeDelta.tv_sec)*1000 + ((float)timeDelta.tv_usec)/1000;

				for(int i=0; i<seedPoints.size(); i++){
						CvPoint* pt=seedPoints.at(i);
						cvCircle(input, *pt, 2, cvScalar(0,0,255), 1);
				}

				cout<<"\tAvg="<<cvScalarToString(stats.avgVal,3)<<endl;
				cout<<"\tVariance="<<cvScalarToString(stats.variance,3)<<endl;
				cout<<"\tStdDev="<<cvScalarToString(stats.stdDev,3)<<endl;
				cout<<"Found "<<total<<" lines and removed "<<removed<<" lines";
				cout<<" - Processed frame in "<<milSecs<<"ms."<<endl;
			}

			cvShowImage("Input", input);
			cvShowImage("ColorSpace", colorSpace);
			cvShowImage("Grey", greyImg);
			cvShowImage("Lines", edgeImg);
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

	cout<<"Total Lines Checked: "<<totalChecked<<endl;
	cout<<"Total Accepted: "<<totalAccepted<<endl;
	cout<<"Stage removed counts"<<endl;
	for(int i=0; i<4; i++){
		cout<<"\tstage"<<i<<": "<<stageRemoved[i]<<endl;
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


	if(colorSpace!=NULL){
		cvReleaseImage(&colorSpace);
	}

	if(greyImg!=NULL){
		cvReleaseImage(&greyImg);
	}

	if(edgeImg!=NULL){
		cvReleaseImage(&edgeImg);
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
			while(!seedPoints.empty()){
				delete seedPoints.back();
				seedPoints.pop_back();
			}
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
