#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\opencv.hpp>

using namespace std;

extern cv::Mat frame;
extern cv::VideoCapture webcam;

extern bool stop;
extern int delay; //ms
extern int l_threshold;
extern int h_threshold;

// camera setting
extern const cv::Size frameSize;

// Colour generator seting
extern const vector<cv::Mat>::size_type sampleNum;

// Preprocessing bilateral filter
extern const int fsize;

// Build the histogram and compute its contents.
// Dimension of the histogram
extern const int dim;
// 2 image channels (H, S)
extern int hsv_channels[];
// Number of bins
extern const int hbins; //number of bins for H
extern const int sbins; //number of bins for S
extern int histSize[];
// Range of rach channels
extern float h_ranges[];
extern float s_ranges[];
extern const float* ranges[];
extern const cv::Size sampleSize;

// Region mark setting
// Remove flood fill noise threshold
extern const int thre;
// Flood fill local tolerance
extern const int tolerance;

extern void regionMark(const cv::Mat &frame, const cv::MatND &hsv_hist, cv::Mat &markMap);

extern void updateTrackAndSize(const cv::Mat &frame, const cv::MatND &hsv_hist, cv::Point &point, cv::Size &size);
