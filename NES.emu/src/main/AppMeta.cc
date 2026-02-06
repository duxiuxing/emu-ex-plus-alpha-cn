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

#include <emuframework/macros.h>
import system;
import emuex;
import imagine;
import std;
#include <emuframework/EmuAppInlines.hh>

namespace EmuEx
{

const std::string_view AppMeta::creditsViewStr
{
	UI_TEXT(CREDITS_INFO_STRING "(c) 2011-2026\nRobert Broglia 保留所有权利\nwww.explusalpha.com\n\n部分代码版权属于\nFCEUX Team\nfceux.com\n\n中文翻译：R-Sam@github\nduxiuxing/emu-ex-plus-alpha-cn")
};
const std::string_view AppMeta::configFilename{"NesEmu.config"};
const bool AppMeta::hasCheats{true};
const bool AppMeta::hasPALVideoSystem{true};
const bool AppMeta::hasResetModes{true};
const bool AppMeta::hasRectangularPixels{true};
const int AppMeta::maxPlayers{4};
const bool AppMeta::needsGlobalInstance{true};
const NameFilterFunc AppMeta::defaultFsFilter{hasNESExtension};
constexpr BundledGameInfo gameInfo
{
	UI_TEXT("俄罗斯方块"),
	UI_TEXT("俄罗斯方块.7z")
};
const std::span<const BundledGameInfo> AppMeta::bundledGameInfo{ &gameInfo, 1 };

constexpr auto dpadKeyInfo = makeArray<KeyInfo>
(
	NesKey::Up,
	NesKey::Right,
	NesKey::Down,
	NesKey::Left
);

constexpr auto centerKeyInfo = makeArray<KeyInfo>
(
	NesKey::Select,
	NesKey::Start
);

constexpr std::array p2StartKeyInfo
{
	KeyInfo{NesKey::Start, {.deviceId = 1}}
};

constexpr auto faceKeyInfo = makeArray<KeyInfo>
(
	NesKey::B,
	NesKey::A
);

constexpr auto turboFaceKeyInfo = turbo(faceKeyInfo);

constexpr std::array comboKeyInfo{KeyInfo{std::array{NesKey::A, NesKey::B}}};

constexpr auto exKeyInfo = makeArray<KeyInfo>
(
	NesKey::toggleDiskSide
);

constexpr auto gpKeyInfo = concatToArrayNow<dpadKeyInfo, centerKeyInfo, faceKeyInfo, turboFaceKeyInfo, comboKeyInfo>;
constexpr auto gp2KeyInfo = transpose(gpKeyInfo, 1);
constexpr auto gp3KeyInfo = transpose(gpKeyInfo, 2);
constexpr auto gp4KeyInfo = transpose(gpKeyInfo, 3);

std::span<const KeyCategory> AppMeta::keyCategories()
{
	static constexpr std::array categories
	{
		KeyCategory
		{
			UI_TEXT("游戏按键"),
			gpKeyInfo
		},
		KeyCategory
		{
			UI_TEXT("游戏按键 (2号手柄)"),
			gp2KeyInfo, 1
		},
		KeyCategory
		{
			UI_TEXT("游戏按键 (3号手柄)"),
			gp3KeyInfo, 2
		},
		KeyCategory
		{
			UI_TEXT("游戏按键 (4号手柄)"),
			gp4KeyInfo, 3
		},
		KeyCategory
		{
			UI_TEXT("其他操作"),
			exKeyInfo
		},
	};
	return categories;
}

std::string_view AppMeta::systemKeyCodeToString(KeyCode c)
{
	switch(NesKey(c))
	{
		case NesKey::Up:
			return UI_TEXT("方向键的上");
		case NesKey::Right:
			return UI_TEXT("方向键的右");
		case NesKey::Down:
			return UI_TEXT("方向键的下");
		case NesKey::Left:
			return UI_TEXT("方向键的左");
		case NesKey::Select:
			return UI_TEXT("选择键");
		case NesKey::Start:
			return UI_TEXT("开始键");
		case NesKey::A:
			return UI_TEXT("A 键");
		case NesKey::B:
			return UI_TEXT("B 键");
		case NesKey::toggleDiskSide:
			return UI_TEXT("弹出磁碟/磁碟翻面");
		default: return "";
	}
}

std::span<const KeyConfigDesc> AppMeta::defaultKeyConfigs()
{
	using namespace Input;

	static constexpr std::array pcKeyboardMap
	{
		KeyMapping{NesKey::Up, Keycode::UP},
		KeyMapping{NesKey::Right, Keycode::RIGHT},
		KeyMapping{NesKey::Down, Keycode::DOWN},
		KeyMapping{NesKey::Left, Keycode::LEFT},
		KeyMapping{NesKey::Select, Keycode::SPACE},
		KeyMapping{NesKey::Start, Keycode::ENTER},
		KeyMapping{NesKey::B, Keycode::Z},
		KeyMapping{NesKey::A, Keycode::X},
	};

	static constexpr std::array genericGamepadMap
	{
		KeyMapping{NesKey::Up, Keycode::UP},
		KeyMapping{NesKey::Right, Keycode::RIGHT},
		KeyMapping{NesKey::Down, Keycode::DOWN},
		KeyMapping{NesKey::Left, Keycode::LEFT},
		KeyMapping{NesKey::Select, Keycode::GAME_SELECT},
		KeyMapping{NesKey::Start, Keycode::GAME_START},
		KeyMapping{NesKey::B, Keycode::GAME_X},
		KeyMapping{NesKey::A, Keycode::GAME_A},
	};

	static constexpr std::array wiimoteMap
	{
		KeyMapping{NesKey::Up, WiimoteKey::UP},
		KeyMapping{NesKey::Right, WiimoteKey::RIGHT},
		KeyMapping{NesKey::Down, WiimoteKey::DOWN},
		KeyMapping{NesKey::Left, WiimoteKey::LEFT},
		KeyMapping{NesKey::B, WiimoteKey::_1},
		KeyMapping{NesKey::A, WiimoteKey::_2},
		KeyMapping{NesKey::Select, WiimoteKey::MINUS},
		KeyMapping{NesKey::Start, WiimoteKey::PLUS},
	};

	return genericKeyConfigs<pcKeyboardMap, genericGamepadMap, wiimoteMap>();
}

bool AppMeta::allowsTurboModifier(KeyCode c)
{
	switch(NesKey(c))
	{
		case NesKey::A ... NesKey::B:
			return true;
		default: return false;
	}
}

constexpr FRect gpImageCoords(IRect cellRelBounds)
{
	constexpr F2Size imageSize{256, 256};
	constexpr int cellSize = 32;
	return (cellRelBounds.relToAbs() * cellSize).as<float>() / imageSize;
}

AssetDesc AppMeta::vControllerAssetDesc(KeyInfo key)
{
	static constexpr struct VirtualControllerAssets
	{
		AssetDesc dpad{AssetFileID::gamepadOverlay, gpImageCoords({{}, {4, 4}})},

		a{AssetFileID::gamepadOverlay,      gpImageCoords({{4, 0}, {2, 2}})},
		b{AssetFileID::gamepadOverlay,      gpImageCoords({{6, 0}, {2, 2}})},
		select{AssetFileID::gamepadOverlay, gpImageCoords({{4, 2}, {2, 1}}), {1, 2}},
		start{AssetFileID::gamepadOverlay,  gpImageCoords({{4, 3}, {2, 1}}), {1, 2}},
		ab{AssetFileID::gamepadOverlay,     gpImageCoords({{6, 2}, {2, 2}})},

		blank{AssetFileID::gamepadOverlay, gpImageCoords({{0, 4}, {2, 2}})};
	} virtualControllerAssets;

	if(key[0] == 0)
		return virtualControllerAssets.dpad;
	switch(NesKey(key[0]))
	{
		case NesKey::A: return NesKey(key[1]) == NesKey::B ? virtualControllerAssets.ab : virtualControllerAssets.a;
		case NesKey::B: return virtualControllerAssets.b;
		case NesKey::Select: return virtualControllerAssets.select;
		case NesKey::Start: return virtualControllerAssets.start;
		default: return virtualControllerAssets.blank;
	}
}

SystemInputDeviceDesc AppMeta::inputDeviceDesc(int idx)
{
	static constexpr std::array gamepadComponents
	{
		InputComponentDesc
		{
			UI_TEXT("方向键"),
			dpadKeyInfo, InputComponent::dPad, LB2DO
		},
		InputComponentDesc
		{
			UI_TEXT("动作键"),
			faceKeyInfo, InputComponent::button, RB2DO
		},
		InputComponentDesc
		{
			UI_TEXT("选择键"),
			{&centerKeyInfo[0], 1}, InputComponent::button, LB2DO
		},
		InputComponentDesc
		{
			UI_TEXT("开始键"),
			{&centerKeyInfo[1], 1}, InputComponent::button, RB2DO
		},
		InputComponentDesc
		{
			UI_TEXT("选择/开始键"),
			centerKeyInfo, InputComponent::button, CB2DO, {.altConfig = true}
		},
		InputComponentDesc
		{
			UI_TEXT("开始键 (2号玩家)"),
			p2StartKeyInfo, InputComponent::button, RB2DO, {.altConfig = true}
		},
	};
	static constexpr SystemInputDeviceDesc gamepadDesc
	{
		UI_TEXT("游戏按键"),
		gamepadComponents
	};
	return gamepadDesc;
}

void AppMeta::onCustomizeNavView(AppNavView& view)
{
	const Gfx::LGradientStopDesc navViewGrad[] =
	{
		{ .0, Gfx::PackedColor::format.build(1. * .4, 0., 0., 1.) },
		{ .3, Gfx::PackedColor::format.build(1. * .4, 0., 0., 1.) },
		{ .97, Gfx::PackedColor::format.build(.5 * .4, 0., 0., 1.) },
		{ 1., view.separatorColor() },
	};
	view.setBackgroundGradient(navViewGrad);
}

}
