#include "OpenCVToolkit.h"

cv::Mat frame;
cv::VideoCapture webcam(0);

// camera setting
const cv::Size frameSize = cv::Size(640, 480);

// Colour generator seting
const vector<cv::Mat>::size_type sampleNum = 6;

// Preprocessing bilateral filter
const int fsize = 3;

// Build the histogram and compute its contents.
// Dimension of the histogram
const int dim = 2;
// 2 image channels (H, S)
int hsv_channels[] = { 0, 1 };
// Number of bins
const int hbins = 50; //number of bins for H
const int sbins = 50; //number of bins for S
int histSize[] = { hbins, sbins };
// Range of rach channels
float h_ranges[] = { 0.0, 255.0 };
float s_ranges[] = { 0.0, 255.0 };
const float* ranges[] = { h_ranges, s_ranges };
const cv::Size sampleSize = cv::Size(60, 60);

// Region mark setting
// Remove flood fill noise threshold
const int thre = 100;
// Flood fill local tolerance
const int tolerance = 200;

void regionMark(const cv::Mat &frame, const cv::MatND &hsv_hist, cv::Mat &markMap) {

    // Back projection operation
    cv::Mat hsv;
    cv::Mat backProject;
    cv::Mat binary(frameSize, CV_8UC3);
    cvtColor(frame, hsv, CV_RGB2HSV);
    cv::cvtColor(frame, hsv, CV_RGB2HSV);
    cv::calcBackProject(&hsv, 1, hsv_channels, hsv_hist, backProject, ranges);
    // Remove noise
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    erode(backProject, binary, element);
    cv::dilate(binary, binary, element);
    //imshow("Back Projection", binary);

    // Mark regions by using flood fill
    markMap = binary.clone();

    threshold(binary, markMap, thre, 255, cv::THRESH_TOZERO);
    // Operation mask that should be a single-channel 8-bit image, 2 pixels wider and 2 pixels taller than image
    cv::Mat mask = cv::Mat::zeros(cv::Size(frameSize.width + 2, frameSize.height + 2), CV_8UC1);
    cv::Mat inverseMask = mask.clone();
    cv::Mat maskROI = mask(cv::Rect(1, 1, frameSize.width, frameSize.height));
    cv::Mat inverseMaskROI = inverseMask(cv::Rect(1, 1, frameSize.width, frameSize.height));
    // Mat currentMask;
    threshold(markMap, maskROI, thre, 255, cv::THRESH_BINARY);
    threshold(markMap, inverseMaskROI, thre, 255, cv::THRESH_BINARY_INV);
    // Find marking locations
    int index = thre + 1;
    vector<cv::Point> markLoc;
    findNonZero(maskROI, markLoc);
    while (markLoc.size() != 0) {
        cv::Point seed = markLoc[0];
        floodFill(markMap, inverseMask, seed, cv::Scalar(static_cast<uchar>(index++)), (cv::Rect*)0, cv::Scalar(tolerance), cv::Scalar(tolerance), 4 | (static_cast<uchar>(255) << 8));
        bitwise_not(inverseMaskROI, maskROI);
        findNonZero(maskROI, markLoc);
    }

}

void updateTrackAndSize(const cv::Mat &frame, const cv::MatND &hsv_hist, cv::Point &point, cv::Size &size) {
    cv::Point trackedCentre;
    //Find the largest region in the image
    cv::Mat region;
    cv::Mat binary;
    regionMark(frame, hsv_hist, region);
    cv::threshold(region, binary, thre, 255, cv::THRESH_BINARY);

    //Morphological Operations
    cv::Mat element3(3, 3, CV_8U, cv::Scalar(1));
    cv::Mat openedImg(frame.cols, frame.rows, CV_8UC1);
    cv::morphologyEx(binary, openedImg, cv::MORPH_OPEN, element3);

    //find large object
    int largest_area = 0;
    int largest_contour_index = 0;
    cv::Rect bounding_rect;
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(openedImg, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    for (int i = 0; i < contours.size(); i++) {         // Iterate through each contour
        double a = cv::contourArea(contours[i]);        // Find the area of contour
        if (a > largest_area) {
            largest_area = a;
            largest_contour_index = i;                  //Store the index of largest contour
            bounding_rect = boundingRect(contours[i]);  // Find the bounding rectangle for biggest contour
        }
    }
    size.width = bounding_rect.width;
    size.height = bounding_rect.height;

    //Scale object centre back to its original size
    point.x = bounding_rect.x + bounding_rect.width / 2;
    point.y = bounding_rect.y + bounding_rect.height / 2;

}