#include <windows.h>
#include "utils.cpp"

struct Render_State {
	int width, height;
	void* memory;

	// A bitmap (BMP) is a graphical object used to create, manipulate and store images as files on a disk
	// A Device-Independent bitmap (DIB) is a raster image file that is similar to the standard Bitmap files (BMP)
	// The BITMAPINFO structure defines the dimensions and color information for a DIB
	BITMAPINFO bitmap_info;
};

global_variable bool running = true;
global_variable Render_State render_state;

#include "renderer.cpp"
#include "platform_common.cpp"
#include "game.cpp"

// WndProc func to handle messages from Windows OS (event-driven)
LRESULT CALLBACK window_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;
	switch (uMsg) {                     // If you receive close or destroy messages from Windows..
		case WM_CLOSE:
		case WM_DESTROY: {
			running = false;            // ..Set running to be false so that the game loop stops
		} break;

		case WM_SIZE: {                 // Recreate screen buffer whenever the window size is changed
			RECT rect;                  // RECT structure defines a rectangle by the coordinates of its upper-left and lower-right corners
			GetClientRect(hwnd, &rect); // Gives the coordinates of a window's client area ie. size of area below status bar
			render_state.width = rect.right - rect.left;
			render_state.height = rect.bottom - rect.top;

			int size = render_state.width * render_state.height * sizeof(u32);

			// Free prev. buffer memory on changing size so that new buffer memory can be allocated
			if (render_state.memory) VirtualFree(render_state.memory, 0, MEM_RELEASE); // Free all (specified by 0 and MEM_RELEASE) memory at address render_state.memory
			render_state.memory = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); // render_state.size memory allocated with READWRITE perms using MEM_COMMIT | MEM_RESERVE type allocation

			// Bitmap_Info struct member initialization
			render_state.bitmap_info.bmiHeader.biSize = sizeof(render_state.bitmap_info.bmiHeader); // The number of bytes required by the structure
			render_state.bitmap_info.bmiHeader.biWidth = render_state.width;                        // width of the bitmap in pixels
			render_state.bitmap_info.bmiHeader.biHeight = render_state.height;                      // height of the bitmap in pixels
			render_state.bitmap_info.bmiHeader.biPlanes = 1;                                        // number of planes for the target device must be set to 1
			render_state.bitmap_info.bmiHeader.biBitCount = 32;                                     // number of bits-per-pixel (32 bits since we're using u32 for each pixel)
			render_state.bitmap_info.bmiHeader.biCompression = BI_RGB;                              // type of compression for a compressed bottom-up (ie. height > 0) bitmap, BI_RGB => uncompressed

		} break;

		default:                        // ..Otherwise, just create the default window procedure for the given parameters and return it
			result = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return result;
}

// Entry Point for the Window-based game
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	// Do not show mouse cursor
	ShowCursor(FALSE);

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
			L"My Pong Game",                  // lpWindowName: Window name to be displayed in the title (if window style allows it)
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

	// Set to always fullscreen
	{
		SetWindowLong(window, GWL_STYLE, GetWindowLong(window, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &mi);
		SetWindowPos(window, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}

	HDC hdc = GetDC(window);                  // Get Device context for our current window to be used as an argument for StretchDIBits()

	Input input = {};                         // Empty Input struct to hold Button_State for all buttons

	// ----------- Begin Frame - Time Delta Calculation -----------------

	float delta_time = 0.016666f;                // 60 FPS assumed at the starting of the frame
	LARGE_INTEGER frame_begin_time;              // Start ISO time stored using large_integer
	QueryPerformanceCounter(&frame_begin_time);  // Store high resolution (<1us) time stamp in frame_begin_time

	// Performance Frequency for Seconds per Frame
	float performance_freq; {                    // Empty scope to store the perf-freq calculations
		LARGE_INTEGER perf;
		QueryPerformanceFrequency(&perf);        // Store Performance Frequency in perf
		performance_freq = (float)perf.QuadPart; // Get 64-bit part from LARGE_INTEGER then convert to float
	}

	while (running) {

		// ------------ (1) Take Input -------------------------
		// MSG Struct used for messages from Windows (or from Users->Windows->our App)
		MSG msgInput;

		// Reset .changed for all buttons to be false at the start of each frame (will be modified by switch (vk_code) when key pressed)
		for (int i = 0; i < BUTTON_COUNT; i++) {
			input.buttons[i].changed = false;
		}

		while (PeekMessage(&msgInput, window, 0, 0, PM_REMOVE)) {

			switch (msgInput.message) {
				case WM_KEYUP:
				case WM_KEYDOWN: {                                            // *Note: Same code to be executed for KEYUP and KEYDOWN messages
					u32 vk_code = (u32)msgInput.wParam;                       // vk_code tells which specific key was pressed

					// Bit-twiddling to check if last bit is 0 ie. if the key with obtained vk_code is down or not (required due to *Note) 
					bool curr_is_down = ((msgInput.lParam & (1 << 31)) == 0); 
					// Additional notes on key state management:
					// curr_is_down = false if the key went up during this frame and curr_is_down = true if the key went down during this frame
					// We won't get WM_KEYUP or WM_KEYDOWN if the key state does not change from the previous frame (or will we?)
			        // bool prev_is_down = ((msgInput.lParam & (1 << 30)) == 0);
					// Transition state: (msgInput.lParam & (1 << 31)) is 1 by default, is 0 when the key is pressed in the curr frame
					// Prev. key state: (msgInput.lParam & (1 << 30)) is 1 by default, is 0 when the key was pressed in the last frame

					switch (vk_code) {

						// ------------------- Multi-line Process_Button() Macro -----------------------------------------------------------------
						// This basically checks ((WM_KEYUP || WM_KEYDOWN) && (vk_code == vk)) ie. if vk = b key went down/up during this frame
						// Get if button b went down or up in this frame from curr_is_down and since it did go down/up in this frame, it must have changed
						#define process_button(b, vk)\
						case vk: {\
							input.buttons[b].changed = curr_is_down != input.buttons[b].is_down;\
							input.buttons[b].is_down = curr_is_down;\
						} break;

						// Processs all buttons using the macro (includes the cases for switch)
						process_button(BUTTON_UP, VK_UP);
						process_button(BUTTON_DOWN, VK_DOWN);
						process_button(BUTTON_LEFT, VK_LEFT);
						process_button(BUTTON_RIGHT, VK_RIGHT);
						process_button(BUTTON_W, 'W');
						process_button(BUTTON_S, 'S');
						process_button(BUTTON_A, 'A');
						process_button(BUTTON_D, 'D');
						process_button(BUTTON_P, 'P');
						process_button(BUTTON_ENTER, VK_RETURN);
						process_button(BUTTON_ESC, VK_ESCAPE);
					}
				} break;

				default: {
					TranslateMessage(&msgInput);
					DispatchMessage(&msgInput);
				}
			}
		}

		// ------------ (2) Simulate stuff ---------------------
		simulate_game(&input, delta_time);

		// ------------ (3) Render stuff on screen -------------
		// StretchDIBits() copies the color data for a rectangle of pixels in a DIB/JPEG/PNG image to the specified destination rectangle
		StretchDIBits(
			hdc,                        // hdc: handle to the destination device context
			0,                          // xDest: x-coordinate of the upper-left corner of the destination rectangle (logical units)
			0,                          // yDest: y-coordinate of the bottom-right corner of the destination rectangle (logical units)
			render_state.width,         // DestWidth: width of the destination rectangle (logical units)
			render_state.height,        // DestHeight: height of the of the destination rectangle (logical units)
			0,                          // xSrc: x-coordinate of the source rectangle (in pixels)
			0,                          // ySrc: y-coordinate of the source rectangle (in pixels)
			render_state.width,         // SrcWidth: width of the source rectangle (in pixels)
			render_state.height,        // SrcHeight: height of the of the source rectangle (in pixels)
			render_state.memory,        // *lpBits: pointer to the image bits which are stored as an array of bytes
			&render_state.bitmap_info,  // *lpbmi: pointer to a BITMAPINFO structure that contains information about the DIB
			DIB_RGB_COLORS,             // iUsage: whether bmiColors of BITMAPINFO struct contains explicit RGB values
			SRCCOPY                     // rop: raster-operation specifies how src pixels and dest pixels are combined to form the new image
		);                              // If the function succeeds, the return value is the number of scan lines copied

		// ----------- End of Frame - Time Delta Calculation -----------------
		// Must be inside running loop to capture end of frame time correctly

		LARGE_INTEGER frame_end_time;              // End ISO time stored using large_integer
		QueryPerformanceCounter(&frame_end_time);  // Store high resolution (<1us) time stamp in frame_end_time

		// Calculate delta time in secs using QuadPart (high res, 64 bit) of the LARGE_INTEGER struct
		// Divide the difference by secs per frame to get the delta time in seconds
		delta_time = (float)(frame_end_time.QuadPart - frame_begin_time.QuadPart) / performance_freq;
		frame_begin_time = frame_end_time;         // Curr. frame_end_time is the next frame_begin_time
	}

}
