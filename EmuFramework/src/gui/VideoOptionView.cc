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
	return ctx.defaultWindowPixelFormat() == PixelFmtRGB565
		? UI_TEXT("RGB565")
		: UI_TEXT("RGBA8888");
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
				UI_TEXT("自动 (设置最佳模式)"),
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
		UI_TEXT("GPU 复制模式"),
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
					UI_TEXT("方形像素"),
					attach,
					[this]()
					{
						app().setVideoAspectRatio(-1);
					},
					MenuItem::Config{.id = std::bit_cast<MenuId>(-1.f)});
			}
			aspectRatioItem.emplace_back(
				UI_TEXT("充满屏幕"),
				attach,
				[this]()
				{
					app().setVideoAspectRatio(0);
				},
				MenuItem::Config{.id = 0});
			aspectRatioItem.emplace_back(
				UI_TEXT("自定义"),
				attach,
				[this](const Input::Event &e)
				{
					pushAndShowNewCollectValueInputView<std::pair<float, float>>(attachParams(), e,
						UI_TEXT("请输入小数或分数"),
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
								app().postErrorMessage(
									UI_TEXT("输入值不在有效的取值范围")
								);
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
		UI_TEXT("宽高比例"),
		attach,
		std::bit_cast<MenuId>(app().videoAspectRatio()),
		aspectRatioItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(idx == aspectRatioItem.size() - 1)
				{
					t.resetString(std::format(
						UI_TEXT("{:g}"),
						app().videoAspectRatio()));
					return true;
				}
				return false;
			}
		},
	},
	contentScaleItems
	{
		{
			UI_TEXT("100%"),
			attach, {.id = 100}
		},
		{
			UI_TEXT("90%"),
			attach, {.id = 90}
		},
		{
			UI_TEXT("80%"),
			attach, {.id = 80}
		},
		{
			UI_TEXT("自动"),
			attach, {.id = optionContentScaleIntegerOnly}
		},
		{
			UI_TEXT("自动 (高度优先)"),
			attach, {.id = optionContentScaleIntegerOnlyY}
		},
		{
			UI_TEXT("自定义"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShowNewCollectValueRangeInputView<int, 10, 200>(attachParams(), e,
					UI_TEXT("请输入 10 到 200 之间的值"),
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
		UI_TEXT("画面缩放"),
		attach,
		MenuId{videoLayer_.scale},
		contentScaleItems,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				if(videoLayer.scale <= 200)
				{
					t.resetString(std::format(
						UI_TEXT("{}%"),
						videoLayer.scale.value()));
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
			UI_TEXT("100%"),
			attach, {.id = 100}
		},
		{
			UI_TEXT("95%"),
			attach, {.id = 95}
		},
		{
			UI_TEXT("90%"),
			attach, {.id = 90}
		},
		{
			UI_TEXT("自定义"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShowNewCollectValueRangeInputView<int, 50, 100>(attachParams(), e,
					UI_TEXT("请输入 50 到 100 之间的值"),
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
		UI_TEXT("应用程序缩放"),
		attach,
		MenuId{app().menuScale},
		menuScaleItems,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format(
					UI_TEXT("{}%"),
					app().menuScale.value()));
				return true;
			},
			.defaultItemOnSelect = [this](TextMenuItem &item) { app().setMenuScale(item.id); }
		},
	},
	contentRotationItem
	{
		{
			UI_TEXT("自动"),
			attach, {.id = Rotation::ANY}
		},
		{
			UI_TEXT("标准"),
			attach, {.id = Rotation::UP}
		},
		{
			UI_TEXT("右转 90 度"),
			attach, {.id = Rotation::RIGHT}
		},
		{
			UI_TEXT("旋转 180 度"),
			attach, {.id = Rotation::DOWN}
		},
		{
			UI_TEXT("左转 90 度"),
			attach, {.id = Rotation::LEFT}
		},
	},
	contentRotation
	{
		UI_TEXT("画面旋转"),
		attach,
		MenuId{app().contentRotation.value()},
		contentRotationItem,
		{
			.defaultItemOnSelect = [this](TextMenuItem &item) { app().setContentRotation(Rotation(item.id.val)); }
		},
	},
	placeVideo
	{
		UI_TEXT("设置画面位置"),
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
		UI_TEXT("图像插值"),
		attach,
		videoLayer_.usingLinearFilter(),
		UI_TEXT("无"),
		UI_TEXT("线性"),
		[this](BoolMenuItem &item)
		{
			videoLayer.setLinearFilter(item.flipBoolValue(*this));
			app().viewController().postDrawToEmuWindows();
		}
	},
	imgEffectItem
	{
		{
			UI_TEXT("关"),
			attach, {.id = ImageEffectId::DIRECT}
		},
		{
			UI_TEXT("hq2x"),
			attach, {.id = ImageEffectId::HQ2X}
		},
		{
			UI_TEXT("Scale2x"),
			attach, {.id = ImageEffectId::SCALE2X}
		},
		{
			UI_TEXT("Prescale 2x"),
			attach, {.id = ImageEffectId::PRESCALE2X}
		},
		{
			UI_TEXT("Prescale 3x"),
			attach, {.id = ImageEffectId::PRESCALE3X}
		},
		{
			UI_TEXT("Prescale 4x"),
			attach, {.id = ImageEffectId::PRESCALE4X}
		},
	},
	imgEffect
	{
		UI_TEXT("图像特效"),
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
			UI_TEXT("关"),
			attach, {.id = 0}
		},
		{
			UI_TEXT("扫描线特效"),
			attach, {.id = ImageOverlayId::SCANLINES}
		},
		{
			UI_TEXT("扫描线特效 2x"),
			attach, {.id = ImageOverlayId::SCANLINES_2}
		},
		{
			UI_TEXT("LCD 屏幕特效"),
			attach, {.id = ImageOverlayId::LCD}
		},
		{
			UI_TEXT("CRT 屏幕遮罩"),
			attach, {.id = ImageOverlayId::CRT_MASK}
		},
		{
			UI_TEXT("CRT 屏幕遮罩 .5x"),
			attach, {.id = ImageOverlayId::CRT_MASK_2}
		},
		{
			UI_TEXT("CRT 屏幕格栅"),
			attach, {.id = ImageOverlayId::CRT_GRILLE}
		},
		{
			UI_TEXT("CRT 屏幕格栅 .5x"),
			attach, {.id = ImageOverlayId::CRT_GRILLE_2}
		}
	},
	overlayEffect
	{
		UI_TEXT("屏幕特效"),
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
		{
			UI_TEXT("100%"),
			attach, {.id = 100}
		},
		{
			UI_TEXT("75%"),
			attach, {.id = 75}
		},
		{
			UI_TEXT("50%"),
			attach, {.id = 50}
		},
		{
			UI_TEXT("25%"),
			attach, {.id = 25}
		},
		{
			UI_TEXT("自定义"),
			attach,
			[this](const Input::Event &e)
			{
				pushAndShowNewCollectValueRangeInputView<int, 0, 100>(attachParams(), e,
					UI_TEXT("请输入 0 到 100 之间的值"),
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
		UI_TEXT("屏幕特效等级"),
		attach,
		MenuId{videoLayer_.overlayIntensity() * 100.f},
		overlayEffectLevelItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format(
					UI_TEXT("{}%"),
					int(videoLayer.overlayIntensity() * 100.f)));
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
			UI_TEXT("自动 (匹配显示格式)"),
			attach, {.id = PixelFormatId::Unset}
		},
		{
			UI_TEXT("RGBA8888"),
			attach, {.id = PixelFormatId::RGBA8888}
		},
		{
			UI_TEXT("RGB565"),
			attach, {.id = PixelFormatId::RGB565}
		},
	},
	imgEffectPixelFormat
	{
		UI_TEXT("效果颜色格式"),
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
					app().postMessage(
						UI_TEXT("请重启应用程序使选项生效")
					);
					return;
				}
				renderPixelFormat.updateDisplayString();
				imgEffectPixelFormat.updateDisplayString();
			};
			items.emplace_back(
				UI_TEXT("自动"),
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
		UI_TEXT("显示颜色格式"),
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
		UI_TEXT("第二屏幕 (仅供测试)"),
		attach,
		false,
		[this](BoolMenuItem &item)
		{
			app().setEmuViewOnExtraWindow(item.flipBoolValue(*this), appContext().mainScreen());
		}
	},
	showOnSecondScreen
	{
		UI_TEXT("扩展屏幕"),
		attach,
		app().showOnSecondScreen,
		UI_TEXT("系统管理"),
		UI_TEXT("游戏画面"),
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
			UI_TEXT("自动 (匹配显示格式)"),
			attach, {.id = PixelFormatId::Unset}
		},
		{
			UI_TEXT("RGBA8888"),
			attach, {.id = PixelFormatId::RGBA8888}
		},
		{
			UI_TEXT("RGB565"),
			attach, {.id = PixelFormatId::RGB565}
		},
	},
	renderPixelFormat
	{
		UI_TEXT("渲染颜色格式"),
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
			UI_TEXT("默认值"),
			attach,
			[this](View &v)
			{
				videoLayer.setBrightness(1.f, ImageChannel::All);
				setAllColorLevelsSelected(MenuId{100});
				v.dismiss();
			}
		},
		{
			UI_TEXT("自定义"),
			attach,
			setVideoBrightnessCustomDel(ImageChannel::All)
		},
	},
	redItem
	{
		{
			UI_TEXT("默认值"),
			attach,
			[this](){ videoLayer.setBrightness(1.f, ImageChannel::Red); },
			{.id = 100}
		},
		{
			UI_TEXT("自定义"),
			attach,
			setVideoBrightnessCustomDel(ImageChannel::Red),
			{.id = defaultMenuId}
		},
	},
	greenItem
	{
		{
			UI_TEXT("默认值"),
			attach,
			[this](){ videoLayer.setBrightness(1.f, ImageChannel::Green); },
			{.id = 100}
		},
		{
			UI_TEXT("自定义"),
			attach,
			setVideoBrightnessCustomDel(ImageChannel::Green),
			{.id = defaultMenuId}
		},
	},
	blueItem
	{
		{
			UI_TEXT("默认值"),
			attach,
			[this](){ videoLayer.setBrightness(1.f, ImageChannel::Blue); },
			{.id = 100}
		},
		{
			UI_TEXT("自定义"),
			attach,
			setVideoBrightnessCustomDel(ImageChannel::Blue),
			{.id = defaultMenuId}
		},
	},
	brightness
	{
		UI_TEXT("设置所有色阶"),
		attach,
		[this](const Input::Event &e)
		{
			pushAndShow(makeViewWithName<TableView>(
				UI_TEXT("所有色阶"),
				brightnessItem), e);
		}
	},
	red
	{
		UI_TEXT("红色"),
		attach,
		MenuId{videoLayer_.channelBrightnessAsInt(ImageChannel::Red)},
		redItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format(
					UI_TEXT("{}%"),
					videoLayer.channelBrightnessAsInt(ImageChannel::Red)));
				return true;
			}
		},
	},
	green
	{
		UI_TEXT("绿色"),
		attach,
		MenuId{videoLayer_.channelBrightnessAsInt(ImageChannel::Green)},
		greenItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format(
					UI_TEXT("{}%"),
					videoLayer.channelBrightnessAsInt(ImageChannel::Green)));
				return true;
			}
		},
	},
	blue
	{
		UI_TEXT("蓝色"),
		attach,
		MenuId{videoLayer_.channelBrightnessAsInt(ImageChannel::Blue)},
		blueItem,
		{
			.onSetDisplayString = [this](auto idx, Gfx::Text &t)
			{
				t.resetString(std::format(
					UI_TEXT("{}%"),
					videoLayer.channelBrightnessAsInt(ImageChannel::Blue)));
				return true;
			}
		},
	},
	colorLevelsHeading
	{
		UI_TEXT("色阶："),
		attach
	},
	advancedHeading
	{
		UI_TEXT("高级："),
		attach
	},
	systemSpecificHeading
	{
		UI_TEXT("专业："),
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
		pushAndShowNewCollectValueRangeInputView<int, 0, 200>(attachParams(), e,
			UI_TEXT("请输入 0 到 200 之间的值"),
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
