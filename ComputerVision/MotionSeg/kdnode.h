#ifndef KDNODE_H
#define KDNODE_H

#include <opencv/cv.h>
#include <vector>
#include <map>
#include <algorithm>
#include <stdlib.h>
#include <iostream>
#include <set>

#include "affineModel.h"

#define MAX_LEVEL 19
#define MIN_POINTS 4
#define MAX_FILL_RATIO 0.8
#define MIN_FILL_RATIO 0.2

using namespace std;

struct Feature{
	CvPoint2D32f* location;
	AffineModel* model;
	bool grouped;
};

enum Split_Direction { X_Axis=0, Y_Axis=1};

int nodeCount=0;
int killCount=0;

class KDNode{
    public:
		KDNode(){
			parent=NULL;
			f=NULL;
			left=NULL;
			right=NULL;
			depth=1;
			location=cvPoint2D32f(0.0, 0.0);
			topLeft=cvPoint2D32f(0.0, 0.0);
			bottomRight=cvPoint2D32f(0.0, 0.0);
			direction=X_Axis;
			pixels=0;
		}

		KDNode(vector<Feature*> points){
			parent=NULL;
			f=NULL;
			left=NULL;
			right=NULL;
			depth=1;
			location=cvPoint2D32f(0.0, 0.0);
			direction=X_Axis;

			pixels=points.size();
			location.x=points.at(0)->location->x;
			location.y=points.at(0)->location->y;
			topLeft=location;
			bottomRight=location;
			
			if(pixels==1){
				return;
			}

			for(int i=1; i<pixels; i++){
				location.x+=points.at(i)->location->x;
				location.y+=points.at(i)->location->y;

				if(points.at(i)->location->x < topLeft.x){
					topLeft.x=points.at(i)->location->x;
				}
				if(points.at(i)->location->x > bottomRight.x){
					bottomRight.x=points.at(i)->location->x;
				}

				if(points.at(i)->location->y > topLeft.y){
					topLeft.y=points.at(i)->location->y;
				}
				if(points.at(i)->location->y < bottomRight.y){
					bottomRight.y=points.at(i)->location->y;
				}
			}

			location.x=location.x/pixels;
			location.y=location.y/pixels;
		}

		bool isLeaf() const{
			return (left==NULL && right==NULL);
		}

		~KDNode(){
			if(left!=NULL){
				delete left;
			}
			if(right!=NULL){
				delete right;
			}
			
			if(f!=NULL){
				delete f;
			}
		}

		static KDNode* buildTree(vector<Feature*> points, int level=0){
			if(points.size()<=0){
				//cout<<"points<=0 :"<<points.size()<<endl;
				killCount++;
				return NULL;
			}
			
			//cout<<"Construct"<<endl;
			KDNode* node=new KDNode(points);
			if(node->pixels==1){
				//cout<<"One"<<endl;
				node->f=points.back();
				return node;
			}

			vector<int> intList;
			vector<Feature*> leftPoints;
			vector<Feature*> rightPoints;

			node->depth=level;

			//cout<<"Branch level="<<level<<" points="<<points.size()<<endl;

			//if(node->size.width > node->size.height){
			node->direction=level%2;
			//int mid=0;
			if(node->direction==X_Axis){
				/*for(int i=0; i<points.size(); i++){
					intList.push_back(points.at(i)->x);
				}
				sort(intList.begin(), intList.end());
				mid=intList[intList.size()/2];*/

				for(int i=0; i<points.size(); i++){
					//if(points.at(i)->x > mid){
					if(points.at(i)->location->x > node->location.x){
						rightPoints.push_back(points[i]);
						continue;
					}
					else if(points.at(i)->location->x < node->location.x){
						leftPoints.push_back(points[i]);
						continue;
					}
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
					if(points.at(i)->location->y > node->location.y){
						rightPoints.push_back(points[i]);
						continue;
					}
					else if(points.at(i)->location->y < node->location.y){
						leftPoints.push_back(points[i]);
						continue;
					}
				}
			}




			node->left=buildTree(leftPoints, level+1);
			node->right=buildTree(rightPoints, level+1);
			
			if(node->left!=NULL){
				node->left->parent=node;
			}
			
			if(node->right!=NULL){
				node->right->parent=node;
			}
			
			nodeCount++;

			//cout<<"Constructed node with "<<node->pixels<<" pixels"<<endl;
			return node;
		}


		Feature* f;
        CvPoint2D32f location;
        CvSize size;
        CvPoint2D32f topLeft;
        CvPoint2D32f bottomRight;
		KDNode* left;
		KDNode* right;
		KDNode* parent;
		int depth;
		int direction;
		int pixels;
};

float pointDistance(CvPoint2D32f* pt0, CvPoint2D32f* pt1){
	float xDelta=(pt0->x - pt1->x);
	xDelta=xDelta*xDelta;
	
	float yDelta=(pt0->y - pt1->y);
	yDelta=yDelta*yDelta;
	
	yDelta=sqrtf(yDelta+xDelta);
	
	return yDelta;
}

set<KDNode*> findNodesInRadius(KDNode* root, Feature* feature, float radius){
	set<KDNode*> results;
	
	if(root->isLeaf()){
		if(root->f!=feature){
			if(pointDistance(&root->location, feature->location) < radius){
				if(!root->f->grouped){
					results.insert(root);
				}
			}
		}
		return results;
	}
		
		
	CvPoint2D32f pt=root->location;
	if(root->direction==X_Axis){
		pt.y=feature->location->y;
	}
	else{
		pt.x=feature->location->x;
	}
	
	if(pointDistance(&pt, feature->location) < radius){
		//Search both left and right
		//cout<<"Search both sides level="<<root->depth<<endl;
		if(root->left!=NULL){
			results=findNodesInRadius(root->left, feature, radius);
		}
		
		if(root->right!=NULL){
			set<KDNode*> temp=findNodesInRadius(root->right, feature, radius);
			results.insert(temp.begin(), temp.end());
		}
	}
	else{
		//cout<<"Nearest side check level="<<root->depth<<endl;
		//Search nearest side
		float leftDist=INFINITY;
		float rightDist=INFINITY;
	
		if(root->left!=NULL){
			leftDist=pointDistance(&root->left->location, feature->location);
		}
		
		if(root->right!=NULL){
			if(isinf(leftDist)){
				//cout<<"Taking left"<<endl;
				return findNodesInRadius(root->right, feature, radius);
			}
			rightDist=pointDistance(&root->right->location, feature->location);
		}
		else if(!isinf(leftDist)){
			//cout<<"Taking left"<<endl;
			return findNodesInRadius(root->left, feature, radius);
		}
		
		if(leftDist < rightDist){
			//cout<<"Taking left"<<endl;
			return findNodesInRadius(root->left, feature, radius);
		}
		else{
			//cout<<"Taking right"<<endl;
			return findNodesInRadius(root->right, feature, radius);
		}
	}

	return results;
}

KDNode* getNode(KDNode* root, Feature* f){
	if(root!=NULL){
		if(root->f==f){
			return root;
		}

		if(root->direction==X_Axis){
			if(root->location.x <= f->location->x){
				return getNode(root->left, f);
			}
			return getNode(root->right, f);
		}
		else{
			if(root->location.y <= f->location->y){
				return getNode(root->left, f);
			}
			return getNode(root->right, f);
		}
	}
}

int maxDepth(KDNode* node){
	if(node!=NULL){
		if(node->f != NULL){
			return 0;
		}
		else{
			int left=maxDepth(node->left)+1;
			int right=maxDepth(node->right)+1;
			if(left>right){
				return left;
			}
			return right;
		}
	}
	return 0;
}

void drawNodes(IplImage* img, KDNode* node, int level){
	if(node->depth < level){
		if(node->left!=NULL){
			drawNodes(img, node->left, level);
		}
		if(node->right!=NULL){
			drawNodes(img, node->right, level);
		}
	}
	//else if(node->depth <= level){
		if(node->depth==level){
		cvRectangle(img, cvPointFrom32f(node->topLeft), cvPointFrom32f(node->bottomRight), CV_RGB(255,255,255), 1);
		}
		
		if(node->direction!=X_Axis){
			cvLine(img, cvPoint(node->topLeft.x, node->location.y), cvPoint(node->bottomRight.x, node->location.y), CV_RGB(255,0,0), 1);
		}
		else{
			cvLine(img, cvPoint(node->location.x, node->topLeft.y), cvPoint(node->location.x, node->bottomRight.y), CV_RGB(0,0,255), 1);
		}
		
		/*cvLine(img, node->location, cvPoint(node->location.x+node->moment.x, node->location.y+node->moment.y), CV_RGB(255,255,0), 1);*/
	//}
}

/*void drawNodeCenters(IplImage*img, KDNode, int level){

}*/

#endif //KDNODE_H
