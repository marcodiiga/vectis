#include <VGUIToolkit/VWindow.h>
#include <algorithm>
#include <stdexcept>

#define VWINCLASS "VWin"

VWindow::VWindow( int width, int height ) 
	: m_width(width)
	, m_height(height)
	, m_memBuffer(0)
	, m_backgroundBrush(0)
{
	// No visible controls for now
	m_endVisible = m_controls.end();

	// Register a windows class and a render area
	WNDCLASSEX wce = {0};
    wce.cbSize        = sizeof( WNDCLASSEX );
    wce.style         = 0;
    wce.lpfnWndProc   = WndProcHelper;
    wce.cbClsExtra    = 0;
    wce.cbWndExtra    = sizeof( VWindow* );
    wce.hInstance     = 0;
    wce.hIcon         = LoadIcon( NULL, IDI_APPLICATION );
    wce.hCursor       = LoadCursor( NULL, IDC_ARROW );
	m_backgroundBrush = (HBRUSH)( CreateSolidBrush(RGB(23,24,20)) );
    wce.hbrBackground = m_backgroundBrush;
    wce.lpszMenuName  = NULL;
    wce.lpszClassName = VWINCLASS;
    wce.hIconSm       = LoadIcon( NULL, IDI_APPLICATION );

    if( !RegisterClassEx( &wce ) ) {
        throw std::runtime_error("Window registration failed");
    }
}

VWindow::~VWindow() {
	DeleteObject(m_backgroundBrush);
}

namespace {
	// Internal comparison functions and predicates to order the visible controls
	bool isVisibleCmp( VControl* e1, VControl* e2 ) {
		if( e1->isVisible() == FALSE && e2->isVisible() == TRUE )
			return false;
		else
			return true;
	}
	struct isNotVisiblePred {
		bool operator() ( VControl* cnt ) {
			return !cnt->isVisible();
		}
	};
}

void
VWindow::addControl( VControl* cnt ) {

	m_controls.push_back( cnt );
	cnt->setParent( this );

	// Sort the vector to keep the visible controls at the beginning
	std::sort( m_controls.begin(), m_controls.end(), isVisibleCmp );
	m_endVisible = std::find_if( m_controls.begin(), m_controls.end(), isNotVisiblePred() );
}

void
VWindow::show() {
	// Create the window
	m_hwnd = CreateWindowEx( WS_EX_CLIENTEDGE, VWINCLASS, "Vectis Prototype 1", WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, CW_USEDEFAULT, m_width, m_height, NULL, NULL, 0, this );

    if( m_hwnd == NULL ) {
        throw std::runtime_error("Window creation failed");
    }

	ShowWindow( m_hwnd, SW_SHOW );

	// Get the message loop running
	MSG msg;
	while( GetMessage( &msg, NULL, 0, 0 ) > 0 ) {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
}

// Main window message loop
LRESULT CALLBACK
VWindow::VWinProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
    switch(msg) {
        case WM_CLOSE:
            DestroyWindow( hwnd );
        break;
        case WM_DESTROY:
            PostQuitMessage( 0 );
        break;
		case WM_CREATE: 
		{
			// Initialize the memory buffer used for double buffering to the maximum extent
			HDC winHDC = GetDC(hwnd);
			HDC screenHDC = GetDC(NULL);
			int screenCX = GetDeviceCaps(screenHDC, HORZRES);
			int screenCY = GetDeviceCaps(screenHDC, VERTRES);
			m_memBuffer = CreateCompatibleBitmap(winHDC, screenCX, screenCY);
			ReleaseDC(hwnd, winHDC);
			ReleaseDC(NULL, screenHDC);

			// Calculate each control extent
			RECT rect;
			GetClientRect(hwnd, &rect);
			for( std::vector<VControl*>::iterator it = m_controls.begin(); it != m_endVisible; ++it )
				(*it)->setControlRect(rect);
			
		} break;
		case WM_ERASEBKGND:
			return TRUE; // Not needed when double buffering
		break;
		case WM_PAINT: 
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			
			//>DEBUG draw into the m_memBuffer
			{
			   HDC wDC = GetDC(hwnd);
			   HDC bDC = CreateCompatibleDC(wDC);
			   HBITMAP bOld = (HBITMAP)SelectObject(bDC,m_memBuffer);
			   HBRUSH brush = CreateSolidBrush(RGB(255,0,0));

			   // Draw background
			   /*RECT rect;
			   GetClientRect( hwnd, &rect );
			   FillRect(bDC, &rect, m_backgroundBrush);*/

			   // Disegna i controlli (c'è da rifarlo con l'update selettivo a seconda di dove clicca
			   // l'utente, del callback di SetTimer (per il caret che blinka) e di cosa inserisce con tastiera
			   for( std::vector<VControl*>::iterator it = m_controls.begin(); it != m_endVisible; ++it )
					(*it)->paint( bDC );

			   DeleteObject(brush);
			   SelectObject(bDC, bOld);
			   DeleteDC(bDC);
			   ReleaseDC(hwnd,wDC);
			}
			//<DEBUG


			// Create a compatible DC, associate it to the bitmap in the memory buffer and
			// draw it to the client area
			HDC tempDC = CreateCompatibleDC(hdc);
			HBITMAP bOld = (HBITMAP)SelectObject(tempDC,m_memBuffer);

			BitBlt(ps.hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom, tempDC, 0, 0, SRCCOPY);
			OutputDebugString("BitBlt from m_memBuffer\n");

			SelectObject(tempDC, bOld);
			DeleteDC(tempDC);
			ReleaseDC(hwnd,tempDC);

			EndPaint(hwnd, &ps);


			//PAINTSTRUCT ps;
			//HDC hdc;
			//RECT rect;

			//GetClientRect( hwnd, &rect );
			//hdc = BeginPaint( hwnd, &ps );

			//// Background color
			//FillRect( hdc, &rect, CreateSolidBrush( RGB( 23,24,20 ) ) );
	
			//// Draw all the visible controls
			//for( std::vector<VControl*>::iterator it = m_controls.begin(); 
			//	 it != m_endVisible; ++it ) {
			//		(*it)->paint( hdc );
			//		OutputDebugString("Redrawing control..\n");
			//}
			//
			//EndPaint( hwnd, &ps );
		} break;
		case WM_SIZE:
			// TODO: there's no need to redraw everything when shrinking down, and just redraw the new area when growing
			RECT rect;
			GetClientRect(this->getHandle(), &rect);
			InvalidateRect( this->getHandle(), &rect, FALSE);
		break;
        default:
            return DefWindowProc( hwnd, msg, wParam, lParam );
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK 
WndProcHelper( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	// WM_NCCREATE is called before the window creation and can pass along any pointer
	if (msg == WM_NCCREATE) {
		LPCREATESTRUCT cs = (LPCREATESTRUCT) lp;
		// Set associated this pointer
		SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR) cs->lpCreateParams );
		// Reset the window message handler
		SetWindowLongPtr( hwnd, GWLP_WNDPROC, (LONG_PTR) WndProcRedirector );
		// Dispatch WM_NCCREATE to the member message handler
		return WndProcRedirector( hwnd, msg, wp, lp );
	} else {
		// Any message before the new WndProc is associated will have to be default processed
		return DefWindowProc( hwnd, msg, wp, lp );
	}
}

LRESULT CALLBACK 
WndProcRedirector( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	// Get the VWindow pointer passed in the user data
	VWindow *w = (VWindow *) GetWindowLongPtr( hwnd, GWLP_USERDATA );
	_ASSERT( w );
	// Redirect messages to the window procedure of the associated window
	return w->VWinProc( hwnd, msg, wp, lp );
}