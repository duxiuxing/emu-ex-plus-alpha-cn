/*  This file is part of PCE.emu.

	PCE.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	PCE.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with PCE.emu.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/EmuViewController.hh>
#include <emuframework/SystemOptionView.hh>
#include <emuframework/AudioOptionView.hh>
#include <emuframework/VideoOptionView.hh>
#include <emuframework/FilePathOptionView.hh>
#include <emuframework/DataPathSelectView.hh>
#include <emuframework/SystemActionsView.hh>
#include <emuframework/EmuInput.hh>
#include <emuframework/viewUtils.hh>
#include <mednafen-emuex/MDFNUtils.hh>
#include "MainApp.hh"
#include <imagine/fs/FS.hh>
#include <imagine/gui/AlertView.hh>
#include <imagine/util/format.hh>
#include <imagine/logger/logger.h>

namespace EmuEx
{

using MainAppHelper = EmuAppHelperBase<MainApp>;

constexpr std::string_view pceFastText
{
	UI_TEXT("pce_fast (推荐使用的默认值)")
};
constexpr std::string_view pceText
{
	UI_TEXT("pce (精度更好，能耗更高)")
};
constexpr std::string_view changeEmuCoreText
{
	UI_TEXT("是否要更改模拟核心？注意，不同核心的存档互不兼容。")
};

class ConsoleOptionView : public TableView, public MainAppHelper
{
	BoolMenuItem sixButtonPad
	{
		UI_TEXT("六键手柄"),
		attachParams(),
		(bool)system().option6BtnPad,
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			system().sessionOptionSet();
			system().option6BtnPad = item.flipBoolValue(*this);
			set6ButtonPadEnabled(app(), system().option6BtnPad);
		}
	};

	BoolMenuItem arcadeCard
	{
		UI_TEXT("街机卡模式"),
		attachParams(),
		(bool)system().optionArcadeCard,
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			system().sessionOptionSet();
			system().optionArcadeCard = item.flipBoolValue(*this);
			app().promptSystemReloadDueToSetOption(attachParams(), e);
		}
	};

	TextHeadingMenuItem videoHeading
	{
		UI_TEXT("视频："),
		attachParams()
	
	};

	TextMenuItem visibleVideoLinesItem[5]
	{
		{
			UI_TEXT("11+224"),
			attachParams(), setVisibleVideoLinesDel(11, 234)
		},
		{
			UI_TEXT("18+224"),
			attachParams(), setVisibleVideoLinesDel(18, 241)
		},
		{
			UI_TEXT("4+232"),
			attachParams(), setVisibleVideoLinesDel(4, 235)
		},
		{
			UI_TEXT("3+239"),
			attachParams(), setVisibleVideoLinesDel(3, 241)
		},
		{
			UI_TEXT("0+242"),
			attachParams(), setVisibleVideoLinesDel(0, 241)
		},
	};

	MultiChoiceMenuItem visibleVideoLines
	{
		UI_TEXT("可见行数"),
		attachParams(),
		[this]()
		{
			switch(system().visibleLines.first)
			{
				default: return 0;
				case 18: return 1;
				case 4: return 2;
				case 3: return 3;
				case 0: return 4;
			}
		}(),
		visibleVideoLinesItem
	};

	TextMenuItem::SelectDelegate setVisibleVideoLinesDel(uint8_t startLine, uint8_t endLine)
	{
		return [=, this]() { system().setVisibleLines({startLine, endLine}); };
	}

	TextMenuItem emuCoreItems[3]
	{
		{
			UI_TEXT("自动"),
			attachParams(), setEmuCoreDel(), {.id = EmuCore::Auto}
		},
		{pceFastText, attachParams(), setEmuCoreDel(), {.id = EmuCore::Fast}},
		{pceText,     attachParams(), setEmuCoreDel(), {.id = EmuCore::Accurate}},
	};

	MultiChoiceMenuItem emuCore
	{
		UI_TEXT("模拟器核心"),
		attachParams(),
		MenuId{system().core},
		emuCoreItems,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(asModuleString(system().resolvedCore()));
				return true;
			}
		},
	};

	TextMenuItem::SelectDelegate setEmuCoreDel()
	{
		return [this](TextMenuItem &item, const Input::Event &e)
		{
			auto c = EmuCore(item.id.val);
			if(c == system().core)
				return true;
			pushAndShowModal(makeView<YesNoAlertView>(changeEmuCoreText,
				YesNoAlertView::Delegates
				{
					.onYes = [this, c](const Input::Event &e)
					{
						system().sessionOptionSet();
						system().core = c;
						emuCore.setSelected(MenuId{c});
						dismissPrevious();
						app().promptSystemReloadDueToSetOption(attachParams(), e);
					}
				}), e, false);
			return false;
		};
	}

	std::array<MenuItem*, 5> menuItem
	{
		&sixButtonPad,
		&arcadeCard,
		&emuCore,
		&videoHeading,
		&visibleVideoLines,
	};

public:
	ConsoleOptionView(ViewAttachParams attach):
		TableView
		{
			UI_TEXT("主机选项"),
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

class CustomFilePathOptionView : public FilePathOptionView, public MainAppHelper
{
	using MainAppHelper::app;
	using MainAppHelper::system;

	TextMenuItem sysCardPath
	{
		biosMenuEntryStr(system().sysCardPath), attachParams(),
		[this](Input::Event e)
		{
			pushAndShow(makeViewWithName<DataFileSelectView<>>(
				UI_TEXT("系统卡文件"),
				app().validSearchPath(FS::dirnameUri(system().sysCardPath)),
				[this](CStringView path, FS::file_type type)
				{
					system().sysCardPath = path;
					logMsg("set system card:%s", system().sysCardPath.data());
					sysCardPath.compile(biosMenuEntryStr(path));
					return true;
				}, hasHuCardExtension), e);
		}
	};

	std::string biosMenuEntryStr(std::string_view path) const
	{
		return std::format(
			UI_TEXT("系统卡文件：{}"),
			appContext().fileUriDisplayName(path));
	}

public:
	CustomFilePathOptionView(ViewAttachParams attach): FilePathOptionView{attach, true}
	{
		loadStockItems();
		item.emplace_back(&sysCardPath);
	}
};

class CustomVideoOptionView : public VideoOptionView, public MainAppHelper
{
	using  MainAppHelper::app;
	using  MainAppHelper::system;

	BoolMenuItem spriteLimit
	{
		UI_TEXT("精灵限制"),
		attachParams(),
		!system().noSpriteLimit,
		[this](BoolMenuItem &item) { system().setNoSpriteLimit(!item.flipBoolValue(*this)); }
	};

	TextMenuItem visibleVideoLinesItem[5]
	{
		{
			UI_TEXT("11+224"),
			attachParams(), setVisibleVideoLinesDel(11, 234)
		},
		{
			UI_TEXT("18+224"),
			attachParams(), setVisibleVideoLinesDel(18, 241)
		},
		{
			UI_TEXT("4+232"),
			attachParams(), setVisibleVideoLinesDel(4, 235)
		},
		{
			UI_TEXT("3+239"),
			attachParams(), setVisibleVideoLinesDel(3, 241)
		},
		{
			UI_TEXT("0+242"),
			attachParams(), setVisibleVideoLinesDel(0, 241)
		},
	};

	MultiChoiceMenuItem visibleVideoLines
	{
		UI_TEXT("默认可见行数"),
		attachParams(),
		[this]()
		{
			switch(system().defaultVisibleLines.first)
			{
				default: return 0;
				case 18: return 1;
				case 4: return 2;
				case 3: return 3;
				case 0: return 4;
			}
		}(),
		visibleVideoLinesItem
	};

	TextMenuItem::SelectDelegate setVisibleVideoLinesDel(uint8_t startLine, uint8_t endLine)
	{
		return [=, this]() { system().defaultVisibleLines = {startLine, endLine}; };
	}

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
		item.emplace_back(&spriteLimit);
		item.emplace_back(&visibleVideoLines);
		item.emplace_back(&correctLineAspect);
	}
};

class CustomSystemOptionView : public SystemOptionView, public MainAppHelper
{
	using MainAppHelper::system;

	TextMenuItem cdSpeedItem[5]
	{
		{
			UI_TEXT("1x"),
			attachParams(), setCdSpeedDel(), {.id = 1}
		},
		{			
			UI_TEXT("2x"),
			attachParams(), setCdSpeedDel(), {.id = 2}
		},
		{
			UI_TEXT("4x"),
			attachParams(), setCdSpeedDel(), {.id = 4}
		},
		{
			UI_TEXT("8x"),
			attachParams(), setCdSpeedDel(), {.id = 8}
		},
	};

	MultiChoiceMenuItem cdSpeed
	{
		UI_TEXT("光盘读取倍速"),
		attachParams(),
		MenuId{system().cdSpeed},
		cdSpeedItem
	};

	TextMenuItem::SelectDelegate setCdSpeedDel()
	{
		return [this](TextMenuItem &item) { system().setCdSpeed(item.id); };
	}

	TextMenuItem emuCoreItems[3]
	{
		{
			UI_TEXT("自动"),
			attachParams(), setEmuCoreDel(), {.id = EmuCore::Auto}
		},
		{pceFastText, attachParams(), setEmuCoreDel(), {.id = EmuCore::Fast}},
		{pceText,     attachParams(), setEmuCoreDel(), {.id = EmuCore::Accurate}},
	};

	MultiChoiceMenuItem emuCore
	{
		UI_TEXT("模拟器核心"),
		attachParams(),
		MenuId{system().defaultCore},
		emuCoreItems,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(asModuleString(system().resolvedDefaultCore()));
				return true;
			}
		},
	};

	TextMenuItem::SelectDelegate setEmuCoreDel()
	{
		return [this](TextMenuItem &item, const Input::Event &e)
		{
			auto c = EmuCore(item.id.val);
			if(c == system().defaultCore)
				return true;
			pushAndShowModal(makeView<YesNoAlertView>(changeEmuCoreText,
				YesNoAlertView::Delegates
				{
					.onYes = [this, c]
					{
						system().defaultCore = c;
						emuCore.setSelected(MenuId{c});
						dismissPrevious();
					}
				}), e, false);
			return false;
		};
	}

	BoolMenuItem saveFilenameType = saveFilenameTypeMenuItem(*this, system());

public:
	CustomSystemOptionView(ViewAttachParams attach): SystemOptionView{attach, true}
	{
		loadStockItems();
		item.emplace_back(&emuCore);
		item.emplace_back(&cdSpeed);
		item.emplace_back(&saveFilenameType);
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

	struct VolumeTypeDesc
	{
		std::string_view name{};
		size_t idx{};
	};

	static constexpr VolumeTypeDesc desc(VolumeType type)
	{
		switch(type)
		{
			case VolumeType::CDDA:
				return
					{
						UI_TEXT("CD-DA 音量"),
						0
					};
			case VolumeType::ADPCM:
				return
					{
						UI_TEXT("ADPCM 音量"),
						1
					};
		}
		bug_unreachable("invalid VolumeType");
	}

	using VolumeChoiceItemArr = std::array<TextMenuItem, 3>;

	VolumeChoiceItemArr volumeLevelChoiceItems(VolumeType type)
	{
		return
		{
			TextMenuItem
			{
				UI_TEXT("默认"),
				attachParams(),
				[=, this]() { system().setVolume(type, 100); },
				{.id = 100}
			},
			TextMenuItem
			{
				UI_TEXT("关"),
				attachParams(),
				[=, this]() { system().setVolume(type, 0); },
				{.id = 0}
			},
			TextMenuItem
			{
				UI_TEXT("自定义"),
				attachParams(),
				[=, this](Input::Event e)
				{
					pushAndShowNewCollectValueRangeInputView<int, 0, 200>(attachParams(), e,
						UI_TEXT("请输入 0 到 200 之间的值"),
						"",
						[=, this](CollectTextInputView&, auto val)
						{
							system().setVolume(type, val);
							volumeLevel[desc(type).idx].setSelected(MenuId{val}, *this);
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
		volumeLevelChoiceItems(VolumeType::CDDA),
		volumeLevelChoiceItems(VolumeType::ADPCM),
	};

	MultiChoiceMenuItem volumeLevelMenuItem(VolumeType type)
	{
		return
		{
			desc(type).name, attachParams(),
			MenuId{system().volume(type)},
			volumeLevelItem[desc(type).idx],
			{
				.onSetDisplayString = [this, type](auto idx, Gfx::Text &t)
				{
					t.resetString(std::format(
						UI_TEXT("{}%"),
						system().volume(type)));
					return true;
				}
			},
		};
	}

	std::array<MultiChoiceMenuItem, 2> volumeLevel
	{
		volumeLevelMenuItem(VolumeType::CDDA),
		volumeLevelMenuItem(VolumeType::ADPCM),
	};

	BoolMenuItem adpcmFilter
	{
		UI_TEXT("ADPCM 低通滤波器"),
		attachParams(),
		system().adpcmFilter,
		[this](BoolMenuItem &item) { system().setAdpcmFilter(item.flipBoolValue(*this)); }
	};

public:
	CustomAudioOptionView(ViewAttachParams attach, EmuAudio& audio): AudioOptionView{attach, audio, true}
	{
		loadStockItems();
		item.emplace_back(&adpcmFilter);
		item.emplace_back(&mixer);
		item.emplace_back(&volumeLevel[0]);
		item.emplace_back(&volumeLevel[1]);
	}
};

std::unique_ptr<View> EmuApp::makeCustomView(ViewAttachParams attach, ViewID id)
{
	switch(id)
	{
		case ViewID::SYSTEM_ACTIONS: return std::make_unique<CustomSystemActionsView>(attach);
		case ViewID::AUDIO_OPTIONS: return std::make_unique<CustomAudioOptionView>(attach, audio);
		case ViewID::VIDEO_OPTIONS: return std::make_unique<CustomVideoOptionView>(attach, videoLayer);
		case ViewID::SYSTEM_OPTIONS: return std::make_unique<CustomSystemOptionView>(attach);
		case ViewID::FILE_PATH_OPTIONS: return std::make_unique<CustomFilePathOptionView>(attach);
		default: return nullptr;
	}
}

}
