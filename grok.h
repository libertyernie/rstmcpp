#pragma once
void DSPCorrelateCoefs(const short* source, int samples, short* coefsOut);
void DSPEncodeFrame(short* pcmInOut, int sampleCount, unsigned char* adpcmOut, const short* coefsIn);
