#ifndef LKT_H
#define LKT_H

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <vector>
#include <iostream>
#include <fstream>

#define GetPixel8u(i,r,c,chan) i->imageData[((r*i->widthStep)+(c*i->nChannels)+chan)]
#define GetPixel16u(i,r,c,chan) ((uint16_t*)i->imageData)[((r*(i->widthStep/sizeof(uint16_t)))+(c*i->nChannels)+chan)]
#define GetPixel32f(i,r,c,chan) (((float*)i->imageData)[((r*(i->widthStep/sizeof(float)))+(c*i->nChannels)+chan)])

#define MAX_ITERATIONS 30

using namespace std;

struct LKFeature{
	vector<CvPoint> points;
	CvPoint estimate;
};

class LKImage{
	public:
		IplImage* gray;
		IplImage* edgeX;
		IplImage* edgeY;
		IplImage* edgeX2;	//Squared edges in X direction
		IplImage* edgeY2;	//Squared edges in Y direction

		LKImage();
		LKImage(IplImage* input, bool calcSquared=false);
		~LKImage();

		void updateEdge2();
		bool hasEdge2();
};

struct LKState{
	int window;
	LKImage* f;
	LKImage* g;
	LKImage* w;
	CvPoint start;
	CvPoint current;
	CvPoint2D32f h;
	float error;
	//CvPoint2D32f error;
};

void calculateLKWeight(LKImage* src1, LKImage* src2, LKImage* dest);

void calculateLKWindowError(LKState& state);

void calculateLKFlow(LKImage* initial, LKImage* final, vector<LKFeature*> features, int size=3);

#endif //LKT_H
