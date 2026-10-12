#include "stdafx.h"
#include "MtWindow.h"
#include <iostream>
#include <Windows.h>


#define POINTER_DOWN 0
#define POINTER_UP  1
#define POINTER_MOVE  2
#define POINTER_BUTTON_FIRST  0
#define POINTER_BUTTON_SECOND  1
#define POINTER_BUTTON_THIRD  2
#define POINTER_BUTTON_FOURTH  3
#define POINTER_BUTTON_FITH  4

static unsigned int getButton(WPARAM wParam)
{
	if (IS_POINTER_FIRSTBUTTON_WPARAM(wParam))
	{
		return POINTER_BUTTON_FIRST;
	}
	if (IS_POINTER_SECONDBUTTON_WPARAM(wParam))
	{
		return POINTER_BUTTON_SECOND;
	}
	if (IS_POINTER_THIRDBUTTON_WPARAM(wParam))
	{
		return POINTER_BUTTON_THIRD;
	}
	if (IS_POINTER_FOURTHBUTTON_WPARAM(wParam))
	{
		return POINTER_BUTTON_FOURTH;
	}
	if (IS_POINTER_FIFTHBUTTON_WPARAM(wParam))
	{
		return POINTER_BUTTON_FITH;
	}
	return POINTER_BUTTON_FIRST;
}


LRESULT CALLBACK WindowSubClassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MtWindow *window = (MtWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	return window->handleWindowMessage(hwnd, uMsg, wParam, lParam);
}

MtWindow::MtWindow(JNIEnv * jniEnv, jobject object, HWND hwnd)
{
	this->jniEnv = jniEnv;
	this->hwdn = hwnd;
	
	jclass cls = jniEnv->GetObjectClass(object);
	this->callbackMethod = jniEnv->GetMethodID(cls, "nativeOnTouchCallback", "(JIIIII)V");
	if (callbackMethod == 0) {
		std::cerr << "Could not get id for callback method" << std::endl;
		throw std::exception();
	}

	SetLastError(0);
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
	DWORD errorId = GetLastError();
	if (errorId != 0) {
		std::cerr << "Error SetWindowLongPtr userdata " << errorId << std::endl;
		throw std::exception();
	}

	SetLastError(0);
	this->originalWndProc = SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WindowSubClassProc);
	errorId = GetLastError();
	if (this->originalWndProc == 0 && errorId != 0) {
		std::cerr << "Error SetWindowLongPtr " << errorId << std::endl;
		throw std::exception();
	}

	EnableMouseInPointer(true);
	RegisterTouchWindow(hwnd, TWF_WANTPALM);
	this->object = jniEnv->NewGlobalRef(object);
}

MtWindow::~MtWindow()
{
	if (this->object != 0) {
		this->jniEnv->DeleteGlobalRef(this->object);
	}
}

LRESULT CALLBACK MtWindow::handleWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_POINTERUP:
	case WM_POINTERDOWN:
	case WM_POINTERUPDATE:
		if (this->handlePointerEvent(wParam))
			return 0;
		break;
	}

	return CallWindowProc((WNDPROC)this->originalWndProc, hwnd, uMsg, wParam, lParam);
}

bool MtWindow::handlePointerEvent(WPARAM wParam)
{
	UINT32 pointerId = GET_POINTERID_WPARAM(wParam);

	POINTER_INFO pointerInfo;
	if (!GetPointerInfo(pointerId, &pointerInfo))
	{
		std::cerr << "GetPointerInfo failed: 0x" << std::hex << GetLastError() << std::endl;
		return false;
	}
	HWND hwnd = pointerInfo.hwndTarget;
	ScreenToClient(hwnd, &(pointerInfo.ptPixelLocation));
	LONG xPos = pointerInfo.ptPixelLocation.x;
	LONG yPos = pointerInfo.ptPixelLocation.y;

	int button = getButton(wParam);

	if (pointerInfo.pointerFlags & POINTER_FLAG_DOWN) {
		this->jniEnv->CallVoidMethod(this->object, this->callbackMethod, (jlong)hwnd, (jint)xPos, (jint)yPos, (jint)pointerId, POINTER_DOWN, button);
		return true;
	}
	if (pointerInfo.pointerFlags & POINTER_FLAG_UPDATE) {
		this->jniEnv->CallVoidMethod(this->object, this->callbackMethod, (jlong)hwnd, (jint)xPos, (jint)yPos, (jint)pointerId, POINTER_MOVE, button);
		return true;
	}
	if (pointerInfo.pointerFlags & POINTER_FLAG_UP) {
		this->jniEnv->CallVoidMethod(this->object, this->callbackMethod, (jlong)hwnd, (jint)xPos, (jint)yPos, (jint)pointerId, POINTER_UP, button);
		return true;
	}
	return false;
}
