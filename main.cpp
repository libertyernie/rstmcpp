#include <iostream>
#include <cstring>
#include "pcm16.h"
#include "wavfactory.h"
#include "encoder.h"

using std::cerr;
using std::endl;
using namespace rstmcpp;
using namespace rstmcpp::pcm16;

int usage() {
	cerr
	<< "rstmcpp - encode and convert between RSTM, CSTM, and FSTM" << endl
	<< "Based on code from BrawlLib" << endl
	<< "https://github.com/libertyernie/RSTMCPP" << endl
	<< endl
	<< "This program is provided as - is without any warranty, implied or otherwise." << endl
	<< "By using this program, the end user agrees to take full responsibility" << endl
	<< "regarding its proper and lawful use.The authors / hosts / distributors cannot be " << endl
	<< "held responsible for any damage resulting in the use of this program, nor can " << endl
	<< "they be held accountable for the manner in which it is used." << endl
	<< endl
	<< "Usage:" << endl
	<< "stm - encode[options] <inputfile> <outputfile>" << endl
	<< endl
	<< "inputfile can be.brstm, .bcstm, .bfstm, or .wav." << endl
	<< "outputfile can be.brstm, .bcstm, or .bfstm." << endl
	<< endl
	<< "Options(WAV input only) : " << endl
	<< "- l             Loop from start of file until end of file" << endl
	<< "- l<start>      Loop from sample <start> until end of file" << endl
	<< "- l<start - end>  Loop from sample <start> until sample <end>" << endl
	<< "- noloop        Do not loop(ignore smpl chunk in WAV file if one exists)" << endl;
	return 1;
}

int main(int argc, char** argv) {
	argc--;
	argv++;

	if (argc == 0) {
		return usage();
	}

	bool forceLoop = false;
	bool forceNoLoop = false;
	int loopStart = 0, loopEnd = 0;
	const char* inputFile = NULL;
	const char* outputFile = NULL;

	while (argc > 0) {
		if (!strcmp(*argv, "/?") || !strcmp(*argv, "-h") || !strcmp(*argv, "--help")) {
			return usage();
		} else if (*argv[0] == '-' && *argv[1] == 'l') {
			forceLoop = true;
			forceNoLoop = false;

			loopStart = 0;
			char* ptr = *argv + 2;
			while (*ptr >= '0' && *ptr <= '9') {
				// Parse digit
				loopStart = loopStart * 10 + (*ptr - '0');
			}
			if (*ptr == '-') {
				// Get loop end
				loopEnd = 0;
				ptr++;
				while (*ptr >= '0' && *ptr <= '9') {
					// Parse digit
					loopEnd = loopEnd * 10 + (*ptr - '0');
				}
			} else {
				loopEnd = -1;
			}
		} else if (!strcmp(*argv, "-noloop")) {
			forceNoLoop = true;
			forceLoop = false;
		} else if (inputFile == NULL) {
			inputFile = *argv;
		} else if (outputFile == NULL) {
			outputFile = *argv;
		} else {
			cerr << "Too many arguments: " << *argv << endl;
			return 1;
		}
		argc--;
		argv++;
	}

	if (inputFile == NULL) {
		cerr << "No input file specified" << endl;
		return 1;
	}
	if (outputFile == NULL) {
		cerr << "No output file specified" << endl;
		return 1;
	}

	FILE* inFile = fopen(inputFile, "rb");
	if (inFile == NULL) {
		cerr << "Could not open file: " << inputFile << endl;
		return 1;
	}

	FILE* outFile = fopen(outputFile, "wb");
	if (outFile == NULL) {
		cerr << "Could not open file for writing: " << outputFile << endl;
		return 1;
	}

	char tag[5];
	tag[4] = '\0';
	fread(tag, 1, 4, inFile);
	fseek(inFile, 0, SEEK_SET);

	const char* ext = outputFile;
	ext += strlen(outputFile);
	while (ext > outputFile && ext[0] != '.') {
		ext--;
	}
	if (!strcmp(".brstm", ext) && !strcmp(".bcstm", ext) && !strcmp(".bfstm", ext)) {
		cerr << "Unsupported output format: " << ext << endl;
		return 1;
	}

	if (!strcmp("RIFF", tag)) {
		try {
			PCM16* wav = wavfactory::from_file(inFile);
			if (forceNoLoop) wav->looping = false;
			if (forceLoop) {
				wav->looping = true;
				wav->loop_start = (wav->samples + loopStart * wav->channels);
				if (loopEnd == 0) {
					wav->loop_end = wav->samples_end;
				} else {
					wav->loop_end = (wav->samples + loopEnd * wav->channels);
				}
			}

			ProgressTracker progress;
			int size;
			char* rstm = (char*)encoder::encode_rstm(wav, &progress, &size);
			delete wav;
			while (size > 0) {
				int r = fwrite(rstm, 1, size, outFile);
				rstm += r;
				size -= r;
			}
		}
		catch (std::exception& e) {
			cerr << e.what() << endl;
		}
	} else if (!strcmp("RSTM", tag)) {

	} else if (!strcmp("CSTM", tag)) {

	} else if (!strcmp("FSTM", tag)) {

	} else {

	}


}
