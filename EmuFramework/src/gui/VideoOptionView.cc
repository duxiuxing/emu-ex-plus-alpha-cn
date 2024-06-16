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

#include <emuframework/VideoOptionView.hh>
#include <emuframework/EmuApp.hh>
#include <emuframework/EmuVideoLayer.hh>
#include <emuframework/EmuVideo.hh>
#include <emuframework/VideoImageEffect.hh>
#include <emuframework/EmuViewController.hh>
#include <emuframework/viewUtils.hh>
#include "PlaceVideoView.hh"
#include <imagine/base/ApplicationContext.hh>
#include <format>
#include <imagine/logger/logger.h>

namespace EmuEx
{

constexpr SystemLogger log{"VideoOptionView"};

static const char *autoWindowPixelFormatStr(IG::ApplicationContext ctx)
{
	return ctx.defaultWindowPixelFormat() == PixelFmtRGB565 ? UI_TEXT("RGB565") : UI_TEXT("RGBA8888");
}

constexpr uint16_t pack(Gfx::DrawableConfig c)
{
	return to_underlying(c.pixelFormat.id) | to_underlying(c.colorSpace) << sizeof(c.colorSpace) * 8;
}

constexpr Gfx::DrawableConfig unpackDrawableConfig(uint16_t c)
{
	return {PixelFormatId(c & 0xFF), Gfx::ColorSpace(c >> sizeof(Gfx::DrawableConfig::colorSpace) * 8)};
}

VideoOptionView::VideoOptionView(ViewAttachParams attach, EmuVideoLayer &videoLayer_, bool customMenu):
	TableView
	{
		// UI_TEXT("Video Options"),
		UI_TEXT("视频选项"),
		attach,
		item
	},
	videoLayer{videoLayer_},
	textureBufferModeItem
	{
		[&]
		{
			decltype(textureBufferModeItem) items;
			items.emplace_back(
				UI_TEXT("Auto (Set optimal mode)"),
				attach,
				[this](View &view)
				{
					app().textureBufferMode = Gfx::TextureBufferMode::DEFAULT;
					auto defaultMode = renderer().evalTextureBufferMode();
					emuVideo().setTextureBufferMode(system(), defaultMode);
					textureBufferMode.setSelected(MenuId{defaultMode});
					view.dismiss();
					return false;
				},
				MenuItem::Config{.id = 0});
			for(auto desc: renderer().textureBufferModes())
			{
				items.emplace_back(desc.name, attach, [this](MenuItem &item)
				{
					app().textureBufferMode = Gfx::TextureBufferMode(item.id.val);
					emuVideo().setTextureBufferMode(system(), Gfx::TextureBufferMode(item.id.val));
				}, MenuItem::Config{.id = desc.mode});
			}
			return items;
		}()
	},
	textureBufferMode
	{
		UI_TEXT("GPU Copy Mode"),
		attach,
		MenuId{renderer().evalTextureBufferMode(app().textureBufferMode)},
		textureBufferModeItem
	},
	aspectRatioItem
	{
		[&]()
		{
			StaticArrayList<TextMenuItem, MAX_ASPECT_RATIO_ITEMS> aspectRatioItem;
			for(const auto &i : EmuSystem::aspectRatioInfos())
			{
				aspectRatioItem.emplace_back(i.name, attach, [this](TextMenuItem &item)
				{
					app().setVideoAspectRatio(std::bit_cast<float>(item.id));
				}, MenuItem::Config{.id = std::bit_cast<MenuId>(i.aspect.ratio<float>())});
			}
			if(EmuSystem::hasRectangularPixels)
			{
				aspectRatioItem.emplace_back(
					UI_TEXT("Square Pixels"),
					attach,
					[this]()
					{
						app().setVideoAspectRatio(-1);
					},
					MenuItem::Config{.id = std::bit_cast<MenuId>(-1.f)});
			}
			aspectRatioItem.emplace_back(
				UI_TEXT("Fill Display"),
				attach,
				[this]()
				{
					app().setVideoAspectRatio(0);
				},
				MenuItem::Config{.id = 0});
			aspectRatioItem.emplace_back(
				UI_TEXT("Custom Value"),
				attach,
				[this](const Input::Event &e)
				{
					pushAndShowNewCollectValueInputView<std::pair<float, float>>(
						attachParams(), e,
						UI_TEXT("Input decimal or fraction"),
						"",
						[this](CollectTextInputView &, auto val)
						{
							float ratio = val.first / val.second;
							if(app().setVideoAspectRatio(ratio))
							{
								aspectRatio.setSelected(std::bit_cast<MenuId>(ratio), *this);
								dismissPrevious();
								return true;
							}
							else
							{
								app().postErrorMessage(UI_TEXT("Value not in range"));
								return false;
							}
						});
					return false;
				},
				MenuItem::Config{.id = defaultMenuId});
			return aspectRatioItem;
		}()
	},
	aspectRatio
	{
		UI_TEXT("Aspect Ratio"),
		attach,
		std::bit_cast<MenuId>(app().videoAspectRatio()),
		aspectRatioItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(idx == aspectRatioItem.size() - 1)
				{
					t.resetString(std::format("{:g}", app().videoAspectRatio()));
					return true;
				}
				return false;
			}
		},
	},
	contentScaleItems
	{
		{
			"100%",
			attach,
			{.id = 100}
		},
		{
			"90%",
			attach,
			{.id = 90}
		},
		{
			"80%",
			attach,
			{.id = 80}
		},
		{
			UI_TEXT("Integer-only"),
			attach,
			{.id = optionContentScaleIntegerOnly}
		},
		{
			UI_TEXT("Integer-only (Height)"),
			attach,
			{.id = optionContentScaleIntegerOnlyY}
		},
		{
			UI_TEXT("Custom Value"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShowNewCollectValueRangeInputView<int, 10, 200>(
					attachParams(), e,
					UI_TEXT("Input 10 to 200"),
					"",
					[this](CollectTextInputView &, auto val)
					{
						app().setContentScale(val);
						contentScale.setSelected(MenuId{val}, *this);
						dismissPrevious();
						return true;
					});
				return false;
			},
			{.id = defaultMenuId}
		},
	},
	contentScale
	{
		UI_TEXT("Content Scale"),
		attach,
		MenuId{videoLayer_.scale},
		contentScaleItems,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(videoLayer.scale <= 200)
				{
					t.resetString(std::format("{}%", videoLayer.scale.value()));
					return true;
				}
				return false;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item) { app().setContentScale(item.id); }
		},
	},
	menuScaleItems
	{
		{
			"100%",
			attach,
			{.id = 100}
		},
		{
			"95%",
			attach,
			{.id = 95}
		},
		{
			"90%",
			attach,
			{.id = 90}
		},
		{
			UI_TEXT("Custom Value"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShowNewCollectValueRangeInputView<int, 50, 100>(
					attachParams(), e,
					UI_TEXT("Input 50 to 100"),
					"",
					[this](CollectTextInputView &, auto val)
					{
						app().setMenuScale(val);
						menuScale.setSelected(MenuId{val}, *this);
						dismissPrevious();
						return true;
					});
				return false;
			},
			{.id = defaultMenuId}
		},
	},
	menuScale
	{
		UI_TEXT("Menu Scale"),
		attach,
		MenuId{app().menuScale},
		menuScaleItems,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format("{}%", app().menuScale.value()));
				return true;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item) { app().setMenuScale(item.id); }
		},
	},
	contentRotationItem
	{
		{
			UI_TEXT("Auto"),
			attach,
			{.id = Rotation::ANY}
		},
		{
			UI_TEXT("Standard"),
			attach,
			{.id = Rotation::UP}
		},
		{
			UI_TEXT("90° Right"),
			attach,
			{.id = Rotation::RIGHT}
		},
		{
			UI_TEXT("Upside Down"),
			attach,
			{.id = Rotation::DOWN}
		},
		{
			UI_TEXT("90° Left"),
			attach,
			{.id = Rotation::LEFT}
		},
	},
	contentRotation
	{
		UI_TEXT("Content Rotation"),
		attach,
		MenuId{app().contentRotation.value()},
		contentRotationItem,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item) { app().setContentRotation(Rotation(item.id.val)); }
		},
	},
	placeVideo
	{
		UI_TEXT("Set Video Position"),
		attach,
		[this](const Input::Event &e)
		{
			if(!system().hasContent())
				return;
			pushAndShowModal(makeView<PlaceVideoView>(videoLayer, app().defaultVController()), e);
		}
	},
	imgFilter
	{
		UI_TEXT("Image Interpolation"),
		attach,
		videoLayer_.usingLinearFilter(),
		UI_TEXT("None"),
		UI_TEXT("Linear"),
		[this](BoolMenuItem &item)
		{
			videoLayer.setLinearFilter(item.flipBoolValue(*this));
			app().viewController().postDrawToEmuWindows();
		}
	},
	imgEffectItem
	{
		{
			UI_TEXT("Off"),
			attach,
			{.id = ImageEffectId::DIRECT}
		},
		{
			UI_TEXT("hq2x"),
			attach,
			{.id = ImageEffectId::HQ2X}
		},
		{
			UI_TEXT("Scale2x"),
			attach,
			{.id = ImageEffectId::SCALE2X}
		},
		{
			UI_TEXT("Prescale 2x"),
			attach,
			{.id = ImageEffectId::PRESCALE2X}
		},
		{
			UI_TEXT("Prescale 3x"),
			attach,
			{.id = ImageEffectId::PRESCALE3X}
		},
		{
			UI_TEXT("Prescale 4x"),
			attach,
			{.id = ImageEffectId::PRESCALE4X}
		},
	},
	imgEffect
	{
		UI_TEXT("Image Effect"),
		attach,
		MenuId{videoLayer_.effectId()},
		imgEffectItem,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				videoLayer.setEffect(system(), ImageEffectId(item.id.val), app().videoEffectPixelFormat());
				app().viewController().postDrawToEmuWindows();
			}
		},
	},
	overlayEffectItem
	{
		{
			UI_TEXT("Off"),
			attach,
			{.id = 0}
		},
		{
			UI_TEXT("Scanlines"),
			attach,
			{.id = ImageOverlayId::SCANLINES}
		},
		{
			UI_TEXT("Scanlines 2x"),
			attach,
			{.id = ImageOverlayId::SCANLINES_2}
		},
		{
			UI_TEXT("LCD Grid"),
			attach,
			{.id = ImageOverlayId::LCD}
		},
		{
			UI_TEXT("CRT Mask"),
			attach,
			{.id = ImageOverlayId::CRT_MASK}
		},
		{
			UI_TEXT("CRT Mask .5x"),
			attach,
			{.id = ImageOverlayId::CRT_MASK_2}
		},
		{
			UI_TEXT("CRT Grille"),
			attach,
			{.id = ImageOverlayId::CRT_GRILLE}
		},
		{
			UI_TEXT("CRT Grille .5x"),
			attach,
			{.id = ImageOverlayId::CRT_GRILLE_2}
		}
	},
	overlayEffect
	{
		UI_TEXT("Overlay Effect"),
		attach,
		MenuId{videoLayer_.overlayEffectId()},
		overlayEffectItem,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				videoLayer.setOverlay(ImageOverlayId(item.id.val));
				app().viewController().postDrawToEmuWindows();
			}
		},
	},
	overlayEffectLevelItem
	{
		{"100%", attach, {.id = 100}},
		{"75%",  attach, {.id = 75}},
		{"50%",  attach, {.id = 50}},
		{"25%",  attach, {.id = 25}},
		{
			UI_TEXT("Custom Value"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShowNewCollectValueRangeInputView<int, 0, 100>(
					attachParams(), e,
					UI_TEXT("Input 0 to 100"),
					"",
					[this](CollectTextInputView &, auto val)
					{
						videoLayer.setOverlayIntensity(val / 100.f);
						app().viewController().postDrawToEmuWindows();
						overlayEffectLevel.setSelected(MenuId{val}, *this);
						dismissPrevious();
						return true;
					});
				return false;
			}, {.id = defaultMenuId}
		},
	},
	overlayEffectLevel
	{
		UI_TEXT("Overlay Effect Level"),
		attach,
		MenuId{videoLayer_.overlayIntensity() * 100.f},
		overlayEffectLevelItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format("{}%", int(videoLayer.overlayIntensity() * 100.f)));
				return true;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				videoLayer.setOverlayIntensity(item.id / 100.f);
				app().viewController().postDrawToEmuWindows();
			}
		},
	},
	imgEffectPixelFormatItem
	{
		{
			UI_TEXT("Auto (Match display format)"),
			attach,
			{.id = PixelFormatId::Unset}
		},
		{
			UI_TEXT("RGBA8888"),
			attach,
			{.id = PixelFormatId::RGBA8888}
		},
		{
			UI_TEXT("RGB565"),
			attach,
			{.id = PixelFormatId::RGB565}
		},
	},
	imgEffectPixelFormat
	{
		UI_TEXT("Effect Color Format"),
		attach,
		MenuId{app().imageEffectPixelFormat.value()},
		imgEffectPixelFormatItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(idx == 0)
				{
					t.resetString(app().videoEffectPixelFormat().name());
					return true;
				}
				else
					return false;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item)
			{
				app().imageEffectPixelFormat = PixelFormatId(item.id.val);
				videoLayer.setEffectFormat(app().videoEffectPixelFormat());
				app().viewController().postDrawToEmuWindows();
			}
		},
	},
	windowPixelFormatItem
	{
		[&]
		{
			decltype(windowPixelFormatItem) items;
			auto setWindowDrawableConfigDel = [this](TextMenuItem &item)
			{
				auto conf = unpackDrawableConfig(item.id);
				if(!app().setWindowDrawableConfig(conf))
				{
					app().postMessage(UI_TEXT("Restart app for option to take effect"));
					return;
				}
				renderPixelFormat.updateDisplayString();
				imgEffectPixelFormat.updateDisplayString();
			};
			items.emplace_back(
				UI_TEXT("Auto"),
				attach,
				setWindowDrawableConfigDel,
				MenuItem::Config{.id = 0});
			for(auto desc: renderer().supportedDrawableConfigs())
			{
				items.emplace_back(desc.name, attach, setWindowDrawableConfigDel, MenuItem::Config{.id = pack(desc.config)});
			}
			return items;
		}()
	},
	windowPixelFormat
	{
		UI_TEXT("Display Color Format"),
		attach,
		MenuId{pack(app().windowDrawableConfig())},
		windowPixelFormatItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(idx == 0)
				{
					t.resetString(autoWindowPixelFormatStr(appContext()));
					return true;
				}
				else
					return false;
			}
		},
	},
	secondDisplay
	{
		UI_TEXT("2nd Window (for testing only)"),
		attach,
		false,
		[this](BoolMenuItem &item)
		{
			app().setEmuViewOnExtraWindow(item.flipBoolValue(*this), appContext().mainScreen());
		}
	},
	showOnSecondScreen
	{
		UI_TEXT("External Screen"),
		attach,
		app().showOnSecondScreen,
		UI_TEXT("OS Managed"),
		UI_TEXT("Emu Content"),
		[this](BoolMenuItem &item)
		{
			app().showOnSecondScreen = item.flipBoolValue(*this);
			if(appContext().screens().size() > 1)
				app().setEmuViewOnExtraWindow(app().showOnSecondScreen, *appContext().screens()[1]);
		}
	},
	renderPixelFormatItem
	{
		{
			UI_TEXT("Auto (Match display format)"),
			attach,
			{.id = PixelFormatId::Unset}
		},
		{"RGBA8888",	attach, {.id = PixelFormatId::RGBA8888}},
		{"RGB565",		attach, {.id = PixelFormatId::RGB565}},
	},
	renderPixelFormat
	{
		UI_TEXT("Render Color Format"),
		attach,
		MenuId{app().renderPixelFormat.value().id},
		renderPixelFormatItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(idx == 0)
				{
					t.resetString(emuVideo().internalRenderPixelFormat().name());
					return true;
				}
				return false;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item) { app().setRenderPixelFormat(PixelFormatId(item.id.val)); }
		},
	},
	brightnessItem
	{
		{
			UI_TEXT("Default"),
			attach,
			[this](View &v)
			{
				videoLayer.setBrightness(1.f, ImageChannel::All);
				setAllColorLevelsSelected(MenuId{100});
				v.dismiss();
			}
		},
		{
			UI_TEXT("Custom Value"),
			attach,
			setVideoBrightnessCustomDel(ImageChannel::All)
		},
	},
	redItem
	{
		{
			UI_TEXT("Default"),
			attach,
			[this](){ videoLayer.setBrightness(1.f, ImageChannel::Red); },
			{.id = 100}
		},
		{
			UI_TEXT("Custom Value"),
			attach,
			setVideoBrightnessCustomDel(ImageChannel::Red),
			{.id = defaultMenuId}
		},
	},
	greenItem
	{
		{
			UI_TEXT("Default"),
			attach,
			[this](){ videoLayer.setBrightness(1.f, ImageChannel::Green); },
			{.id = 100}
		},
		{
			UI_TEXT("Custom Value"),
			attach,
			setVideoBrightnessCustomDel(ImageChannel::Green),
			{.id = defaultMenuId}
		},
	},
	blueItem
	{
		{
			UI_TEXT("Default"),
			attach,
			[this](){ videoLayer.setBrightness(1.f, ImageChannel::Blue); },
			{.id = 100}
		},
		{
			UI_TEXT("Custom Value"),
			attach,
			setVideoBrightnessCustomDel(ImageChannel::Blue),
			{.id = defaultMenuId}
		},
	},
	brightness
	{
		UI_TEXT("Set All Levels"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShow(
				makeViewWithName<TableView>(
					UI_TEXT("All Levels"),
					brightnessItem),
				e);
		}
	},
	red
	{
		UI_TEXT("Red"),
		attach,
		MenuId{videoLayer_.channelBrightnessAsInt(ImageChannel::Red)},
		redItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format("{}%", videoLayer.channelBrightnessAsInt(ImageChannel::Red)));
				return true;
			}
		},
	},
	green
	{
		UI_TEXT("Green"),
		attach,
		MenuId{videoLayer_.channelBrightnessAsInt(ImageChannel::Green)},
		greenItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format("{}%", videoLayer.channelBrightnessAsInt(ImageChannel::Green)));
				return true;
			}
		},
	},
	blue
	{
		UI_TEXT("Blue"),
		attach,
		MenuId{videoLayer_.channelBrightnessAsInt(ImageChannel::Blue)},
		blueItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format("{}%", videoLayer.channelBrightnessAsInt(ImageChannel::Blue)));
				return true;
			}
		},
	},
	colorLevelsHeading
	{
		UI_TEXT("Color Levels"),
		attach
	},
	advancedHeading
	{
		UI_TEXT("Advanced"),
		attach
	},
	systemSpecificHeading
	{
		UI_TEXT("System-specific"),
		attach
	}
{
	if(!customMenu)
	{
		loadStockItems();
	}
}

void VideoOptionView::place()
{
	aspectRatio.setSelected(std::bit_cast<MenuId>(app().videoAspectRatio()), *this);
	TableView::place();
}

void VideoOptionView::loadStockItems()
{
	item.emplace_back(&imgFilter);
	item.emplace_back(&imgEffect);
	item.emplace_back(&overlayEffect);
	item.emplace_back(&overlayEffectLevel);
	item.emplace_back(&contentScale);
	item.emplace_back(&menuScale);
	item.emplace_back(&aspectRatio);
	item.emplace_back(&contentRotation);
	placeVideo.setActive(system().hasContent());
	item.emplace_back(&placeVideo);
	item.emplace_back(&colorLevelsHeading);
	item.emplace_back(&brightness);
	item.emplace_back(&red);
	item.emplace_back(&green);
	item.emplace_back(&blue);
	item.emplace_back(&advancedHeading);
	item.emplace_back(&textureBufferMode);
	if(windowPixelFormatItem.size() > 2)
	{
		item.emplace_back(&windowPixelFormat);
	}
	if(EmuSystem::canRenderRGBA8888)
		item.emplace_back(&renderPixelFormat);
	item.emplace_back(&imgEffectPixelFormat);
	if(used(secondDisplay))
		item.emplace_back(&secondDisplay);
	if(used(showOnSecondScreen) && app().supportsShowOnSecondScreen(appContext()))
		item.emplace_back(&showOnSecondScreen);
}

TextMenuItem::SelectDelegate VideoOptionView::setVideoBrightnessCustomDel(ImageChannel ch)
{
	return [=, this](const Input::Event &e)
	{
		pushAndShowNewCollectValueRangeInputView<int, 0, 200>(
			attachParams(), e,
			UI_TEXT("Input 0 to 200"),
			"",
			[=, this](CollectTextInputView &, auto val)
			{
				videoLayer.setBrightness(val / 100.f, ch);
				if(ch == ImageChannel::All)
					setAllColorLevelsSelected(MenuId{val});
				else
					[&]() -> MultiChoiceMenuItem&
					{
						switch(ch)
						{
							case ImageChannel::All: break;
							case ImageChannel::Red: return red;
							case ImageChannel::Green: return green;
							case ImageChannel::Blue: return blue;
						}
						bug_unreachable("invalid ImageChannel");
					}().setSelected(MenuId{val}, *this);
				dismissPrevious();
				return true;
			});
		return false;
	};
}

void VideoOptionView::setAllColorLevelsSelected(MenuId val)
{
	red.setSelected(val, *this);
	green.setSelected(val, *this);
	blue.setSelected(val, *this);
}

EmuVideo &VideoOptionView::emuVideo() const
{
	return videoLayer.video;
}

}
