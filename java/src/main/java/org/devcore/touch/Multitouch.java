package org.devcore.touch;

public interface Multitouch
{
	/**
	 * Frees any allocated native memory and removes the listeners
	 */
	void dispose();

	/**
	 * Adds a listener to the window with the given window handle/id
	 *
	 * @param hwnd     Window handle
	 * @param listener Touch processor
	 */
	void addWindow(long hwnd, MultitouchListener listener);

	/**
	 * Removes the listener for the window with the given window handle/id
	 */
	void removeWindow(long hwnd);
}
