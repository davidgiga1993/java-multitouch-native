package org.devcore.touch.win;

import org.devcore.touch.Multitouch;
import org.devcore.touch.MultitouchListener;
import org.devcore.touch.util.NativeLibraryLoader;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Handles multitouch events for one or more windows.
 * The coordinates used are always relative to the client area of the window
 */
public class WinMultitouch implements Multitouch
{
	private final List<Long> activeHwnd = new ArrayList<>();
	private final Map<Long, MultitouchListener> callbacks = new HashMap<>();

	private native int initTouch(long hwnFd);

	private native void dispose(long hwnd);

	public WinMultitouch()
	{
		NativeLibraryLoader.load("WinMultitouch");
	}

	@Override
	public void dispose()
	{
		for (long hwnd : activeHwnd)
		{
			dispose(hwnd);
		}
		activeHwnd.clear();
	}

	@Override
	public void removeWindow(long hwnd)
	{
		dispose(hwnd);
		activeHwnd.remove(hwnd);
	}

	@Override
	public void addWindow(long hwnd, MultitouchListener listener)
	{
		if (!Thread.currentThread().getName().equals("main"))
		{
			throw new RuntimeException("Initialization must be done from the main thread");
		}

		if (initTouch(hwnd) != 0)
		{
			throw new RuntimeException("Registering the handler failed");
		}

		activeHwnd.add(hwnd);
		callbacks.put(hwnd, listener);
	}


	/**
	 * This method will be called by the native callback
	 *
	 * @param hwnd    Window handle
	 * @param x       X pos
	 * @param y       Y pos
	 * @param pointer Pointer
	 * @param mode    Click mode
	 * @param button  Button
	 */
	@SuppressWarnings("unused")
	protected void nativeOnTouchCallback(long hwnd, int x, int y, int pointer, int mode, int button)
	{
		MultitouchListener processor = callbacks.get(hwnd);
		if (processor != null)
		{
			processor.onTouch(x, y, pointer, mode, button);
		}
	}
}
