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

#include <emuframework/MainMenuView.hh>
#include <emuframework/EmuApp.hh>
#include <emuframework/EmuSystem.hh>
#include <emuframework/CreditsView.hh>
#include <emuframework/FilePicker.hh>
#include <emuframework/VideoOptionView.hh>
#include <emuframework/TouchConfigView.hh>
#include <emuframework/BundledGamesView.hh>
#include "InputManagerView.hh"
#include "RecentContentView.hh"
#include "FrameTimingView.hh"
#include <emuframework/EmuOptions.hh>
#include <imagine/gui/AlertView.hh>
#include <imagine/base/ApplicationContext.hh>
#include <imagine/fs/FS.hh>
#include <imagine/bluetooth/BluetoothInputDevice.hh>
#include <format>
#include <imagine/logger/logger.h>

namespace EmuEx
{

constexpr SystemLogger log{"AppMenus"};

class OptionCategoryView : public TableView, public EmuAppHelper
{
public:
	OptionCategoryView(ViewAttachParams attach);

protected:
	TextMenuItem subConfig[8];
};

static void onScanStatus(EmuApp &app, BluetoothScanState status, int arg);

template <class ViewT>
static void handledFailedBTAdapterInit(ViewT& view, [[maybe_unused]] ViewAttachParams attach, [[maybe_unused]] const Input::Event& e)
{
	view.app().postErrorMessage(
		UI_TEXT("无法初始化蓝牙适配器")
	);
	#ifdef CONFIG_BLUETOOTH_BTSTACK
	if(!FS::exists("/var/lib/dpkg/info/ch.ringwald.btstack.list"))
	{
		view.pushAndShowModal(std::make_unique<YesNoAlertView>(attach,
			UI_TEXT("BTstack not found, open Cydia and install?"),
			YesNoAlertView::Delegates
			{
				.onYes = [](View &v){ v.appContext().openURL("cydia://package/ch.ringwald.btstack"); }
			}), e, false);
	}
	#endif
}

MainMenuView::MainMenuView(ViewAttachParams attach, bool customMenu):
	TableView{EmuApp::mainViewName(), attach, item},
	loadGame
	{
		UI_TEXT("打开游戏"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShow(FilePicker::forLoading(attachParams(), e), e, false);
		}
	},
	systemActions
	{
		EmuApp::ViewName::SYSTEM_ACTIONS,
		attach,
		[this](const Input::Event &e)
		{
			if(!system().hasContent())
				return;
			pushAndShow(app().makeView(attachParams(), EmuApp::ViewID::SYSTEM_ACTIONS), e);
		}
	},
	recentGames
	{
		UI_TEXT("最近打开过的游戏"),
		attach,
		[this](const Input::Event &e)
		{
			if(app().recentContent.size())
			{
				pushAndShow(makeView<RecentContentView>(app().recentContent), e);
			}
		}
	},
	bundledGames
	{
		UI_TEXT("自带游戏"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShow(makeView<BundledGamesView>(), e);
		}
	},
	options
	{
		UI_TEXT("选项"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShow(makeView<OptionCategoryView>(), e);
		}
	},
	onScreenInputManager
	{
		UI_TEXT("屏幕按键设置"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShow(makeView<TouchConfigView>(app().defaultVController()), e);
		}
	},
	inputManager
	{
		UI_TEXT("实体控制器设置"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShow(makeView<InputManagerView>(app().inputManager), e);
		}
	},
	benchmark
	{
		UI_TEXT("检查游戏"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShow(FilePicker::forBenchmarking(attachParams(), e), e, false);
		}
	},
	scanWiimotes
	{
		UI_TEXT("检测 Wiimotes/iCP/JS1"),
		attach,
		[this](const Input::Event &e)
		{
			app().bluetoothAdapter.openDefault();
			if(app().bluetoothAdapter.isOpen())
			{
				if(Bluetooth::scanForDevices(appContext(), app().bluetoothAdapter,
					[this](BluetoothAdapter &, BluetoothScanState status, int arg)
					{
						onScanStatus(app(), status, arg);
					}))
				{
					app().postMessage(4, false,
						UI_TEXT("正在启动检测...\n(请访问设备官网以获取详细的产品使用说明)")
					);
				}
				else
				{
					app().postMessage(1, false,
						UI_TEXT("检测中")
					);
				}
			}
			else
			{
				handledFailedBTAdapterInit(*this, attachParams(), e);
			}
			postDraw();
		}
	},
	bluetoothDisconnect
	{
		UI_TEXT("断开蓝牙连接"),
		attach,
		[this](const Input::Event &e)
		{
			auto devConnected = Bluetooth::devsConnected(appContext());
			if(devConnected)
			{
				pushAndShowModal(makeView<YesNoAlertView>(std::format(
					UI_TEXT("是否要断开 {} 蓝牙设备？"),
					devConnected),
					YesNoAlertView::Delegates{.onYes = [this]{ app().closeBluetoothConnections(); }}), e);
			}
		}
	},
	acceptPS3ControllerConnection
	{
		UI_TEXT("检测 PS3 手柄"),
		attach,
		[this](const Input::Event &e)
		{
			app().bluetoothAdapter.openDefault();
			if(app().bluetoothAdapter.isOpen())
			{
				app().postMessage(4,
					UI_TEXT("准备按下【PS 键】")
				);
				auto startedScan = Bluetooth::listenForDevices(appContext(), app().bluetoothAdapter,
					[this](BluetoothAdapter&, BluetoothScanState status, int arg)
					{
						switch(status)
						{
							case BluetoothScanState::InitFailed:
							{
								app().postErrorMessage(Config::envIsLinux ? 8 : 2,
									Config::envIsLinux ?
										UI_TEXT("无法注册服务器，请确保此可执行文件已启用 cap_net_bind_service，并且 bluetoothd 进程没有在运行") :
										UI_TEXT("蓝牙设置失败"));
								break;
							}
							case BluetoothScanState::Complete:
							{
								app().postMessage(4,
									UI_TEXT("请按下手柄上的【PS 键】\n(请访问设备官网以获取详细的配对说明)")
								);
								break;
							}
							default: onScanStatus(app(), status, arg);
						}
					});
				if(!startedScan)
				{
					app().postMessage(1,
						UI_TEXT("检测中")
					);
				}
			}
			else
			{
				handledFailedBTAdapterInit(*this, attachParams(), e);
			}
			postDraw();
		}
	},
	about
	{
		UI_TEXT("关于"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShow(makeView<CreditsView>(EmuSystem::creditsViewStr), e);
		}
	},
	exitApp
	{
		UI_TEXT("退出"),
		attach,
		[this]()
		{
			appContext().exit();
		}
	}
{
	if(!customMenu)
	{
		reloadItems();
	}
}

static void onScanStatus(EmuApp &app, BluetoothScanState status, int arg)
{
	switch(status)
	{
		case BluetoothScanState::InitFailed:
		{
			if(Config::envIsIOS)
			{
				app.postErrorMessage(
					UI_TEXT("BTstack 电源打开失败，请确保 iOS 的蓝牙协议栈没有被激活")
				);
			}
			break;
		}
		case BluetoothScanState::Failed:
		{
			app.postErrorMessage(
				UI_TEXT("检测失败")
			);
			break;
		}
		case BluetoothScanState::NoDevs:
		{
			app.postMessage(
				UI_TEXT("没有发现蓝牙设备")
			);
			break;
		}
		case BluetoothScanState::Processing:
		{
			app.postMessage(2, 0, std::format(
				UI_TEXT("正在检测 {} 设备..."),
				arg));
			break;
		}
		case BluetoothScanState::NameFailed:
		{
			app.postErrorMessage(
				UI_TEXT("读取设备名称失败")
			);
			break;
		}
		case BluetoothScanState::Complete:
		{
			int devs = Bluetooth::pendingDevs();
			if(devs)
			{
				app.postMessage(2, 0, std::format(
					UI_TEXT("正在连接 {} 设备..."),
					devs));
				Bluetooth::connectPendingDevs(app.bluetoothAdapter);
			}
			else
			{
				app.postMessage(
					UI_TEXT("检测完成，没有可识别的设备")
				);
			}
			break;
		}
		case BluetoothScanState::Cancelled: break;
		/*case BluetoothScanState::SocketOpenFailed:
		{
			app.postErrorMessage("Failed opening a Bluetooth connection");
		}*/
	}
};

void MainMenuView::onShow()
{
	TableView::onShow();
	log.info("refreshing main menu state");
	recentGames.setActive(app().recentContent.size());
	systemActions.setActive(system().hasContent());
	bluetoothDisconnect.setActive(Bluetooth::devsConnected(appContext()));
}

void MainMenuView::loadFileBrowserItems()
{
	item.emplace_back(&loadGame);
	item.emplace_back(&recentGames);
	if(EmuSystem::hasBundledGames && app().showsBundledGames)
	{
		item.emplace_back(&bundledGames);
	}
}

void MainMenuView::loadStandardItems()
{
	item.emplace_back(&systemActions);
	item.emplace_back(&onScreenInputManager);
	item.emplace_back(&inputManager);
	item.emplace_back(&options);
	if(used(scanWiimotes) && app().showsBluetoothScan)
	{
		item.emplace_back(&scanWiimotes);
		if(used(acceptPS3ControllerConnection))
			item.emplace_back(&acceptPS3ControllerConnection);
		item.emplace_back(&bluetoothDisconnect);
	}
	item.emplace_back(&benchmark);
	item.emplace_back(&about);
	item.emplace_back(&exitApp);
}

void MainMenuView::reloadItems()
{
	item.clear();
	loadFileBrowserItems();
	loadStandardItems();
}

OptionCategoryView::OptionCategoryView(ViewAttachParams attach):
	TableView
	{
		UI_TEXT("选项"),
		attach,
		[this](ItemMessage msg) -> ItemReply
		{
			return msg.visit(overloaded
			{
				[&](const ItemsMessage&) -> ItemReply { return EmuApp::hasGooglePlayStoreFeatures() ? std::size(subConfig) : std::size(subConfig)-1; },
				[&](const GetItemMessage& m) -> ItemReply { return &subConfig[m.idx]; },
			});
		}
	},
	subConfig
	{
		{
			UI_TEXT("渲染耗时"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShow(makeView<FrameTimingView>(), e);
			}
		},
		{
			UI_TEXT("视频"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShow(app().makeView(attachParams(), EmuApp::ViewID::VIDEO_OPTIONS), e);
			}
		},
		{
			UI_TEXT("音频"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShow(app().makeView(attachParams(), EmuApp::ViewID::AUDIO_OPTIONS), e);
			}
		},
		{
			UI_TEXT("系统"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShow(app().makeView(attachParams(), EmuApp::ViewID::SYSTEM_OPTIONS), e);
			}
		},
		{
			UI_TEXT("文件路径"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShow(app().makeView(attachParams(), EmuApp::ViewID::FILE_PATH_OPTIONS), e);
			}
		},
		{
			UI_TEXT("界面"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShow(app().makeView(attachParams(), EmuApp::ViewID::GUI_OPTIONS), e);
			}
		},
		{
			UI_TEXT("在线文档"),
			attach,
			[this]
			{
				appContext().openURL("https://www.explusalpha.com/contents/emuex/documentation");
			}
		}
	}
{
	if(EmuApp::hasGooglePlayStoreFeatures())
	{
		subConfig[lastIndex(subConfig)] =
		{
			UI_TEXT("Beta Testing Opt-in/out"),
			attach,
			[this]()
			{
				appContext().openURL(std::format("https://play.google.com/apps/testing/{}", appContext().applicationId));
			}
		};
	}
}

}
