#include "WindowManager.h"
#include <X11/extensions/XInput2.h>
#include <stdexcept>
#include <iostream>

WindowManager::WindowManager()
	: display(nullptr), xiOpcode(0), wakeupWindow(0), running(false)
{
}

WindowManager::~WindowManager()
{
	this->stopEventLoop();
}

void WindowManager::selectTouchEvents(Window window)
{
	unsigned char mask[XIMaskLen(XI_LASTEVENT)] = {0};
	XISetMask(mask, XI_TouchBegin);
	XISetMask(mask, XI_TouchUpdate);
	XISetMask(mask, XI_TouchEnd);

	XIEventMask eventMask;
	eventMask.deviceid = XIAllMasterDevices;
	eventMask.mask_len = sizeof(mask);
	eventMask.mask = mask;

	if (XISelectEvents(this->display, window, &eventMask, 1) != Success)
	{
		throw std::runtime_error("Unable to select XInput2 touch events for window");
	}
	XFlush(this->display);
}

void WindowManager::addWindow(JNIEnv *jniEnv, jobject object, Window window)
{
	if (this->display == nullptr)
	{
		this->display = XOpenDisplay(nullptr);
		if (this->display == nullptr)
		{
			throw std::runtime_error("Unable to open X11 display");
		}

		int eventBase, errorBase;
		if (!XQueryExtension(this->display, "XInputExtension", &this->xiOpcode, &eventBase, &errorBase))
		{
			XCloseDisplay(this->display);
			this->display = nullptr;
			throw std::runtime_error("XInput extension is not available");
		}

		int major = 2, minor = 2;
		if (XIQueryVersion(this->display, &major, &minor) == BadRequest)
		{
			XCloseDisplay(this->display);
			this->display = nullptr;
			throw std::runtime_error("XInput2 is not available");
		}

		this->wakeupWindow = XCreateSimpleWindow(this->display, DefaultRootWindow(this->display), 0, 0, 1, 1, 0, 0, 0);
	}

	this->selectTouchEvents(window);

	auto mtWindow = new MtWindow(jniEnv, object, this->display, window);
	{
		std::lock_guard<std::mutex> lock(this->windowsMutex);
		this->windows.push_back(mtWindow);
	}

	if (!this->running)
	{
		this->running = true;
		this->eventThread = std::thread(&WindowManager::eventLoop, this);
	}
}

void WindowManager::removeWindow(Window window)
{
	bool empty;
	{
		std::lock_guard<std::mutex> lock(this->windowsMutex);
		for (size_t i = 0; i < this->windows.size(); i++)
		{
			if (this->windows[i]->window != window)
			{
				continue;
			}
			delete this->windows[i];
			this->windows.erase(this->windows.begin() + i);
			break;
		}
		empty = this->windows.empty();
	}

	if (empty)
	{
		this->stopEventLoop();
	}
}

void WindowManager::wakeupEventLoop()
{
	if (this->display == nullptr)
	{
		return;
	}

	XClientMessageEvent event = {0};
	event.type = ClientMessage;
	event.window = this->wakeupWindow;
	event.format = 32;
	event.message_type = XInternAtom(this->display, "MT_WAKEUP", False);
	XSendEvent(this->display, this->wakeupWindow, False, NoEventMask, (XEvent *)&event);
	XFlush(this->display);
}

void WindowManager::stopEventLoop()
{
	if (!this->running)
	{
		return;
	}

	this->running = false;
	this->wakeupEventLoop();
	if (this->eventThread.joinable())
	{
		this->eventThread.join();
	}

	if (this->display != nullptr)
	{
		XDestroyWindow(this->display, this->wakeupWindow);
		XCloseDisplay(this->display);
		this->display = nullptr;
	}
}

MtWindow *WindowManager::findWindow(Window window)
{
	std::lock_guard<std::mutex> lock(this->windowsMutex);
	for (auto mtWindow : this->windows)
	{
		if (mtWindow->window == window)
		{
			return mtWindow;
		}
	}
	return nullptr;
}

void WindowManager::eventLoop()
{
	while (this->running)
	{
		XEvent event;
		XNextEvent(this->display, &event);

		if (!this->running)
		{
			break;
		}

		if (event.type != GenericEvent || event.xcookie.extension != this->xiOpcode)
		{
			continue;
		}

		if (!XGetEventData(this->display, &event.xcookie))
		{
			continue;
		}

		int mode;
		switch (event.xcookie.evtype)
		{
		case XI_TouchBegin:
			mode = 0; // POINTER_DOWN
			break;
		case XI_TouchUpdate:
			mode = 2; // POINTER_MOVE
			break;
		case XI_TouchEnd:
			mode = 1; // POINTER_UP
			break;
		default:
			XFreeEventData(this->display, &event.xcookie);
			continue;
		}

		XIDeviceEvent *deviceEvent = (XIDeviceEvent *)event.xcookie.data;
		Window targetWindow = deviceEvent->event;
		int touchId = deviceEvent->detail;
		double x = deviceEvent->event_x;
		double y = deviceEvent->event_y;
		XFreeEventData(this->display, &event.xcookie);

		MtWindow *mtWindow = this->findWindow(targetWindow);
		if (mtWindow != nullptr)
		{
			mtWindow->handleTouchEvent(mode, x, y, touchId);
		}
	}
}
