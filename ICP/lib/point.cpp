#include "point.h"

#include <math.h>

Transform::Transform(double x, double y, double r){
	xDelta = x;
	yDelta = y;
	rDelta = r;
}

Transform::Transform(Position& p){
	xDelta = p.x;
	yDelta = p.y;
	rDelta = p.a;
}

void Transform::sum(Transform& t){
	xDelta += t.xDelta;
	yDelta += t.yDelta;
	rDelta += t.rDelta;
}

Point::Point(double x, double y){
	this->x = x;
	this->y = y;
}

Point::Point(const Point* other, const Transform* t, const Point* origin){
	this->x = other->x;
	this->y = other->y;
	applyTransform(t);
}

void Point::applyTransform(const Transform* t, const Point* origin){
	if(t==NULL){
		return;
	}

	//if(t->rDelta != 0.0){
		Point zero=Point();
		if(origin == NULL){
			origin = &zero;
		}

		//Compute distance from origin
		double xDist = this->x - origin->x;
		double yDist = this->y - origin->y;
		//double distance = sqrt((xDist*xDist) + (yDist*yDist));

		//Update position
		this->x = (xDist * cos(t->rDelta)) - (yDist* sin(t->rDelta));
		this->y = (xDist * sin(t->rDelta)) + (yDist* cos(t->rDelta));
	//}

	this->x += t->xDelta;
	this->y += t->yDelta;
}

void Point::fromPolar(double theta, double range){
	this->x = range * cos(theta);
	this->y = range * sin(theta);
}

double Point::getSquaredDistance(const Point* other) const {
	double xDiff = this->x - other->x;
	double yDiff = this->y - other->y;
	return ((xDiff*xDiff) + (yDiff*yDiff));
}


double Point::getDistance(const Point* other) const {
	double xDiff = this->x - other->x;
	double yDiff = this->y - other->y;
	return sqrt((xDiff*xDiff) + (yDiff*yDiff));
}
