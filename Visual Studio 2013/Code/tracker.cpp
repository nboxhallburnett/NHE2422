#include "tracker.h"

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


Tracker::~Tracker() {

}

bool Tracker::InitCamera() {
    if (!webcam.isOpened()) {
        std::cout << "Cannot open the video cam" << std::endl;
        return false;
    }
    return true;
}

void Tracker::UpdateCamera() {

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

void Tracker::ShowCameraWindow() {
    std::string result_green = getTrackerString("Green", tracker_green, size_green);
    cv::putText(frame, result_green, cv::Point(10, 20), 1, 1, cv::Scalar(0, 255, 255), 1);
    cv::putText(frame, "+", tracker_green, 1, 3, cv::Scalar(0, 255, 0), 2);

    std::string result_red = getTrackerString("Red", tracker_red, size_red);
    cv::putText(frame, result_red, cv::Point(10, 40), 1, 1, cv::Scalar(0, 255, 255), 1);
    cv::putText(frame, "+", tracker_red, 1, 3, cv::Scalar(0, 0, 255), 2);

    // Show the camera tracker window
    cv::imshow("Green & Red", frame);
}

std::string Tracker::getTrackerString(std::string col, cv::Point point, cv::Size size) {
    std::ostringstream str;
    str << col << " Ball: x=" << point.x << " y=" << point.y << " size: " << size.area();
    return str.str();
}

std::wstring Tracker::getGreenTrackerString() {
    return getTrackerWcharString("Green", tracker_green, size_green);
}

std::wstring Tracker::getRedTrackerString() {
    return getTrackerWcharString("Red", tracker_red, size_red);
}

std::wstring Tracker::getTrackerWcharString(std::string col, cv::Point point, cv::Size size) {
    std::string result_str = getTrackerString(col, point, size);
    return std::wstring(result_str.begin(), result_str.end());
}
