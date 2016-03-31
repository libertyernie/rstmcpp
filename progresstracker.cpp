#include "progresstracker.h"
#include <iostream>

using std::cout;
using std::endl;

using namespace rstmcpp;

ProgressTracker::ProgressTracker() {
	this->cancelled = false;
}

void ProgressTracker::begin(float min, float max, float current) {
	this->minValue = min;
	this->maxValue = max;
	this->currentValue = current;
	cout << "\r[";
	for (int i = 0; i < 77; i++) cout << ' ';
	cout << ']';
	cout.flush();
}

void ProgressTracker::cancel() {
	this->cancelled = true;
	cout << endl;
}

void ProgressTracker::finish() {
	cout << endl;
}

void ProgressTracker::update(float value) {
	if (this->cancelled) return;
	this->currentValue = value;

	cout << "\r[";
	float width = (80 - 3) * this->currentValue / this->maxValue;
	if (width > 77) width = 77;
	for (int i = 0; i < width; i++) cout << '#';
	cout.flush();
}
