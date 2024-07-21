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

#include <emuframework/TouchConfigView.hh>
#include <emuframework/EmuApp.hh>
#include <emuframework/AppKeyCode.hh>
#include <emuframework/viewUtils.hh>
#include <imagine/gui/AlertView.hh>
#include <imagine/gui/TextTableView.hh>
#include <imagine/gfx/RendererCommands.hh>
#include <imagine/util/variant.hh>
#include "PlaceVideoView.hh"
#include "PlaceVControlsView.hh"
#include <utility>
#include <vector>
#include <array>
#include <span>
#include <ranges>
#include <format>

namespace EmuEx
{

constexpr const char *ctrlStateStr[]
{
	UI_TEXT("Off"),
	UI_TEXT("On"),
	UI_TEXT("Hidden")
};

constexpr const char *touchCtrlExtraBtnSizeMenuName[4]
{
	UI_TEXT("None"),
	UI_TEXT("10%"),
	UI_TEXT("20%"),
	UI_TEXT("30%")
};

constexpr int touchCtrlExtraBtnSizeMenuVal[4]
{
	0, 10, 20, 30
};

static void addCategories(EmuApp&, VControllerElement &elem, auto &&addCategory)
{
	if(elem.uiButtonGroup())
	{
		addCategory(appKeyCategory);
	}
	else
	{
		for(auto &cat : EmuApp::keyCategories() | std::views::filter([](auto &c){return !c.multiplayerIndex;}))
		{
			addCategory(cat);
		}
	}
}

class DPadElementConfigView : public TableView, public EmuAppHelper
{
public:
	DPadElementConfigView(ViewAttachParams attach, TouchConfigView &confView_, VController &vCtrl_, VControllerElement &elem_):
		TableView
		{
			UI_TEXT("Edit D-Pad"),
			attach, item
		},
		vCtrl{vCtrl_},
		elem{elem_},
		confView{confView_},
		deadzoneItems
		{
			{
				UI_TEXT("1mm"),
				attach, {.id = 100}
			},
			{
				UI_TEXT("1.35mm"),
				attach, {.id = 135}
			},
			{
				UI_TEXT("1.6mm"),
				attach, {.id = 160}
			},
			{
				UI_TEXT("Custom Value"),
				attach,
				[this](const Input::Event &e)
				{
					pushAndShowNewCollectValueRangeInputView<float, 1, 3>(attachParams(), e,
						UI_TEXT("Input 1.0 to 3.0"),
						"",
						[this](CollectTextInputView &, auto val)
						{
							int scaledIntVal = val * 100.0;
							elem.dPad()->setDeadzone(renderer(), scaledIntVal, window());
							deadzone.setSelected(MenuId{scaledIntVal}, *this);
							dismissPrevious();
							return true;
						});
					return false;
				}, {.id = defaultMenuId}
			},
		},
		deadzone
		{
			UI_TEXT("Deadzone"),
			attach,
			MenuId{elem.dPad()->deadzone()},
			deadzoneItems,
			{
				.onSetDisplayString = [this](auto, Gfx::Text& t)
				{
					t.resetString(std::format(
						UI_TEXT("{:g}mm"),
						elem.dPad()->deadzone() / 100.));
					return true;
				},
				.defaultItemOnSelect = [this](TextMenuItem &item) { elem.dPad()->setDeadzone(renderer(), item.id, window()); }
			},
		},
		diagonalSensitivityItems
		{
			{
				UI_TEXT("None"),
				attach, {.id = 1000}
			},
			{
				UI_TEXT("33% (Low)"),
				attach, {.id = 667}
			},
			{
				UI_TEXT("43% (Medium-Low)"),
				attach, {.id = 570}
			},
			{
				UI_TEXT("50% (Medium)"),
				attach, {.id = 500}
			},
			{
				UI_TEXT("60% (High)"),
				attach, {.id = 400}
			},
			{
				UI_TEXT("Custom Value"),
				attach,
				[this](const Input::Event &e)
				{
					pushAndShowNewCollectValueRangeInputView<float, 0, 99>(attachParams(), e,
						UI_TEXT("Input 0 to 99.0"),
						"",
						[this](CollectTextInputView &, auto val)
						{
							val = 100. - val;
							int scaledIntVal = val * 10.0;
							val /= 100.;
							elem.dPad()->setDiagonalSensitivity(renderer(), val);
							diagonalSensitivity.setSelected(MenuId{scaledIntVal}, *this);
							dismissPrevious();
							return true;
						});
					return false;
				}, {.id = defaultMenuId}
			},
		},
		diagonalSensitivity
		{
			UI_TEXT("Diagonal Sensitivity"),
			attach,
			MenuId{elem.dPad()->diagonalSensitivity() * 1000.f},
			diagonalSensitivityItems,
			{
				.onSetDisplayString = [this](auto, Gfx::Text& t)
				{
					t.resetString(std::format(
						UI_TEXT("{:g}%"),
						100.f - elem.dPad()->diagonalSensitivity() * 100.f));
					return true;
				},
				.defaultItemOnSelect = [this](TextMenuItem &item) { elem.dPad()->setDiagonalSensitivity(renderer(), float(item.id) / 1000.f); }
			},
		},
		stateItems
		{
			{ctrlStateStr[0], attach, {.id = VControllerState::OFF}},
			{ctrlStateStr[1], attach, {.id = VControllerState::SHOWN}},
			{ctrlStateStr[2], attach, {.id = VControllerState::HIDDEN}},
		},
		state
		{
			UI_TEXT("State"),
			attach,
			MenuId{elem.state},
			stateItems,
			{
				.defaultItemOnSelect = [this](TextMenuItem &item)
				{
					elem.state = VControllerState(item.id.val);
					vCtrl.place();
				}
			},
		},
		showBoundingArea
		{
			UI_TEXT("Show Bounding Area"),
			attach,
			elem.dPad()->showBounds(),
			[this](BoolMenuItem &item)
			{
				elem.dPad()->setShowBounds(renderer(), item.flipBoolValue(*this));
				vCtrl.place();
				postDraw();
			}
		},
		remove
		{
			UI_TEXT("Remove This D-Pad"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShowModal(makeView<YesNoAlertView>(
					UI_TEXT("Really remove this d-pad?"),
					YesNoAlertView::Delegates
					{
						.onYes = [this]
						{
							vCtrl.remove(elem);
							vCtrl.place();
							confView.reloadItems();
							dismiss();
						}
					}), e);
			}
		},
		actionsHeading
		{
			UI_TEXT("D-Pad Actions"),
			attach
		},
		actions
		{
			{
				UI_TEXT("Up"),
				app().inputManager.toString(elem.dPad()->config.keys[0]), attach,
				[this](const Input::Event &e) { assignAction(0, e); }
			},
			{
				UI_TEXT("Right"),
				app().inputManager.toString(elem.dPad()->config.keys[1]), attach,
				[this](const Input::Event &e) { assignAction(1, e); }
			},
			{
				UI_TEXT("Down"),
				app().inputManager.toString(elem.dPad()->config.keys[2]), attach,
				[this](const Input::Event &e) { assignAction(2, e); }
			},
			{
				UI_TEXT("Left"),
				app().inputManager.toString(elem.dPad()->config.keys[3]), attach,
				[this](const Input::Event &e) { assignAction(3, e); }
			}
		} {}

	void draw(Gfx::RendererCommands &__restrict__ cmds, ViewDrawParams) const final
	{
		vCtrl.draw(cmds, elem, true);
		TableView::draw(cmds);
	}

	void onShow() final
	{
		vCtrl.applyButtonAlpha(.75);
	}

private:
	VController &vCtrl;
	VControllerElement &elem;
	TouchConfigView &confView;
	TextMenuItem deadzoneItems[4];
	MultiChoiceMenuItem deadzone;
	TextMenuItem diagonalSensitivityItems[6];
	MultiChoiceMenuItem diagonalSensitivity;
	TextMenuItem stateItems[3];
	MultiChoiceMenuItem state;
	BoolMenuItem showBoundingArea;
	TextMenuItem remove;
	TextHeadingMenuItem actionsHeading;
	DualTextMenuItem actions[4];
	std::array<MenuItem*, 10> item{&state, &deadzone, &diagonalSensitivity, &showBoundingArea, &remove,
		&actionsHeading, &actions[0], &actions[1], &actions[2], &actions[3]};

	void assignAction(int idx, const Input::Event &e)
	{
		auto multiChoiceView = makeViewWithName<TextTableView>(
			UI_TEXT("Assign Action"),
			16);
		auto &app = this->app();
		addCategories(app, elem, [&](const KeyCategory &cat)
		{
			for(auto &k : cat.keys)
			{
				multiChoiceView->appendItem(app.inputManager.toString(k),
					[this, k](TextMenuItem &item, View &parentView, const Input::Event &)
					{
						elem.dPad()->config.keys[item.id] = k;
						actions[item.id].set2ndName(this->app().inputManager.toString(k));
						parentView.dismiss();
					}).id = idx;
			}
		});
		pushAndShow(std::move(multiChoiceView), e);
	}
};

class ButtonElementConfigView : public TableView, public EmuAppHelper
{
public:
	using OnChange = DelegateFunc<void()>;

	ButtonElementConfigView(ViewAttachParams attach, OnChange onChange_, VController &vCtrl_, VControllerElement &elem_, VControllerButton &btn_):
		TableView
		{
			UI_TEXT("Edit Button"),
			attach, item
		},
		vCtrl{vCtrl_},
		elem{elem_},
		btn{btn_},
		onChange{onChange_},
		key
		{
			UI_TEXT("Action"),
			app().inputManager.toString(btn_.key), attach,
			[this](const Input::Event &e)
			{
				auto multiChoiceView = makeViewWithName<TextTableView>(
					UI_TEXT("Assign Action"),
					16);
				auto &app = this->app();
				addCategories(app, elem, [&](const KeyCategory &cat)
				{
					for(auto &k : cat.keys)
					{
						multiChoiceView->appendItem(app.inputManager.toString(k),
							[this, k](View &parentView)
							{
								btn.key = k;
								btn.enabled = vCtrl.keyIsEnabled(k);
								key.set2ndName(this->app().inputManager.toString(k));
								turbo.setBoolValue(k.flags.turbo, *this);
								toggle.setBoolValue(k.flags.toggle, *this);
								vCtrl.update(elem);
								onChange.callSafe();
								vCtrl.place();
								parentView.dismiss();
							});
					}
				});
				pushAndShow(std::move(multiChoiceView), e);
			}
		},
		turbo
		{
			UI_TEXT("Turbo"),
			attach,
			bool(btn_.key.flags.turbo),
			[this](BoolMenuItem &item)
			{
				btn.key.flags.turbo = item.flipBoolValue(*this);
				key.set2ndName(app().inputManager.toString(btn.key));
				key.place2nd();
				onChange.callSafe();
			}
		},
		toggle
		{
			UI_TEXT("Toggle"),
			attach,
			bool(btn_.key.flags.toggle),
			[this](BoolMenuItem &item)
			{
				btn.key.flags.toggle = item.flipBoolValue(*this);
				key.set2ndName(app().inputManager.toString(btn.key));
				key.place2nd();
				onChange.callSafe();
			}
		},
		remove
		{
			UI_TEXT("Remove This Button"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShowModal(makeView<YesNoAlertView>(
					UI_TEXT("Really remove this button?"),
					YesNoAlertView::Delegates
					{
						.onYes = [this]
						{
							elem.remove(btn);
							onChange.callSafe();
							vCtrl.place();
							dismiss();
						}
					}), e);
			}
		}
	{
		reloadItems();
	}

private:
	VController &vCtrl;
	VControllerElement &elem;
	VControllerButton &btn;
	OnChange onChange;
	DualTextMenuItem key;
	BoolMenuItem turbo;
	BoolMenuItem toggle;
	TextMenuItem remove;
	std::vector<MenuItem*> item;

	void reloadItems()
	{
		item.clear();
		item.emplace_back(&key);
		if(!btn.key.flags.appCode)
		{
			item.emplace_back(&turbo);
			item.emplace_back(&toggle);
		}
		item.emplace_back(&remove);
	}
};

class ButtonGroupElementConfigView : public TableView, public EmuAppHelper
{
public:
	ButtonGroupElementConfigView(ViewAttachParams attach, TouchConfigView &confView_, VController &vCtrl_, VControllerElement &elem_):
		TableView
		{
			UI_TEXT("Edit Buttons"),
			attach, item
		},
		vCtrl{vCtrl_},
		elem{elem_},
		confView{confView_},
		stateItems
		{
			{ctrlStateStr[0], attach, {.id = VControllerState::OFF}},
			{ctrlStateStr[1], attach, {.id = VControllerState::SHOWN}},
			{ctrlStateStr[2], attach, {.id = VControllerState::HIDDEN}},
		},
		state
		{
			UI_TEXT("State"),
			attach,
			MenuId{elem.state},
			stateItems,
			{
				.defaultItemOnSelect = [this](TextMenuItem &item)
				{
					elem.state = VControllerState(item.id.val);
					vCtrl.place();
				}
			},
		},
		rowSizeItems
		{
			{
				UI_TEXT("1"),
				attach, {.id = 1}
			},
			{
				UI_TEXT("2"),
				attach, {.id = 2}
			},
			{
				UI_TEXT("3"),
				attach, {.id = 3}
			},
			{
				UI_TEXT("4"),
				attach, {.id = 4}
			},
			{
				UI_TEXT("5"),
				attach, {.id = 5}
			},
		},
		rowSize
		{
			UI_TEXT("Buttons Per Row"),
			attach,
			MenuId{elem.rowSize()},
			rowSizeItems,
			{
				.defaultItemOnSelect = [this](TextMenuItem &item)
				{
					elem.setRowSize(item.id);
					vCtrl.place();
				}
			},
		},
		spaceItems
		{
			{
				UI_TEXT("1mm"),
				attach, {.id = 1}
			},
			{
				UI_TEXT("2mm"),
				attach, {.id = 2}
			},
			{
				UI_TEXT("3mm"),
				attach, {.id = 3}
			},
			{
				UI_TEXT("4mm"),
				attach, {.id = 4}
			},
			{
				UI_TEXT("Custom Value"),
				attach,
				[this](const Input::Event &e)
				{
					pushAndShowNewCollectValueRangeInputView<int, 0, 8>(attachParams(), e,
						UI_TEXT("Input 0 to 8"),
						"",
						[this](CollectTextInputView &, auto val)
						{
							elem.buttonGroup()->setSpacing(val, window());
							vCtrl.place();
							space.setSelected(MenuId{val}, *this);
							dismissPrevious();
							return true;
						});
					return false;
				}, {.id = defaultMenuId}
			},
		},
		space
		{
			UI_TEXT("Spacing"),
			attach,
			MenuId{elem.buttonGroup() ? elem.buttonGroup()->spacing() : 0},
			spaceItems,
			{
				.onSetDisplayString = [this](auto, Gfx::Text& t)
				{
					t.resetString(std::format("{}mm", elem.buttonGroup()->spacing()));
					return true;
				},
				.defaultItemOnSelect = [this](TextMenuItem &item)
				{
					elem.buttonGroup()->setSpacing(item.id, window());
					vCtrl.place();
				}
			},
		},
		staggerItems
		{
			{
				UI_TEXT("-0.75x V"),
				attach, {.id = 0}
			},
			{
				UI_TEXT("-0.5x V"),
				attach, {.id = 1}
			},
			{
				UI_TEXT("0"),
				attach, {.id = 2}
			},
			{
				UI_TEXT("0.5x V"),
				attach, {.id = 3}
			},
			{
				UI_TEXT("0.75x V"),
				attach, {.id = 4}
			},
			{
				UI_TEXT("1x H&V"),
				attach, {.id = 5}
			},
		},
		stagger
		{
			UI_TEXT("Stagger"),
			attach,
			MenuId{elem.buttonGroup() ? elem.buttonGroup()->stagger() : 0},
			staggerItems,
			{
				.defaultItemOnSelect = [this](TextMenuItem &item)
				{
					elem.buttonGroup()->setStaggerType(item.id);
					vCtrl.place();
				}
			},
		},
		extraXSizeItems
		{
			{touchCtrlExtraBtnSizeMenuName[0], attach, {.id = touchCtrlExtraBtnSizeMenuVal[0]}},
			{touchCtrlExtraBtnSizeMenuName[1], attach, {.id = touchCtrlExtraBtnSizeMenuVal[1]}},
			{touchCtrlExtraBtnSizeMenuName[2], attach, {.id = touchCtrlExtraBtnSizeMenuVal[2]}},
			{touchCtrlExtraBtnSizeMenuName[3], attach, {.id = touchCtrlExtraBtnSizeMenuVal[3]}},
			{
				UI_TEXT("Custom Value"),
				attach,
				[this](const Input::Event &e)
				{
					pushAndShowNewCollectValueRangeInputView<int, 0, 30>(attachParams(), e,
						UI_TEXT("Input 0 to 30"),
						"",
						[this](CollectTextInputView &, auto val)
						{
							elem.buttonGroup()->layout.xPadding = val;
							vCtrl.place();
							extraXSize.setSelected(MenuId{val}, *this);
							dismissPrevious();
							return true;
						});
					return false;
				}, {.id = defaultMenuId}
			}
		},
		extraXSize
		{
			UI_TEXT("Extended H Bounds"),
			attach,
			MenuId{elem.buttonGroup() ? elem.buttonGroup()->layout.xPadding : 0},
			extraXSizeItems,
			{
				.onSetDisplayString = [this](auto idx, Gfx::Text &t)
				{
					if(!idx)
						return false;
					t.resetString(std::format(
						UI_TEXT("{}%"),
						elem.buttonGroup()->layout.xPadding));
					return true;
				},
				.defaultItemOnSelect = [this](TextMenuItem &item)
				{
					elem.buttonGroup()->layout.xPadding = item.id;
					vCtrl.place();
				}
			},
		},
		extraYSizeItems
		{
			{touchCtrlExtraBtnSizeMenuName[0], attach, {.id = touchCtrlExtraBtnSizeMenuVal[0]}},
			{touchCtrlExtraBtnSizeMenuName[1], attach, {.id = touchCtrlExtraBtnSizeMenuVal[1]}},
			{touchCtrlExtraBtnSizeMenuName[2], attach, {.id = touchCtrlExtraBtnSizeMenuVal[2]}},
			{touchCtrlExtraBtnSizeMenuName[3], attach, {.id = touchCtrlExtraBtnSizeMenuVal[3]}},
			{
				UI_TEXT("Custom Value"),
				attach,
				[this](const Input::Event &e)
				{
					pushAndShowNewCollectValueRangeInputView<int, 0, 30>(attachParams(), e,
						UI_TEXT("Input 0 to 30"),
						"",
						[this](CollectTextInputView &, auto val)
						{
							elem.buttonGroup()->layout.yPadding = val;
							vCtrl.place();
							extraYSize.setSelected(MenuId{val}, *this);
							dismissPrevious();
							return true;
						});
					return false;
				}, {.id = defaultMenuId}
			}
		},
		extraYSize
		{
			UI_TEXT("Extended V Bounds"),
			attach,
			MenuId{elem.buttonGroup() ? elem.buttonGroup()->layout.yPadding : 0},
			extraYSizeItems,
			{
				.onSetDisplayString = [this](auto idx, Gfx::Text &t)
				{
					if(!idx)
						return false;
					t.resetString(std::format(
						UI_TEXT("{}%"),
						elem.buttonGroup()->layout.yPadding));
					return true;
				},
				.defaultItemOnSelect = [this](TextMenuItem &item)
				{
					elem.buttonGroup()->layout.yPadding = item.id;
					vCtrl.place();
				}
			},
		},
		showBoundingArea
		{
			UI_TEXT("Show Bounding Area"),
			attach,
			elem.buttonGroup() ? elem.buttonGroup()->showsBounds() : false,
			[this](BoolMenuItem &item)
			{
				elem.buttonGroup()->setShowBounds(item.flipBoolValue(*this));
				vCtrl.place();
				postDraw();
			}
		},
		add
		{
			UI_TEXT("Add Button To This Group"),
			attach,
			[this](const Input::Event &e)
			{
				auto multiChoiceView = makeViewWithName<TextTableView>(
					UI_TEXT("Add Button"),
					16);
				auto &app = this->app();
				addCategories(app, elem, [&](const KeyCategory &cat)
				{
					for(auto &k : cat.keys)
					{
						multiChoiceView->appendItem(app.inputManager.toString(k),
							[this, k](View &parentView, const Input::Event&)
							{
								elem.add(k);
								vCtrl.update(elem);
								vCtrl.place();
								confView.reloadItems();
								reloadItems();
								parentView.dismiss();
							});
					}
				});
				pushAndShow(std::move(multiChoiceView), e);
			}
		},
		remove
		{
			UI_TEXT("Remove This Button Group"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShowModal(makeView<YesNoAlertView>(
					UI_TEXT("Really remove this button group?"),
					YesNoAlertView::Delegates
					{
						.onYes = [this]
						{
							vCtrl.remove(elem);
							vCtrl.place();
							confView.reloadItems();
							dismiss();
						}
					}), e);
			}
		},
		buttonsHeading
		{
			UI_TEXT("Buttons In Group"),
			attach
		}
	{
		reloadItems();
	}

	void draw(Gfx::RendererCommands &__restrict__ cmds, ViewDrawParams) const final
	{
		vCtrl.draw(cmds, elem, true);
		TableView::draw(cmds);
	}

	void onShow() final
	{
		vCtrl.applyButtonAlpha(.75);
	}

private:
	VController &vCtrl;
	VControllerElement &elem;
	TouchConfigView &confView;
	TextMenuItem stateItems[3];
	MultiChoiceMenuItem state;
	TextMenuItem rowSizeItems[5];
	MultiChoiceMenuItem rowSize;
	TextMenuItem spaceItems[5];
	MultiChoiceMenuItem space;
	TextMenuItem staggerItems[6];
	MultiChoiceMenuItem stagger;
	TextMenuItem extraXSizeItems[5];
	MultiChoiceMenuItem extraXSize;
	TextMenuItem extraYSizeItems[5];
	MultiChoiceMenuItem extraYSize;
	BoolMenuItem showBoundingArea;
	TextMenuItem add;
	TextMenuItem remove;
	TextHeadingMenuItem buttonsHeading;
	std::vector<TextMenuItem> buttonItems;
	std::vector<MenuItem*> item;

	void reloadItems()
	{
		buttonItems.clear();
		item.clear();
		item.emplace_back(&state);
		if(elem.buttonGroup())
		{
			item.emplace_back(&space);
			item.emplace_back(&stagger);
			item.emplace_back(&extraXSize);
			item.emplace_back(&extraYSize);
			item.emplace_back(&showBoundingArea);
		}
		item.emplace_back(&rowSize);
		item.emplace_back(&add);
		item.emplace_back(&remove);
		item.emplace_back(&buttonsHeading);
		auto buttons = elem.buttons();
		buttonItems.reserve(buttons.size());
		for(auto &btn : buttons)
		{
			auto &i = buttonItems.emplace_back(
				btn.name(app().inputManager), attachParams(),
				[this, &btn](const Input::Event &e)
				{
					pushAndShow(makeView<ButtonElementConfigView>([this]()
					{
						confView.reloadItems();
						reloadItems();
					}, vCtrl, elem, btn), e);
				});
			item.emplace_back(&i);
		}
	}
};

class AddNewButtonView : public TableView, public EmuAppHelper
{
public:
	AddNewButtonView(ViewAttachParams attach, TouchConfigView &confView_, VController &vCtrl_):
		TableView
		{
			UI_TEXT("Add New Button Group"),
			attach, buttons
		},
		vCtrl{vCtrl_},
		confView{confView_}
	{
		for(const auto &c : system().inputDeviceDesc(0).components)
		{
			buttons.emplace_back(
				c.name, attach,
				[this, &c]{ add(c); });
		}
		buttons.emplace_back(
			rightUIComponents.name, attach,
			[this]{ add(rightUIComponents); });
		buttons.emplace_back(
			leftUIComponents.name, attach,
			[this]{ add(leftUIComponents); });
		buttons.emplace_back(
			rewindUIComponents.name, attach,
			[this]{ add(rewindUIComponents); });
	}

private:
	VController &vCtrl;
	TouchConfigView &confView;
	std::vector<TextMenuItem> buttons;

	void add(const InputComponentDesc &desc)
	{
		vCtrl.add(desc);
		vCtrl.place();
		confView.reloadItems();
		dismiss();
	}
};

void TouchConfigView::draw(Gfx::RendererCommands &__restrict__ cmds, ViewDrawParams) const
{
	vController.draw(cmds, true);
	TableView::draw(cmds);
}

void TouchConfigView::place()
{
	refreshTouchConfigMenu();
	TableView::place();
}

void TouchConfigView::refreshTouchConfigMenu()
{
	alpha.setSelected(MenuId{vController.buttonAlpha()}, *this);
	touchCtrl.setSelected((int)vController.gamepadControlsVisibility(), *this);
	if(EmuSystem::maxPlayers > 1)
		player.setSelected((int)vController.inputPlayer(), *this);
	size.setSelected(MenuId{vController.buttonSize()}, *this);
	if(app().vibrationManager.hasVibrator())
	{
		vibrate.setBoolValue(vController.vibrateOnTouchInput(), *this);
	}
	showOnTouch.setBoolValue(vController.showOnTouchInput(), *this);
}

TouchConfigView::TouchConfigView(ViewAttachParams attach, VController &vCtrl):
	TableView
	{
		UI_TEXT("On-screen Input Setup"),
		attach, item
	},
	vController{vCtrl},
	touchCtrlItem
	{
		{
			UI_TEXT("Off"),
			attach, {.id = VControllerVisibility::OFF}
		},
		{
			UI_TEXT("On"),
			attach, {.id = VControllerVisibility::ON}
		},
		{
			UI_TEXT("Auto"),
			attach, {.id = VControllerVisibility::AUTO}
		}
	},
	touchCtrl
	{
		UI_TEXT("Use Virtual Gamepad"),
		attach,
		int(vCtrl.gamepadControlsVisibility()),
		touchCtrlItem,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item){ vController.setGamepadControlsVisibility(VControllerVisibility(item.id.val)); }
		},
	},
	playerItems
	{
		[&] -> DynArray<TextMenuItem>
		{
			if(EmuSystem::maxPlayers == 1)
				return {};
			DynArray<TextMenuItem> items{size_t(EmuSystem::maxPlayers)};
			for(auto i : iotaCount(EmuSystem::maxPlayers))
			{
				items[i] = {playerNumStrings[i], attach, {.id = i}};
			}
			return items;
		}()
	},
	player
	{
		UI_TEXT("Virtual Gamepad Player"),
		attach,
		int(vCtrl.inputPlayer()),
		playerItems,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item){ vController.setInputPlayer(item.id); }
		},
	},
	sizeItem
	{
		{
			UI_TEXT("6.5mm"),
			attach, {.id = 650}
		},
		{
			UI_TEXT("7mm"),
			attach, {.id = 700}
		},
		{
			UI_TEXT("7.5mm"),
			attach, {.id = 750}
		},
		{
			UI_TEXT("8mm"),
			attach, {.id = 800}
		},
		{
			UI_TEXT("8.5mm"),
			attach, {.id = 850}
		},
		{
			UI_TEXT("9mm"),
			attach, {.id = 900}
		},
		{
			UI_TEXT("10mm"),
			attach, {.id = 1000}
		},
		{
			UI_TEXT("12mm"),
			attach, {.id = 1200}
		},
		{
			UI_TEXT("14mm"),
			attach, {.id = 1400}
		},
		{
			UI_TEXT("15mm"),
			attach, {.id = 1500}
		},
		{
			UI_TEXT("Custom Value"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShowNewCollectValueRangeInputView<float, 3, 30>(attachParams(), e,
					UI_TEXT("Input 3.0 to 30.0"),
					"",
					[this](CollectTextInputView &, auto val)
					{
						int scaledIntVal = val * 100.0;
						vController.setButtonSize(scaledIntVal);
						size.setSelected(MenuId{scaledIntVal}, *this);
						dismissPrevious();
						return true;
					});
				return false;
			}, {.id = defaultMenuId}
		},
	},
	size
	{
		UI_TEXT("Button Size"),
		attach,
		MenuId{vController.buttonSize()},
		sizeItem,
		{
			.onSetDisplayString = [this](auto, Gfx::Text& t)
			{
				t.resetString(std::format(
					UI_TEXT("{:g}mm"),
					vController.buttonSize() / 100.));
				return true;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item){ vController.setButtonSize(item.id); }
		},
	},
	vibrate
	{
		UI_TEXT("Vibration"),
		attach,
		vController.vibrateOnTouchInput(),
		[this](BoolMenuItem &item)
		{
			vController.setVibrateOnTouchInput(app(), item.flipBoolValue(*this));
		}
	},
	showOnTouch
	{
		UI_TEXT("Show Gamepad If Screen Touched"),
		attach,
		vController.showOnTouchInput(),
		[this](BoolMenuItem &item)
		{
			vController.setShowOnTouchInput(item.flipBoolValue(*this));
		}
	},
	highlightPushedButtons
	{
		UI_TEXT("Highlight Pushed Buttons"),
		attach,
		vController.highlightPushedButtons,
		[this](BoolMenuItem &item)
		{
			vController.highlightPushedButtons = item.flipBoolValue(*this);
		}
	},
	alphaItem
	{
		{
			UI_TEXT("0%"),
			attach, {.id = 0}
		},
		{
			UI_TEXT("10%"),
			attach, {.id = int(255. * .1)}
		},
		{
			UI_TEXT("25%"),
			attach, {.id = int(255. * .25)}
		},
		{
			UI_TEXT("50%"),
			attach, {.id = int(255. * .5)}
		},
		{
			UI_TEXT("65%"),
			attach, {.id = int(255. * .65)}
		},
		{
			UI_TEXT("75%"),
			attach, {.id = int(255. * .75)}
		},
	},
	alpha
	{
		UI_TEXT("Blend Amount"),
		attach,
		MenuId{vController.buttonAlpha()},
		alphaItem,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item){ vController.setButtonAlpha(item.id); }
		},
	},
	btnPlace
	{
		UI_TEXT("Set Button Positions"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShowModal(makeView<PlaceVControlsView>(vController), e);
		}
	},
	placeVideo
	{
		UI_TEXT("Set Video Position"),
		attach,
		[this](const Input::Event &e)
		{
			if(!system().hasContent())
				return;
			pushAndShowModal(makeView<PlaceVideoView>(app().videoLayer, vController), e);
		}
	},
	addButton
	{
		UI_TEXT("Add New Button Group"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShow(makeView<AddNewButtonView>(*this, vController), e);
		}
	},
	allowButtonsPastContentBounds
	{
		UI_TEXT("Allow Buttons In Display Cutout Area"),
		attach,
		vController.allowButtonsPastContentBounds(),
		[this](BoolMenuItem &item)
		{
			vController.setAllowButtonsPastContentBounds(item.flipBoolValue(*this));
			vController.place();
		}
	},
	resetEmuPositions
	{
		UI_TEXT("Reset Emulated Device Positions"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShowModal(makeView<YesNoAlertView>(
				UI_TEXT("Reset buttons to default positions?"),
				YesNoAlertView::Delegates
				{
					.onYes = [this]
					{
						vController.resetEmulatedDevicePositions();
						vController.place();
					}
				}), e);
		}
	},
	resetEmuGroups
	{
		UI_TEXT("Reset Emulated Device Groups"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShowModal(makeView<YesNoAlertView>(
				UI_TEXT("Reset buttons groups to default?"),
				YesNoAlertView::Delegates
				{
					.onYes = [this]
					{
						vController.resetEmulatedDeviceGroups();
						vController.place();
						reloadItems();
					}
				}), e);
		}
	},
	resetUIPositions
	{
		UI_TEXT("Reset UI Positions"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShowModal(makeView<YesNoAlertView>(
				UI_TEXT("Reset buttons to default positions?"),
				YesNoAlertView::Delegates
				{
					.onYes = [this]
					{
						vController.resetUIPositions();
						vController.place();
					}
				}), e);
		}
	},
	resetUIGroups
	{
		UI_TEXT("Reset UI Groups"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShowModal(makeView<YesNoAlertView>(
				UI_TEXT("Reset buttons groups to default?"),
				YesNoAlertView::Delegates
				{
					.onYes = [this]
					{
						vController.resetUIGroups();
						vController.place();
						reloadItems();
					}
				}), e);
		}
	},
	devButtonsHeading
	{
		UI_TEXT("Emulated Device Button Groups"),
		attach
	},
	uiButtonsHeading
	{
		UI_TEXT("UI Button Groups"),
		attach
	},
	otherHeading
	{
		UI_TEXT("Other Options"),
		attach
	}
{
	reloadItems();
}

void TouchConfigView::reloadItems()
{
	elementItems.clear();
	item.clear();
	item.emplace_back(&touchCtrl);
	if(EmuSystem::maxPlayers > 1)
	{
		item.emplace_back(&player);
	}
	item.emplace_back(&size);
	item.emplace_back(&btnPlace);
	placeVideo.setActive(system().hasContent());
	item.emplace_back(&placeVideo);
	item.emplace_back(&devButtonsHeading);
	elementItems.reserve(vController.deviceElements().size() + vController.guiElements().size());
	for(auto &elem : vController.deviceElements())
	{
		auto &i = elementItems.emplace_back(
			elem.name(app().inputManager), attachParams(),
			[this, &elem](const Input::Event &e)
			{
				elem.visit(overloaded
				{
					[&](VControllerDPad &){ pushAndShow(makeView<DPadElementConfigView>(*this, vController, elem), e); },
					[&](VControllerButtonGroup &){ pushAndShow(makeView<ButtonGroupElementConfigView>(*this, vController, elem), e); },
					[](auto &){}
				});
			});
		item.emplace_back(&i);
	}
	item.emplace_back(&uiButtonsHeading);
	for(auto &elem : vController.guiElements())
	{
		auto &i = elementItems.emplace_back(
			elem.name(app().inputManager), attachParams(),
			[this, &elem](const Input::Event &e)
			{
				elem.visit(overloaded
				{
					[&](VControllerUIButtonGroup &){ pushAndShow(makeView<ButtonGroupElementConfigView>(*this, vController, elem), e); },
					[](auto &){}
				});
			});
		item.emplace_back(&i);
	}
	item.emplace_back(&otherHeading);
	item.emplace_back(&addButton);
	if(used(allowButtonsPastContentBounds) && appContext().hasDisplayCutout())
	{
		item.emplace_back(&allowButtonsPastContentBounds);
	}
	if(app().vibrationManager.hasVibrator())
	{
		item.emplace_back(&vibrate);
	}
	item.emplace_back(&showOnTouch);
	item.emplace_back(&highlightPushedButtons);
	item.emplace_back(&alpha);
	item.emplace_back(&resetEmuPositions);
	item.emplace_back(&resetEmuGroups);
	item.emplace_back(&resetUIPositions);
	item.emplace_back(&resetUIGroups);
}

void TouchConfigView::onShow()
{
	vController.applyButtonAlpha(.75);
}

}
