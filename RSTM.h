#pragma once

#include <cstring>

#include "ssbbcommon.h"
#include "RWAV.h"

namespace RSTMCPP
{
	struct HEADHeader;
	struct ADPCHeader;
	struct RSTMDATAHeader;

	struct RSTMHeader
	{
		NW4RCommonHeader _header;
		be_int32_t _headOffset;
		be_int32_t _headLength;
		be_int32_t _adpcOffset;
		be_int32_t _adpcLength;
		be_int32_t _dataOffset;
		be_int32_t _dataLength;

		void Set(int headLen, int adpcLen, int dataLen)
		{
			int len = 0x40;

			//Set header
			_header._tag = 0x4D545352;
			_header._endian = 0xFEFF;
			_header._version = 0x100;
			_header._firstOffset = 0x40;
			_header._numEntries = 2;

			//Set offsets/lengths
			_headOffset = len;
			_headLength = headLen;
			_adpcOffset = (len += headLen);
			_adpcLength = adpcLen;
			_dataOffset = (len += adpcLen);
			_dataLength = dataLen;

			_header._length = len + dataLen;

			//Fill padding
			memset((uint8_t*)(&_header) + 0x28, 0, 0x18);
		}

		HEADHeader* HEADData() { return (HEADHeader*)((uint8_t*)&_header + _headOffset); }
		ADPCHeader* ADPCData() { return (ADPCHeader*)((uint8_t*)&_header + _adpcOffset); }
		RSTMDATAHeader* DATAData() { return (RSTMDATAHeader*)((uint8_t*)&_header + _dataOffset); }
	};

	struct StrmDataInfo;

	struct HEADHeader
	{
		le_uint32_t _tag;
		be_int32_t _size;
		RuintCollection _entries;

		void Set(int size, int channels)
		{
			RuintList* list;
			uint8_t* offset = _entries.Address();
			int dataOffset = 0x60 + (channels * 8);

			_tag = 0x44414548;
			_size = size;

			//Set entry offsets
			_entries.Entries()[0] = 0x18;
			_entries.Entries()[1] = 0x4C;
			_entries.Entries()[2] = 0x5C;

			//Audio info
			//HEADPart1* part1 = Part1;

			//Set single channel info
			list = Part2();
			list->_numEntries = 1; //Number is little-endian
			list->Entries()[0] = 0x58;
			*(AudioFormatInfo*)list->Get(offset, 0) = AudioFormatInfo(2, 0, 1, 0);

			//Set adpcm infos
			list = Part3();
			list->_numEntries = channels; //little-endian
			for (int i = 0; i < channels; i++)
			{
				//Set initial pointer
				list->Entries()[i] = dataOffset;

				//Set embedded pointer
				int rx = dataOffset + 8;
				*(ruint*)(offset + dataOffset) = ruint(rx);
				dataOffset += 8;

				//Set info
				//*(ADPCMInfo*)(offset + dataOffset) = info[i];
				dataOffset += ADPCMInfo::Size;

				//Set padding
				//*(short*)(offset + dataOffset) = 0;
				//dataOffset += 2;
			}

			//Fill remaining
			int* p = (int*)(offset + dataOffset);
			for (dataOffset += 8; dataOffset < size; dataOffset += 4)
				*p++ = 0;
		}

		StrmDataInfo* Part1() { return (StrmDataInfo*)_entries.getPtr(0); } //Audio info
		RuintList* Part2() { return (RuintList*)_entries.getPtr(1); } //ADPC block flags?
		RuintList* Part3() { return (RuintList*)_entries.getPtr(2); } //ADPCMInfo array, one for each channel?

		ADPCMInfo* GetChannelInfo(int index)
		{
			ruint* r = (ruint*)Part3()->Get(_entries.Address(), index);
			return (ADPCMInfo*)r->Offset(_entries.Address());
		}
	};

	struct StrmDataInfo
	{
		AudioFormatInfo _format;
		be_uint16_t _sampleRate; //0x7D00
		be_uint16_t _blockHeaderOffset;
		be_uint32_t _loopStartSample;
		be_uint32_t _numSamples;
		be_uint32_t _dataOffset;
		be_uint32_t _numBlocks;
		be_uint32_t _blockSize;
		be_uint32_t _samplesPerBlock; //0x3800
		be_uint32_t _lastBlockSize; //Without padding
		be_uint32_t _lastBlockSamples;
		be_uint32_t _lastBlockTotal; //Includes padding
		be_uint32_t _dataInterval; //0x3800
		be_uint32_t _bitsPerSample;
	};

	struct ADPCHeader
	{
		le_uint32_t _tag;
		be_int32_t _length;
		int32_t _pad1, _pad2;

		void Set(int length)
		{
			_tag = 0x43504441;
			_length = length;
			_pad1 = _pad2 = 0;
		}

		void* Data() { return ((uint8_t*)&_tag) + 0x10; }
	};

	struct RSTMDATAHeader
	{
		le_uint32_t _tag;
		be_int32_t _length;
		be_int32_t _dataOffset;
		int32_t _pad1;

		void Set(int length)
		{
			_tag = 0x41544144;
			_length = length;
			_dataOffset = 0x18;
			_pad1 = 0;
		}

		void* Data() { return ((uint8_t*)&_tag) + 8 + _dataOffset; }
	};
}
