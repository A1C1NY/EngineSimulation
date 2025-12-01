#pragma once
#include <string>
#include <cmath>
#include <vector>
#include <map>
#include <chrono>
#include <deque>
#include <graphics.h>

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 700;

const COLORREF COLOR_WHITE = RGB(255, 255, 255);
const COLORREF COLOR_BLACK = RGB(0, 0, 0);
const COLORREF COLOR_RED = RGB(255, 0, 0);
const COLORREF COLOR_AMBER = RGB(255, 165, 0); // 琥珀色 (Caution/Alert)
const COLORREF COLOR_YELLOW = RGB(255, 255, 0); // 黄色，但语义上是警戒值
const COLORREF COLOR_GREEN = RGB(0, 255, 0);
const COLORREF COLOR_BLUE = RGB(0, 0, 255);
const COLORREF COLOR_GREY = RGB(100, 100, 100);

class Gauge {
public:
	Gauge(POINT center, int radius, const std::string& label, double max_value);
	void draw(double value, double base_value, double caution_start, double warning_start) const;
private:
    POINT center;
    int radius;
    std::string label;
    double maxVal;
	double valueToAngle(double value) const; // 计算指针角度
};

class Indicator {
public:
    Indicator(const RECT& position, const string& text);
    void draw() const;
    void update();
    void setActive(const COLORREF new_color = COLOR_AMBER);

    string getText() const {return label;}
    RECT getPosition() const {return pos;}

private:
    RECT pos;
    string label;
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
    double lastTime; 
};

class AlertManager {
public:
	AlertManager() : currentAlert({ "", COLOR_BLACK, 0.0, 0.0 }) {} // 初始化为空警报

    void triggerAlert(const std::string& message, COLORREF color);
    void update();
	void draw() const;
	const Alert& getCurrentAlert() const { return currentAlert; } // const版本
	Alert& getCurrentAlert() { return currentAlert; } // 非const版本
	void drawHistory() const;

private:
    Alert currentAlert;
    std::deque<Alert> alertHistory; // 用于UI显示
    double getCurrentTime() const;
};


void initializeIndicators(std::map<std::string, Indicator>& indicators);
void initializeButtons(std::map<std::string, TriangleButton>& thrust_buttons);
void handleMouseClick(int x, int y, void* engine_ptr, void* start_flag_ptr, void* stop_flag_ptr, void* thrust_buttons_ptr);
void initializeUI(const std::string& window_name, void* engine_ptr, void* start_flag_ptr, void* stop_flag_ptr, void* thrust_buttons_ptr);
void updateIndicators();