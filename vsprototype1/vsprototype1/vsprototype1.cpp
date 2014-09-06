// Vectis - windows prototype 1
#include <VGUIToolkit/VGUIToolkit.h>
#include <VGUIToolkit/VWindow.h>
#include <VGUIToolkit/VCodeControl.h>

// Displays the main Vectis Win32 window
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	VGUIToolkit vToolkit;
	VWindow* win = vToolkit.createWindow(1000, 600);

	win->addControl(new VCodeControl());
	win->show();

    return 0;
}