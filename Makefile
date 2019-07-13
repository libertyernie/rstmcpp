CXX = x86_64-w64-mingw32-c++

LIBS :=
SYS := $(shell $(CXX) -dumpmachine)
ifneq (, $(findstring mingw, $(SYS)))
	LIBS := -lws2_32
endif

all:
	c++ -g -std=c++11 -o rstmcpp gc-dspadpcm-encode/grok.c endian.cpp main.cpp pcm16.cpp wavfactory.cpp progresstracker.cpp encoder.cpp

clean:
	rm rstmcpp
