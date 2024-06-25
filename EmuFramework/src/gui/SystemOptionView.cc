/*  This file is part of EmuFramework.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with EmuFramework.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/SystemOptionView.hh>
#include <emuframework/EmuApp.hh>
#include <emuframework/EmuOptions.hh>
#include <emuframework/viewUtils.hh>
#include "CPUAffinityView.hh"
#include <imagine/base/ApplicationContext.hh>
#include <imagine/gui/TextTableView.hh>
#include <imagine/fs/FS.hh>
#include <format>

namespace EmuEx
{

SystemOptionView::SystemOptionView(ViewAttachParams attach, bool customMenu):
	TableView
	{
		// UI_TEXT("System Options"),
		UI_TEXT("系统选项"),
		attach,
		item
	},
	autosaveTimerItem
	{
		{
			// UI_TEXT("Off"),
			UI_TEXT("关"),
			attach,
			{.id = 0}
		},
		{
			// UI_TEXT("5min"),
			UI_TEXT("5 分钟"),
			attach,
			{.id = 5}
		},
		{
			// UI_TEXT("10min"),
			UI_TEXT("10 分钟"),
			attach,
			{.id = 10}
		},
		{
			// UI_TEXT("15min"),
			UI_TEXT("15 分钟"),
			attach,
			{.id = 15}
		},
		{
			// UI_TEXT("Custom Value"),
			UI_TEXT("自定义"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShowNewCollectValueRangeInputView<int, 0, maxAutosaveSaveFreq.count()>(
					attachParams(), e,
					// UI_TEXT("Input 0 to 720"),
					UI_TEXT("请输入 0 到 720 之间的值"),
					"",
					[this](CollectTextInputView &, auto val)
					{
						app().autosaveManager.saveTimer.frequency = Minutes{val};
						autosaveTimer.setSelected(MenuId{val}, *this);
						dismissPrevious();
						return true;
					});
				return false;
			}, {.id = defaultMenuId}
		},
	},
	autosaveTimer
	{
		// UI_TEXT("Autosave Timer"),
		UI_TEXT("自动存档计时"),
		attach,
		MenuId{app().autosaveManager.saveTimer.frequency.count()},
		autosaveTimerItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(!idx)
					return false;
				t.resetString(std::format("{}", app().autosaveManager.saveTimer.frequency));
				return true;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item) { app().autosaveManager.saveTimer.frequency = IG::Minutes{item.id}; }
		},
	},
	autosaveLaunchItem
	{
		{
			// UI_TEXT("Main Slot"),
			UI_TEXT("默认存档点 (自动读取进度)"),
			attach,
			{.id = AutosaveLaunchMode::Load}
		},
		{
			// UI_TEXT("Main Slot (No State)"),
			UI_TEXT("默认存档点 (不读取进度)"),
			attach,
			{.id = AutosaveLaunchMode::LoadNoState}
		},
		{
			// UI_TEXT("No Save Slot"),
			UI_TEXT("不自动存档"),
			attach,
			{.id = AutosaveLaunchMode::NoSave}
		},
		{
			// UI_TEXT("Select Slot"),
			UI_TEXT("选择存档点"),
			attach,
			{.id = AutosaveLaunchMode::Ask}
		},
	},
	autosaveLaunch
	{
		// UI_TEXT("Autosave Launch Mode"),
		UI_TEXT("自动存档启动模式"),
		attach,
		MenuId{app().autosaveManager.autosaveLaunchMode},
		autosaveLaunchItem,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item) { app().autosaveManager.autosaveLaunchMode = AutosaveLaunchMode(item.id.val); }
		},
	},
	autosaveContent
	{
		// UI_TEXT("Autosave Content"),
		UI_TEXT("自动存档内容"),
		attach,
		app().autosaveManager.saveOnlyBackupMemory,
		// UI_TEXT("State & Backup RAM"),
		UI_TEXT("保存进度和内存快照"),
		// UI_TEXT("Only Backup RAM"),
		UI_TEXT("仅保存内存快照"),
		[this](BoolMenuItem &item)
		{
			app().autosaveManager.saveOnlyBackupMemory = item.flipBoolValue(*this);
		}
	},
	confirmOverwriteState
	{
		// UI_TEXT("Confirm Overwrite State"),
		UI_TEXT("确认后再覆盖存档"),
		attach,
		app().confirmOverwriteState,
		[this](BoolMenuItem &item)
		{
			app().confirmOverwriteState = item.flipBoolValue(*this);
		}
	},
	fastModeSpeedItem
	{
		{"1.5x",  attach, {.id = 150}},
		{"2x",    attach, {.id = 200}},
		{"4x",    attach, {.id = 400}},
		{"8x",    attach, {.id = 800}},
		{"16x",   attach, {.id = 1600}},
		{
			// UI_TEXT("Custom Value"),
			UI_TEXT("自定义"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShowNewCollectValueRangeInputView<float, 1, 20>(
					attachParams(), e,
					// UI_TEXT("Input above 1.0 to 20.0"),
					UI_TEXT("请输入 1.0 到 20.0 之间的值"),
					"",
					[this](CollectTextInputView &, auto val)
					{
						auto valAsInt = std::round(val * 100.f);
						app().setAltSpeed(AltSpeedMode::fast, valAsInt);
						fastModeSpeed.setSelected(MenuId{valAsInt}, *this);
						dismissPrevious();
						return true;
					});
				return false;
			}, {.id = defaultMenuId}
		},
	},
	fastModeSpeed
	{
		// UI_TEXT("Fast-forward Speed"),
		UI_TEXT("快进速度"),
		attach,
		MenuId{app().altSpeed(AltSpeedMode::fast)},
		fastModeSpeedItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format("{:g}x", app().altSpeedAsDouble(AltSpeedMode::fast)));
				return true;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item) { app().setAltSpeed(AltSpeedMode::fast, item.id); }
		},
	},
	slowModeSpeedItem
	{
		{"0.25x", attach, {.id = 25}},
		{"0.50x", attach, {.id = 50}},
		{
			// UI_TEXT("Custom Value"),
			UI_TEXT("自定义"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShowNewCollectValueInputView<float>(
					attachParams(), e,
					// UI_TEXT("Input 0.05 up to 1.0"),
					UI_TEXT("请输入 0.05 到 1.0 之间的值"),
					"",
					[this](CollectTextInputView &, auto val)
					{
						auto valAsInt = std::round(val * 100.f);
						if(app().setAltSpeed(AltSpeedMode::slow, valAsInt))
						{
							slowModeSpeed.setSelected(MenuId{valAsInt}, *this);
							dismissPrevious();
							return true;
						}
						else
						{
							// app().postErrorMessage(UI_TEXT("Value not in range"));
							app().postErrorMessage(UI_TEXT("输入值不在有效的取值范围"));
							return false;
						}
					});
				return false;
			}, {.id = defaultMenuId}
		},
	},
	slowModeSpeed
	{
		// UI_TEXT("Slow-motion Speed"),
		UI_TEXT("慢动作速度"),
		attach,
		MenuId{app().altSpeed(AltSpeedMode::slow)},
		slowModeSpeedItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format("{:g}x", app().altSpeedAsDouble(AltSpeedMode::slow)));
				return true;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item) { app().setAltSpeed(AltSpeedMode::slow, item.id); }
		},
	},
	rewind
	{
		UI_TEXT("进度回放："),
		attach
	},
	rewindStatesItem
	{
		{"0",  attach, {.id = 0}},
		{"30", attach, {.id = 30}},
		{"60", attach, {.id = 60}},
		{
			// UI_TEXT("Custom Value"),
			UI_TEXT("自定义"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShowNewCollectValueRangeInputView<int, 0, 50000>(
					attachParams(), e,
					// UI_TEXT("Input 0 to 50000"),
					UI_TEXT("请输入 0 到 50000 之间的值"),
					std::to_string(app().rewindManager.maxStates),
					[this](CollectTextInputView &, auto val)
					{
						app().rewindManager.updateMaxStates(val);
						rewindStates.setSelected(val, *this);
						dismissPrevious();
						return true;
					});
				return false;
			}, {.id = defaultMenuId}
		},
	},
	rewindStates
	{
		// UI_TEXT("Rewind States"),
		UI_TEXT("存档点最大数量"),
		attach,
		MenuId{app().rewindManager.maxStates},
		rewindStatesItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format("{}", app().rewindManager.maxStates));
				return true;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item) { app().rewindManager.updateMaxStates(item.id); }
		},
	},
	rewindTimeInterval
	{
		// UI_TEXT("Rewind State Interval (Seconds)"),
		UI_TEXT("存档点时间间隔 (秒)"),
		std::to_string(app().rewindManager.saveTimer.frequency.count()),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShowNewCollectValueRangeInputView<int, 1, 60>(
				attachParams(), e,
				// UI_TEXT("Input 1 to 60"),
				UI_TEXT("请输入 1 到 60 之间的值"),
				std::to_string(app().rewindManager.saveTimer.frequency.count()),
				[this](CollectTextInputView &, auto val)
				{
					app().rewindManager.saveTimer.frequency = Seconds{val};
					rewindTimeInterval.set2ndName(std::to_string(val));
					return true;
				});
		}
	},
	performanceMode
	{
		// UI_TEXT("Performance Mode"),
		UI_TEXT("性能模式"),
		attach,
		app().useSustainedPerformanceMode,
		// UI_TEXT("Normal"),
		UI_TEXT("普通"),
		// UI_TEXT("Sustained"),
		UI_TEXT("持续"),
		[this](BoolMenuItem &item)
		{
			app().useSustainedPerformanceMode = item.flipBoolValue(*this);
		}
	},
	noopThread
	{
		// UI_TEXT("No-op Thread (Experimental)"),
		UI_TEXT("无操作线程"),
		attach,
		(bool)app().useNoopThread,
		[this](BoolMenuItem &item)
		{
			app().useNoopThread = item.flipBoolValue(*this);
		}
	},
	cpuAffinity
	{
		// UI_TEXT("Configure CPU Affinity"),
		UI_TEXT("CPU 亲和性配置"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShow(makeView<CPUAffinityView>(appContext().cpuCount()), e);
		}
	}
{
	if(!customMenu)
	{
		loadStockItems();
	}
}

void SystemOptionView::loadStockItems()
{
	item.emplace_back(&autosaveLaunch);
	item.emplace_back(&autosaveTimer);
	item.emplace_back(&autosaveContent);
	item.emplace_back(&confirmOverwriteState);
	item.emplace_back(&fastModeSpeed);
	item.emplace_back(&slowModeSpeed);
	if(used(performanceMode) && appContext().hasSustainedPerformanceMode())
		item.emplace_back(&performanceMode);
	if(used(noopThread))
		item.emplace_back(&noopThread);
	if(used(cpuAffinity) && appContext().cpuCount() > 1)
		item.emplace_back(&cpuAffinity);
	item.emplace_back(&rewind);
	item.emplace_back(&rewindStates);
	item.emplace_back(&rewindTimeInterval);
}

}
