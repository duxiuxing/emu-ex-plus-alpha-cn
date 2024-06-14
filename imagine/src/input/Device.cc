/*  This file is part of Imagine.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Imagine.  If not, see <http://www.gnu.org/licenses/> */

#include <imagine/input/Device.hh>
#include <imagine/logger/logger.h>

namespace IG::Input
{

constexpr SystemLogger log{"InputDev"};

static const char *keyButtonName(Key b)
{
	switch(b)
	{
		case 0:
			return UI_TEXT("None");
		case Keycode::SPACE:
			return UI_TEXT("Space");
		case Keycode::A:
			return UI_TEXT("a");
		case Keycode::B:
			return UI_TEXT("b");
		case Keycode::C:
			return UI_TEXT("c");
		case Keycode::D:
			return UI_TEXT("d");
		case Keycode::E:
			return UI_TEXT("e");
		case Keycode::F:
			return UI_TEXT("f");
		case Keycode::G:
			return UI_TEXT("g");
		case Keycode::H:
			return UI_TEXT("h");
		case Keycode::I:
			return UI_TEXT("i");
		case Keycode::J:
			return UI_TEXT("j");
		case Keycode::K:
			return UI_TEXT("k");
		case Keycode::L:
			return UI_TEXT("l");
		case Keycode::M:
			return UI_TEXT("m");
		case Keycode::N:
			return UI_TEXT("n");
		case Keycode::O:
			return UI_TEXT("o");
		case Keycode::P:
			return UI_TEXT("p");
		case Keycode::Q:
			return UI_TEXT("q");
		case Keycode::R:
			return UI_TEXT("r");
		case Keycode::S:
			return UI_TEXT("s");
		case Keycode::T:
			return UI_TEXT("t");
		case Keycode::U:
			return UI_TEXT("u");
		case Keycode::V:
			return UI_TEXT("v");
		case Keycode::W:
			return UI_TEXT("w");
		case Keycode::X:
			return UI_TEXT("x");
		case Keycode::Y:
			return UI_TEXT("y");
		case Keycode::Z:
			return UI_TEXT("z");
		case Keycode::APOSTROPHE:
			return UI_TEXT("'");
		case Keycode::COMMA:
			return UI_TEXT(",");
		case Keycode::MINUS:
			return UI_TEXT("-");
		case Keycode::PERIOD:
			return UI_TEXT(".");
		case Keycode::SLASH:
			return UI_TEXT("/");
		case Keycode::_0:
			return UI_TEXT("0");
		case Keycode::_1:
			return UI_TEXT("1");
		case Keycode::_2:
			return UI_TEXT("2");
		case Keycode::_3:
			return UI_TEXT("3");
		case Keycode::_4:
			return UI_TEXT("4");
		case Keycode::_5:
			return UI_TEXT("5");
		case Keycode::_6:
			return UI_TEXT("6");
		case Keycode::_7:
			return UI_TEXT("7");
		case Keycode::_8:
			return UI_TEXT("8");
		case Keycode::_9:
			return UI_TEXT("9");
		case Keycode::SEMICOLON:
			return UI_TEXT(";");
		case Keycode::EQUALS:
			return UI_TEXT("=");
		case Keycode::LEFT_BRACKET:
			return UI_TEXT("[");
		case Keycode::BACKSLASH:
			return UI_TEXT("\\");
		case Keycode::RIGHT_BRACKET:
			return UI_TEXT("]");
		case Keycode::GRAVE:
			return UI_TEXT("`");
		case Keycode::BACK:
			return UI_TEXT("Back");
		case Keycode::ESCAPE:
			return UI_TEXT("Escape");
		case Keycode::ENTER:
			return UI_TEXT("Enter");
		case Keycode::LALT:
			return UI_TEXT("Left Alt");
		case Keycode::RALT:
			return UI_TEXT("Right Alt");
		case Keycode::LSHIFT:
			return UI_TEXT("Left Shift");
		case Keycode::RSHIFT:
			return UI_TEXT("Right Shift");
		case Keycode::LCTRL:
			return UI_TEXT("Left Ctrl");
		case Keycode::RCTRL:
			return UI_TEXT("Right Ctrl");
		case Keycode::UP:
			return UI_TEXT("Up");
		case Keycode::RIGHT:
			return UI_TEXT("Right");
		case Keycode::DOWN:
			return UI_TEXT("Down");
		case Keycode::LEFT:
			return UI_TEXT("Left");
		case Keycode::BACK_SPACE:
			return UI_TEXT("Back Space");
		case Keycode::MENU:
			return UI_TEXT("Menu");
		case Keycode::HOME:
			return UI_TEXT("Home");
		case Keycode::END:
			return UI_TEXT("End");
		case Keycode::INSERT:
			return UI_TEXT("Insert");
		case Keycode::DELETE:
			return UI_TEXT("Delete");
		case Keycode::TAB:
			return UI_TEXT("Tab");
		case Keycode::SCROLL_LOCK:
			return UI_TEXT("Scroll Lock");
		case Keycode::CAPS:
			return UI_TEXT("Caps Lock");
		case Keycode::PAUSE:
			return UI_TEXT("Pause");
		case Keycode::LSUPER:
			return UI_TEXT("Left Start/Option");
		case Keycode::RSUPER:
			return UI_TEXT("Right Start/Option");
		case Keycode::PGUP:
			return UI_TEXT("Page Up");
		case Keycode::PGDOWN:
			return UI_TEXT("Page Down");
		case Keycode::PRINT_SCREEN:
			return UI_TEXT("Print Screen");
		case Keycode::NUM_LOCK:
			return UI_TEXT("Num Lock");
		case Keycode::NUMPAD_0:
			return UI_TEXT("Numpad 0");
		case Keycode::NUMPAD_1:
			return UI_TEXT("Numpad 1");
		case Keycode::NUMPAD_2:
			return UI_TEXT("Numpad 2");
		case Keycode::NUMPAD_3:
			return UI_TEXT("Numpad 3");
		case Keycode::NUMPAD_4:
			return UI_TEXT("Numpad 4");
		case Keycode::NUMPAD_5:
			return UI_TEXT("Numpad 5");
		case Keycode::NUMPAD_6:
			return UI_TEXT("Numpad 6");
		case Keycode::NUMPAD_7:
			return UI_TEXT("Numpad 7");
		case Keycode::NUMPAD_8:
			return UI_TEXT("Numpad 8");
		case Keycode::NUMPAD_9:
			return UI_TEXT("Numpad 9");
		case Keycode::NUMPAD_DIV:
			return UI_TEXT("Numpad /");
		case Keycode::NUMPAD_MULT:
			return UI_TEXT("Numpad *");
		case Keycode::NUMPAD_SUB:
			return UI_TEXT("Numpad -");
		case Keycode::NUMPAD_ADD:
			return UI_TEXT("Numpad +");
		case Keycode::NUMPAD_DOT:
			return UI_TEXT("Numpad .");
		case Keycode::NUMPAD_COMMA:
			return UI_TEXT("Numpad ,");
		case Keycode::NUMPAD_ENTER:
			return UI_TEXT("Numpad Enter");
		case Keycode::NUMPAD_EQUALS:
			return UI_TEXT("Numpad =");
		#if CONFIG_PACKAGE_X11 || defined __ANDROID__
		case Keycode::AT: return "@";
		case Keycode::STAR: return "*";
		case Keycode::PLUS: return "+";
		case Keycode::POUND: return "#";
		case Keycode::LEFT_PAREN: return "(";
		case Keycode::RIGHT_PAREN: return ")";
		case Keycode::SEARCH: return "Search";
		case Keycode::CLEAR: return "Clear";
		case Keycode::EXPLORER: return "Explorer";
		case Keycode::MAIL: return "Mail";
		case Keycode::VOL_UP: return "Vol Up";
		case Keycode::VOL_DOWN: return "Vol Down";
		#endif
		#if CONFIG_PACKAGE_X11
		case Keycode::NUMPAD_INSERT: return "Numpad Insert";
		case Keycode::NUMPAD_DELETE: return "Numpad Delete";
		case Keycode::NUMPAD_BEGIN: return "Numpad Begin";
		case Keycode::NUMPAD_HOME: return "Numpad Home";
		case Keycode::NUMPAD_END: return "Numpad End";
		case Keycode::NUMPAD_PGUP: return "Numpad Page Up";
		case Keycode::NUMPAD_PGDOWN: return "Numpad Page Down";
		case Keycode::NUMPAD_UP: return "Numpad Up";
		case Keycode::NUMPAD_RIGHT: return "Numpad Right";
		case Keycode::NUMPAD_DOWN: return "Numpad Down";
		case Keycode::NUMPAD_LEFT: return "Numpad Left";
		case Keycode::LMETA: return "Left Meta";
		case Keycode::RMETA: return "Right Meta";
		#endif
		case Keycode::F1:
			return UI_TEXT("F1");
		case Keycode::F2:
			return UI_TEXT("F2");
		case Keycode::F3:
			return UI_TEXT("F3");
		case Keycode::F4:
			return UI_TEXT("F4");
		case Keycode::F5:
			return UI_TEXT("F5");
		case Keycode::F6:
			return UI_TEXT("F6");
		case Keycode::F7:
			return UI_TEXT("F7");
		case Keycode::F8:
			return UI_TEXT("F8");
		case Keycode::F9:
			return UI_TEXT("F9");
		case Keycode::F10:
			return UI_TEXT("F10");
		case Keycode::F11:
			return UI_TEXT("F11");
		case Keycode::F12:
			return UI_TEXT("F12");
		case Keycode::GAME_A:
			return UI_TEXT("A");
		case Keycode::GAME_B:
			return UI_TEXT("B");
		case Keycode::GAME_C:
			return UI_TEXT("C");
		case Keycode::GAME_X:
			return UI_TEXT("X");
		case Keycode::GAME_Y:
			return UI_TEXT("Y");
		case Keycode::GAME_Z:
			return UI_TEXT("Z");
		case Keycode::GAME_L1:
			return UI_TEXT("L1");
		case Keycode::GAME_R1:
			return UI_TEXT("R1");
		case Keycode::GAME_L2:
			return UI_TEXT("L2");
		case Keycode::GAME_R2:
			return UI_TEXT("R2");
		case Keycode::GAME_LEFT_THUMB:
			return UI_TEXT("L-Thumb");
		case Keycode::GAME_RIGHT_THUMB:
			return UI_TEXT("R-Thumb");
		case Keycode::GAME_START:
			return UI_TEXT("Start");
		case Keycode::GAME_SELECT:
			return UI_TEXT("Select");
		case Keycode::GAME_MODE:
			return UI_TEXT("Mode");
		case Keycode::GAME_1: return "G1";
		case Keycode::GAME_2: return "G2";
		case Keycode::GAME_3: return "G3";
		case Keycode::GAME_4: return "G4";
		case Keycode::GAME_5: return "G5";
		case Keycode::GAME_6: return "G6";
		case Keycode::GAME_7: return "G7";
		case Keycode::GAME_8: return "G8";
		case Keycode::GAME_9: return "G9";
		case Keycode::GAME_10: return "G10";
		case Keycode::GAME_11: return "G11";
		case Keycode::GAME_12: return "G12";
		case Keycode::GAME_13: return "G13";
		case Keycode::GAME_14: return "G14";
		case Keycode::GAME_15: return "G15";
		case Keycode::GAME_16: return "G16";
		case Keycode::JS1_XAXIS_POS: return "X Axis+";
		case Keycode::JS1_XAXIS_NEG: return "X Axis-";
		case Keycode::JS1_YAXIS_POS: return "Y Axis+";
		case Keycode::JS1_YAXIS_NEG: return "Y Axis-";
		case Keycode::JS2_XAXIS_POS: return "X Axis+ 2";
		case Keycode::JS2_XAXIS_NEG: return "X Axis- 2";
		case Keycode::JS2_YAXIS_POS: return "Y Axis+ 2";
		case Keycode::JS2_YAXIS_NEG: return "Y Axis- 2";
		case Keycode::JS3_XAXIS_POS: return "X Axis+ 3";
		case Keycode::JS3_XAXIS_NEG: return "X Axis- 3";
		case Keycode::JS3_YAXIS_POS: return "Y Axis+ 3";
		case Keycode::JS3_YAXIS_NEG: return "Y Axis- 3";
		case Keycode::JS_POV_XAXIS_POS: return "POV Right";
		case Keycode::JS_POV_XAXIS_NEG: return "POV Left";
		case Keycode::JS_POV_YAXIS_POS: return "POV Down";
		case Keycode::JS_POV_YAXIS_NEG: return "POV Up";
		case Keycode::JS_RUDDER_AXIS_POS: return "Rudder Right";
		case Keycode::JS_RUDDER_AXIS_NEG: return "Rudder Left";
		case Keycode::JS_WHEEL_AXIS_POS: return "Wheel Right";
		case Keycode::JS_WHEEL_AXIS_NEG: return "Wheel Left";
		case Keycode::JS_LTRIGGER_AXIS:
			return UI_TEXT("L Trigger");
		case Keycode::JS_RTRIGGER_AXIS:
			return UI_TEXT("R Trigger");
		case Keycode::JS_GAS_AXIS:
			return UI_TEXT("Gas");
		case Keycode::JS_BRAKE_AXIS:
			return UI_TEXT("Brake");
		// Android-specific
		#ifdef __ANDROID__
		case Keycode::SYMBOL: return "Sym";
		case Keycode::NUM: return "Num";
		case Keycode::FUNCTION: return "Function";
		case Keycode::CENTER: return "Center";
		case Keycode::CAMERA: return "Camera";
		case Keycode::CALL: return "Call";
		case Keycode::END_CALL: return "End Call";
		case Keycode::FOCUS: return "Focus";
		case Keycode::HEADSET_HOOK: return "Headset Hook";
		case Keycode::MEDIA_PLAY_PAUSE: return "Play";
		case Keycode::MEDIA_STOP: return "Stop";
		case Keycode::MEDIA_NEXT: return "Next";
		case Keycode::MEDIA_PREVIOUS: return "Previous";
		case Keycode::MEDIA_REWIND: return "Rewind";
		case Keycode::MEDIA_FAST_FORWARD: return "Fast Forward";
		#endif
	}
	return "";
}

static const char *ps3SysButtonName(Key b)
{
	#if defined __ANDROID__
	switch(b)
	{
		case Keycode::PS3::CROSS: return "Cross";
		case Keycode::PS3::CIRCLE: return "Circle";
		case Keycode::PS3::SQUARE: return "Square";
		case Keycode::PS3::TRIANGLE: return "Triangle";
		case Keycode::PS3::PS: return "PS";
		case Keycode::GAME_LEFT_THUMB: return "L3";
		case Keycode::GAME_RIGHT_THUMB: return "R3";
	}
	return nullptr;
	#else
	return nullptr;
	#endif
}

#ifdef __ANDROID__
static const char *xperiaPlayButtonName(Key b)
{
	switch(b)
	{
		case Keycode::XperiaPlay::CROSS: return "Cross";
		case Keycode::XperiaPlay::CIRCLE: return "Circle";
		case Keycode::XperiaPlay::SQUARE: return "Square";
		case Keycode::XperiaPlay::TRIANGLE: return "Triangle";
	}
	return nullptr;
}

static const char *ouyaButtonName(Key b)
{
	switch(b)
	{
		case Keycode::Ouya::O: return "O";
		case Keycode::Ouya::U: return "U";
		case Keycode::Ouya::Y: return "Y";
		case Keycode::Ouya::A: return "A";
		case Keycode::Ouya::L3: return "L3";
		case Keycode::Ouya::R3: return "R3";
		case Keycode::MENU: return "System";
	}
	return nullptr;
}
#endif

#ifdef CONFIG_MACHINE_PANDORA
static const char *openPandoraButtonName(Key b)
{
	switch(b)
	{
		case Keycode::Pandora::L: return "L";
		case Keycode::Pandora::R: return "R";
		case Keycode::Pandora::A: return "A";
		case Keycode::Pandora::B: return "B";
		case Keycode::Pandora::Y: return "Y";
		case Keycode::Pandora::X: return "X";
		case Keycode::Pandora::SELECT: return "Select";
		case Keycode::Pandora::START: return "Start";
		case Keycode::Pandora::LOGO: return "Logo";
	}
	return nullptr;
}
#endif

BaseDevice::BaseDevice(int id, Map map, DeviceTypeFlags typeFlags, std::string name):
	name_{std::move(name)}, id_{id}, typeFlags_{typeFlags}, map_{map} {}

void Device::setICadeMode(bool on)
{
	visit([&](auto &d)
	{
		if constexpr(requires {d.setICadeMode(on);})
			d.setICadeMode(on);
	});
}

bool Device::iCadeMode() const
{
	return visit([](auto &d)
	{
		if constexpr(requires {d.iCadeMode();})
			return d.iCadeMode();
		else
			return false;
	});
}

void Device::setJoystickAxesAsKeys(AxisSetId id, bool on)
{
	if(auto axis1 = motionAxis(toAxisIds(id).first))
		axis1->setEmulatesKeys(map(), on);
	if(auto axis2 = motionAxis(toAxisIds(id).second))
		axis2->setEmulatesKeys(map(), on);
}

bool Device::joystickAxesAsKeys(AxisSetId id)
{
	if(auto axis2 = motionAxis(toAxisIds(id).second))
		return axis2->emulatesKeys();
	return false;
}

Axis *Device::motionAxis(AxisId id)
{
	auto axes = motionAxes();
	auto it = std::ranges::find_if(axes, [&](auto &axis){ return axis.id() == id; });
	if(it == std::end(axes))
		return nullptr;
	return &(*it);
}

const char *Device::keyName(Key k) const
{
	auto customName = visit([&](auto &d) -> const char *
	{
		if constexpr(requires {d.keyName(k);})
			return d.keyName(k);
		else
			return nullptr;
	});
	if(customName)
		return customName;
	switch(map())
	{
		default: return "";
		case Map::SYSTEM:
		{
			auto subtypeButtonName = [](Subtype subtype, Key k) -> const char *
				{
					switch(subtype)
					{
						#ifdef __ANDROID__
						case Subtype::XPERIA_PLAY: return xperiaPlayButtonName(k);
						case Subtype::OUYA_CONTROLLER: return ouyaButtonName(k);
						case Subtype::PS3_CONTROLLER: return ps3SysButtonName(k);
						#endif
						#ifdef CONFIG_MACHINE_PANDORA
						case Subtype::PANDORA_HANDHELD: return openPandoraButtonName(k);
						#endif
						default: return {};
					}
				};
			const char *name = subtypeButtonName(subtype(), k);
			if(!name)
				return keyButtonName(k);
			return name;
		}
	}
}

std::string Device::keyString(Key k, KeyNameFlags flags) const
{
	if(auto name = keyName(k);
		std::string_view{name}.size())
	{
		if(flags.basicModifiers)
		{
			using namespace Keycode;
			switch(k)
			{
				case LALT:
				case RALT:
					return UI_TEXT("Alt");
				case LSHIFT:
				case RSHIFT:
					return UI_TEXT("Shift");
				case LCTRL:
				case RCTRL:
					return UI_TEXT("Ctrl");
			}
		}
		return std::string{name};
	}
	else
	{
		return std::format(UI_TEXT("Key Code {:#X}"), k);
	}
}

Map Device::map() const
{
	return visit([](auto &d){ return d.map_; });
}

static DeviceSubtype gamepadSubtype(std::string_view name)
{
	if(name == "Sony PLAYSTATION(R)3 Controller")
	{
		log.info("detected PS3 gamepad");
		return DeviceSubtype::PS3_CONTROLLER;
	}
	else if(name == "OUYA Game Controller")
	{
		log.info("detected OUYA gamepad");
		return DeviceSubtype::OUYA_CONTROLLER;
	}
	return {};
}

static std::string_view gamepadName(uint32_t vendorProductId)
{
	if(vendorProductId == 0x054c05c4) // DualShock 4
	{
		log.info("detected DualShock 4 gamepad");
		return "DualShock 4";
	}
	return {};
}

void BaseDevice::updateGamepadSubtype(std::string_view name, uint32_t vendorProductId)
{
	if(auto updatedSubtype = gamepadSubtype(name);
		updatedSubtype != DeviceSubtype::NONE)
	{
		subtype_ = updatedSubtype;
	}
	if(auto updatedName = gamepadName(vendorProductId);
		updatedName.size())
	{
		name_ = updatedName;
	}
}

}
