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
#include <format>
#include <imagine/logger/logger.h>

namespace EmuEx
{

constexpr SystemLogger log{"FrameTimingView"};

class DetectFrameRateView final: public View, public EmuAppHelper
{
public:
	using DetectFrameRateDelegate = DelegateFunc<void (SteadyClockTime frameTime)>;
	DetectFrameRateDelegate onDetectFrameTime;
	IG::OnFrameDelegate detectFrameRate;
	SteadyClockTime totalFrameTime{};
	SteadyClockTimePoint lastFrameTimestamp{};
	Gfx::Text fpsText;
	int allTotalFrames{};
	int callbacks{};
	std::vector<SteadyClockTime> frameTimeSample{};
	bool useRenderTaskTime = false;

	DetectFrameRateView(ViewAttachParams attach): View(attach),
		fpsText{attach.rendererTask, &defaultFace()}
	{
		defaultFace().precacheAlphaNum(attach.renderer());
		defaultFace().precache(attach.renderer(), ".");
		fpsText.resetString("Preparing to detect frame rate...");
		useRenderTaskTime = !screen()->supportsTimestamps();
		frameTimeSample.reserve(std::round(screen()->frameRate() * 2.));
	}

	~DetectFrameRateView() final
	{
		window().setIntendedFrameRate(0);
		app().setCPUNeedsLowLatency(appContext(), false);
		window().removeOnFrame(detectFrameRate);
	}

	void place() final
	{
		fpsText.compile();
	}

	bool inputEvent(const Input::Event &e) final
	{
		if(e.keyEvent() && e.keyEvent()->pushed(Input::DefaultKey::CANCEL))
		{
			log.info("aborted detection");
			dismiss();
			return true;
		}
		return false;
	}

	void draw(Gfx::RendererCommands &__restrict__ cmds) final
	{
		using namespace IG::Gfx;
		cmds.basicEffect().enableAlphaTexture(cmds);
		fpsText.draw(cmds, viewRect().center(), C2DO, ColorName::WHITE);
	}

	bool runFrameTimeDetection(SteadyClockTime timestampDiff, double slack)
	{
		const int framesToTime = frameTimeSample.capacity() * 10;
		allTotalFrames++;
		frameTimeSample.emplace_back(timestampDiff);
		if(frameTimeSample.size() == frameTimeSample.capacity())
		{
			bool stableFrameTime = true;
			SteadyClockTime frameTimeTotal{};
			{
				SteadyClockTime lastFrameTime{};
				for(auto frameTime : frameTimeSample)
				{
					frameTimeTotal += frameTime;
					if(!stableFrameTime)
						continue;
					double frameTimeDiffSecs =
						std::abs(IG::FloatSeconds(lastFrameTime - frameTime).count());
					if(lastFrameTime.count() && frameTimeDiffSecs > slack)
					{
						log.info("frame times differed by:{}", frameTimeDiffSecs);
						stableFrameTime = false;
					}
					lastFrameTime = frameTime;
				}
			}
			auto frameTimeTotalSecs = FloatSeconds(frameTimeTotal);
			auto detectedFrameTimeSecs = frameTimeTotalSecs / (double)frameTimeSample.size();
			auto detectedFrameTime = round<SteadyClockTime>(detectedFrameTimeSecs);
			{
				if(detectedFrameTime.count())
					fpsText.resetString(std::format("{:g}fps", toHz(detectedFrameTimeSecs)));
				else
					fpsText.resetString("0fps");
				fpsText.compile();
			}
			if(stableFrameTime)
			{
				log.info("found frame time:{}", detectedFrameTimeSecs);
				onDetectFrameTime(detectedFrameTime);
				dismiss();
				return false;
			}
			frameTimeSample.erase(frameTimeSample.cbegin());
			postDraw();
		}
		else
		{
			//log.info("waiting for capacity:{}/{}", frameTimeSample.size(), frameTimeSample.capacity());
		}
		if(allTotalFrames >= framesToTime)
		{
			onDetectFrameTime(SteadyClockTime{});
			dismiss();
			return false;
		}
		else
		{
			if(useRenderTaskTime)
				postDraw();
			return true;
		}
	}

	void onAddedToController(ViewController *, const Input::Event &e) final
	{
		lastFrameTimestamp = SteadyClock::now();
		detectFrameRate =
			[this](IG::FrameParams params)
			{
				const int callbacksToSkip = 10;
				callbacks++;
				if(callbacks < callbacksToSkip)
				{
					if(useRenderTaskTime)
						postDraw();
					return true;
				}
				return runFrameTimeDetection(params.timestamp - std::exchange(lastFrameTimestamp, params.timestamp), 0.00175);
			};
		window().addOnFrame(detectFrameRate);
		app().setCPUNeedsLowLatency(appContext(), true);
	}
};

static std::string makeFrameRateStr(VideoSystem vidSys, const OutputTimingManager &mgr)
{
	auto frameTimeOpt = mgr.frameTimeOption(vidSys);
	if(frameTimeOpt == OutputTimingManager::autoOption)
		// return UI_TEXT("Auto");
		return UI_TEXT("自动");
	else if(frameTimeOpt == OutputTimingManager::originalOption)
		// return UI_TEXT("Original");
		return UI_TEXT("原始");
	else
		return std::format("{:g}Hz", toHz(frameTimeOpt));
}

FrameTimingView::FrameTimingView(ViewAttachParams attach):
	TableView
	{
		// UI_TEXT("Frame Timing Options"),
		UI_TEXT("渲染耗时选项"),
		attach,
		item
	},
	frameIntervalItem
	{
		{
			// UI_TEXT("Full (No Skip)"),
			UI_TEXT("全速 (不跳帧)"),
			attach,
			{.id = 0}
		},
		{
			// UI_TEXT("Full"),
			UI_TEXT("全速"),
			attach,
			{.id = 1}
		},
		{
			UI_TEXT("1/2"),
			attach,
			{.id = 2}
		},
		{
			UI_TEXT("1/3"),
			attach,
			{.id = 3}
		},
		{
			UI_TEXT("1/4"),
			attach,
			{.id = 4}
		},
	},
	frameInterval
	{
		// UI_TEXT("Frame Rate Target"),
		UI_TEXT("最高渲染速度"),
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
			// UI_TEXT("Auto (Match screen when rates are similar)"),
			UI_TEXT("Auto (匹配屏幕刷新率)"),
			attach,
			[this]
			{
				if(!app().viewController().emuWindowScreen()->frameRateIsReliable())
				{
					app().postErrorMessage(
						// UI_TEXT("Reported rate potentially unreliable, ")
						UI_TEXT("根据屏幕刷新率推算出的帧率不一定准确，")
						// UI_TEXT("using the detected rate may give better results"));
						UI_TEXT("使用帧率检测可以获得更准确的结果"));
				}
				onFrameTimeChange(activeVideoSystem, OutputTimingManager::autoOption);
			}, {.id = OutputTimingManager::autoOption.count()}
		},
		{
			// UI_TEXT("Original (Use emulated system's rate)"),
			UI_TEXT("Original (使用模拟系统的帧率)"),
			attach,
			[this]
			{
				onFrameTimeChange(activeVideoSystem, OutputTimingManager::originalOption);
			}, {.id = OutputTimingManager::originalOption.count()}
		},
		{
			// UI_TEXT("Detect Custom Rate"),
			UI_TEXT("帧率检测"),
			attach,
			[this](const Input::Event &e)
			{
				window().setIntendedFrameRate(system().frameRate());
				auto frView = makeView<DetectFrameRateView>();
				frView->onDetectFrameTime =
					[this](SteadyClockTime frameTime)
					{
						if(frameTime.count())
						{
							if(onFrameTimeChange(activeVideoSystem, frameTime))
								dismissPrevious();
						}
						else
						{
							// app().postErrorMessage(UI_TEXT("Detected rate too unstable to use"));
							app().postErrorMessage(UI_TEXT("帧率检测结果异常"));
						}
					};
				pushAndShowModal(std::move(frView), e);
				return false;
			}
		},
		{
			// UI_TEXT("Custom Rate"),
			UI_TEXT("自定义"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShowNewCollectValueInputView<std::pair<double, double>>(
					attachParams(), e,
					// UI_TEXT("Input decimal or fraction"),
					UI_TEXT("请输入小数或分数"),
					"",
					[this](CollectTextInputView&, auto val)
					{
						if(onFrameTimeChange(activeVideoSystem, fromSeconds<SteadyClockTime>(val.second / val.first)))
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
		// UI_TEXT("Frame Rate"),
		UI_TEXT("帧率"),
		attach,
		app().outputTimingManager.frameTimeOptionAsMenuId(VideoSystem::NATIVE_NTSC),
		frameRateItems,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
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
		// UI_TEXT("Frame Rate (PAL)"),
		UI_TEXT("帧率 (PAL)"),
		attach,
		app().outputTimingManager.frameTimeOptionAsMenuId(VideoSystem::PAL),
		frameRateItems,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
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
	frameTimeStats
	{
		// UI_TEXT("Show Frame Time Stats"),
		UI_TEXT("显示渲染耗时信息"),
		attach,
		app().showFrameTimeStats,
		[this](BoolMenuItem &item) { app().showFrameTimeStats = item.flipBoolValue(*this); }
	},
	frameClockItems
	{
		{
			// UI_TEXT("Auto"),
			UI_TEXT("自动"),
			attach,
			MenuItem::Config{.id = FrameTimeSource::Unset}
		},
		{
			// UI_TEXT("Screen (Less latency & power use)"),
			UI_TEXT("Screen (减少延迟和耗电量)"),
			attach,
			MenuItem::Config{.id = FrameTimeSource::Screen}
		},
		{
			// UI_TEXT("Timer (Best for VRR displays)"),
			UI_TEXT("Timer (最适用于支持 VRR 的显示器)"),
			attach,
			MenuItem::Config{.id = FrameTimeSource::Timer}
		},
		{
			// UI_TEXT("Renderer (May buffer multiple frames)"),
			UI_TEXT("Renderer (可能会缓冲多帧视频)"),
			attach,
			MenuItem::Config{.id = FrameTimeSource::Renderer}
		},
	},
	frameClock
	{
		// UI_TEXT("Frame Clock"),
		UI_TEXT("帧时钟"),
		attach,
		MenuId{FrameTimeSource(app().frameTimeSource)},
		frameClockItems,
		MultiChoiceMenuItem::Config
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(wise_enum::to_string(app().effectiveFrameTimeSource()));
				return true;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				app().frameTimeSource = FrameTimeSource(item.id.val);
				app().video.resetImage(); // texture can switch between single/double buffered
			}
		},
	},
	presentModeItems
	{
		{
			// UI_TEXT("Auto"),
			UI_TEXT("自动"),
			attach,
			MenuItem::Config{.id = Gfx::PresentMode::Auto}
		},
		{
			// UI_TEXT("Immediate (Less compositor latency, may drop frames)"),
			UI_TEXT("即时 (较少的延迟，但可能会掉帧)"),
			attach,
			MenuItem::Config{.id = Gfx::PresentMode::Immediate}
		},
		{
			UI_TEXT("队列 (更稳定的帧率)"),
			attach,
			MenuItem::Config{.id = Gfx::PresentMode::FIFO}
		},
	},
	presentMode
	{
		// UI_TEXT("Present Mode"),
		UI_TEXT("渲染模式"),
		attach,
		MenuId{Gfx::PresentMode(app().presentMode)},
		presentModeItems,
		MultiChoiceMenuItem::Config
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				// t.resetString(renderer().evalPresentMode(app().emuWindow(), app().presentMode) == Gfx::PresentMode::FIFO ? UI_TEXT("Queued") : UI_TEXT("Immediate"));
				t.resetString(renderer().evalPresentMode(app().emuWindow(), app().presentMode) == Gfx::PresentMode::FIFO ? UI_TEXT("队列") : UI_TEXT("即时"));
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
			auto setRateDel = [this](TextMenuItem &item) { app().overrideScreenFrameRate = std::bit_cast<FrameRate>(item.id); };
			items.emplace_back(
				// UI_TEXT("Off"),
				UI_TEXT("关"),
				attach,
				setRateDel,
				MenuItem::Config{.id = 0});
			for(auto rate : app().emuScreen().supportedFrameRates())
				items.emplace_back(std::format("{:g}Hz", rate), attach, setRateDel, MenuItem::Config{.id = std::bit_cast<MenuId>(rate)});
			return items;
		}()
	},
	screenFrameRate
	{
		// UI_TEXT("Override Screen Frame Rate"),
		UI_TEXT("优先于屏幕帧率"),
		attach,
		std::bit_cast<MenuId>(FrameRate(app().overrideScreenFrameRate)),
		screenFrameRateItems
	},
	presentationTimeItems
	{
		{
			// UI_TEXT("Full (Apply to all frame rate targets)"),
			UI_TEXT("速度优先 (采用尽可能高的渲染速度)"),
			attach,
			MenuItem::Config{.id = PresentationTimeMode::full}
		},
		{
			// UI_TEXT("Basic (Only apply to lower frame rate targets)"),
			UI_TEXT("平滑优先 (采用尽可能稳定的渲染速度)"),
			attach,
			MenuItem::Config{.id = PresentationTimeMode::basic}
		},
		{
			// UI_TEXT("Off"),
			UI_TEXT("关"),
			attach,
			MenuItem::Config{.id = PresentationTimeMode::off}
		},
	},
	presentationTime
	{
		// UI_TEXT("Precise Frame Pacing"),
		UI_TEXT("帧率调节"),
		attach,
		MenuId{PresentationTimeMode(app().presentationTimeMode)},
		presentationTimeItems,
		MultiChoiceMenuItem::Config
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(app().presentationTimeMode == PresentationTimeMode::off)
					return false;
				// t.resetString(app().presentationTimeMode == PresentationTimeMode::full ? UI_TEXT("Full") : UI_TEXT("Basic"));
				t.resetString(app().presentationTimeMode == PresentationTimeMode::full ? UI_TEXT("速度优先") : UI_TEXT("平滑优先"));
				return true;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				app().presentationTimeMode = PresentationTimeMode(item.id.val);
			}
		},
	},
	blankFrameInsertion
	{
		// UI_TEXT("Allow Blank Frame Insertion"),
		UI_TEXT("允许插入空白帧"),
		attach,
		app().allowBlankFrameInsertion,
		[this](BoolMenuItem &item) { app().allowBlankFrameInsertion = item.flipBoolValue(*this); }
	},
	advancedHeading
	{
		// UI_TEXT("Advanced"),
		UI_TEXT("高级选项："),
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
	if(used(frameTimeStats))
		item.emplace_back(&frameTimeStats);
	item.emplace_back(&advancedHeading);
	item.emplace_back(&frameClock);
	if(used(presentMode))
		item.emplace_back(&presentMode);
	if(used(presentationTime) && renderer().supportsPresentationTime())
		item.emplace_back(&presentationTime);
	item.emplace_back(&blankFrameInsertion);
	if(used(screenFrameRate) && app().emuScreen().supportedFrameRates().size() > 1)
		item.emplace_back(&screenFrameRate);
}

bool FrameTimingView::onFrameTimeChange(VideoSystem vidSys, SteadyClockTime time)
{
	if(!app().outputTimingManager.setFrameTimeOption(vidSys, time))
	{
		app().postMessage(
			4,
			true,
			// std::format(UI_TEXT("{:g}Hz not in valid range"), toHz(time)));
			std::format(UI_TEXT("{:g}Hz 不在有效的取值范围"), toHz(time)));
		return false;
	}
	return true;
}

}
