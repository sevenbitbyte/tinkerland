#ifndef ICPMATCHER_H
#define ICPMATCHER_H

#include "point.h"
#include "kdnode.h"

#include <inttypes.h>

#include <map>
#include <vector>

using namespace std;

#define MAX_ITERATIONS		(5)
#define MIN_ERROR_THRESHOLD	(0.1)

class ICPMatcher{
	public:
		ICPMatcher();
		~ICPMatcher();

		void clear();

		Transform computeTransform(vector<Point*> pts, int maxIter=-1, double minError=-1.0);

		void setCurrentPoints(vector<Point*> pts);
		void setTansform(Transform t);

		KDNode* getPreviousTree();
		vector<Point*> getCurrentPoints();
		Transform getCurrentTransform();
		map<Point*, KDNode*> getMatchingPairs();
		double getMatchError();
		int getNumIterations();



	protected:
		void updateCentroid();

		inline void computeMatchingPairs();
		inline void computeMatchError();
		inline void computeCurrentTransform();


	private:

		vector<Point*> prevPoints;
		KDNode* prevTree;
		Point prevCentroid;
		double prevAngle;

		vector<Point*> currentPoints;
		Point centroid;
		double currentAngle;

		map<Point*, KDNode*> matching;
		Transform transform;
		double error;
		int iterations;
};

#endif // ICPMATCHER_H
