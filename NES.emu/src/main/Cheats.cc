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

#include <imagine/gui/TextEntry.hh>
#include <imagine/util/string.h>
#include <imagine/util/format.hh>
#include <imagine/logger/logger.h>
#include <emuframework/Cheats.hh>
#include <emuframework/EmuApp.hh>
#include <emuframework/viewUtils.hh>
#include "EmuCheatViews.hh"
#include <fceu/driver.h>
#include <fceu/cheat.h>

void EncodeGG(char *str, int a, int v, int c);

namespace EmuEx
{

extern unsigned fceuCheats;
static const int UNCHANGED_VAL = -2;

static bool isValidGGCodeLen(const char *str)
{
	return strlen(str) == 6 || strlen(str) == 8;
}

static auto cheatName(unsigned idx)
{
	std::string name;
	if(!FCEUI_GetCheat(idx, &name, 0, 0, 0, 0, 0)) [[unlikely]]
	{
		// return std::string{UI_TEXT("Corrupt Cheat")};
		return std::string{UI_TEXT("损坏的金手指")};
	}
	return name;
}

EmuEditCheatView::EmuEditCheatView(ViewAttachParams attach, unsigned cheatIdx, RefreshCheatsDelegate onCheatListChanged_):
	BaseEditCheatView
	{
		u"",
		attach,
		u"",
		[this](ItemMessage msg) -> ItemReply
		{
			return msg.visit(overloaded
			{
				[&](const ItemsMessage &m) -> ItemReply { return type ? 3uz : 5uz; },
				[&](const GetItemMessage &m) -> ItemReply
				{
					if(type)
					{
						switch(m.idx)
						{
							case 0: return &name;
							case 1: return &ggCode;
							default: return &remove;
						}
					}
					else
					{
						switch(m.idx)
						{
							case 0: return &name;
							case 1: return &addr;
							case 2: return &value;
							case 3: return &comp;
							default: return &remove;
						}
					}
				},
			});
		},
		[this](TextMenuItem &, View &, Input::Event)
		{
			assert(fceuCheats != 0);
			FCEUI_DelCheat(idx);
			fceuCheats--;
			onCheatListChanged();
			FCEU_FlushGameCheats(nullptr, 0, false);
			dismiss();
			return true;
		},
		onCheatListChanged_
	},
	addr
	{
		// UI_TEXT("Address"),
		UI_TEXT("地址"),
		u"",
		attachParams(),
		[this](DualTextMenuItem &item, View &, Input::Event e)
		{
			pushAndShowNewCollectValueInputView<const char*>(attachParams(), e,
				// UI_TEXT("Input 4-digit hex"),
				UI_TEXT("请输入四位十六进制数字"),
				addrStr,
				[this](CollectTextInputView&, auto str)
				{
					unsigned a = strtoul(str, nullptr, 16);
					if(a > 0xFFFF)
					{
						logMsg("addr 0x%X too large", a);
						// app().postMessage(true, UI_TEXT("Invalid input"));
						app().postMessage(true, UI_TEXT("无效的输入"));
						postDraw();
						return false;
					}
					addrStr = a ? str : "0";
					syncCheat();
					addr.set2ndName(addrStr);
					addr.place();
					postDraw();
					return true;
				}
			);
		}
	},
	value
	{
		// UI_TEXT("Value"),
		UI_TEXT("数值"),
		u"",
		attachParams(),
		[this](DualTextMenuItem &item, View &, Input::Event e)
		{
			pushAndShowNewCollectValueInputView<const char*>(attachParams(), e,
				// UI_TEXT("Input 2-digit hex"),
				UI_TEXT("请输入两位十六进制数字"),
				valueStr,
				[this](CollectTextInputView&, auto str)
				{
					unsigned a = strtoul(str, nullptr, 16);
					if(a > 0xFF)
					{
						logMsg("val 0x%X too large", a);
						// app().postMessage(true, UI_TEXT("Invalid input"));
						app().postMessage(true, UI_TEXT("无效的输入"));
						postDraw();
						return false;
					}
					valueStr = a ? str : "0";
					syncCheat();
					value.set2ndName(valueStr);
					value.place();
					postDraw();
					return true;
				}
			);
		}
	},
	comp
	{
		// UI_TEXT("Compare"),
		UI_TEXT("比较"),
		u"",
		attachParams(),
		[this](DualTextMenuItem &item, View &, Input::Event e)
		{
			pushAndShowNewCollectTextInputView(attachParams(), e,
				// UI_TEXT("Input 2-digit hex or blank"),
				UI_TEXT("请输入两位十六进制数字或留空"),
				compStr.data(),
				[this](CollectTextInputView &view, const char *str)
				{
					if(str)
					{
						if(strlen(str))
						{
							unsigned a = strtoul(str, nullptr, 16);
							if(a > 0xFF)
							{
								logMsg("val 0x%X too large", a);
								// app().postMessage(true, UI_TEXT("Invalid input"));
								app().postMessage(true, UI_TEXT("无效的输入"));
								return true;
							}
							compStr = str;
							comp.set2ndName(str);
						}
						else
						{
							compStr.clear();
							comp.set2ndName();
						}
						syncCheat();
						comp.place();
						postDraw();
					}
					view.dismiss();
					return false;
				});
		}
	},
	ggCode
	{
		// UI_TEXT("GG Code"),
		UI_TEXT("金手指代码"),
		u"",
		attachParams(),
		[this](DualTextMenuItem &item, View &, Input::Event e)
		{
			pushAndShowNewCollectValueInputView<const char*>(attachParams(), e,
				// UI_TEXT("Input Game Genie code"),
				UI_TEXT("请输入金手指代码"),
				ggCodeStr,
				[this](CollectTextInputView&, auto str)
				{
					if(!isValidGGCodeLen(str))
					{
						// app().postMessage(true, UI_TEXT("Invalid, must be 6 or 8 digits"));
						app().postMessage(true, UI_TEXT("金手指代码必须是六位或八位的数字"));
						return false;
					}
					ggCodeStr = str;
					syncCheat();
					ggCode.set2ndName(str);
					ggCode.place();
					postDraw();
					return true;
				}
			);
		}
	},
	idx{cheatIdx}
{
	uint32 a;
	uint8 v;
	int compare;
	{
		std::string nameStr{};
		int gotCheat = FCEUI_GetCheat(cheatIdx, &nameStr, &a, &v, &compare, 0, &type);
		logMsg("got cheat with addr 0x%.4x val 0x%.2x comp %d", a, v, compare);
		name.setName(std::move(nameStr));
	}
	if(type)
	{
		// resetName(UI_TEXT("Edit Code"));
		resetName(UI_TEXT("编辑金手指代码"));
		if(a == 0 && v == 0 && compare == -1)
			ggCodeStr.clear();
		else
		{
			std::array<char, 9> str;
			EncodeGG(str.data(), a, v, compare);
			ggCodeStr = str.data();
		}
		ggCode.set2ndName(ggCodeStr);
	}
	else
	{
		// resetName(UI_TEXT("Edit RAM Patch"));
		resetName(UI_TEXT("编辑内存补丁"));
		IG::formatTo(addrStr, "{:x}", a);
		addr.set2ndName(addrStr);
		IG::formatTo(valueStr, "{:x}", v);
		value.set2ndName(valueStr);
		if(compare == -1)
			compStr.clear();
		else
		{
			IG::formatTo(compStr, "{:x}", compare);
			comp.set2ndName(compStr);
		}
	}
}

void EmuEditCheatView::syncCheat(std::string_view newName)
{
	if(type)
	{
		int a, v, c;
		if(!FCEUI_DecodeGG(ggCodeStr.data(), &a, &v, &c))
		{
			logWarn("error decoding GG code %s", ggCodeStr.data());
			a = 0; v = 0; c = -1;
		}
		if(!FCEUI_SetCheat(idx, newName, a, v, c, -1, 1))
		{
			logWarn("error setting cheat %d", idx);
		}
	}
	else
	{
		int comp = compStr.size() ? strtoul(compStr.data(), nullptr, 16) : -1;
		logMsg("setting comp %d", comp);
		if(!FCEUI_SetCheat(idx,
				newName, strtoul(addrStr.data(), nullptr, 16), strtoul(valueStr.data(), nullptr, 16),
				comp, -1, 0))
		{
			logWarn("error setting cheat %d", idx);
		}
	}
	FCEU_FlushGameCheats(nullptr, 0, false);
}

std::string EmuEditCheatView::cheatNameString() const
{
	return cheatName(idx);
}

void EmuEditCheatView::renamed(std::string_view str)
{
	syncCheat(str);
}

void EmuEditCheatListView::loadCheatItems()
{
	auto cheats = fceuCheats;
	cheat.clear();
	cheat.reserve(cheats);
	for(auto c : iotaCount(cheats))
	{
		cheat.emplace_back(cheatName(c), attachParams(),
			[this, c](TextMenuItem &, View &, Input::Event e)
			{
				pushAndShow(makeView<EmuEditCheatView>(c, [this](){ onCheatListChanged(); }), e);
			});
	}
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
						case 0: return &addGG;
						case 1: return &addRAM;
						default: return &cheat[m.idx - 2];
					}
				},
			});
		}
	},
	addGG
	{
		// UI_TEXT("Add Game Genie Code"),
		UI_TEXT("添加金手指代码"),
		attachParams(),
		[this](TextMenuItem &item, View &, Input::Event e)
		{
			pushAndShowNewCollectTextInputView(attachParams(), e,
				// UI_TEXT("Input Game Genie code"),
				UI_TEXT("请输入金手指代码"),
				"",
				[this](CollectTextInputView &view, const char *str)
				{
					if(str)
					{
						if(!isValidGGCodeLen(str))
						{
							// app().postMessage(true, UI_TEXT("Invalid, must be 6 or 8 digits"));
							app().postMessage(true, UI_TEXT("金手指代码必须是六位或八位的数字"));
							return true;
						}
						{
							int a, v, c;
							if(!FCEUI_DecodeGG(str, &a, &v, &c))
							{
								// app().postMessage(true, UI_TEXT("Invalid code"));
								app().postMessage(true, UI_TEXT("无效的代码"));
								return true;
							}
							//if(!FCEUI_AddCheat(UI_TEXT("Unnamed Cheat"), a, v, c, 1))
							if (!FCEUI_AddCheat(UI_TEXT("新建的金手指"), a, v, c, 1))
							{
								// app().postMessage(true, UI_TEXT("Error adding cheat"));
								app().postMessage(true, UI_TEXT("添加金手指时出错"));
								view.dismiss();
								return false;
							}
						}
						fceuCheats++;
						FCEUI_ToggleCheat(fceuCheats-1);
						logMsg("added new cheat, %d total", fceuCheats);
						FCEU_FlushGameCheats(nullptr, 0, false);
						view.dismiss();
						pushAndShowNewCollectTextInputView(attachParams(), {},
							// UI_TEXT("Input description"),
							UI_TEXT("请输入描述说明"),
							"",
							[this](CollectTextInputView &view, const char *str)
							{
								if(str)
								{
									FCEUI_SetCheat(fceuCheats-1, str, UNCHANGED_VAL, UNCHANGED_VAL, UNCHANGED_VAL, -1, 1);
									onCheatListChanged();
									FCEU_FlushGameCheats(nullptr, 0, false);
									view.dismiss();
								}
								else
								{
									view.dismiss();
								}
								return false;
							}
						);
					}
					else
					{
						view.dismiss();
					}
					return false;
				});
		}
	},
	addRAM
	{
		// UI_TEXT("Add RAM Patch"),
		UI_TEXT("添加内存补丁"),
		attachParams(),
		[this](TextMenuItem &item, View &, Input::Event e)
		{
			pushAndShowNewCollectTextInputView(attachParams(), e,
				// UI_TEXT("Input description"),
				UI_TEXT("请输入描述说明"),
				"",
				[this](CollectTextInputView &view, const char *str)
				{
					if(str)
					{
						if(!FCEUI_AddCheat(str, 0, 0, -1, 0))
						{
							logErr("error adding new cheat");
							view.dismiss();
							return false;
						}
						fceuCheats++;
						FCEUI_ToggleCheat(fceuCheats-1);
						logMsg("added new cheat, %d total", fceuCheats);
						onCheatListChanged();
						FCEU_FlushGameCheats(nullptr, 0, false);
						auto editCheatView = makeView<EmuEditCheatView>(fceuCheats-1, [this](){ onCheatListChanged(); });
						view.dismiss();
						pushAndShow(std::move(editCheatView), {});
					}
					else
					{
						view.dismiss();
					}
					return false;
				});
		}
	}
{
	loadCheatItems();
}

EmuCheatsView::EmuCheatsView(ViewAttachParams attach): BaseCheatsView{attach}
{
	loadCheatItems();
}

void EmuCheatsView::loadCheatItems()
{
	auto cheats = fceuCheats;
	cheat.clear();
	cheat.reserve(cheats);
	for(auto c : iotaCount(cheats))
	{
		std::string name;
		int status = 0;
		if(!FCEUI_GetCheat(c, &name, 0, 0, 0, &status, 0)) [[unlikely]]
		{
			// name = UI_TEXT("Corrupt Cheat");
			name = UI_TEXT("损坏的金手指");
		}
		cheat.emplace_back(std::move(name), attachParams(), status,
			[this, c](BoolMenuItem &item)
			{
				uint32 a;
				uint8 v;
				int compare, type;
				int gotCheat = FCEUI_GetCheat(c, nullptr, &a, &v, &compare, 0, &type);
				if(!gotCheat)
					return;
				if(!item.boolValue() && type && a == 0 && v == 0 && compare == -1)
				{
					// Don't turn on null Game Genie codes
					// app().postMessage(true, UI_TEXT("Game Genie code isn't set"));
					app().postMessage(true, UI_TEXT("没有设置金手指代码"));
					return;
				}
				item.flipBoolValue(*this);
				FCEUI_ToggleCheat(c);
				FCEU_FlushGameCheats(nullptr, 0, false);
			});
	}
}

}
