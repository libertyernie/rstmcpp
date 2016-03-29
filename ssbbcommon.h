#include "endian.h"

using namespace RSTMCPP::endian;

namespace RSTMCPP
{
	/*
	unsafe struct NW4RCommonHeader
	*/
	struct NW4RCommonHeader
	{
		le_uint32_t _tag;
		be_uint16_t _endian;
		be_uint16_t _version;
		be_int32_t _length;
		be_uint16_t _firstOffset;
		be_uint16_t _numEntries;
	};

	/*
	public unsafe struct ruint
	*/
	struct ruint
	{
		enum RefType : uint8_t
		{
			AddressRefType = 0,
			OffsetRefType = 1
		};

		uint8_t _refType;

		uint8_t _dataType;
		//Specifies which struct to get, ex
		//DataRef<T, T1, T2, T3>
		//if dataType == 2, return T2 struct at address

		be_uint16_t _reserved;
		be_int32_t _dataOffset;

		ruint(RefType refType, uint8_t dataType, int data)
		{
			_refType = refType;
			_dataType = dataType;
			_reserved = 0;
			_dataOffset = data;
		}

		void* Offset(void* baseAddr) { return (byte*)baseAddr + _dataOffset; }

		ruint(int r) : _refType(1), _dataType(0), _reserved(0), _dataOffset(r) {}
		operator int() const { return _dataOffset; }
	};

	/*
	public unsafe struct RuintList
	*/
	struct RuintList
	{
		//This address is the base of all ruint entry offsets

		le_uint32_t _numEntries; // Little-endian in RSTM

		uint8_t* Address() { return (uint8_t*)&_numEntries; }
		ruint* Entries() { return (ruint*)(Address() + 4); }

		ruint* operator[] (const int index) {
			return Entries() + index;
		}

		void* Get(void* offset, int index) { return (byte*)offset + (int)Entries()[index]; }
	};

	/*
	unsafe struct RuintCollection
	*/
	struct RuintCollection
	{
	private:
		ruint _first;

	public:
		uint8_t* Address() { return (uint8_t*)&_first; }
		ruint* Entries() { return &_first; }

		void* getPtr(int index) {
			return Address() + Entries()[index];
		}

		void setPtr(int index, void* ptr) {
			int i = (int32_t)((uint8_t*)ptr - Address());
			Entries()[index] = i;
		}

		void* Get(int index) { return Address() + Entries()[index]; }

		void Set(int index, ruint::RefType refType, uint8_t dataType, void* address)
		{
			Entries()[index] = ruint(refType, dataType, (uint8_t*)address - Address());
		}
	};
}
