#include "ui.h"
#include "log.h"
#include <iomanip>
#include <sstream>
#include <graphics.h>
using namespace std;

Gauge::Gauge(POINT center, int radius, const std::string& label, double maxVal)
	: center(center), radius(radius), label(label), maxVal(maxVal) {
};

double Gauge::valueToAngle(double value) const {
	double percentage = value / maxVal;
	return 180 * percentage;
}


void Gauge::draw(double value, double baseValue, double cautionStart, double warningStart) const {
	POINT Gaugecenter = center;
	int GaugeRadius = radius;
	int thickness = 1;
	

	// 先画一个空心的扇形（0-225°）
	setlinecolor(COLOR_WHITE);
	setfillcolor(COLOR_BLACK);
	fillpie(Gaugecenter.x - GaugeRadius, Gaugecenter.y - GaugeRadius,
		Gaugecenter.x + GaugeRadius, Gaugecenter.y + GaugeRadius,
		0, 225);

	if (isnan(value)) {
		// 0°半径数显框里写NaN
		setfillcolor(COLOR_BLACK);
		solidrectangle(Gaugecenter.x + radius, Gaugecenter.y - radius * 0.2,
			Gaugecenter.x + radius * 0.4, Gaugecenter.y);
		settextcolor(COLOR_RED);
		setbkmode(TRANSPARENT);
		settextstyle(20, 0, _T("Consolas"));
		outtextxy(Gaugecenter.x + radius * 0.42, Gaugecenter.y - radius * 0.15, L"NaN");
	}
	else {
		solidrectangle(Gaugecenter.x + radius, Gaugecenter.y - radius * 0.2,
			Gaugecenter.x + radius * 0.4, Gaugecenter.y);
		double currentAngle = valueToAngle(value);
		COLORREF currentColor = COLOR_GREY;
		// 根据数值设定颜色
		if (value >= warningStart) {
			currentColor = COLOR_RED;
		}
		else if (value >= cautionStart) {
			currentColor = COLOR_AMBER;
		}

		setlinecolor(currentColor);
		setfillcolor(currentColor);
		fillpie(Gaugecenter.x - GaugeRadius * 0.9, Gaugecenter.y - GaugeRadius * 0.9,
			Gaugecenter.x + GaugeRadius * 0.9, Gaugecenter.y + GaugeRadius * 0.9,
			0, currentAngle);

		// 写数字
		setfillcolor(COLOR_BLACK);
		solidrectangle(Gaugecenter.x + radius, Gaugecenter.y - radius * 0.2,
			Gaugecenter.x + radius * 0.4, Gaugecenter.y);
		settextcolor(currentColor);
		setbkmode(TRANSPARENT);
		settextstyle(20, 0, _T("Consolas"));
		double displayValue = value;

		if (label.find("N1") != string::npos) {
			displayValue = (value / 40000.0) * 100.0; // N1显示百分比

			// 保留 1 位小数的 wstring
			wostringstream wss;
			wss << fixed << setprecision(1) << displayValue;
			wstring valStr = wss.str();

			outtextxy(Gaugecenter.x + radius * 0.42, Gaugecenter.y - radius * 0.15, valStr.c_str());
		}
		else {
			// 保留到整数
			wostringstream wss;
			wss << fixed << setprecision(0) << displayValue;
			wstring valStr = wss.str();
			outtextxy(Gaugecenter.x + radius * 0.42, Gaugecenter.y - radius * 0.15, valStr.c_str());
		}
	}
	// 仪表标签
	settextcolor(COLOR_WHITE);
	settextstyle(20, 0, _T("Consolas"));
	outtextxy(Gaugecenter.x - radius * 0.3, Gaugecenter.y - radius * 0.3, wstring(label.begin(), label.end()).c_str());
}

Indicator::Indicator(const RECT& position, const std::string& text)
	: pos(position), label(text), isActive(false), color(COLOR_GREY), lastActivatedTime(0.0) {}

void Indicator::draw() const {
	// 画边框
	setlinecolor(COLOR_WHITE);
	rectangle(pos.left, pos.top, pos.right, pos.bottom);
	// 填充颜色
	setfillcolor(color);
	solidrectangle(pos.left + 1, pos.top + 1, pos.right - 1, pos.bottom - 1);
	// 写文字
	settextcolor(isActive ? COLOR_BLACK : COLOR_WHITE);
	setbkmode(TRANSPARENT);
	settextstyle(20, 0, _T("Consolas"));
	int textWidth = textwidth(wstring(label.begin(), label.end()).c_str());
	int textHeight = textheight(wstring(label.begin(), label.end()).c_str());
	int x = (pos.left + pos.right - textWidth) / 2;
	int y = (pos.top + pos.bottom - textHeight) / 2;
	outtextxy(x, y, wstring(label.begin(), label.end()).c_str());
}

void Indicator::update() {
	if (isActive) {
		double currentTime = getCurrenTimeSeconds();
		if (currentTime - lastActivatedTime >= 2.5) {
			isActive = false;
		}
	}
}

void Indicator::setActive(const COLORREF newColor) {
	double currentTime = getCurrenTimeSeconds();

	if (isActive) {
		if (newColor == COLOR_RED) {
			color = COLOR_RED;
			lastActivatedTime = currentTime;
		}
		else if (color == COLOR_RED) {
			lastActivatedTime = currentTime; // 保持红色状态时间
		}
		else if (newColor == COLOR_AMBER) {
			color = COLOR_AMBER;
			lastActivatedTime = currentTime;
		}
		else {
			color = newColor;
			lastActivatedTime = currentTime; // 保持当前状态时间
		}
	}
	else {
		isActive = true;
		color = newColor;
		lastActivatedTime = currentTime;
	}
}

TriangleButton::TriangleButton(const RECT& rectangle, bool direction)
	: rect(rectangle), direction(direction), enabled(true) {}

void TriangleButton::draw() const {
	COLORREF fillcolor = enabled ? COLOR_LIGHT_GREY : COLOR_GREY;
	setfillcolor(fillcolor);
	setlinecolor(fillcolor);

	POINT points[3];
	if (direction) {
		// 向上三角形
		points[0] = { (rect.left + rect.right) / 2, rect.top };
		points[1] = { rect.left, rect.bottom };
		points[2] = { rect.right, rect.bottom };
	}
	else {
		// 向下三角形
		points[0] = { rect.left, rect.top };
		points[1] = { rect.right, rect.top };
		points[2] = { (rect.left + rect.right) / 2, rect.bottom };
	}
	fillpolygon(points, 3);
}

bool TriangleButton::isClicked(int x, int y) const {
	if (!enabled) return false;
	return (x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom);
}

void TriangleButton::setEnabled(bool isEnabled) {
	enabled = isEnabled;
}

double AlertManager::getCurrentTime() const {
	static auto startTime = chrono::high_resolution_clock::now();
	auto currentTime = chrono::high_resolution_clock::now();
	return chrono::duration<double>(currentTime - startTime).count();
}

void AlertManager::triggerAlert(const string& message, COLORREF color) {
	double currentTime = getCurrentTime();
	alertHistory.push_back({ message, color, currentTime});

	// 按照Red > Amber > White优先级更新当前警报
	// 只有新警报颜色优先级更高时才更新当前警报
	bool higherPriority = false;
	if (currentAlert.message.empty()) {
		higherPriority = true;
	}
	else if (color == COLOR_RED && currentAlert.color != COLOR_RED) {
		higherPriority = true;
	}
	else if (color == COLOR_AMBER && currentAlert.color == COLOR_WHITE) {
		higherPriority = true;
	}
	else if (color == COLOR_WHITE && currentAlert.color == COLOR_BLACK) {
		higherPriority = true; // WHITE对于无警报是更高优先级
	}

	// 如果优先级更高，或等级相同但是新警报，则更新当前警报
	if (higherPriority || (color == currentAlert.color && message != currentAlert.message)) {
		currentAlert.message = message;
		currentAlert.color = color;
		currentAlert.timestamp = currentTime;
	}
	else {
		// 否则只更新当前警报的最后时间
		currentAlert.timestamp = currentTime;
	}

	if (!message.empty()) {
		bool found = false;
		for (const auto& alert : alertHistory) {
			if (alert.message == message && currentTime - alert.timestamp < 5.0) {
				found = true;
				break;
			}
		}
		if (!found) {
			alertHistory.push_back({ message, color, currentTime});
		}
	}
}

void AlertManager::update() {
	double currentTime = getCurrentTime();
	// 如果当前警报存在且超过5秒未更新，则清除
	if (!currentAlert.message.empty() && (currentTime - currentAlert.timestamp >= 5.0)) {
		currentAlert = { "", COLOR_BLACK, 0.0 };
	}
	// 清理历史警报
	while (!alertHistory.empty() && currentTime - currentAlert.timestamp >= 5.0) {
		alertHistory.pop_front();
	}
}

void AlertManager::drawHistory() const {
	int baseY = WINDOW_HEIGHT - 150;
	int lineHeight = 20;
	int maxLines = 8;
	int cnt = 0;
	for (auto it = alertHistory.begin(); it != alertHistory.end() && cnt < maxLines; ++it, ++cnt) {
		const Alert& alert = *it;
		int x = 50;
		int y = baseY + cnt * lineHeight;
		int w = WINDOW_WIDTH - 100;
		int h = lineHeight - 4;

		settextcolor(alert.color);
		setbkmode(TRANSPARENT);
		settextstyle(16, 0, _T("Consolas"));

		wstring wmsg = wstring(alert.message.begin(), alert.message.end());
		outtextxy(x + 8, y + 2, wmsg.c_str());
	}
}

