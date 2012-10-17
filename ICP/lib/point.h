#ifndef POINT_H
#define POINT_H

#include <inttypes.h>
#include <stdlib.h>

struct Position{
	double x;
	double y;
	double a;
};

struct Geometry{
	double px;
	double py;
	double pa;
	double sx;
	double sy;
};

struct Transform{
	Transform(double x=0.0, double y=0.0, double r=0.0);
	Transform(Position& p);

	void sum(Transform& t);

	double xDelta;
	double yDelta;
	double rDelta;
};

struct Point{
	Point(double x=0.0, double y=0.0);
	Point(const Point* other, const Transform* t=NULL, const Point* origin=NULL);

	void applyTransform(const Transform* t, const Point* origin=NULL);

	double getSquaredDistance(const Point* other) const;
	double getDistance(const Point* other) const;
	void fromPolar(double theta, double range);

	union{
		double value[2];

		struct{
			double x;
			double y;
		};
	};
};

#endif	//POINT_H
