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

extern "C"
{
	#include <gngeo/resfile.h>
	#include <gngeo/conf.h>
	#include <gngeo/emu.h>
	#include <gngeo/fileio.h>
	#include <gngeo/timer.h>
	#include <gngeo/memory.h>
}
#include <imagine/util/macros.h>

import system;
import emuex;
import imagine;
import std;

namespace EmuEx
{

using namespace IG;
using MainAppHelper = EmuAppHelperBase<MainApp>;

class ConsoleOptionView : public TableView, public MainAppHelper
{
	TextMenuItem timerItem[3]
	{
		{
			UI_TEXT("关"),
			attachParams(), setTimerIntDel(), {.id = 0}
		},
		{
			UI_TEXT("开"),
			attachParams(), setTimerIntDel(), {.id = 1}
		},
		{
			UI_TEXT("自动"),
			attachParams(), setTimerIntDel(), {.id = 2}
		},
	};

	TextMenuItem::SelectDelegate setTimerIntDel()
	{
		return [this](TextMenuItem &item)
		{
			system().sessionOptionSet();
			system().optionTimerInt = item.id;
			system().setTimerIntOption();
		};
	}

	MultiChoiceMenuItem timer
	{
		UI_TEXT("模拟计时器"),
		attachParams(),
		std::min((int)system().optionTimerInt, 2),
		timerItem,
		{
			.onSetDisplayString = [](auto idx, Gfx::Text& t)
			{
				if(idx == 2)
				{
					t.resetString(conf.raster
						? UI_TEXT("开")
						: UI_TEXT("关")
					);
					return true;
				}
				else
					return false;
			}
		},
	};

	std::array<MenuItem*, 1> menuItem
	{
		&timer
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

class CustomSystemOptionView : public SystemOptionView, public MainAppHelper
{
	using MainAppHelper::system;

	TextMenuItem::SelectDelegate setRegionDel()
	{
		return [this](TextMenuItem &item)
		{
			conf.country = (COUNTRY)item.id.val;
			system().optionMVSCountry = conf.country;
		};
	}

	TextMenuItem regionItem[4]
	{
		{
			UI_TEXT("日本"),
			attachParams(), setRegionDel(), {.id = CTY_JAPAN}
		},
		{
			UI_TEXT("欧洲"),
			attachParams(), setRegionDel(), {.id = CTY_EUROPE}
		},
		{
			UI_TEXT("美国"),
			attachParams(), setRegionDel(), {.id = CTY_USA}
		},
		{
			UI_TEXT("亚洲"),
			attachParams(), setRegionDel(), {.id = CTY_ASIA}
		},
	};

	MultiChoiceMenuItem region
	{
		UI_TEXT("MVS 地区"),
		attachParams(),
		std::min((int)conf.country, 3),
		regionItem
	};

	TextMenuItem::SelectDelegate setBiosDel()
	{
		return [this](TextMenuItem &item)
		{
			conf.system = (SYSTEM)item.id.val;
			system().optionBIOSType = conf.system;
		};
	}

	TextMenuItem biosItem[7]
	{
		{
			UI_TEXT("Unibios 2.3"),
			attachParams(), setBiosDel(), {.id = SYS_UNIBIOS}
		},
		{
			UI_TEXT("Unibios 3.0"),
			attachParams(), setBiosDel(), {.id = SYS_UNIBIOS_3_0}
		},
		{
			UI_TEXT("Unibios 3.1"),
			attachParams(), setBiosDel(), {.id = SYS_UNIBIOS_3_1}
		},
		{
			UI_TEXT("Unibios 3.2"),
			attachParams(), setBiosDel(), {.id = SYS_UNIBIOS_3_2}
		},
		{
			UI_TEXT("Unibios 3.3"),
			attachParams(), setBiosDel(), {.id = SYS_UNIBIOS_3_3}
        },
		{
			UI_TEXT("Unibios 4.0"),
			attachParams(), setBiosDel(), {.id = SYS_UNIBIOS_4_0}
		},
		{
			UI_TEXT("MVS"),
			attachParams(), setBiosDel(), {.id = SYS_ARCADE}
		},
	};

	MultiChoiceMenuItem bios
	{
		UI_TEXT("BIOS 类型"),
		attachParams(),
		MenuId{conf.system},
		biosItem
	};

	BoolMenuItem createAndUseCache
	{
		UI_TEXT("使用缓存文件"),
		attachParams(),
		(bool)system().optionCreateAndUseCache,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			system().optionCreateAndUseCache = item.flipBoolValue(*this);
		}
	};

	BoolMenuItem strictROMChecking
	{
		UI_TEXT("严格检查 ROM"),
		attachParams(),
		(bool)system().optionStrictROMChecking,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			system().optionStrictROMChecking = item.flipBoolValue(*this);
		}
	};

public:
	CustomSystemOptionView(ViewAttachParams attach): SystemOptionView{attach, true}
	{
		loadStockItems();
		item.emplace_back(&bios);
		item.emplace_back(&region);
		item.emplace_back(&createAndUseCache);
		item.emplace_back(&strictROMChecking);
	}
};

class EmuGUIOptionView : public GUIOptionView, public MainAppHelper
{
	using MainAppHelper::system;

	BoolMenuItem listAll
	{
		UI_TEXT("列出所有游戏"),
		attachParams(),
		(bool)system().optionListAllGames,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			system().optionListAllGames = item.flipBoolValue(*this);
		}
	};

public:
	EmuGUIOptionView(ViewAttachParams attach): GUIOptionView{attach, true}
	{
		loadStockItems();
		item.emplace_back(&listAll);
	}
};

struct RomListEntry
{
	const char* name;
	unsigned bugs;
};

constexpr RomListEntry romlist[]
{	
	{ "eightman", 0 },	// # - 8号超人
	{ "neocup98", 0 },	// # - 98机皇杯 胜利之路
	{ "2020bb", 0 },	// # - 2020超级棒球
	{ "2020bba", 0 },
	{ "2020bbh", 0 },
	{ "alpham2", 0 },	// A - 阿尔法任务2
	{ "ninjamas", 0 },	// B - 霸王忍法帖
	{ "bstars", 0 },	// B - 棒球之星专业版
	{ "bstars2", 0 },	// B - 棒球之星2
	{ "breakers", 0 },	// B - 爆裂人
	{ "breakrev", 0 },	// B - 爆裂人复仇
	{ "bbbuster", 1 },	// B - 爆炸克星 (有bug)
	{ "wjammers", 0 },	// C - 超级飞碟球
	{ "pspikes2", 0 },	// C - 超级排球2
	{ "gowcaizr", 0 },	// C - 超人学园钢帝王
	{ "ironclad", 0 },	// C - 超铁战机
	{ "ironclado", 0 },
	{ "pnyaa", 0 },		// C - 宠物方块
	{ "s1945p", 0 },	// D - 打击者1945加强版
	{ "turfmast", 0 },	// D - 大联盟高尔夫
	{ "ssideki", 0 },	// D - 得点王1
	{ "ssideki2", 0 },	// D - 得点王2
	{ "ssideki3", 0 },	// D - 得点王3
	{ "ssideki4", 0 },	// D - 得点王4
	{ "superspy", 0 },	// D - 谍报飞龙
	{ "tpgolf", 0 },	// D - 顶尖高尔夫
	{ "tophuntr", 0 },	// D - 顶尖猎人
	{ "tophuntra", 0 },
	{ "popbounc", 0 },	// D - 动物方块
	{ "neomrdo", 0 },	// D - 嘟先生
	{ "flipshot", 0 },	// D - 盾牌大战
	{ "fatfury1", 0 },	// E - 饿狼传说1 宿命之战
	{ "fatfury2", 1 },	// E - 饿狼传说2
	{ "fatfursa", 0 },	// E - 饿狼传说特别版
	{ "fatfursp", 0 },
	{ "fatfury3", 0 },	// E - 饿狼传说3 远古征战
	{ "garou", 0 },		// E - 饿狼传说 狼之印记
	{ "garoubl", 0 },
	{ "garouo", 0 },
	{ "garoup", 0 },
	{ "ctomaday", 0 },	// F - 蕃茄超人
	{ "fbfrenzy", 0 },	// F - 疯狂美式足球
	{ "ridhero", 1 },	// F - 风速英雄 (有bug)
	{ "ridheroh", 1 },
	{ "savagere", 0 },	// F - 风云默示录
	{ "kizuna", 0 },	// F - 风云默示录 超级组队战
	{ "kotm", 0 },		// G - 怪兽之王1
	{ "kotmh", 0 },
	{ "kotm2", 0 },		// G - 怪兽之王2
	{ "ghostlop", 0 },	// G - 鬼精灵投球手
	{ "mslug", 0 },		// H - 合金弹头1
	{ "mslug2", 0 },	// H - 合金弹头2
	{ "mslugx", 0 },	// H - 合金弹头X
	{ "mslug3", 0 },	// H - 合金弹头3
	{ "mslug3b6", 0 },
	{ "mslug3h", 0 },
	{ "mslug4", 0 },	// H - 合金弹头4
	{ "ms4plus", 0 },
	{ "mslug5", 0 },	// H - 合金弹头5
	{ "mslug5h", 0 },
	{ "ms5pcb", 1 },
	{ "ms5plus", 1 },
	{ "totcarib", 0 },	// H - 黑珍珠部队
	{ "maglord", 0 },	// H - 幻魔大战
	{ "maglordh", 0 },
	{ "wakuwak7", 0 },	// H - 火热火热7
	{ "pulstar", 0 },	// H - 惑星战机
	{ "roboarmy", 0 },	// J - 机甲神兵
	{ "socbrawl", 0 },	// J - 机器人足球
	{ "socbrawla", 0 },
	{ "trally", 0 },	// J - 极速英豪
	{ "mosyougi", 0 },	// J - 将棋达人
	{ "strhoop", 0 },	// J - 街头篮球
	{ "puzzledp", 0 },	// J - 解谜气泡
	{ "puzzldpr", 0 },	// J - 解谜气泡R
	{ "karnovr", 0 },	// K - 卡诺夫的复仇
	{ "pgoal", 0 },		// K - 开心射门 5对5迷你足球
	{ "overtop", 0 },	// K - 跨越巅峰
	{ "fightfev", 0 },	// K - 狂热格斗
	{ "fightfeva", 0 },
	{ "neodrift", 0 },	// L - 拉力赛车 新的技术
	{ "bjourney", 0 },	// L - 蓝色之旅
	{ "lbowling", 0 },	// L - 联盟保龄球
	{ "rotd", 1 },		// L - 龙吼 (有bug)
	{ "aof", 0 },		// L - 龙虎之拳1
	{ "aof2", 0 },		// L - 龙虎之拳2
	{ "aof2a", 0 },
	{ "aof3", 0 },		// L - 龙虎之拳3
	{ "aof3k", 0 },
	{ "minasan", 0 },	// M - 麻将大富翁
	{ "mahretsu", 0 },	// M - 麻将狂列传
	{ "gururin", 0 },	// M - 梦幻天堂
	{ "twinspri", 0 },	// M - 梦幻小妖精
	{ "legendos", 0 },	// M - 明日之丈传说
	{ "magdrop2", 0 },	// M - 魔法微量球2
	{ "magdrop3", 0 },	// M - 魔法微量球3
	{ "gpilots", 0 },	// M - 魔鬼飞行员
	{ "gpilotsh", 0 },
	{ "pbobblen", 0 },	// P - 泡泡龙1
	{ "pbobblena", 0 },
	{ "pbobbl2n", 0 },	// P - 泡泡龙2
	{ "bangbead", 0 },	// P - 碰碰球
	{ "miexchng", 0 },	// Q - 钱币对抗赛
	{ "spinmast", 0 },	// Q - 强尼大冒险
	{ "kof94", 0 },		// Q - 拳皇94
	{ "kof95", 0 },		// Q - 拳皇95
	{ "kof95h", 0 },
	{ "kof96", 0 },		// Q - 拳皇96
	{ "kof96h", 0 },
	{ "kof97", 0 },		// Q - 拳皇97
	{ "kof97a", 0 },
	{ "kog", 1 },
	{ "kof97pls", 0 },
	{ "kof98", 0 },		// Q - 拳皇98
	{ "kof98k", 0 },
	{ "kof98n", 0 },
	{ "kof99", 0 },		// Q - 拳皇99
	{ "kof99a", 0 },
	{ "kof99e", 0 },
	{ "kof99n", 0 },
	{ "kof99p", 0 },
	{ "kof2000", 0 },	// Q - 拳皇2000
	{ "kof2000n", 0 },
	{ "kof2001", 0 },	// Q - 拳皇2001
	{ "kof2001h", 0 },
	{ "kof2002", 0 },	// Q - 拳皇2002
	{ "kof2002b", 1 },
	{ "kof2003", 0 },	// Q - 拳皇2003
	{ "kof2003h", 0 },
	{ "svc", 0 },		// Q - 拳皇对街霸
	{ "svcboot", 1 },
	{ "svcpcb", 1 },
	{ "svcpcba", 1 },
	{ "svcplus", 1 },
	{ "svcplusa", 1 },
	{ "svcsplus", 1 },
	{ "quizkof", 0 },	// Q - 拳皇问答
	{ "janshin", 0 },	// Q - 雀神传说
	{ "sdodgeb", 0 },	// R - 热血超级躲避球
	{ "burningf", 0 },	// R - 热血快打
	{ "burningfh", 0 },
	{ "ncombat", 0 },	// R - 忍者大战
	{ "ncombath", 0 },
	{ "ncommand", 0 },	// R - 忍者指令
	{ "stakwin", 0 },	// S - 赛马大亨1
	{ "stakwin2", 0 },	// S - 赛马大亨2
	{ "jockeygp", 1 },	// S - 赛马大奖赛 (有Bug)
	{ "3countb", 0 },	// S - 三回合较量
	{ "bakatono", 0 },	// S - 傻瓜殿下麻将漫游记
	{ "blazstar", 0 },	// S - 闪亮之星
	{ "goalx3", 0 },	// S - 射门！射门！射门！
	{ "ragnagrd", 0 },	// S - 神凰拳
	{ "zintrckb", 0 },	// S - 生物泡泡
	{ "crsword", 0 },	// S - 圣十字剑
	{ "samsho", 0 },	// S - 侍魂1
	{ "samshoh", 0 },
	{ "samsho2", 0 },	// S - 侍魂2 霸王丸地狱变
	{ "samsho3", 0 },	// S - 侍魂3 斩红郎无双剑
	{ "samsho3h", 0 },
	{ "fswords", 0 },
	{ "samsho4", 0 },	// S - 侍魂4 天草降临
	{ "samsho5", 0 },	// S - 侍魂5 零
	{ "samsho5b", 1 },
	{ "samsho5h", 0 },
	{ "samsh5sp", 0 },	// S - 侍魂5 特别版
	{ "samsh5sph", 0 },
	{ "samsh5spn", 0 },
	{ "wh1", 0 },		// S - 世界英雄1
	{ "wh1h", 0 },
	{ "wh1ha", 0 },
	{ "wh2", 0 },		// S - 世界英雄2
	{ "wh2j", 0 },
	{ "wh2jh", 0 },
	{ "whp", 0 },		// S - 世界英雄完美版
	{ "viewpoin", 0 },	// S - 视觉战机
	{ "doubledr", 0 },	// S - 双截龙
	{ "vliner", 1 },	// S - 水果机 (有Bug)
	{ "vlinero", 1 },
	{ "sonicwi2", 0 },	// S - 四国战机2
	{ "sonicwi3", 0 },	// S - 四国战机3
	{ "tws96", 0 },		// T - Tecmo世界杯足球96
	{ "kabukikl", 0 },	// T - 天外魔境真传
	{ "aodk", 0 },		// T - 痛快进行曲
	{ "shocktro", 0 },	// T - 突击骑兵1
	{ "shocktra", 0 },
	{ "shocktr2", 0 },	// T - 突击骑兵2 第二分队
	{ "lans2004", 1 },
	{ "diggerma", 0 },	// W - 挖地先生
	{ "mutnat", 0 },	// W - 外星游龙
	{ "quizdais", 0 },	// W - 问答大搜查线1 最后倒计时
	{ "quizdai2", 0 },	// W - 问答大搜查线2 问答迷侦探
	{ "nitd", 0 },		// W - 午夜噩梦
	{ "nitdbl", 0 },
	{ "ganryu", 0 },	// W - 武藏严流记
	{ "marukodq", 0 },	// X - 小丸子问答
	{ "matrim", 0 },	// X - 新豪血寺一族 斗婚
	{ "matrimbl", 1 },
	{ "zupapa", 0 },	// X - 星星大战
	{ "joyjoy", 0 },	// Y - 阳光方块
	{ "galaxyfg", 0 },	// Y - 银河快打 宇宙战士
	{ "androdun", 0 },	// Y - 勇者之击
	{ "preisle2", 0 },	// Y - 原始岛2
	{ "lastblad", 0 },	// Y - 月华剑士1
	{ "lastbladh", 0 },
	{ "lastsold", 0 },
	{ "lastbld2", 0 },	// Y - 月华剑士2
	{ "nam1975", 0 },	// Y - 越战1975
	{ "panicbom", 0 },	// Z - 炸弹人方块
	{ "neobombe", 0 },	// Z - 炸弹人机皇版
	{ "cyberlip", 0 },	// Z - 战斗边缘
	{ "sengoku", 0 },	// Z - 战国传承1
	{ "sengokh", 0 },
	{ "sengoku2", 0 },	// Z - 战国传承2
	{ "sengoku3", 0 },	// Z - 战国传承3
	{ "rbff1", 0 },		// Z - 真饿狼传说1
	{ "rbff1a", 0 },
	{ "rbffspec", 0 },	// Z - 真饿狼传说特别版
	{ "rbff2", 0 },		// Z - 真饿狼传说2 新的来者
	{ "rbff2h", 0 },
	{ "rbff2k", 0 },
	{ "irrmaze", 1 },	// Z - 终极电流急急棒 (有Bug)
	{ "lresort", 0 },	// Z - 终极手段
	{ "zedblade", 0 },	// Z - 作战名末日
	{ "ct2k3sa", 1 },
	{ "ct2k3sp", 1 },
	{ "cthd2003", 1 },
	{ "kf10thep", 1 },
	{ "kf2k2mp", 1 },
	{ "kf2k2mp2", 1 },
	{ "kf2k2pla", 1 },
	{ "kf2k2pls", 1 },
	{ "kf2k3bl", 1 },
	{ "kf2k3bla", 1 },
	{ "kf2k3pcb", 1 },
	{ "kf2k3pl", 1 },
	{ "kf2k3upl", 1 },
	{ "kf2k5uni", 1 },
	{ "kof10th", 1 },
	{ "kof2k4se", 1 },
};

static FS::PathString gameFilePath(EmuApp &app, std::string_view name)
{
	auto basePath = app.inContentSearchPath(name);
	auto ctx = app.appContext();
	if(auto zipPath = basePath + ".zip";
		ctx.fileUriExists(zipPath))
		return zipPath;
	if(auto sZipPath = basePath + ".7z";
		ctx.fileUriExists(sZipPath))
		return sZipPath;
	if(auto rarPath = basePath + ".rar";
		ctx.fileUriExists(rarPath))
		return rarPath;
	return {};
}

constexpr static bool gameFileExists(std::string_view name, std::string_view nameList)
{
	return containsAny(nameList,
		FS::FileString{name}.append(".zip"),
		FS::FileString{name}.append(".7z"),
		FS::FileString{name}.append(".rar"));
}

class GameListView : public TableView, public MainAppHelper
{
private:
	std::vector<TextMenuItem> item{};

	void loadGame(const RomListEntry &entry, Input::Event e)
	{
		auto gamePath = gameFilePath(app(), entry.name);
		app().createSystemWithMedia({}, gamePath, appContext().fileUriDisplayName(gamePath), e, {}, attachParams(),
			[this](Input::Event e)
			{
				app().recentContent.add(system());
				app().launchSystem(e);
			});
	}

public:
	GameListView(ViewAttachParams attach):
		TableView
		{
			UI_TEXT("游戏列表"),
			attach,
			item
		}
	{
		auto ctx = appContext();
		std::string fileList{}; // hold concatenated list of relevant filenames for fast checking
		fileList.reserve(4095); // avoid initial small re-allocations
		try
		{
			ctx.forEachInDirectoryUri(app().contentSearchPath,
				[&](auto &entry)
				{
					if(entry.type() == FS::file_type::directory)
						return true;
					if(entry.name().size() > 13) // MAME filenames follow 8.3 convention but names may have 9 characters
						return true;
					fileList += entry.name();
					return true;
				});
		}
		catch(...)
		{
			return;
		}
		for(const auto &entry : romlist)
		{
			ROM_DEF *drv = res_load_drv(&ctx, entry.name);
			if(!drv)
				continue;
			auto freeDrv = scopeGuard([&](){ std::free(drv); });
			bool fileExists = gameFileExists(drv->name, fileList);
			if(!system().optionListAllGames && !fileExists)
			{
				continue;
			}
			item.emplace_back(drv->longname, attachParams(),
				[this, &entry](TextMenuItem &item, View &, Input::Event e)
				{
					if(item.active())
					{
						if(entry.bugs)
						{
							app().pushAndShowModalView(makeView<YesNoAlertView>(
								UI_TEXT("这个游戏目前还不能完美运行，是否要继续读取？"),
								YesNoAlertView::Delegates{.onYes = [this, &entry](Input::Event e){ loadGame(entry, e); }}), e);
						}
						else
						{
							loadGame(entry, e);
						}
					}
					else
					{
						app().postMessage(3, 1, std::format(
							UI_TEXT("{} 不存在"),
							entry.name));
					}
					return true;
				});
			item.back().setActive(fileExists);
		}
	}

	int games() const
	{
		return item.size();
	}
};

class UnibiosSwitchesView : public TableView
{
	TextMenuItem regionItem[3]
	{
		{
			UI_TEXT("日本"),
			attachParams(), []() { setRegion(0); }
		},
		{
			UI_TEXT("美国"),
			attachParams(), []() { setRegion(1); }
		},
		{
			UI_TEXT("欧洲"),
			attachParams(), []() { setRegion(2); }
		},
	};

	MultiChoiceMenuItem region
	{
		UI_TEXT("地区"),
		attachParams(),
		(int)memory.memcard[3] & 0x3,
		regionItem
	};

	BoolMenuItem system
	{
		UI_TEXT("模式"),
		attachParams(),
		bool(memory.memcard[2] & 0x80),
		UI_TEXT("主机模式 (AES)"),
		UI_TEXT("街机模式 (MVS)"),
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			bool on = item.flipBoolValue(*this);
			memory.memcard[2] = on ? bit(7) : 0;
		}
	};

	static void setRegion(Uint8 val)
	{
		memory.memcard[3] = val;
	}

	std::array<MenuItem*, 2> items{&region, &system};

public:
	UnibiosSwitchesView(ViewAttachParams attach):
		TableView
		{
			UI_TEXT("Unibios 设置"),
			attach,
			items
		}
	{}

	/*void onShow()
	{
		// TODO
		region.refreshActive();
		system.refreshActive();
	}*/
};

class CustomSystemActionsView : public SystemActionsView
{
private:
	TextMenuItem unibiosSwitches
	{
		UI_TEXT("Unibios 设置"),
 		attachParams(),
		[this](TextMenuItem &item, View &, Input::Event e)
		{
			if(system().hasContent())
			{
				if(item.active())
				{
					pushAndShow(makeView<UnibiosSwitchesView>(), e);
				}
				else
				{
					app().postMessage(
						UI_TEXT("游戏必须和 Unibios 一起使用")
					);
				}
			}
		}
	};

	TextMenuItem options
	{
		UI_TEXT("主机选项"),
		attachParams(),
		[this](TextMenuItem &, View &, Input::Event e)
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
		item.emplace_back(&unibiosSwitches);
		item.emplace_back(&options);
		loadStandardItems();
	}

	void onShow()
	{
		SystemActionsView::onShow();
		bool isUnibios = conf.system >= SYS_UNIBIOS && conf.system <= SYS_UNIBIOS_LAST;
		unibiosSwitches.setActive(system().hasContent() && isUnibios);
	}
};

class CustomMainMenuView : public MainMenuView
{
private:
	TextMenuItem gameList
	{
		UI_TEXT("从列表打开游戏"),
		attachParams(),
		[this](TextMenuItem &, View &, Input::Event e)
		{
			auto gameListMenu = makeView<GameListView>();
			if(!gameListMenu->games())
			{
				app().postMessage(6, true,
					UI_TEXT("没有找到游戏，请选择“打开游戏”，浏览到带有 ROM 档案的文件夹。")
				);
				return;
			}
			pushAndShow(std::move(gameListMenu), e);
		}
	};

	void reloadItems() final
	{
		item.clear();
		loadFileBrowserItems();
		item.emplace_back(&gameList);
		loadStandardItems();
	}

public:
	CustomMainMenuView(ViewAttachParams attach): MainMenuView{attach, true}
	{
		reloadItems();
	}
};

std::unique_ptr<View> EmuApp::makeCustomView(ViewAttachParams attach, ViewID id)
{
	switch(id)
	{
		case ViewID::MAIN_MENU: return std::make_unique<CustomMainMenuView>(attach);
		case ViewID::SYSTEM_ACTIONS: return std::make_unique<CustomSystemActionsView>(attach);
		case ViewID::SYSTEM_OPTIONS: return std::make_unique<CustomSystemOptionView>(attach);
		default: return nullptr;
	}
}

}
