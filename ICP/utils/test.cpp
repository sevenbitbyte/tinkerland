#include "point.h"
#include "kdnode.h"
#include "icpmatcher.h"

#include <vector>
#include <iostream>

using namespace std;

int main(int argc, char** argv){

	cout << "ICP Tester" << endl;

	vector<Point*> points;

	while(1){
		double value1 = 0.0;
		double value2 = 0.0;

		cout << "X Value: ";
		cin >> value1;

		if(cin.eof() || cin.fail() || !cin.good()){ cout<<"END LOOP"<<endl;break; }

		cout << "Y Value: ";
		cin >> value2;

		if(cin.eof()){ break; }

		Point* pt = new Point(value1, value2);
		points.push_back(pt);

	}


	ICPMatcher m;

	cout << "Computing transform of " << points.size() << " points" <<endl;

	Transform t = m.computeTransform(points, 1);

	cout << "Transform: x="<<t.xDelta << "\ty=" << t.yDelta << "\tr=" << t.rDelta << endl;

	cout << "Sifting up one space" << endl;

	//t.yDelta = 1.0;
	t.rDelta = 0.785398163;

	vector<Point*> points2;

	for(int i=0; i<points.size(); i++){
		//points[i]->applyTransform(&t);
		Point* p = new Point(points[i], &t);
		points2.push_back(p);
	}

	cout << "Computing transform of " << points2.size() << " points" <<endl;
	t = m.computeTransform(points2);
	cout << "Transform: x="<<t.xDelta << "\ty=" << t.yDelta << "\tr=" << t.rDelta << endl;

}
