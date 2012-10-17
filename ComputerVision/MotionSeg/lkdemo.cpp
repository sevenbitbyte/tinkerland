/* Lucas-Kanade optical flow with motion Motion Segmentaion*/

#include "affineModel.h"
#include "kdnode.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/time.h>

using namespace std;

IplImage *image = 0, *grey = 0, *prev_grey = 0, *pyramid = 0, *prev_pyramid = 0, *swap_temp;

int win_size = 10;
const int MAX_COUNT = 200;
const int MIN_COUNT = 25;
CvPoint2D32f* points[4] = {0,0,0,0}, *swap_points;
char* status0 = 0;
char* status1 = 0;
int pointCount = 0;
int need_to_init = 0;
int night_mode = 0;
int flags = 0;
int add_remove_pt = 0;
CvPoint pt;


void on_mouse( int event, int x, int y, int flags, void* param )
{
    if( !image )
        return;

    if( image->origin )
        y = image->height - y;

    if( event == CV_EVENT_LBUTTONDOWN )
    {
        pt = cvPoint(x,y);
        add_remove_pt = 1;
    }
}


int main( int argc, char** argv ){
	float milSecs=0;
	timeval timeTemp;
	timeval timeDelta;
    CvCapture* capture = 0;

    if( argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0]))){
        capture = cvCaptureFromCAM( argc == 2 ? argv[1][0] - '0' : 0 );
    }
    else if( argc == 2 ){
        capture = cvCaptureFromAVI( argv[1] );
    }

    if( !capture ){
        fprintf(stderr,"Could not initialize capturing...\n");
        return -1;
    }

    /* print a welcome message, and the OpenCV version */
    printf ("Welcome to lkdemo, using OpenCV version %s (%d.%d.%d)\n",
	    CV_VERSION,
	    CV_MAJOR_VERSION, CV_MINOR_VERSION, CV_SUBMINOR_VERSION);

    printf( "Hot keys: \n"
            "\tq - quit the program\n"
            "\tr - auto-initialize tracking\n"
            "\tc - delete all the points\n"
            "\tn - switch the \"night\" mode on/off\n"
            "To add/remove a feature point click it\n" );

    cvNamedWindow( "LkDemo", 0 );
    cvSetMouseCallback( "LkDemo", on_mouse, 0 );

    while(1){
        IplImage* frame = 0;
        int i, k, c;

        frame = cvQueryFrame( capture );
        if( !frame )
            break;

        if( !image )
        {
            /* allocate all the buffers */
            image = cvCreateImage( cvGetSize(frame), 8, 3 );
            image->origin = frame->origin;
            grey = cvCreateImage( cvGetSize(frame), 8, 1 );
            prev_grey = cvCreateImage( cvGetSize(frame), 8, 1 );
            pyramid = cvCreateImage( cvGetSize(frame), 8, 1 );
            prev_pyramid = cvCreateImage( cvGetSize(frame), 8, 1 );
            points[0] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(CvPoint2D32f));
            points[1] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(CvPoint2D32f));
			points[2] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(CvPoint2D32f));
			points[3] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(CvPoint2D32f));
            status0 = (char*)cvAlloc(MAX_COUNT);
			status1 = (char*)cvAlloc(MAX_COUNT);
            flags = 0;
        }

		//Time how long it takes to process
		gettimeofday(&timeTemp, NULL);

        cvCopy( frame, image, 0 );
        cvCvtColor( image, grey, CV_BGR2GRAY );

        if( night_mode )
            cvZero( image );

        if( need_to_init || pointCount < MIN_COUNT)
        {
            /* automatic initialization */
            IplImage* eig = cvCreateImage( cvGetSize(grey), 32, 1 );
            IplImage* temp = cvCreateImage( cvGetSize(grey), 32, 1 );
            double quality = 0.03;
            double min_distance = 10;

            pointCount = MAX_COUNT;
            cvGoodFeaturesToTrack( grey, eig, temp, points[1], &pointCount,
                                   quality, min_distance, 0, 3, 0, 0.04 );
            cvFindCornerSubPix( grey, points[1], pointCount,
                cvSize(win_size,win_size), cvSize(-1,-1),
                cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
            cvReleaseImage( &eig );
            cvReleaseImage( &temp );

            add_remove_pt = 0;
        }
        else if( pointCount > 0 )
        {
            cvCalcOpticalFlowPyrLK( prev_grey, grey, prev_pyramid, pyramid,
                points[0], points[1], pointCount, cvSize(win_size,win_size), 3, status0, 0,
                cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), flags );
            flags |= CV_LKFLOW_PYR_A_READY;
            
            vector<CvPoint2D32f*> pointsVect;

            for( i = k = 0; i < pointCount; i++ ){
                if( add_remove_pt ){
                    double dx = pt.x - points[1][i].x;
                    double dy = pt.y - points[1][i].y;

                    if( dx*dx + dy*dy <= 25 )
                    {
                        add_remove_pt = 0;
                        continue;
                    }
                }

                if( !status0[i] )
                    continue;

				pointsVect.push_back(&points[1][i]);
                points[1][k++] = points[1][i];
                
                cvCircle( image, cvPointFrom32f(points[1][i]), 3, CV_RGB(0,255,0), -1, 8,0);
				cvLine( image, cvPointFrom32f(points[0][i]), cvPointFrom32f(points[1][i]), CV_RGB(255,0,0), 1, 8, 0);
            }
            pointCount = k;
            
            cout<<"Found "<<k<<" features vector has "<<pointsVect.size()<<endl;
			KDNode* root=KDNode::buildTree(pointsVect);
			if(root!=NULL){
				cout<<"deleted "<<endl;
				drawNodes(image, root, 4);
				delete root;
			}
			else{
				cout<<"NULL KDTree! "<<endl;
			}
        }

        if( add_remove_pt && pointCount < MAX_COUNT )
        {
            points[1][pointCount++] = cvPointTo32f(pt);
            cvFindCornerSubPix( grey, points[1] + pointCount - 1, 1,
                cvSize(win_size,win_size), cvSize(-1,-1),
                cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
            add_remove_pt = 0;
        }

        CV_SWAP( prev_grey, grey, swap_temp );
        CV_SWAP( prev_pyramid, pyramid, swap_temp );
        CV_SWAP( points[0], points[1], swap_points );
        need_to_init = 0;
        
        /********* END PROCESSING *******/
        gettimeofday(&timeDelta, NULL);
        timeDelta.tv_sec-=timeTemp.tv_sec;
        timeDelta.tv_usec-=timeTemp.tv_usec;
        milSecs=timeDelta.tv_sec*1000 + timeDelta.tv_usec/1000;
        cout<<" - Processed frame in "<<milSecs<<"ms."<<endl;
        
        cvShowImage( "LkDemo", image );

        c = cvWaitKey(5);
        if( (char)c == 'q' ){
            break;
        }
        
        switch( (char) c )
        {
			case 'r':
				need_to_init = 1;
				break;
			case 'c':
				pointCount = 0;
				break;
			case 'n':
				night_mode ^= 1;
				break;
			default:
				break;
        }
    }

    cvReleaseCapture( &capture );
    cvDestroyWindow("LkDemo");

    return 0;
}
