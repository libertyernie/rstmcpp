#pragma once

#include <cstdint>
#include "PCM16.h"
#include "ProgressTracker.h"

namespace RSTMCPP {
	namespace RSTMConverter {
		void* Encode(WAV::PCM16Audio* stream, ProgressTracker* progress, int* sizeOut);
	}
}
