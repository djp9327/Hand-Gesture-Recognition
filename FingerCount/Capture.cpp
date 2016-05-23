#include "stdafx.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

Mat src; Mat src_gray; Mat frameDest;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345);

void thresh_callback(int, void*);

void thresh_callback1(int, void*);

//mySkinDetect - detects skin color
void mySkinDetect(Mat& src, Mat& dst);

//distanceP2P - calculates distance between two points
float distanceP2P(Point a, Point b);

//getAngle - calculates angle between three points
float getAngle(Point s, Point f, Point e);

//myMax - returns the maximum of 3 integers
int myMax(int a, int b, int c);

//myMin - returns the minimum of 3 integers
int myMin(int a, int b, int c);

/** @function main */
int main()
{
    /// Load source image and convert it to gray
    VideoCapture cap(0);
    
    // if not successful, exit program
    if (!cap.isOpened())
    {
        cout << "Cannot open the video cam" << endl;
        return -1;
    }
    
    // read a new frame from video
    bool bSuccess0 = cap.read(src);
    
    //if not successful, break loop
    if (!bSuccess0)
    {
        cout << "Cannot read a frame from video stream" << endl;
    }
    
    while (1)
    {
        // read a new frame from video
        bool bSuccess = cap.read(src);
        
        //if not successful, break loop
        if (!bSuccess)
        {
            cout << "Cannot read a frame from video stream" << endl;
            break;
        }
        
        //Mat frameDest;
        frameDest = Mat::zeros(src.rows, src.cols, CV_8UC1); //Returns a zero array of same size as src mat, and of type CV_8UC1
        
        int erosion_size = 4;
        cv::Mat element1 = cv::getStructuringElement(cv::MORPH_ERODE,
                                                     cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
                                                     cv::Point(erosion_size, erosion_size) );
        cv::Mat element2 = cv::getStructuringElement(cv::MORPH_DILATE,
                                                     cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
                                                     cv::Point(erosion_size, erosion_size) );
        erode(frameDest, frameDest, element1);
        dilate(frameDest, frameDest, element2);
        
        
        /// Create Window
        namedWindow("E/D", CV_WINDOW_AUTOSIZE );
        imshow("E/D", frameDest);
        
        //createTrackbar( " Canny thresh:", "Source", &thresh, max_thresh, thresh_callback );
        thresh_callback1( 0, 0 );
        
        
        //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        if (waitKey(30) == 27)
        {
            cout << "esc key is pressed by user" << endl;
            //break;
        }
        
    }
    cap.release();
    return 0;
}

int myMax(int a, int b, int c) {
    int m = a;
    (void)((m < b) && (m = b));
    (void)((m < c) && (m = c));
    return m;
}

int myMin(int a, int b, int c) {
    int m = a;
    (void)((m > b) && (m = b));
    (void)((m > c) && (m = c));
    return m;
}


float distanceP2P(Point a, Point b){
    float d= sqrt(fabs( pow(a.x-b.x,2) + pow(a.y-b.y,2) )) ;
    return d;
}

float getAngle(Point s, Point f, Point e){
    float l1 = distanceP2P(f,s);
    float l2 = distanceP2P(f,e);
    float dot=(s.x-f.x)*(e.x-f.x) + (s.y-f.y)*(e.y-f.y);
    float angle = acos(dot/(l1*l2));
    angle=angle*180/3.14;
    return angle;
}

//Function that detects whether a pixel belongs to the skin based on RGB values
void mySkinDetect(Mat& src, Mat& dst) {
    //Surveys of skin color modeling and detection techniques:
    //Vezhnevets, Vladimir, Vassili Sazonov, and Alla Andreeva. "A survey on pixel-based skin color detection techniques." Proc. Graphicon. Vol. 3. 2003.
    //Kakumanu, Praveen, Sokratis Makrogiannis, and Nikolaos Bourbakis. "A survey of skin-color modeling and detection methods." Pattern recognition 40.3 (2007): 1106-1122.
    for (int i = 0; i < src.rows; i++){
        for (int j = 0; j < src.cols; j++){
            //For each pixel, compute the average intensity of the 3 color channels
            Vec3b intensity = src.at<Vec3b>(i,j); //Vec3b is a vector of 3 uchar (unsigned character)
            int B = intensity[0]; int G = intensity[1]; int R = intensity[2];
            if ((R > 95 && G > 40 && B > 20) && (myMax(R,G,B) - myMin(R,G,B) > 15) && (abs(R-G) > 15) && (R > G) && (R > B)){
                dst.at<uchar>(i,j) = 255;
            }
        }
    }
}

//Function that does frame differencing between the current frame and the previous frame
void myFrameDifferencing(Mat& prev, Mat& curr, Mat& dst) {
    //For more information on operation with arrays: http://docs.opencv.org/modules/core/doc/operations_on_arrays.html
    //For more information on how to use background subtraction methods: http://docs.opencv.org/trunk/doc/tutorials/video/background_subtraction/background_subtraction.html
    absdiff(prev, curr, dst);
    Mat gs = dst.clone();
    cvtColor(dst, gs, CV_BGR2GRAY);
    dst = gs > 50;
    Vec3b intensity = dst.at<Vec3b>(100,100);
}

//Function that accumulates the frame differences for a certain number of pairs of frames
void myMotionEnergy(Vector<Mat> mh, Mat& dst) {
    Mat mh0 = mh[0];
    Mat mh1 = mh[1];
    Mat mh2 = mh[2];
    
    for (int i = 0; i < dst.rows; i++){
        for (int j = 0; j < dst.cols; j++){
            if (mh0.at<uchar>(i,j) == 255 || mh1.at<uchar>(i,j) == 255 ||mh2.at<uchar>(i,j) == 255){
                dst.at<uchar>(i,j) = 255;
            }
        }
    }
}

/** @function thresh_callback */
void thresh_callback(int, void* )
{
    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    
    /// Detect edges using canny
    Canny(frameDest, canny_output, thresh, thresh*2, 3 );
    /// Find contours
    findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    
    /// Draw contours
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for( int i = 0; i< contours.size(); i++ )
    {
        //Scalar color = Scalar(255, 255, 255);
        Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );
    }
    
    /// Show in a window
    namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
    imshow( "Contours", drawing );
}

void thresh_callback1(int, void* )
{
    Mat src_copy = src.clone();
    Mat threshold_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    
    mySkinDetect(src, frameDest);
    
    int erosion_size = 3;
    cv::Mat element1 = cv::getStructuringElement(cv::MORPH_ERODE,
                                                 cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
                                                 cv::Point(erosion_size, erosion_size) );
    cv::Mat element2 = cv::getStructuringElement(cv::MORPH_DILATE,
                                                 cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
                                                 cv::Point(erosion_size, erosion_size) );
    erode(frameDest, frameDest, element1);
    dilate(frameDest, frameDest, element2);
    
    namedWindow("Skin", CV_WINDOW_AUTOSIZE);
    //imshow("Skin", frameDest);
    
    findContours(frameDest, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    
    /// Find the convex hull object for each contour
    vector<vector<Point> >hull( contours.size() );
    for( int i = 0; i < contours.size(); i++ )
    {  convexHull( Mat(contours[i]), hull[i], false ); }
    
    /// Draw contours + hull results
    Mat drawing = Mat::zeros(frameDest.size(), CV_8UC3 );
    
    int count = 0;
    
    if(contours.size()>0)
    {
        vector<std::vector<int> >hull( contours.size() );
        vector<vector<Vec4i>> convDef(contours.size() );
        vector<vector<Point>> hull_points(contours.size());
        vector<vector<Point>> defect_points(contours.size());
        
        
        for( int i = 0; i < contours.size(); i++ )
        {
            if(contourArea(contours[i])>8000)
            {
                convexHull( contours[i], hull[i], false );
                convexityDefects( contours[i],hull[i], convDef[i]);
                
                for(int k=0;k<hull[i].size();k++)
                {
                    int ind=hull[i][k];
                    hull_points[i].push_back(contours[i][ind]);
                }
                
                for(int k=0;k<convDef[i].size();k++)
                {
                    if(convDef[i][k][3]>20*256) // filter defects by depth
                    {
                        int ind_0=convDef[i][k][0];
                        int ind_1=convDef[i][k][1];
                        int ind_2=convDef[i][k][2];
                        defect_points[i].push_back(contours[i][ind_2]);
                        cv::circle(drawing,contours[i][ind_0],5,Scalar(0,255,0),-1);
                        cv::circle(drawing,contours[i][ind_1],5,Scalar(0,255,0),-1);
                        cv::circle(drawing,contours[i][ind_2],5,Scalar(0,0,255),-1);
                        cv::line(drawing,contours[i][ind_2],contours[i][ind_0],Scalar(0,0,255),1);
                        cv::line(drawing,contours[i][ind_2],contours[i][ind_1],Scalar(0,0,255),1);
                        
                        if (getAngle(contours[i][ind_0], contours[i][ind_2], contours[i][ind_1]) > 15 && getAngle(contours[i][ind_0], contours[i][ind_2], contours[i][ind_1]) < 100){
                            count = count + 1;
                        }
                        
                    }
                }
                
                drawContours( drawing, contours, i, Scalar(255,255,255), 1, 8, vector<Vec4i>(), 0, Point() );
                drawContours( drawing, hull_points, i, Scalar(255,255,255), 1, 8, vector<Vec4i>(), 0, Point() );
            }
        }
    }
    count = count + 1;
    string counts = std::to_string(count);
    cout << count << endl;
    cv::putText(drawing, counts, cvPoint(30,30),
            FONT_HERSHEY_COMPLEX, 0.8, cvScalar(200,200,250), 1, CV_AA);
    imshow( "Hull demo", drawing );
}
