//--------------------------------------------------------------------------------------
// File: tracker.h
//
// This file contains the definitions for obtaining object tracking data using OpenCV
//--------------------------------------------------------------------------------------

#pragma once

#include "OpenCVToolkit.h"

//--------------------------------------------------------------------------------------
// This class is used for tracking objects
//--------------------------------------------------------------------------------------
class Tracker {
public:
    //--------------------------------------------------------------------------------------
    // Functions
    //--------------------------------------------------------------------------------------

    // Constructor and Destructor
    Tracker();
    ~Tracker();

    // Camera data
    bool InitCamera();
    void UpdateCamera();
    void ShowCameraWindow();

    // Ball tracking data
    cv::Point getGreenPosition() { return tracker_green; };
    cv::Point getRedPosition() { return tracker_red; };
    int getGreenSize() { return size_green.area(); };
    int getRedSize() { return size_red.area(); };

    // Ball tracking strings
    std::wstring getGreenTrackerString();
    std::wstring getRedTrackerString();

    // Tracking area
    cv::Size frameSize = frameSize;

private:
    //--------------------------------------------------------------------------------------
    // Variables
    //--------------------------------------------------------------------------------------

    // Histograms
    cv::MatND hist_green = cv::Mat::zeros(cv::Size(sbins, hbins), CV_32F);
    cv::MatND hist_red = cv::Mat::zeros(cv::Size(sbins, hbins), CV_32F);

    // Colour tracking
    cv::Point tracker_green;
    cv::Point tracker_red;

    // Colour depth
    cv::Size size_green;
    cv::Size size_red;

    //--------------------------------------------------------------------------------------
    // Functions
    //--------------------------------------------------------------------------------------

    std::string getTrackerString(std::string col, cv::Point point, cv::Size size);
    std::wstring getTrackerWcharString(std::string col, cv::Point point, cv::Size size);
};
