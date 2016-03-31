#pragma once

#include "pcm16.h"

namespace rstmcpp {
	namespace pcm16 {
		namespace wavfactory {
			PCM16* FromFile(FILE* file);

			int exportWavSize(const PCM16* lwav);
			void exportWav(const PCM16* lwav, void* dest, int size);
		}
	}
}