/*  This file is part of GBA.emu.

	GBA.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	GBA.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with GBA.emu.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/Cheats.hh>
#include <emuframework/EmuApp.hh>
#include <emuframework/viewUtils.hh>
#include "EmuCheatViews.hh"
#include "MainSystem.hh"
#include "GBASys.hh"
#include <imagine/fs/FS.hh>
#include <imagine/gui/TextEntry.hh>
#include <imagine/util/string.h>
#include <imagine/util/format.hh>
#include <imagine/logger/logger.h>
#include <core/gba/gbaCheats.h>
#include <core/gba/gba.h>

namespace EmuEx
{

static void writeCheatFile(EmuSystem &);

EmuEditCheatView::EmuEditCheatView(ViewAttachParams attach, int cheatIdx, RefreshCheatsDelegate onCheatListChanged_):
	BaseEditCheatView
	{
		UI_TEXT("编辑金手指代码"),
		attach,
		cheatsList[cheatIdx].desc,
		items,
		[this](TextMenuItem &item, View &parent, Input::Event e)
		{
			cheatsDelete(gGba.cpu, idx, true);
			onCheatListChanged();
			writeCheatFile(system());
			dismiss();
			return true;
		},
		onCheatListChanged_
	},
	items{&name, &code, &remove},
	code
	{
		UI_TEXT("金手指代码"),
		cheatsList[cheatIdx].codestring,
		attach,
		[this](DualTextMenuItem &, View &, Input::Event)
		{
			app().postMessage(
				UI_TEXT("要更改此代码，请删除并重新添加它")
			);
		}
	},
	idx{cheatIdx}
{}

std::string_view EmuEditCheatView::cheatNameString() const
{
	return cheatsList[idx].desc;
}

void EmuEditCheatView::renamed(std::string_view str)
{
	auto &cheat = cheatsList[idx];
	strncpy(cheat.desc, str.data(), sizeof(cheat.desc));
	writeCheatFile(system());
}

void EmuEditCheatListView::loadCheatItems()
{
	cheat.clear();
	cheat.reserve(cheatsList.size());
	for(auto &c : cheatsList)
	{
		cheat.emplace_back(c.desc, attachParams(),
			[this, idx = std::distance(cheatsList.data(), &c)](Input::Event e)
			{
				pushAndShow(makeView<EmuEditCheatView>(idx, [this](){ onCheatListChanged(); }), e);
			});
	}
}

void EmuEditCheatListView::addNewCheat(int isGSv3)
{
	if(cheatsList.size() == cheatsList.capacity())
	{
		app().postMessage(true,
			UI_TEXT("已达个数上限，请先删除一些再添加")
		);
		return;
	}
	pushAndShowNewCollectTextInputView(attachParams(), {},
		isGSv3
			? UI_TEXT("请输入 xxxxxxxx yyyyyyyy")
			: UI_TEXT("请输入 GS 码 (xxxxxxxx yyyyyyyy) 或 AR 码 (xxxxxxxx yyyy)"),
		"",
		[this, isGSv3](CollectTextInputView &view, const char *str)
		{
			if(str)
			{
				auto tempStr{IG::toUpperCase(str)};
				if(tempStr.size() == 17 && tempStr[8] == ' ')
				{
					logMsg("removing middle space in text");
					tempStr.erase(tempStr.begin() + 8);
				}
				if(isGSv3 ?
					cheatsAddGSACode(gGba.cpu, tempStr.data(), UNNAMED_CHEAT, true) :
					((tempStr.size() == 16 && cheatsAddGSACode(gGba.cpu, tempStr.data(), UNNAMED_CHEAT, false))
					|| cheatsAddCBACode(gGba.cpu, tempStr.data(), UNNAMED_CHEAT)))
				{
					logMsg("added new cheat, %d total", (int)cheatsList.size());
				}
				else
				{
					app().postMessage(true,
						UI_TEXT("无效的格式")
					);
					return true;
				}
				cheatsDisable(gGba.cpu, cheatsList.size()-1);
				onCheatListChanged();
				writeCheatFile(system());
				view.dismiss();
				pushAndShowNewCollectTextInputView(attachParams(), {},
					UI_TEXT("请输入描述说明"),
					"",
					[this](CollectTextInputView &view, const char *str)
					{
						if(str)
						{
							auto &cheat = cheatsList[cheatsList.size()-1];
							strncpy(cheat.desc, str, sizeof(cheat.desc));
							onCheatListChanged();
							view.dismiss();
						}
						else
						{
							view.dismiss();
						}
						return false;
					});
			}
			else
			{
				view.dismiss();
			}
			return false;
		});
}

EmuEditCheatListView::EmuEditCheatListView(ViewAttachParams attach):
	BaseEditCheatListView
	{
		attach,
		[this](ItemMessage msg) -> ItemReply
		{
			return msg.visit(overloaded
			{
				[&](const ItemsMessage &m) -> ItemReply { return 2 + cheat.size(); },
				[&](const GetItemMessage &m) -> ItemReply
				{
					switch(m.idx)
					{
						case 0: return &addGS12CBCode;
						case 1: return &addGS3Code;
						default: return &cheat[m.idx - 2];
					}
				},
			});
		}
	},
	addGS12CBCode
	{
		UI_TEXT("添加 GS v1-2/CB 码"),
		attach,
		[this](TextMenuItem &item, View &, Input::Event e)
		{
			addNewCheat(false);
		}
	},
	addGS3Code
	{
		UI_TEXT("添加 GS v3 码"),
		attach,
		[this](TextMenuItem &item, View &, Input::Event e)
		{
			addNewCheat(true);
		}
	}
{
	loadCheatItems();
}

void EmuCheatsView::loadCheatItems()
{
	cheat.clear();
	cheat.reserve(cheatsList.size());
	for(auto &c : cheatsList)
	{
		cheat.emplace_back(c.desc, attachParams(), c.enabled,
			[this, idx = std::distance(cheatsList.data(), &c)](BoolMenuItem &item, Input::Event e)
			{
				bool on = item.flipBoolValue(*this);
				if(on)
					cheatsEnable(idx);
				else
					cheatsDisable(gGba.cpu, idx);
				writeCheatFile(system());
			});
	}
}

EmuCheatsView::EmuCheatsView(ViewAttachParams attach): BaseCheatsView{attach}
{
	loadCheatItems();
}

static void writeCheatFile(EmuSystem &sys_)
{
	auto &sys = static_cast<GbaSystem&>(sys_);
	auto ctx = sys.appContext();
	auto filename = sys.userFilePath(sys.cheatsDir, ".clt");
	if(cheatsList.empty())
	{
		logMsg("deleting cheats file %s", filename.data());
		ctx.removeFileUri(filename);
		return;
	}
	cheatsSaveCheatList(ctx, filename.data());
}

void readCheatFile(EmuSystem &sys_)
{
	auto &sys = static_cast<GbaSystem&>(sys_);
	auto filename = sys.userFilePath(sys.cheatsDir, ".clt");
	if(cheatsLoadCheatList(sys.appContext(), filename.data()))
	{
		logMsg("loaded cheat file: %s", filename.data());
	}
}

}
