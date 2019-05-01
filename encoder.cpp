#include "encoder.h"
#include "pcm16.h"
#include "cstm.h"
#include "rstm.h"
#include "grok.h"
#include <cstdlib>
#include <iostream>
#include <vector>

using std::malloc;
using std::vector;
using std::cerr;

using namespace rstmcpp;
using namespace rstmcpp::pcm16;

void EncodeBlock(int16_t* source, int samples, uint8_t* dest, int16_t* coefs) {
	for (int i = 0; i < samples; i += 14, source += 14, dest += 8) {
		int s = samples - i;
		if (s > 14) s = 14;
		DSPEncodeFrame(source, s, dest, coefs);
	}
}

char* encoder::encode(PCM16* stream, ProgressTracker* progress, int* sizeOut, int type) {
    if (type == FileType::RSTM) {
        return (char*)encode_rstm(stream, progress, sizeOut);
    } else {
        return (char*)encode_cstm(stream, progress, sizeOut);
    }
}

CSTMHeader* encoder::encode_cstm(PCM16* stream, ProgressTracker* progress, int* sizeOut) {
    RSTMHeader* rstm = encoder::encode_rstm(stream, progress, sizeOut);

    StrmDataInfo* strmDataInfo = rstm->HEADData()->Part1();
    int channels = strmDataInfo->_format._channels;

    if (strmDataInfo->_format._encoding != 2)
        cerr << "CSTM export only supports ADPCM encoding." << endl;

    // Get section sizes from the BRSTM - BCSTM is such a similar format that we can assume the sizes will match.
    int rstmSize = 0x40;
    int infoSize = rstm->_headLength;
    int seekSize = rstm->_adpcLength;
    int dataSize = rstm->_dataLength;

    //Create byte array
    void* address = malloc(rstmSize + infoSize + seekSize + dataSize);
	memset(address, 0, rstmSize + infoSize + seekSize + dataSize);

        //Get section pointers
        CSTMHeader* cstm = (CSTMHeader*)address;
        CSTMINFOHeader* info = (CSTMINFOHeader*)((uint8_t*)cstm + rstmSize);
        CSTMSEEKHeader* seek = (CSTMSEEKHeader*)((uint8_t*)info + infoSize);
        CSTMDATAHeader* data = (CSTMDATAHeader*)((uint8_t*)seek + seekSize);

        //Initialize sections
        cstm->Set(infoSize, seekSize, dataSize);
        info->Set(infoSize, channels);
        seek->Set(seekSize);
        data->Set(dataSize);

        //Set HEAD data
        info->_dataInfo = CSTMDataInfo(strmDataInfo);

        //Create one ADPCMInfo for each channel
        //IntPtr* adpcData = stackalloc IntPtr[channels];
        for (int i = 0; i < channels; i++) {
            *info->GetChannelInfo(i) = CSTMADPCMInfo(rstm->HEADData()->GetChannelInfo(i));
        }

        be_int16_t* seekFrom = (be_int16_t*)rstm->ADPCData()->Data();
        short* seekTo = (short*)seek->Data();
        for (int i = 0; i < seek->_length / 2 - 8; i++)
        {
            *(seekTo++) = *(seekFrom++);
        }

        uint8_t* dataFrom = (uint8_t*)rstm->DATAData()->Data();
        uint8_t* dataTo = data->Data();
        memmove(dataTo, dataFrom, (uint32_t)data->_length - sizeof(uint32_t) * 8);
    return cstm;
}

RSTMHeader* encoder::encode_rstm(PCM16* stream, ProgressTracker* progress, int* sizeOut) {
	int tmp;
	bool looped = stream->looping;
	int channels = stream->channels;
	int samples;
	int blocks;
	int sampleRate = stream->sampleRate;
	int lbSamples, lbSize, lbTotal;
	int loopPadding, loopStart, totalSamples;
	int16_t* tPtr;

	if (looped)
	{
		loopStart = (stream->loop_start - stream->samples) / stream->channels;
		samples = (stream->loop_end - stream->samples) / stream->channels; //Set sample size to end sample. That way the audio gets cut off when encoding.

		//If loop point doesn't land on a block, pad the stream so that it does.
		if ((tmp = loopStart % 0x3800) != 0)
		{
			loopPadding = 0x3800 - tmp;
			loopStart += loopPadding;
		} else
			loopPadding = 0;

		totalSamples = loopPadding + samples;
	} else
	{
		loopPadding = loopStart = 0;
		totalSamples = samples = (stream->samples_end - stream->samples) / stream->channels;
	}

	if (progress != nullptr)
		progress->begin(0, totalSamples * channels * 3, 0);

	blocks = (totalSamples + 0x37FF) / 0x3800;

	//Initialize stream info
	if ((tmp = totalSamples % 0x3800) != 0)
	{
		lbSamples = tmp;
		lbSize = (lbSamples + 13) / 14 * 8;
		lbTotal = lbSize;
		while (lbTotal % 0x20 != 0) lbTotal++; // TODO optimize?
	} else
	{
		lbSamples = 0x3800;
		lbTotal = lbSize = 0x2000;
	}

	//Get section sizes
	int rstmSize = 0x40;
	int headSize = 0x68 + (channels * 0x40);
	while (headSize % 0x20 != 0) headSize++;
	int adpcSize = (blocks - 1) * 4 * channels + 0x10;
	while (adpcSize % 0x20 != 0) adpcSize++;
	int dataSize = ((blocks - 1) * 0x2000 + lbTotal) * channels + 0x20;

	if (sizeOut != nullptr) {
		*sizeOut = rstmSize + headSize + adpcSize + dataSize;
	}
	void* address = malloc(rstmSize + headSize + adpcSize + dataSize);
	memset(address, 0, rstmSize + headSize + adpcSize + dataSize);

	//Get section pointers
	RSTMHeader* rstm = (RSTMHeader*)address;
	HEADHeader* head = (HEADHeader*)((uint8_t*)rstm + rstmSize);
	ADPCHeader* adpc = (ADPCHeader*)((uint8_t*)head + headSize);
	RSTMDATAHeader* data = (RSTMDATAHeader*)((uint8_t*)adpc + adpcSize);

	//Initialize sections
	rstm->Set(headSize, adpcSize, dataSize);
	head->Set(headSize, channels);
	adpc->Set(adpcSize);
	data->Set(dataSize);

	//Set HEAD data
	StrmDataInfo* part1 = head->Part1();
	part1->_format = AudioFormatInfo(2, (uint8_t)(looped ? 1 : 0), (uint8_t)channels, 0);
	part1->_sampleRate = (uint16_t)sampleRate;
	part1->_blockHeaderOffset = 0;
	part1->_loopStartSample = loopStart;
	part1->_numSamples = totalSamples;
	part1->_dataOffset = rstmSize + headSize + adpcSize + 0x20;
	part1->_numBlocks = blocks;
	part1->_blockSize = 0x2000;
	part1->_samplesPerBlock = 0x3800;
	part1->_lastBlockSize = lbSize;
	part1->_lastBlockSamples = lbSamples;
	part1->_lastBlockTotal = lbTotal;
	part1->_dataInterval = 0x3800;
	part1->_bitsPerSample = 4;

	//Create one ADPCMInfo for each channel
	vector<ADPCMInfo*> pAdpcm;
	for (int i = 0; i < channels; i++)
	{
		ADPCMInfo* p = head->GetChannelInfo(i);
		*p = ADPCMInfo();
		p->_pad = 0;
		pAdpcm.push_back(p);
	}

	//Create buffer for each channel
	vector<int16_t*> channelBuffers;
	int bufferSamples = totalSamples + 2; //Add two samples for initial yn values
	for (int i = 0; i < channels; i++)
	{
		channelBuffers.push_back(tPtr = (int16_t*)malloc(bufferSamples * 2)); //Two bytes per sample

		//Zero padding samples and initial yn values
		for (int x = 0; x < (loopPadding + 2); x++)
			*tPtr++ = 0;
	}

	//Fill buffers
	stream->samples_pos = stream->samples;
	int16_t* sampleBuffer = (int16_t*)malloc(channels * sizeof(int16_t*));

	for (int i = 2; i < bufferSamples; i++)
	{
		if (looped && stream->samples_pos == stream->loop_end)
			stream->samples_pos = stream->loop_start;

		stream->readSamples(sampleBuffer, 1);
		for (int x = 0; x < channels; x++)
			channelBuffers[x][i] = sampleBuffer[x];
	}

	free(sampleBuffer);

	//Calculate coefs
	for (int i = 0; i < channels; i++) {
		DSPCorrelateCoefs(channelBuffers[i] + 2, totalSamples, (int16_t*)pAdpcm[i]);
		if (progress)
			progress->update(progress->currentValue + totalSamples);
	}

	//Encode blocks
	uint8_t* dPtr = (uint8_t*)data->Data();
	be_int16_t* pyn = (be_int16_t*)adpc->Data();
	for (int sIndex = 0, bIndex = 1; sIndex < totalSamples; sIndex += 0x3800, bIndex++)
	{
		int blockSamples = totalSamples - sIndex;
		if (blockSamples > 0x3800) blockSamples = 0x3800;
		for (int x = 0; x < channels; x++)
		{
			int16_t* sPtr = channelBuffers[x] + sIndex;

			//Set block yn values
			if (bIndex != blocks)
			{
				*pyn++ = sPtr[0x3801];
				*pyn++ = sPtr[0x3800];
			}

			//Encode block (include yn in sPtr)
			EncodeBlock(sPtr, blockSamples, dPtr, (int16_t*)pAdpcm[x]);

			//Set initial ps
			if (bIndex == 1)
				pAdpcm[x]->_ps = *dPtr;

			//Advance output pointer
			if (bIndex == blocks)
			{
				//Fill remaining
				dPtr += lbSize;
				for (int i = lbSize; i < lbTotal; i++)
					*dPtr++ = 0;
			} else
				dPtr += 0x2000;
		}

		if (progress != nullptr)
		{
			if ((sIndex % 0x3800) == 0)
				progress->update(progress->currentValue + (0x7000 * channels));
		}
	}

	//Reverse coefs, if necessary
	for (int i = 0; i < channels; i++)
	{
		int16_t* from = (int16_t*)pAdpcm[i]->_coefs;
		be_int16_t* to = (be_int16_t*)from;
		for (int x = 0; x < 16; x++)
			*to++ = *from++;
	}

	//Write loop states
	if (looped)
	{
		//Can't we just use block states?
		int loopBlock = loopStart / 0x3800;
		int loopChunk = (loopStart - (loopBlock * 0x3800)) / 14;
		dPtr = (uint8_t*)data->Data() + (loopBlock * 0x2000 * channels) + (loopChunk * 8);
		tmp = (loopBlock == blocks - 1) ? lbTotal : 0x2000;

		for (int i = 0; i < channels; i++, dPtr += tmp)
		{
			//Use adjusted samples for yn values
			tPtr = channelBuffers[i] + loopStart;
			pAdpcm[i]->_lps = *dPtr;
			pAdpcm[i]->_lyn2 = *tPtr++;
			pAdpcm[i]->_lyn1 = *tPtr;
		}
	}

	//Free memory
	for (int i = 0; i < channels; i++)
		free(channelBuffers[i]);

	if (progress != nullptr)
		progress->finish();

	return rstm;
}
