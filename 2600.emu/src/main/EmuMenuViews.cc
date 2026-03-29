/*  This file is part of 2600.emu.

	2600.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	2600.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with 2600.emu.  If not, see <http://www.gnu.org/licenses/> */

#include <stella/emucore/Props.hxx>
#include <stella/emucore/Control.hxx>
#include <stella/emucore/Paddles.hxx>
#include <SoundEmuEx.hh>
import system;
import emuex;
import imagine;

namespace EmuEx
{

using namespace IG;
using MainAppHelper = EmuAppHelperBase<MainApp>;

class CustomAudioOptionView : public AudioOptionView, public MainAppHelper
{
	using MainAppHelper::system;

	TextMenuItem::SelectDelegate setResampleQualityDel()
	{
		return [this](TextMenuItem &item)
		{
			A2600System::log.info("set resampling quality:{}", item.id.val);
			system().optionAudioResampleQuality = AudioSettings::ResamplingQuality(item.id.val);
			system().osystem.soundEmuEx().setResampleQuality(system().optionAudioResampleQuality);
		};
	}

	TextMenuItem resampleQualityItem[3]
	{
		{UI_TEXT("Low"),   attachParams(), setResampleQualityDel(), {.id = AudioSettings::ResamplingQuality::nearestNeighbour}},
		{UI_TEXT("High"),  attachParams(), setResampleQualityDel(), {.id = AudioSettings::ResamplingQuality::lanczos_2}},
		{UI_TEXT("Ultra"), attachParams(), setResampleQualityDel(), {.id = AudioSettings::ResamplingQuality::lanczos_3}},
	};

	MultiChoiceMenuItem resampleQuality
	{
		UI_TEXT("Resampling Quality"),
		attachParams(),
		MenuId{system().optionAudioResampleQuality.value()},
		resampleQualityItem
	};

public:
	CustomAudioOptionView(ViewAttachParams attach, EmuAudio& audio): AudioOptionView{attach, audio, true}
	{
		loadStockItems();
		item.emplace_back(&resampleQuality);
	}
};

class CustomVideoOptionView : public VideoOptionView, public MainAppHelper
{
	using MainAppHelper::system;

	TextMenuItem tvPhosphorBlendItem[4]
	{
		{UI_TEXT("70%"),  attachParams(), setTVPhosphorBlendDel(), {.id = 70}},
		{UI_TEXT("80%"),  attachParams(), setTVPhosphorBlendDel(), {.id = 80}},
		{UI_TEXT("90%"),  attachParams(), setTVPhosphorBlendDel(), {.id = 90}},
		{UI_TEXT("100%"), attachParams(), setTVPhosphorBlendDel(), {.id = 100}},
	};

	MultiChoiceMenuItem tvPhosphorBlend
	{
		UI_TEXT("TV Phosphor Blending"),
		attachParams(),
		MenuId{system().optionTVPhosphorBlend},
		tvPhosphorBlendItem
	};

	TextMenuItem::SelectDelegate setTVPhosphorBlendDel()
	{
		return [this](TextMenuItem &item)
		{
			system().optionTVPhosphorBlend = item.id;
			system().setRuntimeTVPhosphor(system().optionTVPhosphor, item.id);
		};
	}

public:
	CustomVideoOptionView(ViewAttachParams attach, EmuVideoLayer& layer): VideoOptionView{attach, layer, true}
	{
		loadStockItems();
		item.emplace_back(&systemSpecificHeading);
		item.emplace_back(&tvPhosphorBlend);
	}
};

class ConsoleOptionView : public TableView, public MainAppHelper
{
	TextMenuItem tvPhosphorItem[3]
	{
		{UI_TEXT("Off"),  attachParams(), setTVPhosphorDel(), {.id = 0}},
		{UI_TEXT("On"),   attachParams(), setTVPhosphorDel(), {.id = 1}},
		{UI_TEXT("Auto"), attachParams(), setTVPhosphorDel(), {.id = TV_PHOSPHOR_AUTO}},
	};

	MultiChoiceMenuItem tvPhosphor
	{
		UI_TEXT("Simulate TV Phosphor"),
		attachParams(),
		MenuId{system().optionTVPhosphor},
		tvPhosphorItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(idx == 2 && system().osystem.hasConsole())
				{
					bool phospherInUse = system().osystem.console().properties().get(PropType::Display_Phosphor) == "YES";
					t.resetString(
						phospherInUse
							? UI_TEXT("On")
							: UI_TEXT("Off"));
					return true;
				}
				else
					return false;
			}
		},
	};

	TextMenuItem videoSystemItem[7]
	{
		{UI_TEXT("Auto"),     attachParams(), setVideoSystemDel(), {.id = 0}},
		{UI_TEXT("NTSC"),     attachParams(), setVideoSystemDel(), {.id = 1}},
		{UI_TEXT("PAL"),      attachParams(), setVideoSystemDel(), {.id = 2}},
		{UI_TEXT("SECAM"),    attachParams(), setVideoSystemDel(), {.id = 3}},
		{UI_TEXT("NTSC 50"),  attachParams(), setVideoSystemDel(), {.id = 4}},
		{UI_TEXT("PAL 60"),   attachParams(), setVideoSystemDel(), {.id = 5}},
		{UI_TEXT("SECAM 60"), attachParams(), setVideoSystemDel(), {.id = 6}},
	};

	MultiChoiceMenuItem videoSystem
	{
		UI_TEXT("Video System"),
		attachParams(),
		MenuId{system().optionVideoSystem},
		videoSystemItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(idx == 0 && system().osystem.hasConsole())
				{
					t.resetString(system().osystem.console().about().DisplayFormat.c_str());
					return true;
				}
				else
					return false;
			}
		},
	};

	TextMenuItem::SelectDelegate setTVPhosphorDel()
	{
		return [this](TextMenuItem &item)
		{
			system().sessionOptionSet();
			system().optionTVPhosphor = item.id;
			system().setRuntimeTVPhosphor(item.id, system().optionTVPhosphorBlend);
		};
	}

	TextMenuItem::SelectDelegate setVideoSystemDel()
	{
		return [this](TextMenuItem &item, const Input::Event &e)
		{
			system().sessionOptionSet();
			system().optionVideoSystem = item.id;
			app().promptSystemReloadDueToSetOption(attachParams(), e);
		};
	}

	TextMenuItem inputPortsItem[5]
	{
		{UI_TEXT("Auto"),            attachParams(), setInputPortsDel(), {.id = Controller::Type::Unknown}},
		{UI_TEXT("Joystick"),        attachParams(), setInputPortsDel(), {.id = Controller::Type::Joystick}},
		{UI_TEXT("Paddles"),         attachParams(), setInputPortsDel(), {.id = Controller::Type::Paddles}},
		{UI_TEXT("Genesis Gamepad"), attachParams(), setInputPortsDel(), {.id = Controller::Type::Genesis}},
		{UI_TEXT("Booster Grip"),    attachParams(), setInputPortsDel(), {.id = Controller::Type::BoosterGrip}},
	};

	MultiChoiceMenuItem inputPorts
	{
		UI_TEXT("Input Ports"),
		attachParams(),
		MenuId{system().optionInputPort1.value()},
		inputPortsItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text& t)
			{
				if(idx == 0 && system().osystem.hasConsole())
				{
					t.resetString(asString(system().osystem.console().leftController().type()));
					return true;
				}
				else
					return false;
			}
		},
	};

	TextMenuItem::SelectDelegate setInputPortsDel()
	{
		return [this](TextMenuItem &item)
		{
			system().sessionOptionSet();
			system().optionInputPort1 = Controller::Type(item.id.val);
			if(system().osystem.hasConsole())
			{
				system().setControllerType(app(), system().osystem.console(), Controller::Type(item.id.val));
			}
		};
	}

	TextMenuItem aPaddleRegionItem[4]
	{
		{UI_TEXT("Off"),        attachParams(), setAPaddleRegionDel(), {.id = PaddleRegionMode::OFF}},
		{UI_TEXT("Left Half"),  attachParams(), setAPaddleRegionDel(), {.id = PaddleRegionMode::LEFT}},
		{UI_TEXT("Right Half"), attachParams(), setAPaddleRegionDel(), {.id = PaddleRegionMode::RIGHT}},
		{UI_TEXT("Full"),       attachParams(), setAPaddleRegionDel(), {.id = PaddleRegionMode::FULL}},
	};

	MultiChoiceMenuItem aPaddleRegion
	{
		UI_TEXT("Analog Paddle Region"),
		attachParams(),
		MenuId{system().optionPaddleAnalogRegion},
		aPaddleRegionItem
	};

	TextMenuItem::SelectDelegate setAPaddleRegionDel()
	{
		return [this](TextMenuItem &item)
		{
			system().sessionOptionSet();
			system().updatePaddlesRegionMode(app(), (PaddleRegionMode)item.id.val);
		};
	}

	TextMenuItem dPaddleSensitivityItem[2]
	{
		{UI_TEXT("Default"), attachParams(), [this]() { setDPaddleSensitivity(1); }, {.id = 1}},
		{UI_TEXT("Custom Value"), attachParams(),
			[this](const Input::Event &e)
			{
				pushAndShowNewCollectValueInputView<int>(attachParams(), e,
					UI_TEXT("Input 1 to 20"), "",
					[this](CollectTextInputView&, auto val)
					{
						if(system().optionPaddleDigitalSensitivity.isValid(val))
						{
							setDPaddleSensitivity(val);
							dPaddleSensitivity.setSelected(lastIndex(dPaddleSensitivityItem), *this);
							dismissPrevious();
							return true;
						}
						else
						{
							app().postErrorMessage(UI_TEXT("Value not in range"));
							return false;
						}
					});
				return false;
			}, {.id = defaultMenuId}
		}
	};

	MultiChoiceMenuItem dPaddleSensitivity
	{
		UI_TEXT("Digital Paddle Sensitivity"),
		attachParams(),
		MenuId{system().optionPaddleDigitalSensitivity},
		dPaddleSensitivityItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format(UI_TEXT("{}"), system().optionPaddleDigitalSensitivity.value()));
				return true;
			}
		},
	};

	void setDPaddleSensitivity(uint8_t val)
	{
		system().sessionOptionSet();
		system().optionPaddleDigitalSensitivity = val;
		Paddles::setDigitalSensitivity(system().optionPaddleDigitalSensitivity);
	}

	std::array<MenuItem*, 5> menuItem
	{
		&tvPhosphor,
		&videoSystem,
		&inputPorts,
		&aPaddleRegion,
		&dPaddleSensitivity,
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

class VCSSwitchesView : public TableView, public MainAppHelper
{
	BoolMenuItem diff1
	{
		UI_TEXT("Left (P1) Difficulty"),
		attachParams(),
		system().p1DiffB,
		UI_TEXT("A"),
		UI_TEXT("B"),
		[this](BoolMenuItem& item)
		{
			system().p1DiffB = item.flipBoolValue(*this);
		}
	};

	BoolMenuItem diff2
	{
		UI_TEXT("Right (P2) Difficulty"),
		attachParams(),
		system().p2DiffB,
		UI_TEXT("A"),
		UI_TEXT("B"),
		[this](BoolMenuItem& item)
		{
			system().p2DiffB = item.flipBoolValue(*this);
		}
	};

	BoolMenuItem color
	{
		UI_TEXT("Color"),
		attachParams(),
		system().vcsColor,
		[this](BoolMenuItem& item)
		{
			system().vcsColor = item.flipBoolValue(*this);
		}
	};

	std::array<MenuItem*, 3> menuItem
	{
		&diff1,
		&diff2,
		&color
	};

public:
	VCSSwitchesView(ViewAttachParams attach):
		TableView
		{
			UI_TEXT("Switches"),
			attach,
			menuItem
		}
	{}

	void onShow() final
	{
		diff1.setBoolValue(system().p1DiffB, *this);
		diff2.setBoolValue(system().p2DiffB, *this);
		color.setBoolValue(system().vcsColor, *this);
	}

};

class CustomSystemActionsView : public SystemActionsView
{
private:
	TextMenuItem switches
	{
		UI_TEXT("Console Switches"),
		attachParams(),
		[this](const Input::Event &e)
		{
			if(system().hasContent())
			{
				pushAndShow(makeView<VCSSwitchesView>(), e);
			}
		}
	};

	TextMenuItem options
	{
		UI_TEXT("Console Options"),
		attachParams(),
		[this](const Input::Event &e)
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
		item.emplace_back(&switches);
		item.emplace_back(&options);
		loadStandardItems();
	}
};

std::unique_ptr<View> EmuApp::makeCustomView(ViewAttachParams attach, ViewID id)
{
	switch(id)
	{
		case ViewID::AUDIO_OPTIONS: return std::make_unique<CustomAudioOptionView>(attach, audio);
		case ViewID::VIDEO_OPTIONS: return std::make_unique<CustomVideoOptionView>(attach, videoLayer);
		case ViewID::SYSTEM_ACTIONS: return std::make_unique<CustomSystemActionsView>(attach);
		default: return nullptr;
	}
}

}
