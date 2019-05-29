#pragma once

#include <cstring>
#include <iostream>

#include "rstm.h"

using std::cerr;
using std::endl;

namespace rstmcpp
{
    /// <summary>
    /// A type/offset pair similar to ruint.
    /// </summary>
    struct CWAVReference {
        enum RefType : uint16_t {
            ReferenceTable = 0x7100,
            SampleData = 0x1F00,
            DSPADPCMInfo = 0x0300,
            InfoBlock = 0x7000,
            DataBlock = 0x7001
        };

        uint16_t _type;
        be_uint16_t _padding;
        int32_t _dataOffset;
    }; 
    /// <summary>
    /// A list of CSTMReferences, beginning with a length value. Similar to RuintList.
    /// </summary>
    struct CWAVReferenceList {
        int32_t _numEntries;

        uint8_t* Address() { return (uint8_t*)&_numEntries; }
        CWAVReference* Entries() { return (CWAVReference*)(Address() + 4); }
    };

    struct CWAVDataInfo
    {
        AudioFormatInfo _format;
        le_uint32_t _sampleRate;
        le_uint32_t _loopStartSample;
        le_uint32_t _numSamples;

        le_uint32_t _pad;
        //le_uint32_t _lastBlockTotal; //Includes padding

        CWAVDataInfo(StrmDataInfo* o)
        {
            _format = o->_format;
            _sampleRate = (le_uint32_t)o->_sampleRate;
            _loopStartSample = (le_uint32_t)o->_loopStartSample;
            _numSamples = (le_uint32_t)o->_numSamples;
            _pad = 0;
        }
    };

    struct CWAVADPCMInfo
    {
        le_uint16_t _coefs[16];

        le_int16_t _ps; //Predictor and scale. This will be initialized to the predictor and scale value of the sample's first frame.
        le_int16_t _yn1; //History data; used to maintain decoder state during sample playback.
        le_int16_t _yn2; //History data; used to maintain decoder state during sample playback.
        le_int16_t _lps; //Predictor/scale for the loop point frame. If the sample does not loop, this value is zero.
        le_int16_t _lyn1; //History data for the loop point. If the sample does not loop, this value is zero.
        le_int16_t _lyn2; //History data for the loop point. If the sample does not loop, this value is zero.

        CWAVADPCMInfo()
        {
            for (int i = 0; i < 16; i++)
                _coefs[i] = 0;

            _ps = 0;
            _yn1 = 0;
            _yn2 = 0;
            _lps = 0;
            _lyn1 = 0;
            _lyn2 = 0;
        }

        CWAVADPCMInfo(ADPCMInfo* o)
        {
            for (int i = 0; i < 16; i++)
                _coefs[i] = (le_uint16_t)o->_coefs[i];

            _ps = (le_int16_t)o->_ps;
            _yn1 = (le_int16_t)o->_yn1;
            _yn2 = (le_int16_t)o->_yn2;
            _lps = (le_int16_t)o->_lps;
            _lyn1 = (le_int16_t)o->_lyn1;
            _lyn2 = (le_int16_t)o->_lyn2;
        }
    };

    struct CWAVChannelInfo {
        CWAVReference _samples;
        CWAVReference _infoRef;
        uint32_t _pad;
        CWAVADPCMInfo _adpcmInfo;

        void Set(int index, be_uint32_t lbSize) {
            _samples._dataOffset = lbSize * index + 24;
            _samples._type = CWAVReference::RefType::SampleData + index;
            _infoRef._dataOffset = 20;
            _infoRef._type = CWAVReference::RefType::DSPADPCMInfo + index;
            _pad = 0;
        }
    };

    struct CWAVINFOHeader
    {
        le_uint32_t _tag;
        le_int32_t _size;
        CWAVDataInfo _dataInfo;

        CWAVReferenceList* ChannelInfoRefTable() {
            return (CWAVReferenceList*)(uint8_t*)(&_dataInfo + 1);
        }

        void* ChannelInfoRefTableEnd() {
            CWAVReferenceList* prevTable = ChannelInfoRefTable();
            int count = prevTable->_numEntries;
            if (count == 0) cerr << "Channel info's ref table must be populated before adpcm info can be accessed." << endl;
            if (count > 255 || count < 0) cerr << "This file seems to have an absurd number of channels" << endl;
            int* ptr = (int*)prevTable;
            ptr += 1 + count * 2;
            return ptr;
        }

        CWAVChannelInfo* ChannelInfoEntries() {
            return (CWAVChannelInfo*)ChannelInfoRefTableEnd();
        }

        CWAVChannelInfo* GetChannelInfo(int index) {
            return (CWAVChannelInfo*)((uint8_t*)(ChannelInfoEntries() + index));
        }

        uint8_t* Address() {
            return (uint8_t*)&_tag;
        }; 

        void Set(int size, int channels, StrmDataInfo* strmDataInfo, int lbSize)
        {
            _tag = 0x4F464E49;
            _size = size;

            ChannelInfoRefTable()->_numEntries = channels;
            
            _dataInfo = CWAVDataInfo(strmDataInfo);

            //Set adpcm infos
            for (int i = 0; i < channels; i++) {
                ChannelInfoRefTable()->Entries()[i]._type = CWAVReference::RefType::ReferenceTable + i;
                ChannelInfoRefTable()->Entries()[i]._dataOffset = 4 + 8 * channels + i * (sizeof(CWAVADPCMInfo) + 20);
                ChannelInfoEntries()[i].Set(i, lbSize);
            }
        }
    };

    struct CWAVDataHeader
    {
        le_uint32_t _tag;
        le_int32_t _length;
        be_uint32_t _pad1;

        void Set(int length)
        {
            _tag = 0x41544144;
            _length = length;
            _pad1 = 0;
        }

        uint8_t* Address() { return (uint8_t*)&_tag; }
        uint8_t* Data() { return Address() + 0x20; }
    };

    struct CWAVHeader
    {
        le_uint32_t _tag;
        be_uint16_t _endian;
        le_uint16_t _headerSize;
        le_uint32_t _version;
        le_uint32_t _length;
        le_uint16_t _numBlocks;
        be_uint16_t _reserved;
        CWAVReference _infoBlockRef;
        le_uint32_t _infoBlockSize;
        CWAVReference _dataBlockRef;
        le_uint32_t _dataBlockSize;

        uint8_t* Address() {return (uint8_t*)&_tag;}

        void Set(int infoLen, int dataLen)
        {
            int len = 0x40;

            //Set header
            _tag = 0x56415743;
            _endian = 0xFFFE;
            _headerSize = len;
            _version = 0x02010000;
            _numBlocks = 2;
            _reserved = 0;

            //Set offsets/lengths
            _infoBlockRef._type = CWAVReference::RefType::InfoBlock;
            _infoBlockRef._dataOffset = len;
            _infoBlockSize = infoLen;
            _dataBlockRef._type = CWAVReference::RefType::DataBlock;
            _dataBlockRef._dataOffset = (len += infoLen);
            _dataBlockSize = dataLen;

            _length = len + dataLen;
        }

        CWAVINFOHeader* INFOData() { return (CWAVINFOHeader*)(Address() + _infoBlockRef._dataOffset); }
        CWAVDataHeader* DATAData() { return (CWAVDataHeader*)(Address() + _dataBlockRef._dataOffset); }
    };
}
