#pragma once

#include <VGUIToolkit/VControl.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vector>

// WndProc helper classes used to pass a VWindow pointer along
LRESULT CALLBACK WndProcHelper( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp );
LRESULT CALLBACK WndProcRedirector( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp );

class VWindow {
public:
	VWindow( int width, int height );

	void addControl( VControl* cnt );

	HWND getHandle() { return m_hwnd; }

	void show();

	LRESULT CALLBACK VWinProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

private:
	HWND m_hwnd;
	int m_width, m_height;

	// Controls owned by this window are stored in this vector. The iterator points to one
	// element after the last control which is visible in the window. If there is no visible
	// control (or no control at all), it points to vector::end()
	std::vector<VControl*> m_controls;
	std::vector<VControl*>::iterator m_endVisible;
};
