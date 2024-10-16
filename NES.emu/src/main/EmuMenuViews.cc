/*  This file is part of NES.emu.

	NES.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	NES.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with NES.emu.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/EmuApp.hh>
#include <emuframework/EmuViewController.hh>
#include <emuframework/AudioOptionView.hh>
#include <emuframework/VideoOptionView.hh>
#include <emuframework/FilePathOptionView.hh>
#include <emuframework/DataPathSelectView.hh>
#include <emuframework/UserPathSelectView.hh>
#include <emuframework/SystemOptionView.hh>
#include <emuframework/SystemActionsView.hh>
#include <emuframework/FilePicker.hh>
#include <emuframework/viewUtils.hh>
#include "EmuCheatViews.hh"
#include "MainApp.hh"
#include <imagine/gui/AlertView.hh>
#include <imagine/fs/FS.hh>
#include <imagine/util/format.hh>
#include <imagine/util/string.h>
#include <fceu/fds.h>
#include <fceu/sound.h>
#include <fceu/fceu.h>
#include <imagine/logger/logger.h>

extern int pal_emulation;

namespace EmuEx
{

constexpr SystemLogger log{"NES.emu"};

using MainAppHelper = EmuAppHelperBase<MainApp>;

class ConsoleOptionView : public TableView, public MainAppHelper
{
	BoolMenuItem fourScore
	{
		UI_TEXT("四人玩家适配器"),
		attachParams(),
		(bool)system().optionFourScore,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			system().sessionOptionSet();
			system().optionFourScore = item.flipBoolValue(*this);
			system().setupNESFourScore();
		}
	};

	static uint16_t packInputEnums(ESI port1, ESI port2)
	{
		return (uint16_t)port1 | ((uint16_t)port2 << 8);
	}

	static std::pair<ESI, ESI> unpackInputEnums(uint16_t packed)
	{
		return {ESI(packed & 0xFF), ESI(packed >> 8)};
	}

	TextMenuItem inputPortsItem[4]
	{
		{
			UI_TEXT("自动"),
			attachParams(), {.id = packInputEnums(SI_UNSET, SI_UNSET)}
		},
		{
			UI_TEXT("手柄"),
			attachParams(), {.id = packInputEnums(SI_GAMEPAD, SI_GAMEPAD)}
		},
		{
			UI_TEXT("光枪 (2号玩家)"),
			attachParams(), {.id = packInputEnums(SI_GAMEPAD, SI_ZAPPER)}
		},
		{
			UI_TEXT("光枪 (1号玩家对战)"),
			attachParams(), {.id = packInputEnums(SI_ZAPPER, SI_GAMEPAD)}
		},
	};

	MultiChoiceMenuItem inputPorts
	{
		UI_TEXT("输入端口"),
		attachParams(),
		MenuId{packInputEnums(system().inputPort1.value(), system().inputPort2.value())},
		inputPortsItem,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				system().sessionOptionSet();
				auto [port1, port2] = unpackInputEnums(item.id);
				system().inputPort1 = port1;
				system().inputPort2 = port2;
				system().setupNESInputPorts();
			}
		}
	};

	BoolMenuItem fcMic
	{
		UI_TEXT("2号玩家为麦克风"),
		attachParams(),
		replaceP2StartWithMicrophone,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			system().sessionOptionSet();
			replaceP2StartWithMicrophone = item.flipBoolValue(*this);
		}
	};

	TextMenuItem videoSystemItem[4]
	{
		{
			UI_TEXT("自动"),
			attachParams(), {.id = 0}
		},
		{
			UI_TEXT("NTSC"),
			attachParams(), {.id = 1}
		},
		{
			UI_TEXT("PAL"),
			attachParams(), {.id = 2}
		},
		{
			UI_TEXT("Dendy"),
			attachParams(), {.id = 3}
		},
	};

	MultiChoiceMenuItem videoSystem
	{
		UI_TEXT("视频制式"),
		attachParams(),
		MenuId{system().optionVideoSystem},
		videoSystemItem,
		{
			.onSetDisplayString = [](auto idx, Gfx::Text &t)
			{
				if(idx == 0)
				{
					t.resetString(dendy
						? UI_TEXT("Dendy")
						: pal_emulation
							? UI_TEXT("PAL")
							: UI_TEXT("NTSC")
					);
					return true;
				}
				return false;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item, Input::Event e)
			{
				system().sessionOptionSet();
				system().optionVideoSystem = item.id;
				setRegion(item.id, system().optionDefaultVideoSystem, system().autoDetectedRegion);
				app().promptSystemReloadDueToSetOption(attachParams(), e);
			}
		},
	};

	BoolMenuItem compatibleFrameskip
	{
		UI_TEXT("跳帧模式"),
		attachParams(),
		(bool)system().optionCompatibleFrameskip,
		UI_TEXT("快速"),
		UI_TEXT("兼容"),
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			if(!item.boolValue())
			{
				app().pushAndShowModalView(makeView<YesNoAlertView>(
					UI_TEXT("如果当前游戏在快进/跳帧时出现故障，")
					UI_TEXT("请使用兼容模式。"),
					YesNoAlertView::Delegates
					{
						.onYes = [this, &item]
						{
							system().sessionOptionSet();
							system().optionCompatibleFrameskip = item.flipBoolValue(*this);
						}
					}), e);
			}
			else
			{
				system().sessionOptionSet();
				system().optionCompatibleFrameskip = item.flipBoolValue(*this);
			}
		}
	};

	TextHeadingMenuItem videoHeading
	{
		UI_TEXT("视频："),
		attachParams()
	};

	static uint16_t packVideoLines(uint8_t start, uint8_t total)
	{
		return (uint16_t)start | ((uint16_t)total << 8);
	}

	static std::pair<uint8_t, uint8_t> unpackVideoLines(uint16_t packed)
	{
		return {uint8_t(packed & 0xFF), uint8_t(packed >> 8)};
	}

	TextMenuItem visibleVideoLinesItem[4]
	{
		{
			UI_TEXT("8+224"),
			attachParams(), {.id = packVideoLines(8, 224)}
		},
		{
			UI_TEXT("8+232"),
			attachParams(), {.id = packVideoLines(8, 232)}
		},
		{
			UI_TEXT("0+232"),
			attachParams(), {.id = packVideoLines(0, 232)}
		},
		{
			UI_TEXT("0+240"),
			attachParams(), {.id = packVideoLines(0, 240)}
		},
	};

	MultiChoiceMenuItem visibleVideoLines
	{
		UI_TEXT("可见行数"),
		attachParams(),
		MenuId{packVideoLines(system().optionStartVideoLine, system().optionVisibleVideoLines)},
		visibleVideoLinesItem,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				auto [startLine, lines] = unpackVideoLines(item.id);
				system().sessionOptionSet();
				system().optionStartVideoLine = startLine;
				system().optionVisibleVideoLines = lines;
				system().updateVideoPixmap(app().video, system().optionHorizontalVideoCrop, system().optionVisibleVideoLines);
				system().renderFramebuffer(app().video);
				app().viewController().placeEmuViews();
			}
		}
	};

	BoolMenuItem horizontalVideoCrop
	{
		UI_TEXT("左右两边各裁剪8个像素"),
		attachParams(),
		(bool)system().optionHorizontalVideoCrop,
		[this](BoolMenuItem &item)
		{
			system().sessionOptionSet();
			system().optionHorizontalVideoCrop = item.flipBoolValue(*this);
			system().updateVideoPixmap(app().video, system().optionHorizontalVideoCrop, system().optionVisibleVideoLines);
			system().renderFramebuffer(app().video);
			app().viewController().placeEmuViews();
		}
	};

	TextHeadingMenuItem overclocking
	{
		UI_TEXT("超频："),
		attachParams()
	};

	BoolMenuItem overclockingEnabled
	{
		UI_TEXT("超频开关"),
		attachParams(),
		overclock_enabled,
		[this](BoolMenuItem &item)
		{
			system().sessionOptionSet();
			overclock_enabled = item.flipBoolValue(*this);
		}
	};

	DualTextMenuItem extraLines
	{
		UI_TEXT("每帧的额外行数"),
		std::to_string(postrenderscanlines), attachParams(),
		[this](const Input::Event &e)
		{
			pushAndShowNewCollectValueRangeInputView<int, 0, maxExtraLinesPerFrame>(attachParams(), e,
				UI_TEXT("请输入 0 到 30000 之间的值"),
				std::to_string(postrenderscanlines),
				[this](CollectTextInputView&, auto val)
				{
					system().sessionOptionSet();
					postrenderscanlines = val;
					extraLines.set2ndName(std::to_string(val));
					return true;
				});
		}
	};

	DualTextMenuItem vblankMultipler
	{
		UI_TEXT("垂直消隐行倍数"),
		std::to_string(vblankscanlines), attachParams(),
		[this](const Input::Event &e)
		{
			pushAndShowNewCollectValueRangeInputView<int, 0, maxVBlankMultiplier>(attachParams(), e,
				UI_TEXT("请输入 0 到 16 之间的值"),
				std::to_string(vblankscanlines),
				[this](CollectTextInputView&, auto val)
				{
					system().sessionOptionSet();
					vblankscanlines = val;
					vblankMultipler.set2ndName(std::to_string(val));
					return true;
				});
		}
	};

	std::array<MenuItem*, 12> menuItem
	{
		&inputPorts,
		&fourScore,
		&fcMic,
		&compatibleFrameskip,
		&videoHeading,
		&videoSystem,
		&visibleVideoLines,
		&horizontalVideoCrop,
		&overclocking,
		&overclockingEnabled,
		&extraLines,
		&vblankMultipler,
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

class CustomVideoOptionView : public VideoOptionView, public MainAppHelper
{
	using  MainAppHelper::app;
	using  MainAppHelper::system;

	BoolMenuItem spriteLimit
	{
		UI_TEXT("精灵限制"),
		attachParams(),
		(bool)system().optionSpriteLimit,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			system().optionSpriteLimit = item.flipBoolValue(*this);
			FCEUI_DisableSpriteLimitation(!system().optionSpriteLimit);
		}
	};

	TextMenuItem videoSystemItem[4]
	{
		{
			UI_TEXT("自动"),
			attachParams(), [this](){ system().optionDefaultVideoSystem = 0; }
		},
		{
			UI_TEXT("NTSC"),
			attachParams(), [this](){ system().optionDefaultVideoSystem = 1; }
		},
		{
			UI_TEXT("PAL"),
			attachParams(), [this](){ system().optionDefaultVideoSystem = 2; }
		},
		{
			UI_TEXT("Dendy"),
			attachParams(), [this](){ system().optionDefaultVideoSystem = 3; }
		},
	};

	MultiChoiceMenuItem videoSystem
	{
		UI_TEXT("默认视频制式"),
		attachParams(),
		system().optionDefaultVideoSystem.value(),
		videoSystemItem
	};

	static constexpr auto digitalPrimePalPath = "Digital Prime (FBX).pal";
	static constexpr auto smoothPalPath = "Smooth V2 (FBX).pal";
	static constexpr auto magnumPalPath = "Magnum (FBX).pal";
	static constexpr auto classicPalPath = "Classic (FBX).pal";
	static constexpr auto wavebeamPalPath = "Wavebeam.pal";
	static constexpr auto lightfulPalPath = "Lightful.pal";
	static constexpr auto palightfulPalPath = "Palightful.pal";

	void setPalette(IG::ApplicationContext ctx, IG::CStringView palPath)
	{
		if(palPath.size())
			system().defaultPalettePath = palPath;
		else
			system().defaultPalettePath = {};
		system().setDefaultPalette(ctx, palPath);
		auto &app = EmuApp::get(ctx);
		app.renderSystemFramebuffer();
	}

	constexpr size_t defaultPaletteCustomFileIdx()
	{
		return lastIndex(defaultPalItem);
	}

	TextMenuItem defaultPalItem[9]
	{
		{
			UI_TEXT("FCEUX"),
			attachParams(), [this]() { setPalette(appContext(), ""); }
		},
		{
			UI_TEXT("Digital Prime (FBX)"),
			attachParams(), [this]() { setPalette(appContext(), digitalPrimePalPath); }
		},
		{
			UI_TEXT("Smooth V2 (FBX)"),
			attachParams(), [this]() { setPalette(appContext(), smoothPalPath); }
		},
		{
			UI_TEXT("Magnum (FBX)"),
			attachParams(), [this]() { setPalette(appContext(), magnumPalPath); }
		},
		{
			UI_TEXT("Classic (FBX)"),
			attachParams(), [this]() { setPalette(appContext(), classicPalPath); }
		},
		{
			UI_TEXT("Wavebeam"),
			attachParams(), [this]() { setPalette(appContext(), wavebeamPalPath); }
		},
		{
			UI_TEXT("Lightful"),
			attachParams(), [this]() { setPalette(appContext(), lightfulPalPath); }
		},
		{
			UI_TEXT("Palightful"),
			attachParams(), [this]() { setPalette(appContext(), palightfulPalPath); }
		},
		{
			UI_TEXT("自定义"),
			attachParams(),
			[this](Input::Event e)
			{
				auto fsFilter = [](std::string_view name) { return endsWithAnyCaseless(name, ".pal"); };
				auto fPicker = makeView<FilePicker>(FSPicker::Mode::FILE, fsFilter, e, false);
				fPicker->setOnSelectPath(
					[this](FSPicker &picker, IG::CStringView path, std::string_view name, Input::Event)
					{
						setPalette(appContext(), path.data());
						defaultPal.setSelected(defaultPaletteCustomFileIdx());
						dismissPrevious();
						picker.dismiss();
					});
				fPicker->setPath(app().contentSearchPath, e);
				app().pushAndShowModalView(std::move(fPicker), e);
				return false;
			}},
	};

	MultiChoiceMenuItem defaultPal
	{
		UI_TEXT("默认调色板"),
		attachParams(),
		[this]()
		{
			if(system().defaultPalettePath.empty()) return 0;
			if(system().defaultPalettePath == digitalPrimePalPath) return 1;
			if(system().defaultPalettePath == smoothPalPath) return 2;
			if(system().defaultPalettePath == magnumPalPath) return 3;
			if(system().defaultPalettePath == classicPalPath) return 4;
			if(system().defaultPalettePath == wavebeamPalPath) return 5;
			if(system().defaultPalettePath == lightfulPalPath) return 6;
			if(system().defaultPalettePath == palightfulPalPath) return 7;
			return (int)defaultPaletteCustomFileIdx();
		}(),
		defaultPalItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(idx == defaultPaletteCustomFileIdx())
				{
					t.resetString(IG::withoutDotExtension(appContext().fileUriDisplayName(system().defaultPalettePath)));
					return true;
				}
				return false;
			}
		},
	};

	TextMenuItem visibleVideoLinesItem[4]
	{
		{
			UI_TEXT("8+224"),
			attachParams(), setVisibleVideoLinesDel(8, 224)
		},
		{
			UI_TEXT("8+232"),
			attachParams(), setVisibleVideoLinesDel(8, 232)
		},
		{
			UI_TEXT("0+232"),
			attachParams(), setVisibleVideoLinesDel(0, 232)
		},
		{
			UI_TEXT("0+240"),
			attachParams(), setVisibleVideoLinesDel(0, 240)
		},
	};

	MultiChoiceMenuItem visibleVideoLines
	{
		UI_TEXT("默认可见行数"),
		attachParams(),
		[this]()
		{
			switch(system().optionDefaultVisibleVideoLines)
			{
				default: return 0;
				case 232: return system().optionDefaultStartVideoLine == 8 ? 1 : 2;
				case 240: return 3;
			}
		}(),
		visibleVideoLinesItem
	};

	TextMenuItem::SelectDelegate setVisibleVideoLinesDel(uint8_t startLine, uint8_t lines)
	{
		return [this, startLine, lines]()
		{
			system().optionDefaultStartVideoLine = startLine;
			system().optionDefaultVisibleVideoLines = lines;
		};
	}

	BoolMenuItem correctLineAspect
	{
		UI_TEXT("修正行每行宽高比例"),
		attachParams(),
		(bool)system().optionCorrectLineAspect,
		[this](BoolMenuItem &item)
		{
			system().optionCorrectLineAspect = item.flipBoolValue(*this);
			app().viewController().placeEmuViews();
		}
	};

public:
	CustomVideoOptionView(ViewAttachParams attach, EmuVideoLayer &layer): VideoOptionView{attach, layer, true}
	{
		loadStockItems();
		item.emplace_back(&systemSpecificHeading);
		item.emplace_back(&defaultPal);
		item.emplace_back(&videoSystem);
		item.emplace_back(&spriteLimit);
		item.emplace_back(&visibleVideoLines);
		item.emplace_back(&correctLineAspect);
	}
};

class CustomAudioOptionView : public AudioOptionView, public MainAppHelper
{
	using MainAppHelper::system;

	void setQuality(int quaility)
	{
		system().optionSoundQuality = quaility;
		FCEUI_SetSoundQuality(quaility);
	}

	TextMenuItem qualityItem[3]
	{
		{
			UI_TEXT("正常"),
			attachParams(), [this](){ setQuality(0); }
		},
		{
			UI_TEXT("高"),
			attachParams(), [this](){ setQuality(1); }
		},
		{
			UI_TEXT("最高"),
			attachParams(), [this](){ setQuality(2); }
		}
	};

	MultiChoiceMenuItem quality
	{
		UI_TEXT("模拟质量"),
		attachParams(),
		system().optionSoundQuality.value(),
		qualityItem
	};

	BoolMenuItem lowPassFilter
	{
		UI_TEXT("低通滤波器"),
		attachParams(),
		(bool)FSettings.lowpass,
		[this](BoolMenuItem &item)
		{
			FCEUI_SetLowPass(item.flipBoolValue(*this));
		}
	};

	BoolMenuItem swapDutyCycles
	{
		UI_TEXT("交换占空比"),
		attachParams(),
		swapDuty,
		[this](BoolMenuItem &item)
		{
			swapDuty = item.flipBoolValue(*this);
		}
	};

	TextHeadingMenuItem mixer
	{
		UI_TEXT("音频混合器："),
		attachParams()
	};

	BoolMenuItem squareWave1
	{
		UI_TEXT("方波 #1"),
		attachParams(),
		(bool)FSettings.Square1Volume,
		[this](BoolMenuItem &item)
		{
			FSettings.Square1Volume = item.flipBoolValue(*this) ? 256 : 0;
		}
	};

	BoolMenuItem squareWave2
	{
		UI_TEXT("方波 #2"),
		attachParams(),
		(bool)FSettings.Square2Volume,
		[this](BoolMenuItem &item)
		{
			FSettings.Square2Volume = item.flipBoolValue(*this) ? 256 : 0;
		}
	};

	BoolMenuItem triangleWave1
	{
		UI_TEXT("三角波"),
		attachParams(),
		(bool)FSettings.TriangleVolume,
		[this](BoolMenuItem &item)
		{
			FSettings.TriangleVolume = item.flipBoolValue(*this) ? 256 : 0;
		}
	};

	BoolMenuItem noise
	{
		UI_TEXT("噪音"),
		attachParams(),
		(bool)FSettings.NoiseVolume,
		[this](BoolMenuItem &item)
		{
			FSettings.NoiseVolume = item.flipBoolValue(*this) ? 256 : 0;
		}
	};

	BoolMenuItem dpcm
	{
		UI_TEXT("DPCM"),
		attachParams(),
		(bool)FSettings.PCMVolume,
		[this](BoolMenuItem &item)
		{
			FSettings.PCMVolume = item.flipBoolValue(*this) ? 256 : 0;
		}
	};

public:
	CustomAudioOptionView(ViewAttachParams attach, EmuAudio& audio): AudioOptionView{attach, audio, true}
	{
		loadStockItems();
		item.emplace_back(&quality);
		item.emplace_back(&lowPassFilter);
		item.emplace_back(&swapDutyCycles);
		item.emplace_back(&mixer);
		item.emplace_back(&squareWave1);
		item.emplace_back(&squareWave2);
		item.emplace_back(&triangleWave1);
		item.emplace_back(&noise);
		item.emplace_back(&dpcm);
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
					log.info("set cheats path:{}", path);
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
					log.info("set patches path:{}", path);
					system().patchesDir = path;
					patchesPath.compile(patchesMenuName(appContext(), path));
				}), e);
		}
	};

	TextMenuItem palettesPath
	{
		palettesMenuName(appContext(), system().palettesDir), attachParams(),
		[this](const Input::Event &e)
		{
			pushAndShow(makeViewWithName<UserPathSelectView>(
				UI_TEXT("调色板文件夹"),
				system().userPath(system().palettesDir),
				[this](CStringView path)
				{
					log.info("set palettes path:{}", path);
					system().palettesDir = path;
					palettesPath.compile(palettesMenuName(appContext(), path));
				}), e);
		}
	};

	TextMenuItem fdsBios
	{
		biosMenuEntryStr(system().fdsBiosPath), attachParams(),
		[this](TextMenuItem &, View &, Input::Event e)
		{
			pushAndShow(makeViewWithName<DataFileSelectView<>>(
				UI_TEXT("磁碟机 BIOS"),
				app().validSearchPath(FS::dirnameUri(system().fdsBiosPath)),
				[this](CStringView path, FS::file_type type)
				{
					system().fdsBiosPath = path;
					log.info("set fds bios:{}", path);
					fdsBios.compile(biosMenuEntryStr(path));
					return true;
				}, hasFDSBIOSExtension), e);
		}
	};

	std::string biosMenuEntryStr(CStringView path) const
	{
		return std::format(
			UI_TEXT("磁碟机的 BIOS 文件：{}"),
			appContext().fileUriDisplayName(path));
	}

public:
	CustomFilePathOptionView(ViewAttachParams attach): FilePathOptionView{attach, true}
	{
		loadStockItems();
		item.emplace_back(&cheatsPath);
		item.emplace_back(&patchesPath);
		item.emplace_back(&palettesPath);
		item.emplace_back(&fdsBios);
	}
};

class FDSControlView : public TableView, public MainAppHelper
{
private:
	static constexpr unsigned DISK_SIDES = 4;
	TextMenuItem setSide[DISK_SIDES]
	{
		{
			UI_TEXT("指定1号磁碟 A面"),
			attachParams(),
			[this](View &view, Input::Event e)
			{
				FCEU_FDSSetDisk(0, system());
				view.dismiss();
			}
		},
		{
			UI_TEXT("指定1号磁碟 B面"),
			attachParams(),
			[this](View &view, Input::Event e)
			{
				FCEU_FDSSetDisk(1, system());
				view.dismiss();
			}
		},
		{
			UI_TEXT("指定2号磁碟 A面"),
			attachParams(),
			[this](View &view, Input::Event e)
			{
				FCEU_FDSSetDisk(2, system());
				view.dismiss();
			}
		},
		{
			UI_TEXT("指定2号磁碟 B面"),
			attachParams(),
			[this](View &view, Input::Event e)
			{
				FCEU_FDSSetDisk(3, system());
				view.dismiss();
			}
		}
	};

	TextMenuItem insertEject
	{
		UI_TEXT("弹出"),
		attachParams(),
		[](View& view)
		{
			if(FCEU_FDSInserted())
			{
				FCEU_FDSInsert();
				view.dismiss();
			}
		}
	};

	std::array<TextMenuItem*, 5> items{&setSide[0], &setSide[1], &setSide[2], &setSide[3], &insertEject};

public:
	FDSControlView(ViewAttachParams attach):
		TableView
		{
			UI_TEXT("磁碟机控制"),
			attach,
			items
		}
	{
		setSide[0].setActive(0 < FCEU_FDSSides());
		setSide[1].setActive(1 < FCEU_FDSSides());
		setSide[2].setActive(2 < FCEU_FDSSides());
		setSide[3].setActive(3 < FCEU_FDSSides());
		insertEject.setActive(FCEU_FDSInserted());
	}
};

class CustomSystemActionsView : public SystemActionsView
{
private:
	TextMenuItem fdsControl
	{
		u"", attachParams(),
		[this](Input::Event e) { pushAndShow(makeView<FDSControlView>(), e); }
	};

	void refreshFDSItem()
	{
		if(!isFDS)
			return;
		if(!FCEU_FDSInserted())
			fdsControl.compile(
				UI_TEXT("磁碟机控制 (没有磁碟)")
			);
		else
			fdsControl.compile(std::format(
				UI_TEXT("磁碟机控制 ({}号磁碟 {}面)"),
				(FCEU_FDSCurrentSide() >> 1) + 1, (FCEU_FDSCurrentSide() & 1) ? 'B' : 'A'));
	}

	TextMenuItem options
	{
		UI_TEXT("主机选项"),
		attachParams(),
		[this](Input::Event e) { pushAndShow(makeView<ConsoleOptionView>(), e); }
	};

public:
	CustomSystemActionsView(ViewAttachParams attach): SystemActionsView{attach, true}
	{
		if(isFDS)
			item.emplace_back(&fdsControl);
		item.emplace_back(&options);
		loadStandardItems();
	}

	void onShow()
	{
		SystemActionsView::onShow();
		refreshFDSItem();
	}
};

class CustomSystemOptionView : public SystemOptionView, public MainAppHelper
{
	using MainAppHelper::system;

	BoolMenuItem skipFdcAccess
	{
		UI_TEXT("优化磁碟读写性能"),
		attachParams(),
		system().fastForwardDuringFdsAccess,
		[this](BoolMenuItem &item)
		{
			system().fastForwardDuringFdsAccess = item.flipBoolValue(*this);
		}
	};

public:
	CustomSystemOptionView(ViewAttachParams attach): SystemOptionView{attach, true}
	{
		loadStockItems();
		item.emplace_back(&skipFdcAccess);
	}
};

std::unique_ptr<View> EmuApp::makeCustomView(ViewAttachParams attach, ViewID id)
{
	switch(id)
	{
		case ViewID::SYSTEM_ACTIONS: return std::make_unique<CustomSystemActionsView>(attach);
		case ViewID::VIDEO_OPTIONS: return std::make_unique<CustomVideoOptionView>(attach, videoLayer);
		case ViewID::AUDIO_OPTIONS: return std::make_unique<CustomAudioOptionView>(attach, audio);
		case ViewID::SYSTEM_OPTIONS: return std::make_unique<CustomSystemOptionView>(attach);
		case ViewID::FILE_PATH_OPTIONS: return std::make_unique<CustomFilePathOptionView>(attach);
		default: return nullptr;
	}
}

}
