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

#include <emuframework/SystemActionsView.hh>
#include <emuframework/EmuApp.hh>
#include <emuframework/EmuSystem.hh>
#include <emuframework/EmuVideo.hh>
#include <emuframework/EmuViewController.hh>
#include <emuframework/CreditsView.hh>
#include <emuframework/StateSlotView.hh>
#include <emuframework/InputManagerView.hh>
#include <emuframework/BundledGamesView.hh>
#include <emuframework/viewUtils.hh>
#include "AutosaveSlotView.hh"
#include "ResetAlertView.hh"
#include <imagine/gui/TextEntry.hh>
#include <imagine/base/ApplicationContext.hh>
#include <imagine/util/format.hh>
#include <imagine/logger/logger.h>

namespace EmuEx
{

constexpr SystemLogger log{"SystemActionsView"};

static auto autoSaveName(EmuApp &app)
{
	return std::format(
		// UI_TEXT("Autosave Slot ({})"),
		UI_TEXT("当前的自动存档点：{}"),
		app.autosaveManager.slotFullName()
	);
}

static std::string saveAutosaveName(EmuApp &app)
{
	auto &autosaveManager = app.autosaveManager;
	if(!autosaveManager.timerFrequency().count())
		// return UI_TEXT("Save Autosave State");
		return UI_TEXT("保存自动存档的进度");
	return std::format(
		// UI_TEXT("Save Autosave State (Timer In {:%M:%S})"),
		UI_TEXT("保存自动存档的进度 (倒计时 {:%M:%S})"),
		duration_cast<Seconds>(autosaveManager.saveTimer.nextFireTime())
	);
}

SystemActionsView::SystemActionsView(ViewAttachParams attach, bool customMenu):
	TableView
	{
		// UI_TEXT("System Actions"),
		UI_TEXT("游戏菜单"),
		attach,
		item
	},
	cheats
	{
		// UI_TEXT("Cheats"),
		UI_TEXT("金手指"),
		attach,
		[this](const Input::Event &e)
		{
			if(system().hasContent())
			{
				pushAndShow(app().makeView(attachParams(), EmuApp::ViewID::LIST_CHEATS), e);
			}
		}
	},
	reset
	{
		// UI_TEXT("Reset"),
		UI_TEXT("重启游戏"),
		attach,
		[this](const Input::Event &e)
		{
			if(!system().hasContent())
				return;
			pushAndShowModal(resetAlertView(attachParams(), app()), e);
		}
	},
	autosaveSlot
	{
		autoSaveName(app()), attach,
		[this](const Input::Event &e) { pushAndShow(makeView<AutosaveSlotView>(), e); }
	},
	autosaveNow
	{
		saveAutosaveName(app()), attach,
		[this](TextMenuItem &item, const Input::Event &e)
		{
			if(!item.active())
				return;
			pushAndShowModal(
				makeView<YesNoAlertView>(
					// UI_TEXT("Really save state?"),
					UI_TEXT("是否要保存进度？"),
					YesNoAlertView::Delegates
					{
						.onYes = [this]
						{
							if(app().autosaveManager.save(AutosaveActionSource::Manual))
								app().showEmulation();
						}
					}
				), e
			);
		}
	},
	revertAutosave
	{
		// UI_TEXT("Load Autosave State"),
		UI_TEXT("读取自动存档的进度"),
		attach,
		[this](TextMenuItem &item, const Input::Event &e)
		{
			if(!item.active())
				return;
			auto saveTime = app().autosaveManager.stateTimeAsString();
			if(saveTime.empty())
			{
				// app().postMessage(UI_TEXT("No saved state"));
				app().postMessage(UI_TEXT("无效的存档进度"));
				return;
			}
			pushAndShowModal(
				makeView<YesNoAlertView>(
					std::format(
						// UI_TEXT("Really load state from: {}?"),
						UI_TEXT("是否要从 {} 读取进度？"),
						saveTime
					),
					YesNoAlertView::Delegates
					{
						.onYes = [this]
						{
							if(app().autosaveManager.load(AutosaveActionSource::Manual))
								app().showEmulation();
						}
					}
				), e
			);
		}
	},
	stateSlot
	{
		// UI_TEXT("Manual Save States"),
		UI_TEXT("手动存档点"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShow(makeView<StateSlotView>(), e);
		}
	},
	addLauncherIcon
	{
		// UI_TEXT("Add Content Shortcut To Launcher"),
		UI_TEXT("添加游戏快捷方式到桌面"),
		attach,
		[this](const Input::Event &e)
		{
			if(!system().hasContent())
				return;
			if(system().contentDirectory().empty())
			{
				// shortcuts to bundled games not yet supported
				return;
			}
			pushAndShowNewCollectValueInputView<const char*>(
				attachParams(), e,
				// UI_TEXT("Shortcut Name"),
				UI_TEXT("快捷方式名称"),
				system().contentDisplayName(),
				[this](CollectTextInputView &, auto str)
				{
					appContext().addLauncherIcon(str, system().contentLocation());
					app().postMessage(
						2, false,
						std::format(
							// UI_TEXT("Added shortcut:\n{}"),
							UI_TEXT("已添加快捷方式：\n{}"),
							str
						)
					);
					return true;
				}
			);
		}
	},
	screenshot
	{
		// UI_TEXT("Screenshot Next Frame"),
		UI_TEXT("截图"),
		attach,
		[this](const Input::Event &e)
		{
			if(!system().hasContent())
				return;
			auto pathName = appContext().fileUriDisplayName(app().screenshotDirectory());
			if(pathName.empty())
			{
				// app().postMessage(UI_TEXT("Save path isn't valid"));
				app().postMessage(UI_TEXT("无效的截图保存路径"));
				return;
			}
			pushAndShowModal(
				makeView<YesNoAlertView>(
					std::format(
						// UI_TEXT("Save screenshot to folder {}?"),
						UI_TEXT("是否把截图保存到文件夹 {}？"),
						pathName
					),
					YesNoAlertView::Delegates
					{
						.onYes = [this]
						{
							app().video.takeGameScreenshot();
							system().runFrame({}, &app().video, nullptr);
						}
					}
				), e
			);
		}
	},
	resetSessionOptions
	{
		// UI_TEXT("Reset Saved Options"),
		UI_TEXT("重置选项"),
		attach,
		[this](const Input::Event &e)
		{
			if(!app().hasSavedSessionOptions())
				return;
			pushAndShowModal(
				makeView<YesNoAlertView>(
					// UI_TEXT("Reset saved options for the currently running system to defaults? Some options only take effect next time the system loads."),
					UI_TEXT("是否要将选项恢复默认值？某些选项需要重启模拟器才能生效。"),
					YesNoAlertView::Delegates
					{
						.onYes = [this]
						{
							resetSessionOptions.setActive(false);
							app().deleteSessionOptions();
						}
					}
				), e
			);
		}
	},
	close
	{
		// UI_TEXT("Close Content"),
		UI_TEXT("关闭游戏"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShowModal(app().makeCloseContentView(), e);
		}
	}
{
	if(!customMenu)
	{
		loadStandardItems();
	}
}

void SystemActionsView::onShow()
{
	if(app().viewController().isShowingEmulation())
		return;
	TableView::onShow();
	log.info("refreshing action menu state");
	assert(system().hasContent());
	autosaveSlot.compile(autoSaveName(app()));
	autosaveNow.compile(saveAutosaveName(app()));
	autosaveNow.setActive(app().autosaveManager.slotName() != noAutosaveName);
	revertAutosave.setActive(app().autosaveManager.slotName() != noAutosaveName);
	resetSessionOptions.setActive(app().hasSavedSessionOptions());
}

void SystemActionsView::loadStandardItems()
{
	if(EmuSystem::hasCheats)
	{
		item.emplace_back(&cheats);
	}
	item.emplace_back(&reset);
	item.emplace_back(&autosaveSlot);
	item.emplace_back(&revertAutosave);
	item.emplace_back(&autosaveNow);
	item.emplace_back(&stateSlot);
	if(used(addLauncherIcon))
		item.emplace_back(&addLauncherIcon);
	item.emplace_back(&screenshot);
	item.emplace_back(&resetSessionOptions);
	item.emplace_back(&close);
}

}
