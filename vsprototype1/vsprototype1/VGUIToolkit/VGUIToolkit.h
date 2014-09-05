#pragma once

#include <VGUIToolkit/VWindow.h>
#include <memory>
#include <vector>

// An exception class used to store exception data
//class VGUIException {
//	VGUIException
//};


class VGUIToolkit {
public:
	VGUIToolkit();

	// Create a VWindow
	VWindow* createWindow( int width, int height );


private:
	
	std::vector<std::shared_ptr<VWindow>> m_windows;
};