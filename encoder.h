#pragma once

#include <cstdint>
#include "pcm16.h"
#include "rstm.h"
#include "progresstracker.h"

namespace rstmcpp {
	namespace encoder {
		RSTMHeader* encode_rstm(pcm16::PCM16* stream, ProgressTracker* progress, int* sizeOut);
	}
}
