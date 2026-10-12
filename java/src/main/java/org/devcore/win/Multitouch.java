package org.devcore.win;

import org.devcore.util.NativeLibraryLoader;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Handles multitouch events for one or more windows.
 * The coordinates used are always relative to the client area of the window
 */
public class Multitouch
{
	public static final int POINTER_DOWN = 0;
	public static final int POINTER_UP = 1;
	public static final int POINTER_MOVE = 2;


	public static final int BUTTON_1 = 0;
	public static final int BUTTON_2 = 1;
	public static final int BUTTON_3 = 2;
	public static final int BUTTON_4 = 3;
	public static final int BUTTON_5 = 4;

	private final List<Long> activeHwnd = new ArrayList<>();
	private final Map<Long, MultitouchProcessor> callbacks = new HashMap<>();

	private native int initTouch(long hwnd);

	private native void dispose(long hwnd);

	public Multitouch()
	{
		NativeLibraryLoader.load("WinMultitouch");
	}

	/**
	 * Frees any allocated native memory and removes the listeners
	 */
	public void dispose()
	{
		for (long hwnd : activeHwnd)
		{
			dispose(hwnd);
		}
		activeHwnd.clear();
	}

	/**
	 * Removes the listener for the window with the given hwnd
	 *
	 * @param hwnd Window handle
	 */
	public void removeWindow(long hwnd)
	{
		dispose(hwnd);
		activeHwnd.remove(hwnd);
	}

	/**
	 * Adds a listener to the window with the given handle
	 *
	 * @param hwnd      Window handle
	 * @param processor Touch processor
	 */
	public void addWindow(long hwnd, MultitouchProcessor processor)
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
		callbacks.put(hwnd, processor);
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
		MultitouchProcessor processor = callbacks.get(hwnd);
		if (processor != null)
		{
			processor.onTouch(x, y, pointer, mode, button);
		}
	}
}
