#pragma once

#include "pcm16.h"

namespace rstmcpp {
	namespace pcm16 {
		namespace wavfactory {
			PCM16* from_file(FILE* file);

			int get_size(const PCM16* lwav);
			void export_to_ptr(const PCM16* lwav, void* dest, int size);
		}
	}
}