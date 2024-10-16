/*  This file is part of MD.emu.

	MD.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	MD.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with MD.emu.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/SystemOptionView.hh>
#include <emuframework/AudioOptionView.hh>
#include <emuframework/FilePathOptionView.hh>
#include <emuframework/DataPathSelectView.hh>
#include <emuframework/UserPathSelectView.hh>
#include <emuframework/SystemActionsView.hh>
#include "MainApp.hh"
#include <imagine/gui/AlertView.hh>
#include <imagine/fs/FS.hh>
#include <imagine/util/format.hh>
#include <imagine/util/string.h>
#include "input.h"
#include "io_ctrl.h"
#include "vdp_ctrl.h"

namespace EmuEx
{

using MainAppHelper = EmuAppHelperBase<MainApp>;

class ConsoleOptionView : public TableView, public MainAppHelper
{
	BoolMenuItem sixButtonPad
	{
		UI_TEXT("六键手柄"),
		attachParams(),
		(bool)system().option6BtnPad,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			system().sessionOptionSet();
			system().option6BtnPad = item.flipBoolValue(*this);
			system().setupInput(app());
		}
	};

	BoolMenuItem multitap
	{
		UI_TEXT("四人玩家适配器"),
		attachParams(),
		(bool)system().optionMultiTap,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			system().optionMultiTap = item.flipBoolValue(*this);
			system().setupInput(app());
		}
	};

	constexpr const char *inputSystemName(int system)
	{
		switch(system)
		{
			case SYSTEM_MENACER:
				return UI_TEXT("Menacer 光枪");
			case SYSTEM_JUSTIFIER:
				return UI_TEXT("Justifier 光枪");
		}
		return UI_TEXT("手柄");
	}

	TextMenuItem inputPortsItem[4]
	{
		{
			UI_TEXT("自动"),
			attachParams(), setInputPortsDel(-1, -1), {.id = -1}
		},
		{
			UI_TEXT("手柄"),
			attachParams(), setInputPortsDel(SYSTEM_MD_GAMEPAD, SYSTEM_MD_GAMEPAD), {.id = SYSTEM_MD_GAMEPAD}
		},
		{
			UI_TEXT("Menacer 光枪"),
			attachParams(), setInputPortsDel(SYSTEM_MD_GAMEPAD, SYSTEM_MENACER),    {.id = SYSTEM_MENACER}
		},
		{
			UI_TEXT("Justifier 光枪"),
			attachParams(), setInputPortsDel(SYSTEM_MD_GAMEPAD, SYSTEM_JUSTIFIER),  {.id = SYSTEM_JUSTIFIER}
		},
	};

	MultiChoiceMenuItem inputPorts
	{
		UI_TEXT("输入端口"),
		attachParams(),
		MenuId{system().mdInputPortDev[1]},
		inputPortsItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(inputSystemName(input.system[1]));
				return true;
			}
		},
	};

	TextMenuItem::SelectDelegate setInputPortsDel(int8_t port1, int8_t port2)
	{
		return [this, port1, port2]()
		{
			system().sessionOptionSet();
			system().optionInputPort1 = system().mdInputPortDev[0] = port1;
			system().optionInputPort2 = system().mdInputPortDev[1] = port2;
			system().setupInput(app());
		};
	}

	TextMenuItem videoSystemItem[3]
	{
		{
			UI_TEXT("自动"),
			attachParams(), [this](Input::Event e){ setVideoSystem(0, e); }
		},
		{
			UI_TEXT("NTSC"),
			attachParams(), [this](Input::Event e){ setVideoSystem(1, e); }
		},
		{
			UI_TEXT("PAL"),
			attachParams(),  [this](Input::Event e){ setVideoSystem(2, e); }
		},
	};

	MultiChoiceMenuItem videoSystem
	{
		UI_TEXT("视频制式"),
		attachParams(),
		system().optionVideoSystem.value(),
		videoSystemItem,
		{
			.onSetDisplayString = [](auto idx, Gfx::Text& t)
			{
				if(idx == 0)
				{
					t.resetString(vdp_pal
						? UI_TEXT("PAL")
						: UI_TEXT("NTSC")
					);
					return true;
				}
				return false;
			}
		},
	};

	void setVideoSystem(int val, Input::Event e)
	{
		system().sessionOptionSet();
		system().optionVideoSystem = val;
		app().promptSystemReloadDueToSetOption(attachParams(), e);
	}

	TextMenuItem regionItem[4]
	{
		{
			UI_TEXT("自动"),
			attachParams(), [this](Input::Event e){ setRegion(0, e); }
		},
		{
			UI_TEXT("美国"),
			attachParams(), [this](Input::Event e){ setRegion(1, e); }
		},
		{
			UI_TEXT("欧洲"),
			attachParams(), [this](Input::Event e){ setRegion(2, e); }
		},
		{
			UI_TEXT("日本"),
			attachParams(), [this](Input::Event e){ setRegion(3, e); }
		},
	};

	MultiChoiceMenuItem region
	{
		UI_TEXT("游戏地区"),
		attachParams(),
		std::min((int)config.region_detect, 4),
		regionItem,
		{
			.onSetDisplayString = [](auto idx, Gfx::Text& t)
			{
				if(idx == 0)
				{
					auto regionStr = [](unsigned region)
					{
						switch(region)
						{
							case REGION_USA:
								return UI_TEXT("美国");
							case REGION_EUROPE:
								return UI_TEXT("欧洲");
							default:
								return UI_TEXT("日本");
						}
					};
					t.resetString(regionStr(region_code));
					return true;
				}
				return false;
			}
		}
	};

	void setRegion(int val, Input::Event e)
	{
		system().sessionOptionSet();
		system().optionRegion = val;
		app().promptSystemReloadDueToSetOption(attachParams(), e);
	}

	StaticArrayList<MenuItem*, 5> item;

public:
	ConsoleOptionView(ViewAttachParams attach):
		TableView
		{
			UI_TEXT("主机选项"),
			attach,
			item
		}
	{
		if(system_hw != SYSTEM_PBC)
		{
			item.emplace_back(&inputPorts);
			item.emplace_back(&sixButtonPad);
			item.emplace_back(&multitap);
		}
		item.emplace_back(&videoSystem);
		item.emplace_back(&region);
	}
};

class CustomSystemActionsView : public SystemActionsView
{
private:
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

	BoolMenuItem smsFM
	{
		UI_TEXT("MarkIII 调频声音单元"),
		attachParams(),
		(bool)system().optionSmsFM,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			system().optionSmsFM = item.flipBoolValue(*this);
			config_ym2413_enabled = system().optionSmsFM;
		}
	};

public:
	CustomAudioOptionView(ViewAttachParams attach, EmuAudio& audio): AudioOptionView{attach, audio, true}
	{
		loadStockItems();
		item.emplace_back(&smsFM);
	}
};

class CustomSystemOptionView : public SystemOptionView, public MainAppHelper
{
	using MainAppHelper::app;
	using MainAppHelper::system;

	BoolMenuItem bigEndianSram
	{
		UI_TEXT("使用大端字节序 SRAM"),
		attachParams(),
		(bool)system().optionBigEndianSram,
		[this](BoolMenuItem &item, Input::Event e)
		{
			app().pushAndShowModalView(makeView<YesNoAlertView>(
				UI_TEXT("警告，此选项会改变 SRAM 存档文件的格式。 ")
				UI_TEXT("开启后可以使存档文件兼容 Gens 等其它模拟器。 ")
				UI_TEXT("错误的设置将会导致 SRAM 在读取存档文件时出现数据损坏。"),
				YesNoAlertView::Delegates{.onYes = [this]{ system().optionBigEndianSram = bigEndianSram.flipBoolValue(*this); }}), e);
		}
	};

public:
	CustomSystemOptionView(ViewAttachParams attach): SystemOptionView{attach, true}
	{
		loadStockItems();
		item.emplace_back(&bigEndianSram);
	}
};

class CustomFilePathOptionView : public FilePathOptionView, public MainAppHelper
{
	using MainAppHelper::app;
	using MainAppHelper::system;

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

	#ifndef NO_SCD
	static constexpr std::string_view biosHeadingStr[3]
	{
		UI_TEXT("美国区的 CD BIOS"),
		UI_TEXT("日本区的 CD BIOS"),
		UI_TEXT("欧洲区的 CD BIOS")
	};

	static int8_t regionCodeToIdx(uint8_t region)
	{
		switch(region)
		{
			default: return 0;
			case REGION_JAPAN_NTSC: return 1;
			case REGION_EUROPE: return 2;
		}
	}

	FS::PathString &pathFromRegion(uint8_t region)
	{
		switch(region)
		{
			default: return system().cdBiosUSAPath;
			case REGION_JAPAN_NTSC: return system().cdBiosJpnPath;
			case REGION_EUROPE: return system().cdBiosEurPath;
		}
	}

	TextMenuItem cdBiosPath[3]
	{
		{biosMenuEntryStr(REGION_USA, pathFromRegion(REGION_USA)),               attachParams(), setCDBiosPathDel(REGION_USA)},
		{biosMenuEntryStr(REGION_JAPAN_NTSC, pathFromRegion(REGION_JAPAN_NTSC)), attachParams(), setCDBiosPathDel(REGION_JAPAN_NTSC)},
		{biosMenuEntryStr(REGION_EUROPE, pathFromRegion(REGION_EUROPE)),         attachParams(), setCDBiosPathDel(REGION_EUROPE)}
	};

	std::string biosMenuEntryStr(uint8_t region, IG::CStringView path) const
	{
		auto regionStr = biosHeadingStr[regionCodeToIdx(region)];
		return std::format(
			UI_TEXT("{}：{}"),
			regionStr, appContext().fileUriDisplayName(path));
	}

	TextMenuItem::SelectDelegate setCDBiosPathDel(uint8_t region)
	{
		return [this, region](const Input::Event &e)
		{
			pushAndShow(makeViewWithName<DataFileSelectView<>>(biosHeadingStr[regionCodeToIdx(region)],
				app().validSearchPath(pathFromRegion(region)),
				[this, region](CStringView path, FS::file_type type)
				{
					auto idx = regionCodeToIdx(region);
					pathFromRegion(region) = path;
					logMsg("set bios:%d to path:%s", idx, pathFromRegion(region).data());
					cdBiosPath[idx].compile(biosMenuEntryStr(region, path));
					return true;
				}, hasMDExtension), e);
		};
	}
	#endif

	public:
	CustomFilePathOptionView(ViewAttachParams attach): FilePathOptionView{attach, true}
	{
		loadStockItems();
		item.emplace_back(&cheatsPath);
		#ifndef NO_SCD
		for(auto i : iotaCount(3))
		{
			item.emplace_back(&cdBiosPath[i]);
		}
		#endif
	}
};

std::unique_ptr<View> EmuApp::makeCustomView(ViewAttachParams attach, ViewID id)
{
	switch(id)
	{
		case ViewID::AUDIO_OPTIONS: return std::make_unique<CustomAudioOptionView>(attach, audio);
		case ViewID::SYSTEM_ACTIONS: return std::make_unique<CustomSystemActionsView>(attach);
		case ViewID::SYSTEM_OPTIONS: return std::make_unique<CustomSystemOptionView>(attach);
		case ViewID::FILE_PATH_OPTIONS: return std::make_unique<CustomFilePathOptionView>(attach);
		default: return nullptr;
	}
}

}
