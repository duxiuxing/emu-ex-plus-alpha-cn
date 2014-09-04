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

#define LOGTAG "AndroidWindow"
#include "../common/windowPrivate.hh"
#include "../common/screenPrivate.hh"
#include <imagine/base/GLContext.hh>
#include <imagine/base/android/sdk.hh>
#include "android.hh"
#include "ASurface.hh"
#include <imagine/util/fd-utils.h>
#include <android/native_activity.h>
#include <android/native_window_jni.h>
#include <android/looper.h>
#include "internal.hh"

namespace Base
{

extern JavaInstMethod<jobject> jPresentation;
static JavaInstMethod<void> jPresentationShow;
static JavaInstMethod<void> jPresentationDeinit;

Window *deviceWindow()
{
	return Window::window(0);
}

static int winFormatFromEGLConfig(EGLDisplay display, EGLConfig config)
{
	EGLint nId;
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &nId);
	if(!nId)
	{
		nId = WINDOW_FORMAT_RGBA_8888;
		EGLint alphaSize;
		eglGetConfigAttrib(display, config, EGL_ALPHA_SIZE, &alphaSize);
		if(!alphaSize)
			nId = WINDOW_FORMAT_RGBX_8888;
		EGLint redSize;
		eglGetConfigAttrib(display, config, EGL_RED_SIZE, &redSize);
		if(redSize < 8)
			nId = WINDOW_FORMAT_RGB_565;
		//logWarn("config didn't provide a native format id, guessing %d", nId);
	}
	return nId;
}

static void initPresentationJNI(JNIEnv* env, jobject presentation)
{
	if(jPresentationDeinit)
		return; // already init
	logMsg("Setting up Presentation JNI functions");
	auto cls = env->GetObjectClass(presentation);
	jPresentationShow.setup(env, cls, "show", "()V");
	jPresentationDeinit.setup(env, cls, "deinit", "()V");
	JNINativeMethod method[] =
	{
		{
			"onSurfaceCreated", "(JLandroid/view/Surface;)V",
			(void*)(void JNICALL(*)(JNIEnv* env, jobject thiz, jlong windowAddr, jobject surface))
			([](JNIEnv* env, jobject thiz, jlong windowAddr, jobject surface)
			{
				auto nWin = ANativeWindow_fromSurface(env, surface);
				auto &win = *((Window*)windowAddr);
				androidWindowInitSurface(win, nWin);
			})
		},
		{
			"onSurfaceRedrawNeeded", "(J)V",
			(void*)(void JNICALL(*)(JNIEnv* env, jobject thiz, jlong windowAddr))
			([](JNIEnv* env, jobject thiz, jlong windowAddr)
			{
				auto &win = *((Window*)windowAddr);
				androidWindowNeedsRedraw(win);
			})
		},
		{
			"onSurfaceDestroyed", "(J)V",
			(void*)(void JNICALL(*)(JNIEnv* env, jobject thiz, jlong windowAddr))
			([](JNIEnv* env, jobject thiz, jlong windowAddr)
			{
				auto &win = *((Window*)windowAddr);
				ANativeWindow_release(win.nWin);
				androidWindowSurfaceDestroyed(win);
			})
		},
	};
	env->RegisterNatives(cls, method, sizeofArray(method));
}

IG::Point2D<float> Window::pixelSizeAsMM(IG::Point2D<int> size)
{
	assert(screen().xDPI && screen().yDPI);
	float xdpi = ASurface::isStraightOrientation(osOrientation) ? screen().xDPI : screen().yDPI;
	float ydpi = ASurface::isStraightOrientation(osOrientation) ? screen().yDPI : screen().xDPI;
	return {((float)size.x / xdpi) * 25.4f, ((float)size.y / ydpi) * 25.4f};
}

IG::Point2D<float> Window::pixelSizeAsSMM(IG::Point2D<int> size)
{
	assert(screen().densityDPI);
	return {((float)size.x / screen().densityDPI) * 25.4f, ((float)size.y / screen().densityDPI) * 25.4f};
}

bool Window::setValidOrientations(uint oMask)
{
	using namespace Base;
	logMsg("requested orientation change to %s", Base::orientationToStr(oMask));
	int toSet = -1;
	switch(oMask)
	{
		bdefault: toSet = -1; // SCREEN_ORIENTATION_UNSPECIFIED
		bcase VIEW_ROTATE_0: toSet = 1; // SCREEN_ORIENTATION_PORTRAIT
		bcase VIEW_ROTATE_90: toSet = 0; // SCREEN_ORIENTATION_LANDSCAPE
		bcase VIEW_ROTATE_180: toSet = 9; // SCREEN_ORIENTATION_REVERSE_PORTRAIT
		bcase VIEW_ROTATE_270: toSet = 8; // SCREEN_ORIENTATION_REVERSE_LANDSCAPE
		bcase VIEW_ROTATE_90 | VIEW_ROTATE_270: toSet = 6; // SCREEN_ORIENTATION_SENSOR_LANDSCAPE
		bcase VIEW_ROTATE_0 | VIEW_ROTATE_180: toSet = 7; // SCREEN_ORIENTATION_SENSOR_PORTRAIT
		bcase VIEW_ROTATE_ALL: toSet = 10; // SCREEN_ORIENTATION_FULL_SENSOR
	}
	jSetRequestedOrientation(jEnv(), jBaseActivity, toSet);
	return true;
}

bool Window::requestOrientationChange(uint o)
{
	// no-op, OS manages orientation changes
	return false;
}

uint GLBufferConfigAttributes::defaultColorBits()
{
	return (!Config::MACHINE_IS_GENERIC_ARM && Base::androidSDK() >= 11) ? 24 : 16;
}

CallResult Window::init(const WindowConfig &config)
{
	if(initialInit)
		return OK;
	BaseWindow::init();
	if(!Config::BASE_MULTI_WINDOW && windows())
	{
		bug_exit("no multi-window support");
	}
	#ifdef CONFIG_BASE_MULTI_SCREEN
	screen_ = &config.screen();
	#endif

	initialInit = true;
	#ifdef CONFIG_BASE_MULTI_WINDOW
	window_.push_back(this);
	if(window_.size() > 1)
	{
		logMsg("making presentation window");
		assert(&screen() != Screen::screen(0));
		auto env = jEnv();
		jDialog = env->NewGlobalRef(jPresentation(env, Base::jBaseActivity, screen().aDisplay, this));
		initPresentationJNI(env, jDialog);
		jPresentationShow(env, jDialog);
	}
	else
	{
		logMsg("making device window");
	}
	#else
	mainWin = this;
	#endif
	eglConfig = config.glConfig().glConfig;
	pixelFormat = winFormatFromEGLConfig(GLContext::eglDisplay(), eglConfig);
	if(Base::androidSDK() < 11 && this == deviceWindow())
	{
		// In testing with CM7 on a Droid, not setting window format to match
		// what's used in ANativeWindow_setBuffersGeometry() may cause performance issues
		auto env = jEnv();
		#ifndef NDEBUG
		logMsg("setting window format to %d (current %d)", pixelFormat, jWinFormat(env, jBaseActivity));
		#endif
		jSetWinFormat(env, jBaseActivity, pixelFormat);
	}
	// default to screen's size
	updateSize({screen().width(), screen().height()});
	contentRect.x2 = width();
	contentRect.y2 = height();
	return OK;
}

void Window::deinit()
{
	assert(this != deviceWindow());
	#ifdef CONFIG_BASE_MULTI_WINDOW
	if(jDialog)
	{
		logMsg("deinit presentation");
		if(nWin)
		{
			ANativeWindow_release(nWin);
		}
		androidWindowSurfaceDestroyed(*this);
		auto env = jEnv();
		jPresentationDeinit(env, jDialog);
		env->DeleteGlobalRef(jDialog);
	}
	#endif
}

void Window::show()
{
	postDraw();
}

IG::WindowRect Window::contentBounds() const
{
	return contentRect;
}

bool Window::hasSurface()
{
	return nWin;
}

void AndroidWindow::initEGLSurface(EGLDisplay display)
{
	assert(display != EGL_NO_DISPLAY);
	assert(nWin);
	if(surface != EGL_NO_SURFACE)
	{
		return;
	}
	logMsg("creating EGL surface for native window %p", nWin);
	surface = eglCreateWindowSurface(display, eglConfig, nWin, nullptr);
	assert(surface != EGL_NO_SURFACE);
}

void AndroidWindow::destroyEGLSurface(EGLDisplay display)
{
	if(surface == EGL_NO_SURFACE)
	{
		return;
	}
	if(eglGetCurrentSurface(EGL_DRAW) == surface)
	{
		GLContext::setDrawable(nullptr);
		if(onGLDrawableChanged)
			onGLDrawableChanged(nullptr);
	}
	logMsg("destroying EGL surface for native window %p", nWin);
	if(eglDestroySurface(display, surface) == EGL_FALSE)
	{
		bug_exit("couldn't destroy surface");
	}
	surface = EGL_NO_SURFACE;
}

void AndroidWindow::updateContentRect(const IG::WindowRect &rect)
{
	contentRect = rect;
	surfaceChange.addContentRectResized();
}

void androidWindowInitSurface(Window &win, ANativeWindow *nWin)
{
	win.nWin = nWin;
	#ifndef NDEBUG
	logMsg("creating window with native visual ID: %d with format: %d", win.pixelFormat, ANativeWindow_getFormat(nWin));
	#endif
	ANativeWindow_setBuffersGeometry(nWin, 0, 0, win.pixelFormat);
	win.initEGLSurface(GLContext::eglDisplay());
	win.setNeedsDraw(true);
}

void androidWindowNeedsRedraw(Window &win)
{
	logMsg("window needs redraw event");
	win.postDraw();
}

void androidWindowContentRectChanged(Window &win, const IG::WindowRect &rect, const IG::Point2D<int> &winSize)
{
	logMsg("content rect change event: %d:%d:%d:%d in %dx%d",
		rect.x, rect.y, rect.x2, rect.y2, winSize.x, winSize.y);
	win.updateContentRect(rect);
	if(win.updateSize(winSize) && androidSDK() < 19 && !Config::MACHINE_IS_OUYA)
	{
		// On some OS versions like CM7 on the HP Touchpad,
		// the very next frame is rendered incorrectly
		// (as if the window still has its previous size).
		// Re-create the EGLSurface to make sure EGL sees
		// the new size.
		win.destroyEGLSurface(GLContext::eglDisplay());
		win.initEGLSurface(GLContext::eglDisplay());
	}
	win.postDraw();
}

void androidWindowSurfaceDestroyed(Window &win)
{
	win.unpostDraw();
	win.destroyEGLSurface(GLContext::eglDisplay());
	win.nWin = nullptr;
}

void Window::postDraw()
{
	if(hasSurface())
	{
		setNeedsDraw(true);
		screen().postFrame();
	}
}

void Window::unpostDraw()
{
	setNeedsDraw(false);
}

void restoreOpenGLContext()
{
	if(!AndroidGLContext::validateActivityThreadContext())
	{
		logMsg("current context was restored");
		if(onGLDrawableChanged)
			onGLDrawableChanged(nullptr);
	}
}

void Window::setTitle(const char *name) {}

void Window::setAcceptDnd(bool on) {}



}