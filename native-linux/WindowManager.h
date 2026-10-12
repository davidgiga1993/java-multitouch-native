#pragma once
#include <jni.h>
#include <X11/Xlib.h>
#include <vector>
#include <thread>
#include <mutex>
#include "MtWindow.h"

class WindowManager
{
public:
	WindowManager();
	~WindowManager();

	/*
	Adds a multitouch listener to the given window
	*/
	void addWindow(JNIEnv *jniEnv, jobject object, Window window);

	/*
	Removes the listener for the given window
	*/
	void removeWindow(Window window);

private:
	Display *display;
	int xiOpcode;
	Window wakeupWindow;
	bool running;
	std::thread eventThread;
	std::mutex windowsMutex;
	std::vector<MtWindow *> windows;

	void selectTouchEvents(Window window);
	void eventLoop();
	void wakeupEventLoop();
	void stopEventLoop();
	MtWindow *findWindow(Window window);
};
