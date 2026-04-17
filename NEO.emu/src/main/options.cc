/*  This file is part of NEO.emu.

	NEO.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	NEO.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with NEO.emu.  If not, see <http://www.gnu.org/licenses/> */

module;
extern "C"
{
	#include <gngeo/roms.h>
	#include <gngeo/emu.h>
}

module system;

namespace EmuEx
{

void NeoSystem::setTimerIntOption()
{
	if(optionTimerInt == 2)
	{
		bool needsTimer = hasContent() && containsAny(contentDisplayName(),
			"Sidekicks 2", "Sidekicks 3", "Ultimate 11", "Neo-Geo Cup", "Spin Master", "Neo Turf Masters");
		if(needsTimer) log.info("auto enabled timer interrupt");
		conf.raster = needsTimer;
	}
	else
	{
		conf.raster = optionTimerInt;
	}
}

void NeoSystem::onOptionsLoaded()
{
	conf.system = SYSTEM(optionBIOSType.value());
	conf.country = COUNTRY(optionMVSCountry.value());
}

bool NeoSystem::resetSessionOptions(EmuApp &app)
{
	optionTimerInt.reset();
	setTimerIntOption();
	return true;
}

bool NeoSystem::readConfig(ConfigType type, MapIO &io, unsigned key)
{
	if(type == ConfigType::MAIN)
	{
		switch(key)
		{
			case CFGKEY_LIST_ALL_GAMES: return readOptionValue(io, optionListAllGames);
			case CFGKEY_BIOS_TYPE: return readOptionValue(io, optionBIOSType);
			case CFGKEY_MVS_COUNTRY: return readOptionValue(io, optionMVSCountry);
			case CFGKEY_CREATE_USE_CACHE: return readOptionValue(io, optionCreateAndUseCache);
			case CFGKEY_STRICT_ROM_CHECKING: return readOptionValue(io, optionStrictROMChecking);
		}
	}
	else if(type == ConfigType::SESSION)
	{
		switch(key)
		{
			case CFGKEY_TIMER_INT: return readOptionValue(io, optionTimerInt);
		}
	}
	return false;
}

void NeoSystem::writeConfig(ConfigType type, FileIO &io)
{
	if(type == ConfigType::MAIN)
	{
		writeOptionValueIfNotDefault(io, optionListAllGames);
		writeOptionValueIfNotDefault(io, optionBIOSType);
		writeOptionValueIfNotDefault(io, optionMVSCountry);
		writeOptionValueIfNotDefault(io, optionCreateAndUseCache);
		writeOptionValueIfNotDefault(io, optionStrictROMChecking);
	}
	else if(type == ConfigType::SESSION)
	{
		writeOptionValueIfNotDefault(io, optionTimerInt);
	}
}

void NeoSystem::translate_long_name(ROM_DEF* drv)
{
	static std::map<std::string_view, std::string_view> s_drvName2DisplayName = {
		// #
		{ "2020bb", "# - 2020超级棒球" }, { "2020bba", "# - 2020超级棒球 (第2套)" }, { "2020bbh", "# - 2020超级棒球 (第3套)" },
		{ "3countb", "S - 三回合较量" },
		// A
		{ "alpham2", "A - 阿尔法任务2 最后的守护神" },
		{ "androdun", "Y - 勇者之击" },
		{ "aodk", "T - 痛快进行曲" },
		{ "aof", "L - 龙虎之拳1" },
		{ "aof2", "L - 龙虎之拳2" }, { "aof2a", "L - 龙虎之拳2 (第2套)" },
		{ "aof3", "L - 龙虎之拳3 斗士之路" }, { "aof3k", "L - 龙虎之拳3 斗士之路 (韩版)" },
		{ "sonicwi2", "S - 四国战机2" },
		{ "sonicwi3", "S - 四国战机3" },
		// B
		{ "bakatono", "S - 傻瓜殿下麻将漫游记" },
		{ "bangbead", "P - 碰碰球" },
		{ "bbbuster", "B - 爆炸克星 (有bug)" },
		{ "bjourney", "L - 蓝色之旅" },
		{ "blazstar", "S - 闪亮之星" },
		{ "breakers", "B - 爆裂人" },
		{ "breakrev", "B - 爆裂人复仇" },
		{ "bstars", "B - 棒球之星专业版" },
		{ "bstars2", "B - 棒球之星2" },
		{ "burningf", "R - 热血快打" }, { "burningfh", "R - 热血快打 (第2套)" },
		{ "flipshot", "D - 盾牌大战" },
		// C
		{ "crsword", "S - 圣十字剑" },
		// { "ct2k3sa", "拳皇卧虎藏龙2003 风云 超级加强替换版" }, { "ct2k3sp", "拳皇卧虎藏龙2003 风云 超级加强版" }, { "cthd2003", "拳皇卧虎藏龙2003" },
		{ "ctomaday", "F - 蕃茄超人" },
		{ "cyberlip", "Z - 战斗边缘" },
		{ "marukodq", "X - 小丸子问答" },
		// D
		{ "diggerma", "W - 挖地先生" },
		{ "doubledr", "S - 双截龙格斗" },
		// E
		{ "eightman", "# - 8号超人" },
		// F
		{ "fatfursp", "E - 饿狼传说特别版" }, { "fatfursa", "E - 饿狼传说特别版 (第2套)" },
		{ "fatfury1", "E - 饿狼传说1 宿命之战" },
		{ "fatfury2", "E - 饿狼传说2" },
		{ "fatfury3", "E - 饿狼传说3 远古征战" },
		{ "fbfrenzy", "F - 疯狂美式足球" },
		{ "fightfev", "K - 狂热格斗" }, { "fightfeva", "K - 狂热格斗 (第2套)" },
		{ "kabukikl", "T - 天外魔境真传" },
		// G
		{ "galaxyfg", "Y - 银河快打 宇宙战士" },
		{ "ganryu", "W - 武藏严流记" },
		{ "garou", "E - 饿狼传说 狼之印记" }, { "garoubl", "E - 饿狼传说 狼之印记 (盗版)" }, { "garouo", "E - 饿狼传说 狼之印记 (第2套)" }, { "garoup", "E - 饿狼传说 狼之印记 (工程版)" },
		{ "ghostlop", "G - 鬼精灵投球手" },
		{ "goalx3", "S - 射门！射门！射门！" },
		{ "gpilots", "M - 魔鬼飞行员" }, { "gpilotsh", "M - 魔鬼飞行员 (第2套)" },
		{ "gururin", "M - 梦幻天堂" },
		// I
		{ "ironclad", "C - 超铁战机" }, { "ironclado", "C - 超铁战机 (旧的工程版)" },
		// J
		{ "janshin", "Q - 雀神传说" },
		{ "jockeygp", "S - 赛马大奖赛 (有Bug)" },		
		// K
		{ "karnovr", "K - 卡诺夫的复仇" },
		// { "kf2k2mp", "" }, { "kf2k2mp2", "" }, { "kf2k2pla", "" }, { "kf2k2pls", "" },
		// { "kf2k3bl", "" }, { "kf2k3bla", "" }, { "kf2k3pcb", "" }, { "kf2k3pl", "" }, { "kf2k3upl", "" },
		// { "kf2k5uni", "" },
		// { "kf10thep", "" },
		{ "kizuna", "F - 风云默示录 超级组队战" },
		// { "kof2k4se", "" },
		// { "kof10th", "" },		
		{ "kotm", "G - 怪兽之王1" }, { "kotmh", "G - 怪兽之王1 (第2套)" },
		{ "kotm2", "G - 怪兽之王2" },
		// L
		{ "lbowling", "L - 联盟保龄球" },
		{ "legendos", "M - 明日之丈传说" },
		{ "lresort", "Z - 终极手段" },
		// M
		{ "magdrop2", "M - 魔法微量球2" },
		{ "magdrop3", "M - 魔法微量球3" },
		{ "maglord", "H - 幻魔大战" }, { "maglordh", "H - 幻魔大战 (第2套)" },
		{ "mahretsu", "M - 麻将狂列传" },		
		{ "matrim", "X - 新豪血寺一族 斗婚" }, { "matrimbl", "X - 新豪血寺一族 斗婚 (盗版 有Bug)" },
		{ "miexchng", "Q - 钱币对抗赛" },
		{ "minasan", "M - 麻将大富翁" },
		{ "mslug", "H - 合金弹头1" },
		{ "mslug2", "H - 合金弹头2" },
		{ "mslug3", "H - 合金弹头3" }, { "mslug3b6", "H - 合金弹头3 (盗版)" }, { "mslug3h", "H - 合金弹头3 (未加密)" },
		{ "mslug4", "H - 合金弹头4" }, { "ms4plus", "H - 合金弹头4加强版 (盗版)" },
		{ "mslug5", "H - 合金弹头5" }, { "mslug5h", "H - 合金弹头5 (第2套)" }, { "ms5pcb", "H - 合金弹头5 (JAMMA标准主板 有Bug)" }, { "ms5plus", "H - 合金弹头5加强版 (盗版 有Bug)" },
		{ "mslugx", "H - 合金弹头X" },
		{ "mutnat", "W - 外星游龙" },
		// N
		{ "nam1975", "Y - 越战1975" },
		{ "ncombat", "R - 忍者大战" }, { "ncombath", "R - 忍者大战 (第2套)" },
		{ "ncommand", "R - 忍者指令" },
		{ "neobombe", "Z - 炸弹人机皇版" },
		{ "neocup98", "# - 98机皇杯 胜利之路" },
		{ "neodrift", "L - 拉力赛车 新的技术" },
		{ "neomrdo", "D - 嘟先生" },
		{ "ninjamas", "B - 霸王忍法帖" },
		{ "nitd", "W - 午夜噩梦" }, { "nitdbl", "W - 午夜噩梦 (盗版)" },
		{ "turfmast", "D - 大联盟高尔夫" },
		// O
		{ "overtop", "K - 跨越巅峰" },
		// P
		{ "joyjoy", "Y - 阳光方块" },
		{ "panicbom", "Z - 炸弹人方块" },
		{ "pbobbl2n", "P - 泡泡龙2" },
		{ "pbobblen", "P - 泡泡龙1" }, { "pbobblena", "P - 泡泡龙1 (第2套)" },
		{ "pgoal", "K - 开心射门 5对5迷你足球" },
		{ "pnyaa", "C - 宠物方块" },
		{ "popbounc", "D - 动物方块" },
		{ "preisle2", "Y - 原始岛2" },
		{ "pspikes2", "C  - 超级排球2" },
		{ "pulstar", "H - 惑星战机" },
		{ "puzzldpr", "J - 解谜气泡R" },
		{ "puzzledp", "J - 解谜气泡" },
		// Q
		{ "quizdai2", "W - 问答大搜查线2 问答迷侦探" },
		{ "quizdais", "W - 问答大搜查线1 最后倒计时" },
		{ "quizkof", "Q - 拳皇问答" },
		// R
		{ "ragnagrd", "S - 神凰拳" },
		{ "rbff1", "Z - 真饿狼传说1" }, { "rbff1a", "Z - 真饿狼传说1 (修复版)" },
		{ "rbff2", "Z - 真饿狼传说2 新的来者" }, { "rbff2h", "Z - 真饿狼传说2 新的来者 (第2套)" }, { "rbff2k", "Z - 真饿狼传说2 新的来者 (韩版)" },
		{ "rbffspec", "Z - 真饿狼传说特别版" },
		{ "ridhero", "F - 风速英雄 (有bug)" }, { "ridheroh", "F - 风速英雄 (第2套 有bug)" },
		{ "roboarmy", "J - 机甲神兵" },
		{ "rotd", "L - 龙吼 (有bug)" },
		// S
		{ "mosyougi", "J - 将棋达人" },
		{ "s1945p", "D - 打击者1945加强版" },
		{ "samsh5sp", "S - 侍魂5 特别版" }, { "samsh5sph", "S - 侍魂5 特别版 (第2套)" }, { "samsh5spn", "S - 侍魂5 特别版 (第3套)" },
		{ "samsho", "S - 侍魂1" }, { "samshoh", "S - 侍魂1 (第2套)" },
		{ "samsho2", "S - 侍魂2 霸王丸地狱变" },
		{ "samsho3", "S - 侍魂3 斩红郎无双剑" }, { "samsho3h", "S - 侍魂3 斩红郎无双剑 (第2套)" }, { "fswords", "S - 侍魂3 斩红郎无双剑 (韩版)" },
		{ "samsho4", "S - 侍魂4 天草降临" },
		{ "samsho5", "S - 侍魂5 零" }, { "samsho5b", "S - 侍魂5 零 (盗版 有Bug)" }, { "samsho5h", "S - 侍魂5 零 (第2套)" },
		{ "savagere", "F - 风云默示录" },
		{ "sdodgeb", "R - 热血超级躲避球" },
		{ "sengoku", "Z - 战国传承1" }, { "sengokh", "Z - 战国传承1 (第2套)" }, 
		{ "sengoku2", "Z - 战国传承2" },
		{ "sengoku3", "Z - 战国传承3" },
		{ "shocktr2", "T - 突击骑兵2 第二分队" }, { "lans2004", "T - 突击骑兵2 第二分队 (盗版 有Bug)" },
		{ "shocktro", "T - 突击骑兵1" }, { "shocktra", "T - 突击骑兵1 (第2套)" },
		{ "socbrawl", "J - 机器人足球" }, { "socbrawla", "J - 机器人足球 (第2套)" },
		{ "spinmast", "Q - 强尼大冒险" },
		{ "ssideki", "D - 得点王1" },
		{ "ssideki2", "D - 得点王2" },
		{ "ssideki3", "D - 得点王3" },
		{ "ssideki4", "D - 得点王4" },
		{ "stakwin", "S - 赛马大亨1" },
		{ "stakwin2", "S - 赛马大亨2" },
		{ "strhoop", "J - 街头篮球" },
		{ "svc", "Q - 拳皇对街霸" }, { "svcboot", "Q - 拳皇对街霸 (盗版 有Bug)" }, { "svcpcb", "Q - 拳皇对街霸 (JAMMA标准主板 第1套 有Bug)" }, { "svcpcba", "Q - 拳皇对街霸 (JAMMA标准主板 第2套 有Bug)" },
		{ "svcplus", "Q - 拳皇对街霸加强版 (盗版 第1套 有Bug)" }, { "svcplusa", "Q - 拳皇对街霸加强版 (盗版 第2套 有Bug)" },
		{ "svcsplus", "Q - 拳皇对街霸超级加强版 (盗版 有Bug)" },
		// T
		{ "irrmaze", "Z - 终极电流急急棒 (有Bug)" },
		{ "kof94", "Q - 拳皇94" },
		{ "kof95", "Q - 拳皇95" }, { "kof95h", "Q - 拳皇95 (第2套)" },
		{ "kof96", "Q - 拳皇96" }, { "kof96h", "Q - 拳皇96 (第2套)" },
		{ "kof97", "Q - 拳皇97" }, { "kof97a", "Q - 拳皇97 (第2套)" }, { "kog", "Q - 拳皇97 (盗版 有Bug)" },{ "kof97pls", "Q - 拳皇97加强版 (盗版)" },
		{ "kof98", "Q - 拳皇98" }, { "kof98k", "Q - 拳皇98 (韩版)" }, { "kof98n", "Q - 拳皇98 (未加密)" },
		{ "kof99", "Q - 拳皇99" }, { "kof99a", "Q - 拳皇99 (第2套)" }, { "kof99e", "Q - 拳皇99 (早期版)" }, { "kof99n", "Q - 拳皇99 (未加密)" }, { "kof99p", "Q - 拳皇99 (工程版)" },
		{ "kof2000", "Q - 拳皇2000" }, { "kof2000n", "Q - 拳皇2000 (未加密)" },
		{ "kof2001", "Q - 拳皇2001" }, { "kof2001h", "Q - 拳皇2001 (第2套)" },
		{ "kof2002", "Q - 拳皇2002" }, { "kof2002b", "Q - 拳皇2002 (盗版 有Bug)" },
		{ "kof2003", "Q - 拳皇2003" }, { "kof2003h", "Q - 拳皇2003 (第2套)" },
		{ "lastblad", "Y - 月华剑士1" }, { "lastbladh", "Y - 月华剑士1 (第2套)" }, { "lastsold", "Y - 月华剑士1 (韩版)" },
		{ "lastbld2", "Y - 月华剑士2" },
		{ "superspy", "D - 谍报飞龙" },
		{ "tophuntr", "D - 顶尖猎人" }, { "tophuntra", "D - 顶尖猎人 (第2套)" },
		{ "totcarib", "H - 黑珍珠部队" },
		{ "tpgolf", "D - 顶尖高尔夫" },
		{ "trally", "J - 极速英豪" },
		{ "twinspri", "M - 梦幻小妖精" },
		{ "tws96", "T - Tecmo世界杯足球96" },
		// V
		{ "gowcaizr", "C - 超人学园钢帝王" },
		{ "viewpoin", "S - 视觉战机" },
		{ "vliner", "S - 水果机 (有Bug)" }, { "vlinero", "S - 水果机 (第2套 有Bug)" },
		// W
		{ "wakuwak7", "H - 火热火热7" },
		{ "wh1", "S - 世界英雄1" }, { "wh1h", "S - 世界英雄1 (第2套)" }, { "wh1ha", "S - 世界英雄1 (第3套)" },
		{ "wh2", "S - 世界英雄2" },
		{ "wh2j", "S - 世界英雄2喷射版" }, { "wh2jh", "S - 世界英雄2喷射版 (第2套)" },
		{ "whp", "S - 世界英雄完美版" },
		{ "wjammers", "C - 超级飞碟球" },
		// Z
		{ "zedblade", "Z - 作战名末日" },
		{ "zintrckb", "S - 生物泡泡" },
		{ "zupapa", "X - 星星大战" },
	};

	auto it = s_drvName2DisplayName.find(drv->name);
	if (it != s_drvName2DisplayName.end())
	{
		auto length = it->second.length();
		it->second.copy(drv->longname, length);
		drv->longname[length] = '\0';
	}
}

}
