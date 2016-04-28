#pragma once

// OpenCV
#include "OpenCVToolkit.h"

//--------------------------------------------------------------------------------------
// This class is used to track two objects using OpenCV
//--------------------------------------------------------------------------------------
class Tracker {
public:
    // Functions

    Tracker();
    ~Tracker();

    bool InitCamera();
    void UpdateCamera();
    void ShowCameraWindow();

    cv::Point getGreenPosition() { return tracker_green; };
    cv::Point getRedPosition() { return tracker_red; };
    int getGreenSize() { return size_green.area(); };
    int getRedSize() { return size_red.area(); };

    std::wstring getGreenTrackerString();
    std::wstring getRedTrackerString();

    cv::Size frameSize = frameSize;

private:
    // Variables

    // Histograms
    cv::MatND hist_green = cv::Mat::zeros(cv::Size(sbins, hbins), CV_32F);
    cv::MatND hist_red = cv::Mat::zeros(cv::Size(sbins, hbins), CV_32F);

    // Colour tracking
    cv::Point tracker_green;
    cv::Point tracker_red;
    // Colour depth
    cv::Size size_green;
    cv::Size size_red;

    // Functions

    std::string getTrackerString(std::string col, cv::Point point, cv::Size size);
    std::wstring getTrackerWcharString(std::string col, cv::Point point, cv::Size size);

};

