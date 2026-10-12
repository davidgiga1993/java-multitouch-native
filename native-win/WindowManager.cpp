#include "stdafx.h"
#include "WindowManager.h"
#include "MtWindow.h"


void WindowManager::addWindow(JNIEnv *jniEnv, jobject object, HWND hwnd)
{
	auto window = new MtWindow(jniEnv, object, hwnd);
	this->windows.push_back(window);
}

void WindowManager::removeWindow(HWND hwnd)
{
	for (int X = 0; X < this->windows.size(); X++) {
		auto window = this->windows[X];
		if (window->hwdn != hwnd) {
			continue;
		}
		delete window;
		this->windows.erase(this->windows.begin() + X);
		return;
	}
}

WindowManager::~WindowManager()
{
}
