/*  This file is part of NEO.emu.

	NEO.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	NEO.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with NEO.emu.  If not, see <http://www.gnu.org/licenses/> */

#define LOGTAG "main"
#include <emuframework/EmuSystemInlines.hh>
#include <emuframework/EmuAppInlines.hh>
#include <imagine/fs/ArchiveFS.hh>
#include <imagine/fs/FS.hh>
#include <imagine/io/FileIO.hh>
#include <imagine/util/ScopeGuard.hh>
#include <imagine/util/format.hh>

extern "C"
{
	#include <gngeo/roms.h>
	#include <gngeo/conf.h>
	#include <gngeo/emu.h>
	#include <gngeo/fileio.h>
	#include <gngeo/timer.h>
	#include <gngeo/memory.h>
	#include <gngeo/video.h>
	#include <gngeo/screen.h>
	#include <gngeo/menu.h>
	#include <gngeo/resfile.h>

	CONFIG conf{};
	GN_Rect visible_area;

	GN_Surface *buffer{};
	static CONF_ITEM rompathConfItem{};

	CONF_ITEM* cf_get_item_by_name(const char *nameStr)
	{
		using namespace EmuEx;
		//logMsg("getting conf item %s", name);
		static CONF_ITEM conf{};
		std::string_view name{nameStr};
		if(name == "dump")
		{
			static CONF_ITEM dump{};
			return &dump;
		}
		else if(name == "effect")
		{
			strcpy(conf.data.dt_str.str, "none");
		}
		else if(name == "blitter")
		{
			strcpy(conf.data.dt_str.str, "soft");
		}
		else if(name == "transpack")
		{
			strcpy(conf.data.dt_str.str, "");
		}
		else
		{
			logErr("unknown conf item %s", nameStr);
		}
		return &conf;
	}

	const char *get_gngeo_dir(void *contextPtr)
	{
		auto &sys = EmuEx::EmuApp::get(*(IG::ApplicationContext*)contextPtr).system();
		return sys.contentSaveDirectoryPtr();
	}

	PathArray get_rom_path(void *contextPtr)
	{
		auto &sys = EmuEx::EmuApp::get(*(IG::ApplicationContext*)contextPtr).system();
		PathArray path;
		strncpy(path.data, sys.contentDirectory().data(), sizeof(path));
		return path;
	}
}

CLINK void main_frame(void *emuTaskPtr, void *neoSystemPtr, void *emuVideoPtr);

namespace EmuEx
{

const char *EmuSystem::creditsViewStr = CREDITS_INFO_STRING "(c) 2012-2022\nRobert Broglia\nwww.explusalpha.com\n\nPortions (c) the\nGngeo Team\ncode.google.com/p/gngeo";
bool EmuSystem::handlesGenericIO = false; // TODO: need to re-factor GnGeo file loading code
bool EmuSystem::canRenderRGBA8888 = false;
double EmuSystem::staticFrameTime = 264. / 15625.; // ~59.18Hz
bool EmuApp::needsGlobalInstance = true;

NeoSystem::NeoSystem(ApplicationContext ctx):
	EmuSystem{ctx}
{
	visible_area.x = 0;//16;
	visible_area.y = 16;
	visible_area.w = 304;//320;
	visible_area.h = 224;
	sdlSurf.pitch = FBResX*2;
	sdlSurf.w = FBResX;
	sdlSurf.pixels = screenBuff;
	buffer = &sdlSurf;
	conf.sound = 1;
	conf.sample_rate = 44100; // must be initialized to any valid value for YM2610Init()
	strcpy(rompathConfItem.data.dt_str.str, ".");
	if(!Config::envIsAndroid)
	{
		IG::formatTo(datafilePath, "{}/gngeo_data.zip", appContext().assetPath());
	}
}

const char *EmuSystem::shortSystemName() const
{
	return "NeoGeo";
}

const char *EmuSystem::systemName() const
{
	return "Neo Geo";
}

static bool hasNeoGeoExtension(std::string_view name)
{
	return false; // archives handled by EmuFramework
}

EmuSystem::NameFilterFunc EmuSystem::defaultFsFilter = hasNeoGeoExtension;
EmuSystem::NameFilterFunc EmuSystem::defaultBenchmarkFsFilter = hasNeoGeoExtension;

void NeoSystem::reset(EmuApp &, ResetMode mode)
{
	assert(hasContent());
	neogeo_reset();
	cpu_z80_init();
	YM2610Reset();
}

FS::FileString NeoSystem::stateFilename(int slot, std::string_view name) const
{
	return IG::format<FS::FileString>("{}.0{}.sta", name, saveSlotCharUpper(slot));
}

void NeoSystem::saveState(IG::CStringView path)
{
	auto ctx = appContext();
	if(!save_stateWithName(&ctx, path))
		return EmuSystem::throwFileWriteError();
}

void NeoSystem::loadState(EmuApp &app, IG::CStringView path)
{
	auto ctx = app.appContext();
	if(!load_stateWithName(&ctx, path))
		return EmuSystem::throwFileReadError();
}

static auto nvramPath(EmuSystem &sys)
{
	return sys.contentSaveFilePath(".nv");
}

static auto memcardPath(EmuSystem &sys)
{
	return sys.contentSavePath("memcard");
}

void NeoSystem::onFlushBackupMemory(BackupMemoryDirtyFlags flags)
{
	if(!hasContent())
		return;
	if(flags & SRAM_DIRTY_BIT)
		FileUtils::writeToUri(appContext(), nvramPath(*this), {memory.sram, 0x10000});
	if(flags & MEMCARD_DIRTY_BIT)
		FileUtils::writeToUri(appContext(), memcardPath(*this), {memory.memcard, 0x800});
}

void NeoSystem::closeSystem()
{
	close_game();
}

static auto openGngeoDataIO(IG::ApplicationContext ctx, IG::CStringView filename)
{
	#ifdef __ANDROID__
	return ctx.openAsset(filename, IO::AccessHint::ALL);
	#else
	return FS::fileFromArchive(static_cast<NeoApp&>(ctx.application()).system().datafilePath, filename);
	#endif
}

void NeoSystem::loadContent(IO &, EmuSystemCreateParams, OnLoadProgressDelegate onLoadProgressFunc)
{
	if(contentDirectory().empty())
	{
		throwMissingContentDirError();
	}
	onLoadProgress = onLoadProgressFunc;
	auto resetOnLoadProgress = IG::scopeGuard([&](){ onLoadProgress = {}; });
	auto ctx = appContext();
	ROM_DEF *drv = res_load_drv(&ctx, contentName().data());
	if(!drv)
	{
		throw std::runtime_error("This game isn't recognized");
	}
	auto freeDrv = IG::scopeGuard([&](){ free(drv); });
	logMsg("rom set %s, %s", drv->name, drv->longname);
	auto gnoFilename = EmuSystem::contentSaveFilePath(".gno");
	if(optionCreateAndUseCache && ctx.fileUriExists(gnoFilename))
	{
		logMsg("loading .gno file");
		char errorStr[1024];
		if(!init_game(&ctx, gnoFilename.data(), errorStr))
		{
			throw std::runtime_error(errorStr);
		}
	}
	else
	{
		char errorStr[1024];
		if(!init_game(&ctx, drv->name, errorStr))
		{
			throw std::runtime_error(errorStr);
		}

		if(optionCreateAndUseCache && !ctx.fileUriExists(gnoFilename))
		{
			logMsg("%s doesn't exist, creating", gnoFilename.data());
			dr_save_gno(&memory.rom, gnoFilename.data());
		}
	}
	EmuSystem::setContentDisplayName(drv->longname);
	setTimerIntOption();
	neogeo_frame_counter = 0;
	neogeo_frame_counter_speed = 8;
	fc = 0;
	last_line = 0;
	// clear excess bits from universe bios region/system settings
	memory.memcard[2] = memory.memcard[2] & 0x80;
	memory.memcard[3] = memory.memcard[3] & 0x3;
}

void NeoSystem::configAudioRate(IG::FloatSeconds frameTime, int rate)
{
	conf.sample_rate = std::round(rate / staticFrameTime * frameTime.count());
	if(hasContent())
	{
		logMsg("setting YM2610 rate to %d", conf.sample_rate);
		YM2610ChangeSamplerate(conf.sample_rate);
	}
}

void NeoSystem::renderFramebuffer(EmuVideo &video)
{
	video.startFrameWithFormat({}, videoPixmap());
}

void NeoSystem::runFrame(EmuSystemTaskContext taskCtx, EmuVideo *video, EmuAudio *audio)
{
	//logMsg("run frame %d", (int)processGfx);
	if(video)
		IG::fill(screenBuff, (uint16_t)current_pc_pal[4095]);
	main_frame(&taskCtx, this, video);
	auto audioFrames = updateAudioFramesPerVideoFrame();
	Uint16 audioBuff[audioFrames * 2];
	YM2610Update_stream(audioFrames, audioBuff);
	if(audio)
	{
		audio->writeFrames(audioBuff, audioFrames);
	}
}

FS::FileString NeoSystem::contentDisplayNameForPath(IG::CStringView path) const
{
	auto contentName = contentDisplayNameForPathDefaultImpl(path);
	if(contentName.empty())
		return {};
	auto ctx = appContext();
	ROM_DEF *drv = res_load_drv(&ctx, contentName.data());
	if(!drv)
		return contentName;
	auto freeDrv = IG::scopeGuard([&](){ free(drv); });
	return drv->longname;
}

void EmuApp::onCustomizeNavView(EmuApp::NavView &view)
{
	const Gfx::LGradientStopDesc navViewGrad[] =
	{
		{ .0, Gfx::VertexColorPixelFormat.build((255./255.) * .4, (215./255.) * .4, (0./255.) * .4, 1.) },
		{ .3, Gfx::VertexColorPixelFormat.build((255./255.) * .4, (215./255.) * .4, (0./255.) * .4, 1.) },
		{ .97, Gfx::VertexColorPixelFormat.build((85./255.) * .4, (71./255.) * .4, (0./255.) * .4, 1.) },
		{ 1., view.separatorColor() },
	};
	view.setBackgroundGradient(navViewGrad);
}

}

using namespace EmuEx;

CLINK int gn_strictROMChecking()
{
	return static_cast<NeoSystem&>(gSystem()).optionStrictROMChecking;
}

CLINK ROM_DEF *res_load_drv(void *contextPtr, const char *name)
{
	auto drvFilename = IG::format<FS::PathString>(DATAFILE_PREFIX "rom/{}.drv", name);
	auto io = EmuEx::openGngeoDataIO(*((IG::ApplicationContext*)contextPtr), drvFilename);
	if(!io)
	{
		logErr("Can't open driver %s", name);
		return nullptr;
	}

	// Fill out the driver struct
	auto drv = (ROM_DEF*)calloc(sizeof(ROM_DEF), 1);
	io.read(drv->name, 32);
	io.read(drv->parent, 32);
	io.read(drv->longname, 128);
	drv->year = io.get<uint32_t>(); // TODO: LE byte-swap on uint32_t reads
	for(auto i : iotaCount(10))
		drv->romsize[i] = io.get<uint32_t>();
	drv->nb_romfile = io.get<uint32_t>();
	for(auto i : iotaCount(drv->nb_romfile))
	{
		io.read(drv->rom[i].filename, 32);
		drv->rom[i].region = io.get<uint8_t>();
		drv->rom[i].src = io.get<uint32_t>();
		drv->rom[i].dest = io.get<uint32_t>();
		drv->rom[i].size = io.get<uint32_t>();
		drv->rom[i].crc = io.get<uint32_t>();
	}
	return drv;
}

CLINK void *res_load_data(void *contextPtr, const char *name)
{
	auto io = EmuEx::openGngeoDataIO(*((IG::ApplicationContext*)contextPtr), name);
	if(!io)
	{
		logErr("Can't data file %s", name);
		return nullptr;
	}
	auto size = io.size();
	auto buffer = (char*)malloc(size);
	io.read(buffer, size);
	return buffer;
}

CLINK void screen_update(void *emuTaskCtxPtr, void *neoSystemPtr, void *emuVideoPtr)
{
	auto taskCtxPtr = (EmuSystemTaskContext*)emuTaskCtxPtr;
	auto emuVideo = (EmuVideo*)emuVideoPtr;
	if(emuVideo) [[likely]]
	{
		//logMsg("screen render");
		emuVideo->startFrameWithFormat(*taskCtxPtr, ((NeoSystem*)neoSystemPtr)->videoPixmap());
	}
	else
	{
		//logMsg("skipping render");
	}
}

void open_nvram(void *contextPtr, char *name)
{
	auto &ctx = *((IG::ApplicationContext*)contextPtr);
	auto &sys = EmuEx::EmuApp::get(ctx).system();
	IG::FileUtils::readFromUri(ctx, EmuEx::nvramPath(sys), {memory.sram, 0x10000});
}

void open_memcard(void *contextPtr, char *name)
{
	auto &ctx = *((IG::ApplicationContext*)contextPtr);
	auto &sys = EmuEx::EmuApp::get(ctx).system();
	IG::FileUtils::readFromUri(ctx, EmuEx::memcardPath(sys), {memory.memcard, 0x800});
}

void sramWritten()
{
	EmuEx::gSystem().onBackupMemoryWritten(SRAM_DIRTY_BIT);
}

void memcardWritten()
{
	EmuEx::gSystem().onBackupMemoryWritten(MEMCARD_DIRTY_BIT);
}

void gn_init_pbar(unsigned action, int size)
{
	auto &sys = static_cast<NeoSystem&>(gSystem());
	logMsg("init pbar %d, %d", action, size);
	if(sys.onLoadProgress)
	{
		auto actionString = [](unsigned action)
		{
			switch(action)
			{
				default:
				case PBAR_ACTION_LOADROM: { return ""; } // defaults to "Loading..."
				case PBAR_ACTION_DECRYPT: { return "Decrypting..."; }
				case PBAR_ACTION_SAVEGNO: { return "Building Cache...\n(may take a while)"; };
			}
		};
		sys.onLoadProgress(0, size, actionString(action));
	}
}

void gn_update_pbar(int pos)
{
	auto &sys = static_cast<NeoSystem&>(gSystem());
	logMsg("update pbar %d", pos);
	if(sys.onLoadProgress)
	{
		sys.onLoadProgress(pos, 0, nullptr);
	}
}
