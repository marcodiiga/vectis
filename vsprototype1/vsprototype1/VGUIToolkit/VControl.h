#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class VWindow;

class VControl {
public:
	VControl() : m_visible(true) {}

	virtual void paint( HDC& hdc ) = 0;

	void setControlRect(RECT& rect) { m_controlRect = rect; }

	bool isVisible() { return m_visible; }
	void setVisible( bool visible ) { m_visible = visible; }

	void setParent( VWindow* win ) { m_parent = win; }

protected:
	bool m_visible;
	VWindow* m_parent;
	RECT m_controlRect;
};
