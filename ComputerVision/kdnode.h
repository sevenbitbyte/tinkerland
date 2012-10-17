#ifndef KDNODE_H
#define KDNODE_H

#include <opencv/cv.h>
#include <vector>
#include <map>
#include <algorithm>
#include <stdlib.h>

#define MAX_LEVEL 9
#define MIN_POINTS 4
#define MAX_FILL_RATIO 0.8
#define MIN_FILL_RATIO 0.2

using namespace std;

enum Split_Direction { X_Axis=0, Y_Axis=1};

int nodeCount=0;
int killCount=0;

class KDNode{
    public:
		KDNode(){
			left=NULL;
			right=NULL;
			depth=1;
			location=cvPoint(0,0);
			topLeft=cvPoint(0,0);
			bottomRight=cvPoint(0,0);
			direction=X_Axis;
			size=cvSize(0,0);
			pixels=0;
		}

		KDNode(vector<CvPoint*> points){
			left=NULL;
			right=NULL;
			depth=1;
			location=cvPoint(0,0);
			direction=X_Axis;
			size=cvSize(0,0);

			pixels=points.size();
			location.x=points.at(0)->x;
			location.y=points.at(0)->y;
			topLeft=location;
			bottomRight=location;

			for(int i=1; i<pixels; i++){
				location.x+=points.at(i)->x;
				location.y+=points.at(i)->y;

				if(points.at(i)->x < topLeft.x){
					topLeft.x=points.at(i)->x;
				}
				if(points.at(i)->x > bottomRight.x){
					bottomRight.x=points.at(i)->x;
				}

				if(points.at(i)->y > topLeft.y){
					topLeft.y=points.at(i)->y;
				}
				if(points.at(i)->y < bottomRight.y){
					bottomRight.y=points.at(i)->y;
				}
			}

			location.x=location.x/pixels;
			location.y=location.y/pixels;

			size.width=bottomRight.x-topLeft.x;
			size.height=topLeft.y-bottomRight.y;

			moment=cvPoint(0,0);

			for(int i=0; i<pixels; i++){
				moment.x+=abs(points.at(i)->x-location.x);
				moment.y+=abs(points.at(i)->y-location.y);
			}

			moment.x=moment.x/pixels;
			moment.y=moment.y/pixels;
		}

		~KDNode(){
			if(left!=NULL){
				delete left;
			}
			if(right!=NULL){
				delete right;
			}
		}

		static KDNode* buildTree(vector<CvPoint*> points, int max_level=MAX_LEVEL, int level=0){
			if(level>=max_level || points.size()<MIN_POINTS){
				for(int i=0; i<points.size(); i++){
					delete points.at(i);
				}
				killCount++;
				return NULL;
			}
			KDNode* node=new KDNode(points);
			int area=(node->size.width * node->size.height);
			if(area < 4){
				for(int i=0; i<points.size(); i++){
					delete points.at(i);
				}
				delete node;
				killCount++;
				return NULL;
			}

			float fillRatio = node->pixels/area;
			if(fillRatio > MAX_FILL_RATIO){
			//if(area == node->pixels){
				for(int i=0; i<points.size(); i++){
					delete points.at(i);
				}
				//delete node;
				killCount++;
				return node;
			}

			if(level+1 > max_level){
				return node;
			}

			vector<int> intList;
			vector<CvPoint*> leftPoints;
			vector<CvPoint*> rightPoints;

			node->depth=level;

			//if(node->size.width > node->size.height){
			//node->direction=level%2;
			//int mid=0;
			//if(node->direction==X_Axis){
			if(node->size.width > node->size.height){
				/*for(int i=0; i<points.size(); i++){
					intList.push_back(points.at(i)->x);
				}
				sort(intList.begin(), intList.end());
				mid=intList[intList.size()/2];*/

				for(int i=0; i<points.size(); i++){
					//if(points.at(i)->x > mid){
					if(points.at(i)->x > node->location.x){
						rightPoints.push_back(points[i]);
						continue;
					}
					leftPoints.push_back(points[i]);
				}
			}
			else{
				/*for(int i=0; i<points.size(); i++){
					intList.push_back(points.at(i)->y);
				}
				sort(intList.begin(), intList.end());
				mid=intList[intList.size()/2];*/

				for(int i=0; i<points.size(); i++){
					//if(points.at(i)->y > mid){
					if(points.at(i)->y > node->location.y){
						rightPoints.push_back(points[i]);
						continue;
					}
					leftPoints.push_back(points[i]);
				}
			}




			node->left=buildTree(leftPoints, max_level, level+1);
			node->right=buildTree(rightPoints, max_level, level+1);
			nodeCount++;

			//cout<<"Constructed node with "<<node->pixels<<" pixels"<<endl;
			return node;
		}

        CvPoint location;
        CvSize size;
        CvPoint topLeft;
        CvPoint bottomRight;
        CvPoint moment;
		KDNode* left;
		KDNode* right;
		int depth;
		int direction;
		int pixels;
};

void drawNodes(IplImage* img, KDNode* node, int level){
	if(node->depth < level){
		if(node->left!=NULL){
			drawNodes(img, node->left, level);
		}
		if(node->right!=NULL){
			drawNodes(img, node->right, level);
		}
	}
	else if(node->depth == level){

		cvRectangle(img, node->topLeft, node->bottomRight, CV_RGB(255,0,0), 1);
		cvLine(img, cvPoint(node->location.x, node->topLeft.y), cvPoint(node->location.x, node->bottomRight.y), CV_RGB(0,0,255), 1);
		cvLine(img, cvPoint(node->topLeft.x, node->location.y), cvPoint(node->bottomRight.x, node->location.y), CV_RGB(0,0,255), 1);
		/*cvLine(img, node->location, cvPoint(node->location.x+node->moment.x, node->location.y+node->moment.y), CV_RGB(255,255,0), 1);*/
	}
}

/*void drawNodeCenters(IplImage*img, KDNode, int level){

}*/

#endif //KDNODE_H
