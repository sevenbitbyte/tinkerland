#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <math.h>
#include <iostream>
#include <string>

#define EXIT_SUCCESS 0
#define EXIT_ARG_ERR -1
#define EXIT_FILE_ERR -2
#define EXIT_CAM_ERR -3

using namespace std;

IplImage* getFrame(CvCapture *);
void avgFrame(IplImage* , IplImage*);
void diffFrame(IplImage*, IplImage*, IplImage*);
void printUsage(char*);
void pause(char);

int main(int argc, char **argv){
	string fileName="";
	int camIndex=0;
    char flag;
    char options[]="hf:c::";
    CvCapture* capture=NULL;
	IplImage* avgImg=NULL;
	IplImage* dispImg=NULL;
	IplImage* diffImg=NULL;
	IplImage* accImg=NULL;

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
				cout<<camIndex<<endl;
				break;
			case 'f':
				fileName=optarg;
				//cout<<optarg<<endl;
				break;
			default:
				printUsage(argv[0]);
				exit(EXIT_ARG_ERR);
		}
    }

    if(fileName.size() > 0){
		capture = cvCaptureFromAVI(fileName.c_str());
		if(!capture){
			cerr<<"Error: Could not open file "<<fileName<<endl;
			exit(EXIT_FILE_ERR);
		}
		cout<<"File: "<<fileName<<endl;
    }
    else{
		capture = cvCaptureFromCAM(camIndex);
		if(!capture){
			cerr<<"Error: Could not open camera "<<camIndex<<endl;
			exit(EXIT_CAM_ERR);
		}
		cout<<"Camera["<<camIndex<<"]"<<endl;
    }

    IplImage* frame=NULL;
    char keyVal='p';

    cvNamedWindow("InputFrame", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("AverageFrame", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("DiffedFrame", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("AccumulatorFrame", CV_WINDOW_AUTOSIZE);

	cout<<"Press any key to start"<<endl;


	for(int i=0; i<15; i++){
		frame=getFrame(capture);
		cvShowImage("InputFrame", frame);
		cvShowImage("AverageFrame", avgImg);
	}
	avgImg=cvCloneImage(frame);
	diffImg=cvCloneImage(frame);
	accImg=cvCreateImage( cvSize(frame->width, frame->height), IPL_DEPTH_32F, frame->nChannels);
	cvShowImage("AverageFrame", avgImg);
	cvShowImage("DiffedFrame", diffImg);
	cvShowImage("AccumulatorFrame", accImg);

	keyVal=cvWaitKey(0);

	bool stepMode=false;
    while(keyVal!='q'){

		if(keyVal=='p'){
			pause('p');
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
					cout<<"Exit program"<<endl;
					break;
				}
				else if(keyVal=='n'){	//Next frame
					cout<<"Step"<<endl;
					break;
				}
			}
		}
		else{
			keyVal=cvWaitKey(2);
		}
		//cvAcc(frame, accImg);
		//cvRunningAvg(frame, accImg, 1);
		//cvAddWeighted(frame, 0.1, avgImg, 0.9, 0, avgImg);
		//cvAddWeighted(frame, 1, avgImg, 0, 0, avgImg);
		cvAbsDiff(frame, avgImg, diffImg);
		avgImg=cvCloneImage(frame);
		cvShowImage("InputFrame", frame);
		cvShowImage("AverageFrame", avgImg);
		cvShowImage("DiffedFrame", diffImg);
		cvShowImage("AccumulatorFrame", accImg);
		frame=getFrame(capture);
    }

	cvReleaseImage(&avgImg);
	cvReleaseImage(&diffImg);
	cvReleaseImage(&accImg);
    cvReleaseCapture(&capture);
    cvDestroyAllWindows();
}

void pause(char key){
	char keyVal=0;
	while(keyVal!=key){
		keyVal=cvWaitKey(50);
	}
}

IplImage* getFrame(CvCapture *cvCpt){
    if(!cvGrabFrame(cvCpt)){
        cerr<<"Warning: Could not grab frame"<<endl;
        return NULL;
    }
    return cvRetrieveFrame(cvCpt);
}

void avgFrame(IplImage* in, IplImage* avg){
	if(in->imageSize == avg->imageSize){
		//Uncomment for siezure mode
		/*for(int i=0; i<avg->imageSize; i++){
			avg->imageData[i]+=in->imageData[i];
			avg->imageData[i]=avg->imageData[i]/2;
		}*/


		int chans=avg->nChannels;
		int step=avg->widthStep;
		for(int row=0; row < avg->height; row++){
			for(int col=0; col < avg->width; col++){
				for(int chan=0; chan < avg->nChannels; chan++){
					int val=avg->imageData[row*step+col*chans+chan]*3 + in->imageData[row*step+col*chans+chan];
					avg->imageData[row*step+col*chans+chan] = val/4;
				}
			}
		}
	}
	else{
		cerr<<"Error: Mismatched image size!"<<endl;
	}
}

void diffFrame(IplImage* in, IplImage* avg, IplImage* out){
	if(in->imageSize == avg->imageSize){
		int chans=avg->nChannels;
		int step=avg->widthStep;
		for(int row=0; row < avg->height; row++){
			for(int col=0; col < avg->width; col++){
				for(int chan=0; chan < avg->nChannels; chan++){
					unsigned int val=(unsigned char)avg->imageData[row*step+col*chans+chan] -  (unsigned char)in->imageData[row*step+col*chans+chan];
					if(val > 255){
						val=255;
					}
					if(val < 240 ){
						val=0;
					}
					out->imageData[row*step+col*chans+chan] = val;
				}
			}
		}
	}
	else{
		cerr<<"Error: Mismatched image size!"<<endl;
	}
}

void printUsage(char *name){
	cout << "Usage: " << name << " [options]" << endl << endl;
	cout << "\t-c\t[camId]\tRead from camera" << endl;
	cout << "\t-f\t[file]\tRead from [file]" << endl;
}
