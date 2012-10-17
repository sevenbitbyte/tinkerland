#include "utils.h"
#include "kdnode.h"
#include "affineModel.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/time.h>

using namespace std;

#define MAX_FEATURES 600
#define MIN_FEATURES 100
#define MIN_TRACKED 50
#define FEATURE_QUALITY 0.03
#define FEATURE_RADIUS 10


//Buffers
AffineModel affineModels[MAX_FEATURES];
CvPoint2D32f* pointsPtr[4];
uint32_t tracked[MAX_FEATURES];
IplImage* greyImg=NULL;
IplImage* prevGreyImg=NULL;
IplImage* pyramid=NULL;
IplImage* prevPyramid=NULL;
IplImage* eigenImg=NULL;
IplImage* tempImg=NULL;
int detectedFeatures=0;
int trackedFeatures=0;
int lktFlags=0;
char featureStatus[MAX_FEATURES];


void detectFeatures();
void segmentFeatures(KDNode* root, vector<Feature*> features);


struct FeatureGroup{
	set<KDNode*> features;
	AffineModel model;
};

int main(int argc, char** argv){
	
	if(parseArgs(argc, argv)){
	
		pointsPtr[0]=(CvPoint2D32f*) malloc(sizeof(CvPoint2D32f) * MAX_FEATURES);
		pointsPtr[1]=(CvPoint2D32f*) malloc(sizeof(CvPoint2D32f) * MAX_FEATURES);
		pointsPtr[2]=(CvPoint2D32f*) malloc(sizeof(CvPoint2D32f) * MAX_FEATURES);
		pointsPtr[3]=(CvPoint2D32f*) malloc(sizeof(CvPoint2D32f) * MAX_FEATURES);
		memset(tracked, 0, MAX_FEATURES);
	
		runLoop();
		
		free(pointsPtr[0]);
		free(pointsPtr[1]);
		free(pointsPtr[2]);
		free(pointsPtr[3]);
	}
}




bool allocateUserData(IplImage *input){
	greyImg=cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 1);
	prevGreyImg=cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 1);
	pyramid=cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 1);
	prevPyramid=cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 1);
	eigenImg=cvCreateImage( cvGetSize(input), IPL_DEPTH_32F, 1 );
	tempImg=cvCreateImage( cvGetSize(input), IPL_DEPTH_32F, 1 );
	
	return true;
}

void deallocateUserData(){
	if(greyImg!=NULL){
		cvReleaseImage(&greyImg);
		greyImg=NULL;
	}
	
	if(prevGreyImg!=NULL){
		cvReleaseImage(&prevGreyImg);
		prevGreyImg=NULL;
	}
	
	if(pyramid!=NULL){
		cvReleaseImage(&pyramid);
		pyramid=NULL;
	}
	
	if(prevPyramid!=NULL){
		cvReleaseImage(&prevPyramid);
		prevPyramid=NULL;
	}
	
	if(eigenImg!=NULL){
		cvReleaseImage(&eigenImg);
		eigenImg=NULL;
	}
	
	if(tempImg!=NULL){
		cvReleaseImage(&tempImg);
		tempImg=NULL;
	}
}

bool preProcess(IplImage *input){
	CvPoint2D32f* temp=pointsPtr[3];
	pointsPtr[3]=pointsPtr[2];
	pointsPtr[2]=pointsPtr[1];
	pointsPtr[1]=pointsPtr[0];
	pointsPtr[0]=temp;
	

	return true;
}

bool processImage(IplImage *input){

	cvCvtColor(input, greyImg, CV_BGR2GRAY);

	if(frameCount > 2){
		if(detectedFeatures<MIN_FEATURES || resetRequest){
			detectFeatures();
			for(int i=0; i<detectedFeatures; i++){
				tracked[i]=0;
			}
			resetRequest=false;
		}
		else{
			cvCalcOpticalFlowPyrLK( prevGreyImg, greyImg, prevPyramid, pyramid,
													pointsPtr[1], pointsPtr[0], detectedFeatures, cvSize(10,10), 3, featureStatus, NULL,
													cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), lktFlags );
			lktFlags |= CV_LKFLOW_PYR_A_READY;
			
			trackedFeatures=0;
			int solvedAffinesModels=0;
			int attemptedSolves=0;
			
			vector<Feature*> featureVect;
			
			for(int i=0; i<detectedFeatures; i++){
				if(featureStatus[i]==0){
					tracked[i]=0;
					pointsPtr[0][i]=pointsPtr[1][i];
					/*pointsPtr[0][i].x=0.0;
					pointsPtr[0][i].y=0.0;*/
				}
				else{
					tracked[i]++;
					trackedFeatures++;
					
					if(tracked[i]>=4){
						attemptedSolves++;
						affineModels[i].solved=calcAffineModel(&affineModels[i], pointsPtr[0][i], pointsPtr[1][i], pointsPtr[2][i], pointsPtr[3][i]);
						if(affineModels[i].solved==true){
							Feature* f = new Feature;
							f->grouped=false;
							f->location=&pointsPtr[0][i];
							f->model=&affineModels[i];
							featureVect.push_back(f);
							solvedAffinesModels++;						
						}
					}
				}
			}
		
			//segmentFeatures();
			
			KDNode* root=KDNode::buildTree(featureVect);
			if(root!=NULL){
				//while(!featureVect.empty()){
					FeatureGroup* group=new FeatureGroup;
					KDNode* node=getNode(root, featureVect.back());
					if(node==NULL){
						cout<<"ERROR: Null getNode result!"<<endl;
						//exit(-1);
					}
					//group->features.insert(featureVect.back());
					
					set<KDNode*> neighbors = findNodesInRadius(root, featureVect.back(), 50.0);
					/*group->features.insert(neighbors.begin(), neighbors.end());
					
					for(int i=0; i<6; i++){
						group->model.val[i]=0;
					}
					
					set<KDNode*>::iterator iter=group->features.begin();
					for(iter; iter!=group->features.end(); iter++)	{
						for(int i=0; i<6; i++){
							group->model.val[i]+=(*iter)->f->model->val[i];
						}
					}
					
					for(int i=0; i<6; i++){
						group->model.val[i]=group->model.val[i]/(float)group->features.size();
					}*/
					
					
				
				
				/*set<KDNode*> neighbors = findNodesInRadius(root, featureVect.back(), 50.0);
				cout<<"Got "<<neighbors.size()<<" neighbors ";*/
				
				CvPoint p=cvPointFrom32f(*featureVect.back()->location);
				cvDrawCircle(input, p, 50, cvScalar(0, 255, 255), 1);
				
				  for( set<KDNode*>::const_iterator iter = neighbors.begin(); iter != neighbors.end(); ++iter ) {
					p=cvPointFrom32f(*(*iter)->f->location);
					cvDrawCircle(input, p, 5, cvScalar(0, 255, 255), 1);
				}
				
				drawNodes(input, root, 4);
				//cout<<"Max Depth="<<maxDepth(node)<<" ";
				delete root;
			}
			
			
			
			/*while(!featureVect.empty()){
				delete featureVect.back();
				featureVect.pop_back();
			}*/
			
			cout<<"Tracked "<<trackedFeatures<<" features and solved "<<solvedAffinesModels<<" models out of "<<attemptedSolves;
			
			/*if(solvedAffinesModels > 0){				
				//
			}
			else{
				waitPause();
			}*/
			
			if(trackedFeatures<MIN_TRACKED){
				detectedFeatures=0;
			}
		}
	}
	else{
		detectFeatures();
	}
	return true;
}

/*void segmentFeatures(KDNode* root, vector<Feature*> features){
	
}*/



void detectFeatures(){
	detectedFeatures=MAX_FEATURES;
	cvGoodFeaturesToTrack(greyImg, eigenImg, tempImg, pointsPtr[0], &detectedFeatures, FEATURE_QUALITY, FEATURE_RADIUS, NULL, 3, 0, 0.04);

	cvFindCornerSubPix(greyImg, pointsPtr[0], detectedFeatures, cvSize(10,10), cvSize(-1,-1), cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
	cout<<"Found "<<detectedFeatures<<" features ";
}



bool postProcess(IplImage *input){

	for(int i=0; i<detectedFeatures; i++){
		CvPoint p = cvPointFrom32f(pointsPtr[0][i]);
		if(tracked[i] >= 4){
			cvLine(input, cvPoint(p.x-2, p.y), cvPoint(p.x+2, p.y), cvScalar(0,255,0), 1);
			cvLine(input, cvPoint(p.x, p.y-2), cvPoint(p.x, p.y+2), cvScalar(0,255,0), 1);			
		}
		else{
			cvLine(input, cvPoint(p.x-2, p.y), cvPoint(p.x+2, p.y), cvScalar(0,0,255), 1);
			cvLine(input, cvPoint(p.x, p.y-2), cvPoint(p.x, p.y+2), cvScalar(0,0,255), 1);
		}
	}

	IplImage* tempGrey=NULL;
	CV_SWAP(prevGreyImg, greyImg, tempGrey);
	CV_SWAP(prevPyramid, pyramid, tempGrey);
	
	cvShowImage("InputFrame", input)	;
	return true;
}
