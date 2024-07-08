#include <emuframework/EmuApp.hh>
#include <emuframework/AudioOptionView.hh>
#include <emuframework/FilePathOptionView.hh>
#include <emuframework/DataPathSelectView.hh>
#include <emuframework/UserPathSelectView.hh>
#include <emuframework/SystemActionsView.hh>
#include <emuframework/viewUtils.hh>
#include "EmuCheatViews.hh"
#include "MainApp.hh"
#include <imagine/util/format.hh>
#ifndef SNES9X_VERSION_1_4
#include <apu/apu.h>
#include <apu/bapu/snes/snes.hpp>
#include <ppu.h>
#endif
#include <imagine/logger/logger.h>

namespace EmuEx
{

using MainAppHelper = EmuAppHelperBase<MainApp>;

constexpr bool HAS_NSRT = !IS_SNES9X_VERSION_1_4;

#ifndef SNES9X_VERSION_1_4
class CustomAudioOptionView : public AudioOptionView, public MainAppHelper
{
	using MainAppHelper::system;

	void setDSPInterpolation(uint8_t val)
	{
		logMsg("set DSP interpolation:%u", val);
		system().optionAudioDSPInterpolation = val;
		SNES::dsp.spc_dsp.interpolation = val;
	}

	TextMenuItem dspInterpolationItem[5]
	{
		{
			UI_TEXT("None"),
			attachParams(), [this](){ setDSPInterpolation(0); }
		},
		{
			UI_TEXT("Linear"),
			attachParams(), [this](){ setDSPInterpolation(1); }
		},
		{
			UI_TEXT("Gaussian"),
			attachParams(), [this](){ setDSPInterpolation(2); }
		},
		{
			UI_TEXT("Cubic"),
			attachParams(), [this](){ setDSPInterpolation(3); }
		},
		{
			UI_TEXT("Sinc"),
			attachParams(), [this](){ setDSPInterpolation(4); }
		},
	};

	MultiChoiceMenuItem dspInterpolation
	{
		UI_TEXT("DSP Interpolation"),
		attachParams(),
		system().optionAudioDSPInterpolation.value(),
		dspInterpolationItem
	};

public:
	CustomAudioOptionView(ViewAttachParams attach, EmuAudio &audio): AudioOptionView{attach, audio, true}
	{
		loadStockItems();
		item.emplace_back(&dspInterpolation);
	}
};
#endif

class ConsoleOptionView : public TableView, public MainAppHelper
{
	BoolMenuItem multitap
	{
		UI_TEXT("5-Player Adapter"),
		attachParams(),
		(bool)system().optionMultitap,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			system().sessionOptionSet();
			system().optionMultitap = item.flipBoolValue(*this);
			system().setupSNESInput(app().defaultVController());
		}
	};

	TextMenuItem inputPortsItem[HAS_NSRT ? 5 : 4]
	{
		#ifndef SNES9X_VERSION_1_4
		{
			UI_TEXT("Auto (NSRT)"),
			attachParams(), setInputPortsDel(), {.id = SNES_AUTO_INPUT}
		},
		#endif
		{
			UI_TEXT("Gamepads"),
			attachParams(), setInputPortsDel(), {.id = SNES_JOYPAD}
		},
		{
			UI_TEXT("Superscope"),
			attachParams(), setInputPortsDel(), {.id = SNES_SUPERSCOPE}
		},
		{
			UI_TEXT("Justifier"),
			attachParams(), setInputPortsDel(), {.id = SNES_JUSTIFIER}
		},
		{
			UI_TEXT("Mouse"),
			attachParams(), setInputPortsDel(), {.id = SNES_MOUSE_SWAPPED}
		},
	};

	MultiChoiceMenuItem inputPorts
	{
		UI_TEXT("Input Ports"),
		attachParams(),
		MenuId{system().snesInputPort},
		inputPortsItem
	};

	TextMenuItem::SelectDelegate setInputPortsDel()
	{
		return [this](TextMenuItem &item)
		{
			system().sessionOptionSet();
			system().optionInputPort = item.id;
			system().snesInputPort = item.id;
			system().setupSNESInput(app().defaultVController());
		};
	}

	TextMenuItem videoSystemItem[4]
	{
		{
			UI_TEXT("Auto"),
			attachParams(), [this](Input::Event e){ setVideoSystem(0, e); }
		},
		{
			UI_TEXT("NTSC"),
			attachParams(), [this](Input::Event e){ setVideoSystem(1, e); }
		},
		{
			UI_TEXT("PAL"),
			attachParams(), [this](Input::Event e){ setVideoSystem(2, e); }
		},
		{
			UI_TEXT("NTSC + PAL Spoof"),
			attachParams(), [this](Input::Event e){ setVideoSystem(3, e); }
		},
	};

	MultiChoiceMenuItem videoSystem
	{
		UI_TEXT("System"),
		attachParams(),
		system().optionVideoSystem.value(),
		videoSystemItem
	};

	void setVideoSystem(int val, Input::Event e)
	{
		system().sessionOptionSet();
		system().optionVideoSystem = val;
		app().promptSystemReloadDueToSetOption(attachParams(), e);
	}

	TextHeadingMenuItem videoHeading
	{
		UI_TEXT("Video"),
		attachParams()
	};

	BoolMenuItem allowExtendedLines
	{
		UI_TEXT("Allow Extended 239/478 Lines"),
		attachParams(),
		(bool)system().optionAllowExtendedVideoLines,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			system().sessionOptionSet();
			system().optionAllowExtendedVideoLines = item.flipBoolValue(*this);
		}
	};

	TextMenuItem deinterlaceModeItems[2]
	{
		{
			UI_TEXT("Bob"),
			attachParams(), {.id = DeinterlaceMode::Bob}
		},
		{
			UI_TEXT("Weave"),
			attachParams(), {.id = DeinterlaceMode::Weave}
		},
	};

	MultiChoiceMenuItem deinterlaceMode
	{
		UI_TEXT("Deinterlace Mode"),
		attachParams(),
		MenuId{system().deinterlaceMode},
		deinterlaceModeItems,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				system().sessionOptionSet();
				system().deinterlaceMode = DeinterlaceMode(item.id.val);
			}
		}
	};

	#ifndef SNES9X_VERSION_1_4
	TextHeadingMenuItem emulationHacks
	{
		UI_TEXT("Emulation Hacks"),
		attachParams()
	};

	BoolMenuItem blockInvalidVRAMAccess
	{
		UI_TEXT("Allow Invalid VRAM Access"),
		attachParams(),
		(bool)!system().optionBlockInvalidVRAMAccess,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			system().sessionOptionSet();
			system().optionBlockInvalidVRAMAccess = !item.flipBoolValue(*this);
			PPU.BlockInvalidVRAMAccess = system().optionBlockInvalidVRAMAccess;
		}
	};

	BoolMenuItem separateEchoBuffer
	{
		UI_TEXT("Separate Echo Buffer From Ram"),
		attachParams(),
		(bool)system().optionSeparateEchoBuffer,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			system().sessionOptionSet();
			system().optionSeparateEchoBuffer = item.flipBoolValue(*this);
			SNES::dsp.spc_dsp.separateEchoBuffer = system().optionSeparateEchoBuffer;
		}
	};

	void setSuperFXClock(unsigned val)
	{
		system().sessionOptionSet();
		system().optionSuperFXClockMultiplier = val;
		setSuperFXSpeedMultiplier(system().optionSuperFXClockMultiplier);
	}

	TextMenuItem superFXClockItem[2]
	{
		{
			UI_TEXT("100%"),
			attachParams(), [this]() { setSuperFXClock(100); }
		},
		{
			UI_TEXT("Custom Value"),
			attachParams(),
			[this](Input::Event e)
			{
				pushAndShowNewCollectValueInputView<int>(attachParams(), e,
					UI_TEXT("Input 5 to 250"),
					"",
					[this](CollectTextInputView&, auto val)
					{
						if(system().optionSuperFXClockMultiplier.isValid(val))
						{
							setSuperFXClock(val);
							superFXClock.setSelected(lastIndex(superFXClockItem), *this);
							dismissPrevious();
							return true;
						}
						else
						{
							app().postErrorMessage(
								UI_TEXT("Value not in range")
							);
							return false;
						}
					});
				return false;
			}
		},
	};

	MultiChoiceMenuItem superFXClock
	{
		UI_TEXT("SuperFX Clock Multiplier"),
		attachParams(),
		[this]()
		{
			if(system().optionSuperFXClockMultiplier == 100)
				return 0;
			else
				return 1;
		}(),
		superFXClockItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format(
					UI_TEXT("{}%"),
					system().optionSuperFXClockMultiplier.value()));
				return true;
			}
		},
	};
	#endif

	std::array<MenuItem*, IS_SNES9X_VERSION_1_4 ? 6 : 10> menuItem
	{
		&inputPorts,
		&multitap,
		&videoHeading,
		&videoSystem,
		&allowExtendedLines,
		&deinterlaceMode,
		#ifndef SNES9X_VERSION_1_4
		&emulationHacks,
		&blockInvalidVRAMAccess,
		&separateEchoBuffer,
		&superFXClock,
		#endif
	};

public:
	ConsoleOptionView(ViewAttachParams attach):
		TableView
		{
			UI_TEXT("Console Options"),
			attach,
			menuItem
		}
	{}
};

class CustomSystemActionsView : public SystemActionsView
{
private:
	TextMenuItem options
	{
		UI_TEXT("Console Options"),
		attachParams(),
		[this](TextMenuItem &, View &, Input::Event e)
		{
			if(system().hasContent())
			{
				pushAndShow(makeView<ConsoleOptionView>(), e);
			}
		}
	};

public:
	CustomSystemActionsView(ViewAttachParams attach): SystemActionsView{attach, true}
	{
		item.emplace_back(&options);
		loadStandardItems();
	}
};

class CustomFilePathOptionView : public FilePathOptionView, public MainAppHelper
{
	using MainAppHelper::system;
	using MainAppHelper::app;

	TextMenuItem cheatsPath
	{
		cheatsMenuName(appContext(), system().cheatsDir), attachParams(),
		[this](const Input::Event &e)
		{
			pushAndShow(makeViewWithName<UserPathSelectView>(
				UI_TEXT("Cheats"),
				system().userPath(system().cheatsDir),
				[this](CStringView path)
				{
					logMsg("set cheats path:%s", path.data());
					system().cheatsDir = path;
					cheatsPath.compile(cheatsMenuName(appContext(), path));
				}), e);
		}
	};

	TextMenuItem patchesPath
	{
		patchesMenuName(appContext(), system().patchesDir), attachParams(),
		[this](const Input::Event &e)
		{
			pushAndShow(makeViewWithName<UserPathSelectView>(
				UI_TEXT("Patches"),
				system().userPath(system().patchesDir),
				[this](CStringView path)
				{
					logMsg("set patches path:%s", path.data());
					system().patchesDir = path;
					patchesPath.compile(patchesMenuName(appContext(), path));
				}), e);
		}
	};

	static std::string satMenuName(IG::ApplicationContext ctx, std::string_view userPath)
	{
		return std::format(
			UI_TEXT("Satellaview Files: {}"),
			userPathToDisplayName(ctx, userPath));
	}

	TextMenuItem satPath
	{
		satMenuName(appContext(), system().satDir), attachParams(),
		[this](const Input::Event &e)
		{
			pushAndShow(makeViewWithName<UserPathSelectView>(
				UI_TEXT("Satellaview Files"),
				system().userPath(system().satDir),
				[this](CStringView path)
				{
					logMsg("set satellaview files path:%s", path.data());
					system().satDir = path;
					satPath.compile(satMenuName(appContext(), path));
				}), e);
		}
	};

	TextMenuItem bsxBios
	{
		bsxMenuName(system().bsxBiosPath), attachParams(),
		[this](const Input::Event &e)
		{
			pushAndShow(makeViewWithName<DataFileSelectView<>>(
				UI_TEXT("BS-X BIOS"),
				app().validSearchPath(FS::dirnameUri(system().bsxBiosPath)),
				[this](CStringView path, FS::file_type type)
				{
					system().bsxBiosPath = path;
					logMsg("set BS-X bios:%s", path.data());
					bsxBios.compile(bsxMenuName(path));
					return true;
				}, Snes9xSystem::hasBiosExtension), e);
		}
	};

	std::string bsxMenuName(CStringView path) const
	{
		return std::format(
			UI_TEXT("BS-X BIOS: {}"),
			appContext().fileUriDisplayName(path));
	}

	TextMenuItem sufamiBios
	{
		sufamiMenuName(system().sufamiBiosPath), attachParams(),
		[this](const Input::Event &e)
		{
			pushAndShow(makeViewWithName<DataFileSelectView<>>(
				UI_TEXT("Sufami Turbo BIOS"),
				app().validSearchPath(FS::dirnameUri(system().sufamiBiosPath)),
				[this](CStringView path, FS::file_type type)
				{
					system().sufamiBiosPath = path;
					logMsg("set Sufami Turbo bios:%s", path.data());
					sufamiBios.compile(sufamiMenuName(path));
					return true;
				}, Snes9xSystem::hasBiosExtension), e);
		}
	};

	std::string sufamiMenuName(CStringView path) const
	{
		return std::format(
			UI_TEXT("Sufami Turbo BIOS: {}"),
			appContext().fileUriDisplayName(path));
	}

public:
	CustomFilePathOptionView(ViewAttachParams attach): FilePathOptionView{attach, true}
	{
		loadStockItems();
		item.emplace_back(&cheatsPath);
		item.emplace_back(&patchesPath);
		item.emplace_back(&satPath);
		item.emplace_back(&bsxBios);
		item.emplace_back(&sufamiBios);
	}
};

std::unique_ptr<View> EmuApp::makeCustomView(ViewAttachParams attach, ViewID id)
{
	switch(id)
	{
		#ifndef SNES9X_VERSION_1_4
		case ViewID::AUDIO_OPTIONS: return std::make_unique<CustomAudioOptionView>(attach, audio);
		#endif
		case ViewID::FILE_PATH_OPTIONS: return std::make_unique<CustomFilePathOptionView>(attach);
		case ViewID::SYSTEM_ACTIONS: return std::make_unique<CustomSystemActionsView>(attach);
		case ViewID::EDIT_CHEATS: return std::make_unique<EmuEditCheatListView>(attach);
		case ViewID::LIST_CHEATS: return std::make_unique<EmuCheatsView>(attach);
		default: return nullptr;
	}
}

}
