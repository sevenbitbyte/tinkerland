#ifndef AFFINEMODEL_H
#define AFFINEMODEL_H

#include <gsl/gsl_matrix.h>
#include <opencv/cv.h>

#define ERROR_MSG() cout<<"ERROR - "<<__PRETTY_FUNCTION__<<", "

struct AffineModel{
	float val[6];
	bool solved;
};

//CvPoint2D32f

bool calcAffineModel(AffineModel* model, CvPoint2D32f& pt0, CvPoint2D32f& pt1, CvPoint2D32f& pt2, CvPoint2D32f& pt3);

//Solves the linear system represented by mat using the Gauss-Jordan elimination method
bool solveLinearSystem(gsl_matrix* mat);

void printMatrix(gsl_matrix* mat);

void readMatrix(gsl_matrix* mat);

void testSolver();

#endif // AFFINEMODEL_H
