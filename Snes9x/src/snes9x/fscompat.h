/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once

#ifdef __cplusplus
#include <string>
#include <string_view>

enum s9x_getdirtype
{
	DEFAULT_DIR = 0,
	HOME_DIR,
	ROMFILENAME_DIR,
	ROM_DIR,
	SRAM_DIR,
	SNAPSHOT_DIR,
	SCREENSHOT_DIR,
	SPC_DIR,
	CHEAT_DIR,
	PATCH_DIR,
	BIOS_DIR,
	LOG_DIR,
	SAT_DIR,
	LAST_DIR
};

struct SplitPath
{
   bool ext_is(const std::string &ext);
    std::string drive;
    std::string dir;
    std::string stem;
    std::string ext;
};

SplitPath splitpath(std::string filename);
std::string makepath(const std::string &drive, 
                     const std::string &directory, 
                     const std::string &stem, 
                     const std::string &extension);
std::string makepath(const SplitPath &);

std::string S9xBasename (std::string_view);
std::string S9xBasenameNoExt (std::string_view);
std::string S9xGetFilename (std::string_view ext, enum s9x_getdirtype dirtype);
std::string S9xGetFilename (std::string_view filename, std::string_view ext, enum s9x_getdirtype dirtype);
std::string S9xGetDirectory (enum s9x_getdirtype);
std::string S9xGetFilenameInc (std::string_view, enum s9x_getdirtype);
std::string S9xGetFullFilename (std::string_view name, enum s9x_getdirtype);
void S9xReadBSXBios(uint8 *data);
#endif
