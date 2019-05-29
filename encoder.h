#pragma once

#include <cstdint>
#include "pcm16.h"
#include "cwav.h"
#include "cstm.h"
#include "rstm.h"
#include "progresstracker.h"

namespace rstmcpp {
	namespace encoder {

        enum FileType : int {
            RSTM = 0,
            CSTM = 1,
            CWAV = 2,
            BFSTM = 3
        };

        char* encode(pcm16::PCM16* stream, ProgressTracker* progress, int* sizeOut, int type);
        CWAVHeader* encode_cwav(pcm16::PCM16* stream, ProgressTracker* progress, int* sizeOut);
        CSTMHeader* encode_cstm(pcm16::PCM16* stream, ProgressTracker* progress, int* sizeOut);
		RSTMHeader* encode_rstm(pcm16::PCM16* stream, ProgressTracker* progress, int* sizeOut);
	}
}
