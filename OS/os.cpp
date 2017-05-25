#pragma warning(disable : 4996)
#include <iostream>
#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <sstream> 
#include <cstdlib>
#include <string>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <time.h>
#include<windows.h>

using namespace std;
using namespace cv;

CvMemStorage* storage = NULL;
CvMemStorage* storage1[13] = { NULL };
CvSeq* contours = 0;
CvSeq* contours1[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
CvRect rect;
IplImage* grayImage1[13];
IplImage* diffImage1[13];
IplImage* bkgImage1[13];
IplImage* copy1;

int countSeat[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int checkSeat[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int closeSeat[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int saveSeatdata[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
String saveSeattime[13] = { "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0" };
int save = 0;
int curTime = 999;
String passAns;

#pragma comment(lib, "mysqlcppconn.lib")

const string server = "tcp://127.0.0.1";
const string username = "root";
const string password = "1234";

sql::Driver *driver;
sql::Connection *dbConn;
sql::Statement *stmt;
sql::Statement *stmt1;

struct time {
	int ti_hour;
	int ti_min;
	int ti_sec;
};

void gettime(struct time *);


void connect_mysql()
{
	try
	{
		driver = get_driver_instance();
	}
	catch (sql::SQLException e)
	{
		cout << "Couldn't get database driver : " << e.what() << endl;
		system("pause");
		exit(1);
	}
	try {
		dbConn = driver->connect(server, username, password);
	}

	catch (sql::SQLException e)
	{
		cout << "Couldn't connect to database : " << e.what() << endl;
		system("pause");
		exit(1);
	}
}

int main()
{
	string info, timeinfo, sti, smi, ssec;

	struct time sttime;
	int num = 1;

	int flag = 1;
	storage = cvCreateMemStorage(0);
	for (int i = 0; i < 13; i++){
		storage1[i] = cvCreateMemStorage(0);
	}
	connect_mysql();
	// 움직임을 검출할 비디오 파일을 포인터 capture에 저장한다.
	CvCapture *capture = cvCaptureFromFile("C:\\Users\\Tim\\Documents\\Visual Studio 2013\\Projects\\OS\\OS\\열람실1.avi");
	//CvCapture *capture1 = cvCaptureFromFile("C:\\Users\\Tim\\Desktop\\2016-3\\DATA\\Entrance.mp4");
	if (!capture)    {
		std::cout << "The video file was not found" << std::endl;
		return 0;
	}
	/*if (!capture1)    {
		std::cout << "The video file was not found" << std::endl;
		system("pause");
		return 0;
	}*/

	int width = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
	int height = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
	CvSize size = cvSize(width, height);

	// Initial background image

	IplImage *bkgImage = cvCreateImage(size, IPL_DEPTH_8U, 1);
	IplImage *temp = cvQueryFrame(capture);
	cvCvtColor(temp, bkgImage, CV_BGR2GRAY);

	if (bkgImage == NULL)    return -1;

	// cvCreateImage 함수를 사용하여 그레이 스케일로 변환을 위한 grayImage와 차영상을 위한 diffImage를 생성한다.
	IplImage *grayImage = cvCreateImage(size, IPL_DEPTH_8U, 1);
	IplImage *diffImage = cvCreateImage(size, IPL_DEPTH_8U, 1);
	IplImage *frame = NULL;
	IplImage *frame1 = NULL;

	int t = 0;    // frame count
	int nThreshold = 50;

	clock_t begin, end;
	begin = clock();        // 시간설정

	while (1)    {
		// capture로부터 프레임을 획득하여 포인터 frame에 저장한다.
		if (!capture)    exit;

		frame = cvQueryFrame(capture);
		//frame1 = cvQueryFrame(capture1);
		// cvCvtColor 함수를 사용하여 cvQueryFrame 함수로 획득한 frame을 그레이 스케일 영상으로 변환하여 grayImage에 저장한다.

		cvCvtColor(frame, grayImage, CV_BGR2GRAY);
		cvAbsDiff(grayImage, bkgImage, diffImage);

		cvThreshold(diffImage, diffImage, nThreshold, 255, CV_THRESH_BINARY);
		cvErode(diffImage, diffImage, 0, 5);
		cvDilate(diffImage, diffImage, 0, 5);
		cvFindContours(diffImage, storage, &contours);


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

		cvShowImage("9", grayImage1[12]);

		// cvAbsDiff 함수로 현재의 입력 비디오 프레임과 그레이 스케일 영상인 grayImage와 배경 영상인 bkgImage와의 차이의 절대값을 계산하여 diffImage에 저장한다.
		//cvAbsDiff(grayImage, bkgImage, diffImage);  original code

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
			if (checkSeat[0] == 2)
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
			if (checkSeat[1] == 2)
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
			if (checkSeat[2] == 2)
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
			if (checkSeat[3] == 2)
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
			if (checkSeat[4] == 2)
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
			if (checkSeat[5] == 2)
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
			if (countSeat[6] > 100 && countSeat[6] < 200){
				checkSeat[6] = 2;
			}
			else if (countSeat[6] > 200){
				checkSeat[6] = 1;
			}
			if (checkSeat[6] == 2)
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
			if (checkSeat[7] == 2)
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
			if (checkSeat[9] == 2)
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
			if (checkSeat[10] == 2)
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
			if (checkSeat[11] == 2)
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
			if (checkSeat[12] == 2)
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

		cout << "**************************" << endl;
		cvShowImage("grayImage", grayImage);
		cvShowImage("diffImage", diffImage);
		//cvShowImage("상상랩", frame1);

		char chKey = cvWaitKey(3);
		if (chKey == 27)    {    // Esc
			break;

		}
		end = clock();          // 시간설정
		curTime = (end - begin) / CLOCKS_PER_SEC;
		if (curTime % 1 == 0)
		{
			if (save != curTime){
				cout << "수행시간 : " << ((end - begin) / CLOCKS_PER_SEC) << endl;
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
							smi =  to_string(sttime.ti_min);
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

				stmt = dbConn->createStatement();
				
				cout << "select num:  ";;

				cout << passAns << endl;

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

				stmt1 = dbConn->createStatement();
				try {
					stmt1->execute("use spaceinfo");

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
					sqlStr += "')";
					int affectedRows = stmt1->executeUpdate(sqlStr);
					cout << "Done! Num. rows affected: " << affectedRows << endl;

				}
				catch (sql::SQLException e)
				{
					cout << "SQL error. Error message : " << e.what() << endl;
					system("pause");
					exit(1);
				}

				save = curTime;
		
				String imageName,imageName1;
				imageName = "C:\\Users\\Tim\\Documents\\Visual Studio 2013\\Projects\\OS\\ImageFile\\" + timeinfo + ".jpg";
				imageName1 = "C:\\Users\\Tim\\Documents\\Visual Studio 2013\\Projects\\OS\\ImageFile\\save.jpg";
				cvSaveImage(imageName.c_str(), frame);
				cvSaveImage(imageName1.c_str(), frame);
			}

		}
		for (int i = 0; i < 13; i++){
			cvReleaseImage(&bkgImage1[i]);
			cvReleaseImage(&grayImage1[i]);
			cvReleaseImage(&diffImage1[i]);
		}
	}


	cvDestroyAllWindows();
	cvReleaseImage(&temp);
	cvReleaseImage(&bkgImage);
	cvReleaseImage(&grayImage);
	cvReleaseImage(&diffImage);
	cvReleaseCapture(&capture);

	return 0;
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