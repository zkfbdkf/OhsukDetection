#pragma warning(disable : 4996)
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <string> 
#include <time.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv\cv.h>
#include <cstdlib>
#include <iostream>
#include <conio.h>           // it may be necessary to change or remove this line if not using Windows
#include "DBConnection.cpp"
#include "Blob.h"

using namespace std;
using namespace cv;
#pragma comment(lib, "mysqlcppconn.lib")
// global variables ///////////////////////////////////////////////////////////////////////////////
const Scalar SCALAR_BLACK = Scalar(0.0, 0.0, 0.0);
const Scalar SCALAR_WHITE = Scalar(255.0, 255.0, 255.0);
const Scalar SCALAR_YELLOW = Scalar(0.0, 255.0, 255.0);
const Scalar SCALAR_GREEN = Scalar(0.0, 200.0, 0.0);
const Scalar SCALAR_RED = Scalar(0.0, 0.0, 255.0);

sql::Statement *stmt;
sql::Connection *conn;

// function prototypes ////////////////////////////////////////////////////////////////////////////
void matchCurrentFrameBlobsToExistingBlobs(vector<Blob> &existingBlobs, vector<Blob> &currentFrameBlobs);
void addBlobToExistingBlobs(Blob &currentFrameBlob, vector<Blob> &existingBlobs, int &intIndex);
void addNewBlob(Blob &currentFrameBlob, vector<Blob> &existingBlobs);
double distanceBetweenPoints(Point point1, Point point2);
bool checkIfBlobsCrossedTheLine(vector<Blob> &blobs, int &intHorizontalLinePosition, int &peopleCount);
void drawpeopleCountOnImage(int &peopleCount, Mat &imgFrame2Copy);
void gettime(struct time *);
// for library//////////////////////////////////////////////////////////////////////////////////
Rect bounding_rect;
CvMemStorage* storage = NULL;
CvMemStorage* storage1[13] = { NULL };
CvSeq* contours = 0;
CvSeq* contours1[13]  {0};
CvRect rect;
IplImage* grayImage1[13];
IplImage* diffImage1[13];
IplImage* bkgImage1[13];
IplImage* copy1;
// for enterance//////////////////////////////////////////////////////////////////////////////////
Mat imgFrame1;
Mat imgFrame2;
Mat imgFrame3;
Mat imgFrame4;
vector<Blob> blobs;

struct time {
	int ti_hour;
	int ti_min;
	int ti_sec;
};


int main(void) {
	DBConnection dbConn;
	conn = dbConn.getConn();

	int closeSeat[13] = { 0 };
	int countSeat[13] = { 0 };
	int checkSeat[13] = { 0 };
	int saveSeatdata[13] = { 0 };
	String saveSeattime[13] = { "0" };

	int save = 0;
	int curTime = 99;
	clock_t begin, end;
	string info, timeinfo, sti, smi, ssec;
	String passAns;

	struct time sttime;

	VideoCapture capVideo;
	VideoCapture capVideo2;

	Point crossingLine[2];

	int peopleCount = 0;

	//capVideo.open("[VAP]출입자2.avi");

	if (!capVideo.isOpened()) {                                                 // if unable to open video file
		cout << "error reading video file" << endl << endl;      // show error message
		return(0);
	}

	if (capVideo.get(CV_CAP_PROP_FRAME_COUNT) < 2) {
		cout << "error: video file must have at least two frames";
		return(0);
	}
	capVideo.read(imgFrame1);
	capVideo.read(imgFrame2);

	int intHorizontalLinePosition = (int)round((double)imgFrame1.rows * 0.35);  //draw line

	crossingLine[0].x = 0;
	crossingLine[0].y = intHorizontalLinePosition;

	crossingLine[1].x = imgFrame1.cols - 1;
	crossingLine[1].y = intHorizontalLinePosition;

	bool blnFirstFrame = true;

	// initialize storage
	storage = cvCreateMemStorage(0);
	for (int i = 0; i < 13; i++){
		storage1[i] = cvCreateMemStorage(0);
	}

	CvCapture * capture = cvCaptureFromFile("열람실1.avi");

	if (!capture)    {
		cout << "The video file was not found" << endl;
		return 0;
	}
	int width = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
	int height = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
	CvSize size = cvSize(width, height);

	IplImage *bkgImage = cvCreateImage(size, IPL_DEPTH_8U, 1);
	cvGrabFrame(capture);
	IplImage *temp = cvRetrieveFrame(capture);
	cvCvtColor(temp, bkgImage, CV_BGR2GRAY);

	if (bkgImage == NULL)    return -1;

	IplImage *grayImage = cvCreateImage(size, IPL_DEPTH_8U, 1);
	IplImage *diffImage = cvCreateImage(size, IPL_DEPTH_8U, 1);
	IplImage *frame = NULL;

	int t = 0;    // frame count
	int nThreshold = 50;

	begin = clock();
	// 시간설정

	while (1)   {
		frame = cvQueryFrame(capture);

		if (!frame){
			system("pause");
			break;
		}

		// cvCvtColor 함수를 사용하여 cvQueryFrame 함수로 획득한 frame을 그레이 스케일 영상으로 변환하여 grayImage에 저장한다.
		cvCvtColor(frame, grayImage, CV_BGR2GRAY);
		cvAbsDiff(grayImage, bkgImage, diffImage);

		cvThreshold(diffImage, diffImage, 35, 255, CV_THRESH_BINARY);
		cvErode(diffImage, diffImage, 0, 5);
		cvDilate(diffImage, diffImage, 0, 5);


		//set ROI
		grayImage1[0] = (IplImage*)cvClone(grayImage);
		cvSetImageROI(grayImage1[0], cvRect(00, 390, 100, 90));
		diffImage1[0] = (IplImage*)cvClone(diffImage);
		cvSetImageROI(diffImage1[0], cvRect(00, 390, 100, 90));
		bkgImage1[0] = (IplImage*)cvClone(bkgImage);
		cvSetImageROI(bkgImage1[0], cvRect(00, 390, 100, 90));

		grayImage1[1] = (IplImage*)cvClone(grayImage);
		cvSetImageROI(grayImage1[1], cvRect(60, 260, 110, 100));
		diffImage1[1] = (IplImage*)cvClone(diffImage);
		cvSetImageROI(diffImage1[1], cvRect(60, 260, 110, 100));
		bkgImage1[1] = (IplImage*)cvClone(bkgImage);
		cvSetImageROI(bkgImage1[1], cvRect(60, 260, 110, 100));

		grayImage1[2] = (IplImage*)cvClone(grayImage);
		cvSetImageROI(grayImage1[2], cvRect(155, 200, 120, 100));
		diffImage1[2] = (IplImage*)cvClone(diffImage);
		cvSetImageROI(diffImage1[2], cvRect(155, 200, 120, 100));
		bkgImage1[2] = (IplImage*)cvClone(bkgImage);
		cvSetImageROI(bkgImage1[2], cvRect(155, 200, 120, 100));

		grayImage1[3] = (IplImage*)cvClone(grayImage);
		cvSetImageROI(grayImage1[3], cvRect(233, 130, 100, 90));
		diffImage1[3] = (IplImage*)cvClone(diffImage);
		cvSetImageROI(diffImage1[3], cvRect(233, 130, 100, 90));
		bkgImage1[3] = (IplImage*)cvClone(bkgImage);
		cvSetImageROI(bkgImage1[3], cvRect(233, 130, 100, 90));

		grayImage1[4] = (IplImage*)cvClone(grayImage);
		cvSetImageROI(grayImage1[4], cvRect(345, 80, 55, 80));
		diffImage1[4] = (IplImage*)cvClone(diffImage);
		cvSetImageROI(diffImage1[4], cvRect(345, 80, 55, 80));
		bkgImage1[4] = (IplImage*)cvClone(bkgImage);
		cvSetImageROI(bkgImage1[4], cvRect(345, 80, 55, 80));

		grayImage1[5] = (IplImage*)cvClone(grayImage);
		cvSetImageROI(grayImage1[5], cvRect(400, 50, 48, 60));
		diffImage1[5] = (IplImage*)cvClone(diffImage);
		cvSetImageROI(diffImage1[5], cvRect(400, 50, 48, 60));
		bkgImage1[5] = (IplImage*)cvClone(bkgImage);
		cvSetImageROI(bkgImage1[5], cvRect(400, 50, 48, 60));

		grayImage1[6] = (IplImage*)cvClone(grayImage);
		cvSetImageROI(grayImage1[6], cvRect(440, 35, 48, 60));
		diffImage1[6] = (IplImage*)cvClone(diffImage);
		cvSetImageROI(diffImage1[6], cvRect(440, 35, 48, 60));
		bkgImage1[6] = (IplImage*)cvClone(bkgImage);
		cvSetImageROI(bkgImage1[6], cvRect(440, 35, 48, 60));

		grayImage1[7] = (IplImage*)cvClone(grayImage);
		cvSetImageROI(grayImage1[7], cvRect(492, 0, 60, 52));
		diffImage1[7] = (IplImage*)cvClone(diffImage);
		cvSetImageROI(diffImage1[7], cvRect(492, 0, 60, 52));
		bkgImage1[7] = (IplImage*)cvClone(bkgImage);
		cvSetImageROI(bkgImage1[7], cvRect(492, 0, 60, 52));

		grayImage1[8] = (IplImage*)cvClone(grayImage);
		diffImage1[8] = (IplImage*)cvClone(diffImage);
		bkgImage1[8] = (IplImage*)cvClone(bkgImage);

		grayImage1[9] = (IplImage*)cvClone(grayImage);
		cvSetImageROI(grayImage1[9], cvRect(566, 115, 65, 60));
		diffImage1[9] = (IplImage*)cvClone(diffImage);
		cvSetImageROI(diffImage1[9], cvRect(566, 115, 65, 60));
		bkgImage1[9] = (IplImage*)cvClone(bkgImage);
		cvSetImageROI(bkgImage1[9], cvRect(566, 115, 65, 60));

		grayImage1[10] = (IplImage*)cvClone(grayImage);
		cvSetImageROI(grayImage1[10], cvRect(515, 178, 65, 55));
		diffImage1[10] = (IplImage*)cvClone(diffImage);
		cvSetImageROI(diffImage1[10], cvRect(515, 178, 65, 55));
		bkgImage1[10] = (IplImage*)cvClone(bkgImage);
		cvSetImageROI(bkgImage1[10], cvRect(515, 178, 65, 55));

		grayImage1[11] = (IplImage*)cvClone(grayImage);
		cvSetImageROI(grayImage1[11], cvRect(455, 230, 80, 70));
		diffImage1[11] = (IplImage*)cvClone(diffImage);
		cvSetImageROI(diffImage1[11], cvRect(455, 230, 80, 70));
		bkgImage1[11] = (IplImage*)cvClone(bkgImage);
		cvSetImageROI(bkgImage1[11], cvRect(455, 230, 80, 70));

		grayImage1[12] = (IplImage*)cvClone(grayImage);
		cvSetImageROI(grayImage1[12], cvRect(363, 315, 100, 90));
		diffImage1[12] = (IplImage*)cvClone(diffImage);
		cvSetImageROI(diffImage1[12], cvRect(363, 315, 100, 90));
		bkgImage1[12] = (IplImage*)cvClone(bkgImage);
		cvSetImageROI(bkgImage1[12], cvRect(363, 315, 100, 90));


		// cvAbsDiff 함수로 현재의 입력 비디오 프레임과 그레이 스케일 영상인 grayImage와 배경 영상인 bkgImage와의 차이의 절대값을 계산하여 diffImage에 저장한다.

		for (int i = 0; i < 13; i++){
			cvAbsDiff(grayImage1[i], bkgImage1[i], diffImage1[i]);
		}

		// diffImage에서 0인 화소는 변화가 없는 화소이며, 값이 크면 클수록 배경 영상과의 차이가 크게 일어난 화소이다.
		// cvThreshold 함수를 사용하여 cvThreshold=50 이상인 화소만을 255로 저장하고, 임계값 이하인 값은 0으로 저장한다.
		// 임계값은 실험 또는 자동으로 적절히 결정해야 한다.

		for (int i = 0; i < 4; i++){
			cvThreshold(diffImage1[i], diffImage1[i], 40, 255, CV_THRESH_BINARY);
			cvErode(diffImage1[i], diffImage1[i], 0, 5);
			cvDilate(diffImage1[i], diffImage1[i], 0, 6);
			cvFindContours(diffImage1[i], storage1[i], &contours1[i]);
		}
		cvThreshold(diffImage1[4], diffImage1[4], 25, 255, CV_THRESH_BINARY);
		cvErode(diffImage1[4], diffImage1[4], 0, 5);
		cvDilate(diffImage1[4], diffImage1[4], 0, 6);
		cvFindContours(diffImage1[4], storage1[4], &contours1[4]);

		cvThreshold(diffImage1[5], diffImage1[5], 40, 255, CV_THRESH_BINARY);
		cvErode(diffImage1[5], diffImage1[5], 0, 5);
		cvDilate(diffImage1[5], diffImage1[5], 0, 6);
		cvFindContours(diffImage1[5], storage1[5], &contours1[5]);

		cvThreshold(diffImage1[6], diffImage1[6], nThreshold, 255, CV_THRESH_BINARY);
		cvErode(diffImage1[6], diffImage1[6], 0, 5);
		cvDilate(diffImage1[6], diffImage1[6], 0, 5);
		cvFindContours(diffImage1[6], storage, &contours1[6]);

		cvThreshold(diffImage1[7], diffImage1[7], 25, 255, CV_THRESH_BINARY);
		cvErode(diffImage1[7], diffImage1[7], 0, 05);
		cvDilate(diffImage1[7], diffImage1[7], 0, 05);
		cvFindContours(diffImage1[7], storage, &contours1[7]);

		cvThreshold(diffImage1[8], diffImage1[8], 40, 255, CV_THRESH_BINARY);
		cvErode(diffImage1[8], diffImage1[8], 0, 5);
		cvDilate(diffImage1[8], diffImage1[8], 0, 6);
		cvFindContours(diffImage1[8], storage1[8], &contours1[8]);

		cvThreshold(diffImage1[9], diffImage1[9], 30, 255, CV_THRESH_BINARY);
		cvErode(diffImage1[9], diffImage1[9], 0, 5);
		cvDilate(diffImage1[9], diffImage1[9], 0, 6);
		cvFindContours(diffImage1[9], storage1[9], &contours1[9]);

		cvThreshold(diffImage1[10], diffImage1[10], 35, 255, CV_THRESH_BINARY);
		cvErode(diffImage1[10], diffImage1[10], 0, 5);
		cvDilate(diffImage1[10], diffImage1[10], 0, 5);
		cvFindContours(diffImage1[10], storage1[10], &contours1[10]);

		cvThreshold(diffImage1[11], diffImage1[11], 40, 255, CV_THRESH_BINARY);
		cvErode(diffImage1[11], diffImage1[11], 0, 7);
		cvDilate(diffImage1[11], diffImage1[11], 0, 7);
		cvFindContours(diffImage1[11], storage1[11], &contours1[11]);

		cvThreshold(diffImage1[12], diffImage1[12], 37, 255, CV_THRESH_BINARY);
		cvErode(diffImage1[12], diffImage1[12], 0, 7);
		cvDilate(diffImage1[12], diffImage1[12], 0, 7);
		cvFindContours(diffImage1[12], storage1[12], &contours1[12]);

		if (contours1[0] == 0 && checkSeat[0] == 1)
		{
			closeSeat[0]++;
			if (contours1[0] == 0 && closeSeat[0] > 20){
				checkSeat[0] = 0;
				countSeat[0] = 0;
				closeSeat[0] = 0;
			}
		}
		else{
			if (contours1[0] == 0 && countSeat[0] > 0)
			{
				countSeat[0] --;
				checkSeat[0] = 0;
			}
			else{
				countSeat[0]++;
			}
			if (countSeat[0] > 100 && countSeat[0] < 200){
				checkSeat[0] = 2;
			}
			else if (countSeat[0] > 200){
				checkSeat[0] = 1;
			}
			if (checkSeat[0] == 1)
				cout << "personDetect 0" << endl;
			else{
			}
		}
		for (; contours1[0] != 0; contours1[0] = contours1[0]->h_next)
		{
			rect = cvBoundingRect(contours1[0], 0); //extract bounding box for current contour

			//drawing rectangle
			cvRectangle(grayImage,
				cvPoint(00 + rect.x, 390 + rect.y),
				cvPoint(00 + rect.x + rect.width, 390 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);
			cvRectangle(frame,
				cvPoint(00 + rect.x, 390 + rect.y),
				cvPoint(00 + rect.x + rect.width, 390 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);
		}

		if (contours1[1] == 0 && checkSeat[1] == 1)
		{
			closeSeat[1]++;
			if (contours1[1] == 0 && closeSeat[1] > 20){
				checkSeat[1] = 0;
				countSeat[1] = 0;
				closeSeat[1] = 0;
			}
		}
		else{
			if (contours1[1] == 0 && countSeat[1] > 0)
			{
				countSeat[1] --;
				checkSeat[1] = 0;
			}
			else{
				countSeat[1]++;
			}
			if (countSeat[1] > 100 && countSeat[1] < 200){
				checkSeat[1] = 2;
			}
			else if (countSeat[1] > 200){
				checkSeat[1] = 1;
			}
			if (checkSeat[1] == 1)
				cout << "personDetect 1" << endl;
			else{
			}
		}
		for (; contours1[1] != 0; contours1[1] = contours1[1]->h_next)
		{
			rect = cvBoundingRect(contours1[1], 0); //extract bounding box for current contour

			//drawing rectangle
			cvRectangle(grayImage,
				cvPoint(60 + rect.x, 240 + rect.y),
				cvPoint(60 + rect.x + rect.width, 240 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);
			cvRectangle(frame,
				cvPoint(60 + rect.x, 240 + rect.y),
				cvPoint(60 + rect.x + rect.width, 240 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);

		}

		if (contours1[2] == 0 && checkSeat[2] == 1)
		{
			closeSeat[2]++;
			if (contours1[2] == 0 && closeSeat[2] > 20){
				checkSeat[2] = 0;
				countSeat[2] = 0;
				closeSeat[2] = 0;
			}
		}
		else{
			if (contours1[2] == 0 && countSeat[2] > 0)
			{
				countSeat[2] --;
				checkSeat[2] = 0;
			}
			else{
				countSeat[2]++;
			}
			if (countSeat[2] > 100 && countSeat[2] < 200){
				checkSeat[2] = 2;
			}
			else if (countSeat[2] > 200){
				checkSeat[2] = 1;
			}
			if (checkSeat[2] == 1)
				cout << "personDetect 2" << endl;
			else{
			}
		}
		for (; contours1[2] != 0; contours1[2] = contours1[2]->h_next)
		{
			rect = cvBoundingRect(contours1[2], 0); //extract bounding box for current contour

			//drawing rectangle
			cvRectangle(grayImage,
				cvPoint(155 + rect.x, 200 + rect.y),
				cvPoint(155 + rect.x + rect.width, 200 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);
			cvRectangle(frame,
				cvPoint(155 + rect.x, 200 + rect.y),
				cvPoint(155 + rect.x + rect.width, 200 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);

		}

		if (contours1[3] == 0 && checkSeat[3] == 1)
		{
			closeSeat[3]++;
			if (contours1[3] == 0 && closeSeat[3] > 20){
				checkSeat[3] = 0;
				countSeat[3] = 0;
				closeSeat[3] = 0;
			}
		}
		else{
			if (contours1[3] == 0 && countSeat[3] > 0)
			{
				countSeat[3] --;
				checkSeat[3] = 0;
			}
			else{
				countSeat[3]++;
			}
			if (countSeat[3] > 70 && countSeat[3] < 170){
				checkSeat[3] = 2;
			}
			else if (countSeat[3] > 170){
				checkSeat[3] = 1;
			}
			if (checkSeat[3] == 1)
				cout << "personDetect 3" << endl;
			else{
			}
		}
		for (; contours1[3] != 0; contours1[3] = contours1[3]->h_next)
		{
			rect = cvBoundingRect(contours1[3], 0); //extract bounding box for current contour

			//drawing rectangle
			cvRectangle(grayImage,
				cvPoint(233 + rect.x, 130 + rect.y),
				cvPoint(233 + rect.x + rect.width, 130 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);
			cvRectangle(frame,
				cvPoint(233 + rect.x, 130 + rect.y),
				cvPoint(233 + rect.x + rect.width, 130 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);

		}

		if (contours1[4] == 0 && checkSeat[4] == 1)
		{
			closeSeat[4]++;
			if (contours1[4] == 0 && closeSeat[4] > 20){
				checkSeat[4] = 0;
				countSeat[4] = 0;
				closeSeat[4] = 0;
			}
		}
		else{
			if (contours1[4] == 0 && countSeat[4] > 0)
			{
				countSeat[4] --;
				checkSeat[4] = 0;
			}
			else{
				countSeat[4]++;
			}
			if (countSeat[4] > 80 && countSeat[4] < 170){
				checkSeat[4] = 2;
			}
			else if (countSeat[4] > 170){
				checkSeat[4] = 1;
			}
			if (checkSeat[4] == 1)
				cout << "personDetect 4" << endl;
			else{
			}
		}
		for (; contours1[4] != 0; contours1[4] = contours1[4]->h_next)
		{
			rect = cvBoundingRect(contours1[4], 0); //extract bounding box for current contour

			//drawing rectangle
			cvRectangle(grayImage,
				cvPoint(345 + rect.x, 80 + rect.y),
				cvPoint(345 + rect.x + rect.width, 80 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);
			cvRectangle(frame,
				cvPoint(345 + rect.x, 80 + rect.y),
				cvPoint(345 + rect.x + rect.width, 80 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);

		}

		if (contours1[5] == 0 && checkSeat[5] == 1)
		{
			closeSeat[5]++;
			if (contours1[5] == 0 && closeSeat[5] > 20){
				checkSeat[5] = 0;
				countSeat[5] = 0;
				closeSeat[5] = 0;
			}
		}
		else{
			if (contours1[5] == 0 && countSeat[5] > 0)
			{
				countSeat[5] --;
				checkSeat[5] = 0;
			}
			else{
				countSeat[5]++;
			}
			if (countSeat[5] > 100 && countSeat[5] < 200){
				checkSeat[5] = 2;
			}
			else if (countSeat[5] > 200){
				checkSeat[5] = 1;
			}
			if (checkSeat[5] == 1)
				cout << "personDetect 5" << endl;
			else{
			}
		}
		for (; contours1[5] != 0; contours1[5] = contours1[5]->h_next)
		{
			rect = cvBoundingRect(contours1[5], 0); //extract bounding box for current contour
			//drawing rectangle
			cvRectangle(grayImage,
				cvPoint(400 + rect.x, 50 + rect.y),
				cvPoint(400 + rect.x + rect.width, 50 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);
			cvRectangle(frame,
				cvPoint(400 + rect.x, 50 + rect.y),
				cvPoint(400 + rect.x + rect.width, 50 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);

		}

		if (contours1[6] == 0 && checkSeat[6] == 1)
		{
			closeSeat[6]++;
			if (contours1[6] == 0 && closeSeat[6] > 20){
				checkSeat[6] = 0;
				countSeat[6] = 0;
				closeSeat[6] = 0;
			}
		}
		else{
			if (contours1[6] == 0 && countSeat[6] > 0)
			{
				countSeat[6] --;
				checkSeat[6] = 0;
			}
			else{
				countSeat[6]++;
			}
			if (countSeat[6] > 70 && countSeat[6] < 180){
				checkSeat[6] = 2;
			}
			else if (countSeat[6] > 180){
				checkSeat[6] = 1;
			}
			if (checkSeat[6] == 1)
				cout << "personDetect 6" << endl;
			else{
			}
		}
		for (; contours1[6] != 0; contours1[6] = contours1[6]->h_next)
		{
			rect = cvBoundingRect(contours1[6], 0); //extract bounding box for current contour
			//drawing rectangle
			cvRectangle(grayImage,
				cvPoint(440 + rect.x, 35 + rect.y),
				cvPoint(440 + rect.x + rect.width, 35 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);
			cvRectangle(frame,
				cvPoint(440 + rect.x, 35 + rect.y),
				cvPoint(440 + rect.x + rect.width, 35 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);

		}

		if (contours1[7] == 0 && checkSeat[7] == 1)
		{
			closeSeat[7]++;
			if (contours1[7] == 0 && closeSeat[7] > 20){
				checkSeat[7] = 0;
				countSeat[7] = 0;
				closeSeat[7] = 0;
			}
		}
		else{
			if (contours1[7] == 0 && countSeat[7] > 0)
			{
				countSeat[7] --;
				checkSeat[7] = 0;
			}
			else{
				countSeat[7]++;
			}
			if (countSeat[7] > 100 && countSeat[7] < 220){
				checkSeat[7] = 2;
			}
			else if (countSeat[7] > 220){
				checkSeat[7] = 1;
			}
			if (checkSeat[7] == 1)
				cout << "personDetect 7" << endl;
			else{
			}
		}
		for (; contours1[7] != 0; contours1[7] = contours1[7]->h_next)
		{
			rect = cvBoundingRect(contours1[7], 0); //extract bounding box for current contour
			//drawing rectangle
			cvRectangle(grayImage,
				cvPoint(493 + rect.x, 0 + rect.y),
				cvPoint(493 + rect.x + rect.width, 0 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);
			cvRectangle(frame,
				cvPoint(493 + rect.x, 0 + rect.y),
				cvPoint(493 + rect.x + rect.width, 0 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);

		}

		if (contours1[9] == 0 && checkSeat[9] == 1)
		{
			closeSeat[9]++;
			if (contours1[9] == 0 && closeSeat[9] > 20){
				checkSeat[9] = 0;
				countSeat[9] = 0;
				closeSeat[9] = 0;
			}
		}
		else{
			if (contours1[9] == 0 && countSeat[9] > 0)
			{
				countSeat[9] --;
				checkSeat[9] = 0;
			}
			else{
				countSeat[9]++;
			}
			if (countSeat[9] > 70 && countSeat[9] < 170){
				checkSeat[9] = 2;
			}
			else if (countSeat[9] > 170){
				checkSeat[9] = 1;
			}
			if (checkSeat[9] == 1)
				cout << "personDetect 9" << endl;
			else{
			}
		}
		for (; contours1[9] != 0; contours1[9] = contours1[9]->h_next)
		{
			rect = cvBoundingRect(contours1[9], 0); //extract bounding box for current contour

			//drawing rectangle
			cvRectangle(grayImage,
				cvPoint(566 + rect.x, 115 + rect.y),
				cvPoint(566 + rect.x + rect.width, 115 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);
			cvRectangle(frame,
				cvPoint(566 + rect.x, 115 + rect.y),
				cvPoint(566 + rect.x + rect.width, 115 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);

		}

		if (contours1[10] == 0 && checkSeat[10] == 1)
		{
			closeSeat[10]++;
			if (contours1[10] == 0 && closeSeat[10] > 20){
				checkSeat[10] = 0;
				countSeat[10] = 0;
				closeSeat[10] = 0;
			}
		}
		else{
			if (contours1[10] == 0 && countSeat[10] > 0)
			{
				countSeat[10] --;
				checkSeat[10] = 0;
			}
			else{
				countSeat[10]++;
			}
			if (countSeat[10] > 100 && countSeat[10] < 200){
				checkSeat[10] = 2;
			}
			else if (countSeat[10] > 200){
				checkSeat[10] = 1;
			}
			if (checkSeat[10] == 1)
				cout << "personDetect 10" << endl;
			else{
			}
		}
		for (; contours1[10] != 0; contours1[10] = contours1[10]->h_next)
		{
			rect = cvBoundingRect(contours1[10], 0); //extract bounding box for current contour

			//drawing rectangle
			cvRectangle(grayImage,
				cvPoint(515 + rect.x, 178 + rect.y),
				cvPoint(515 + rect.x + rect.width, 178 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);
			cvRectangle(frame,
				cvPoint(515 + rect.x, 178 + rect.y),
				cvPoint(515 + rect.x + rect.width, 178 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);

		}

		if (contours1[11] == 0 && checkSeat[11] == 1)
		{
			closeSeat[11]++;
			if (contours1[11] == 0 && closeSeat[11] > 20){
				checkSeat[11] = 0;
				countSeat[11] = 0;
				closeSeat[11] = 0;
			}
		}
		else{
			if (contours1[11] == 0 && countSeat[11] > 0)
			{
				countSeat[11] --;
				checkSeat[11] = 0;
			}
			else{
				countSeat[11]++;
			}
			if (countSeat[11] > 80 && countSeat[11] < 170){
				checkSeat[11] = 2;
			}
			else if (countSeat[11] > 170){
				checkSeat[11] = 1;
			}
			if (checkSeat[11] == 1)
				cout << "personDetect 11" << endl;
			else{
			}
		}
		for (; contours1[11] != 0; contours1[11] = contours1[11]->h_next)
		{
			rect = cvBoundingRect(contours1[11], 0); //extract bounding box for current contour

			//drawing rectangle
			cvRectangle(grayImage,
				cvPoint(455 + rect.x, 230 + rect.y),
				cvPoint(455 + rect.x + rect.width, 230 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);
			cvRectangle(frame,
				cvPoint(455 + rect.x, 230 + rect.y),
				cvPoint(455 + rect.x + rect.width, 230 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);

		}

		if (contours1[12] == 0 && checkSeat[12] == 1)
		{
			closeSeat[12]++;
			if (contours1[12] == 0 && closeSeat[12] > 30){
				checkSeat[12] = 0;
				countSeat[12] = 0;
				closeSeat[12] = 0;
			}
		}
		else{
			if (contours1[12] == 0 && countSeat[12] > 0)
			{
				countSeat[12] --;
				checkSeat[12] = 0;
			}
			else{
				countSeat[12]++;
			}
			if (countSeat[12] > 100 && countSeat[12] < 200){
				checkSeat[12] = 2;
			}
			else if (countSeat[12] > 200){
				checkSeat[12] = 1;
			}
			if (checkSeat[12] == 1)
				cout << "personDetect 12" << endl;
			else{
			}
		}
		for (; contours1[12] != 0; contours1[12] = contours1[12]->h_next)
		{
			rect = cvBoundingRect(contours1[12], 0); //extract bounding box for current contour

			//drawing rectangle
			cvRectangle(grayImage,
				cvPoint(363 + rect.x, 315 + rect.y),
				cvPoint(363 + rect.x + rect.width, 315 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);
			cvRectangle(frame,
				cvPoint(363 + rect.x, 315 + rect.y),
				cvPoint(363 + rect.x + rect.width, 315 + rect.y + rect.height),
				cvScalar(0, 0, 255, 0),
				2, 8, 0);

		}

		cvShowImage("grayImage", grayImage);
		

		//cvShowImage("diffImage", diffImage);

		for (int i = 0; i < 13; i++){
			cvReleaseImage(&bkgImage1[i]);
			cvReleaseImage(&grayImage1[i]);
			cvReleaseImage(&diffImage1[i]);
		}

		end = clock();

		vector<Blob> currentFrameBlobs;

		Mat imgFrame1Copy = imgFrame1.clone();
		Mat imgFrame2Copy = imgFrame2.clone();

		Mat imgDifference;
		Mat imgThresh;

		cvtColor(imgFrame1Copy, imgFrame1Copy, CV_BGR2GRAY);
		cvtColor(imgFrame2Copy, imgFrame2Copy, CV_BGR2GRAY);

		GaussianBlur(imgFrame1Copy, imgFrame1Copy, Size(5, 5), 0);
		GaussianBlur(imgFrame2Copy, imgFrame2Copy, Size(5, 5), 0);

		absdiff(imgFrame1Copy, imgFrame2Copy, imgDifference);
		threshold(imgDifference, imgThresh, 30, 255.0, CV_THRESH_BINARY);

		Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));

		for (unsigned int i = 0; i < 2; i++) {

			dilate(imgThresh, imgThresh, element);
			dilate(imgThresh, imgThresh, element);
			erode(imgThresh, imgThresh, element);
		}

		Mat imgThreshCopy = imgThresh.clone();

		vector<vector<Point> > contours;

		findContours(imgThreshCopy, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		vector<vector<Point> > convexHulls(contours.size());

		for (unsigned int i = 0; i < contours.size(); i++) {
			convexHull(contours[i], convexHulls[i]);
		}

		for (auto &convexHull : convexHulls) {
			Blob possibleBlob(convexHull);

			if (possibleBlob.currentBoundingRect.area() > 15000 &&
				possibleBlob.dblCurrentAspectRatio > 0.2 &&
				possibleBlob.dblCurrentAspectRatio < 4.0 &&
				possibleBlob.currentBoundingRect.width > 30 &&
				possibleBlob.currentBoundingRect.height > 30 &&
				possibleBlob.dblCurrentDiagonalSize > 60.0 &&
				(contourArea(possibleBlob.currentContour) / (double)possibleBlob.currentBoundingRect.area()) > 0.50) {
				currentFrameBlobs.push_back(possibleBlob);
			}
		}

		if (blnFirstFrame == true) {
			for (auto &currentFrameBlob : currentFrameBlobs) {
				blobs.push_back(currentFrameBlob);
			}
		}
		else {
			matchCurrentFrameBlobsToExistingBlobs(blobs, currentFrameBlobs);
		}

		imgFrame2Copy = imgFrame2.clone();          // get another copy of frame 2 since we changed the previous frame 2 copy in the processing above

		bool blnAtLeastOneBlobCrossedTheLine = checkIfBlobsCrossedTheLine(blobs, intHorizontalLinePosition, peopleCount);

		if (blnAtLeastOneBlobCrossedTheLine == true) {
			line(imgFrame2Copy, crossingLine[0], crossingLine[1], SCALAR_GREEN, 2);
		}
		else {
			line(imgFrame2Copy, crossingLine[0], crossingLine[1], SCALAR_RED, 2);
		}

		drawpeopleCountOnImage(peopleCount, imgFrame2Copy);

		//imshow("imgThresh", imgThresh);
		imshow("imgFrame2Copy", imgFrame2Copy);

		//cv:moveWindow("imgThresh", 650, 0);//move window
	cv:moveWindow("grayImage", 0, 200);//move window

		currentFrameBlobs.clear();

		imgFrame1 = imgFrame2.clone();           // move frame 1 up to where frame 2 is

		if ((capVideo.get(CV_CAP_PROP_POS_FRAMES) + 1) < capVideo.get(CV_CAP_PROP_FRAME_COUNT)) {
			capVideo.read(imgFrame2);
		}
		else {
			cout << "end of video\n";
			break;
		}

		blnFirstFrame = false;

		curTime = (end - begin) / CLOCKS_PER_SEC;

		if (curTime % 1 == 0)
		{
			if (save != curTime){
				passAns = to_string(checkSeat[0]) + to_string(checkSeat[1]) + to_string(checkSeat[2]) + to_string(checkSeat[3]) +
					to_string(checkSeat[4]) + to_string(checkSeat[5]) + to_string(checkSeat[6]) + to_string(checkSeat[7]) +
					to_string(checkSeat[8]) + to_string(checkSeat[9]) + to_string(checkSeat[10]) + to_string(checkSeat[11]) +
					to_string(checkSeat[12]);

				gettime(&sttime);
				if (sttime.ti_hour < 10){//10시 이전의 string 정리
					if (sttime.ti_min < 10) {//10분 이전의 string정리
						if (sttime.ti_sec < 10) {//10초 이전의 string정리
							sti = "0" + to_string(sttime.ti_hour);
							smi = "0" + to_string(sttime.ti_min);
							ssec = "0" + to_string(sttime.ti_sec);
						}
						else{
							sti = "0" + to_string(sttime.ti_hour);
							smi = "0" + to_string(sttime.ti_min);
							ssec = to_string(sttime.ti_sec);
						}
					}
					else{
						if (sttime.ti_sec < 10){
							sti = "0" + to_string(sttime.ti_hour);
							smi = to_string(sttime.ti_min);
							ssec = "0" + to_string(sttime.ti_sec);
						}
						else{
							sti = "0" + to_string(sttime.ti_hour);
							smi = to_string(sttime.ti_min);
							ssec = to_string(sttime.ti_sec);
						}
					}
				}
				else{
					if (sttime.ti_min < 10){
						if (sttime.ti_sec < 10){
							sti = to_string(sttime.ti_hour);
							smi = "0" + to_string(sttime.ti_min);
							ssec = "0" + to_string(sttime.ti_sec);
						}
						else{
							sti = to_string(sttime.ti_hour);
							smi = "0" + to_string(sttime.ti_min);
							ssec = to_string(sttime.ti_sec);
						}
					}
					else{
						if (sttime.ti_sec < 10){
							sti = to_string(sttime.ti_hour);
							smi = to_string(sttime.ti_min);
							ssec = "0" + to_string(sttime.ti_sec);
						}
						else{
							sti = to_string(sttime.ti_hour);
							smi = to_string(sttime.ti_min);
							ssec = to_string(sttime.ti_sec);
						}
					}
				}

				timeinfo = sti + smi + ssec;//시간정보

				if (checkSeat[0] == 1){
					if (saveSeatdata[0] == 0 || saveSeatdata[0] == 2){
						saveSeattime[0] = timeinfo;
					}
					saveSeatdata[0] = checkSeat[0];
				}
				else{
					saveSeatdata[0] = checkSeat[0];
					saveSeattime[0] = "0";
				}

				if (checkSeat[1] == 1){
					if (saveSeatdata[1] == 0 || saveSeatdata[1] == 2){
						saveSeattime[1] = timeinfo;
					}

					saveSeatdata[1] = checkSeat[1];
				}
				else{
					saveSeatdata[1] = checkSeat[1];
					saveSeattime[1] = "0";
				}

				if (checkSeat[2] == 1){
					if (saveSeatdata[2] == 0 || saveSeatdata[2] == 2){
						saveSeattime[2] = timeinfo;
					}
					saveSeatdata[2] = checkSeat[2];
				}
				else{
					saveSeatdata[2] = checkSeat[2];
					saveSeattime[2] = "0";
				}

				if (checkSeat[3] == 1){
					if (saveSeatdata[3] == 0 || saveSeatdata[3] == 2){
						saveSeattime[3] = timeinfo;
					}
					saveSeatdata[3] = checkSeat[3];
				}
				else{
					saveSeatdata[3] = checkSeat[3];
					saveSeattime[3] = "0";
				}

				if (checkSeat[4] == 1){
					if (saveSeatdata[4] == 0 || saveSeatdata[4] == 2){
						saveSeattime[4] = timeinfo;
					}
					saveSeatdata[4] = checkSeat[4];
				}
				else{
					saveSeatdata[4] = checkSeat[4];
					saveSeattime[4] = "0";
				}

				if (checkSeat[5] == 1){
					if (saveSeatdata[5] == 0 || saveSeatdata[5] == 2){
						saveSeattime[5] = timeinfo;
					}
					saveSeatdata[5] = checkSeat[5];
				}
				else{
					saveSeatdata[5] = checkSeat[5];
					saveSeattime[5] = "0";
				}

				if (checkSeat[6] == 1){
					if (saveSeatdata[6] == 0 || saveSeatdata[6] == 2){
						saveSeattime[6] = timeinfo;
					}
					saveSeatdata[6] = checkSeat[6];
				}
				else{
					saveSeatdata[6] = checkSeat[6];
					saveSeattime[6] = "0";
				}
				if (checkSeat[7] == 1){
					if (saveSeatdata[7] == 0 || saveSeatdata[7] == 2){
						saveSeattime[7] = timeinfo;
					}
					saveSeatdata[7] = checkSeat[7];
				}
				else{
					saveSeatdata[7] = checkSeat[7];
					saveSeattime[7] = "0";
				}
				if (checkSeat[8] == 1){
					if (saveSeatdata[8] == 0 || saveSeatdata[8] == 2){
						saveSeattime[8] = timeinfo;
					}
					saveSeatdata[8] = checkSeat[8];
				}
				else{
					saveSeatdata[8] = checkSeat[8];
					saveSeattime[8] = "0";
				}
				if (checkSeat[9] == 1){
					if (saveSeatdata[9] == 0 || saveSeatdata[9] == 2){
						saveSeattime[9] = timeinfo;
					}
					saveSeatdata[9] = checkSeat[9];
				}
				else{
					saveSeatdata[9] = checkSeat[9];
					saveSeattime[9] = "0";
				}
				if (checkSeat[10] == 1){
					if (saveSeatdata[10] == 0 || saveSeatdata[10] == 2){
						saveSeattime[10] = timeinfo;
					}
					saveSeatdata[10] = checkSeat[10];
				}
				else{
					saveSeatdata[10] = checkSeat[10];
					saveSeattime[10] = "0";
				}
				if (checkSeat[11] == 1){
					if (saveSeatdata[11] == 0 || saveSeatdata[11] == 2){
						saveSeattime[11] = timeinfo;
					}
					saveSeatdata[11] = checkSeat[11];
				}
				else{
					saveSeatdata[11] = checkSeat[11];
					saveSeattime[11] = "0";
				}
				if (checkSeat[12] == 1){
					if (saveSeatdata[12] == 0 || saveSeatdata[12] == 2){
						saveSeattime[12] = timeinfo;
					}
					saveSeatdata[12] = checkSeat[12];
				}
				else{
					saveSeatdata[12] = checkSeat[12];
					saveSeattime[12] = "0";
				}
				stmt = conn->createStatement();
				try {
					stmt->execute("use spaceinfo");
					string sqlStr = "INSERT INTO osdata VALUES('";
					sqlStr += timeinfo;
					sqlStr += "',";
					sqlStr += "'";
					sqlStr += passAns;
					sqlStr += "',";
					sqlStr += "'";
					sqlStr += saveSeattime[0];
					sqlStr += "',";
					sqlStr += "'";
					sqlStr += saveSeattime[1];
					sqlStr += "',";
					sqlStr += "'";
					sqlStr += saveSeattime[2];
					sqlStr += "',";
					sqlStr += "'";
					sqlStr += saveSeattime[3];
					sqlStr += "',";
					sqlStr += "'";
					sqlStr += saveSeattime[4];
					sqlStr += "',";
					sqlStr += "'";
					sqlStr += saveSeattime[5];
					sqlStr += "',";
					sqlStr += "'";
					sqlStr += saveSeattime[6];
					sqlStr += "',";
					sqlStr += "'";
					sqlStr += saveSeattime[7];
					sqlStr += "',";
					sqlStr += "'";
					sqlStr += saveSeattime[8];
					sqlStr += "',";
					sqlStr += "'";
					sqlStr += saveSeattime[9];
					sqlStr += "',";
					sqlStr += "'";
					sqlStr += saveSeattime[10];
					sqlStr += "',";
					sqlStr += "'";
					sqlStr += saveSeattime[11];
					sqlStr += "',";
					sqlStr += "'";
					sqlStr += saveSeattime[12];
					sqlStr += "',";
					sqlStr += "'";
					sqlStr += to_string(peopleCount);
					sqlStr += "')";
					int affectedRows = stmt->executeUpdate(sqlStr);
					//cout << "Done! Num. rows affected: " << affectedRows << endl;

				}
				catch (sql::SQLException e)
				{
					cout << "SQL error. Error message : " << e.what() << endl;
					system("pause");
					exit(1);
				}
				save = curTime;

				String imageName, imageName1;
				imageName = "C:\\Users\\Tim\\Documents\\Visual Studio 2013\\Projects\\OS\\ImageFile\\" + timeinfo + ".jpg";
				imageName1 = "C:\\Users\\Tim\\Documents\\Visual Studio 2013\\Projects\\OS\\ImageFile\\save.jpg";
				cvSaveImage(imageName.c_str(), frame);
				cvSaveImage(imageName1.c_str(), frame);
			}
		}
		char chKey = cvWaitKey(1);
		if (chKey == 27)    {
			// Esc
			break;
		}
	}
	cvReleaseCapture(&capture);
	cvDestroyAllWindows();
	
	/*cvReleaseImage(&temp);
	cvReleaseImage(&bkgImage);
	cvReleaseImage(&grayImage);
	cvReleaseImage(&diffImage);
	cvReleaseCapture(&capture);*/

	return(0);
}

//function
///////////////////////////////////////////////////////////////////////////////////////////////////
void matchCurrentFrameBlobsToExistingBlobs(vector<Blob> &existingBlobs, vector<Blob> &currentFrameBlobs) {

	for (auto &existingBlob : existingBlobs) {

		existingBlob.blnCurrentMatchFoundOrNewBlob = false;

		existingBlob.predictNextPosition();
	}

	for (auto &currentFrameBlob : currentFrameBlobs) {

		int intIndexOfLeastDistance = 0;
		double dblLeastDistance = 100000.0;

		for (unsigned int i = 0; i < existingBlobs.size(); i++) {

			if (existingBlobs[i].blnStillBeingTracked == true) {

				double dblDistance = distanceBetweenPoints(currentFrameBlob.centerPositions.back(), existingBlobs[i].predictedNextPosition);

				if (dblDistance < dblLeastDistance) {
					dblLeastDistance = dblDistance;
					intIndexOfLeastDistance = i;
				}
			}
		}

		if (dblLeastDistance < currentFrameBlob.dblCurrentDiagonalSize * 0.5) {
			addBlobToExistingBlobs(currentFrameBlob, existingBlobs, intIndexOfLeastDistance);
		}
		else {
			addNewBlob(currentFrameBlob, existingBlobs);
		}

	}

	for (auto &existingBlob : existingBlobs) {

		if (existingBlob.blnCurrentMatchFoundOrNewBlob == false) {
			existingBlob.intNumOfConsecutiveFramesWithoutAMatch++;
		}

		if (existingBlob.intNumOfConsecutiveFramesWithoutAMatch >= 5) {
			existingBlob.blnStillBeingTracked = false;
		}

	}

}

void addBlobToExistingBlobs(Blob &currentFrameBlob, vector<Blob> &existingBlobs, int &intIndex) {

	existingBlobs[intIndex].currentContour = currentFrameBlob.currentContour;
	existingBlobs[intIndex].currentBoundingRect = currentFrameBlob.currentBoundingRect;

	existingBlobs[intIndex].centerPositions.push_back(currentFrameBlob.centerPositions.back());

	existingBlobs[intIndex].dblCurrentDiagonalSize = currentFrameBlob.dblCurrentDiagonalSize;
	existingBlobs[intIndex].dblCurrentAspectRatio = currentFrameBlob.dblCurrentAspectRatio;

	existingBlobs[intIndex].blnStillBeingTracked = true;
	existingBlobs[intIndex].blnCurrentMatchFoundOrNewBlob = true;
}

void addNewBlob(Blob &currentFrameBlob, vector<Blob> &existingBlobs) {

	currentFrameBlob.blnCurrentMatchFoundOrNewBlob = true;
	existingBlobs.push_back(currentFrameBlob);
}

double distanceBetweenPoints(Point point1, Point point2) {

	int intX = abs(point1.x - point2.x);
	int intY = abs(point1.y - point2.y);

	return(sqrt(pow(intX, 2) + pow(intY, 2)));
}

bool checkIfBlobsCrossedTheLine(vector<Blob> &blobs, int &intHorizontalLinePosition, int &peopleCount) {
	bool blnAtLeastOneBlobCrossedTheLine = false;

	for (auto blob : blobs) {

		if (blob.blnStillBeingTracked == true && blob.centerPositions.size() >= 2) {
			int prevFrameIndex = (int)blob.centerPositions.size() - 2;
			int currFrameIndex = (int)blob.centerPositions.size() - 1;

			if (blob.centerPositions[prevFrameIndex].y > intHorizontalLinePosition && blob.centerPositions[currFrameIndex].y <= intHorizontalLinePosition) {
				if (peopleCount > 0){
					peopleCount--;
					blnAtLeastOneBlobCrossedTheLine = true;
				}
			}
			else if (blob.centerPositions[prevFrameIndex].y < intHorizontalLinePosition && blob.centerPositions[currFrameIndex].y >= intHorizontalLinePosition){
				peopleCount++;
				blnAtLeastOneBlobCrossedTheLine = true;
			}
		}

	}

	return blnAtLeastOneBlobCrossedTheLine;
}

void drawpeopleCountOnImage(int &peopleCount, Mat &imgFrame2Copy) {

	int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
	double dblFontScale = (imgFrame2Copy.rows * imgFrame2Copy.cols) / 300000.0;
	int intFontThickness = (int)round(dblFontScale * 1.5);

	Size textSize = getTextSize(to_string(peopleCount), intFontFace, dblFontScale, intFontThickness, 0);

	Point ptTextBottomLeftPosition;

	ptTextBottomLeftPosition.x = imgFrame2Copy.cols - 1 - (int)((double)textSize.width * 1.25);
	ptTextBottomLeftPosition.y = (int)((double)textSize.height * 1.25);

	putText(imgFrame2Copy, to_string(peopleCount), ptTextBottomLeftPosition, intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);

}

void gettime(struct time *p) {
	char temp[30];
	char *cp;
	_strtime(temp); // temp char배열 안에 00:00:00(시:분:초)순의 문자열 상태로 저장 

	cp = strtok(temp, ":"); // temp배열의 첫번째 ':'까지 문자열를 끊어내어 cp에게 시에 해당하는 문자열의 시작주소를 리턴 

	p->ti_hour = atoi(cp); // 끊어낸 시에 해당하는 문자열을 정수형 데이터로 변환하여 p->ti_hour에 저장 

	cp = strtok(NULL, ":"); // temp배열의 두번째 ':'까지 문자열를 끊어내어 cp에게 분에 해당하는 문자열의 시작주소를 리턴 

	p->ti_min = atoi(cp); // 끊어낸 분에 해당하는 문자열을 정수형 데이터로 변환하여 p->ti_min에 저장 

	cp = strtok(NULL, ":"); // temp배열의 세번째 ':'까지 문자열를 끊어내려하나 ':'은 나타나지 않고 문자열이 
	// 끝나게 되므로 남은 문자열 전체를 리턴한다 즉, cp에게 초에 해당하는 문자열의 시작주소가 리턴한다. 

	p->ti_sec = atoi(cp); // 끊어낸 초에 해당하는 문자열을 정수형 데이터로 변환하여 p->ti_sec에 저장 
}