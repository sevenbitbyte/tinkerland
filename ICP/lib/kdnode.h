#ifndef KDNODE_H
#define KDNODE_H

#include "point.h"

#include <map>
#include <vector>
#include <algorithm>

#include <stdlib.h>
#include <inttypes.h>

#define MAX_LEVEL 9
#define MIN_POINTS 4
#define MAX_FILL_RATIO 0.8
#define MIN_FILL_RATIO 0.2
#define NUM_DIMENSIONS 2

using namespace std;

enum Split_Direction { X_Axis=0, Y_Axis=1};

class KDNode{
    public:
		KDNode(KDNode* p=NULL);
		~KDNode();

		double getWidth();
		double getHeight();

		static KDNode* getNearestNode(KDNode* root, const Point* pt);

		static KDNode* buildKDTree(vector<Point*> points, int depth=0, KDNode* p=NULL);

		static int getMaxDepth(KDNode* node);
		static int getNodeCount(KDNode* node);

		Point location;
		Point topLeft;
		Point bottomRight;
		/*union{
			KDNode* nodes[2];

			struct{*/
				KDNode* left;
				KDNode* right;
			/*};
		};*/

		KDNode* parent;
		int depth;
		int direction;
};

#endif //KDNODE_H
