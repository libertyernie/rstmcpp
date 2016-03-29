#pragma once

namespace RSTMCPP {
	class ProgressTracker {
	public:
		ProgressTracker();

		void update(float value);
		void begin(float min, float max, float current);
		void finish();
		void cancel();

		float minValue;
		float maxValue;
		float currentValue;
		bool cancelled;
	};
}
