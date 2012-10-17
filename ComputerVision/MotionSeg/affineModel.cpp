#include "affineModel.h"
#include <gsl/gsl_matrix.h>
#include <opencv/cv.h>
#include <math.h>

#include <iostream>

using namespace std;

bool calcAffineModel(AffineModel* model, CvPoint2D32f& pt0, CvPoint2D32f& pt1, CvPoint2D32f& pt2, CvPoint2D32f& pt3){
	gsl_matrix* mat=gsl_matrix_alloc(3, 4);

	

	gsl_matrix_set(mat, 0, 0, pt0.x);
	gsl_matrix_set(mat, 0, 1, pt0.y);
	gsl_matrix_set(mat, 0, 2, 1.0f);
	gsl_matrix_set(mat, 0, 3, pt1.x);
	
	gsl_matrix_set(mat, 1, 0, pt1.x);
	gsl_matrix_set(mat, 1, 1, pt1.y);
	gsl_matrix_set(mat, 1, 2, 1.0f);
	gsl_matrix_set(mat, 1, 3, pt2.x);
	
	gsl_matrix_set(mat, 2, 0, pt2.x);
	gsl_matrix_set(mat, 2, 1, pt2.y);
	gsl_matrix_set(mat, 2, 2, 1.0f);
	gsl_matrix_set(mat, 2, 3, pt3.x);
	
	if(!solveLinearSystem(mat)){
		ERROR_MSG()<<"Failed to solve X axis!"<<endl;
		gsl_matrix_free(mat);
		return false;
	}
	
	model->val[0]=gsl_matrix_get(mat, 0, 3);
	model->val[1]=gsl_matrix_get(mat, 1, 3);
	model->val[2]=gsl_matrix_get(mat, 2, 3);
	
	
	gsl_matrix_set(mat, 0, 0, pt0.x);
	gsl_matrix_set(mat, 0, 1, pt0.y);
	gsl_matrix_set(mat, 0, 2, 1.0f);
	gsl_matrix_set(mat, 0, 3, pt1.y);
	
	gsl_matrix_set(mat, 1, 0, pt1.x);
	gsl_matrix_set(mat, 1, 1, pt1.y);
	gsl_matrix_set(mat, 1, 2, 1.0f);
	gsl_matrix_set(mat, 1, 3, pt2.y);
	
	gsl_matrix_set(mat, 2, 0, pt2.x);
	gsl_matrix_set(mat, 2, 1, pt2.y);
	gsl_matrix_set(mat, 2, 2, 1.0f);
	gsl_matrix_set(mat, 2, 3, pt3.y);	
	
	if(!solveLinearSystem(mat)){
		ERROR_MSG()<<"Failed to solve Y axis!"<<endl;
		gsl_matrix_free(mat);
		return false;
	}
	
	//CV_SWAP(NULL, NULL, NULL);

	model->val[3]=gsl_matrix_get(mat, 0, 3);
	model->val[4]=gsl_matrix_get(mat, 1, 3);
	model->val[5]=gsl_matrix_get(mat, 2, 3);
	
	gsl_matrix_free(mat);
	return true;
}

bool solveLinearSystem(gsl_matrix* mat){
	double max=0.0f;
	int maxRow=0;
	
	//STEP 1: Find row with max value in col 0
	for(int i=0; i < mat->size1; i++){
		double val=fabs(gsl_matrix_get(mat, i, 0));
		if(val > max){
			max=val;
			maxRow=i;
		}
	}
	if(max==0){
		ERROR_MSG()<<"Failed at step 1!"<<endl;
		return false;
	}
	
	//STEP 2: Swap pivot row into 0th position
	if(maxRow!=0){
		gsl_matrix_swap_rows(mat, 0, maxRow);
	}
	
	//STEP 3: Divide first row by max value
	for(int i=0; i < mat->size2; i++){
		gsl_matrix_set(mat, 0, i, gsl_matrix_get(mat, 0, i)/max);
	}
	
	//Step 4: Multiply 1st row by 1st element of 2nd row, then subtract this multiple from the 2nd row
	double factor=gsl_matrix_get(mat, 1, 0);
	for(int i=0; i < mat->size2; i++){
		double val=gsl_matrix_get(mat, 1, i) - (factor * gsl_matrix_get(mat, 0, i));
		gsl_matrix_set(mat, 1, i, val);
	}

	//Step 5: Multiply 1st row by 1st element of 3rd row, then subtract this multiple from the 3rd row
	factor=gsl_matrix_get(mat, 2, 0);
	for(int i=0; i < mat->size2; i++){
		double val=gsl_matrix_get(mat, 2, i) - (factor * gsl_matrix_get(mat, 0, i));
		gsl_matrix_set(mat, 2, i, val);
	}
	
	//STEP 6: Find row with max value in col 1, exclude row 0
	
	max=0.0f;
	maxRow=0;
	for(int i=1; i < mat->size1; i++){
		double val=fabs(gsl_matrix_get(mat, i, 1));
		if(val > max){
			max=val;
			maxRow=i;
		}
	}
	if(max==0){
		ERROR_MSG()<<"Failed at step 6!"<<endl;
		return false;
	}
	
	if(maxRow==2){
		gsl_matrix_swap_rows(mat, 1, 2);
	}
	
	//STEP 7: Divide 2nd row by max value
	for(int i=0; i < mat->size2; i++){
		gsl_matrix_set(mat, 1, i, gsl_matrix_get(mat, 1, i)/max);
	}
	
	//Step 8: Multiply 2nd row by 2nd element in 1st row, and subtract from 1st row
	factor=gsl_matrix_get(mat, 0, 1);
	for(int i=0; i < mat->size2; i++){
		double val=gsl_matrix_get(mat, 0, i) - (factor * gsl_matrix_get(mat, 1, i));
		gsl_matrix_set(mat, 0, i, val);
	}
	
	//Step 9: Multiply 2nd row by 2nd element in 3rd row, and subtract from 3rd row
	factor=gsl_matrix_get(mat, 2, 1);
	for(int i=0; i < mat->size2; i++){
		double val=gsl_matrix_get(mat, 2, i) - (factor * gsl_matrix_get(mat, 1, i));
		gsl_matrix_set(mat, 2, i, val);
	}
	
	//Step 10: Divide 3rd row by 3rd element
	max=gsl_matrix_get(mat, 2, 2);
	for(int i=0; i < mat->size2; i++){
		gsl_matrix_set(mat, 2, i, gsl_matrix_get(mat, 2, i)/max);
	}
	
	//Step 11: Multiply 3rd row by 3rd element in 1st row, and subtract from 1st row
	factor=gsl_matrix_get(mat, 0, 2);
	for(int i=0; i < mat->size2; i++){
		double val=gsl_matrix_get(mat, 0, i) - (factor * gsl_matrix_get(mat, 2, i));
		gsl_matrix_set(mat, 0, i, val);
	}
	
	//Step 12: Multiply 3rd row by 3rd element in 2nd row, and subtract from 2nd row
	factor=gsl_matrix_get(mat, 1, 2);
	for(int i=0; i < mat->size2; i++){
		double val=gsl_matrix_get(mat, 1, i) - (factor * gsl_matrix_get(mat, 2, i));
		gsl_matrix_set(mat, 1, i, val);
	}
	
	return true;
}

void printMatrix(gsl_matrix* mat){
	for(int row=0; row < mat->size1; row++){
		cout<<"\t|\t";
		for(int col=0; col < mat->size2; col++){
			cout<<gsl_matrix_get(mat, row, col)<<"\t";
		}
		cout<<"|"<<endl;
	}
}

void readMatrix(gsl_matrix* mat){
	for(int row=0; row < mat->size1; row++){
		for(int col=0; col < mat->size2; col++){
			cout<<"["<<row<<", "<<col<<"]"<<"=";
			double val=0.0f;
			cin>>val;
			gsl_matrix_set(mat, row, col, val);
		}
	}
}

void testSolver(){
	gsl_matrix* mat=gsl_matrix_alloc(3,4);
	
	cout<<"Enter Matrix"<<endl;
	readMatrix(mat);
	cout<<"Read Matrix:"<<endl;
	printMatrix(mat);
	
	if(solveLinearSystem(mat)){
		cout<<"Solved Matrix:"<<endl;
	}
	else{
		cout<<"ERROR - Failed to solve matrix!"<<endl;
	}
	printMatrix(mat);
	
	gsl_matrix_free(mat);
}
