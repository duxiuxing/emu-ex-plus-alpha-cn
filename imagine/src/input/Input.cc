/*  This file is part of Imagine.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Imagine.  If not, see <http://www.gnu.org/licenses/> */

#include <imagine/input/Event.hh>
#include <imagine/input/Device.hh>
#include <imagine/base/Window.hh>
#include <imagine/base/Timer.hh>
#include <imagine/base/ApplicationContext.hh>
#include <imagine/base/Application.hh>
#include <imagine/util/ranges.hh>
#include <imagine/logger/logger.h>
#include <optional>

namespace IG::Input
{

static constexpr Key iCadeMap[12]
{
	Keycode::UP, Keycode::RIGHT, Keycode::DOWN, Keycode::LEFT,
	Keycode::GAME_X, Keycode::GAME_B,
	Keycode::GAME_A, Keycode::GAME_Y,
	Keycode::GAME_R1, Keycode::GAME_L1,
	Keycode::GAME_START, Keycode::GAME_SELECT // labeled E1 & E2 on some controllers
};

static Key keyToICadeOnKey(Key key)
{
	switch(key)
	{
		case Keycode::W : return iCadeMap[0];
		case Keycode::D : return iCadeMap[1];
		case Keycode::X : return iCadeMap[2];
		case Keycode::A : return iCadeMap[3];
		case Keycode::Y : return iCadeMap[4];
		case Keycode::H : return iCadeMap[5];
		case Keycode::U : return iCadeMap[6];
		case Keycode::J : return iCadeMap[7];
		case Keycode::I : return iCadeMap[8];
		case Keycode::K : return iCadeMap[9];
		case Keycode::O : return iCadeMap[10];
		case Keycode::L : return iCadeMap[11];
	}
	return 0;
}

static Key keyToICadeOffKey(Key key)
{
	switch(key)
	{
		case Keycode::E : return iCadeMap[0];
		case Keycode::C : return iCadeMap[1];
		case Keycode::Z : return iCadeMap[2];
		case Keycode::Q : return iCadeMap[3];
		case Keycode::T : return iCadeMap[4];
		case Keycode::R : return iCadeMap[5];
		case Keycode::F : return iCadeMap[6];
		case Keycode::N : return iCadeMap[7];
		case Keycode::M : return iCadeMap[8];
		case Keycode::P : return iCadeMap[9];
		case Keycode::G : return iCadeMap[10];
		case Keycode::V : return iCadeMap[11];
	}
	return 0;
}

std::string_view BaseEvent::mapName(Map map)
{
	switch(map)
	{
		default:
			return UI_TEXT("Unknown");
		case Map::SYSTEM:
			return UI_TEXT("Key Input");
		case Map::POINTER:
			return UI_TEXT("Pointer");
		case Map::REL_POINTER:
			return UI_TEXT("Relative Pointer");
		#ifdef CONFIG_INPUT_BLUETOOTH
		case Map::WIIMOTE:
			return UI_TEXT("Wiimote");
		case Map::WII_CC:
			return UI_TEXT("Classic / Wii U Pro Controller");
		case Map::ICONTROLPAD:
			return UI_TEXT("iControlPad");
		case Map::ZEEMOTE:
			return UI_TEXT("Zeemote JS1");
		#endif
		#ifdef CONFIG_BLUETOOTH_SERVER
		case Map::PS3PAD:
			return UI_TEXT("PS3 Gamepad");
		#endif
		#ifdef CONFIG_INPUT_APPLE_GAME_CONTROLLER
		case Map::APPLE_GAME_CONTROLLER:
			return UI_TEXT("Apple Game Controller");
		#endif
	}
}

size_t BaseEvent::mapNumKeys(Map)
{
	return Input::Keycode::COUNT;
}

const char *sourceStr(Source src)
{
	switch(src)
	{
		case Source::UNKNOWN: break;
		case Source::KEYBOARD:
			return UI_TEXT("Keyboard");
		case Source::GAMEPAD:
			return UI_TEXT("Gamepad");
		case Source::MOUSE:
			return UI_TEXT("Mouse");
		case Source::TOUCHSCREEN:
			return UI_TEXT("Touchscreen");
		case Source::NAVIGATION:
			return UI_TEXT("Navigation");
		case Source::JOYSTICK:
			return UI_TEXT("Joystick");
	}
	return UI_TEXT("Unknown");
}

const char *actionStr(Action act)
{
	switch(act)
	{
		case Action::UNUSED:
			return UI_TEXT("Unused");
		case Action::RELEASED:
			return UI_TEXT("Released");
		case Action::PUSHED:
			return UI_TEXT("Pushed");
		case Action::MOVED:
			return UI_TEXT("Moved");
		case Action::MOVED_RELATIVE:
			return UI_TEXT("Moved Relative");
		case Action::EXIT_VIEW:
			return UI_TEXT("Exit View");
		case Action::ENTER_VIEW:
			return UI_TEXT("Enter View");
		case Action::SCROLL_UP:
			return UI_TEXT("Scroll Up");
		case Action::SCROLL_DOWN:
			return UI_TEXT("Scroll Down");
		case Action::CANCELED:
			return UI_TEXT("Canceled");
	}
	return UI_TEXT("Unknown");
}

Map validateMap(uint8_t mapValue)
{
	switch(mapValue)
	{
		default: return Map::UNKNOWN;
		case (uint8_t)Map::SYSTEM:
		#ifdef CONFIG_INPUT_BLUETOOTH
		case (uint8_t)Map::WIIMOTE:
		case (uint8_t)Map::WII_CC:
		case (uint8_t)Map::ICONTROLPAD:
		case (uint8_t)Map::ZEEMOTE:
		#endif
		#ifdef CONFIG_BLUETOOTH_SERVER
		case (uint8_t)Map::PS3PAD:
		#endif
		#ifdef CONFIG_INPUT_APPLE_GAME_CONTROLLER
		case (uint8_t)Map::APPLE_GAME_CONTROLLER:
		#endif
		return Map(mapValue);
	}
}

DirectionKeys directionKeys()
{
	return {Keycode::UP, Keycode::RIGHT, Keycode::DOWN, Keycode::LEFT};
}

}

namespace IG
{

void BaseApplication::startKeyRepeatTimer(Input::KeyEvent event)
{
	if(!allowKeyRepeatTimer_)
		return;
	if(!event.pushed())
	{
		// only repeat PUSHED action, otherwise cancel the timer
		//logMsg("repeat event is not for pushed action");
		cancelKeyRepeatTimer();
		return;
	}
	//logMsg("starting key repeat");
	keyRepeatEvent = event;
	keyRepeatEvent.setRepeatCount(1);
	if(!keyRepeatTimer) [[unlikely]]
	{
		keyRepeatTimer.emplace(TimerDesc{.debugLabel = "keyRepeatTimer"},
			[this]
			{
				//logMsg("repeating key event");
				if(keyRepeatEvent.pushed()) [[likely]]
					dispatchKeyInputEvent(keyRepeatEvent);
				return true;
			});
	}
	keyRepeatTimer->run(IG::Milliseconds(400), IG::Milliseconds(50));
}

void BaseApplication::cancelKeyRepeatTimer()
{
	//logMsg("cancelled key repeat");
	if(!keyRepeatTimer)
		return;
	keyRepeatTimer->cancel();
	keyRepeatEvent = {};
}

void BaseApplication::deinitKeyRepeatTimer()
{
	keyRepeatTimer.reset();
}

void BaseApplication::setAllowKeyRepeatTimer(bool on)
{
	allowKeyRepeatTimer_ = on;
	if(!on)
	{
		deinitKeyRepeatTimer();
	}
}

const InputDeviceContainer &BaseApplication::inputDevices() const
{
	return inputDev;
}

Input::Device &BaseApplication::addInputDevice(ApplicationContext ctx, std::unique_ptr<Input::Device> ptr, bool notify)
{
	ptr->setEnumId(nextInputDeviceEnumId(ptr->name()));
	auto &devPtr = inputDev.emplace_back(std::move(ptr));
	if(notify)
	{
		onEvent(ctx, Input::DeviceChangeEvent{*devPtr, Input::DeviceChange::added});
	}
	return *devPtr;
}

void BaseApplication::removeInputDevice(ApplicationContext ctx, Input::Device &d, bool notify)
{
	removeInputDeviceIf(ctx, [&](const auto &devPtr){ return devPtr.get() == &d; }, notify);
}

void BaseApplication::removeInputDevice(ApplicationContext ctx, Input::Map map, int id, bool notify)
{
	removeInputDeviceIf(ctx, [&](const auto &devPtr){ return devPtr->map() == map && devPtr->id() == id; }, notify);
}

void BaseApplication::removeInputDevices(ApplicationContext ctx, Input::Map map, bool notify)
{
	while(auto removedDevice = moveOut(inputDev, [&](const auto &iDev){ return iDev->map() == map; }))
	{
		if(notify)
		{
			onEvent(ctx, Input::DeviceChangeEvent{*removedDevice, Input::DeviceChange::removed});
		}
	}
}

void BaseApplication::removeInputDevice(ApplicationContext ctx, InputDeviceContainer::iterator it, bool notify)
{
	if(it == inputDev.end()) [[unlikely]]
		return;
	auto removedDevPtr = std::move(*it);
	inputDev.erase(it);
	logMsg("removed input device:%s,%d", removedDevPtr->name().data(), removedDevPtr->enumId());
	cancelKeyRepeatTimer();
	if(notify)
	{
		onEvent(ctx, Input::DeviceChangeEvent{*removedDevPtr, Input::DeviceChange::removed});
	}
}

uint8_t BaseApplication::nextInputDeviceEnumId(std::string_view name) const
{
	static constexpr uint8_t maxEnum = 64;
	for(auto i : iotaCount(maxEnum))
	{
		auto it = std::ranges::find_if(inputDev,
			[&](auto &devPtr){ return devPtr->name() == name && devPtr->enumId() == i; });
		if(it == inputDev.end())
			return i;
	}
	return maxEnum;
}

bool BaseApplication::dispatchRepeatableKeyInputEvent(Input::KeyEvent e, Window &win)
{
	e.setKeyFlags(swappedConfirmKeys());
	return win.dispatchRepeatableKeyInputEvent(e);
}

bool BaseApplication::dispatchRepeatableKeyInputEvent(Input::KeyEvent e)
{
	return dispatchRepeatableKeyInputEvent(e, mainWindow());
}

bool BaseApplication::dispatchKeyInputEvent(Input::KeyEvent e, Window &win)
{
	e.setKeyFlags(swappedConfirmKeys());
	if(e.device()->iCadeMode() && processICadeKey(e, win))
		return true;
	return win.dispatchInputEvent(e);
}

bool BaseApplication::dispatchKeyInputEvent(Input::KeyEvent e)
{
	return dispatchKeyInputEvent(e, mainWindow());
}

void BaseApplication::dispatchInputDeviceChange(ApplicationContext ctx, const Input::Device &d, Input::DeviceChange change)
{
	onEvent(ctx, Input::DeviceChangeEvent{d, change});
}

std::optional<bool> BaseApplication::swappedConfirmKeysOption() const
{
	if(swappedConfirmKeys() == Input::SWAPPED_CONFIRM_KEYS_DEFAULT)
		return {};
	return swappedConfirmKeys();
}

bool BaseApplication::swappedConfirmKeys() const
{
	return swappedConfirmKeys_;
}

void BaseApplication::setSwappedConfirmKeys(std::optional<bool> opt)
{
	if(!opt)
		return;
	swappedConfirmKeys_ = *opt;
}

uint8_t BaseApplication::keyEventFlags() const
{
	return swappedConfirmKeys();
}

[[gnu::weak]] void ApplicationContext::enumInputDevices() const {}

bool ApplicationContext::keyInputIsPresent() const
{
	return Input::Device::anyTypeFlagsPresent(*this, {.keyboard = true, .gamepad = true});
}

void ApplicationContext::flushInputEvents()
{
	flushSystemInputEvents();
	flushInternalInputEvents();
}

void ApplicationContext::flushInternalInputEvents()
{
	// TODO
}

[[gnu::weak]] bool ApplicationContext::hasInputDeviceHotSwap() const { return Config::Input::DEVICE_HOTSWAP; }

[[gnu::weak]] void ApplicationContext::flushSystemInputEvents() {}

bool BaseApplication::processICadeKey(const Input::KeyEvent &e, Window &win)
{
	using namespace IG::Input;
	if(auto onKey = keyToICadeOnKey(e.key()))
	{
		if(e.state() == Action::PUSHED)
		{
			//logMsg("pushed iCade keyboard key: %s", dev.keyName(key));
			dispatchRepeatableKeyInputEvent({Map::SYSTEM, onKey, Action::PUSHED, 0, 0, Source::GAMEPAD, e.time(), e.device()}, win);
		}
		return true;
	}
	if(auto offKey = keyToICadeOffKey(e.key()))
	{
		if(e.state() == Action::PUSHED)
		{
			dispatchRepeatableKeyInputEvent({Map::SYSTEM, offKey, Action::RELEASED, 0, 0, Source::GAMEPAD, e.time(), e.device()}, win);
		}
		return true;
	}
	return false; // not an iCade key
}

}
