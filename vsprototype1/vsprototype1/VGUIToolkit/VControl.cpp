//#include <VGUIToolkit/VControl.h>
//#include <stdexcept>
//
//VControl::VControl( WNDPROC controlWndProc, LPCSTR className, int controlDataSize ) {
//	WNDCLASSEX ctrlWndClassEx = { 0 };
//
//	// Don't use CS_HREDRAW and CS_VREDRAW otherwise WM_SIZE would invalidate the
//	// entire control area each time a redraw is needed
//	ctrlWndClassEx.cbSize        = sizeof( WNDCLASSEX );
//    ctrlWndClassEx.style         = CS_GLOBALCLASS;
//    ctrlWndClassEx.lpfnWndProc   = controlWndProc;
//    ctrlWndClassEx.cbClsExtra    = controlDataSize;
//    ctrlWndClassEx.cbWndExtra    = 0;
//    ctrlWndClassEx.hCursor       = LoadCursor( NULL, IDC_ARROW );
//    ctrlWndClassEx.lpszClassName = className;
//
//    if(!RegisterClassEx(&ctrlWndClassEx)) {
//        throw std::runtime_error("Window registration failed");
//    }
//}