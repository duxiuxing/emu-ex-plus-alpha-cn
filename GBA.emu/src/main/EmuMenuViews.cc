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
#include "EmuCheatViews.hh"
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
			UI_TEXT("自动"),
			attachParams(), {.id = AutoTristate::Auto}
		},
		{
			UI_TEXT("关"),
			attachParams(), {.id = AutoTristate::Off}
		},
		{
			UI_TEXT("开"),
			attachParams(), {.id = AutoTristate::On}
		},
	};

	MultiChoiceMenuItem bios
	{
		UI_TEXT("使用 BIOS"),
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
			UI_TEXT("自动"),
			attachParams(), {.id = RtcMode::AUTO}
		},
		{
			UI_TEXT("关"),
			attachParams(), {.id = RtcMode::OFF}
		},
		{
			UI_TEXT("开"),
			attachParams(), {.id = RtcMode::ON}
		},
	};

	MultiChoiceMenuItem rtc
	{
		UI_TEXT("RTC 模拟"),
		attachParams(),
		MenuId{system().optionRtcEmulation.value()},
		rtcItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(idx == 0)
				{
					t.resetString(rtcIsEnabled()
						? UI_TEXT("开")
						: UI_TEXT("关")
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
			UI_TEXT("自动"),
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
			UI_TEXT("无"),
			attachParams(), {.id = packSaveTypeOverride(GBA_SAVE_NONE)}
		},
	};

	MultiChoiceMenuItem saveType
	{
		UI_TEXT("存档类型"),
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
						UI_TEXT("是否要改变存档类型？.sav 文件中的现有数据可能会丢失，在继续操作之前请做好备份。"),
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
			UI_TEXT("自动"),
			attachParams(), {.id = GbaSensorType::Auto}
		},
		{
			UI_TEXT("无"),
			attachParams(), {.id = GbaSensorType::None}
		},
		{
			UI_TEXT("加速度传感器"),
			attachParams(), {.id = GbaSensorType::Accelerometer}
		},
		{
			UI_TEXT("陀螺仪"),
			attachParams(), {.id = GbaSensorType::Gyroscope}
		},
		{
			UI_TEXT("光线感应器"),
			attachParams(), {.id = GbaSensorType::Light}
		},
	};

	MultiChoiceMenuItem hardwareSensor
	{
		UI_TEXT("硬件传感器"),
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
			UI_TEXT("主机选项"),
			attach,
			menuItem
		} {}
};

class CustomSystemActionsView : public SystemActionsView
{
	TextMenuItem options
	{
		UI_TEXT("主机选项"),
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
		UI_TEXT("音频混合器："),
		attachParams()
	};

	using VolumeChoiceItemArr = std::array<TextMenuItem, 3>;

	VolumeChoiceItemArr volumeLevelChoiceItems(bool gbVol)
	{
		return
		{
			TextMenuItem
			{
				UI_TEXT("默认"),
				attachParams(),
				[this, gbVol]() { soundSetVolume(gGba, 1.f, gbVol); },
				{.id = 100}
			},
			TextMenuItem
			{
				UI_TEXT("关"),
				attachParams(),
				[this, gbVol]() { soundSetVolume(gGba, 0, gbVol); },
				{.id = 0}
			},
			TextMenuItem
			{
				UI_TEXT("自定义"),
				attachParams(),
				[this, gbVol](Input::Event e)
				{
					pushAndShowNewCollectValueRangeInputView<int, 0, 100>(attachParams(), e,
						UI_TEXT("请输入 0 到 100 之间的值"),
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
				? UI_TEXT("GB APU 音量")
				: UI_TEXT("PCM 音量"),
			attachParams(),
			MenuId{soundVolumeAsInt(gGba, gbVol)},
			volumeLevelItem[gbVol ? 1 : 0],
			{
				.onSetDisplayString = [this, gbVol](auto idx, Gfx::Text &t)
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
			UI_TEXT("脉冲 #1"),
			0x1),
		channelEnableItem(
			UI_TEXT("脉冲 #2"),
			0x2),
		channelEnableItem(
			UI_TEXT("波形"),
			0x4),
		channelEnableItem(
			UI_TEXT("噪音"),
			0x8),
	};

	std::array<TextMenuItem, 2> filteringLevelItem
	{
		TextMenuItem
		{
			UI_TEXT("默认"),
			attachParams(),
			[this]() { soundSetFiltering(gGba, .5f); },
			{.id = 50}
		},
		TextMenuItem
		{
			UI_TEXT("自定义"),
			attachParams(),
			[this](Input::Event e)
			{
				pushAndShowNewCollectValueRangeInputView<int, 0, 100>(attachParams(), e,
					UI_TEXT("请输入 0 到 100 之间的值"),
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
		UI_TEXT("过滤级别"),
		attachParams(),
		MenuId{soundFilteringAsInt(gGba)},
		filteringLevelItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
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
		UI_TEXT("滤波器"),
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
		UI_TEXT("默认使用 BIOS"),
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
			UI_TEXT("夜晚"),
			attachParams(), {.id = 0}
		},
		{
			UI_TEXT("室内"),
			attachParams(), {.id = 100}},
		{
			UI_TEXT("阴天"),
			attachParams(), {.id = 1000}
		},
		{
			UI_TEXT("晴天"),
			attachParams(), {.id = 10000}
		},
		{
			UI_TEXT("自定义"),
			attachParams(),
			[this](Input::Event e)
			{
				pushAndShowNewCollectValueRangeInputView<int, 0, 50000>(attachParams(), e,
					UI_TEXT("请输入 0 到 50000 之间的值"),
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
		UI_TEXT("光照强度"),
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
				UI_TEXT("金手指文件夹"),
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
				UI_TEXT("补丁文件夹"),
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
			UI_TEXT("BIOS：{}"),
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
		case ViewID::EDIT_CHEATS: return std::make_unique<EmuEditCheatListView>(attach);
		case ViewID::LIST_CHEATS: return std::make_unique<EmuCheatsView>(attach);
		default: return nullptr;
	}
}

}
