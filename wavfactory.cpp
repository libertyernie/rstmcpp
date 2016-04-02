#include <stdexcept>
#include <cstring>
#include "endian.h"
#include "wavfactory.h"

using namespace rstmcpp::pcm16;
using namespace rstmcpp::endian;

// WAVEFORMATEX
struct fmt {
	le_uint16_t format;
	le_int16_t channels;
	le_int32_t sampleRate;
	le_int32_t byteRate;
	le_int16_t blockAlign;
	le_int16_t bitsPerSample;
};

// GUID
struct le_guid {
	le_int32_t data1;
	le_int16_t data2;
	le_int16_t data3;
	uint8_t data4[8];
};

bool operator==(const struct le_guid& a, const struct le_guid& b)
{
	return a.data1 == b.data1
		&& a.data2 == b.data2
		&& a.data3 == b.data3
		&& memcmp(a.data4, b.data4, 8) == 0;
}

const struct le_guid KSDATAFORMAT_SUBTYPE_PCM = { 0x00000001L, 0x0000, 0x0010,{ 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

// WAVEFORMATEXTENSIBLE
struct fmt_extensible {
	struct fmt fmt;
	union {
		le_int16_t validBitsPerSample;
		le_int16_t samplesPerBlock;
		le_int16_t reserved;
	} samples;
	le_int32_t channelMask;
	le_guid subFormat;
};

struct smpl {
	le_uint32_t manufacturer;
	le_uint32_t product;
	le_uint32_t samplePeriod;
	le_uint32_t midiUnityNote;
	le_uint32_t midiPitchFraction;
	le_uint32_t smpteFormat;
	int8_t smpteOffsetHours;
	uint8_t smpteOffsetMinutes;
	uint8_t smpteOffsetSeconds;
	uint8_t smpteOffsetFrames;
	le_uint32_t sampleLoopCount;
	le_uint32_t samplerDataCount;
};

struct smpl_loop {
	le_uint32_t loopID;
	le_int32_t type;
	le_int32_t start;
	le_int32_t end;
	le_uint32_t fraction;
	le_int32_t playCount;
};

PCM16* wavfactory::from_file(FILE* file) {
	char buffer[12];
	int r = fread(buffer, 1, 12, file);
    if (r == 0) {
		throw std::runtime_error("No data in stream");
    } else if (r < 12) {
		throw std::runtime_error("Unexpected end of stream in first 12 bytes");
    }

    if (strncmp(buffer, "RIFF", 4) != 0) {
		throw std::runtime_error("RIFF header not found");
    }
	if (strncmp(buffer + 8, "WAVE", 4) != 0) {
		throw std::runtime_error("WAVE header not found");
    }

    int channels = 0;
    int sampleRate = 0;

	le_int16_t* sample_data = NULL;
	int sample_data_length_bytes = 0;
    bool convert_from_8_bit = false;

    int loopStart = -1;
    int loopEnd;

    // Keep reading chunk headers into a buffer of 8 bytes
    while ((r = fread(buffer, 1, 8, file)) > 0) {
        if (r < 8) {
			throw std::runtime_error("Unexpected end of stream in chunk header");
        } else {
            // Four ASCII characters
			char id[5];
			id[4] = '\0';
			strncpy(id, buffer, 4);

			le_int32_t* px = (le_int32_t*)(buffer + 4);
			int32_t chunklength = *px;

            char* buffer2;
            if (id == "data" && chunklength == -1) {
				throw std::runtime_error("No length specified in data chunk");
            } else {
                // Look at the length of the chunk and read that many bytes into a byte array.
				buffer2 = (char*)malloc(chunklength);
				char* end = buffer2 + chunklength;
				char* ptr = buffer2;
				while (ptr < end) {
					int r = fread(ptr, 1, end - ptr, file);
					ptr += r;
					if (r == 0) {
						char str[128];
						str[127] = '\0';
						snprintf(str, 127, "Unexpected end of data in \"%s\" chunk: expected %d bytes, got %d bytes", id, end - buffer2, end - ptr);
						throw std::runtime_error(str);
					}
                }
            }

            if (!strcmp(id, "fmt ")) {
                // Format chunk
				struct fmt* fmt = (struct fmt*)buffer2;
                if (fmt->format != 1) {
                    if (fmt->format == 65534) {
                        // WAVEFORMATEXTENSIBLE
                        fmt_extensible* ext = (fmt_extensible*)fmt;
                        if (ext->subFormat == KSDATAFORMAT_SUBTYPE_PCM) {
                            // KSDATAFORMAT_SUBTYPE_PCM
                        } else {
                            throw std::runtime_error("Only uncompressed PCM suppported - found WAVEFORMATEXTENSIBLE with unsupported subformat");
                        }
                    } else {
                        throw std::runtime_error("Only uncompressed PCM suppported - found unknown format");
                    }
                } else if (fmt->bitsPerSample != 16) {
					le_int16_t aa = fmt->bitsPerSample;
					int16_t ab = aa;
                    if (fmt->bitsPerSample == 8) {
                        convert_from_8_bit = true;
                    } else {
                        throw std::runtime_error("Only 8-bit and 16-bit wave files supported");
                    }
                }

                channels = fmt->channels;
                sampleRate = fmt->sampleRate;
            } else if (!strcmp(id, "data")) {
                // Data chunk - contains samples
				if (sample_data != NULL) {
					throw std::runtime_error("Multiple data chunks found");
				}
				sample_data = (le_int16_t*)buffer2;
				sample_data_length_bytes = chunklength;
            } else if (!strcmp(id, "smpl")) {
                // sampler chunk
                struct smpl* smpl = (struct smpl*)buffer2;
                if (smpl->sampleLoopCount > 1) {
                    throw std::runtime_error("Cannot read looping .wav file with more than one loop");
                } else if (smpl->sampleLoopCount == 1) {
                    // There is one loop - we only care about start and end points
                    smpl_loop* loop = (smpl_loop*)(smpl + 1);
                    if (loop->type != 0) {
                        throw std::runtime_error("Cannot read looping .wav file with loop of type other than 0");
                    }
                    loopStart = loop->start;
                    loopEnd = loop->end;
                }
            } else {
                //Console.Error.WriteLine("Ignoring unknown chunk " + id);
            }

			if ((void*)buffer2 != (void*)sample_data) {
				free(buffer2);
			}
        }
    }

    if (sampleRate == 0) {
        throw std::runtime_error("Format chunk not found");
    }
    if (sample_data == NULL) {
        throw std::runtime_error("Data chunk not found");
    }

    if (convert_from_8_bit) {
        le_int16_t* new_sample_data = (le_int16_t*)malloc(sample_data_length_bytes * 2);
        uint8_t* ptr = (uint8_t*)sample_data;
        for (int i = 0; i < sample_data_length_bytes * 2; i++) {
            new_sample_data[i] = (le_int16_t)((ptr[i] - 0x80) << 8);
        }

		free(sample_data);
        sample_data = new_sample_data;
		sample_data_length_bytes *= 2;
    }

	int16_t* sample_data_native = sample_data_native = (int16_t*)malloc(sample_data_length_bytes);
	for (int i = 0; i < sample_data_length_bytes / 2; i++) {
		sample_data_native[i] = sample_data[i];
	}
	free(sample_data);

    PCM16* wav = new PCM16(channels, sampleRate, sample_data_native, sample_data_length_bytes / 2, loopStart, loopEnd);

	free(sample_data_native);
    return wav;
}

int wavfactory::get_size(const PCM16* lwav) {
	int length = 12 + 8 + sizeof(fmt) + 8 + ((lwav->samples_end - lwav->samples) * 2);
	if (lwav->looping) {
		length += 8 + sizeof(smpl) + sizeof(smpl_loop);
	}
	return length;
}

void wavfactory::export_to_ptr(const PCM16* lwav, void* dest, int size) {
	char* ptr = (char*)dest;
	*(ptr++) = 'R';
	*(ptr++) = 'I';
	*(ptr++) = 'F';
	*(ptr++) = 'F';
	*(int32_t*)ptr = size - 8;
	ptr += 4;
	*(ptr++) = 'W';
	*(ptr++) = 'A';
	*(ptr++) = 'V';
	*(ptr++) = 'E';

	*(ptr++) = 'f';
	*(ptr++) = 'm';
	*(ptr++) = 't';
	*(ptr++) = ' ';
	*(int32_t*)ptr = sizeof(fmt);
	ptr += 4;

	struct fmt* fmt = (struct fmt*)ptr;
	fmt->format = 1;
	fmt->channels = lwav->channels;
	fmt->sampleRate = lwav->sampleRate;
	fmt->byteRate = lwav->sampleRate * lwav->channels * 2;
	fmt->blockAlign = (int16_t)(lwav->channels * 2);
	fmt->bitsPerSample = 16;
	ptr += sizeof(struct fmt);

	*(ptr++) = 'd';
	*(ptr++) = 'a';
	*(ptr++) = 't';
	*(ptr++) = 'a';
	int32_t samplesLength = (lwav->samples_end - lwav->samples) * 2;
	*(int32_t*)ptr = samplesLength;
	ptr += 4;

	memcpy(ptr, lwav->samples, samplesLength);
	ptr += samplesLength;

	if (lwav->looping) {
		*(ptr++) = 's';
		*(ptr++) = 'm';
		*(ptr++) = 'p';
		*(ptr++) = 'l';
		*(int*)ptr = sizeof(struct smpl) + sizeof(struct smpl_loop);
		ptr += 4;

		struct smpl* smpl = (struct smpl*)ptr;
		smpl->sampleLoopCount = 1;
		ptr += sizeof(struct smpl);

		struct smpl_loop* loop = (struct smpl_loop*)ptr;
		loop->loopID = 0;
		loop->type = 0;
		loop->start = (lwav->loop_start - lwav->samples) / lwav->channels;
		loop->end = (lwav->loop_end - lwav->samples) / lwav->channels;
		loop->fraction = 0;
		loop->playCount = 0;
		ptr += sizeof(struct smpl_loop);
	}
}
