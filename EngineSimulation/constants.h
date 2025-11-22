#pragma once
#include <opencv2/opencv.hpp>

const double FUEL_CAPACITY = 20000.0;
const double AMBIENT_TEMP = 20.0;
const double N1_MAX_RATED = 40000.0;
const double N1_MAX = N1_MAX_RATED * 1.25;
const double EGT_MAX = 1200.0;
const double FUEL_FLOW_MAX = 50.0;
const double N1_STABLE_THRESHOLD = 0.95 * N1_MAX_RATED;
const double STEP = 0.005;

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 700;
const cv::Scalar COLOR_WHITE(255, 255, 255);
const cv::Scalar COLOR_BLACK(0, 0, 0);
const cv::Scalar COLOR_RED(0, 0, 255);
const cv::Scalar COLOR_AMBER(0, 165, 255);
const cv::Scalar COLOR_YELLOW(0, 255, 255);
const cv::Scalar COLOR_GREEN(0, 255, 0);
const cv::Scalar COLOR_BLUE(255, 0, 0);
const cv::Scalar COLOR_GREY(100, 100, 100);

