CXX = c++

LIBS :=
SYS := $(shell $(CXX) -dumpmachine)
ifneq (, $(findstring mingw, $(SYS)))
	LIBS := -lws2_32
endif

all:
	$(CXX) -g -std=c++11 -o rstmcpp gc-dspadpcm-encode/grok.c endian.cpp main.cpp pcm16.cpp wavfactory.cpp progresstracker.cpp encoder.cpp $(LIBS)

clean:
	rm rstmcpp
