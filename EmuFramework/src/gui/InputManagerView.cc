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

#include <emuframework/ButtonConfigView.hh>
#include <emuframework/EmuApp.hh>
#include <emuframework/EmuViewController.hh>
#include <emuframework/AppKeyCode.hh>
#include <emuframework/EmuOptions.hh>
#include <emuframework/viewUtils.hh>
#include "InputManagerView.hh"
#include "ProfileSelectView.hh"
#include "../InputDeviceData.hh"
#include <imagine/gui/TextEntry.hh>
#include <imagine/gui/TextTableView.hh>
#include <imagine/gui/AlertView.hh>
#include <imagine/base/ApplicationContext.hh>
#include <imagine/gfx/RendererCommands.hh>
#include <imagine/bluetooth/BluetoothAdapter.hh>
#include <imagine/util/ScopeGuard.hh>
#include <imagine/util/format.hh>
#include <imagine/util/variant.hh>
#include <imagine/util/bit.hh>
#include <imagine/logger/logger.h>

namespace EmuEx
{

constexpr SystemLogger log{"InputManagerView"};
constexpr auto confirmDeleteDeviceSettingsStr =
	UI_TEXT("是否要删除当前的设备设置？使用中的按键配置会被保留");
constexpr auto confirmDeleteProfileStr =
	UI_TEXT("是否要删除当前的按键配置？使用它的设备将恢复默认的按键配置");

IdentInputDeviceView::IdentInputDeviceView(ViewAttachParams attach):
	View(attach),
	text
	{
		attach.rendererTask,
		UI_TEXT("在输入设备上按任意键即可进入其设置菜单"),
		&defaultFace()
	},
	quads{attach.rendererTask, {.size = 1}} {}

void IdentInputDeviceView::place()
{
	quads.write(0, {.bounds = displayRect().as<int16_t>()});
	text.compile({.maxLineSize = int(viewRect().xSize() * 0.95f)});
}

bool IdentInputDeviceView::inputEvent(const Input::Event& e, ViewInputEventParams)
{
	return e.visit(overloaded
	{
		[&](const Input::MotionEvent &e)
		{
			if(e.released())
			{
				dismiss();
				return true;
			}
			return false;
		},
		[&](const Input::KeyEvent &e)
		{
			if(e.pushed())
			{
				auto del = onIdentInput;
				dismiss();
				del(e);
				return true;
			}
			return false;
		}
	});
}

void IdentInputDeviceView::draw(Gfx::RendererCommands&__restrict__ cmds, ViewDrawParams) const
{
	using namespace IG::Gfx;
	auto &basicEffect = cmds.basicEffect();
	cmds.set(BlendMode::OFF);
	basicEffect.disableTexture(cmds);
	cmds.setColor({.4, .4, .4});
	cmds.drawQuad(quads, 0);
	basicEffect.enableAlphaTexture(cmds);
	text.draw(cmds, viewRect().center(), C2DO, ColorName::WHITE);
}

InputManagerView::InputManagerView(ViewAttachParams attach,
	InputManager &inputManager_):
	TableView
	{
		UI_TEXT("实体控制器设置"),
		attach, item
	},
	inputManager{inputManager_},
	deleteDeviceConfig
	{
		UI_TEXT("删除设备设置"),
		attach,
		[this](TextMenuItem &item, View &, const Input::Event &e)
		{
			auto &savedDevConfigs = inputManager.savedDevConfigs;
			if(!savedDevConfigs.size())
			{
				app().postMessage(
					UI_TEXT("无效的设备设置")
				);
				return;
			}
			auto multiChoiceView = makeViewWithName<TextTableView>(item, savedDevConfigs.size());
			for(auto &ePtr : savedDevConfigs)
			{
				multiChoiceView->appendItem(Input::Device::makeDisplayName(ePtr->name, ePtr->enumId),
					[this, &deleteDeviceConfig = *ePtr](const Input::Event &e)
					{
						pushAndShowModal(makeView<YesNoAlertView>(confirmDeleteDeviceSettingsStr,
							YesNoAlertView::Delegates
							{
								.onYes = [this, &deleteDeviceConfig]
								{
									inputManager.deleteDeviceSavedConfig(appContext(), deleteDeviceConfig);
									dismissPrevious();
								}
							}), e);
					});
			}
			pushAndShow(std::move(multiChoiceView), e);
		}
	},
	deleteProfile
	{
		UI_TEXT("删除按键配置"),
		attach,
		[this](TextMenuItem &item, View &, const Input::Event &e)
		{
			auto &customKeyConfigs = inputManager.customKeyConfigs;
			if(!customKeyConfigs.size())
			{
				app().postMessage(
					UI_TEXT("无效的按键配置")
				);
				return;
			}
			auto multiChoiceView = makeViewWithName<TextTableView>(item, customKeyConfigs.size());
			for(auto &ePtr : customKeyConfigs)
			{
				multiChoiceView->appendItem(ePtr->name,
					[this, deleteProfilePtr = ePtr.get()](const Input::Event &e)
					{
						pushAndShowModal(makeView<YesNoAlertView>(confirmDeleteProfileStr,
							YesNoAlertView::Delegates
							{
								.onYes = [this, deleteProfilePtr]
								{
									log.info("deleting profile:{}", deleteProfilePtr->name);
									inputManager.deleteKeyProfile(appContext(), deleteProfilePtr);
									dismissPrevious();
								}
							}), e);
					});
			}
			pushAndShow(std::move(multiChoiceView), e);
		}
	},
	rescanOSDevices
	{
		UI_TEXT("重新检测系统输入设备"),
		attach,
		[this](const Input::Event &e)
		{
			appContext().enumInputDevices();
			int devices = 0;
			auto ctx = appContext();
			for(auto &e : ctx.inputDevices())
			{
				if(e->map() == Input::Map::SYSTEM)
					devices++;
			}
			app().postMessage(2, false, std::format(
				UI_TEXT("检测到 {} 个系统设备"),
				devices));
		}
	},
	identDevice
	{
		UI_TEXT("自动检测设备并设置"),
		attach,
		[this](const Input::Event &e)
		{
			auto identView = makeView<IdentInputDeviceView>();
			identView->onIdentInput =
				[this](const Input::Event &e)
				{
					auto dev = e.device();
					if(dev)
					{
						pushAndShowDeviceView(*dev, e);
					}
				};
			pushAndShowModal(std::move(identView), e);
		}
	},
	generalOptions
	{
		UI_TEXT("常规选项"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShow(makeView<InputManagerOptionsView>(app().viewController().inputView), e);
		}
	},
	deviceListHeading
	{
		UI_TEXT("设备设置列表："),
		attach,
	}
{
	inputManager.onUpdateDevices = [this]()
	{
		popTo(*this);
		auto selectedCell = selected;
		loadItems();
		highlightCell(selectedCell);
		place();
		show();
	};
	deleteDeviceConfig.setActive(inputManager.savedDevConfigs.size());
	deleteProfile.setActive(inputManager.customKeyConfigs.size());
	loadItems();
}

InputManagerView::~InputManagerView()
{
	inputManager.onUpdateDevices = {};
}

void InputManagerView::loadItems()
{
	auto ctx = appContext();
	item.clear();
	item.reserve(16);
	item.emplace_back(&identDevice);
	item.emplace_back(&generalOptions);
	item.emplace_back(&deleteDeviceConfig);
	item.emplace_back(&deleteProfile);
	doIfUsed(rescanOSDevices, [&](auto &mItem)
	{
		if(ctx.androidSDK() >= 12 && ctx.androidSDK() < 16)
			item.emplace_back(&mItem);
	});
	item.emplace_back(&deviceListHeading);
	inputDevName.clear();
	inputDevName.reserve(ctx.inputDevices().size());
	for(auto &devPtr : ctx.inputDevices())
	{
		auto &devItem = inputDevName.emplace_back(inputDevData(*devPtr).displayName, attachParams(),
			[this, &dev = *devPtr](const Input::Event &e)
			{
				pushAndShowDeviceView(dev, e);
			});
		if(devPtr->hasKeys() && !devPtr->isPowerButton())
		{
			item.emplace_back(&devItem);
		}
		else
		{
			log.info("not adding device:{} to list", devPtr->name());
		}
	}
}

void InputManagerView::onShow()
{
	TableView::onShow();
	deleteDeviceConfig.setActive(inputManager.savedDevConfigs.size());
	deleteProfile.setActive(inputManager.customKeyConfigs.size());
}

void InputManagerView::pushAndShowDeviceView(const Input::Device &dev, const Input::Event &e)
{
	pushAndShow(makeViewWithName<InputManagerDeviceView>(inputDevData(dev).displayName, *this, dev, inputManager), e);
}

InputManagerOptionsView::InputManagerOptionsView(ViewAttachParams attach, EmuInputView& emuInputView_):
	TableView
	{
		UI_TEXT("常规选项"),
		attach, item
	},
	mogaInputSystem
	{
		UI_TEXT("MOGA 手柄支持"),
		attach,
		app().mogaManagerIsActive(),
		[this](BoolMenuItem &item)
		{
			if(!app().mogaManagerIsActive() && !appContext().packageIsInstalled("com.bda.pivot.mogapgp"))
			{
				app().postMessage(8,
					UI_TEXT("在使用 MOGA Pocket 之前，请先从谷歌 Play 商店下载并安装 MOGA Pivot 应用程序。 ")
					UI_TEXT("MOGA Pro 或更新型号的手柄, 需要先切换到模式 B，然后在安卓设备的设置中进行蓝牙配对连接。")
				);
				return;
			}
			app().setMogaManagerActive(item.flipBoolValue(*this), true);
		}
	},
	notifyDeviceChange
	{
		UI_TEXT("设备变更通知"),
		attach,
		app().notifyOnInputDeviceChange,
		[this](BoolMenuItem &item)
		{
			app().notifyOnInputDeviceChange = item.flipBoolValue(*this);
		}
	},
	bluetoothHeading
	{
		UI_TEXT("应用程序运行时的蓝牙选项："),
		attach,
	},
	keepBtActive
	{
		UI_TEXT("后台运行时仍保持连接"),
		attach,
		app().keepBluetoothActive,
		[this](BoolMenuItem &item)
		{
			app().keepBluetoothActive = item.flipBoolValue(*this);
		}
	},
	btScanSecsItem
	{
		{"2secs",  attach, MenuItem::Config{.id = 2}},
		{"4secs",  attach, MenuItem::Config{.id = 4}},
		{"6secs",  attach, MenuItem::Config{.id = 6}},
		{"8secs",  attach, MenuItem::Config{.id = 8}},
		{"10secs", attach, MenuItem::Config{.id = 10}}
	},
	btScanSecs
	{
		UI_TEXT("检测时间"),
		attach,
		MenuId{app().bluetoothAdapter.scanSecs},
		btScanSecsItem,
		MultiChoiceMenuItem::Config
		{
			.defaultItemOnSelect = [this](TextMenuItem &item) { app().bluetoothAdapter.scanSecs = item.id; }
		}
	},
	btScanCache
	{
		UI_TEXT("缓存检测结果"),
		attach,
		app().bluetoothAdapter.useScanCache,
		[this](BoolMenuItem &item)
		{
			app().bluetoothAdapter.useScanCache = item.flipBoolValue(*this);
		}
	},
	altGamepadConfirm
	{
		UI_TEXT("【确认键】和【取消键】交换位置"),
		attach,
		app().swappedConfirmKeys(),
		[this](BoolMenuItem &item)
		{
			app().setSwappedConfirmKeys(item.flipBoolValue(*this));
		}
	},
	emuInputView{emuInputView_}
{
	if constexpr(MOGA_INPUT)
	{
		item.emplace_back(&mogaInputSystem);
	}
	item.emplace_back(&altGamepadConfirm);
	#if 0
	if(Input::hasTrackball())
	{
		item.emplace_back(&relativePointerDecel);
	}
	#endif
	if(appContext().hasInputDeviceHotSwap())
	{
		item.emplace_back(&notifyDeviceChange);
	}
	if(used(bluetoothHeading))
	{
		item.emplace_back(&bluetoothHeading);
		if(used(keepBtActive))
		{
			item.emplace_back(&keepBtActive);
		}
		if(used(btScanSecs))
		{
			item.emplace_back(&btScanSecs);
		}
		if(used(btScanCache))
		{
			item.emplace_back(&btScanCache);
		}
	}
}

static bool customKeyConfigsContainName(auto &customKeyConfigs, std::string_view name)
{
	return find(customKeyConfigs, [&](auto &confPtr){ return confPtr->name == name; }).has_value();
}

InputManagerDeviceView::InputManagerDeviceView(UTF16String name, ViewAttachParams attach,
	InputManagerView &rootIMView_, const Input::Device &dev, InputManager &inputManager_):
	TableView{std::move(name), attach, item},
	inputManager{inputManager_},
	rootIMView{rootIMView_},
	playerItems
	{
		[&]
		{
			DynArray<TextMenuItem> items{EmuSystem::maxPlayers + 1uz};
			items[0] =
				{
					UI_TEXT("多人合作"),
					attach, {.id = playerIndexMulti}
				};
			for(auto i : iotaCount(EmuSystem::maxPlayers))
			{
				items[i + 1] = {playerNumStrings[i], attach, {.id = i}};
			}
			return items;
		}()
	},
	player
	{
		UI_TEXT("玩家索引"),
		attach,
		MenuId{inputDevData(dev).devConf.savedPlayer()},
		playerItems,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				auto playerVal = item.id;
				bool changingMultiplayer = (playerVal == playerIndexMulti && devConf.savedPlayer() != playerIndexMulti) ||
					(playerVal != playerIndexMulti && devConf.savedPlayer() == playerIndexMulti);
				devConf.setSavedPlayer(inputManager, playerVal);
				if(changingMultiplayer)
				{
					loadItems();
					place();
					show();
				}
				else
					onShow();
			}
		},
	},
	loadProfile
	{
		u"", attach,
		[this](const Input::Event &e)
		{
			auto profileSelectMenu = makeView<ProfileSelectView>(devConf.device().map(),
				devConf.keyConf(inputManager).name, app());
			profileSelectMenu->onProfileChange =
					[this](std::string_view profile)
					{
						log.info("set key profile:{}", profile);
						devConf.setKeyConfName(inputManager, profile);
						onShow();
					};
			pushAndShow(std::move(profileSelectMenu), e);
		}
	},
	renameProfile
	{
		UI_TEXT("重命名配置"),
		attach,
		[this](const Input::Event &e)
		{
			if(!devConf.mutableKeyConf(inputManager))
			{
				app().postMessage(2,
					UI_TEXT("无法重命名应用程序自带的配置")
				);
				return;
			}
			pushAndShowNewCollectValueInputView<const char*>(attachParams(), e,
				UI_TEXT("请输入按键配置的名称"),
				devConf.keyConf(inputManager).name,
				[this](CollectTextInputView &, auto str)
				{
					if(customKeyConfigsContainName(inputManager.customKeyConfigs, str))
					{
						app().postErrorMessage(
							UI_TEXT("与已有的配置命名冲突")
						);
						postDraw();
						return false;
					}
					devConf.mutableKeyConf(inputManager)->name = str;
					onShow();
					postDraw();
					return true;
				});
		}
	},
	newProfile
	{
		UI_TEXT("新建配置"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShowModal(makeView<YesNoAlertView>(
				UI_TEXT("是否要新建一个按键配置？新的配置内容会以当前配置为初始值。"),
				YesNoAlertView::Delegates
				{
					.onYes = [this](const Input::Event &e)
					{
						pushAndShowNewCollectValueInputView<const char*>(attachParams(), e,
							UI_TEXT("请输入按键配置的名称"),
							"",
							[this](CollectTextInputView &, auto str)
							{
								if(customKeyConfigsContainName(inputManager.customKeyConfigs, str))
								{
									app().postErrorMessage(
										UI_TEXT("与已有的配置命名冲突")
									);
									return false;
								}
								devConf.setKeyConfCopiedFromExisting(inputManager, str);
								log.info("created new profile:{}", devConf.keyConf(inputManager).name);
								onShow();
								postDraw();
								return true;
							});
					}
				}), e);
		}
	},
	deleteProfile
	{
		UI_TEXT("删除配置"),
		attach,
		[this](const Input::Event &e)
		{
			if(!devConf.mutableKeyConf(inputManager))
			{
				app().postMessage(2,
					UI_TEXT("无法删除应用程序自带的配置")
				);
				return;
			}
			pushAndShowModal(makeView<YesNoAlertView>(confirmDeleteProfileStr,
				YesNoAlertView::Delegates
				{
					.onYes = [this]
					{
						auto conf = devConf.mutableKeyConf(inputManager);
						if(!conf)
						{
							bug_unreachable("confirmed deletion of a read-only key config, should never happen");
						}
						log.info("deleting profile:{}", conf->name);
						inputManager.deleteKeyProfile(appContext(), conf);
					}
				}), e);
		}
	},
	iCadeMode
	{
		UI_TEXT("iCade 模式"),
		attach,
		inputDevData(dev).devConf.iCadeMode(),
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			if constexpr(Config::envIsIOS)
			{
				confirmICadeMode();
			}
			else
			{
				if(!item.boolValue())
				{
					pushAndShowModal(makeView<YesNoAlertView>(
						UI_TEXT("开启此模式后可以用兼容 iCade 的蓝牙设备进行输入，非 iCade 设备请不要开启"),
						UI_TEXT("开启"),
						UI_TEXT("取消"),
						YesNoAlertView::Delegates{.onYes = [this]{ confirmICadeMode(); }}), e);
				}
				else
					confirmICadeMode();
			}
		}
	},
	consumeUnboundKeys
	{
		UI_TEXT("响应未映射的按键"),
		attach,
		inputDevData(dev).devConf.shouldHandleUnboundKeys,
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			devConf.shouldHandleUnboundKeys = item.flipBoolValue(*this);
			devConf.save(inputManager);
		}
	},
	joystickAxisStick1Keys
	{
		UI_TEXT("使用1号摇杆作为方向键"),
		attach,
		inputDevData(dev).devConf.joystickAxesAsKeys(Input::AxisSetId::stick1),
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			devConf.setJoystickAxesAsKeys(Input::AxisSetId::stick1, item.flipBoolValue(*this));
			devConf.save(inputManager);
		}
	},
	joystickAxisStick2Keys
	{
		UI_TEXT("使用2号摇杆作为方向键"),
		attach,
		inputDevData(dev).devConf.joystickAxesAsKeys(Input::AxisSetId::stick2),
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			devConf.setJoystickAxesAsKeys(Input::AxisSetId::stick2, item.flipBoolValue(*this));
			devConf.save(inputManager);
		}
	},
	joystickAxisHatKeys
	{
		UI_TEXT("使用飞行摇杆作为方向键"),
		attach,
		inputDevData(dev).devConf.joystickAxesAsKeys(Input::AxisSetId::hat),
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			devConf.setJoystickAxesAsKeys(Input::AxisSetId::hat, item.flipBoolValue(*this));
			devConf.save(inputManager);
		}
	},
	joystickAxisTriggerKeys
	{
		UI_TEXT("使用左右扳机作为 L2/R2 键"),
		attach,
		inputDevData(dev).devConf.joystickAxesAsKeys(Input::AxisSetId::triggers),
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			devConf.setJoystickAxesAsKeys(Input::AxisSetId::triggers, item.flipBoolValue(*this));
			devConf.save(inputManager);
		}
	},
	joystickAxisPedalKeys
	{
		UI_TEXT("使用刹车/油门作为 L2/R2 键"),
		attach,
		inputDevData(dev).devConf.joystickAxesAsKeys(Input::AxisSetId::pedals),
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			devConf.setJoystickAxesAsKeys(Input::AxisSetId::pedals, item.flipBoolValue(*this));
			devConf.save(inputManager);
		}
	},
	categories
	{
		UI_TEXT("按键映射："),
		attach
	},
	options
	{
		UI_TEXT("选项："),
		attach
	},
	joystickSetup
	{
		UI_TEXT("摇杆设置："),
		attach
	},
	devConf{inputDevData(dev).devConf}
{
	loadProfile.setName(std::format(
		UI_TEXT("按键配置：{}"),
		devConf.keyConf(inputManager).name));
	renameProfile.setActive(devConf.mutableKeyConf(inputManager));
	deleteProfile.setActive(devConf.mutableKeyConf(inputManager));
	loadItems();
}

void InputManagerDeviceView::addCategoryItem(const KeyCategory &cat)
{
	auto &catItem = inputCategory.emplace_back(cat.name, attachParams(),
		[this, &cat](const Input::Event &e)
		{
			pushAndShow(makeView<ButtonConfigView>(rootIMView, cat, devConf), e);
		});
	item.emplace_back(&catItem);
}

void InputManagerDeviceView::loadItems()
{
	auto &dev = devConf.device();
	item.clear();
	auto categoryCount = EmuApp::keyCategories().size();
	bool hasJoystick = dev.motionAxes().size();
	auto joystickItemCount = hasJoystick ? 9 : 0;
	item.reserve(categoryCount + joystickItemCount + 12);
	inputCategory.clear();
	inputCategory.reserve(categoryCount + 1);
	if(EmuSystem::maxPlayers > 1)
	{
		item.emplace_back(&player);
	}
	item.emplace_back(&loadProfile);
	item.emplace_back(&categories);
	addCategoryItem(appKeyCategory);
	for(auto &cat : EmuApp::keyCategories())
	{
		if(cat.multiplayerIndex && devConf.savedPlayer() != playerIndexMulti)
			continue;
		addCategoryItem(cat);
	}
	item.emplace_back(&options);
	item.emplace_back(&newProfile);
	item.emplace_back(&renameProfile);
	item.emplace_back(&deleteProfile);
	if(hasICadeInput && (dev.map() == Input::Map::SYSTEM && dev.hasKeyboard()))
	{
		item.emplace_back(&iCadeMode);
	}
	if constexpr(Config::envIsAndroid)
	{
		item.emplace_back(&consumeUnboundKeys);
	}
	if(hasJoystick)
	{
		item.emplace_back(&joystickSetup);
		if(dev.motionAxis(Input::AxisId::X))
			item.emplace_back(&joystickAxisStick1Keys);
		if(dev.motionAxis(Input::AxisId::Z))
			item.emplace_back(&joystickAxisStick2Keys);
		if(dev.motionAxis(Input::AxisId::HAT0X))
			item.emplace_back(&joystickAxisHatKeys);
		if(dev.motionAxis(Input::AxisId::LTRIGGER))
			item.emplace_back(&joystickAxisTriggerKeys);
		if(dev.motionAxis(Input::AxisId::BRAKE))
			item.emplace_back(&joystickAxisPedalKeys);
	}
}

void InputManagerDeviceView::onShow()
{
	TableView::onShow();
	loadProfile.compile(std::format(
		UI_TEXT("按键配置：{}"),
		devConf.keyConf(inputManager).name));
	bool keyConfIsMutable = devConf.mutableKeyConf(inputManager);
	renameProfile.setActive(keyConfIsMutable);
	deleteProfile.setActive(keyConfIsMutable);
}

void InputManagerDeviceView::confirmICadeMode()
{
	devConf.setICadeMode(iCadeMode.flipBoolValue(*this));
	devConf.save(inputManager);
	onShow();
	app().defaultVController().setPhysicalControlsPresent(appContext().keyInputIsPresent());
}

}
