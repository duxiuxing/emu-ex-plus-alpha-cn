/*  This file is part of Saturn.emu.

	Saturn.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Saturn.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Saturn.emu.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/EmuViewController.hh>
#include <emuframework/SystemOptionView.hh>
#include <emuframework/AudioOptionView.hh>
#include <emuframework/VideoOptionView.hh>
#include <emuframework/FilePathOptionView.hh>
#include <emuframework/DataPathSelectView.hh>
#include <emuframework/SystemActionsView.hh>
#include <emuframework/EmuInput.hh>
#include <mednafen-emuex/MDFNUtils.hh>
#include "MainApp.hh"
#include <imagine/fs/FS.hh>
#include <imagine/gui/AlertView.hh>
#include <imagine/util/format.hh>
#include <ss/cart.h>
#include <imagine/logger/logger.h>

namespace EmuEx
{

using MainAppHelper = EmuAppHelperBase<MainApp>;
using namespace MDFN_IEN_SS;

constexpr SystemLogger log{"Saturn.emu"};

static bool hasBIOSExtension(std::string_view name)
{
	return endsWithAnyCaseless(name, ".bin");
}

class CustomFilePathOptionView : public FilePathOptionView, public MainAppHelper
{
	using MainAppHelper::app;
	using MainAppHelper::system;

	TextMenuItem naBiosPath
	{
		naBiosMenuEntryStr(system().naBiosPath), attachParams(),
		[this](Input::Event e)
		{
			pushAndShow(makeViewWithName<DataFileSelectView<ArchivePathSelectMode::exclude>>(
				UI_TEXT("NA/EU BIOS"),
				app().validSearchPath(FS::dirnameUri(system().naBiosPath)),
				[this](CStringView path, FS::file_type type)
				{
					system().naBiosPath = path;
					log.info("set bios:{}", system().naBiosPath);
					naBiosPath.compile(naBiosMenuEntryStr(path));
					return true;
				}, hasBIOSExtension), e);
		}
	};

	std::string naBiosMenuEntryStr(std::string_view path) const
	{
		return std::format(
			UI_TEXT("NA/EU BIOS 文件：{}"),
			appContext().fileUriDisplayName(path));
	}

	TextMenuItem jpBiosPath
	{
		jpBiosMenuEntryStr(system().jpBiosPath), attachParams(),
		[this](Input::Event e)
		{
			pushAndShow(makeViewWithName<DataFileSelectView<ArchivePathSelectMode::exclude>>(
				UI_TEXT("JP BIOS"),
				app().validSearchPath(FS::dirnameUri(system().jpBiosPath)),
				[this](CStringView path, FS::file_type type)
				{
					system().jpBiosPath = path;
					log.info("set bios:{}", system().jpBiosPath);
					jpBiosPath.compile(jpBiosMenuEntryStr(path));
					return true;
				}, hasBIOSExtension), e);
		}
	};

	std::string jpBiosMenuEntryStr(std::string_view path) const
	{
		return std::format(
			UI_TEXT("JP BIOS 文件：{}"),
			appContext().fileUriDisplayName(path));
	}

	TextMenuItem kof95ROMPath
	{
		kof95MenuEntryStr(system().kof95ROMPath), attachParams(),
		[this](Input::Event e)
		{
			pushAndShow(makeViewWithName<DataFileSelectView<ArchivePathSelectMode::exclude>>(
				UI_TEXT("格斗之王 '95 ROM"),
				app().validSearchPath(FS::dirnameUri(system().kof95ROMPath)),
				[this](CStringView path, FS::file_type type)
				{
					system().kof95ROMPath = path;
					log.info("set bios:{}", system().kof95ROMPath);
					kof95ROMPath.compile(kof95MenuEntryStr(path));
					return true;
				}, hasBIOSExtension), e);
		}
	};

	std::string kof95MenuEntryStr(std::string_view path) const
	{
		return std::format(
			UI_TEXT("格斗之王 '95 ROM 文件：{}"),
			appContext().fileUriDisplayName(path));
	}

	TextMenuItem ultramanROMPath
	{
		ultramanMenuEntryStr(system().ultramanROMPath), attachParams(),
		[this](Input::Event e)
		{
			pushAndShow(makeViewWithName<DataFileSelectView<ArchivePathSelectMode::exclude>>(
				UI_TEXT("奥特曼 ROM"),
				app().validSearchPath(FS::dirnameUri(system().ultramanROMPath)),
				[this](CStringView path, FS::file_type type)
				{
					system().ultramanROMPath = path;
					log.info("set bios:{}", system().ultramanROMPath);
					ultramanROMPath.compile(ultramanMenuEntryStr(path));
					return true;
				}, hasBIOSExtension), e);
		}
	};

	std::string ultramanMenuEntryStr(std::string_view path) const
	{
		return std::format(
			UI_TEXT("奥特曼 ROM 文件：{}"),
			appContext().fileUriDisplayName(path));
	}

public:
	CustomFilePathOptionView(ViewAttachParams attach): FilePathOptionView{attach, true}
	{
		loadStockItems();
		item.emplace_back(&naBiosPath);
		item.emplace_back(&jpBiosPath);
		item.emplace_back(&kof95ROMPath);
		item.emplace_back(&ultramanROMPath);
	}
};

constexpr auto cartTypeToString(int t)
{
	switch(t)
	{
		case CART_NONE:
			return UI_TEXT("无");
		case CART_BACKUP_MEM:
			return UI_TEXT("512K 备份 RAM");
		case CART_EXTRAM_1M:
			return UI_TEXT("1M RAM");
		case CART_EXTRAM_4M:
			return UI_TEXT("4M RAM");
		case CART_CS1RAM_16M:
			return UI_TEXT("16M CS1 RAM");
		case CART_KOF95:
			return UI_TEXT("格斗之王 '95");
		case CART_ULTRAMAN:
			return UI_TEXT("奥特曼");
	}
	return "";
}

constexpr auto regionToString(int t)
{
	switch(t)
	{
		case SMPC_AREA_JP:
			return UI_TEXT("日本");
		case SMPC_AREA_NA:
			return UI_TEXT("北美");
		case SMPC_AREA_EU_PAL:
			return UI_TEXT("欧洲");
		case SMPC_AREA_KR:
			return UI_TEXT("韩国");
		case SMPC_AREA_ASIA_NTSC:
			return UI_TEXT("亚洲 (NTSC)");
		case SMPC_AREA_ASIA_PAL:
			return UI_TEXT("亚洲 (PAL)");
		case SMPC_AREA_CSA_NTSC:
			return UI_TEXT("巴西");
		case SMPC_AREA_CSA_PAL:
			return UI_TEXT("拉丁美洲");
	}
	return "";
}

class ConsoleOptionView : public TableView, public MainAppHelper
{
	TextMenuItem cartTypeItems[8]
	{
		{
			UI_TEXT("自动"),
			attachParams(), {.id = CART__RESERVED}
		},
		{cartTypeToString(CART_NONE),       attachParams(), {.id = CART_NONE}},
		{cartTypeToString(CART_BACKUP_MEM), attachParams(), {.id = CART_BACKUP_MEM}},
		{cartTypeToString(CART_EXTRAM_1M),  attachParams(), {.id = CART_EXTRAM_1M}},
		{cartTypeToString(CART_EXTRAM_4M),  attachParams(), {.id = CART_EXTRAM_4M}},
		{cartTypeToString(CART_CS1RAM_16M), attachParams(), {.id = CART_CS1RAM_16M}},
		{cartTypeToString(CART_KOF95),      attachParams(), {.id = CART_KOF95}},
		{cartTypeToString(CART_ULTRAMAN),   attachParams(), {.id = CART_ULTRAMAN}},
	};

	MultiChoiceMenuItem cartType
	{
		UI_TEXT("卡带类型"),
		attachParams(),
		MenuId{system().cartType},
		cartTypeItems,
		{
			.onSetDisplayString = [](auto idx, Gfx::Text &t)
			{
				if(idx == 0)
				{
					t.resetString(cartTypeToString(ActiveCartType));
					return true;
				}
				return false;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item, const Input::Event &e)
			{
				system().sessionOptionSet();
				system().cartType = item.id;
				app().promptSystemReloadDueToSetOption(attachParams(), e);
			}
		}
	};

	TextMenuItem regionItems[9]
	{
		{
			UI_TEXT("自动"),
			attachParams(), {.id = 0}
		},
		{regionToString(SMPC_AREA_JP),        attachParams(), {.id = SMPC_AREA_JP}},
		{regionToString(SMPC_AREA_NA),        attachParams(), {.id = SMPC_AREA_NA}},
		{regionToString(SMPC_AREA_EU_PAL),    attachParams(), {.id = SMPC_AREA_EU_PAL}},
		{regionToString(SMPC_AREA_KR),        attachParams(), {.id = SMPC_AREA_KR}},
		{regionToString(SMPC_AREA_ASIA_NTSC), attachParams(), {.id = SMPC_AREA_ASIA_NTSC}},
		{regionToString(SMPC_AREA_ASIA_PAL),  attachParams(), {.id = SMPC_AREA_ASIA_PAL}},
		{regionToString(SMPC_AREA_CSA_NTSC),  attachParams(), {.id = SMPC_AREA_CSA_NTSC}},
		{regionToString(SMPC_AREA_CSA_PAL),   attachParams(), {.id = SMPC_AREA_CSA_PAL}},
	};

	MultiChoiceMenuItem region
	{
		UI_TEXT("地区"),
		attachParams(),
		MenuId{system().region},
		regionItems,
		{
			.onSetDisplayString = [](auto idx, Gfx::Text &t)
			{
				if(idx == 0)
				{
					t.resetString(regionToString(AreaCode));
					return true;
				}
				return false;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item, const Input::Event &e)
			{
				system().sessionOptionSet();
				system().region = item.id;
				app().promptSystemReloadDueToSetOption(attachParams(), e);
			}
		}
	};

	DynArray<TextMenuItem> discItems;

	MultiChoiceMenuItem disc
	{
		UI_TEXT("光盘"),
		attachParams(),
		MenuId{system().currentDiscId()},
		discItems
	};

	TextMenuItem::SelectDelegate setDiscDel()
	{
		return [this](TextMenuItem &item, const Input::Event &e)
		{
			system().setDisc(item.id);
		};
	}

	TextHeadingMenuItem videoHeading
	{
		UI_TEXT("视频："),
		attachParams()
	};

	BoolMenuItem showHOverscan
	{
		UI_TEXT("显示过度扫描裁剪掉的图像"),
		attachParams(),
		system().showHOverscan,
		[this](BoolMenuItem &item)
		{
			system().sessionOptionSet();
			system().setShowHOverscan(item.flipBoolValue(*this));
		}
	};

	TextMenuItem visibleNtscVideoLinesItem[2]
	{
		{
			UI_TEXT("8+224"),
			attachParams(), {.id = std::bit_cast<MenuId>(VideoLineRange{8, 231})}
		},
		{
			UI_TEXT("0+240"),
			attachParams(), {.id = std::bit_cast<MenuId>(VideoLineRange{0, 239})}
		},
	};

	TextMenuItem visiblePalVideoLinesItem[3]
	{
		{
			UI_TEXT("16+256"),
			attachParams(), {.id = std::bit_cast<MenuId>(VideoLineRange{16, 271})}
		},
		{
			UI_TEXT("32+224"),
			attachParams(), {.id = std::bit_cast<MenuId>(VideoLineRange{32, 255})}
		},
		{
			UI_TEXT("0+288"),
			attachParams(), {.id = std::bit_cast<MenuId>(VideoLineRange{0, 287})}
		},
	};

	MultiChoiceMenuItem visibleVideoLines
	{
		UI_TEXT("可见行数"),
		attachParams(),
		std::bit_cast<MenuId>(system().videoLines),
		[&]() -> std::span<TextMenuItem>
		{
			if(MDFN_IEN_SS::VDP2::PAL)
				return visiblePalVideoLinesItem;
			else
				return visibleNtscVideoLinesItem;
		}(),
		{
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				system().sessionOptionSet();
				system().setVideoLines(std::bit_cast<VideoLineRange>(item.id));
			}
		}
	};

	TextMenuItem deinterlaceModeItems[2]
	{
		{
			UI_TEXT("Bob 算法"),
			attachParams(), {.id = DeinterlaceMode::Bob}
		},
		{
			UI_TEXT("Weave 算法"),
			attachParams(), {.id = DeinterlaceMode::Weave}
		},
	};

	MultiChoiceMenuItem deinterlaceMode
	{
		UI_TEXT("反交错算法"),
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

	TextMenuItem contentRotationItems[5]
	{
		{
			UI_TEXT("自动"),
			attachParams(), {.id = Rotation::ANY}
		},
		{
			UI_TEXT("标准"),
			attachParams(), {.id = Rotation::UP}
		},
		{
			UI_TEXT("右转 90 度"),
			attachParams(), {.id = Rotation::RIGHT}
		},
		{
			UI_TEXT("旋转 180 度"),
			attachParams(), {.id = Rotation::DOWN}
		},
		{
			UI_TEXT("左转 90 度"),
			attachParams(), {.id = Rotation::LEFT}
		},
	};

	MultiChoiceMenuItem contentRotation
	{
		UI_TEXT("画面旋转"),
		attachParams(),
		MenuId{system().sysContentRotation},
		contentRotationItems,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				system().sessionOptionSet();
				system().sysContentRotation = Rotation(item.id.val);
				app().updateContentRotation();
			}
		}
	};

	TextMenuItem widescreenModeItems[3]
	{
		{
			UI_TEXT("自动"),
			attachParams(), {.id = WidescreenMode::Auto}
		},
		{
			UI_TEXT("开"),
			attachParams(), {.id = WidescreenMode::On}
		},
		{
			UI_TEXT("关"),
			attachParams(), {.id = WidescreenMode::Off}
		},
	};

	MultiChoiceMenuItem widescreenMode
	{
		UI_TEXT("宽屏模式"),
		attachParams(),
		MenuId{system().widescreenMode},
		widescreenModeItems,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				system().sessionOptionSet();
				system().widescreenMode = WidescreenMode(item.id.val);
				app().viewController().placeEmuViews();
			}
		}
	};

	TextHeadingMenuItem inputHeading
	{
		UI_TEXT("输入："),
		attachParams()
	};

	BoolMenuItem multitapItem(int idx)
	{
		return
		{
			std::format(
				UI_TEXT("端口 {} 扩展模式"),
				idx + 1),
			attachParams(),
			system().inputConfig.multitaps[idx],
			[this, idx](BoolMenuItem &item)
			{
				system().sessionOptionSet();
				system().inputConfig.multitaps[idx] = item.flipBoolValue(*this);
				system().applyInputConfig(app());
				updateActiveDevices();
			}
		};
	}

	BoolMenuItem multitaps[2]{multitapItem(0), multitapItem(1)};

	size_t inputDeviceIdx{};

	TextMenuItem inputDeviceItems[3]
	{
		{
			UI_TEXT("手柄"),
			attachParams(), setInputPortsDel(), {.id = InputDeviceType::gamepad}
		},
		{
			UI_TEXT("光枪"),
			attachParams(), setInputPortsDel(), {.id = InputDeviceType::gun}
		},
		{
			UI_TEXT("无"),
			attachParams(), setInputPortsDel(), {.id = InputDeviceType::none}
		},
	};

	MultiChoiceMenuItem inputDeviceItem(int idx)
	{
		return
		{
			std::format(
				UI_TEXT("端口 {}"),
				idx + 1),
			attachParams(),
			MenuId{system().inputConfig.devs[idx]},
			inputDeviceItems,
			{
				.onSelect = [this, idx](MultiChoiceMenuItem &item, View &view, const Input::Event &e)
				{
					inputDeviceIdx = idx;
					item.defaultOnSelect(view, e);
				}
			}
		};
	}

	MultiChoiceMenuItem inputDevices[12]
	{
		inputDeviceItem(0), inputDeviceItem(1),  inputDeviceItem(2),
		inputDeviceItem(3), inputDeviceItem(4),  inputDeviceItem(5),
		inputDeviceItem(6), inputDeviceItem(7),  inputDeviceItem(8),
		inputDeviceItem(9), inputDeviceItem(10), inputDeviceItem(11),
	};

	TextMenuItem::SelectDelegate setInputPortsDel()
	{
		return [this](TextMenuItem &item)
		{
			system().sessionOptionSet();
			system().inputConfig.devs[inputDeviceIdx] = InputDeviceType(item.id.val);
			system().applyInputConfig(app());
			updateActiveDevices();
		};
	}

	std::vector<MenuItem*> menuItems;

	void updateActiveDevices()
	{
		for(auto &i : inputDevices)
		{
			i.setActive(true);
		}
		if(!system().inputConfig.multitaps[0] && !system().inputConfig.multitaps[1]) // no multitaps
		{
			for(auto &i : inputDevices | std::views::drop(2) | std::views::take(10))
			{
				i.setActive(false);
			}
		}
		else if(system().inputConfig.multitaps[0] != system().inputConfig.multitaps[1]) // 1 multitap
		{
			for(auto &i : inputDevices | std::views::drop(7))
			{
				i.setActive(false);
			}
		}
	}

public:
	ConsoleOptionView(ViewAttachParams attach):
		TableView
		{
			UI_TEXT("主机选项"),
			attach,
			menuItems
		},
		discItems
		{
			[&](auto &system)
			{
				auto discItems = DynArray<TextMenuItem>{system.CDInterfaces.size() + 1};
				discItems[0] =
					{
						UI_TEXT("弹出"),
						attachParams(), setDiscDel(), {.id = -1}
					};
				const char *numStrings[] {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14" , "15", "16"};
				for(auto i : iotaCount(system.CDInterfaces.size()))
				{
					discItems[i + 1] = {numStrings[i], attachParams(), setDiscDel(), {.id = i}};
				}
				return discItems;
			}(system())
		}
	{
		menuItems.emplace_back(&cartType);
		menuItems.emplace_back(&region);
		menuItems.emplace_back(&disc);
		menuItems.emplace_back(&videoHeading);
		menuItems.emplace_back(&showHOverscan);
		menuItems.emplace_back(&visibleVideoLines);
		menuItems.emplace_back(&deinterlaceMode);
		menuItems.emplace_back(&contentRotation);
		menuItems.emplace_back(&widescreenMode);
		menuItems.emplace_back(&inputHeading);
		menuItems.emplace_back(&multitaps[0]);
		menuItems.emplace_back(&multitaps[1]);
		for(auto &i : inputDevices)
		{
			menuItems.emplace_back(&i);
		}
		updateActiveDevices();
	}
};

class CustomSystemActionsView : public SystemActionsView
{
private:
	TextMenuItem options
	{
		UI_TEXT("主机选项"),
		attachParams(),
		[this](Input::Event e) { pushAndShow(makeView<ConsoleOptionView>(), e); }
	};

public:
	CustomSystemActionsView(ViewAttachParams attach): SystemActionsView{attach, true}
	{
		item.emplace_back(&options);
		loadStandardItems();
	}
};

class CustomSystemOptionView : public SystemOptionView, public MainAppHelper
{
	using MainAppHelper::system;

	BoolMenuItem autoSetRTC
	{
		UI_TEXT("启动时自动设置实时时钟"),
		attachParams(),
		system().autoRTCTime,
		[this](BoolMenuItem &item)
		{
			system().autoRTCTime = item.flipBoolValue(*this);
		}
	};

	TextMenuItem biosLanguageItems[6]
	{
		{
			UI_TEXT("英语"),
			attachParams(), {.id = SMPC_RTC_LANG_ENGLISH}
		},
		{
			UI_TEXT("德语"),
			attachParams(), {.id = SMPC_RTC_LANG_GERMAN}
		},
		{
			UI_TEXT("法语"),
			attachParams(), {.id = SMPC_RTC_LANG_FRENCH}
		},
		{
			UI_TEXT("西班牙语"),
			attachParams(), {.id = SMPC_RTC_LANG_SPANISH}
		},
		{
			UI_TEXT("意大利语"),
			attachParams(), {.id = SMPC_RTC_LANG_ITALIAN}
		},
		{
			UI_TEXT("日语"),
			attachParams(), {.id = SMPC_RTC_LANG_JAPANESE}
		},
	};

	MultiChoiceMenuItem biosLanguage
	{
		UI_TEXT("BIOS 语言"),
		attachParams(),
		MenuId{system().biosLanguage},
		biosLanguageItems,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				system().biosLanguage = item.id;
			}
		}
	};

	BoolMenuItem saveFilenameType = saveFilenameTypeMenuItem(*this, system());

public:
	CustomSystemOptionView(ViewAttachParams attach): SystemOptionView{attach, true}
	{
		loadStockItems();
		item.emplace_back(&biosLanguage);
		item.emplace_back(&autoSetRTC);
		item.emplace_back(&saveFilenameType);
	}
};

class CustomVideoOptionView : public VideoOptionView, public MainAppHelper
{
	using  MainAppHelper::app;
	using  MainAppHelper::system;

	BoolMenuItem showHOverscan
	{
		UI_TEXT("默认显示过度扫描裁剪掉的图像"),
		attachParams(),
		system().defaultShowHOverscan,
		[this](BoolMenuItem &item)
		{
			system().defaultShowHOverscan = item.flipBoolValue(*this);
		}
	};

	TextMenuItem visibleVideoLinesItem[2]
	{
		{
			UI_TEXT("8+224"),
			attachParams(), {.id = std::bit_cast<MenuId>(VideoLineRange{8, 231})}
		},
		{
			UI_TEXT("0+240"),
			attachParams(), {.id = std::bit_cast<MenuId>(VideoLineRange{0, 239})}
		},
	};

	MultiChoiceMenuItem visibleVideoLines
	{
		UI_TEXT("NTSC 制式默认可见行数"),
		attachParams(),
		std::bit_cast<MenuId>(system().defaultNtscLines),
		visibleVideoLinesItem,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				system().defaultNtscLines = std::bit_cast<VideoLineRange>(item.id.val);
			}
		}
	};

	BoolMenuItem correctLineAspect
	{
		UI_TEXT("修正每行宽高比例"),
		attachParams(),
		system().correctLineAspect,
		[this](BoolMenuItem &item)
		{
			system().correctLineAspect = item.flipBoolValue(*this);
			app().viewController().placeEmuViews();
		}
	};

public:
	CustomVideoOptionView(ViewAttachParams attach, EmuVideoLayer &layer): VideoOptionView{attach, layer, true}
	{
		loadStockItems();
		item.emplace_back(&systemSpecificHeading);
		item.emplace_back(&showHOverscan);
		item.emplace_back(&visibleVideoLines);
		item.emplace_back(&correctLineAspect);
	}
};

std::unique_ptr<View> EmuApp::makeCustomView(ViewAttachParams attach, ViewID id)
{
	switch(id)
	{
		case ViewID::FILE_PATH_OPTIONS: return std::make_unique<CustomFilePathOptionView>(attach);
		case ViewID::SYSTEM_OPTIONS: return std::make_unique<CustomSystemOptionView>(attach);
		case ViewID::SYSTEM_ACTIONS: return std::make_unique<CustomSystemActionsView>(attach);
		case ViewID::VIDEO_OPTIONS: return std::make_unique<CustomVideoOptionView>(attach, videoLayer);
		default: return nullptr;
	}
}

}
