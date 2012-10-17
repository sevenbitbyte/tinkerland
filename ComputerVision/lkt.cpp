#include "lkt.h"

#include "../Common/timer.h"
#include <math.h>

#define DEBUG_LKT_ERROR false
#define SIMPLE_LKT_ERROR false

using namespace std;

/*class LKImage{
	IplImage* gray;
	IplImage* edgeX;
	IplImage* edgeY;
	IplImage* edgeX2;	//Squared edges in X direction
	IplImage* edgeY2;	//Squared edges in Y direction*/

LKImage::LKImage(){
	gray=NULL;
	edgeX=NULL;
	edgeY=NULL;
	edgeX2=NULL;
	edgeY2=NULL;
}

LKImage::LKImage(IplImage* input, bool calcSquared){
	cout<<"LKImage::LKImage - ";
		gray=NULL;
		edgeX=NULL;
		edgeY=NULL;
		edgeX2=NULL;
		edgeY2=NULL;
	if(input==NULL){
		cout<<"Bail!"<<endl;
		return;
	}


	gray=cvCreateImage( cvGetSize(input), IPL_DEPTH_8U, 1 );
	edgeX=cvCreateImage( cvGetSize(input), IPL_DEPTH_32F, 1 );
	edgeY=cvCreateImage( cvGetSize(input), IPL_DEPTH_32F, 1 );

	cvCvtColor(input, gray, CV_RGB2GRAY);
	IplImage* grayTemp = cvCreateImage( cvGetSize(input), IPL_DEPTH_32F, 1 );
	cvConvertScale(gray, grayTemp, 1/255.);
	cvReleaseImage(&gray);
	gray=grayTemp;

	//cvConvertScale(gray, edgeX, 1/255.);
	//cvConvertScale(gray, edgeY, 1/255.);

	cvSobel( gray, edgeX, 1, 0, 1);
	cvSobel( gray, edgeY, 0, 1, 1);





	if(calcSquared){
		edgeX2=cvCreateImage( cvGetSize(input), IPL_DEPTH_32F, 1 );
		edgeY2=cvCreateImage( cvGetSize(input), IPL_DEPTH_32F, 1 );

		cvMul(edgeX, edgeX, edgeX2);
		cvMul(edgeY, edgeY, edgeY2);
	}
	else{
		edgeX2=NULL;
		edgeY2=NULL;
	}

	cout<<"Constructed"<<endl;
}

LKImage::~LKImage(){
	cout<<"LKImage::~LKImage - DEBUG"<<endl;
	if(gray!=NULL){
		cvReleaseImage(&gray);
	}

	if(edgeX!=NULL){
		cvReleaseImage(&edgeX);
	}

	if(edgeY!=NULL){
		cvReleaseImage(&edgeY);
	}

	if(edgeX2!=NULL){
		cvReleaseImage(&edgeX2);
	}

	if(edgeY2!=NULL){
		cvReleaseImage(&edgeY2);
	}
}

void LKImage::updateEdge2(){
	if(edgeX!=NULL && edgeY!=NULL){
		if(edgeX2==NULL){
			edgeX2=cvCreateImage( cvGetSize(gray), IPL_DEPTH_32F, 1 );
		}

		if(edgeY2==NULL){
			edgeY2=cvCreateImage( cvGetSize(gray), IPL_DEPTH_32F, 1 );
		}

		cvMul(edgeX, edgeX, edgeX2);
		cvMul(edgeY, edgeY, edgeY2);
	}
}

bool LKImage::hasEdge2(){
	return edgeX!=NULL && edgeY!=NULL;
}






void calculateLKFlow(LKImage* initial, LKImage* final, vector<LKFeature*> features, int size){
	if((size%2)!=1){
		cerr<<"calculateLKFlow - ERROR, size must be odd!"<<endl;
		return;
	}

	if(features.size() < 1){
		cerr<<"calculateLKFlow - WARNING, passed feature vector containing zero features!"<<endl;
		return;
	}

	Timer timer(true);
	LKImage weight;
	calculateLKWeight(final, initial, &weight);
	timer.stop();
	cout<<"calculateLKWeight - Times ";
	timer.printLine();


	LKState state;
	state.g=final;
	state.f=initial;
	state.w=&weight;
	state.window=size;
	
	cvShowImage("EdgeX2", state.f->edgeX2);
	cvShowImage("EdgeY2", state.f->edgeY2);
	
	for(int i=0; i<features.size(); i++){	//For all features
		LKFeature* feature=features.at(i);
		state.start=feature->points.back();
		state.current=feature->estimate;
		state.h=cvPoint2D32f(0.0f, 0.0f);

		cout<<endl<<"Feature: "<<i<<endl;
		cout<<"Start: "<<state.start.x<<", "<<state.start.y<<endl;
		cout<<"Current: "<<state.current.x<<", "<<state.current.y<<endl;
		for(int j=0; j<MAX_ITERATIONS; j++){
			if(state.current.x < 0 || state.current.x > final->gray->width-1 || state.current.y < 0 || state.current.y > final->gray->height-1){
				cout<<"\tBAIL"<<j<<endl;
				break;
			}
			cout<<"\tcalculateLKWindowError("<<j;
			calculateLKWindowError(state);			
			
			state.current.x=state.start.x+(int)rintf(state.h.x);
			state.current.y=state.start.y+(int)rintf(state.h.y);

			cout<<") Current: "<<state.h.x<<", "<<state.h.y<<endl;
			
			if(state.h.x==0.0f && state.h.y==0.0f || fabsf(state.h.x)<0.2f && fabsf(state.h.y)<0.2f){
				cout<<"\tGot matching point"<<endl;
				break;
			}
		}
		feature->points.push_back(state.current);
		cout<<"DONE, Current: "<<state.current.x<<", "<<state.current.y<<endl;
		cout<<endl;
	}
}

/*
struct LKState{
	int window;
	LKImage* f;
	LKImage* g;
	LKImage* w;
	CvPoint start;
	CvPoint current;
	CvPoint2D32f errorSum;
}

*/

void calculateLKWindowError(LKState &state){

	float temp1x=0.0f;
	float temp2x=0.0f;
	float temp1y=0.0f;
	float temp2y=0.0f;
	state.error=0.0f;

	#if SIMPLE_LKT_ERROR
	temp2x=1;
	temp2y=1;
	#endif

	for(int j=-state.window/2; j<=(state.window/2); j++){
		for(int k=-state.window/2; k<=(state.window/2); k++){
			int currentRow=j+state.current.y+(int)rintf(state.h.y);
			int currentCol=k+state.current.x+(int)rintf(state.h.x);
			
			if(currentCol<0 || currentRow<0 || currentCol>state.f->gray->width-1 || currentRow>state.f->gray->height-1){
				break;
			}
			
			float w=GetPixel32f(state.w->edgeX, j+state.start.y, k+state.start.x, 0);
			if(isnan(w)!=0 || isinf(w)!=0){
				cout<<"w=NAN"<<endl;
				break;
			}

			float f=GetPixel32f(state.f->gray, currentRow, currentCol, 0);
			float g=GetPixel32f(state.g->gray, j+state.start.y, k+state.start.x, 0);
			float df=GetPixel32f(state.f->edgeX, currentRow, currentCol, 0);
			float df2=GetPixel32f(state.f->edgeX2, currentRow, currentCol, 0);

			if(isnan(f) || isinf(f)){
				cout<<"f is nan"<<endl;
				break;
			}
			
			if(isnan(g) || isinf(f)){
				cout<<"g is nan"<<endl;
				break;
			}
			
			if(isnan(df) || isinf(df)){
				cout<<"df is nan"<<endl;
				break;
			}
			
			if(isnan(df2) || isinf(df2)){
				cout<<"df2 is nan"<<endl;
				break;
			}

			
			#if DEBUG_LKT_ERROR
			cout<<"\t("<<j<<", "<<k<<")  f="<<f<<" g="<<g<<" w="<<w<<" df="<<df<<" df2="<<df2;
			#endif

			#if SIMPLE_LKT_ERROR
			if(df==0){
				break;
			}
			temp1x+=(g-f)/df;
			temp2x++;
			#else
			temp1x+=w*df*(g-f);
			temp2x+=w*df2;
			#endif

			#if DEBUG_LKT_ERROR
			cout<<" tmp1x="<<temp1x<<" tmp2x="<<temp2x;
			#endif

			df=GetPixel32f(state.f->edgeY, currentRow, currentCol, 0);
			df2=GetPixel32f(state.f->edgeY2, currentRow, currentCol, 0);

			if(isnan(df) || isinf(df)){
				cout<<"df is nan"<<endl;
				break;
			}
			
			if(isnan(df2) || isinf(df2)){
				cout<<"df2 is nan"<<endl;
				break;
			}

			#if SIMPLE_LKT_ERROR
			if(df==0){
				break;
			}
			temp1y+=(g-f)/df;
			temp2y++;
			#else
			temp1y+=w*df*(g-f);
			temp2y+=w*df2;
			#endif
			
			#if DEBUG_LKT_ERROR
			cout<<" dfY="<<df<<" df2Y="<<df2<<" tmp1y="<<temp1y<<" tmp2y="<<temp2y<<endl;
			#endif
		}
	}

	float xDelta=temp1x/temp2x;
	float yDelta=temp1y/temp2y;
	
	if(isnan(xDelta)){
		xDelta=0;
	}
	else if(xDelta==INFINITY || temp2x==0){
		cout<<"XINF";
		xDelta=0;
	}
	
	if(isnan(yDelta)){
		yDelta=0;
	}
	else if(yDelta==INFINITY || temp2y==0){
		cout<<"YINF";
		yDelta=0;
	}

	state.h.x+=xDelta;
	state.h.y+=yDelta;
}

void calculateLKWeight(LKImage* src1, LKImage* src2, LKImage* dest){
	//if(src1!=NULL && src2!=NULL && dest!=NULL){
	
	dest->edgeX=cvCreateImage( cvGetSize(src1->gray), IPL_DEPTH_32F, 1);
	dest->edgeY=cvCreateImage( cvGetSize(src1->gray), IPL_DEPTH_32F, 1);
	
	//cvSet(dest->edgeX, cvScalar(255));
	//cvSet(dest->edgeY, cvScalar(255));
	
		

		cvAbsDiff(src1->edgeX, src2->edgeX, dest->edgeX);
		cvAbsDiff(src1->edgeY, src2->edgeY, dest->edgeY);

		cvDiv(NULL, dest->edgeX, dest->edgeX, 1);
		cvDiv(NULL, dest->edgeY, dest->edgeY, 1);
		
	//}
}
