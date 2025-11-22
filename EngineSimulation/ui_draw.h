#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <map>
#include <string>
#include "engine.h"
#include "ui.h"

// UI »æÖÆº¯Êý
void drawUI(cv::Mat& canvas, const std::vector<Gauge>& gauges, const std::map<std::string, Indicator>& indicators, const std::map<std::string, TriangleButton>& thrust_buttons, const Engine& engine, const AlertManager& alert_manager);
void drawGauges(cv::Mat& canvas, const std::vector<Gauge>& gauges, const Engine& engine);
void drawButtons(cv::Mat& canvas, const Engine& engine, const std::map<std::string, TriangleButton>& thrust_buttons);
void drawFuelInfo(cv::Mat& canvas, const Engine& engine);
void drawAllIndicators(cv::Mat& canvas, const std::map<std::string, Indicator>& indicators);
void drawStatusMessage(cv::Mat& canvas, const Engine& engine);
