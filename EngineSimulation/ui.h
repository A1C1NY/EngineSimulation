#pragma once
#include <string>
#include <cmath>
#include <vector>
#include <map>
#include <chrono>
#include <deque>
#include <graphics.h>
#include <Windows.h>

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 700;

const COLORREF COLOR_WHITE = RGB(255, 255, 255);
const COLORREF COLOR_BLACK = RGB(0, 0, 0);
const COLORREF COLOR_RED = RGB(255, 0, 0);
const COLORREF COLOR_AMBER = RGB(255, 165, 0); // 琥珀色 (Caution/Alert)
const COLORREF COLOR_YELLOW = RGB(255, 255, 0); // 黄色，但语义上是警戒值
const COLORREF COLOR_GREEN = RGB(0, 255, 0);
const COLORREF COLOR_BLUE = RGB(0, 0, 255);
const COLORREF COLOR_LIGHT_GREY = RGB(180, 180, 180);
const COLORREF COLOR_GREY = RGB(100, 100, 100);

class Gauge {
public:
	Gauge(POINT center, int radius, const std::string& label, double max_value);
	void draw(double value, double baseValue, double cautionStart, double warningStart) const;
private:
    POINT center;
    int radius;
    std::string label;
    double maxVal;
	double valueToAngle(double value) const; // 计算指针角度
};

class Indicator {
public:
    Indicator(const RECT& position, const std::string& text);
    void draw() const;
    void update();
    void setActive(const COLORREF newColor = COLOR_AMBER);
    void deactivate();

    std::string getText() const {return label;}
    RECT getPosition() const {return pos;}

private:
    RECT pos;
    std::string label;
    bool isActive;
	COLORREF color;
	double lastActivatedTime;
};

class TriangleButton {
public:
	TriangleButton(const RECT& rect, bool direction); // true代表上，false代表下
    void draw() const;
    bool isClicked(int x, int y) const;
    void setEnabled(bool enabled);

private:
    RECT rect;
    bool direction; // true for up, false for down
    bool enabled;
};


struct Alert {
    std::string message;
    COLORREF color;
    double timestamp; 
};

class AlertInfo {
public:
	AlertInfo() : currentAlert({ "", COLOR_BLACK, 0.0}) {}

    void triggerAlert(const std::string& message, COLORREF color);
    void update();
	const Alert& getCurrentAlert() const { return currentAlert; } // const版本
	Alert& getCurrentAlert() { return currentAlert; } // 非const版本
	void drawHistory() const;

private:
    Alert currentAlert;
    std::deque<Alert> alertHistory;
    double getCurrentTime() const;
}; 


void initializeIndicators(std::map<std::string, Indicator>& indicators);
void initializeButtons(std::map<std::string, TriangleButton>& thrustButtons);
void handleMouseClick(int x, int y, void* enginePtr, void* startFlagPtr, void* stopFlagPtr, void* thrustButtonsPtr);
void initializeUI(const std::string& windowName, void* enginePtr, void* startFlagPtr, void* stopFlagPtr, void* thrustButtonsPtr);
void fixConsoleWindow();