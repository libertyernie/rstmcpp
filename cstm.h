#pragma once

#include <cstring>
#include <iostream>

#include "ssbbcommon.h"

using std::cerr;
using std::endl;

namespace rstmcpp
{
    /// <summary>
    /// A type/offset pair similar to ruint.
    /// </summary>
    struct CSTMReference {
        enum RefType : uint16_t {
            ByteTable = 0x0100,
            ReferenceTable = 0x0101,
            SampleData = 0x1F00,
            DSPADPCMInfo = 0x0300,
            InfoBlock = 0x4000,
            SeekBlock = 0x4001,
            DataBlock = 0x4002,
            StreamInfo = 0x4100,
            TrackInfo = 0x4101,
            ChannelInfo = 0x4102
        };

        RefType _type;
        be_uint16_t _padding;
        int32_t _dataOffset;
    }; 
    /// <summary>
    /// A list of CSTMReferences, beginning with a length value. Similar to RuintList.
    /// </summary>
    struct CSTMReferenceList {
        int32_t _numEntries;

        uint8_t* Address() { return (uint8_t*)&_numEntries; }
        CSTMReference* Entries { return (CSTMReference*)(Address + 4); }
    };

    struct CSTMHeader
    {
        const le_uint32_t Tag = 0x4D545343;

        le_uint32_t _tag;
        be_uint16_t _endian;
        be_uint16_t _headerSize;
        be_uint32_t _version;
        be_uint32_t _length;
        be_uint16_t _numBlocks;
        be_uint16_t _reserved;
        CSTMReference _infoBlockRef;
        be_uint32_t _infoBlockSize;
        CSTMReference _seekBlockRef;
        be_uint32_t _seekBlockSize;
        CSTMReference _dataBlockRef;
        be_uint32_t _dataBlockSize;

        Endian* getEndian() { return (Endian*)(be_uint16_t)_endian; }
        void setEndian() { _endian = (be_uint16_t)value; }

        uint8_t* Address() {return (uint8_t*)&_tag;}

        void Set(int infoLen, int seekLen, int dataLen)
        {
            int len = 0x40;

            //Set header
            _tag = Tag;
            Endian = Endian.Little;
            _headerSize = (be_uint16_t)len;
            _version = 0x02000000;
            _numBlocks = 3;
            _reserved = 0;

            //Set offsets/lengths
            _infoBlockRef._type = CSTMReference.RefType.InfoBlock;
            _infoBlockRef._dataOffset = len;
            _infoBlockSize = infoLen;
            _seekBlockRef._type = CSTMReference.RefType.SeekBlock;
            _seekBlockRef._dataOffset = (len += infoLen);
            _seekBlockSize = seekLen;
            _dataBlockRef._type = CSTMReference.RefType.DataBlock;
            _dataBlockRef._dataOffset = (len += seekLen);
            _dataBlockSize = dataLen;

            _length = len + dataLen;
        }

        CSTMINFOHeader* INFOData { return (CSTMINFOHeader*)(Address + _infoBlockRef._dataOffset); }
        CSTMSEEKHeader* SEEKData { return (CSTMSEEKHeader*)(Address + _seekBlockRef._dataOffset); } 
        CSTMDATAHeader* DATAData { return (CSTMDATAHeader*)(Address + _dataBlockRef._dataOffset); }
    };

    /// <summary>
    /// Represents a single TrackInfo segment (with the assumption that there will be only one in a file.) Some unknown data (the Byte Table) is hardcoded in the constructor.
    /// </summary>
    struct CSTMTrackInfoStub {
        uint8_t _volume;
        uint8_t _pan;
        be_uint16_t _padding;
        CSTMReference _byteTableReference;
        be_uint32_t _byteTableCount;
        be_int32_t _byteTable;

        public CSTMTrackInfoStub(int volume, int pan) {
            _volume = volume;
            _pan = pan;
            _padding = 0;
            _byteTableReference._type = CSTMReference.RefType.ByteTable;
            _byteTableReference._padding = 0;
            _byteTableReference._dataOffset = 12;
            _byteTableCount = 2;
            _byteTable = 0x00010000;
        }
    };

    struct CSTMINFOHeader
    {
        const le_uint32_t Tag = 0x4F464E49;

        le_uint32_t _tag;
        be_int32_t _size;
        CSTMReference _streamInfoRef;
        CSTMReference _trackInfoRefTableRef;
        CSTMReference _channelInfoRefTableRef;
        CSTMDataInfo _dataInfo;

        void* DataInfoEnd(CSTMDataInfo* dataInfoPtr = &_dataInfo) {
            uint8_t* endptr = (uint8_t*)(dataInfoPtr + 1);
            return endptr;
        }

        // Below properties will find different parts of the INFO header, assuming that there are zero or one TrackInfo structures.

        CSTMReferenceList* TrackInfoRefTable {
            return (CSTMReferenceList*)DataInfoEnd;
        }

        CSTMReferenceList* ChannelInfoRefTable {
            if (_trackInfoRefTableRef._dataOffset == -1) {
                fixed (CSTMReference* x = &_streamInfoRef)
                {
                    // Look to see what the _channelInfoRefTableRef says - but if it's a file we're building, it may not have been filled in yet.
                    CSTMReferenceList* fromRef = (CSTMReferenceList*)((void*)x + _channelInfoRefTableRef._dataOffset);
                    // If it's not filled in, give a 0x0C gap to allow for the TrackInfoRefTable.
                    CSTMReferenceList* guess = (CSTMReferenceList*)(DataInfoEnd + 0x0C);
                    if (fromRef > guess) {
                        cerr << "There is extra data between the DataInfo and ChannelInfoRefTable that will be discarded." << endl;
                        return fromRef;
                    }
                    return guess;
                }
            }

            CSTMReferenceList* prevTable = TrackInfoRefTable;
            int* ptr = (int*)prevTable;
            int count = prevTable->_numEntries;
            if (count == 0) cerr << "Track info's ref table must be populated before channel info's ref table can be accessed." << endl;
            if (count == 16777216) {
                count = 1;
            }
            if (count != 1) cerr << "BCSTM files with more than one track data section are not supported." << endl;
            ptr += 1 + count * 2;
            return (CSTMReferenceList*)ptr;
        }

        void* ChannelInfoRefTableEnd {
            CSTMReferenceList* prevTable = ChannelInfoRefTable;
            int count = prevTable->_numEntries;
            if (count == 0) cerr << "Channel info's ref table must be populated before track info can be accessed." << endl;
            if (count > 255 || count < 0) cerr << "This file seems to have an absurd number of channels" << endl;
            int* ptr = (int*)prevTable;
            ptr += 1 + count * 2;
            return ptr;
        }

        CSTMTrackInfoStub* TrackInfo {
            if (_trackInfoRefTableRef._dataOffset == -1) return null;

            return (CSTMTrackInfoStub*)ChannelInfoRefTableEnd;
        }

        CSTMReference* ChannelInfoEntries {
            if (_trackInfoRefTableRef._dataOffset == -1) {
                return (CSTMReference*)ChannelInfoRefTableEnd;
            } else {
                int* ptr = (int*)(TrackInfo + 1);
                return (CSTMReference*)ptr;
            }
        }

        CSTMADPCMInfo* GetChannelInfo(int index) {
            if (ChannelInfoEntries[index]._dataOffset == 0) throw new Exception("Channel info entries must be populated with references to ADPCM data before ADPCM data can be accessed.");
            return (CSTMADPCMInfo*)((byte*)(ChannelInfoEntries + index) + ChannelInfoEntries[index]._dataOffset);
        }

        uint8_t* Address() {
            return (uint8_t*)&_tag;
        }; 

        void Set(int size, int channels)
        {
            _tag = Tag;
            _size = size;

            TrackInfoRefTable->_numEntries = 1;
            ChannelInfoRefTable->_numEntries = channels;

            _streamInfoRef._type = CSTMReference.RefType.StreamInfo;
            _streamInfoRef._dataOffset = 0x18;
            _trackInfoRefTableRef._type = CSTMReference.RefType.ReferenceTable;
            _trackInfoRefTableRef._dataOffset = (byte*)TrackInfoRefTable - (Address + 8);
            _channelInfoRefTableRef._type = CSTMReference.RefType.ReferenceTable;
            _channelInfoRefTableRef._dataOffset = (byte*)ChannelInfoRefTable - (Address + 8);

            //Set single track info
            *TrackInfo = new CSTMTrackInfoStub(0x7F, 0x40);
            TrackInfoRefTable->Entries[0]._type = CSTMReference.RefType.TrackInfo;
            TrackInfoRefTable->Entries[0]._dataOffset = ((VoidPtr)TrackInfo - TrackInfoRefTable);

            //Set adpcm infos
            for (int i = 0; i < channels; i++) {
                ChannelInfoEntries[i]._dataOffset = 0x8 * channels + 0x26 * i;
                ChannelInfoEntries[i]._type = CSTMReference.RefType.DSPADPCMInfo;

                //Set initial pointer
                ChannelInfoRefTable->Entries[i]._dataOffset = ((VoidPtr)(ChannelInfoEntries + i) - (VoidPtr)ChannelInfoRefTable);
                ChannelInfoRefTable->Entries[i]._type = CSTMReference.RefType.ChannelInfo;
            }
        }
    };

    struct CSTMDataInfo
    {
        AudioFormatInfo _format;
        be_uint16_t _sampleRate;
        be_uint16_t _loopStartSample;
        be_uint16_t _numSamples;

        be_uint16_t _numBlocks;
        be_uint16_t _blockSize;
        be_uint16_t _samplesPerBlock;
        be_uint16_t _lastBlockSize;

        be_uint16_t _lastBlockSamples;
        be_uint16_t _lastBlockTotal; //Includes padding
        be_uint16_t _bitsPerSample;
        be_uint16_t _dataInterval;

        public CSTMReference _sampleDataRef;

        public CSTMDataInfo(StrmDataInfo o, int dataOffset = 0x18)
        {
            _format = o._format;
            _sampleRate = o._sampleRate;
            _loopStartSample = o._loopStartSample;
            _numSamples = o._numSamples;

            _numBlocks = o._numBlocks;
            _blockSize = o._blockSize;
            _samplesPerBlock = o._samplesPerBlock;
            _lastBlockSize = o._lastBlockSize;

            _lastBlockSamples = o._lastBlockSamples;
            _lastBlockTotal = o._lastBlockTotal;
            _bitsPerSample = o._bitsPerSample;
            _dataInterval = o._dataInterval;

            _sampleDataRef._type = CSTMReference.RefType.SampleData;
            _sampleDataRef._padding = 0;
            _sampleDataRef._dataOffset = dataOffset;
        }
    };

    struct CSTMSEEKHeader
    {
        const le_uint32_t Tag = 0x4B454553;

        le_uint32_t _tag;
        be_int32_t _length;
        uint32_t _pad1, _pad2;

        void Set(int length)
        {
            _tag = Tag;
            _length = length;
            _pad1 = _pad2 = 0;
        }

        uint8_t* Address() { return (uint8_t*)&_tag; }
        uint8_t* Data() { return Address + 0x10; }
    };

    struct CSTMDATAHeader
    {
        const int Size = 0x20;
        const le_uint32_t Tag = 0x41544144;

        le_uint32_t _tag;
        be_int32_t _length;
        be_uint32_t _dataOffset;
        be_uint32_t _pad1;

        void Set(int length)
        {
            _tag = Tag;
            _length = length;
            // _dataOffset = 0x18 matches froggestspirit's BRSTM2BCSTM converter.
            // To match the behavior of soneek's PHP converter, set this to 0.
            _dataOffset = 0x18;
            _pad1 = 0;
        }

        uint8_t* Address() { return (uint8_t*)&_tag; }
        uint8_t* Data() { return Address + 0x20; }
    };

    struct CSTMADPCMInfo
    {
        const int Size = 0x2E;

        be_uint16_t _coefs[16];

        be_uint16_t _gain;
        be_int16_t _ps; //Predictor and scale. This will be initialized to the predictor and scale value of the sample's first frame.
        be_int16_t _yn1; //History data; used to maintain decoder state during sample playback.
        be_int16_t _yn2; //History data; used to maintain decoder state during sample playback.
        be_int16_t _lps; //Predictor/scale for the loop point frame. If the sample does not loop, this value is zero.
        be_int16_t _lyn1; //History data for the loop point. If the sample does not loop, this value is zero.
        be_int16_t _lyn2; //History data for the loop point. If the sample does not loop, this value is zero.
        int16_t _pad;

        CSTMADPCMInfo(ADPCMInfo o)
        {
            be_uint16_t[] c = o.Coefs;
            for (int i = 0; i < 16; i++)
                _coefs[i] = c[i];

            _gain = o._gain;
            _ps = o._ps;
            _yn1 = o._yn1;
            _yn2 = o._yn2;
            _lps = o._lps;
            _lyn1 = o._lyn1;
            _lyn2 = o._lyn2;
            _pad = o._pad;
        }
    }
}
