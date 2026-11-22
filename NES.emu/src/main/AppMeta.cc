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
	CREDITS_INFO_STRING UI_TEXT("(c) 2011-2026\nRobert Broglia 保留所有权利\nwww.explusalpha.com\n\n中文翻译：R-Sam@github\nduxiuxing/emu-ex-plus-alpha-cn\n\n部分代码版权属于\nFCEUX 团队\nfceux.com")
};
const std::string_view AppMeta::configFilename{"NesEmu.config"};
const bool AppMeta::hasCheats{true};
const bool AppMeta::hasPALVideoSystem{true};
const bool AppMeta::hasResetModes{true};
const bool AppMeta::hasRectangularPixels{true};
const int AppMeta::maxPlayers{4};
const bool AppMeta::needsGlobalInstance{true};
const NameFilterFunc AppMeta::defaultFsFilter{hasNESExtension};

constexpr BundledGameInfo gameInfo[] =
{
	{
		UI_TEXT("B - 蝙蝠侠1(中)"), // Batman - The Video Game (T-Chi)
		"batmavidgamc.zip"
	},
	{
		UI_TEXT("B - 蝙蝠侠2 小丑归来(美)"), // Batman - Return of the Joker (USA)
		"batmaretjok.zip"
	},
	{
		UI_TEXT("B - 蝙蝠侠3 蝙蝠侠归来"), // Batman Returns (USA)
		"batmanreturns.zip"
	},
	{
		UI_TEXT("C - 超级马力欧兄弟2(美)"), // Super Mario Bros. 2 (USA) (Rev A)
		"smb2.zip"
	},
	{
		UI_TEXT("C - 超级马力欧兄弟3(中)"), // Super Mario Bros. 3 (T-Chi)
		"smb3c.zip"
	},
	{
		UI_TEXT("C - 成龙之龙(美)"), // Jackie Chan (USA)
		"jackiechan.zip"
	},
	{
		UI_TEXT("C - 赤色要塞(中)"), // Jackal (T-Chi, v1.1)
		"jackalc.zip"
	},
	{
		UI_TEXT("C - 赤影战士(中)"), // Kage (T-Chi, v2.0)
		"kagec.zip"
	},
	{
		UI_TEXT("C - 穿越火线(日)"), // Cross Fire (Japan)
		"crossfire.zip"
	},
	{
		UI_TEXT("E - 俄罗斯方块(美)"), // Tetris (USA)
		"tetris.zip"
	},
	{
		UI_TEXT("E - 恶魔城1(中)"), // Akumajou Dracula (T-Chi)
		"akumajoudraculac.zip"
	},
	{
		UI_TEXT("E - 恶魔城2 西蒙的冒险(美)"), // Castlevania II - Simon's Quest (USA)
		"castlii.zip"
	},
	{
		UI_TEXT("E - 恶魔城3 德古拉的诅咒(美)"), // Castlevania III - Dracula's Curse (USA)
		"castliii.zip"
	},
	{
		UI_TEXT("E - 恶魔王子 少年德古拉(英)"), // Kid Dracula (English)
		"kiddracula.zip"
	},
	{
		UI_TEXT("H - 魂斗罗1(中)"), // Contra (T-Chi)
		"contrac.zip"
	},
	{
		UI_TEXT("H - 魂斗罗2 超级魂斗罗(中)"), // Super Contra (T-Chi)
		"supercontrac.zip"
	},
	{
		UI_TEXT("H - 魂斗罗力量(中)"), // Contra Force (2nd option, T-Chi)
		"contraforcec.zip"
	},
	{
		UI_TEXT("H - 火之鸟凤凰篇 我王的冒险(英)"), // Hinotori Houou Hen - Gaou no Bouken (T-Eng)
		"hinothouhen.zip"
	},
	{
		UI_TEXT("J - 机械战警1(美)"), // RoboCop (USA)
		"robocop.zip"
	},
	{
		UI_TEXT("J - 机械战警2(美)"), // RoboCop 2 (USA)
		"robocop2.zip"
	},
	{
		UI_TEXT("J - 机械战警3(美)"), // RoboCop 3 (USA)
		"robocop3.zip"
	},
	{
		UI_TEXT("K - 空中魂斗罗(美)"), // S.C.A.T. - Special Cybernetic Attack Team (USA)
		"scat.zip"
	},
	{
		UI_TEXT("K - 空中魂斗罗 最终任务(英)"), // Final Mission (T-Eng)
		"finalmission.zip"
	},
	{
		UI_TEXT("K - 魁！！男塾 疾风一号生(英)"), // Sakigake!! Otoko Juku - Shippuu Ichi Gou Sei (T-Eng)
		"sakigake.zip"
	},
	{
		UI_TEXT("L - 绿色兵团(美)"), // Rush'n Attack (USA)
		"rushnattack.zip"
	},
	{
		UI_TEXT("L - 洛克人1(美)"), // Mega Man (USA)
		"megaman.zip"
	},
	{
		UI_TEXT("L - 洛克人2(美)"), // Mega Man 2 (USA)
		"megaman2.zip"
	},
	{
		UI_TEXT("L - 洛克人3(美)"), // Mega Man 3 (USA)
		"megaman3.zip"
	},
	{
		UI_TEXT("L - 洛克人4(美)"), // Mega Man 4 (USA) (Rev A)
		"megaman4.zip"
	},
	{
		UI_TEXT("L - 洛克人5(美)"), // Mega Man 5 (USA)
		"megaman5.zip"
	},
	{
		UI_TEXT("L - 洛克人6(美)"), // Mega Man 6 (USA)
		"megaman6.zip"
	},
	{
		UI_TEXT("M - 马戏团(日)"), // Circus Charlie (Japan)
		"circuscharlie.zip"
	},
	{
		UI_TEXT("M - 冒险岛1(美)"), // Adventure Island (USA)
		"adventureisland.zip"
	},
	{
		UI_TEXT("M - 冒险岛2(美)"), // Adventure Island II (USA)
		"advenislii.zip"
	},
	{
		UI_TEXT("M - 冒险岛3(美)"), // Adventure Island 3 (USA)
		"advenisl3.zip"
	},
	{
		UI_TEXT("M - 冒险岛4(英)"), // Adventure Island IV (T-Eng)
		"advenisliv.zip"
	},
	{
		UI_TEXT("M - 密特罗德初代(美)"), // Metroid (USA)
		"metroid.zip"
	},
	{
		UI_TEXT("M - 密特罗德 叛逆黎明(美)"), // Metroid - Rogue Dawn (USA)
		"metrorogdaw.zip"
	},
	{
		UI_TEXT("Q - Q版沙罗曼蛇(英)"), // Parodius Da! (T-Eng)
		"parodiusda.zip"
	},
	{
		UI_TEXT("R - 人间兵器(美)"), // Code Name - Viper (USA)
		"codenam.zip"
	},
	{
		UI_TEXT("R - 忍者龙剑传1(美)"), // Ninja Gaiden (USA)
		"ninjagaiden.zip"
	},
	{
		UI_TEXT("R - 忍者龙剑传2 暗黑的邪神剑(美)"), // Ninja Gaiden II - The Dark Sword of Chaos (USA)
		"ninjagaiii.zip"
	},
	{
		UI_TEXT("R - 忍者龙剑传3 黄泉的方舟(美)"), // Ninja Gaiden III - The Ancient Ship of Doom (USA)
		"ninjagaiiii.zip"
	},
	{
		UI_TEXT("R - 忍者神龟1 激龟忍者传(中)"), // Teenage Mutant Ninja Turtles (T-Chi, v1.0)
		"tmntc.zip"
	},
	{
		UI_TEXT("R - 忍者神龟2 街机移植版(美)"), // Teenage Mutant Ninja Turtles II - The Arcade Game (USA)
		"tmntiiarcgam.zip"
	},
	{
		UI_TEXT("R - 忍者神龟3 曼哈顿计划(美)"), // Teenage Mutant Ninja Turtles III - The Manhattan Project (USA)
		"tmntiii.zip"
	},
	{
		UI_TEXT("R - 忍者神龟格斗"), // Teenage Mutant Ninja Turtles Tournament Fighters (USA)
		"tmnttoufig.zip"
	},
	{
		UI_TEXT("S - SD快打旋风(美)"), // Mighty Final Fight (USA)
		"mightfinfig.zip"
	},
	{
		UI_TEXT("S - 三眼神童(中)"), // Mitsume ga Tooru (T-Chi)
		"mitsumegatooruc.zip"
	},
	{
		UI_TEXT("S - 沙罗曼蛇(美)"), // Life Force (USA)
		"lifeforce.zip"
	},
	{
		UI_TEXT("S - 沙罗曼蛇(中)"), // Salamander (T-Chi, v1.2)
		"salamanderc.zip"
	},
	{
		UI_TEXT("S - 双截龙1(美)"), // Double Dragon (USA)
		"doubledragon.zip"
	},
	{
		UI_TEXT("S - 双截龙2 复仇(美)"), // Double Dragon II - The Revenge (USA)
		"doubldraii.zip"
	},
	{
		UI_TEXT("S - 双截龙3 罗塞塔之石(美)"), // Double Dragon III - The Sacred Stones (USA)
		"doubldraiii.zip"
	},
	{
		UI_TEXT("S - 松鼠大作战1(美)"), // Chip 'n Dale - Rescue Rangers (USA)
		"chipndalresra.zip"
	},
	{
		UI_TEXT("S - 松鼠大作战2(美)"), // Chip 'n Dale - Rescue Rangers 2 (USA)
		"chipndalresra2.zip"
	},
	{
		UI_TEXT("T - 坦克大战(日)"), // BattleCity (Japan)
		"battlecity.zip"
	},
	{
		UI_TEXT("T - 淘金者1(美)"), // Lode Runner (USA)
		"loderunner.zip"
	},
	{
		UI_TEXT("T - 淘金者2(日)"), // Championship Lode Runner (Japan)
		"champlodrun.zip"
	},
	{
		UI_TEXT("T - 特种部队1(美)"), // G.I. Joe - A Real American Hero (USA)
		"gijoeareamhe.zip"
	},
	{
		UI_TEXT("T - 特种部队2(美)"), // G.I. Joe - The Atlantis Factor (USA)
		"gijoeatlfa.zip"
	},
	{
		UI_TEXT("T - 踢王(美)"), // Kick Master (USA)
		"kickmaster.zip"
	},
	{
		UI_TEXT("T - 脱狱(美)"), // P.O.W. - Prisoners of War (USA)
		"pow.zip"
	},
	{
		UI_TEXT("X - 星际魂斗罗(美)"), // Journey to Silius (USA)
		"journsil.zip"
	},	
	{
		UI_TEXT("Y - 宇宙巡航机1(美)"), // Gradius (USA)
		"gradius.zip"
	},
	{
		UI_TEXT("Y - 宇宙巡航机2(日)"), // Gradius II (Japan)
		"gradiusii.zip"
	}
};
const std::span<const BundledGameInfo> AppMeta::bundledGameInfo{gameInfo, std::size(gameInfo)};

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
		KeyMapping{NesKey::B, Keycode::Z}, KeyMapping{KeyInfo::turboKey(NesKey::B), Keycode::A},
		KeyMapping{NesKey::A, Keycode::X}, KeyMapping{KeyInfo::turboKey(NesKey::A), Keycode::S},
	};

	static constexpr std::array genericGamepadMap
	{
		KeyMapping{NesKey::Up, Keycode::UP},
		KeyMapping{NesKey::Right, Keycode::RIGHT},
		KeyMapping{NesKey::Down, Keycode::DOWN},
		KeyMapping{NesKey::Left, Keycode::LEFT},
		KeyMapping{NesKey::Select, Keycode::GAME_SELECT},
		KeyMapping{NesKey::Start, Keycode::GAME_START},
		KeyMapping{NesKey::B, Keycode::GAME_X}, KeyMapping{KeyInfo::turboKey(NesKey::B), Keycode::GAME_Y},
		KeyMapping{NesKey::A, Keycode::GAME_A}, KeyMapping{KeyInfo::turboKey(NesKey::A), Keycode::GAME_B},
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
