#include "ProgressTracker.h"
#include <iostream>

using std::cout;
using std::endl;

RSTMCPP::ProgressTracker::ProgressTracker() {
	this->cancelled = false;
}

/*
		public void Begin(float min, float max, float current) {
*/
void RSTMCPP::ProgressTracker::begin(float min, float max, float current) {
	this->minValue = min;
	this->maxValue = max;
	this->currentValue = current;
	cout << "\r[";
	for (int i = 0; i < 77; i++) cout << ' ';
	cout << ']';
	cout.flush();
}

void RSTMCPP::ProgressTracker::cancel() {
	this->cancelled = true;
	cout << endl;
}

void RSTMCPP::ProgressTracker::finish() {
	cout << endl;
}

void RSTMCPP::ProgressTracker::update(float value) {
	if (this->cancelled) return;
	this->currentValue = value;

	cout << "\r[";
	float width = (80 - 3) * this->currentValue / this->maxValue;
	for (int i = 0; i < width; i++) cout << '#';
	cout.flush();
}
