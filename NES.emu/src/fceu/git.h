#ifndef __FCEU_GIT
#define __FCEU_GIT

#include <string>

enum EGIT
{
	GIT_CART	= 0,  //Cart
	GIT_VSUNI	= 1,  //VS Unisystem
	GIT_FDS		= 2,  // Famicom Disk System
	GIT_NSF		= 3,  //NES Sound Format
};

enum EGIV
{
	GIV_NTSC	= 0,  //NTSC emulation.
	GIV_PAL		= 1,  //PAL emulation.
	GIV_USER	= 2,  //What was set by FCEUI_SetVidSys().
};

enum EGIPPU
{
	GIPPU_USER			= 0,
	GIPPU_RP2C04_0001	= 1,
	GIPPU_RP2C04_0002	= 2,
	GIPPU_RP2C04_0003	= 3,
	GIPPU_RP2C04_0004	= 4,
	GIPPU_RC2C03B		= 5,
	GIPPU_RC2C05_01		= 6,
	GIPPU_RC2C05_02		= 7,
	GIPPU_RC2C05_03		= 8,
	GIPPU_RC2C05_04		= 9,
};

enum EGIVS
{
	EGIVS_NORMAL	= 0,
	EGIVS_RBI		= 1, // RBI Baseball protection
	EGIVS_TKO		= 2, // TKO Boxing protection
	EGIVS_XEVIOUS	= 3, // Super Xevious protection
};

enum ESIS
{
	SIS_NONE		= 0,
	SIS_DATACH		= 1,
	SIS_NWC			= 2,
	SIS_VSUNISYSTEM	= 3, // Is it used?
	SIS_NSF			= 4,
};

//input device types for the standard joystick port
enum ESI
{
	SI_UNSET		= -1,
	SI_NONE			= 0,
	SI_GAMEPAD		= 1,
	SI_ZAPPER		= 2,
	SI_POWERPADA	= 3,
	SI_POWERPADB	= 4,
	SI_ARKANOID		= 5,
	SI_MOUSE		= 6,
	SI_SNES			= 7,
	SI_SNES_MOUSE	= 8,
	SI_VIRTUALBOY	= 9,
	SI_LCDCOMP_ZAPPER  = 10,

	SI_COUNT = SI_LCDCOMP_ZAPPER
};



inline const char* ESI_Name(ESI esi)
{
	static const char * const names[] =
	{
		"<none>",
		"Gamepad",
		"Zapper",
		"Power Pad A",
		"Power Pad B",
		"Arkanoid Paddle",
		"Subor Mouse",
		"SNES Pad",
		"SNES Mouse",
		"Virtual Boy",
		"LCD Zapper (Advance)"
	};

	if(esi >= SI_NONE && esi <= SI_COUNT)
		return names[esi];
	else return "<invalid ESI>";
}


//input device types for the expansion port
enum ESIFC
{
	SIFC_UNSET		= -1,
	SIFC_NONE		= 0,
	SIFC_ARKANOID	= 1,
	SIFC_SHADOW		= 2,
	SIFC_4PLAYER	= 3,
	SIFC_FKB		= 4,
	SIFC_SUBORKB	= 5,
	SIFC_PEC586KB	= 6,
	SIFC_HYPERSHOT	= 7,
	SIFC_MAHJONG	= 8,
	SIFC_QUIZKING	= 9,
	SIFC_FTRAINERA	= 10,
	SIFC_FTRAINERB	= 11,
	SIFC_OEKAKIDS	= 12,
	SIFC_BWORLD		= 13,
	SIFC_TOPRIDER	= 14,
	SIFC_FAMINETSYS = 15,
	SIFC_HORI4PLAYER = 16,

	SIFC_COUNT = SIFC_HORI4PLAYER
};


inline const char* ESIFC_Name(ESIFC esifc)
{
	static const char * const names[] =
	{
		"<none>",
		"Arkanoid Paddle",
		"Hyper Shot gun",
		"4-Player Adapter",
		"Family Keyboard",
		"Subor Keyboard",
		"PEC586 Keyboard",
		"HyperShot Pads",
		"Mahjong",
		"Quiz King Buzzers",
		"Family Trainer A",
		"Family Trainer B",
		"Oeka Kids Tablet",
		"Barcode World",
		"Top Rider",
		"Famicom Network Controller",
		"Hori 4-Player Adapter"
	};

	if(esifc >= SIFC_NONE && esifc <= SIFC_COUNT)
		return names[esifc];
	else return "<invalid ESIFC>";
}


#include "utils/md5.h"

struct FCEUGI
{
	std::string name;	//Game name, UTF8 encoding
	int mappernum = 0;

	EGIT type = GIT_CART;
	EGIV vidsys = GIV_USER;                //Current emulated video system;
	ESI input[2] = { SI_UNSET, SI_UNSET }; //Desired input for emulated input ports 1 and 2; -1 for unknown desired input.
	ESIFC inputfc = SIFC_UNSET;            //Desired Famicom expansion port device. -1 for unknown desired input.
	ESIS cspecial = SIS_NONE;              //Special cart expansion: DIP switches, barcode reader, etc.
	EGIPPU vs_ppu = GIPPU_USER;            //PPU type for Vs. System
	EGIVS vs_type = EGIVS_NORMAL;          //Vs. System type
	uint8 vs_cswitch = SIS_NONE;           // Switch first and second controllers for Vs. System

	MD5DATA MD5;

	//mbg 6/8/08 - ???
	int soundrate = 0;  //For Ogg Vorbis expansion sound wacky support.  0 for default.
	int soundchan = 0;  //Number of sound channels.

	std::string filename;
	std::string archiveFilename;
	int archiveCount = 0;  // the number of files that were in the archive
	int archiveIndex = -1; // the index of the file within the archive
	bool loadedFromTmpFile = false; // Was loaded from temporary file, file most likely no longer exists
};

#endif
