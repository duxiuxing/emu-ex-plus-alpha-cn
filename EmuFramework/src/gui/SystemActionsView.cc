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
#include "InputOverridesView.hh"
#include "AutosaveSlotView.hh"
#include "ResetAlertView.hh"
#include <emuframework/EmuApp.hh>
#include <emuframework/Cheats.hh>
#include <emuframework/StateSlotView.hh>
import imagine;

namespace EmuEx
{

constexpr SystemLogger log{"SystemActionsView"};

static auto autoSaveName(EmuApp &app)
{
	return std::format(
		UI_TEXT("当前自动存档槽位：{}"),
		app.autosaveManager.slotFullName());
}

static std::string saveAutosaveName(EmuApp &app)
{
	auto &autosaveManager = app.autosaveManager;
	if(!autosaveManager.timerFrequency().count())
		return UI_TEXT("保存进度 (到自动存档槽位)");
	return std::format(
		UI_TEXT("自动保存进度 (倒计时 {:%M:%S})"),
		duration_cast<Seconds>(autosaveManager.saveTimer.nextFireDuration()));
}

SystemActionsView::SystemActionsView(ViewAttachParams attach, bool customMenu):
	TableView
	{
		EmuApp::ViewName::SYSTEM_ACTIONS,
		attach, item
	},
	cheats
	{
		UI_TEXT("作弊项"),
		attach,
		[this](const Input::Event &e)
		{
			if(system().hasContent())
			{
				pushAndShow(makeView<CheatsView>(), e);
			}
		}
	},
	reset
	{
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
			pushAndShowModal(makeView<YesNoAlertView>(
				UI_TEXT("是否要保存进度？"),
				YesNoAlertView::Delegates
				{
					.onYes = [this]
					{
						if(app().autosaveManager.save(AutosaveActionSource::Manual))
							app().showEmulation();
					}
				}), e);
		}
	},
	revertAutosave
	{
		UI_TEXT("加载进度 (从自动存档槽位)"),
		attach,
		[this](TextMenuItem &item, const Input::Event &e)
		{
			if(!item.active())
				return;
			auto saveTime = app().autosaveManager.stateTimeAsString();
			if(saveTime.empty())
			{
				app().postMessage(
					UI_TEXT("未保存进度")
				);
				return;
			}
			pushAndShowModal(makeView<YesNoAlertView>(std::format(
				UI_TEXT("是否要从 {} 加载进度？"),
				saveTime),
				YesNoAlertView::Delegates
				{
					.onYes = [this]
					{
						if(app().autosaveManager.load(AutosaveActionSource::Manual))
							app().showEmulation();
					}
				}), e);
		}
	},
	stateSlot
	{
		UI_TEXT("手动存档槽位"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShow(makeView<StateSlotView>(), e);
		}
	},
	inputOverrides
	{
		UI_TEXT("输入设置 (这个游戏)"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShow(makeView<InputOverridesView>(app().inputManager), e);
		}
	},
	addLauncherIcon
	{
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
			pushAndShowNewCollectValueInputView<const char*>(attachParams(), e,
				UI_TEXT("快捷方式名称"),
				system().contentDisplayName(),
				[this](CollectTextInputView &, auto str)
				{
					appContext().addLauncherIcon(str, system().contentLocation());
					app().postMessage(2, false, std::format(
						UI_TEXT("已添加快捷方式：\n{}"),
						str));
					return true;
				});
		}
	},
	screenshot
	{
		UI_TEXT("截图"),
		attach,
		[this](const Input::Event &e)
		{
			if(!system().hasContent())
				return;
			auto pathName = appContext().fileUriDisplayName(app().screenshotDirectory());
			if(pathName.empty())
			{
				app().postMessage(
					UI_TEXT("无效的保存路径")
				);
				return;
			}
			pushAndShowModal(makeView<YesNoAlertView>(std::format(
				UI_TEXT("是否把截图保存到文件夹 {}？"),
				pathName),
				YesNoAlertView::Delegates
				{
					.onYes = [this]
					{
						app().video.takeGameScreenshot();
						system().runFrame({}, &app().video, nullptr);
					}
				}), e);
		}
	},
	resetSessionOptions
	{
		UI_TEXT("重置选项"),
		attach,
		[this](const Input::Event &e)
		{
			if(!app().hasSavedSessionOptions())
				return;
			pushAndShowModal(makeView<YesNoAlertView>(
				UI_TEXT("是否要恢复各个选项的默认值？某些选项需要重启模拟器才能生效。"),
				YesNoAlertView::Delegates
				{
					.onYes = [this]
					{
						resetSessionOptions.setActive(false);
						app().deleteSessionOptions();
					}
				}), e);
		}
	},
	close
	{
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
	assume(system().hasContent());
	autosaveSlot.compile(autoSaveName(app()));
	autosaveNow.compile(saveAutosaveName(app()));
	autosaveNow.setActive(app().autosaveManager.slotName() != noAutosaveName);
	revertAutosave.setActive(app().autosaveManager.slotName() != noAutosaveName);
	resetSessionOptions.setActive(app().hasSavedSessionOptions());
}

void SystemActionsView::loadStandardItems()
{
	if(AppMeta::hasCheats)
	{
		item.emplace_back(&cheats);
	}
	item.emplace_back(&reset);
	item.emplace_back(&autosaveSlot);
	item.emplace_back(&revertAutosave);
	item.emplace_back(&autosaveNow);
	item.emplace_back(&stateSlot);
	item.emplace_back(&inputOverrides);
	if(used(addLauncherIcon))
		item.emplace_back(&addLauncherIcon);
	item.emplace_back(&screenshot);
	item.emplace_back(&resetSessionOptions);
	item.emplace_back(&close);
}

}
