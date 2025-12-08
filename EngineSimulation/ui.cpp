#include "ui.h"
#include "log.h"
#include <iomanip>
#include <sstream>
#include <graphics.h>
using namespace std;
#define pi 3.14159265358979323846

Gauge::Gauge(POINT center, int radius, const std::string& label, double maxVal)
	: center(center), radius(radius), label(label), maxVal(maxVal) {
};

double Gauge::valueToAngle(double value) const {
	// 将输入值归一化到 0.0 到 1.0 之间
	double percentage = value / maxVal;
	if (percentage < 0) percentage = 0;

	// 将百分比映射到 0 到 210 度的角度范围
	return - 180.0 * percentage / 180 * pi;
}


void Gauge::draw(double value, double baseValue, double cautionStart, double warningStart) const {
	POINT Gaugecenter = center;
	int GaugeRadius = radius;

	// 1. 绘制仪表背景 (从 210° 到 0° 的空心扇形) 要转化弧度制
	setlinecolor(COLOR_WHITE);
	setfillcolor(COLOR_BLACK);
	pie(Gaugecenter.x - GaugeRadius, Gaugecenter.y - GaugeRadius,
		Gaugecenter.x + GaugeRadius, Gaugecenter.y + GaugeRadius,
		double(160) / 180 * pi , 0);

	if (isnan(value)) {
		// 在数字读数位置绘制 "NaN"
		setfillcolor(COLOR_BLACK);
		solidrectangle(Gaugecenter.x + radius * 0.4, Gaugecenter.y - radius * 0.2,
			Gaugecenter.x + radius * 1.0, Gaugecenter.y);
		settextcolor(COLOR_RED);
		setbkmode(TRANSPARENT);
		settextstyle(20, 0, _T("Consolas"));
		outtextxy(Gaugecenter.x + radius * 0.42, Gaugecenter.y - radius * 0.15, L"NaN");
	}
	else {
		// 2. 根据数值设定颜色
		COLORREF currentColor = COLOR_LIGHT_GREY; // 默认白色
		if (value >= warningStart) {
			currentColor = COLOR_RED;
		}
		else if (value >= cautionStart) {
			currentColor = COLOR_AMBER;
		}

		// 3. 绘制代表当前值的实心扇形
		double fillAngle = valueToAngle(value);

		if (fillAngle != 0) {
			setlinecolor(currentColor);
			setfillcolor(currentColor);

			solidpie(Gaugecenter.x - GaugeRadius * 0.9, Gaugecenter.y - GaugeRadius * 0.9,
				Gaugecenter.x + GaugeRadius * 0.95, Gaugecenter.y + GaugeRadius * 0.9,
				fillAngle, 0);
		}

		// 4. 绘制数字读数
		settextcolor(currentColor);
		setbkmode(TRANSPARENT);
		settextstyle(20, 0, _T("Consolas"));
		double displayValue = value;

		if (label.find("N1") != string::npos) {
			displayValue = (value / 40000.0) * 100.0; // N1显示百分比
			wostringstream wss;
			wss << fixed << setprecision(1) << displayValue;
			wstring valStr = wss.str();
			outtextxy(Gaugecenter.x + radius * 0.42, Gaugecenter.y - radius * 0.3, valStr.c_str());
		}
		else {
			wostringstream wss;
			wss << fixed << setprecision(0) << displayValue;
			wstring valStr = wss.str();
			outtextxy(Gaugecenter.x + radius * 0.42, Gaugecenter.y - radius * 0.3, valStr.c_str());
		}
	}
	// 5. 绘制仪表标签
	settextcolor(COLOR_WHITE);
	settextstyle(20, 0, _T("Consolas"));
	outtextxy(Gaugecenter.x - radius * 0.2, Gaugecenter.y - radius * 0.7, wstring(label.begin(), label.end()).c_str());
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
		if (currentTime - lastActivatedTime >= 2) {
			isActive = false;
			color = COLOR_GREY;
		}
	}
}

void Indicator::setActive(const COLORREF newColor) {
	double currentTime = getCurrenTimeSeconds();

	// 如果已激活且颜色相同，只刷新时间戳（保持持续显示）
	if (isActive && color == newColor) {
		lastActivatedTime = currentTime;
		return;
	}

	if (!isActive) {
		// 指示灯未激活，直接激活
		isActive = true;
		color = newColor;
		lastActivatedTime = currentTime;
	}
	else {
		// 已激活，按优先级判断是否更新颜色
		if (newColor == COLOR_RED) {
			color = COLOR_RED;
			lastActivatedTime = currentTime;
		}
		else if (newColor == COLOR_AMBER && color != COLOR_RED) {
			color = COLOR_AMBER;
			lastActivatedTime = currentTime;
		}
		else if (newColor == COLOR_WHITE && color != COLOR_RED && color != COLOR_AMBER) {
			color = newColor;
			lastActivatedTime = currentTime;
		}
		else {
			// 新颜色优先级不高于当前颜色，只刷新时间戳
			lastActivatedTime = currentTime;
		}
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

double AlertInfo::getCurrentTime() const {
	static auto startTime = chrono::high_resolution_clock::now();
	auto currentTime = chrono::high_resolution_clock::now();
	return chrono::duration<double>(currentTime - startTime).count();
}

void AlertInfo::triggerAlert(const string& message, COLORREF color) {
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

void AlertInfo::update() {
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

void AlertInfo::drawHistory() const {
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


void initializeIndicators(map<string, Indicator>& indicators) {
	// map<string, Indicator>直接用string找到对应的Indicator
	indicators.clear();
	const int w = 199, h = 25, xStart = 20, yStart = 420, xOffset = 110, yOffset = 30;

	indicators.emplace("N1_L_S1_Fail", Indicator({ xStart, yStart, xStart + w, yStart + h }, "N1 L S1 Fail"));
	indicators.emplace("N1_L_S2_Fail", Indicator({ xStart + xOffset, yStart, xStart + xOffset + w, yStart + h }, "N1 L S2 Fail"));
	indicators.emplace("EGT_L_S1_Fail", Indicator({ xStart + 2 * xOffset, yStart, xStart + 2 * xOffset + w, yStart + h }, "EGT L S1 Fail"));
	indicators.emplace("EGT_L_S2_Fail", Indicator({ xStart + 3 * xOffset, yStart, xStart + 3 * xOffset + w, yStart + h }, "EGT L S2 Fail"));
	indicators.emplace("FuelResFail", Indicator({ xStart + 4 * xOffset, yStart, xStart + 4 * xOffset + w, yStart + h }, "Fuel Res Fail"));

	indicators.emplace("N1_R_S1_Fail", Indicator({ xStart, yStart + yOffset, xStart + w, yStart + yOffset + h }, "N1 R S1 Fail"));
	indicators.emplace("N1_R_S2_Fail", Indicator({ xStart + xOffset, yStart + yOffset, xStart + xOffset + w, yStart + yOffset + h }, "N1 R S2 Fail"));
	indicators.emplace("EGT_R_S1_Fail", Indicator({ xStart + 2 * xOffset, yStart + yOffset, xStart + 2 * xOffset + w, yStart + yOffset + h }, "EGT R S1 Fail"));
	indicators.emplace("EGT_R_S2_Fail", Indicator({ xStart + 3 * xOffset, yStart + yOffset, xStart + 3 * xOffset + w, yStart + yOffset + h }, "EGT R S2 Fail"));
	indicators.emplace("FuelFlowFail", Indicator({ xStart + 4 * xOffset, yStart + yOffset, xStart + 4 * xOffset + w, yStart + yOffset + h }, "Fuel Flow Fail"));

	indicators.emplace("LowFuel", Indicator({ xStart, yStart + 2 * yOffset, xStart + w, yStart + 2 * yOffset + h }, "Low Fuel"));
	indicators.emplace("N1SFail", Indicator({ xStart + xOffset, yStart + 2 * yOffset, xStart + xOffset + w, yStart + 2 * yOffset + h }, "N1 Sys Fail"));
	indicators.emplace("OverFF", Indicator({ xStart + 2 * xOffset, yStart + 2 * yOffset, xStart + 2 * xOffset + w, yStart + 2 * yOffset + h }, "Over FF"));
	indicators.emplace("OverSpd1", Indicator({ xStart + 3 * xOffset, yStart + 2 * yOffset, xStart + 3 * xOffset + w, yStart + 2 * yOffset + h }, "OverSpd L"));
	indicators.emplace("OverSpd2", Indicator({ xStart + 4 * xOffset, yStart + 2 * yOffset, xStart + 4 * xOffset + w, yStart + 2 * yOffset + h }, "OverSpd R"));

	indicators.emplace("EGTSFail", Indicator({ xStart, yStart + 3 * yOffset, xStart + w, yStart + 3 * yOffset + h }, "EGT Sys Fail"));
	indicators.emplace("OverTemp1", Indicator({ xStart + xOffset, yStart + 3 * yOffset, xStart + xOffset + w, yStart + 3 * yOffset + h }, "OverTemp S1"));
	indicators.emplace("OverTemp2", Indicator({ xStart + 2 * xOffset, yStart + 3 * yOffset, xStart + 2 * xOffset + w, yStart + 3 * yOffset + h }, "OverTemp S2"));
	indicators.emplace("OverTemp3", Indicator({ xStart + 3 * xOffset, yStart + 3 * yOffset, xStart + 3 * xOffset + w, yStart + 3 * yOffset + h }, "OverTemp C1"));
	indicators.emplace("OverTemp4", Indicator({ xStart + 4 * xOffset, yStart + 3 * yOffset, xStart + 4 * xOffset + w, yStart + 3 * yOffset + h }, "OverTemp C2"));
}

void initializeButtons(std::map<std::string, TriangleButton>& thrustButtons) {
	thrustButtons.clear();
	thrustButtons.emplace("ThrustUp", TriangleButton({ 400, 100, 400 + 20, 100 + 14 }, true)); // Up
	thrustButtons.emplace("ThrustDown", TriangleButton({ 400, 116, 400 + 20, 116 + 14 }, false)); // Down
}

void handleMouseClick(int x, int y, void* enginePtr, void* startFlagPtr, void* stopFlagPtr, void* thrustButtonsPtr) {
	if (x >= 150 && x <= 250 && y >= 380 && y <= 410) {
		if (startFlagPtr) {
			*(bool*)startFlagPtr = true;
		}
	}
	else if (x >= 260 && x <= 360 && y >= 380 && y <= 410) {
		if (stopFlagPtr) {
			*(bool*)stopFlagPtr = true;
		}
	}
	// 推力按钮
	else if (thrustButtonsPtr && enginePtr) {
		std::map<std::string, TriangleButton>* thrust_buttons = (std::map<std::string, TriangleButton>*)thrustButtonsPtr;

		// 确保 map 中有这些键
		if (thrust_buttons->count("ThrustUp") && thrust_buttons->at("ThrustUp").isClicked(x, y)) {
			((Engine*)enginePtr)->increaseThrust();
		}
		else if (thrust_buttons->count("ThrustDown") && thrust_buttons->at("ThrustDown").isClicked(x, y)) {
			((Engine*)enginePtr)->decreaseThrust();
		}
	}
}

void initializeUI(const string& windowName, void* enginePtr, void* startFlagPtr, void* stopFlagPtr, void* thrustButtonsPtr) {
	// EasyX Initialization
	initgraph(WINDOW_WIDTH, WINDOW_HEIGHT, EW_SHOWCONSOLE); // EW_SHOWCONSOLE keeps the console window open
	setbkcolor(BLACK);
	cleardevice();
}

