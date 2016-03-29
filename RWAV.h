#pragma once

#include "endian.h"

using namespace RSTMCPP::endian;

namespace RSTMCPP
{
	/*
	public unsafe struct AudioFormatInfo
	*/
	struct AudioFormatInfo
	{
		uint8_t _encoding;
		uint8_t _looped;
		uint8_t _channels;
		uint8_t _sampleRate24;

		AudioFormatInfo(byte encoding, byte looped, byte channels, byte unk)
		{
			_encoding = encoding; _looped = looped; _channels = channels; _sampleRate24 = unk;
		}
	};

	/*
	public unsafe struct ADPCMInfo
	*/
	struct ADPCMInfo
	{
		static const int Size = 0x30;

		be_uint16_t _coefs[16];

		be_uint16_t _gain;
		be_int16_t _ps; //Predictor and scale. This will be initialized to the predictor and scale value of the sample's first frame.
		be_int16_t _yn1; //History data; used to maintain decoder state during sample playback.
		be_int16_t _yn2; //History data; used to maintain decoder state during sample playback.
		be_int16_t _lps; //Predictor/scale for the loop point frame. If the sample does not loop, this value is zero.
		be_int16_t _lyn1; //History data for the loop point. If the sample does not loop, this value is zero.
		be_int16_t _lyn2; //History data for the loop point. If the sample does not loop, this value is zero.
		int16_t _pad;
	};
}
