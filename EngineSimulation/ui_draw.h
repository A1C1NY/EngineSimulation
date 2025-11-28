#pragma once
#include <graphics.h>
#include <vector>
#include <map>
#include <string>
#include "engine.h"
#include "ui.h"

// UI »æÖÆº¯Êý
void drawUI(const std::vector<Gauge>& gauges, const std::map<std::string, Indicator>& indicators, const std::map<std::string, TriangleButton>& thrust_buttons, const Engine& engine, const AlertManager& alert_manager);
void drawGauges(const std::vector<Gauge>& gauges, const Engine& engine);
void drawButtons(const Engine& engine, const std::map<std::string, TriangleButton>& thrust_buttons);
void drawFuelInfo(const Engine& engine);
void drawAllIndicators(const std::map<std::string, Indicator>& indicators);
void drawStatusMessage(const Engine& engine);
