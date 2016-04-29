//--------------------------------------------------------------------------------------
// File: tracker.cpp
//
// This file contains the implementations for obtaining object tracking data
//--------------------------------------------------------------------------------------

#include "tracker.h"

//--------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------
Tracker::Tracker() {

    tracker_green = cv::Point(0, 0);
    tracker_red = cv::Point(0, 0);
    size_green = cv::Size(0, 0);
    size_red = cv::Size(0, 0);

    // Initialise the green ball tracker
    cv::FileStorage fs_g("Histograms/colour_hist_GREEN.yml", cv::FileStorage::READ);
    if (!fs_g.isOpened()) { std::cout << "unable to open file storage!" << std::endl; }
    fs_g["histogram"] >> hist_green;
    fs_g.release();

    // Inititalise the red ball tracker
    cv::FileStorage fs_r("Histograms/colour_hist_RED.yml", cv::FileStorage::READ);
    if (!fs_r.isOpened()) { std::cout << "unable to open file storage!" << std::endl; }
    fs_r["histogram"] >> hist_red;
    fs_r.release();
}

//--------------------------------------------------------------------------------------
// Clean up
//--------------------------------------------------------------------------------------
Tracker::~Tracker() {
    hist_green.release();
    hist_red.release();
}

bool Tracker::InitCamera() {
    if (!webcam.isOpened()) {
        std::cout << "Cannot open the video cam" << std::endl;
        return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------
// Poll the camera for updated tracking information
//--------------------------------------------------------------------------------------
void Tracker::UpdateCamera() {

    // If a new frame can't be read, return
    if (!webcam.read(frame)) {
        return;
    }

    // See the image as mirror
    cv::flip(frame, frame, 1);

    // Green Colour recognition
    if (hist_green.data) {
        updateTrackAndSize(frame, hist_green, tracker_green, size_green);
    }
    // Red Colour recognition
    if (hist_red.data) {
        updateTrackAndSize(frame, hist_red, tracker_red, size_red);
    }
}

//--------------------------------------------------------------------------------------
// Display the camera input, with related tracking info
//--------------------------------------------------------------------------------------
void Tracker::ShowCameraWindow() {
    // Get green ball tracking info
    std::string result_green = getTrackerString("Green", tracker_green, size_green);
    cv::putText(frame, result_green, cv::Point(10, 20), 1, 1, cv::Scalar(0, 255, 255), 1);
    cv::putText(frame, "+", tracker_green, 1, 3, cv::Scalar(0, 255, 0), 2);

    // Get red ball tracking data
    std::string result_red = getTrackerString("Red", tracker_red, size_red);
    cv::putText(frame, result_red, cv::Point(10, 40), 1, 1, cv::Scalar(0, 255, 255), 1);
    cv::putText(frame, "+", tracker_red, 1, 3, cv::Scalar(0, 0, 255), 2);

    // Show the camera tracker window
    cv::imshow("Green & Red", frame);
}

//--------------------------------------------------------------------------------------
// Returns a string containing the tracking data for a provided object
//--------------------------------------------------------------------------------------
std::string Tracker::getTrackerString(std::string col, cv::Point point, cv::Size size) {
    std::ostringstream str;
    str << col << " Ball - x: " << point.x << " y: " << point.y << " size: " << size.area();
    return str.str();
}

//--------------------------------------------------------------------------------------
// Returns a wstring containing the tracking data for the green ball
//--------------------------------------------------------------------------------------
std::wstring Tracker::getGreenTrackerString() {
    return getTrackerWcharString("Green", tracker_green, size_green);
}

//--------------------------------------------------------------------------------------
// Returns a wstring containing the tracking data for the red ball
//--------------------------------------------------------------------------------------
std::wstring Tracker::getRedTrackerString() {
    return getTrackerWcharString("Red", tracker_red, size_red);
}

//--------------------------------------------------------------------------------------
// Returns a wstring containing the tracking data for a provided object
//--------------------------------------------------------------------------------------
std::wstring Tracker::getTrackerWcharString(std::string col, cv::Point point, cv::Size size) {
    std::string result_str = getTrackerString(col, point, size);
    return std::wstring(result_str.begin(), result_str.end());
}
