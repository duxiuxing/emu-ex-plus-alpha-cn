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
		{ "2020bb", "2020超级棒球" }, { "2020bba", "2020超级棒球 (第2套)" }, { "2020bbh", "2020超级棒球 (第3套)" },
		{ "3countb", "三回合较量" },
		// A
		{ "alpham2", "阿尔法任务2 最后的守护神" },
		{ "androdun", "勇者之击" },
		{ "aodk", "痛快进行曲" },
		{ "aof", "龙虎之拳1" },
		{ "aof2", "龙虎之拳2" }, { "aof2a", "龙虎之拳2 (NGH-056)" },
		{ "aof3", "龙虎之拳3 斗士之路" }, { "aof3k", "龙虎之拳3 斗士之路 (韩版)" },
		{ "sonicwi2", "四国战机2" },
		{ "sonicwi3", "四国战机3" },
		// B
		{ "bakatono", "傻瓜殿下麻将漫游记" },
		{ "bangbead", "碰碰球" },
		{ "bbbuster", "爆炸克星 (有bug)" },
		{ "bjourney", "蓝色之旅" },
		{ "blazstar", "闪亮之星" },
		{ "breakers", "爆裂人" },
		{ "breakrev", "爆裂人复仇" },
		{ "bstars", "棒球之星专业版" },
		{ "bstars2", "棒球之星2" },
		{ "burningf", "热血快打" }, { "burningfh", "热血快打 (NGH-018 美版)" },
		{ "flipshot", "盾牌大战" },
		// C
		{ "crsword", "圣十字剑" },
		// { "ct2k3sa", "拳皇卧虎藏龙2003 风云 超级加强替换版" }, { "ct2k3sp", "拳皇卧虎藏龙2003 风云 超级加强版" }, { "cthd2003", "拳皇卧虎藏龙2003" },
		{ "ctomaday", "蕃茄超人" },
		{ "cyberlip", "战斗边缘" },
		{ "marukodq", "小丸子问答" },
		// D
		{ "diggerma", "挖地先生" },
		{ "doubledr", "双截龙格斗" },
		// E
		{ "eightman", "8号超人" },
		// F
		{ "fatfursp", "饿狼传说特别版" }, { "fatfursa", "饿狼传说特别版 (第2套)" },
		{ "fatfury1", "饿狼传说1 宿命之战" },
		{ "fatfury2", "饿狼传说2" },
		{ "fatfury3", "饿狼传说3 远古征战" },
		{ "fbfrenzy", "疯狂美式足球" },
		{ "fightfev", "狂热格斗" }, { "fightfeva", "狂热格斗 (第2套)" },
		{ "kabukikl", "天外魔境真传" },
		// G
		{ "galaxyfg", "银河快打 宇宙战士" },
		{ "ganryu", "武藏严流记" },
		{ "garou", "饿狼传说 狼之印记" }, { "garoubl", "饿狼传说 狼之印记 (盗版)" }, { "garouo", "饿狼传说 狼之印记 (第2套)" }, { "garoup", "饿狼传说 狼之印记 (工程版)" },
		{ "ghostlop", "鬼精灵投球手" },
		{ "goalx3", "射门！射门！射门！" },
		// { "gowcaizr", "超人学园钢帝王" },
		{ "gpilots", "魔鬼飞行员" }, { "gpilotsh", "魔鬼飞行员 (NGH-020 美版)" },
		{ "gururin", "梦幻天堂" },
		// I
		{ "ironclad", "超铁战机" }, { "ironclado", "超铁战机 (较旧的工程版)" },
		// { "irrmaze", "终极电流急急棒" },
		// J
		{ "janshin", "雀神传说" },
		{ "jockeygp", "赛马大奖赛" },
		// { "joyjoy", "阳光方块" },
		// K
		{ "karnovr", "卡诺夫的复仇" },
		// { "kf2k2mp", "" }, { "kf2k2mp2", "" }, { "kf2k2pla", "" }, { "kf2k2pls", "" },
		// { "kf2k3bl", "" }, { "kf2k3bla", "" }, { "kf2k3pcb", "" }, { "kf2k3pl", "" }, { "kf2k3upl", "" },
		// { "kf2k5uni", "" },
		// { "kf10thep", "" },
		{ "kizuna", "风云默示录 超级组队战" },
		// { "kof2k4se", "" },
		// { "kof10th", "" },
		// { "kof94", "" },
		// { "kof95", "" }, { "kof95h", "" },
		// { "kof96", "" }, { "kof96h", "" },
		// { "kof97", "" }, { "kof97a", "" }, { "kof97pls", "" },
		// { "kof98", "" }, { "kof98k", "" }, { "kof98n", "" },
		// { "kof99", "" }, { "kof99a", "" }, { "kof99e", "" }, { "kof99n", "" }, { "kof99p", "" },
		// { "kof2000", "" }, { "kof2000n", "" },
		// { "kof2001", "" }, { "kof2001h", "" },
		// { "kof2002", "" }, { "kof2002b", "" },
		// { "kof2003", "" }, { "kof2003h", "" },
		// { "kog", "格斗之王 (拳皇97 盗版)" },
		{ "kotm", "怪兽之王1" }, { "kotmh", "怪兽之王1 (第2套)" },
		{ "kotm2", "怪兽之王2" },
		// L
		// { "lans2004", "" },
		// { "lastblad", "" }, { "lastbladh", "" },
		// { "lastbld2", "" },
		// { "lastsold", "" },
		{ "lbowling", "联盟保龄球" },
		{ "legendos", "明日之丈传说" },
		{ "lresort", "终极手段" },
		// M
		{ "magdrop2", "魔法微量球2" },
		{ "magdrop3", "魔法微量球3" },
		{ "maglord", "幻魔大战" }, { "maglordh", "幻魔大战 (NGH-005)" },
		{ "mahretsu", "麻将狂列传" },		
		{ "matrim", "新豪血寺一族 斗婚" }, { "matrimbl", "新豪血寺一族 斗婚 (盗版)" },
		{ "miexchng", "钱币对抗赛" },
		{ "minasan", "麻将大富翁" },
		// { "mosyougi", "" },
		// { "ms4plus", "" },
		// { "ms5pcb", "" }, { "ms5plus", "" },
		{ "mslug", "合金弹头1" },
		{ "mslug2", "合金弹头2" },
		{ "mslug3", "合金弹头3" }, { "mslug3b6", "合金弹头3 (盗版)" }, { "mslug3h", "合金弹头3 (NGH-2560)" },
		{ "mslug4", "合金弹头4" },
		{ "mslug5", "合金弹头5" }, { "mslug5h", "合金弹头5 (NGH-2608)" },
		{ "mslugx", "合金弹头X" },
		{ "mutnat", "外星游龙" },
		// N
		{ "nam1975", "越战1975" },
		{ "ncombat", "忍者大战" }, { "ncombath", "忍者大战 (NGH-009)" },
		{ "ncommand", "忍者指令" },
		{ "neobombe", "炸弹人机皇版" },
		{ "neocup98", "98机皇杯 胜利之路" },
		{ "neodrift", "拉力赛车 新的技术" },
		{ "neomrdo", "嘟先生" },
		{ "ninjamas", "霸王忍法帖" },
		{ "nitd", "午夜噩梦" }, { "nitdbl", "午夜噩梦 (盗版)" },
		{ "turfmast", "大联盟高尔夫" },
		// { "overtop", "" },
		// { "panicbom", "" },
		// { "pbobbl2n", "" },
		// { "pbobblen", "" }, { "pbobblena", "" },
		// { "pgoal", "" },
		// { "pnyaa", "" },
		// { "popbounc", "" },
		// { "preisle2", "" },
		// { "pspikes2", "" },
		// { "pulstar", "" },
		// { "puzzldpr", "" }, { "puzzledp", "" },
		// { "quizdai2", "" }, { "quizdais", "" },
		// { "quizkof", "" },
		// { "ragnagrd", "" },
		// { "rbff1", "" }, { "rbff1a", "" },
		// { "rbff2", "" }, { "rbff2h", "" }, { "rbff2k", "" },
		// { "rbffspec", "" },
		// { "ridhero", "" }, { "ridheroh", "" },
		// { "roboarmy", "" },
		// { "rotd", "" },
		// { "s1945p", "" },
		// { "samsh5sp", "" }, { "samsh5sph", "" }, { "samsh5spn", "" },
		// { "samsho", "" }, { "samshoh", "" },
		// { "samsho2", "" },
		{ "samsho3", "侍魂3" }, { "samsho3h", "侍魂3 (NGH-087)" }, { "fswords", "侍魂3 (韩版)" },
		{ "samsho4", "侍魂4" },
		{ "samsho5", "侍魂5" }, { "samsho5b", "侍魂5 (盗版)" }, { "samsho5h", "侍魂5 (NGH-2700)" },
		// { "savagere", "" },
		// { "sdodgeb", "" },
		// { "sengokh", "" }, { "sengoku", "" },
		// { "sengoku2", "" },
		// { "sengoku3", "" },
		// { "shocktr2", "" }, { "shocktra", "" }, { "shocktro", "" },
		// { "socbrawl", "" }, { "socbrawla", "" },
		// { "spinmast", "" },
		// { "ssideki", "" },
		// { "ssideki2", "" },
		// { "ssideki3", "" },
		// { "ssideki4", "" },
		// { "stakwin", "" },
		// { "stakwin2", "" },
		// { "strhoop", "" },
		// { "superspy", "" },
		// { "svc", "" }, { "svcboot", "" }, { "svcpcb", "" }, { "svcpcba", "" },
		// { "svcplus", "" }, { "svcplusa", "" }, { "svcsplus", "" },
		// { "tophuntr", "" }, { "tophuntra", "" },
		// { "totcarib", "" },
		// { "tpgolf", "" },
		// { "trally", "" },
		// { "twinspri", "" },
		// { "tws96", "" },
		// { "viewpoin", "" },
		// { "vliner", "" }, { "vlinero", "" },
		// { "wakuwak7", "" },
		// { "wh1", "" }, { "wh1h", "" }, { "wh1ha", "" },
		// { "wh2", "" }, { "wh2j", "" }, { "wh2jh", "" },
		// { "whp", "" },
		// { "wjammers", "" },
		// { "zedblade", "" },
		// { "zintrckb", "" },
		// { "zupapa", "" },
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
