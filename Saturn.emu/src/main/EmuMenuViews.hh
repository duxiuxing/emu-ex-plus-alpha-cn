#pragma once
#include <emuframework/OptionView.hh>
#include <emuframework/MenuView.hh>

class SystemOptionView : public OptionView
{
public:

	char biosPathStr[256] {0};
	TextMenuItem biosPath
	{
		"",
		[this](TextMenuItem &, View &, Input::Event e)
		{
			auto &biosSelectMenu = *new BiosSelectMenu{"BIOS", &::biosPath, hasBIOSExtension, window()};
			biosSelectMenu.init();
			biosSelectMenu.onBiosChange() =
				[this]()
				{
					logMsg("set bios %s", ::biosPath.data());
					printBiosMenuEntryStr(biosPathStr);
					biosPath.compile(projP);
				};
			viewStack.pushAndShow(biosSelectMenu, e);
		}
	};

	template <size_t S>
	static void printBiosMenuEntryStr(char (&str)[S])
	{
		string_printf(str, "BIOS: %s", strlen(::biosPath.data()) ? FS::basename(::biosPath).data() : "None set");
	}

	MultiChoiceSelectMenuItem sh2Core
	{
		"SH2",
		[](MultiChoiceMenuItem &, View &, int val)
		{
			assert(val < (int)IG::size(SH2CoreList)-1);
			yinit.sh2coretype = SH2CoreList[val]->id;
			optionSH2Core = SH2CoreList[val]->id;
		}
	};

	void sh2CoreInit()
	{
		static const char *str[6];

		int setting = 0, cores = 0;
		iterateTimes(IG::size(SH2CoreList)-1, i)
		{
			if(i == IG::size(str))
				break;
			str[i] = SH2CoreList[i]->Name;
			if(SH2CoreList[i]->id == yinit.sh2coretype)
				setting = i;
			cores++;
		}

		sh2Core.init(str, setting, cores);
	}

public:
	SystemOptionView(Base::Window &win):
		OptionView(win)
	{}

	void loadSystemItems(MenuItem *item[], uint &items)
	{
		OptionView::loadSystemItems(item, items);
		if(IG::size(SH2CoreList) > 2)
		{
			sh2CoreInit(); item[items++] = &sh2Core;
		}
		printBiosMenuEntryStr(biosPathStr);
		biosPath.init(biosPathStr); item[items++] = &biosPath;
	}
};

class SystemMenuView : public MenuView
{
public:
	SystemMenuView(Base::Window &win): MenuView(win) {}
};
