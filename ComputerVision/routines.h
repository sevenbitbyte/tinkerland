#ifndef ROUTINES_H
#define ROUTINES_H

#include <vector>

using namespace std;

#define GetPixelPtrD8(i,r,c,chan) i->imageData[((r*i->widthStep)+(c*i->nChannels)+chan)]
#define GetPixelPtrD16(i,r,c,chan) i->imageData[((r*(i->widthStep/sizeof(uint16_t)))+(c*i->nChannels)+chan)]
#define GetPixelPtrD32(i,r,c,chan) ((float*)i->imageData)[((r*i->widthStep)/4+(c*i->nChannels)+chan)]

#define CALC_COVAR false

extern uint32_t targetHist[256][3];

void colorMaximization(IplImage* src, IplImage* dst){
	for(int row=0; row<src->height; row++){
		for(int col=0; col<src->width; col++){
			uint8_t chan0=(uint8_t)GetPixelPtrD8(src, row, col, 0);
			uint8_t chan1=(uint8_t)GetPixelPtrD8(src, row, col, 1);
			uint8_t chan2=(uint8_t)GetPixelPtrD8(src, row, col, 2);

			if(chan0 > chan1){
				if(chan0 > chan2){	//chan0 is brightest
					GetPixelPtrD8(dst, row, col, 1)=0;
					GetPixelPtrD8(dst, row, col, 2)=0;
				}
				else{	//chan2 is brightest
					GetPixelPtrD8(dst, row, col, 0)=0;
					GetPixelPtrD8(dst, row, col, 1)=0;
				}
			}
			else{
				if(chan1 > chan2){	//chan1 is brightest
					GetPixelPtrD8(dst, row, col, 0)=0;
					GetPixelPtrD8(dst, row, col, 2)=0;
				}
				else{	//chan2 is brightest
					GetPixelPtrD8(dst, row, col, 0)=0;
					GetPixelPtrD8(dst, row, col, 1)=0;
				}
			}

			//GetPixelPtrD8(distImg, row, col, 0) = (uint8_t)(dist);
		}
	}
}

struct PixelStats{
	PixelStats(){
		avgVal=cvScalarAll(0);
		variance=cvScalarAll(0);
		stdDev=cvScalarAll(0);
		memset((void*)coVar, 0, 3*3*sizeof(double));
	}

	CvScalar avgVal;
	CvScalar variance;
	CvScalar stdDev;
	double coVar[3][3];
};

void calcStats(IplImage* img, vector<CvPoint*> points, PixelStats* stats){
	//Calaculate average
	for(int i=0; i<points.size(); i++){
			CvPoint* pt=points.at(i);

			for(int c=0; c<img->nChannels; c++){
				stats->avgVal.val[c]+=(uint8_t)GetPixelPtrD8(img, pt->y, pt->x, c);

				if(targetHist!=NULL){
					uint8_t val=(uint8_t)GetPixelPtrD8(img, pt->y, pt->x, c);
					targetHist[val][c]++;
				}
			}
	}

	for(int c=0; c<img->nChannels; c++){
		stats->avgVal.val[c]/=points.size();
	}

	//Calculate Variance
	for(int i=0; i<points.size(); i++){
		CvPoint* pt=points.at(i);
		double distance=0.0;
		CvScalar distTemp=cvScalarAll(0);

		for(int c=0; c<img->nChannels; c++){
			distTemp.val[c]=((uint8_t)GetPixelPtrD8(img, pt->y, pt->x, c))-stats->avgVal.val[c];
			distTemp.val[c]*=distTemp.val[c];
			stats->variance.val[c]+=distTemp.val[c];
			//distance+=distTemp.val[c];
		}

#if CALC_COVAR
		stats->coVar[0][1]+=(((uint8_t)GetPixelPtrD8(img,pt->y,pt->x,0))/255) * (((uint8_t)GetPixelPtrD8(img,pt->y,pt->x,1))/255);

		stats->coVar[0][2]+=(((uint8_t)GetPixelPtrD8(img,pt->y,pt->x,0))/255) * (((uint8_t)GetPixelPtrD8(img,pt->y,pt->x,2))/255);

		stats->coVar[1][2]+=(((uint8_t)GetPixelPtrD8(img,pt->y,pt->x,1))/255) * (((uint8_t)GetPixelPtrD8(img,pt->y,pt->x,2))/255);

		stats->coVar[0][0]+=(((uint8_t)GetPixelPtrD8(img,pt->y,pt->x,0))/255) * (((uint8_t)GetPixelPtrD8(img,pt->y,pt->x,0))/255);

		stats->coVar[1][1]+=(((uint8_t)GetPixelPtrD8(img,pt->y,pt->x,1))/255) * (((uint8_t)GetPixelPtrD8(img,pt->y,pt->x,1))/255);

		stats->coVar[2][2]+=(((uint8_t)GetPixelPtrD8(img,pt->y,pt->x,2))/255) * (((uint8_t)GetPixelPtrD8(img,pt->y,pt->x,2))/255);
#endif
		//distance=sqrt(distance);
	}

#if CALC_COVAR
	stats->coVar[0][1]=(stats->coVar[0][1]/points.size())-(stats->avgVal.val[0]/255)*(stats->avgVal.val[1]/255);
	stats->coVar[1][0]=stats->coVar[0][1];

	stats->coVar[0][2]=(stats->coVar[0][2]/points.size())-(stats->avgVal.val[0]/255)*(stats->avgVal.val[2]/255);
	stats->coVar[2][0]=stats->coVar[0][2];

	stats->coVar[1][2]=(stats->coVar[1][2]/points.size())-(stats->avgVal.val[1]/255)*(stats->avgVal.val[2]/255);
	stats->coVar[2][1]=stats->coVar[1][2];

	stats->coVar[0][0]=(stats->coVar[0][0]/points.size())-(stats->avgVal.val[0]/255)*(stats->avgVal.val[0]/255);

	stats->coVar[1][1]=(stats->coVar[1][1]/points.size())-(stats->avgVal.val[1]/255)*(stats->avgVal.val[1]/255);

	stats->coVar[2][2]=(stats->coVar[2][2]/points.size())-(stats->avgVal.val[2]/255)*(stats->avgVal.val[2]/255);

	CvMat* invCoVarMat=cvCreateMat(3, 3, CV_32FC1);
	for(int i=0; i<3; i++){
		for(int j=0; j<3; j++){
			CvScalar v;
			v.val[0]=stats->coVar[i][j];
			cvSet2D(invCoVarMat, i, j, v);
		}
	}

	cvInvert(invCoVarMat, invCoVarMat, CV_SVD);

	for(int i=0; i<3; i++){
		for(int j=0; j<3; j++){
			CvScalar v=cvGet2D(invCoVarMat, i, j);
			stats->coVar[i][j]=v.val[0];
		}
	}

	cvReleaseMat(&invCoVarMat);
#endif

	for(int c=0; c<img->nChannels; c++){
		stats->variance.val[c]/=points.size();
		stats->stdDev.val[c]=sqrt(stats->variance.val[c])/2;
	}


}

#endif
