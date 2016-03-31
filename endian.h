#pragma once

#ifdef _MSC_VER
#include <WinSock2.h>
#else
#include <arpa/inet.h> // for ntohs() etc.
#endif
#include <cstdint>

namespace rstmcpp {
	namespace endian {
		int32_t swap32(int32_t num);
		int16_t swap16(int16_t num);

		class be_int16_t {
		public:
			be_int16_t() : be_val_(0) {
			}
			// Transparently cast from uint16_t
			be_int16_t(const int16_t &val) : be_val_(htons(val)) {
			}
			// Transparently cast to uint16_t
			operator int16_t() const {
				return ntohs(be_val_);
			}
		private:
			int16_t be_val_;
		};

		class be_uint16_t {
		public:
			be_uint16_t() : be_val_(0) {
			}
			// Transparently cast from uint16_t
			be_uint16_t(const uint16_t &val) : be_val_(htons(val)) {
			}
			// Transparently cast to uint16_t
			operator uint16_t() const {
				return ntohs(be_val_);
			}
		private:
			uint16_t be_val_;
		};

		class be_int32_t {
		public:
			be_int32_t() : be_val_(0) {
			}
			// Transparently cast from uint16_t
			be_int32_t(const int32_t &val) : be_val_(htonl(val)) {
			}
			// Transparently cast to uint16_t
			operator int32_t() const {
				return ntohl(be_val_);
			}
		private:
			int32_t be_val_;
		};

		class be_uint32_t {
		public:
			be_uint32_t() : be_val_(0) {
			}
			// Transparently cast from uint16_t
			be_uint32_t(const uint32_t &val) : be_val_(htonl(val)) {
			}
			// Transparently cast to uint16_t
			operator uint32_t() const {
				return ntohl(be_val_);
			}
		private:
			uint32_t be_val_;
		};

		class le_int16_t {
		public:
			le_int16_t() : le_val_(0) {
			}
			// Transparently cast from uint16_t
			le_int16_t(const int16_t &val) : le_val_(swap16(htons(val))) {
			}
			// Transparently cast to uint16_t
			operator int16_t() const {
				return swap16(ntohs(le_val_));
			}
		private:
			int16_t le_val_;
		};

		class le_uint16_t {
		public:
			le_uint16_t() : le_val_(0) {
			}
			// Transparently cast from uint16_t
			le_uint16_t(const uint16_t &val) : le_val_(swap16(htons(val))) {
			}
			// Transparently cast to uint16_t
			operator uint16_t() const {
				return swap16(ntohs(le_val_));
			}
		private:
			uint16_t le_val_;
		};

		class le_int32_t {
		public:
			le_int32_t() : le_val_(0) {
			}
			// Transparently cast from uint16_t
			le_int32_t(const int32_t &val) : le_val_(swap32(htonl(val))) {
			}
			// Transparently cast to uint16_t
			operator int32_t() const {
				return swap32(ntohl(le_val_));
			}
		private:
			int32_t le_val_;
		};

		class le_uint32_t {
		public:
			le_uint32_t() : le_val_(0) {
			}
			// Transparently cast from uint16_t
			le_uint32_t(const uint32_t &val) : le_val_(swap32(htonl(val))) {
			}
			// Transparently cast to uint16_t
			operator uint32_t() const {
				return swap32(ntohl(le_val_));
			}
		private:
			uint32_t le_val_;
		};
	}
}
