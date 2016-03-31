all:
	c++ -std=c++11 -o rstmcpp gc-dspadpcm-encode/grok.c endian.cpp main.cpp pcm16.cpp wavfactory.cpp progresstracker.cpp encoder.cpp

clean:
	rm rstmcpp
