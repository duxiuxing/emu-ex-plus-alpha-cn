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

#include "AutosaveSlotView.hh"
#include <emuframework/EmuApp.hh>
#include <imagine/gui/AlertView.hh>
#include <emuframework/viewUtils.hh>
#include <format>

namespace EmuEx
{

using SlotTextMenuItem = AutosaveSlotView::SlotTextMenuItem;

class ManageAutosavesView : public TableView, public EmuAppHelper
{
public:
	ManageAutosavesView(ViewAttachParams, AutosaveSlotView &srcView,
		const std::vector<SlotTextMenuItem> &menuItems);
	void updateItem(std::string_view name, std::string_view newName);
	bool hasItems() const { return extraSlotItems.size(); }

private:
	AutosaveSlotView &srcView;
	std::vector<SlotTextMenuItem> extraSlotItems;
};

class EditAutosaveView : public TableView, public EmuAppHelper
{
public:
	EditAutosaveView(ViewAttachParams attach, ManageAutosavesView &srcView_, std::string_view slotName_):
		TableView{slotName_, attach, menuItems},
		srcView{srcView_},
		slotName{slotName_},
		rename
		{
			// UI_TEXT("Rename"),
			UI_TEXT("重命名"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShowNewCollectValueInputView<const char*>(attachParams(), e,
					// UI_TEXT("Input name"),
					UI_TEXT("请输入存档点名称"),
					slotName,
					[this](CollectTextInputView &, auto str)
					{
						if(appContext().fileUriExists(system().contentLocalSaveDirectory(str)))
						{
							// app().postErrorMessage(UI_TEXT("A save slot with that name already exists"));
							app().postErrorMessage(UI_TEXT("已经存在同名的存档点"));
							return false;
						}
						if(!app().autosaveManager.renameSlot(slotName, str))
						{
							// app().postErrorMessage(UI_TEXT("Error renaming save slot"));
							app().postErrorMessage(UI_TEXT("重命名存档点时出错"));
							return false;
						}
						srcView.updateItem(slotName, str);
						dismiss();
						return true;
					}
				);
			}
		},
		remove
		{
			// UI_TEXT("Delete"),
			UI_TEXT("删除"),
			attach,
			[this](const Input::Event &e)
			{
				if(slotName == app().autosaveManager.slotName())
				{
					// app().postErrorMessage(UI_TEXT("Can't delete the currently active save slot"));
					app().postErrorMessage(UI_TEXT("无法删除当前正在使用的存档点"));
					return;
				}
				pushAndShowModal(
					makeView<YesNoAlertView>(
						// UI_TEXT("Really delete this save slot?"),
						UI_TEXT("是否要删除这个存档点？"),
						YesNoAlertView::Delegates
						{
							.onYes = [this]
							{
								app().autosaveManager.deleteSlot(slotName);
								srcView.updateItem(slotName, "");
								if(!srcView.hasItems())
									srcView.dismiss();
								dismiss();
							}
						}
					), e
				);
			}
		} {}

private:
	ManageAutosavesView &srcView;
	std::string slotName;
	TextMenuItem rename;
	TextMenuItem remove;
	std::array<MenuItem*, 2> menuItems{&rename, &remove};
};

ManageAutosavesView::ManageAutosavesView(ViewAttachParams attach, AutosaveSlotView &srcView,
	const std::vector<SlotTextMenuItem> &items):
	TableView
	{
		// UI_TEXT("Manage Save Slots"),
		UI_TEXT("管理存档点"),
		attach,
		extraSlotItems
	},
	srcView{srcView}
{
	extraSlotItems.reserve(items.size());
	for(auto &i : items)
	{
		extraSlotItems.emplace_back(i.slotName, i.text().stringView(), attach,
			[this](TextMenuItem &item, const Input::Event &e)
			{
				pushAndShow(makeView<EditAutosaveView>(*this, static_cast<SlotTextMenuItem&>(item).slotName), e);
			}
		);
	}
}

static std::string slotDescription(EmuApp &app, std::string_view saveName)
{
	auto desc = app.appContext().fileUriFormatLastWriteTimeLocal(app.autosaveManager.statePath(saveName));
	if(desc.empty())
		// desc = UI_TEXT("No saved state");
		desc = UI_TEXT("无效的存档进度");
	return desc;
}

void ManageAutosavesView::updateItem(std::string_view name, std::string_view newName)
{
	auto it = std::ranges::find_if(extraSlotItems, [&](auto &i) { return i.slotName == name; });
	if(it == extraSlotItems.end()) [[unlikely]]
		return;
	if(newName.empty())
	{
		extraSlotItems.erase(it);
	}
	else
	{
		it->setName(
			std::format(
				// UI_TEXT("{}: {}"),
				UI_TEXT("{}：{}"),
				newName, slotDescription(app(), newName)
			)
		);
		it->slotName = newName;
	}
	place();
	srcView.updateItem(name, newName);
}

AutosaveSlotView::AutosaveSlotView(ViewAttachParams attach):
	TableView
	{
		// UI_TEXT("Autosave Slot"),
		UI_TEXT("自动存档点"),
		attach,
		menuItems
	},
	newSlot
	{
		// UI_TEXT("Create New Save Slot"),
		UI_TEXT("新建存档点"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShowNewCollectValueInputView<const char*>(attachParams(), e,
				// UI_TEXT("Save Slot Name"),
				UI_TEXT("存档点的名称"),
				"",
				[this](CollectTextInputView &, auto str_)
				{
					std::string_view name{str_};
					if(appContext().fileUriExists(app().system().contentLocalSaveDirectory(name)))
					{
						// app().postErrorMessage(UI_TEXT("A save slot with that name already exists"));
						app().postErrorMessage(UI_TEXT("已经存在同名的存档点"));
						return false;
					}
					if(!app().autosaveManager.setSlot(name))
					{
						// app().postErrorMessage(UI_TEXT("Error creating save slot"));
						app().postErrorMessage(UI_TEXT("新建存档点时出错"));
						return false;
					}
					app().showEmulation();
					refreshItems();
					return true;
				}
			);
		}
	},
	manageSlots
	{
		// UI_TEXT("Manage Save Slots"),
		UI_TEXT("管理存档点"),
		attach,
		[this](const Input::Event &e)
		{
			if(extraSlotItems.empty())
			{
				// app().postMessage(UI_TEXT("No extra save slots exist"));
				app().postMessage(UI_TEXT("没有额外的存档点"));
				return;
			}
			pushAndShow(makeView<ManageAutosavesView>(*this, extraSlotItems), e);
		}
	},
	actions
	{
		// UI_TEXT("Actions"),
		UI_TEXT("操作："),
		attach
	}
{
	refreshSlots();
	loadItems();
}

void AutosaveSlotView::refreshSlots()
{
	mainSlot =
	{
		std::format(
			// UI_TEXT("Main: {}"),
			UI_TEXT("默认存档点：{}"),
			slotDescription(app(), "")
		),
		attachParams(),
		[this]()
		{
			if(app().autosaveManager.setSlot(""))
			{
				app().showEmulation();
				refreshItems();
			}
		}
	};
	if(app().autosaveManager.slotName().empty())
		mainSlot.setHighlighted(true);
	extraSlotItems.clear();
	auto ctx = appContext();
	auto &sys = system();
	ctx.forEachInDirectoryUri(sys.contentLocalSaveDirectory(),
		[&](const FS::directory_entry &e)
		{
			if(e.type() != FS::file_type::directory)
				return true;
			auto &item = extraSlotItems.emplace_back(e.name(),
				std::format(
					// UI_TEXT("{}: {}"),
					UI_TEXT("{}：{}"),
					e.name(), slotDescription(app(), e.name())
				),
				attachParams(),
				[this](TextMenuItem &item)
				{
					if(app().autosaveManager.setSlot(static_cast<SlotTextMenuItem&>(item).slotName))
					{
						app().showEmulation();
						refreshItems();
					}
				}
			);
			if(app().autosaveManager.slotName() == e.name())
				item.setHighlighted(true);
			return true;
		},
		{.test = true}
	);
	noSaveSlot =
	{
		// UI_TEXT("No Save"),
		UI_TEXT("不保存"),
		attachParams(),
		[this]()
		{
			if(app().autosaveManager.setSlot(noAutosaveName))
			{
				app().showEmulation();
				refreshItems();
			}
		}
	};
	if(app().autosaveManager.slotName() == noAutosaveName)
		noSaveSlot.setHighlighted(true);
}

void AutosaveSlotView::refreshItems()
{
	refreshSlots();
	loadItems();
	place();
}

void AutosaveSlotView::loadItems()
{
	menuItems.clear();
	if(!system().hasContent())
		return;
	menuItems.emplace_back(&mainSlot);
	for(auto &i : extraSlotItems)
		menuItems.emplace_back(&i);
	menuItems.emplace_back(&noSaveSlot);
	menuItems.emplace_back(&actions);
	menuItems.emplace_back(&newSlot);
	menuItems.emplace_back(&manageSlots);
	manageSlots.setActive(extraSlotItems.size());
}

void AutosaveSlotView::updateItem(std::string_view name, std::string_view newName)
{
	auto it = std::ranges::find_if(extraSlotItems, [&](auto &i) { return i.slotName == name; });
	if(it == extraSlotItems.end()) [[unlikely]]
		return;
	if(newName.empty())
	{
		extraSlotItems.erase(it);
		loadItems();
	}
	else
	{
		it->setName(
			std::format(
				// UI_TEXT("{}: {}"),
				UI_TEXT("{}：{}"),
				newName, slotDescription(app(), newName)
			)
		);
		it->slotName = newName;
	}
	place();
}

}
