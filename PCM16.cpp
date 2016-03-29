#include <stdexcept>
#include <cstdlib>
#include "PCM16.h"

using std::memcpy;

/*
public PCM16Audio(int channels, int sampleRate, short[] sample_data, int? loop_start = null, int? loop_end = null) {
*/
void RSTMCPP::WAV::PCM16Audio::initWav(int channels, int sampleRate, int16_t* sample_data, int sample_count, int loop_start, int loop_end) {
	if (channels > 65535) throw std::invalid_argument("Streams of more than 65535 channels not supported");
	if (channels <= 0) throw new std::invalid_argument("Number of channels must be a positive integer");
	if (sampleRate <= 0) throw new std::invalid_argument("Sample rate must be a positive integer");

	if (loop_start >= 0 && loop_end > sample_count / channels) {
		throw new std::invalid_argument("The end of the loop is past the end of the file. Double-check the program that generated this data.");
	}

	this->channels = channels;
	this->sampleRate = sampleRate;

	this->samples = (int16_t*)malloc(sizeof(int16_t) * sample_count);
	this->samples_pos = this->samples;
	this->samples_end = this->samples + sample_count;
	memcpy(this->samples, sample_data, sample_count * sizeof(int16_t));
	int16_t xx = this->samples_end[-1];

	if (loop_start < 0) {
		this->looping = false;
		this->loop_start = this->samples;
		this->loop_end = this->samples_end;
	} else {
		this->looping = true;
		this->loop_start = this->samples + this->channels * loop_start;
		this->loop_end = this->samples + this->channels * loop_end;
	}
}

RSTMCPP::WAV::PCM16Audio::PCM16Audio(int channels, int sampleRate, int16_t* sample_data, int sample_count) {
	initWav(channels, sampleRate, sample_data, sample_count, -1, -1);
};

RSTMCPP::WAV::PCM16Audio::PCM16Audio(int channels, int sampleRate, int16_t* sample_data, int sample_count, int loop_start, int loop_end) {
	initWav(channels, sampleRate, sample_data, sample_count, loop_start, loop_end);
};

int RSTMCPP::WAV::PCM16Audio::readSamples(void* destAddr, int numSamplesEachChannel) {
	int numSamplesTotal = numSamplesEachChannel * this->channels;

	if (this->samples_pos + numSamplesTotal > this->samples_end) {
		numSamplesTotal = this->samples_end - this->samples_pos;
	}

	memcpy(destAddr, this->samples_pos, sizeof(uint16_t) * numSamplesTotal);
	this->samples_pos += numSamplesTotal;

	return numSamplesTotal / this->channels;
}

RSTMCPP::WAV::PCM16Audio::~PCM16Audio() {
	free(samples);
}
