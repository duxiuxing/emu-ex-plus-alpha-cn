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

#include "FrameTimingView.hh"
#include <emuframework/EmuApp.hh>
#include <emuframework/EmuAppHelper.hh>
#include <emuframework/EmuViewController.hh>
#include <emuframework/viewUtils.hh>
#include <imagine/base/Screen.hh>
#include <imagine/base/ApplicationContext.hh>
#include <imagine/gfx/Renderer.hh>
#include <imagine/gfx/RendererCommands.hh>
import std;

namespace EmuEx
{

static std::string makeFrameRateStr(VideoSystem vidSys, const OutputTimingManager& mgr)
{
	auto opt = mgr.frameRateOption(vidSys);
	if(opt == OutputTimingManager::autoOption)
		return UI_TEXT("Auto");
	else if(opt == OutputTimingManager::originalOption)
		return UI_TEXT("Original");
	else
		return std::format(
			UI_TEXT("{:g}Hz"),
			toHz(opt));
}

FrameTimingView::FrameTimingView(ViewAttachParams attach):
	TableView
	{
		UI_TEXT("Frame Timing Options"),
		attach, item
	},
	frameIntervalItem
	{
		{
			UI_TEXT("Full (No Skip)"),
			attach, {.id = 0}
		},
		{
			UI_TEXT("Full"),
			attach, {.id = 1}
		},
		{
			UI_TEXT("1/2"),
			attach, {.id = 2}
		},
		{
			UI_TEXT("1/3"),
			attach, {.id = 3}
		},
		{
			UI_TEXT("1/4"),
			attach, {.id = 4}
		},
	},
	frameInterval
	{
		UI_TEXT("Frame Rate Target"),
		attach,
		MenuId{app().frameInterval},
		frameIntervalItem,
		MultiChoiceMenuItem::Config
		{
			.defaultItemOnSelect = [this](TextMenuItem &item) { app().frameInterval.setUnchecked(item.id); }
		},
	},
	frameRateItems
	{
		{
			UI_TEXT("Auto (Match screen when rates are similar)"),
			attach,
			[this]
			{
				onFrameRateChange(activeVideoSystem, OutputTimingManager::autoOption);
			}, {.id = OutputTimingManager::autoOption.count()}
		},
		{
			UI_TEXT("Original (Use emulated system's rate)"),
			attach,
			[this]
			{
				onFrameRateChange(activeVideoSystem, OutputTimingManager::originalOption);
			}, {.id = OutputTimingManager::originalOption.count()}
		},
		{
			UI_TEXT("Custom Rate"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShowNewCollectValueInputView<std::pair<double, double>>(attachParams(), e,
					UI_TEXT("Input decimal or fraction"),
					"",
					[this](CollectTextInputView&, auto val)
					{
						if(onFrameRateChange(activeVideoSystem, fromSeconds<SteadyClockDuration>(val.second / val.first)))
						{
							if(activeVideoSystem == VideoSystem::NATIVE_NTSC)
								frameRate.setSelected(defaultMenuId, *this);
							else
								frameRatePAL.setSelected(defaultMenuId, *this);
							dismissPrevious();
							return true;
						}
						else
							return false;
					});
				return false;
			}, {.id = defaultMenuId}
		},
	},
	frameRate
	{
		UI_TEXT("Input Rate"),
		attach,
		app().outputTimingManager.frameRateOptionAsMenuId(VideoSystem::NATIVE_NTSC),
		frameRateItems,
		{
			.onSetDisplayString = [this](auto, Gfx::Text& t)
			{
				t.resetString(makeFrameRateStr(VideoSystem::NATIVE_NTSC, app().outputTimingManager));
				return true;
			},
			.onSelect = [this](MultiChoiceMenuItem &item, View &view, const Input::Event &e)
			{
				activeVideoSystem = VideoSystem::NATIVE_NTSC;
				item.defaultOnSelect(view, e);
			},
		},
	},
	frameRatePAL
	{
		UI_TEXT("Input Rate (PAL)"),
		attach,
		app().outputTimingManager.frameRateOptionAsMenuId(VideoSystem::PAL),
		frameRateItems,
		{
			.onSetDisplayString = [this](auto, Gfx::Text& t)
			{
				t.resetString(makeFrameRateStr(VideoSystem::PAL, app().outputTimingManager));
				return true;
			},
			.onSelect = [this](MultiChoiceMenuItem &item, View &view, const Input::Event &e)
			{
				activeVideoSystem = VideoSystem::PAL;
				item.defaultOnSelect(view, e);
			},
		},
	},
	frameTimingStats
	{
		UI_TEXT("Show Frame Timing Stats"),
		attach,
		app().showFrameTimingStats,
		[this](BoolMenuItem &item) { app().showFrameTimingStats = item.flipBoolValue(*this); }
	},
	lowLatencyVideo
	{
		UI_TEXT("Low Latency Mode"),
		attach,
		app().lowLatencyVideo,
		[this](BoolMenuItem& item) { app().setLowLatencyVideo(item.flipBoolValue(*this)); }
	},
	frameClockItems
	{
		[&]()
		{
			StaticArrayList<TextMenuItem, maxFrameClockItems> frameClockItems;
			frameClockItems.emplace_back(
				UI_TEXT("Auto"),
				attach, MenuItem::Config{.id = FrameClockSource::Unset});
			if(app().emuWindow().supportsFrameClockSource(FrameClockSource::Screen))
				frameClockItems.emplace_back(
					UI_TEXT("Screen (For standard displays)"),
					attach, MenuItem::Config{.id = FrameClockSource::Screen});
			if(app().emuWindow().supportsFrameClockSource(FrameClockSource::Renderer))
				frameClockItems.emplace_back(
					UI_TEXT("Renderer (For drivers with double buffering)"),
					attach, MenuItem::Config{.id = FrameClockSource::Renderer});
			frameClockItems.emplace_back(
				UI_TEXT("Timer (For VRR displays)"),
				attach, MenuItem::Config{.id = FrameClockSource::Timer});
			return frameClockItems;
		}()
	},
	frameClock
	{
		UI_TEXT("Frame Clock"),
		attach,
		MenuId{FrameClockSource(app().frameClockSource)},
		frameClockItems,
		MultiChoiceMenuItem::Config
		{
			.onSetDisplayString = [this](auto, Gfx::Text& t)
			{
				t.resetString(wise_enum::to_string(app().effectiveFrameClockSource()));
				return true;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				app().frameClockSource = FrameClockSource(item.id.val);
				app().video.resetImage(); // texture can switch between single/double buffered
			}
		},
	},
	outputRateModeItems
	{
		{
			UI_TEXT("Auto"),
			attach, MenuItem::Config{.id = OutputFrameRateMode::Auto}
		},
		{
			UI_TEXT("Detect (Calculate rate during emulation)"),
			attach, MenuItem::Config{.id = OutputFrameRateMode::Detect}
		},
		{
			UI_TEXT("Screen (Use reported rate directly)"),
			attach, MenuItem::Config{.id = OutputFrameRateMode::Screen}
		},
	},
	outputRateMode
	{
		UI_TEXT("Output Rate"),
		attach,
		MenuId{OutputFrameRateMode(app().outputFrameRateMode)},
		outputRateModeItems,
		MultiChoiceMenuItem::Config
		{
			.onSetDisplayString = [this](auto, Gfx::Text& t)
			{
				t.resetString(wise_enum::to_string(app().effectiveOutputFrameRateMode()));
				return true;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				app().outputFrameRateMode = OutputFrameRateMode(item.id.val);
			}
		},
	},
	presentModeItems
	{
		{
			UI_TEXT("Auto"),
			attach, MenuItem::Config{.id = Gfx::PresentMode::Auto}
		},
		{
			UI_TEXT("Immediate (For VRR/VSync off use)"),
			attach, MenuItem::Config{.id = Gfx::PresentMode::Immediate}
		},
		{
			UI_TEXT("Queued (For standard use)"),
			attach, MenuItem::Config{.id = Gfx::PresentMode::FIFO}
		},
	},
	presentMode
	{
		UI_TEXT("Present Mode"),
		attach,
		MenuId{Gfx::PresentMode(app().presentMode)},
		presentModeItems,
		MultiChoiceMenuItem::Config
		{
			.onSetDisplayString = [this](auto, Gfx::Text& t)
			{
				t.resetString(renderer().evalPresentMode(app().emuWindow(), app().presentMode) == Gfx::PresentMode::FIFO
					? UI_TEXT("Queued")
					: UI_TEXT("Immediate")
				);
				return true;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				app().presentMode = Gfx::PresentMode(item.id.val);
			}
		},
	},
	screenFrameRateItems
	{
		[&]
		{
			std::vector<TextMenuItem> items;
			items.emplace_back(
				UI_TEXT("Off"),
				attach, [this]() { app().overrideScreenFrameRate = 0; }, MenuItem::Config{.id = 0});
			for(auto rate : app().emuScreen().supportedFrameRates())
			{
				doIfUsed(screenFrameRateItems, [&]([[maybe_unused]] auto& _)
				{
					items.emplace_back(
						std::format(
							UI_TEXT("{:g}Hz"),
							rate.hz()),
						attach,
						[this, hz = rate.hz()]()
						{
							app().overrideScreenFrameRate = hz;
						},
						MenuItem::Config{.id = std::bit_cast<MenuId>(float(rate.hz()))});
				});
			}
			return items;
		}()
	},
	screenFrameRate
	{
		UI_TEXT("Override Screen Frame Rate"),
		attach,
		std::bit_cast<MenuId>(float(app().overrideScreenFrameRate)),
		screenFrameRateItems
	},
	blankFrameInsertion
	{
		UI_TEXT("Allow Blank Frame Insertion"),
		attach,
		app().allowBlankFrameInsertion,
		[this](BoolMenuItem &item) { app().allowBlankFrameInsertion = item.flipBoolValue(*this); }
	},
	advancedHeading
	{
		UI_TEXT("Advanced"),
		attach
	}
{
	loadStockItems();
}

void FrameTimingView::loadStockItems()
{
	item.emplace_back(&frameInterval);
	item.emplace_back(&frameRate);
	if(EmuSystem::hasPALVideoSystem)
	{
		item.emplace_back(&frameRatePAL);
	}
	if(app().emuWindow().supportsFrameClockSource(FrameClockSource::Screen))
		item.emplace_back(&outputRateMode);
	item.emplace_back(&frameTimingStats);
	item.emplace_back(&advancedHeading);
	item.emplace_back(&frameClock);
	if(used(presentMode))
		item.emplace_back(&presentMode);
	item.emplace_back(&blankFrameInsertion);
	if(used(screenFrameRate) && app().emuScreen().supportedFrameRates().size() > 1)
		item.emplace_back(&screenFrameRate);
	item.emplace_back(&lowLatencyVideo);
}

bool FrameTimingView::onFrameRateChange(VideoSystem vidSys, SteadyClockDuration d)
{
	if(!app().outputTimingManager.setFrameRateOption(vidSys, d))
	{
		app().postMessage(4, true, std::format(
			UI_TEXT("{:g}Hz not in valid range"),
			toHz(d)));
		return false;
	}
	return true;
}

}
