#include <windows.h>

bool running = true;
void* buffer_memory;
int buffer_width;
int buffer_height;

// A bitmap (BMP) is a graphical object used to create, manipulate and store images as files on a disk
// A Device-Independent bitmap (DIB) is a raster image file that is similar to the standard Bitmap files (BMP)
// The BITMAPINFO structure defines the dimensions and color information for a DIB
BITMAPINFO buffer_bitmap_info;

// WndProc func to handle messages from Windows OS (event-driven)
LRESULT CALLBACK window_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;
	switch (uMsg)                   // If you receive close or destroy messages from Windows..
	{
	case WM_CLOSE:
	case WM_DESTROY: {
		running = false;            // ..Set running to be false so that the game loop stops
	} break;

	case WM_SIZE: {                 // Recreate screen buffer whenever the window size is changed
		RECT rect;                  // RECT structure defines a rectangle by the coordinates of its upper-left and lower-right corners
		GetClientRect(hwnd, &rect); // Gives the coordinates of a window's client area ie. size of area below status bar
		buffer_width = rect.right - rect.left;
		buffer_height = rect.bottom - rect.top;

		int buffer_size = buffer_width * buffer_height * sizeof(unsigned int);

		// Free prev. buffer memory on changing size so that new buffer memory can be allocated
		if (buffer_memory) VirtualFree(buffer_memory, 0, MEM_RELEASE); // Free all (specified by 0 and MEM_RELEASE) memory at address buffer_memory
		buffer_memory = VirtualAlloc(0, buffer_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); // buffer_size memory allocated with READWRITE perms using MEM_COMMIT | MEM_RESERVE type allocation

		// Bitmap_Info struct member initialization
		buffer_bitmap_info.bmiHeader.biSize = sizeof(buffer_bitmap_info.bmiHeader); // The number of bytes required by the structure
		buffer_bitmap_info.bmiHeader.biWidth = buffer_width;                        // width of the bitmap in pixels
		buffer_bitmap_info.bmiHeader.biHeight = buffer_height;                      // height of the bitmap in pixels
		buffer_bitmap_info.bmiHeader.biPlanes = 1;                                  // number of planes for the target device must be set to 1
		buffer_bitmap_info.bmiHeader.biBitCount = 32;                               // number of bits-per-pixel (32 bits since we're using unsigned int for each pixel)
		buffer_bitmap_info.bmiHeader.biCompression = BI_RGB;                        // type of compression for a compressed bottom-up (ie. height > 0) bitmap, BI_RGB => uncompressed

	} break;

	default: // ..Otherwise, just create the default window procedure for the given parameters and return it
		result = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return result;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {

	// Create a Window class that can be used to create our kind of windows
	WNDCLASS window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpszClassName = L"Game Window Class";
	window_class.lpfnWndProc = window_callback;        // Function ptr to our defined window procedure handler function

	// Register class so that windows knows about it and CreateWindow() creates windows using this class
	RegisterClass(&window_class);

	// Create window using our defined Window class and WINAPI CreateWindow()
	HWND window =
		CreateWindow(                         // Creates a Window using window class, window title, window style, initial position and size of window
			window_class.lpszClassName,       // lpClassName: A null-terminated string created by a previous call to the RegisterClass
			L"My First Game",                 // lpWindowName: Window name to be displayed in the title (if window style allows it)
			WS_OVERLAPPEDWINDOW | WS_VISIBLE, // dwStyle: style of the window being created, can be a combination of the window style values (using bitfields)
			CW_USEDEFAULT,                    // x: intial horizontal position of the window (CW_USEDEFAULT for default x)
			CW_USEDEFAULT,                    // y: intial vertical position of the window (CW_USEDEFAULT for default y)
			1280,                             // nWidth: width (in device units) of the window
			720,                              // nHeight: height (in device units) of the window
			0,                                // hWndParent (optional): handle to the parent or owner window of the window being created
			0,                                // hMenu (optional): handle to a menu
			hInstance,                        // hInstance (optional): handle to the instance of the module to be associated with the window
			0                                 // lpParam (optional): message is sent to the created window by this function before it returns
		);                                    // If the function succeeds, the return value is a handle to the new window

	HDC hdc = GetDC(window);                  // Get Device context for our current window to be used as an argument for StretchDIBits()


	while (running) {

		// (1) Take Input

		// Struct for messages from Windows (or from Users->Windows->our App)
		MSG message;
		while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		// (2) Simulate stuff
		unsigned int* pixel = (unsigned int*)buffer_memory;
		for (int y = 0; y < buffer_height; y++) {
			for (int x = 0; x < buffer_width; x++) {
				*pixel++ = (y*y)/4 + (x*x)/3 - (x*y);
			}
		}

		// (3) Render stuff on screen

		// StretchDIBits() copies the color data for a rectangle of pixels in a DIB/JPEG/PNG image to the specified destination rectangle
		StretchDIBits(
			hdc,                  // hdc: handle to the destination device context
			0,                    // xDest: x-coordinate of the upper-left corner of the destination rectangle (logical units)
			0,                    // yDest: y-coordinate of the bottom-right corner of the destination rectangle (logical units)
			buffer_width,         // DestWidth: width of the destination rectangle (logical units)
			buffer_height,        // DestHeight: height of the of the destination rectangle (logical units)
			0,                    // xSrc: x-coordinate of the source rectangle (in pixels)
			0,                    // ySrc: y-coordinate of the source rectangle (in pixels)
			buffer_width,         // SrcWidth: width of the source rectangle (in pixels)
			buffer_height,        // SrcHeight: height of the of the source rectangle (in pixels)
			buffer_memory,        // *lpBits: pointer to the image bits which are stored as an array of bytes
			&buffer_bitmap_info,  // *lpbmi: pointer to a BITMAPINFO structure that contains information about the DIB
			DIB_RGB_COLORS,       // iUsage: whether bmiColors of BITMAPINFO struct contains explicit RGB values
			SRCCOPY               // rop: raster-operation specifies how src pixels and dest pixels are combined to form the new image
		);                        // If the function succeeds, the return value is the number of scan lines copied
	}
	return 0;
}
