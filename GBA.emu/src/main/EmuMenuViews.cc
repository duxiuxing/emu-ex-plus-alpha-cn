/*  This file is part of GBA.emu.

	GBA.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	GBA.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with GBA.emu.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/EmuApp.hh>
#include <emuframework/SystemOptionView.hh>
#include <emuframework/AudioOptionView.hh>
#include <emuframework/FilePathOptionView.hh>
#include <emuframework/UserPathSelectView.hh>
#include <emuframework/SystemActionsView.hh>
#include <emuframework/DataPathSelectView.hh>
#include <emuframework/viewUtils.hh>
#include "MainApp.hh"
#include "GBASys.hh"
#include <imagine/gui/AlertView.hh>
#include <imagine/util/format.hh>
#include <imagine/util/string.h>
#include <core/gba/gba.h>
#include <core/gba/gbaRtc.h>
#include <core/gba/gbaSound.h>
#include <format>
#include <imagine/logger/logger.h>

namespace EmuEx
{

constexpr SystemLogger log{"GBA.emu"};

using MainAppHelper = EmuAppHelperBase<MainApp>;

class ConsoleOptionView : public TableView, public MainAppHelper
{
	TextMenuItem biosItems[3]
	{
		{
			UI_TEXT("Auto"),
			attachParams(), {.id = AutoTristate::Auto}
		},
		{
			UI_TEXT("Off"),
			attachParams(), {.id = AutoTristate::Off}
		},
		{
			UI_TEXT("On"),
			attachParams(), {.id = AutoTristate::On}
		},
	};

	MultiChoiceMenuItem bios
	{
		UI_TEXT("Use BIOS"),
		attachParams(),
		MenuId{system().useBios.value()},
		biosItems,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item, const Input::Event &e)
			{
				system().sessionOptionSet();
				system().useBios = AutoTristate(item.id.val);
				app().promptSystemReloadDueToSetOption(attachParams(), e);
			}
		}
	};

	TextMenuItem rtcItem[3]
	{
		{
			UI_TEXT("Auto"),
			attachParams(), {.id = RtcMode::AUTO}
		},
		{
			UI_TEXT("Off"),
			attachParams(), {.id = RtcMode::OFF}
		},
		{
			UI_TEXT("On"),
			attachParams(), {.id = RtcMode::ON}
		},
	};

	MultiChoiceMenuItem rtc
	{
		UI_TEXT("RTC Emulation"),
		attachParams(),
		MenuId{system().optionRtcEmulation.value()},
		rtcItem,
		{
			.onSetDisplayString = [](auto idx, Gfx::Text& t)
			{
				if(idx == 0)
				{
					t.resetString(rtcIsEnabled()
						? UI_TEXT("On")
						: UI_TEXT("Off")
					);
					return true;
				}
				return false;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				system().sessionOptionSet();
				system().setRTC(system().optionRtcEmulation = RtcMode(item.id.val));
			}
		}
	};

	TextMenuItem saveTypeItem[7]
	{
		{
			UI_TEXT("Auto"),
			attachParams(), {.id = packSaveTypeOverride(GBA_SAVE_AUTO)}
		},
		{
			UI_TEXT("EEPROM"),
			attachParams(), {.id = packSaveTypeOverride(GBA_SAVE_EEPROM)}
		},
		{
			UI_TEXT("SRAM"),
			attachParams(), {.id = packSaveTypeOverride(GBA_SAVE_SRAM)}
		},
		{
			UI_TEXT("Flash (64K)"),
			attachParams(), {.id = packSaveTypeOverride(GBA_SAVE_FLASH, SIZE_FLASH512)}
		},
		{
			UI_TEXT("Flash (128K)"),
			attachParams(), {.id = packSaveTypeOverride(GBA_SAVE_FLASH, SIZE_FLASH1M)}
		},
		{
			UI_TEXT("EEPROM + Sensor"),
			attachParams(), {.id = packSaveTypeOverride(GBA_SAVE_EEPROM_SENSOR)}
		},
		{
			UI_TEXT("None"),
			attachParams(), {.id = packSaveTypeOverride(GBA_SAVE_NONE)}
		},
	};

	MultiChoiceMenuItem saveType
	{
		UI_TEXT("Save Type"),
		attachParams(),
		MenuId{system().optionSaveTypeOverride},
		saveTypeItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(idx == 0)
				{
					t.resetString(saveTypeStr(system().detectedSaveType, system().detectedSaveSize));
					return true;
				}
				return false;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item, const Input::Event &e)
			{
				if(system().optionSaveTypeOverride == (uint32_t)item.id)
					return true;
				static auto setSaveTypeOption = [](GbaApp &app, uint32_t optVal, ViewAttachParams attach, const Input::Event &e)
				{
					app.system().sessionOptionSet();
					app.system().optionSaveTypeOverride = optVal;
					app.promptSystemReloadDueToSetOption(attach, e);
				};
				if(saveMemoryHasContent())
				{
					pushAndShowModal(makeView<YesNoAlertView>(
						UI_TEXT("Really change save type? Existing data in .sav file may be lost so please make a backup before proceeding."),
						YesNoAlertView::Delegates
						{
							.onYes = [this, optVal = item.id](const Input::Event &e)
							{
								setSaveTypeOption(app(), optVal, attachParams(), e);
							}
						}), e);
					return false;
				}
				else
				{
					setSaveTypeOption(app(), item.id, attachParams(), e);
					return true;
				}
			}
		}
	};

	#ifdef IG_CONFIG_SENSORS
	TextMenuItem hardwareSensorItem[5]
	{
		{
			UI_TEXT("Auto"),
			attachParams(), {.id = GbaSensorType::Auto}
		},
		{
			UI_TEXT("None"),
			attachParams(), {.id = GbaSensorType::None}
		},
		{
			UI_TEXT("Accelerometer"),
			attachParams(), {.id = GbaSensorType::Accelerometer}
		},
		{
			UI_TEXT("Gyroscope"),
			attachParams(), {.id = GbaSensorType::Gyroscope}
		},
		{
			UI_TEXT("Light"),
			attachParams(), {.id = GbaSensorType::Light}
		},
	};

	MultiChoiceMenuItem hardwareSensor
	{
		UI_TEXT("Hardware Sensor"),
		attachParams(),
		MenuId{system().sensorType},
		hardwareSensorItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(idx == 0)
				{
					t.resetString(wise_enum::to_string(system().detectedSensorType));
					return true;
				}
				return false;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				system().setSensorType(GbaSensorType(item.id.val));
			}
		},
	};
	#endif

	std::array<MenuItem*, Config::SENSORS ? 4 : 3> menuItem
	{
		&bios,
		&rtc
		, &saveType
		#ifdef IG_CONFIG_SENSORS
		, &hardwareSensor
		#endif
	};

public:
	ConsoleOptionView(ViewAttachParams attach):
		TableView
		{
			UI_TEXT("Console Options"),
			attach,
			menuItem
		} {}
};

class CustomSystemActionsView : public SystemActionsView
{
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

class CustomAudioOptionView : public AudioOptionView, public MainAppHelper
{
	using MainAppHelper::system;
	using MainAppHelper::app;

	TextHeadingMenuItem mixer
	{
		UI_TEXT("Mixer"),
		attachParams()
	};

	using VolumeChoiceItemArr = std::array<TextMenuItem, 3>;

	VolumeChoiceItemArr volumeLevelChoiceItems(bool gbVol)
	{
		return
		{
			TextMenuItem
			{
				UI_TEXT("Default"),
				attachParams(),
				[gbVol]() { soundSetVolume(gGba, 1.f, gbVol); },
				{.id = 100}
			},
			TextMenuItem
			{
				UI_TEXT("Off"),
				attachParams(),
				[gbVol]() { soundSetVolume(gGba, 0, gbVol); },
				{.id = 0}
			},
			TextMenuItem
			{
				UI_TEXT("Custom Value"),
				attachParams(),
				[this, gbVol](Input::Event e)
				{
					pushAndShowNewCollectValueRangeInputView<int, 0, 100>(attachParams(), e,
						UI_TEXT("Input 0 to 100"),
						"",
						[this, gbVol](CollectTextInputView&, auto val)
						{
							soundSetVolume(gGba, val / 100.f, gbVol);
							size_t idx = gbVol ? 1 : 0;
							volumeLevel[idx].setSelected(MenuId{val}, *this);
							dismissPrevious();
							return true;
						});
					return false;
				}, {.id = defaultMenuId}
			}
		};
	}

	std::array<VolumeChoiceItemArr, 2> volumeLevelItem
	{
		volumeLevelChoiceItems(false),
		volumeLevelChoiceItems(true),
	};

	MultiChoiceMenuItem volumeLevelMenuItem(bool gbVol)
	{
		return
		{
			gbVol
				? UI_TEXT("GB APU Volume")
				: UI_TEXT("PCM Volume"),
			attachParams(),
			MenuId{soundVolumeAsInt(gGba, gbVol)},
			volumeLevelItem[gbVol ? 1 : 0],
			{
				.onSetDisplayString = [gbVol](auto idx, Gfx::Text& t)
				{
					t.resetString(std::format(
						UI_TEXT("{}%"),
						soundVolumeAsInt(gGba, gbVol)));
					return true;
				}
			},
		};
	}

	std::array<MultiChoiceMenuItem, 2> volumeLevel
	{
		volumeLevelMenuItem(false),
		volumeLevelMenuItem(true),
	};

	BoolMenuItem channelEnableItem(auto &&name, int mask)
	{
		return
		{
			IG_forward(name), attachParams(),
			bool(soundGetEnable(gGba) & mask),
			[this, mask](BoolMenuItem &item)
			{
				soundSetEnable(gGba, setOrClearBits(soundGetEnable(gGba), mask, item.flipBoolValue(*this)));
			}
		};
	}

	std::array<BoolMenuItem, 6> channelEnable
	{
		channelEnableItem(
			UI_TEXT("PCM #1"),
			0x100),
		channelEnableItem(
			UI_TEXT("PCM #2"),
			0x200),
		channelEnableItem(
			UI_TEXT("Pulse #1"),
			0x1),
		channelEnableItem(
			UI_TEXT("Pulse #2"),
			0x2),
		channelEnableItem(
			UI_TEXT("Wave"),
			0x4),
		channelEnableItem(
			UI_TEXT("Noise"),
			0x8),
	};

	std::array<TextMenuItem, 2> filteringLevelItem
	{
		TextMenuItem
		{
			UI_TEXT("Default"),
			attachParams(),
			[]() { soundSetFiltering(gGba, .5f); },
			{.id = 50}
		},
		TextMenuItem
		{
			UI_TEXT("Custom Value"),
			attachParams(),
			[this](Input::Event e)
			{
				pushAndShowNewCollectValueRangeInputView<int, 0, 100>(attachParams(), e,
					UI_TEXT("Input 0 to 100"),
					"",
					[this](CollectTextInputView&, auto val)
					{
						soundSetFiltering(gGba, val / 100.f);
						filteringLevel.setSelected(MenuId{val}, *this);
						dismissPrevious();
						return true;
					});
				return false;
			}, {.id = defaultMenuId}
		}
	};

	MultiChoiceMenuItem filteringLevel
	{
		UI_TEXT("Filtering Level"),
		attachParams(),
		MenuId{soundFilteringAsInt(gGba)},
		filteringLevelItem,
		{
			.onSetDisplayString = [](auto, Gfx::Text& t)
			{
				t.resetString(std::format(
					UI_TEXT("{}%"),
					soundFilteringAsInt(gGba)));
				return true;
			}
		},
	};

	BoolMenuItem filtering
	{
		UI_TEXT("Filtering"),
		attachParams(),
		soundGetInterpolation(gGba),
		[this](BoolMenuItem &item)
		{
			soundSetInterpolation(gGba, item.flipBoolValue(*this));
		}
	};

public:
	CustomAudioOptionView(ViewAttachParams attach, EmuAudio& audio): AudioOptionView{attach, audio, true}
	{
		loadStockItems();
		item.emplace_back(&filtering);
		item.emplace_back(&filteringLevel);
		item.emplace_back(&mixer);
		item.emplace_back(&volumeLevel[0]);
		item.emplace_back(&channelEnable[0]);
		item.emplace_back(&channelEnable[1]);
		item.emplace_back(&volumeLevel[1]);
		item.emplace_back(&channelEnable[2]);
		item.emplace_back(&channelEnable[3]);
		item.emplace_back(&channelEnable[4]);
		item.emplace_back(&channelEnable[5]);
	}
};

class CustomSystemOptionView : public SystemOptionView, public MainAppHelper
{
	using MainAppHelper::system;
	using MainAppHelper::app;

	BoolMenuItem bios
	{
		UI_TEXT("Default Use BIOS"),
		attachParams(),
		system().defaultUseBios,
		[this](BoolMenuItem &item)
		{
			system().defaultUseBios = item.flipBoolValue(*this);
		}
	};

	#ifdef IG_CONFIG_SENSORS
	TextMenuItem lightSensorScaleItem[5]
	{
		{
			UI_TEXT("Darkness"),
			attachParams(), {.id = 0}
		},
		{
			UI_TEXT("Indoor Light"),
			attachParams(), {.id = 100}},
		{
			UI_TEXT("Overcast Day"),
			attachParams(), {.id = 1000}
		},
		{
			UI_TEXT("Sunny Day"),
			attachParams(), {.id = 10000}
		},
		{
			UI_TEXT("Custom Value"),
			attachParams(),
			[this](Input::Event e)
			{
				pushAndShowNewCollectValueRangeInputView<int, 0, 50000>(attachParams(), e,
					UI_TEXT("Input 0 to 50000"),
					"",
					[this](CollectTextInputView&, auto val)
					{
						system().lightSensorScaleLux = val;
						lightSensorScale.setSelected(MenuId{val}, *this);
						dismissPrevious();
						return true;
					});
				return false;
			}, {.id = defaultMenuId}
		}
	};

	MultiChoiceMenuItem lightSensorScale
	{
		UI_TEXT("Light Sensor Scale"),
		attachParams(),
		MenuId{system().lightSensorScaleLux},
		lightSensorScaleItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format(
					UI_TEXT("{} lux"),
					(int)system().lightSensorScaleLux));
				return true;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				system().lightSensorScaleLux = item.id;
			}
		},
	};
	#endif

public:
	CustomSystemOptionView(ViewAttachParams attach): SystemOptionView{attach, true}
	{
		loadStockItems();
		item.emplace_back(&bios);
		#ifdef IG_CONFIG_SENSORS
		item.emplace_back(&lightSensorScale);
		#endif
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

	TextMenuItem biosPath
	{
		biosMenuEntryStr(system().biosPath), attachParams(),
		[this](Input::Event e)
		{
			pushAndShow(makeViewWithName<DataFileSelectView<>>(
				UI_TEXT("BIOS"),
				app().validSearchPath(FS::dirnameUri(system().biosPath)),
				[this](CStringView path, FS::file_type type)
				{
					system().biosPath = path;
					log.info("set BIOS:{}", system().biosPath);
					biosPath.compile(biosMenuEntryStr(path));
					return true;
				}, hasBiosExtension), e);
		}
	};

	std::string biosMenuEntryStr(std::string_view path) const
	{
		return std::format(
			UI_TEXT("BIOS: {}"),
			appContext().fileUriDisplayName(path));
	}

	static bool hasBiosExtension(std::string_view name)
	{
		return endsWithAnyCaseless(name, ".bin", ".rom");
	}

public:
	CustomFilePathOptionView(ViewAttachParams attach): FilePathOptionView{attach, true}
	{
		loadStockItems();
		item.emplace_back(&cheatsPath);
		item.emplace_back(&patchesPath);
		item.emplace_back(&biosPath);
	}
};

std::unique_ptr<View> EmuApp::makeCustomView(ViewAttachParams attach, ViewID id)
{
	switch(id)
	{
		case ViewID::SYSTEM_ACTIONS: return std::make_unique<CustomSystemActionsView>(attach);
		case ViewID::SYSTEM_OPTIONS: return std::make_unique<CustomSystemOptionView>(attach);
		case ViewID::AUDIO_OPTIONS: return std::make_unique<CustomAudioOptionView>(attach, audio);
		case ViewID::FILE_PATH_OPTIONS: return std::make_unique<CustomFilePathOptionView>(attach);
		default: return nullptr;
	}
}

}
