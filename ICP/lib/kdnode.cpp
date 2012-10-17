#include "debug.h"
#include "kdnode.h"
#include "point.h"

#include <math.h>
#include <vector>
#include <map>
#include <algorithm>
#include <stdlib.h>
#include <inttypes.h>


int _workingAxis = 0;

bool axisSort(const Point* pt1, const Point* pt2){
	return (pt1->value[_workingAxis] > pt2->value[_workingAxis]);
}

KDNode::KDNode(KDNode* p){
	left=NULL;
	right=NULL;
	parent=p;
	depth=0;
	location=Point();
	direction=X_Axis;
}

KDNode::~KDNode(){
	if(left!=NULL){
		delete left;
	}

	if(right!=NULL){
		delete right;
	}
}

double KDNode::getWidth(){
	return fabs(topLeft.x - bottomRight.x);
}


double KDNode::getHeight(){
	return fabs(topLeft.y - bottomRight.y);
}

KDNode* KDNode::getNearestNode(KDNode* root, const Point* pt){
	if(root == NULL || pt == NULL){
		return NULL;
	}

	KDNode* currentNode=root;

	while(1){
		int axis = currentNode->direction;
		if(currentNode->location.value[axis] <= pt->value[axis]){
			if(currentNode->left != NULL){
				currentNode = currentNode->left;
			}
			else{
				break;
			}
		}
		else{
			if(currentNode->right != NULL){
				currentNode = currentNode->right;
			}
			else{
				break;
			}
		}
	}

	KDNode* bestNode=NULL;
	double bestSqDist=0;

	while(currentNode != root){

		//Check if currentNode is closer
		double sqDist = currentNode->location.getSquaredDistance(pt);
		if(sqDist < bestSqDist || bestNode==NULL){
			//Current node is closest
			bestNode = currentNode;
			bestSqDist = sqDist;
		}

		//Check if a node on the other side of current plane is closer
		int axis = currentNode->direction;
		sqDist = currentNode->location.value[axis] - pt->value[axis];
		sqDist = sqDist*sqDist;		//Square value to simplify comparison

		if(sqDist < bestSqDist){
			//Check for closer node on other side of spliting plane
			KDNode* temp=NULL;

			//Select branch on opposite side
			if(currentNode->location.value[axis] > pt->value[axis]){
				temp = currentNode->left;
			}
			else{
				temp = currentNode->right;
			}

			if(temp != NULL){
				//Find best node in opposing branch
				temp = getNearestNode(temp, pt);

				if(temp!=NULL){
					sqDist = temp->location.getSquaredDistance(pt);

					if(sqDist < bestSqDist){
						//Point on other side is closests seen
						bestNode = temp;
						bestSqDist = sqDist;
					}
				}
			}
		}

		currentNode = currentNode->parent;
	}

	return bestNode;
}


KDNode* KDNode::buildKDTree(vector<Point*> points, int depth, KDNode* p){
	//DEBUG_MSG("Got " << points.size() << " points" <<endl);
	if(points.empty()){
		return NULL;
	}

	_workingAxis = depth % NUM_DIMENSIONS;

	//DEBUG_MSG("Set axis="<< _workingAxis <<endl);


	//Sort vector of points
	sort(points.begin(), points.end(), axisSort);

	int medianIndex = (points.size()>>1);

	//DEBUG_MSG("medianIndex="<< medianIndex <<endl);


	vector<Point*>::iterator median = points.begin()+medianIndex;

	//DEBUG_MSG("Newing KDNode"<<endl);

	KDNode* node = new KDNode(p);

	//DEBUG_MSG("Newed"<<endl);

	if(points.size() > 0){
		node->topLeft = *points.front();
		node->bottomRight = *points.front();
	}

	for(uint32_t i=0; i<points.size(); i++){
		Point* p=points[i];

		if(p->x < node->topLeft.x){
			node->topLeft.x = p->x;
		}
		if(p->x > node->bottomRight.x){
			node->bottomRight.x = p->x;
		}

		if(p->y > node->topLeft.y){
			node->topLeft.y = p->y;
		}
		if(p->y < node->bottomRight.y){
			node->bottomRight.y = p->y;
		}
	}

	node->depth = depth;
	node->direction = _workingAxis;

	//DEBUG_MSG("**median"<<endl);

	node->location = **median;

	//DEBUG_MSG("Building left"<<endl);

	vector<Point*> leftPoints = vector<Point*>(points.begin(), median);

	//DEBUG_MSG("leftPoints has "<< leftPoints.size() << "elements" << endl);

	node->left = buildKDTree(leftPoints, depth+1, node);

	//DEBUG_MSG("Building right"<<endl);

	vector<Point*> rightPoints = vector<Point*>(median+1, points.end());

	//DEBUG_MSG("rightPoints has "<< rightPoints.size() << "elements" << endl);

	node->right = buildKDTree(rightPoints, depth+1, node);

	return node;
}


int KDNode::getMaxDepth(KDNode* node){
	int depth = node->depth;

	if(node->left != NULL){
		depth = getMaxDepth(node->left);
	}

	if(node->right != NULL){
		depth = max(depth, getMaxDepth(node->right));
	}

	return depth;
}

int KDNode::getNodeCount(KDNode* node){
	int count = 0;

	if(node->left != NULL){
		count = getNodeCount(node->left);
	}

	if(node->right != NULL){
		count += getNodeCount(node->right);
	}

	return count+1;
}

