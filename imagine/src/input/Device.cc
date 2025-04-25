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

constexpr auto keyButtonName(Key b)
{
	switch(b)
	{
		case 0:
			return UI_TEXT("无");
		case Keycode::SPACE:
			return UI_TEXT("空格键");
		case Keycode::A:
			return UI_TEXT("a 键");
		case Keycode::B:
			return UI_TEXT("b 键");
		case Keycode::C:
			return UI_TEXT("c 键");
		case Keycode::D:
			return UI_TEXT("d 键");
		case Keycode::E:
			return UI_TEXT("e 键");
		case Keycode::F:
			return UI_TEXT("f 键");
		case Keycode::G:
			return UI_TEXT("g 键");
		case Keycode::H:
			return UI_TEXT("h 键");
		case Keycode::I:
			return UI_TEXT("i 键");
		case Keycode::J:
			return UI_TEXT("j 键");
		case Keycode::K:
			return UI_TEXT("k 键");
		case Keycode::L:
			return UI_TEXT("l 键");
		case Keycode::M:
			return UI_TEXT("m 键");
		case Keycode::N:
			return UI_TEXT("n 键");
		case Keycode::O:
			return UI_TEXT("o 键");
		case Keycode::P:
			return UI_TEXT("p 键");
		case Keycode::Q:
			return UI_TEXT("q 键");
		case Keycode::R:
			return UI_TEXT("r 键");
		case Keycode::S:
			return UI_TEXT("s 键");
		case Keycode::T:
			return UI_TEXT("t 键");
		case Keycode::U:
			return UI_TEXT("u 键");
		case Keycode::V:
			return UI_TEXT("v 键");
		case Keycode::W:
			return UI_TEXT("w 键");
		case Keycode::X:
			return UI_TEXT("x 键");
		case Keycode::Y:
			return UI_TEXT("y 键");
		case Keycode::Z:
			return UI_TEXT("z 键");
		case Keycode::APOSTROPHE:
			return UI_TEXT("' 键 (单引号)");
		case Keycode::COMMA:
			return UI_TEXT(", 键");
		case Keycode::MINUS:
			return UI_TEXT("- 键");
		case Keycode::PERIOD:
			return UI_TEXT(". 键");
		case Keycode::SLASH:
			return UI_TEXT("/ 键");
		case Keycode::_0:
			return UI_TEXT("0 键");
		case Keycode::_1:
			return UI_TEXT("1 键");
		case Keycode::_2:
			return UI_TEXT("2 键");
		case Keycode::_3:
			return UI_TEXT("3 键");
		case Keycode::_4:
			return UI_TEXT("4 键");
		case Keycode::_5:
			return UI_TEXT("5 键");
		case Keycode::_6:
			return UI_TEXT("6 键");
		case Keycode::_7:
			return UI_TEXT("7 键");
		case Keycode::_8:
			return UI_TEXT("8 键");
		case Keycode::_9:
			return UI_TEXT("9 键");
		case Keycode::SEMICOLON:
			return UI_TEXT("; 键");
		case Keycode::EQUALS:
			return UI_TEXT("= 键");
		case Keycode::LEFT_BRACKET:
			return UI_TEXT("[ 键");
		case Keycode::BACKSLASH:
			return UI_TEXT("\\ 键");
		case Keycode::RIGHT_BRACKET:
			return UI_TEXT("] 键");
		case Keycode::GRAVE:
			return UI_TEXT("` 键 (反引号)");
		case Keycode::BACK:
			return UI_TEXT("返回键");
		case Keycode::ESCAPE:
			return UI_TEXT("Esc 键");
		case Keycode::ENTER:
			return UI_TEXT("回车键");
		case Keycode::LALT:
			return UI_TEXT("左 Alt 键");
		case Keycode::RALT:
			return UI_TEXT("右 Alt 键");
		case Keycode::LSHIFT:
			return UI_TEXT("左 Shift 键");
		case Keycode::RSHIFT:
			return UI_TEXT("右 Shift 键");
		case Keycode::LCTRL:
			return UI_TEXT("左 Ctrl 键");
		case Keycode::RCTRL:
			return UI_TEXT("右 Ctrl 键");
		case Keycode::UP:
			return UI_TEXT("方向键的上");
		case Keycode::RIGHT:
			return UI_TEXT("方向键的右");
		case Keycode::DOWN:
			return UI_TEXT("方向键的下");
		case Keycode::LEFT:;
			return UI_TEXT("方向键的左");
		case Keycode::BACK_SPACE:
			return UI_TEXT("退格键");
		case Keycode::MENU:
			return UI_TEXT("菜单键");
		case Keycode::HOME:
			return UI_TEXT("Home 键");
		case Keycode::END:
			return UI_TEXT("End 键");
		case Keycode::INSERT:
			return UI_TEXT("插入键");
		case Keycode::DELETE:
			return UI_TEXT("删除键");
		case Keycode::TAB:
			return UI_TEXT("Tab 键");
		case Keycode::SCROLL_LOCK:
			return UI_TEXT("滑动锁定键");
		case Keycode::CAPS:
			return UI_TEXT("大写切换键");
		case Keycode::PAUSE:
			return UI_TEXT("暂停键");
		case Keycode::LSUPER:
			return UI_TEXT("左边的开始/选项键");
		case Keycode::RSUPER:
			return UI_TEXT("右边的开始/选项键");
		case Keycode::PGUP:
			return UI_TEXT("向上翻页键");
		case Keycode::PGDOWN:
			return UI_TEXT("向下翻页键");
		case Keycode::PRINT_SCREEN:
			return UI_TEXT("截屏键");
		case Keycode::NUM_LOCK:
			return UI_TEXT("数字锁定键");
		case Keycode::NUMPAD_0:
			return UI_TEXT("0 键 (数字键盘)");
		case Keycode::NUMPAD_1:
			return UI_TEXT("1 键 (数字键盘)");
		case Keycode::NUMPAD_2:
			return UI_TEXT("2 键 (数字键盘)");
		case Keycode::NUMPAD_3:
			return UI_TEXT("3 键 (数字键盘)");
		case Keycode::NUMPAD_4:
			return UI_TEXT("4 键 (数字键盘)");
		case Keycode::NUMPAD_5:
			return UI_TEXT("5 键 (数字键盘)");
		case Keycode::NUMPAD_6:
			return UI_TEXT("6 键 (数字键盘)");
		case Keycode::NUMPAD_7:
			return UI_TEXT("7 键 (数字键盘)");
		case Keycode::NUMPAD_8:
			return UI_TEXT("8 键 (数字键盘)");
		case Keycode::NUMPAD_9:
			return UI_TEXT("9 键 (数字键盘)");
		case Keycode::NUMPAD_DIV:
			return UI_TEXT("/ 键 (数字键盘)");
		case Keycode::NUMPAD_MULT:
			return UI_TEXT("* 键 (数字键盘)");
		case Keycode::NUMPAD_SUB:
			return UI_TEXT("- 键 (数字键盘)");
		case Keycode::NUMPAD_ADD:
			return UI_TEXT("+ 键 (数字键盘)");
		case Keycode::NUMPAD_DOT:
			return UI_TEXT(". 键 (数字键盘)");
		case Keycode::NUMPAD_COMMA:
			return UI_TEXT(", 键 (数字键盘)");
		case Keycode::NUMPAD_ENTER:
			return UI_TEXT("回车键 (数字键盘)");
		case Keycode::NUMPAD_EQUALS:
			return UI_TEXT("= 键 (数字键盘)");
		#if CONFIG_PACKAGE_X11 || defined __ANDROID__
		case Keycode::AT:
			return UI_TEXT("@ 键");
		case Keycode::STAR:
			return UI_TEXT("* 键");
		case Keycode::PLUS:
			return UI_TEXT("+ 键");
		case Keycode::POUND:
			return UI_TEXT("# 键");
		case Keycode::LEFT_PAREN:
			return UI_TEXT("( 键");
		case Keycode::RIGHT_PAREN:
			return UI_TEXT(") 键");
		case Keycode::SEARCH:
			return UI_TEXT("搜索键");
		case Keycode::CLEAR:
			return UI_TEXT("清除键");
		case Keycode::EXPLORER:
			return UI_TEXT("Explorer 键");
		case Keycode::MAIL:
			return UI_TEXT("Mail 键");
		case Keycode::VOL_UP:
			return UI_TEXT("提高音量键");
		case Keycode::VOL_DOWN:
			return UI_TEXT("降低音量键");
		#endif
		#if CONFIG_PACKAGE_X11
		case Keycode::NUMPAD_INSERT:
			return UI_TEXT("插入键 (数字键盘)");
		case Keycode::NUMPAD_DELETE:
			return UI_TEXT("删除键 (数字键盘)");
		case Keycode::NUMPAD_BEGIN:
			return UI_TEXT("Begin 键 (数字键盘)");
		case Keycode::NUMPAD_HOME:
			return UI_TEXT("Home 键 (数字键盘)");
		case Keycode::NUMPAD_END:
			return UI_TEXT("End 键 (数字键盘)");
		case Keycode::NUMPAD_PGUP:
			return UI_TEXT("向上翻页键 (数字键盘)");
		case Keycode::NUMPAD_PGDOWN:
			return UI_TEXT("向下翻页键 (数字键盘)");
		case Keycode::NUMPAD_UP:
			return UI_TEXT("方向键的上 (数字键盘)");
		case Keycode::NUMPAD_RIGHT:
			return UI_TEXT("方向键的右 (数字键盘)");
		case Keycode::NUMPAD_DOWN:
			return UI_TEXT("方向键的下 (数字键盘)");
		case Keycode::NUMPAD_LEFT:
			return UI_TEXT("方向键的左 (数字键盘)");
		case Keycode::LMETA:
			return UI_TEXT("左 Meta 键");
		case Keycode::RMETA:
			return UI_TEXT("右 Meta 键");
		#endif
		case Keycode::F1:
			return UI_TEXT("F1 键");
		case Keycode::F2:
			return UI_TEXT("F2 键");
		case Keycode::F3:
			return UI_TEXT("F3 键");
		case Keycode::F4:
			return UI_TEXT("F4 键");
		case Keycode::F5:
			return UI_TEXT("F5 键");
		case Keycode::F6:
			return UI_TEXT("F6 键");
		case Keycode::F7:
			return UI_TEXT("F7 键");
		case Keycode::F8:
			return UI_TEXT("F8 键");
		case Keycode::F9:
			return UI_TEXT("F9 键");
		case Keycode::F10:
			return UI_TEXT("F10 键");
		case Keycode::F11:
			return UI_TEXT("F11 键");
		case Keycode::F12:
			return UI_TEXT("F12 键");
		case Keycode::GAME_A:
			return UI_TEXT("A 键");
		case Keycode::GAME_B:
			return UI_TEXT("B 键");
		case Keycode::GAME_C:
			return UI_TEXT("C 键");
		case Keycode::GAME_X:
			return UI_TEXT("X 键");
		case Keycode::GAME_Y:
			return UI_TEXT("Y 键");
		case Keycode::GAME_Z:
			return UI_TEXT("Z 键");
		case Keycode::GAME_L1:
			return UI_TEXT("L1 键");
		case Keycode::GAME_R1:
			return UI_TEXT("R1 键");
		case Keycode::GAME_L2:
			return UI_TEXT("L2 键");
		case Keycode::GAME_R2:
			return UI_TEXT("R2 键");
		case Keycode::GAME_LEFT_THUMB:
			return UI_TEXT("L-Thumb 键");
		case Keycode::GAME_RIGHT_THUMB:
			return UI_TEXT("R-Thumb 键");
		case Keycode::GAME_START:
			return UI_TEXT("开始键");
		case Keycode::GAME_SELECT:
			return UI_TEXT("选择键");
		case Keycode::GAME_MODE:
			return UI_TEXT("模式键");
		case Keycode::GAME_1:
			return UI_TEXT("G1 键");
		case Keycode::GAME_2:
			return UI_TEXT("G2 键");
		case Keycode::GAME_3:
			return UI_TEXT("G3 键");
		case Keycode::GAME_4:
			return UI_TEXT("G4 键");
		case Keycode::GAME_5:
			return UI_TEXT("G5 键");
		case Keycode::GAME_6:
			return UI_TEXT("G6 键");
		case Keycode::GAME_7:
			return UI_TEXT("G7 键");
		case Keycode::GAME_8:
			return UI_TEXT("G8 键");
		case Keycode::GAME_9:
			return UI_TEXT("G9 键");
		case Keycode::GAME_10:
			return UI_TEXT("G10 键");
		case Keycode::GAME_11:
			return UI_TEXT("G11 键");
		case Keycode::GAME_12:
			return UI_TEXT("G12 键");
		case Keycode::GAME_13:
			return UI_TEXT("G13 键");
		case Keycode::GAME_14:
			return UI_TEXT("G14 键");
		case Keycode::GAME_15:
			return UI_TEXT("G15 键");
		case Keycode::GAME_16:
			return UI_TEXT("G16 键");
		case Keycode::JS1_XAXIS_POS:
			return UI_TEXT("左摇杆的右");
		case Keycode::JS1_XAXIS_NEG:
			return UI_TEXT("左摇杆的左");
		case Keycode::JS1_YAXIS_POS:
			return UI_TEXT("左摇杆的下");
		case Keycode::JS1_YAXIS_NEG:
			return UI_TEXT("左摇杆的上");
		case Keycode::JS2_XAXIS_POS:
			return UI_TEXT("右摇杆的右");
		case Keycode::JS2_XAXIS_NEG:
			return UI_TEXT("右摇杆的左");
		case Keycode::JS2_YAXIS_POS:
			return UI_TEXT("右摇杆的下");
		case Keycode::JS2_YAXIS_NEG:
			return UI_TEXT("右摇杆的上");
		case Keycode::JS3_XAXIS_POS:
			return UI_TEXT("X Axis+ 3");
		case Keycode::JS3_XAXIS_NEG:
			return UI_TEXT("X Axis- 3");
		case Keycode::JS3_YAXIS_POS:
			return UI_TEXT("Y Axis+ 3");
		case Keycode::JS3_YAXIS_NEG:
			return UI_TEXT("Y Axis- 3");
		case Keycode::JS_POV_XAXIS_POS:
			return UI_TEXT("苦力帽的右");
		case Keycode::JS_POV_XAXIS_NEG:
			return UI_TEXT("苦力帽的左");
		case Keycode::JS_POV_YAXIS_POS:
			return UI_TEXT("苦力帽的下");
		case Keycode::JS_POV_YAXIS_NEG:
			return UI_TEXT("苦力帽的上");
		case Keycode::JS_RUDDER_AXIS_POS:
			return UI_TEXT("方向舵的右");
		case Keycode::JS_RUDDER_AXIS_NEG:
			return UI_TEXT("方向舵的左");
		case Keycode::JS_WHEEL_AXIS_POS:
			return UI_TEXT("滚轮右拨");
		case Keycode::JS_WHEEL_AXIS_NEG:
			return UI_TEXT("滚轮左拨");
		case Keycode::JS_LTRIGGER_AXIS:
			return UI_TEXT("左扳机");
		case Keycode::JS_RTRIGGER_AXIS:
			return UI_TEXT("右扳机");
		case Keycode::JS_GAS_AXIS:
			return UI_TEXT("油门");
		case Keycode::JS_BRAKE_AXIS:
			return UI_TEXT("刹车");
		// Android-specific
		#ifdef __ANDROID__
		case Keycode::SYMBOL:
			return UI_TEXT("系统键");
		case Keycode::NUM:
			return UI_TEXT("数字锁定键");
		case Keycode::FUNCTION:
			return UI_TEXT("功能键");
		case Keycode::CENTER:
			return UI_TEXT("中央键");
		case Keycode::CAMERA:
			return UI_TEXT("相机键");
		case Keycode::CALL:
			return UI_TEXT("呼叫键");
		case Keycode::END_CALL:
			return UI_TEXT("结束呼叫键");
		case Keycode::FOCUS:
			return UI_TEXT("对焦键");
		case Keycode::HEADSET_HOOK:
			return UI_TEXT("插入耳机");
		case Keycode::MEDIA_PLAY_PAUSE:
			return UI_TEXT("播放键");
		case Keycode::MEDIA_STOP:
			return UI_TEXT("停止键");
		case Keycode::MEDIA_NEXT:
			return UI_TEXT("下一个文件");
		case Keycode::MEDIA_PREVIOUS:
			return UI_TEXT("上一个文件");
		case Keycode::MEDIA_REWIND:
			return UI_TEXT("倒带键");
		case Keycode::MEDIA_FAST_FORWARD:
			return UI_TEXT("快进键");
		#endif
	}
	return "";
}

constexpr const char* ps3SysButtonName([[maybe_unused]] Key b)
{
	#if defined __ANDROID__
	switch(b)
	{
		case Keycode::PS3::CROSS:
			return UI_TEXT("交叉键");
		case Keycode::PS3::CIRCLE:
			return UI_TEXT("圆圈键");
		case Keycode::PS3::SQUARE:
			return UI_TEXT("正方键");
		case Keycode::PS3::TRIANGLE:
			 return UI_TEXT("三角键");
		case Keycode::PS3::PS:
			return UI_TEXT("PS 键");
		case Keycode::GAME_LEFT_THUMB:
			return UI_TEXT("L3 键");
		case Keycode::GAME_RIGHT_THUMB:
			return UI_TEXT("R3 键");
	}
	return nullptr;
	#else
	return nullptr;
	#endif
}

#ifdef __ANDROID__
constexpr const char* xperiaPlayButtonName(Key b)
{
	switch(b)
	{
		case Keycode::XperiaPlay::CROSS:
			return UI_TEXT("交叉键");
		case Keycode::XperiaPlay::CIRCLE:
			return UI_TEXT("圆圈键");
		case Keycode::XperiaPlay::SQUARE:
			return UI_TEXT("正方键");
		case Keycode::XperiaPlay::TRIANGLE:
			return UI_TEXT("三角键");
	}
	return nullptr;
}

constexpr const char* ouyaButtonName(Key b)
{
	switch(b)
	{
		case Keycode::Ouya::O:
			return UI_TEXT("O 键");
		case Keycode::Ouya::U:
			return UI_TEXT("U 键");
		case Keycode::Ouya::Y:
			return UI_TEXT("Y 键");
		case Keycode::Ouya::A:
			return UI_TEXT("A 键");
		case Keycode::Ouya::L3:
			return UI_TEXT("L3 键");
		case Keycode::Ouya::R3:
			return UI_TEXT("R3 键");
		case Keycode::MENU:
			return UI_TEXT("系统键");
	}
	return nullptr;
}
#endif

#ifdef CONFIG_MACHINE_PANDORA
constexpr const char* openPandoraButtonName(Key b)
{
	switch(b)
	{
		case Keycode::Pandora::L:
			return UI_TEXT("L 键");
		case Keycode::Pandora::R:
			return UI_TEXT("R 键");
		case Keycode::Pandora::A:
			return UI_TEXT("A 键");
		case Keycode::Pandora::B:
			return UI_TEXT("B 键");
		case Keycode::Pandora::Y:
			return UI_TEXT("Y 键");
		case Keycode::Pandora::X:
			return UI_TEXT("X 键");
		case Keycode::Pandora::SELECT:
			return UI_TEXT("选择键");
		case Keycode::Pandora::START:
			return UI_TEXT("开始键");
		case Keycode::Pandora::LOGO:
			return UI_TEXT("Logo 键");
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
			auto subtypeButtonName = [](Subtype subtype, [[maybe_unused]] Key k) -> const char *
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
					return UI_TEXT("Alt 键");
				case LSHIFT:
				case RSHIFT:
					return UI_TEXT("Shift 键");
				case LCTRL:
				case RCTRL:
					return UI_TEXT("Ctrl 键");
			}
		}
		return std::string{name};
	}
	else
	{
		return std::format(
			UI_TEXT("按键码 {:#X}"),
			k);
	}
}

std::string Device::displayName() const
{
	return makeDisplayName(name(), enumId());
}

std::string Device::makeDisplayName(std::string_view name, int id)
{
	if(id)
	{
		return std::format(
			UI_TEXT("{} #{}"),
			name, id + 1);
	}
	else
	{
		return std::string{name};
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
