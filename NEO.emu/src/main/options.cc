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
		{ "2020bb", "2020超级棒球" }, { "2020bba", "2020超级棒球 (第2套)" }, { "2020bbh", "2020超级棒球 (第3套)" },
		{ "3countb", "三回合较量" },
		{ "alpham2", "阿尔法任务2" },
		{ "androdun", "勇者之击" },
		{ "aodk", "痛快进行曲" },
		{ "aof", "龙虎之拳1" },
		{ "aof2", "龙虎之拳2" }, { "aof2a", "龙虎之拳2 (第2套)" },
		{ "aof3", "龙虎之拳3 斗士之路" }, { "aof3k", "龙虎之拳3 斗士之路 (韩版)" },
		{ "bakatono", "傻瓜殿下麻将漫游记" },
		{ "bangbead", "碰碰球" },
		{ "bbbuster", "爆炸克星 (有bug)" },
		{ "bjourney", "蓝色之旅" },
		{ "blazstar", "闪亮之星" },
		{ "breakers", "爆裂人" },
		{ "breakrev", "爆裂人复仇" },
		{ "bstars", "棒球之星Pro" },
		{ "bstars2", "棒球之星2" },
		{ "burningf", "热血快打" }, { "burningfh", "热血快打 (第2套)" },
		{ "flipshot", "盾牌大战" },
		{ "crsword", "圣十字剑" },
		{ "ct2k3sa", "拳皇卧虎藏龙2003 风云 超级加强替换版" }, { "ct2k3sp", "拳皇卧虎藏龙2003 风云 超级加强版" }, { "cthd2003", "拳皇卧虎藏龙2003" },
		{ "ctomaday", "蕃茄超人" },
		{ "cyberlip", "战斗边缘" },
		{ "diggerma", "挖地先生" },
		{ "doubledr", "双截龙" },
		{ "eightman", "八超人" },
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
