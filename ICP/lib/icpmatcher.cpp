#define BUILD_DEBUG true

#include "debug.h"
#include "icpmatcher.h"

#include <math.h>
#include <inttypes.h>
#include <stdint.h>

#include <map>
#include <vector>

using namespace std;

ICPMatcher::ICPMatcher(){
	prevTree = NULL;

	clear();
}

ICPMatcher::~ICPMatcher(){
	if(prevTree != NULL){
		delete prevTree;
	}
}

void ICPMatcher::clear(){
	if(prevTree != NULL){
		delete prevTree;
	}

	prevTree = NULL;
	currentPoints.clear();
	matching.clear();
	transform = Transform();
	centroid = Point();
	prevCentroid = Point();
	prevAngle = 0.0;
	currentAngle = 0.0;

	error = 0.0;
	iterations = 0;
}

Transform ICPMatcher::computeTransform(vector<Point*> pts, int maxIter, double minError){
	if(maxIter < 0){
		maxIter = MAX_ITERATIONS;
	}

	if(minError <= 0.0){
		minError = MIN_ERROR_THRESHOLD;
	}

	setCurrentPoints(pts);

	int iterCount=0;

	do{
		computeMatchingPairs();
		computeCurrentTransform();
		computeMatchError();

		DEBUG_MSG("iter="<<iterCount<<"\terror="<<error
				<<"\txDelta="<<transform.xDelta
				<<"\tyDelta="<<transform.yDelta
				<<"\trDelta="<<transform.rDelta<<endl);

		iterCount++;
	} while(error > minError && iterCount < maxIter);

	#ifdef BUILD_DEBUG
	if(error <= minError){
		DEBUG_MSG("ERROR THRESHOLD error="<<error<<endl);
	}
	if(iterCount >= maxIter){
		DEBUG_MSG("MAX ITERATIONS iter="<<iterCount<<endl);
	}
	#endif
	
	iterations = iterCount;

	return transform;
}

void ICPMatcher::setCurrentPoints(vector<Point*> pts){
	DEBUG_MSG(""<<endl);
	if(prevTree != NULL){
		//Clean up
		delete prevTree;
	}

	if(currentPoints.size() > 0){
		prevTree = KDNode::buildKDTree(currentPoints);
		prevCentroid = centroid;
		prevAngle = currentAngle;
		prevPoints = currentPoints;
	}

	error = 0.0;
	iterations = 0;
	transform = Transform();
	matching.clear();

	currentPoints = pts;

	//Update centroid
	updateCentroid();
}

void ICPMatcher::updateCentroid(){

	DEBUG_MSG(""<<endl);
	centroid.x = 0;
	centroid.y = 0;

	for(uint32_t i=0; i<currentPoints.size(); i++){
		centroid.x += currentPoints[i]->x;
		centroid.y += currentPoints[i]->y;
	}

	centroid.x = centroid.x / currentPoints.size();
	centroid.y = centroid.y / currentPoints.size();
}

void ICPMatcher::setTansform(Transform t){
	transform = t;
}

KDNode* ICPMatcher::getPreviousTree(){
	return prevTree;
}

vector<Point*> ICPMatcher::getCurrentPoints(){
	return currentPoints;
}

Transform ICPMatcher::getCurrentTransform(){
	return transform;
}

map<Point*, KDNode*> ICPMatcher::getMatchingPairs(){
	return matching;
}

double ICPMatcher::getMatchError(){
	return error;
}

int ICPMatcher::getNumIterations(){
	return iterations;
}

void ICPMatcher::computeMatchingPairs(){
	//DEBUG_MSG("prevTree="<<(int)prevTree<<endl);
	matching.clear();

	//Used to compute angle of rotation of points
	double coVariance = 0.0;
	double variance = 0.0;

	if(prevTree == NULL){
		//Nothing to match against

		//Still need to compute the currentAngle
		for(uint32_t i=0; i<currentPoints.size(); i++){
			Point* pt = currentPoints[i];

			//Update variance and covariance of point positions
			double xVar = (pt->x - centroid.x);
			coVariance += xVar + (pt->y - centroid.y);
			variance += xVar * xVar;
		}

		//Update variance and covariance of point positions
		variance = variance / currentPoints.size();
		coVariance = coVariance / (currentPoints.size() - 1);

		//Compute angle of rotation of points
		currentAngle = atan2(coVariance, variance);
		return;
	}

	//Only compute point transform when needed
	if(transform.xDelta != 0.0 || transform.yDelta != 0.0 || transform.rDelta != 0.0){
		//Need to transform location of all points
		Point origin = Point(0.0, 0.0);
		Point newCentroid = Point(&centroid, &transform, &origin);

		//Iterate over all points
		for(uint32_t i=0; i<currentPoints.size(); i++){
			//Transform point
			Point pt = Point(currentPoints[i], &transform, &newCentroid);

			//Find nearest point
			KDNode* node = KDNode::getNearestNode(prevTree, &pt);

			//Update variance and covariance of point positions
			double xVar = (pt.x - newCentroid.x);
			coVariance += xVar + (pt.y - newCentroid.y);
			variance += xVar * xVar;

			matching.insert(make_pair(currentPoints[i], node));
		}
	}
	else{
		//No need to compute point transform


		//Iterate over all points
		for(uint32_t i=0; i<currentPoints.size(); i++){
			Point* pt = currentPoints[i];
			//Find nearest point
			KDNode* node = KDNode::getNearestNode(prevTree, pt);

			//Update variance and covariance of point positions
			double xVar = (pt->x - centroid.x);
			coVariance += xVar + (pt->y - centroid.y);
			variance += xVar * xVar;

			matching.insert(make_pair(pt, node));
		}
	}

	//Update variance and covariance of point positions
	variance = variance / currentPoints.size();
	coVariance = coVariance / (currentPoints.size() - 1);

	//Compute angle of rotation of points
	currentAngle = atan2(coVariance, variance);
}

void ICPMatcher::computeMatchError(){
	DEBUG_MSG(""<<endl);
	error = 0.0;

	map<Point*, KDNode*>::iterator matchIter = matching.begin();

	for(; matchIter != matching.end(); matchIter++){
		Point p=Point(matchIter->first, &transform, &centroid);
		error += p.getDistance(&(matchIter->second->location));
	}

	error = error / matching.size();
}

void ICPMatcher::computeCurrentTransform(){
	DEBUG_MSG(""<<endl);
	if(prevTree == NULL){
		transform = Transform();
		return;
	}

	//Point origin = Point(0.0, 0.0);
	//Point newCentroid = Point(&centroid, &transform, &origin);

	//Detemine translation
	transform.xDelta = prevCentroid.x - centroid.x;
	transform.yDelta = prevCentroid.y - centroid.y;

	//Detemine rotation
	transform.rDelta = prevAngle - currentAngle;
}



