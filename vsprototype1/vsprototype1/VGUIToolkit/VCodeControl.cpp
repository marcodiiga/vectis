#include <VGUIToolkit/VCodeControl.h>
#include <VGUIToolkit/VWindow.h>
#include <fstream>

VCodeControl::VCodeControl()
{
	m_controlRect.bottom = m_controlRect.left = m_controlRect.right = m_controlRect.top = -1;
	// Create the clipping region for this control
	//m_clipRegion = CreateRectRgn(10,10,200,200);
	// DEBUG
	loadTextFile("vsprototype1.cpp");
}

void 
VCodeControl::paint(HDC& hdc) {
	// TODO: handle dimensions, drawing area (dpi stuff?) and rects
	
	// First select our clipping region
	// CreateRectRgn (if needed)
	//SelectClipRgn(hdc, m_clipRegion);

	// Draw control background
	FillRect( hdc, &m_controlRect, m_parent->getBackgroundBrush() );

	//RECT rect;
	//rect.top = 
	//GetRgnBox(m_clipRegion, &rect);
	//Rectangle( hdc, rect.left,rect.top,  rect.right, rect.bottom);

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
