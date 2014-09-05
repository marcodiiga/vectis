#include <VGUIToolkit/VCodeControl.h>
#include <VGUIToolkit/VWindow.h>
#include <fstream>

VCodeControl::VCodeControl( int x, int y, int cx, int cy ) {
	// Create the clipping region for this control
	m_clipRegion = CreateRectRgn(x,y,cx,cy);
	// DEBUG
	loadTextFile("vsprototype1.cpp");
}

void 
VCodeControl::paint(HDC& hdc) {
	// TODO: handle dimensions, drawing area (dpi stuff?) and rects
	
	// First select our clipping region
	SelectClipRgn(hdc, m_clipRegion);

	RECT rect;
	rect.top = 
	GetClientRect(m_parent->getHandle(), &rect);
	Rectangle( hdc, rect.left,rect.top,  rect.right, rect.bottom);

	if( !m_plainText.empty() ) {
		// Render the file text
		TextOut(hdc, 0, 0, m_plainText.c_str(), (int)m_plainText.size());
	}
}

bool
VCodeControl::loadTextFile( std::string filePath ) {
	// Fast file reading with C++ streams
	std::ifstream file( filePath, std::ios::in );
	if( !file.is_open() )
		return false;
    file.seekg( 0, std::ios::end );
    m_plainText.resize( file.tellg() );
    file.seekg( 0, std::ios::beg );
    file.read( &m_plainText[0], m_plainText.size() );
    file.close();
	return true;
}
