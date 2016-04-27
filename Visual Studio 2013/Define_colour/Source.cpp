#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\opencv.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

//////////////////////////////////////////
//Global variations and demo parameters///
//////////////////////////////////////////

//camera setting
const Size frameSize = Size(640, 480);

//Colour generator seting
const vector<Mat>::size_type sampleNum = 6;

//Preprocessing bilateral filter
const int fsize = 3;

// Build the histogram and compute its contents.
// Dimension of the histogram
const int dim = 2;
// 2 image channels (H, S)
int hsv_channels[] = { 0, 1 };
//Number of bins
const int hbins = 50; //number of bins for H
const int sbins = 50; //number of bins for S
int histSize[] = { hbins, sbins };
//Range of rach channels
float h_ranges[] = { 0.0, 255.0 };
float s_ranges[] = { 0.0, 255.0 };
const float* ranges[] = { h_ranges, s_ranges };
cv::MatND hist = Mat::zeros(Size(sbins, hbins), CV_32F);//Place to put the histgram

//Region mark setting
//Remove flood fill noise threshold
const int thre = 100;
//Flood fill local tolerance
const int tolerance = 200;

//Mouse events for colour
bool clickedL = false;
bool clickedR = false;
bool histCalc = false;
Scalar sampleRectColour = Scalar(0, 255, 0);
void mouse_callback_colour(int event, int x, int y, int, void* param)
{
	switch (event) {
	default:
		break;
	case CV_EVENT_LBUTTONUP:
		clickedL = true;
		clickedR = false;
		histCalc = true;
		sampleRectColour = Scalar(0, 0, 255);
		break;
	case CV_EVENT_LBUTTONDOWN:
		clickedL = false;
		clickedR = false;
		histCalc = false;
		sampleRectColour = Scalar(0, 0, 255);
		break;
	case CV_EVENT_RBUTTONUP:
		clickedL = false;
		clickedR = true;
		histCalc = false;
		break;
	}
};

void regionMark(const Mat &frame, const MatND &hsv_hist, Mat &markMap)
{
	//Back projection operation
	Mat hsv;
	Mat backProject;
	Mat binary(frameSize, CV_8UC3);
	cvtColor(frame, hsv, CV_RGB2HSV);
	cv::cvtColor(frame, hsv, CV_RGB2HSV);
	cv::calcBackProject(&hsv, 1, hsv_channels, hsv_hist, backProject, ranges);
	//Remove noise
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
	erode(backProject, binary, element);
	dilate(binary, binary, element);
	imshow("Back Projection", binary);

	//Mark regions by using flood fill			
	markMap = binary.clone();

	threshold(binary, markMap, thre, 255, THRESH_TOZERO);
	//Operation mask that should be a single-channel 8-bit image, 2 pixels wider and 2 pixels taller than image
	Mat mask = Mat::zeros(Size(frameSize.width + 2, frameSize.height + 2), CV_8UC1);
	Mat inverseMask = mask.clone();
	Mat maskROI = mask(Rect(1, 1, frameSize.width, frameSize.height));
	Mat inverseMaskROI = inverseMask(Rect(1, 1, frameSize.width, frameSize.height));
	//Mat currentMask;
	threshold(markMap, maskROI, thre, 255, THRESH_BINARY);
	threshold(markMap, inverseMaskROI, thre, 255, THRESH_BINARY_INV);
	//Find marking locations
	int index = thre + 1;
	vector<Point> markLoc;
	findNonZero(maskROI, markLoc);
	while (markLoc.size() != 0)
	{
		Point seed = markLoc[0];
		floodFill(markMap, inverseMask, seed, cv::Scalar(static_cast<uchar>(index++)), (cv::Rect*)0, cv::Scalar(tolerance), cv::Scalar(tolerance), 4 | (static_cast<uchar>(255) << 8));
		bitwise_not(inverseMaskROI, maskROI);
		findNonZero(maskROI, markLoc);
	}
}

int main()
{
	VideoCapture webcam(0);

	if (!webcam.isOpened())
	{
		cout << "Cannot open the video cam..." << endl;
		return -1;
	}

	int const delay = 30;
	bool stop = false;
	const Size sampleSize = Size(60, 60);

	//UI ROIs
	Mat UI = Mat::zeros(Size(frameSize.width, frameSize.height + sampleSize.height), CV_8UC3);
	//Region for user interaction
	Mat clickROI = UI(Rect(0, 0, frameSize.width, frameSize.height));
	//Six regions for storing and showing sample list
	Rect SampleArea((frameSize.width - sampleSize.width) / 2, (frameSize.height - sampleSize.height) / 2, sampleSize.width, sampleSize.height);
	vector<Mat> listSample; //store
	Mat listSampleHSV[sampleNum]; //store
	vector<Mat> listROI(sampleNum);//show
	for (vector<Mat>::size_type ix = 0; ix != listROI.size(); ix++)
		listROI[ix] = UI(Rect(sampleSize.width*ix + 1, frameSize.height, sampleSize.width, sampleSize.height));

	string order1 = "Cover the rectangle area with colour samples";
	string order2 = "Repeat the sampling for six times with different angles and locaitons";
	string order3 = "Left click to add sample; Right click to delete last sample";

	Mat webcamImg;
	Mat frame;
	//Setup mouse callback
	namedWindow("Colour Sampler", 0);
	setMouseCallback("Colour Sampler", mouse_callback_colour);

	while (!stop)
	{
		if (!webcam.read(webcamImg))
			break;
		flip(webcamImg, frame, 1);//See the image as mirror
		//bilateralFilter(webcamImg, frame, fsize, 150, 150);
		addWeighted(clickROI, 1, frame, 1, 0, clickROI);

		//Draw Text and rectangles on the interface
		rectangle(UI, Rect(0, 0, frameSize.width, 90), Scalar(70, 70, 70), -1);
		rectangle(UI, Rect(0, frameSize.height - 30, frameSize.width, 30), Scalar(70, 70, 70), -1);
		putText(UI, order1, Point(10, 30), FONT_HERSHEY_COMPLEX_SMALL, 0.6, Scalar(0, 255, 255), 1);
		putText(UI, order2, Point(10, 60), FONT_HERSHEY_COMPLEX_SMALL, 0.6, Scalar(0, 255, 255), 1);
		rectangle(UI, SampleArea, sampleRectColour);
		putText(UI, order3, Point(10, frameSize.height - 10), FONT_HERSHEY_COMPLEX_SMALL, 0.6, Scalar(0, 255, 255), 1);

		//Sampling Algorithm
		if (clickedL && (listSample.size() < sampleNum))
		{
			listSample.push_back(frame(SampleArea).clone());
			clickedL = false;
			sampleRectColour = Scalar(0, 255, 0);
		}
		if (clickedL && (listSample.size() >= sampleNum))
		{
			listSample.push_back(frame(SampleArea).clone());
			for (vector<Mat>::iterator iter = listSample.begin(); iter != listSample.end() - 1; iter++)
				*iter = *(iter + 1);
			listSample.resize(sampleNum);
			clickedL = false;
			sampleRectColour = Scalar(0, 255, 0);
		}
		if (clickedR && !(listSample.empty()))
		{
			listSample.pop_back();
			clickedR = false;
		}


		//Show samples
		if (!(listSample.empty()))
		{
			vector<Mat>::iterator iterROI = listROI.begin();
			for (vector<Mat>::iterator iterSample = listSample.begin(); iterSample != listSample.end(); iterSample++) {
				addWeighted(*iterROI, 1, *iterSample, 1, 0, *iterROI);
				iterROI++;
			}

		}
		imshow("Colour Sampler", UI);

		//Back porjection calculation
		if (listSample.size() == sampleNum)
		{
			MatND hsv_hist = Mat::zeros(Size(1, hbins), CV_32F);
			//RGB to HSV
			size_t i = 0;
			for (vector<Mat>::iterator iterSample = listSample.begin(); iterSample != listSample.end(); iterSample++) {
				cvtColor(*iterSample, *(listSampleHSV + i), CV_RGB2HSV);
				i++;
			}
			//Get histogram
			if (histCalc)
			{
				hist = Mat::zeros(Size(sbins, hbins), CV_32F);
				for (int i = 0; i != sampleNum; i++) {
					calcHist(listSampleHSV + i, 1, hsv_channels, cv::Mat(), hsv_hist, dim, histSize, ranges, true, false);
					addWeighted(hist, 1, hsv_hist, 1, 0, hist);

				}
				histCalc = false;
			}

			Mat regions;
			regionMark(frame, hist, regions);
			applyColorMap(regions, regions, COLORMAP_JET);
			imshow("Region Map", regions);

		}

		clickROI = Mat::zeros(clickROI.size(), CV_8UC3);
		for (vector<Mat>::size_type ix = 0; ix != listROI.size(); ix++)
			listROI[ix] = Mat::zeros(sampleSize, CV_8UC3);
		if (waitKey(delay) >= 0)
			stop = true;
	}

	cv::destroyAllWindows();
	webcam.release();

	cv::FileStorage fs("../Histograms/colour_hist.yml", cv::FileStorage::WRITE);
	if (!fs.isOpened()) { std::cout << "unable to open file storage!" << std::endl; return -1; }
	fs << "histogram" << hist;
	fs.release();

	return 0;
}