#pragma once

#include <cstdio>
#include <stdexcept>
#include "PCM16.h"

namespace RSTMCPP {
	namespace WAV {
		namespace PCM16Factory {
			PCM16Audio* FromFile(FILE* file);

			int exportWavSize(const PCM16Audio* lwav);
			void exportWav(const PCM16Audio* lwav, void* dest, int size);
		}
	}
}