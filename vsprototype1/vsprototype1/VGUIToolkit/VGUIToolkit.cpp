#include <VGUIToolkit/VGUIToolkit.h>

VGUIToolkit::VGUIToolkit() {}

VWindow*
VGUIToolkit::createWindow( int width, int height ) {
	VWindow* ptr = new VWindow( width, height );
	m_windows.push_back(std::shared_ptr<VWindow>(ptr));
	return ptr;
}