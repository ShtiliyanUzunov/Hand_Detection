#include "stdafx.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "classifier.h"
#include "imageProcessor.h"

#include <iostream>

using namespace cv;
using namespace std;

char key = '_';
Mat src, roi, seg;
Classifier seg_cls;
ImageProcessor imgProcessor;
VideoCapture cam;
vector<Point> fingerPoints, insidePoints;
bool train = false;
const Rect roi_rect(0, 0, 400, 300);

string classify(vector<Point>& inputPoints);

void initCamera();

int main()
{
	initCamera();

	do
	{
		fingerPoints.clear();
		insidePoints.clear();

		//get roi
		cam >> src;
		roi = src(roi_rect).clone();
		
		//preprocessing
		imgProcessor.BlurImage(roi);
		imgProcessor.ConvertToHsv(roi);
		imgProcessor.MakeUnimodal(roi);
		

		seg_cls.SegmentImage(roi, seg);
		imgProcessor.clearNoise(seg);
		imgProcessor.CreateConvexHull(seg, fingerPoints, insidePoints);
		imgProcessor.drawCircles(src, fingerPoints, Scalar(255, 100, 0));
		imgProcessor.drawCircles(src, insidePoints, Scalar(100, 255, 0));

		std::ostringstream str;
		str << "Fingers count:" << fingerPoints.size();
		cv::putText(src, str.str(), Point(10, 30), CV_FONT_HERSHEY_PLAIN, 2, Scalar(0, 0, 0), 2);

		std::ostringstream clsString;
		clsString << "Class: " + classify(fingerPoints);
		cv::putText(src, clsString.str(), Point(10, 60), CV_FONT_HERSHEY_PLAIN, 2, Scalar(0, 0, 0), 2);

		// Display degug info
		rectangle(src, roi_rect, Scalar(0, 0, 255), 2);
		imshow("Image", src);
		imshow("Segmented", seg);

		key = waitKey(33);
	} while (key != 'q');

	return 0;
}

string classify(vector<Point>& inputPoints) {
	int size = inputPoints.size();
	switch (size) {
	case 5 :
		return "open";
	case 2 :
		return "rock";
	case 1 :
		return "closed";
	default :
		return "unknown";
	}
}

void onMouse(int event, int x, int y, int flags, void* data)
{
	switch (event)
	{
	case CV_EVENT_LBUTTONUP: { 
		int index = (x + y * roi.cols) * 3;
		Point3i pixel(roi.data[index], roi.data[index + 1], roi.data[index + 2]);
		seg_cls.AddPoint(pixel);
	}
	default:
		break;
	}
}

void initCamera() {
	namedWindow("Image");
	setMouseCallback("Image", onMouse);

	cam = VideoCapture(0);

	if (!cam.isOpened())
	{
		cout << "Cannot open camera." << endl;
	}
}