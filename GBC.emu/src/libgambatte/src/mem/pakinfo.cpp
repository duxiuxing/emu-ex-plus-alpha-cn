#include "pakinfo_internal.h"

#include <cstring>
#include <imagine/util/macros.h>

namespace gambatte {

enum { flag_multipak = 1, flag_header_checksum_ok = 2, };

static bool isHeaderChecksumOk(unsigned const char header[]) {
	unsigned csum = 0;
	for (int i = 0x134; i < 0x14D; ++i)
		csum -= header[i] + 1;

	return (csum & 0xFF) == header[0x14D];
}

static bool isMbc2(unsigned char h147) { return h147 == 5 || h147 == 6; }

unsigned numRambanksFromH14x(unsigned char h147, unsigned char h149) {
	switch (h149) {
	case 0x00: return isMbc2(h147) ? 1 : 0;
	case 0x01:
	case 0x02: return 1;
	default: case 0x03: return 4;
	case 0x04: return 16;
	case 0x05: return 8;
	}
}

PakInfo::PakInfo()
: flags_(), rombanks_()
{
	std::memset(h144x_, 0 , sizeof h144x_);
}

PakInfo::PakInfo(bool multipak, unsigned rombanks, unsigned char const romheader[])
: flags_(  multipak * flag_multipak
         + isHeaderChecksumOk(romheader) * flag_header_checksum_ok),
  rombanks_(rombanks)
{
	std::memcpy(h144x_, romheader + 0x144, sizeof h144x_);
}

bool PakInfo::headerChecksumOk() const { return flags_ & flag_header_checksum_ok; }

static char const * h147ToCstr(unsigned char const h147) {
	switch (h147) {
	case 0x00:
		return UI_TEXT("NULL");
	case 0x01:
		return UI_TEXT("MBC1");
	case 0x02:
		return UI_TEXT("MBC1 [RAM]");
	case 0x03:
		return UI_TEXT("MBC1 [RAM,battery]");
	case 0x05:
		return UI_TEXT("MBC2");
	case 0x06:
		return UI_TEXT("MBC2 [battery]");
	case 0x08:
		return UI_TEXT("NULL [RAM]");
	case 0x09:
		return UI_TEXT("NULL [RAM,battery]");
	case 0x0B:
		return UI_TEXT("MMM01");
	case 0x0C:
		return UI_TEXT("MMM01 [RAM]");
	case 0x0D:
		return UI_TEXT("MMM01 [RAM,battery]");
	case 0x0F:
		return UI_TEXT("MBC3 [RTC,battery]");
	case 0x10:
		return UI_TEXT("MBC3 [RAM,RTC,battery]");
	case 0x11:
		return UI_TEXT("MBC3");
	case 0x12:
		return UI_TEXT("MBC3 [RAM]");
	case 0x13:
		return UI_TEXT("MBC3 [RAM,battery]");
	case 0x15:
		return UI_TEXT("MBC4");
	case 0x16:
		return UI_TEXT("MBC4 [RAM]");
	case 0x17:
		return UI_TEXT("MBC4 [RAM,battery]");
	case 0x19:
		return UI_TEXT("MBC5");
	case 0x1A:
		return UI_TEXT("MBC5 [RAM]");
	case 0x1B:
		return UI_TEXT("MBC5 [RAM,battery]");
	case 0x1C:
		return UI_TEXT("MBC5 [rumble]");
	case 0x1D:
		return UI_TEXT("MBC5 [RAM,rumble]");
	case 0x1E:
		return UI_TEXT("MBC5 [RAM,rumble,battery]");
	case 0xFC:
		return UI_TEXT("Pocket Camera");
	case 0xFD:
		return UI_TEXT("Bandai TAMA5");
	case 0xFE:
		return UI_TEXT("HuC3");
	case 0xFF:
		return UI_TEXT("HuC1 [RAM,battery]");
	}

	return UI_TEXT("Unknown");
}

std::string const PakInfo::mbc() const {
	std::string h147str = h147ToCstr(h144x_[3]);

	if (flags_ & flag_multipak)
		h147str += UI_TEXT(" (Custom MultiPak)");

	return h147str;
}

unsigned PakInfo::rambanks() const { return numRambanksFromH14x(h144x_[3], h144x_[5]); }
unsigned PakInfo::rombanks() const { return rombanks_; }

}
