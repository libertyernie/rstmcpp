#include "endian.h"

int32_t rstmcpp::endian::swap32(int32_t num) {
	return ((num & 0xff000000) >> 24) | ((num & 0x00ff0000) >> 8) | ((num & 0x0000ff00) << 8) | (num << 24);
}
int16_t rstmcpp::endian::swap16(int16_t num) {
	return ((num & 0xff00) >> 8) | (num << 8);
}