all:
	c++ -std=c++11 -o rstmcpp AudioConverter.cpp endian.cpp main.cpp PCM16.cpp PCM16Factory.cpp ProgressTracker.cpp RSTMConverter.cpp

clean:
	rm rstmcpp
