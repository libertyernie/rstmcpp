#pragma once

#include <cstdint>
#include "ProgressTracker.h"

namespace RSTMCPP {
	namespace AudioConverter {
		void CalcCoefs(int16_t* source, int samples, int16_t* dest, ProgressTracker* progress);

		void EncodeBlock(int16_t* source, int samples, uint8_t* dest, int16_t* coefs);

		//Make sure source includes the yn values (16 samples total)
		void EncodeChunk(int16_t* source, int samples, uint8_t* dest, int16_t* coefs);

		void Something10(double** bufferArray, int mask, double** multiBuffer, int multiIndex, double val);

		double Something11(double* source1, double* source2);

		void Something12(double* source, double* dest);

		void Something9(double** bufferArray, double* buffer2, int mask, double value);

		int Something8(double* src, double* omgBuffer, double* dst, double* outVar);

		void Something7(double* src, double* dst);


		void Something1(short* source, double* dest);

		void Something2(short* source, double** outList);

		bool Something3(double** outList, int* dest, int* unk);

		void Something4(double** outList, int* dest, double* block);

		int Something5(double* block, double* buffer);

		void Something6(double* omgBuffer, double* buffer);
	}
}