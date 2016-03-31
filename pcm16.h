#pragma once

#include "endian.h"

namespace rstmcpp {
	namespace pcm16 {
		struct PCM16 {
		public:
			int channels;
			int sampleRate;
			rstmcpp::endian::le_int16_t* samples;
			rstmcpp::endian::le_int16_t* samples_pos;
			rstmcpp::endian::le_int16_t* samples_end;

			bool looping;
			rstmcpp::endian::le_int16_t* loop_start;
			rstmcpp::endian::le_int16_t* loop_end;

			PCM16(int channels, int sampleRate, int16_t* sample_data, int sample_count);
			PCM16(int channels, int sampleRate, int16_t* sample_data, int sample_count, int loop_start, int loop_end);

			int getBitsPerSample() { return 16; }
			
			int readSamples(void* destAddr, int numSamplesEachChannel);
			void wrap();

			~PCM16();

		private:
			void initWav(int channels, int sampleRate, int16_t* sample_data, int sample_count, int loop_start, int loop_end);
		};
	}
}
