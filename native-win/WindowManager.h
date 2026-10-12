#pragma once
#include <jni.h>
#include <vector>
#include "MtWindow.h"

class WindowManager
{
public:
	/*
	Adds a multitouch listener to the given window
	*/
	void addWindow(JNIEnv *jniEnv, jobject object, HWND hwnd);
	
	/*
	Removes the listener for the given window
	*/
	void removeWindow(HWND hwnd);

	~WindowManager();

private:
	std::vector<MtWindow *> windows;
};

