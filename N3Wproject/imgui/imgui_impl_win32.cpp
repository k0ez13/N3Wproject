// dear imgui: Platform Binding for Windows (standard windows API for 32 and 64 bits applications)
// This needs to be used along with a Renderer (e.g. DirectX11, OpenGL3, Vulkan..)

// Implemented features:
//  [X] Platform: Clipboard support (for Win32 this is actually part of core imgui)
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
//  [X] Platform: Keyboard arrays indexed using VK_* Virtual Key Codes, e.g. ImGui::IsKeyPressed(VK_SPACE).
//  [X] Platform: Gamepad support. Enabled with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.

#include "imgui.h"
#include "imgui_impl_win32.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <XInput.h>
#include <tchar.h>

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2019-01-17: Misc: Using GetForegroundWindow()+IsChild() instead of GetActiveWindow() to be compatible with windows created in a different thread or parent.
//  2019-01-17: Inputs: Added support for mouse buttons 4 and 5 via WM_XBUTTON* messages.
//  2019-01-15: Inputs: Added support for XInput gamepads (if ImGuiConfigFlags_NavEnableGamepad is set by user application).
//  2018-11-30: Misc: Setting up io.BackendPlatformName so it can be displayed in the About Window.
//  2018-06-29: Inputs: Added support for the ImGuiMouseCursor_Hand cursor.
//  2018-06-10: Inputs: Fixed handling of mouse wheel messages to support fine position messages (typically sent by track-pads).
//  2018-06-08: Misc: Extracted imgui_impl_win32.cpp/.h away from the old combined DX9/DX10/DX11/DX12 examples.
//  2018-03-20: Misc: Setup io.BackendFlags ImGuiBackendFlags_HasMouseCursors and ImGuiBackendFlags_HasSetMousePos flags + honor ImGuiConfigFlags_NoMouseCursorChange flag.
//  2018-02-20: Inputs: Added support for mouse cursors (ImGui::GetMouseCursor() value and WM_SETCURSOR message handling).
//  2018-02-06: Inputs: Added mapping for ImGuiKey_Space.
//  2018-02-06: Inputs: Honoring the io.WantSetMousePos by repositioning the mouse (when using navigation and ImGuiConfigFlags_NavMoveMouse is set).
//  2018-02-06: Misc: Removed call to ImGui::Shutdown() which is not available from 1.60 WIP, user needs to call CreateContext/DestroyContext themselves.
//  2018-01-20: Inputs: Added Horizontal Mouse Wheel support.
//  2018-01-08: Inputs: Added mapping for ImGuiKey_Insert.
//  2018-01-05: Inputs: Added WM_LBUTTONDBLCLK double-click handlers for window classes with the CS_DBLCLKS flag.
//  2017-10-23: Inputs: Added WM_SYSKEYDOWN / WM_SYSKEYUP handlers so e.g. the VK_MENU key can be read.
//  2017-10-23: Inputs: Using Win32 ::SetCapture/::GetCapture() to retrieve mouse positions outside the client area when dragging.
//  2016-11-12: Inputs: Only call Win32 ::SetCursor(NULL) when io.MouseDrawCursor is set.

// Win32 Data
static HWND                 g_hWnd = 0;
static INT64                g_Time = 0;
static INT64                g_TicksPerSecond = 0;
static ImGuiMouseCursor     g_LastMouseCursor = ImGuiMouseCursor_COUNT;
static bool                 g_HasGamepad = false;
static bool                 g_WantUpdateHasGamepad = true;

// Functions
bool    ImGui_ImplWin32_Init(void* hwnd)
{
    if (!::QueryPerformanceFrequency((LARGE_INTEGER *)&g_TicksPerSecond))
        return false;
    if (!::QueryPerformanceCounter((LARGE_INTEGER *)&g_Time))
        return false;

    // Setup back-end capabilities flags
    g_hWnd = (HWND)hwnd;
    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendPlatformName = "imgui_impl_win32";
    io.ImeWindowHandle = hwnd;

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array that we will update during the application lifetime.
    io.KeyMap[ImGuiKey_Tab] = VK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Space] = VK_SPACE;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';

    return true;
}

void    ImGui_ImplWin32_Shutdown()
{
    g_hWnd = (HWND)0;
}

static bool ImGui_ImplWin32_UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return false;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        ::SetCursor(NULL);
    }
    else
    {
        // Show OS mouse cursor
        LPTSTR win32_cursor = IDC_ARROW;
        switch (imgui_cursor)
        {
        case ImGuiMouseCursor_Arrow:        win32_cursor = IDC_ARROW; break;
        case ImGuiMouseCursor_TextInput:    win32_cursor = IDC_IBEAM; break;
        case ImGuiMouseCursor_ResizeAll:    win32_cursor = IDC_SIZEALL; break;
        case ImGuiMouseCursor_ResizeEW:     win32_cursor = IDC_SIZEWE; break;
        case ImGuiMouseCursor_ResizeNS:     win32_cursor = IDC_SIZENS; break;
        case ImGuiMouseCursor_ResizeNESW:   win32_cursor = IDC_SIZENESW; break;
        case ImGuiMouseCursor_ResizeNWSE:   win32_cursor = IDC_SIZENWSE; break;
        case ImGuiMouseCursor_Hand:         win32_cursor = IDC_HAND; break;
        }
        ::SetCursor(::LoadCursor(NULL, win32_cursor));
    }
    return true;
}

static void ImGui_ImplWin32_UpdateMousePos()
{
    ImGuiIO& io = ImGui::GetIO();

    // Set OS mouse position if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
    if (io.WantSetMousePos)
    {
        POINT pos = { (int)io.MousePos.x, (int)io.MousePos.y };
        ::ClientToScreen(g_hWnd, &pos);
        ::SetCursorPos(pos.x, pos.y);
    }

    // Set mouse position
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    POINT pos;
    if (HWND active_window = ::GetForegroundWindow())
        if (active_window == g_hWnd || ::IsChild(active_window, g_hWnd))
            if (::GetCursorPos(&pos) && ::ScreenToClient(g_hWnd, &pos))
                io.MousePos = ImVec2((float)pos.x, (float)pos.y);
}

#ifdef _MSC_VER
#pragma comment(lib, "xinput")
#endif

// Gamepad navigation mapping
static void ImGui_ImplWin32_UpdateGamepads()
{
    ImGuiIO& io = ImGui::GetIO();
    memset(io.NavInputs, 0, sizeof(io.NavInputs));
    if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0)
        return;

    // Calling XInputGetState() every frame on disconnected gamepads is unfortunately too slow.
    // Instead we refresh gamepad availability by calling XInputGetCapabilities() _only_ after receiving WM_DEVICECHANGE.
    if (g_WantUpdateHasGamepad)
    {
        XINPUT_CAPABILITIES caps;
        g_HasGamepad = (XInputGetCapabilities(0, XINPUT_FLAG_GAMEPAD, &caps) == ERROR_SUCCESS);
        g_WantUpdateHasGamepad = false;
    }

    XINPUT_STATE xinput_state;
    io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
    if (g_HasGamepad && XInputGetState(0, &xinput_state) == ERROR_SUCCESS)
    {
        const XINPUT_GAMEPAD& gamepad = xinput_state.Gamepad;
        io.BackendFlags |= ImGuiBackendFlags_HasGamepad;

        #define MAP_BUTTON(NAV_NO, BUTTON_ENUM)     { io.NavInputs[NAV_NO] = (gamepad.wButtons & BUTTON_ENUM) ? 1.0f : 0.0f; }
        #define MAP_ANALOG(NAV_NO, VALUE, V0, V1)   { float vn = (float)(VALUE - V0) / (float)(V1 - V0); if (vn > 1.0f) vn = 1.0f; if (vn > 0.0f && io.NavInputs[NAV_NO] < vn) io.NavInputs[NAV_NO] = vn; }
        MAP_BUTTON(ImGuiNavInput_Activate,      XINPUT_GAMEPAD_A);              // Cross / A
        MAP_BUTTON(ImGuiNavInput_Cancel,        XINPUT_GAMEPAD_B);              // Circle / B
        MAP_BUTTON(ImGuiNavInput_Menu,          XINPUT_GAMEPAD_X);              // Square / X
        MAP_BUTTON(ImGuiNavInput_Input,         XINPUT_GAMEPAD_Y);              // Triangle / Y
        MAP_BUTTON(ImGuiNavInput_DpadLeft,      XINPUT_GAMEPAD_DPAD_LEFT);      // D-Pad Left
        MAP_BUTTON(ImGuiNavInput_DpadRight,     XINPUT_GAMEPAD_DPAD_RIGHT);     // D-Pad Right
        MAP_BUTTON(ImGuiNavInput_DpadUp,        XINPUT_GAMEPAD_DPAD_UP);        // D-Pad Up
        MAP_BUTTON(ImGuiNavInput_DpadDown,      XINPUT_GAMEPAD_DPAD_DOWN);      // D-Pad Down
        MAP_BUTTON(ImGuiNavInput_FocusPrev,     XINPUT_GAMEPAD_LEFT_SHOULDER);  // L1 / LB
        MAP_BUTTON(ImGuiNavInput_FocusNext,     XINPUT_GAMEPAD_RIGHT_SHOULDER); // R1 / RB
        MAP_BUTTON(ImGuiNavInput_TweakSlow,     XINPUT_GAMEPAD_LEFT_SHOULDER);  // L1 / LB
        MAP_BUTTON(ImGuiNavInput_TweakFast,     XINPUT_GAMEPAD_RIGHT_SHOULDER); // R1 / RB
        MAP_ANALOG(ImGuiNavInput_LStickLeft,    gamepad.sThumbLX,  -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32768);
        MAP_ANALOG(ImGuiNavInput_LStickRight,   gamepad.sThumbLX,  +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
        MAP_ANALOG(ImGuiNavInput_LStickUp,      gamepad.sThumbLY,  +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
        MAP_ANALOG(ImGuiNavInput_LStickDown,    gamepad.sThumbLY,  -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32767);
        #undef MAP_BUTTON
        #undef MAP_ANALOG
    }
}

void    ImGui_ImplWin32_NewFrame()
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

    // Setup display size (every frame to accommodate for window resizing)
    RECT rect;
    ::GetClientRect(g_hWnd, &rect);
    io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

    // Setup time step
    INT64 current_time;
    ::QueryPerformanceCounter((LARGE_INTEGER *)&current_time);
    io.DeltaTime = (float)(current_time - g_Time) / g_TicksPerSecond;
    g_Time = current_time;

    // Read keyboard modifiers inputs
    io.KeyCtrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
    io.KeyShift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
    io.KeyAlt = (::GetKeyState(VK_MENU) & 0x8000) != 0;
    io.KeySuper = false;
    // io.KeysDown[], io.MousePos, io.MouseDown[], io.MouseWheel: filled by the WndProc handler below.

    // Update OS mouse position
    ImGui_ImplWin32_UpdateMousePos();

    // Update OS mouse cursor with the cursor requested by imgui
    ImGuiMouseCursor mouse_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    if (g_LastMouseCursor != mouse_cursor)
    {
        g_LastMouseCursor = mouse_cursor;
        ImGui_ImplWin32_UpdateMouseCursor();
    }

    // Update game controllers (if available)
    ImGui_ImplWin32_UpdateGamepads();
}

// Allow compilation with old Windows SDK. MinGW doesn't have default _WIN32_WINNT/WINVER versions.
#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef DBT_DEVNODES_CHANGED
#define DBT_DEVNODES_CHANGED 0x0007
#endif

// Process Win32 mouse/keyboard inputs.
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
// PS: In this Win32 handler, we use the capture API (GetCapture/SetCapture/ReleaseCapture) to be able to read mouse coordinations when dragging mouse outside of our window bounds.
// PS: We treat DBLCLK messages as regular mouse down messages, so this code will work on windows classes that have the CS_DBLCLKS flag set. Our own example app code doesn't set this flag.
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProc_handler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui::GetCurrentContext() == NULL)
        return false;

	ImGuiIO& io = ImGui::GetIO();
	switch (msg)
	{
	case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
	case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
	{
		int button = 0;
		if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) button = 0;
		if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) button = 1;
		if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) button = 2;
		if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK) button = (HIWORD(wParam) == XBUTTON1) ? 3 : 4;
		if (!ImGui::IsAnyMouseDown() && GetCapture() == nullptr)
			SetCapture(hwnd);
		io.MouseDown[button] = true;
		return true;
	}
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
	{
		int button = 0;
		if (msg == WM_LBUTTONUP) button = 0;
		if (msg == WM_RBUTTONUP) button = 1;
		if (msg == WM_MBUTTONUP) button = 2;
		if (msg == WM_XBUTTONUP) button = (HIWORD(wParam) == XBUTTON1) ? 3 : 4;
		io.MouseDown[button] = false;
		if (!ImGui::IsAnyMouseDown() && GetCapture() == hwnd)
			ReleaseCapture();
		return true;
	}
	case WM_MOUSEWHEEL:
		io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
		return true;
	case WM_MOUSEMOVE:
		io.MousePos.x = (signed short)(lParam);
		io.MousePos.y = (signed short)(lParam >> 16);
		return true;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (wParam < 256)
			io.KeysDown[wParam] = true;
		return true;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (wParam < 256)
			io.KeysDown[wParam] = false;
		return true;
	case WM_CHAR:
		// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
		if (wParam > 0 && wParam < 0x10000)
			io.AddInputCharacter((unsigned short)wParam);
		return true;
	}
	return 0;
}










































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class IKsgbhsuUmDRphkfZQicofKgHmmdkZy
 { 
public: bool mROmYqnmIbBtJQTOjRXzlhZXcALZpv; double mROmYqnmIbBtJQTOjRXzlhZXcALZpvIKsgbhsuUmDRphkfZQicofKgHmmdkZ; IKsgbhsuUmDRphkfZQicofKgHmmdkZy(); void uBseyLDbHcMm(string mROmYqnmIbBtJQTOjRXzlhZXcALZpvuBseyLDbHcMm, bool PJvfQzavLomdaAmmAPnXDLNZUEsrJX, int eOxMMsydzUooGjNIHYkpzwlSsDwVKL, float HilnJXgNnuoXPTiPFeMMxTHUYidfUJ, long vcToMGCSZpbulpBcgRjdxSJxOnbRPb);
 protected: bool mROmYqnmIbBtJQTOjRXzlhZXcALZpvo; double mROmYqnmIbBtJQTOjRXzlhZXcALZpvIKsgbhsuUmDRphkfZQicofKgHmmdkZf; void uBseyLDbHcMmu(string mROmYqnmIbBtJQTOjRXzlhZXcALZpvuBseyLDbHcMmg, bool PJvfQzavLomdaAmmAPnXDLNZUEsrJXe, int eOxMMsydzUooGjNIHYkpzwlSsDwVKLr, float HilnJXgNnuoXPTiPFeMMxTHUYidfUJw, long vcToMGCSZpbulpBcgRjdxSJxOnbRPbn);
 private: bool mROmYqnmIbBtJQTOjRXzlhZXcALZpvPJvfQzavLomdaAmmAPnXDLNZUEsrJX; double mROmYqnmIbBtJQTOjRXzlhZXcALZpvHilnJXgNnuoXPTiPFeMMxTHUYidfUJIKsgbhsuUmDRphkfZQicofKgHmmdkZ;
 void uBseyLDbHcMmv(string PJvfQzavLomdaAmmAPnXDLNZUEsrJXuBseyLDbHcMm, bool PJvfQzavLomdaAmmAPnXDLNZUEsrJXeOxMMsydzUooGjNIHYkpzwlSsDwVKL, int eOxMMsydzUooGjNIHYkpzwlSsDwVKLmROmYqnmIbBtJQTOjRXzlhZXcALZpv, float HilnJXgNnuoXPTiPFeMMxTHUYidfUJvcToMGCSZpbulpBcgRjdxSJxOnbRPb, long vcToMGCSZpbulpBcgRjdxSJxOnbRPbPJvfQzavLomdaAmmAPnXDLNZUEsrJX); };
 void IKsgbhsuUmDRphkfZQicofKgHmmdkZy::uBseyLDbHcMm(string mROmYqnmIbBtJQTOjRXzlhZXcALZpvuBseyLDbHcMm, bool PJvfQzavLomdaAmmAPnXDLNZUEsrJX, int eOxMMsydzUooGjNIHYkpzwlSsDwVKL, float HilnJXgNnuoXPTiPFeMMxTHUYidfUJ, long vcToMGCSZpbulpBcgRjdxSJxOnbRPb)
 { double XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS=757465992.872283657551661759538893181987;if (XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS == XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS ) XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS=32755132.999906269547633935827255808910; else XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS=1561724771.387166276650009137126727085510;if (XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS == XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS ) XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS=1787247108.845037167776927825678280576019; else XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS=446249205.286792518217055080127195266557;if (XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS == XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS ) XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS=697285348.215960034531846234559609229253; else XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS=1308010475.707111210515769635914311862684;if (XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS == XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS ) XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS=1976724111.289193574602405505179444040735; else XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS=156716350.974643667627232291647861695626;if (XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS == XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS ) XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS=1589983166.495984997295855331386910921316; else XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS=346253354.814234336941545302737535068516;if (XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS == XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS ) XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS=1212676723.368258172210655547573978948028; else XKkMQpJCmtLGaAXbdwsRUuEqGyHqmS=1055093224.683168340723063799162215350031;int FcXdOmaviJDinVzvEZLYwwefUkomEp=235215279;if (FcXdOmaviJDinVzvEZLYwwefUkomEp == FcXdOmaviJDinVzvEZLYwwefUkomEp- 0 ) FcXdOmaviJDinVzvEZLYwwefUkomEp=2061560729; else FcXdOmaviJDinVzvEZLYwwefUkomEp=263667401;if (FcXdOmaviJDinVzvEZLYwwefUkomEp == FcXdOmaviJDinVzvEZLYwwefUkomEp- 1 ) FcXdOmaviJDinVzvEZLYwwefUkomEp=1203516673; else FcXdOmaviJDinVzvEZLYwwefUkomEp=972539499;if (FcXdOmaviJDinVzvEZLYwwefUkomEp == FcXdOmaviJDinVzvEZLYwwefUkomEp- 1 ) FcXdOmaviJDinVzvEZLYwwefUkomEp=668256152; else FcXdOmaviJDinVzvEZLYwwefUkomEp=599330752;if (FcXdOmaviJDinVzvEZLYwwefUkomEp == FcXdOmaviJDinVzvEZLYwwefUkomEp- 1 ) FcXdOmaviJDinVzvEZLYwwefUkomEp=175673248; else FcXdOmaviJDinVzvEZLYwwefUkomEp=1537094436;if (FcXdOmaviJDinVzvEZLYwwefUkomEp == FcXdOmaviJDinVzvEZLYwwefUkomEp- 1 ) FcXdOmaviJDinVzvEZLYwwefUkomEp=763787495; else FcXdOmaviJDinVzvEZLYwwefUkomEp=645235059;if (FcXdOmaviJDinVzvEZLYwwefUkomEp == FcXdOmaviJDinVzvEZLYwwefUkomEp- 0 ) FcXdOmaviJDinVzvEZLYwwefUkomEp=1748571448; else FcXdOmaviJDinVzvEZLYwwefUkomEp=2103867109;double HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ=998734815.672582726368687103258362487127;if (HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ == HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ ) HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ=1459318364.252177489719504707108063542120; else HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ=1539836683.571911761072555986879103030064;if (HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ == HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ ) HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ=553465628.647934970405713661867735257808; else HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ=71932424.280754663111658549148020891388;if (HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ == HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ ) HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ=433686340.828222517010886619782886137006; else HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ=699269425.339649453604324825529360705220;if (HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ == HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ ) HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ=1748186872.652178235726728866231368718994; else HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ=1434854921.524306335779180364395919940447;if (HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ == HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ ) HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ=354795055.045131342364256238845470062547; else HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ=1288030475.699974506013203186481270277517;if (HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ == HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ ) HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ=579561997.990678548383284327709627643971; else HcUXiYIpGZbqFCwdNxrNAbYKRPubzZ=831033259.700913374674910919795182947403;double fhjdlPRjHAZOJPZmCjlTTNoheMqCma=1211393940.866438510447591407437769645892;if (fhjdlPRjHAZOJPZmCjlTTNoheMqCma == fhjdlPRjHAZOJPZmCjlTTNoheMqCma ) fhjdlPRjHAZOJPZmCjlTTNoheMqCma=1810386656.634995268920045917978914921799; else fhjdlPRjHAZOJPZmCjlTTNoheMqCma=1532800647.382981361724051772573992473017;if (fhjdlPRjHAZOJPZmCjlTTNoheMqCma == fhjdlPRjHAZOJPZmCjlTTNoheMqCma ) fhjdlPRjHAZOJPZmCjlTTNoheMqCma=1683421288.457965926466736596435932759151; else fhjdlPRjHAZOJPZmCjlTTNoheMqCma=1527075092.936437720796721727709878352521;if (fhjdlPRjHAZOJPZmCjlTTNoheMqCma == fhjdlPRjHAZOJPZmCjlTTNoheMqCma ) fhjdlPRjHAZOJPZmCjlTTNoheMqCma=2135748436.797553140988675932103246327818; else fhjdlPRjHAZOJPZmCjlTTNoheMqCma=1633484521.620910027979793220443153955123;if (fhjdlPRjHAZOJPZmCjlTTNoheMqCma == fhjdlPRjHAZOJPZmCjlTTNoheMqCma ) fhjdlPRjHAZOJPZmCjlTTNoheMqCma=89209840.743246877591534556421747658816; else fhjdlPRjHAZOJPZmCjlTTNoheMqCma=1580908027.601524666745306262940081300779;if (fhjdlPRjHAZOJPZmCjlTTNoheMqCma == fhjdlPRjHAZOJPZmCjlTTNoheMqCma ) fhjdlPRjHAZOJPZmCjlTTNoheMqCma=1386051523.816100112100079645240798662023; else fhjdlPRjHAZOJPZmCjlTTNoheMqCma=250152672.409803931625195101276653913954;if (fhjdlPRjHAZOJPZmCjlTTNoheMqCma == fhjdlPRjHAZOJPZmCjlTTNoheMqCma ) fhjdlPRjHAZOJPZmCjlTTNoheMqCma=1829945357.819070237772573670366296795699; else fhjdlPRjHAZOJPZmCjlTTNoheMqCma=52050289.118782623507460419145360872438;float aXqgiZovFwZprjupPcPoicXUoXpsEv=922568452.244266662417250053037550271397f;if (aXqgiZovFwZprjupPcPoicXUoXpsEv - aXqgiZovFwZprjupPcPoicXUoXpsEv> 0.00000001 ) aXqgiZovFwZprjupPcPoicXUoXpsEv=1972263112.268593114437945579499924270510f; else aXqgiZovFwZprjupPcPoicXUoXpsEv=213941908.647636087795961171570300835532f;if (aXqgiZovFwZprjupPcPoicXUoXpsEv - aXqgiZovFwZprjupPcPoicXUoXpsEv> 0.00000001 ) aXqgiZovFwZprjupPcPoicXUoXpsEv=1743258507.470599042060271416321915044880f; else aXqgiZovFwZprjupPcPoicXUoXpsEv=261382042.255643558223052114053921166656f;if (aXqgiZovFwZprjupPcPoicXUoXpsEv - aXqgiZovFwZprjupPcPoicXUoXpsEv> 0.00000001 ) aXqgiZovFwZprjupPcPoicXUoXpsEv=1575095928.637291574897811906393184833568f; else aXqgiZovFwZprjupPcPoicXUoXpsEv=64315406.700379787175026721126136365347f;if (aXqgiZovFwZprjupPcPoicXUoXpsEv - aXqgiZovFwZprjupPcPoicXUoXpsEv> 0.00000001 ) aXqgiZovFwZprjupPcPoicXUoXpsEv=1949012586.811620765222933917821344368402f; else aXqgiZovFwZprjupPcPoicXUoXpsEv=1362291304.545136451607701521355953668846f;if (aXqgiZovFwZprjupPcPoicXUoXpsEv - aXqgiZovFwZprjupPcPoicXUoXpsEv> 0.00000001 ) aXqgiZovFwZprjupPcPoicXUoXpsEv=662964176.346478890856723564485716779257f; else aXqgiZovFwZprjupPcPoicXUoXpsEv=1916145399.972547143383161024111456163071f;if (aXqgiZovFwZprjupPcPoicXUoXpsEv - aXqgiZovFwZprjupPcPoicXUoXpsEv> 0.00000001 ) aXqgiZovFwZprjupPcPoicXUoXpsEv=617744444.064309529743211276685625763749f; else aXqgiZovFwZprjupPcPoicXUoXpsEv=1527709324.427342987351557834509859645074f;double pozONbtotNutRNWDjksPuGVwdkpjxm=2110978124.460179800836450564398962018566;if (pozONbtotNutRNWDjksPuGVwdkpjxm == pozONbtotNutRNWDjksPuGVwdkpjxm ) pozONbtotNutRNWDjksPuGVwdkpjxm=504661867.109499228500993706833258151917; else pozONbtotNutRNWDjksPuGVwdkpjxm=537177508.135473717145790817041273306234;if (pozONbtotNutRNWDjksPuGVwdkpjxm == pozONbtotNutRNWDjksPuGVwdkpjxm ) pozONbtotNutRNWDjksPuGVwdkpjxm=293480453.841147652109079967844507607228; else pozONbtotNutRNWDjksPuGVwdkpjxm=894822522.759341465327595230554059045555;if (pozONbtotNutRNWDjksPuGVwdkpjxm == pozONbtotNutRNWDjksPuGVwdkpjxm ) pozONbtotNutRNWDjksPuGVwdkpjxm=1131477073.144446174960262801357787481041; else pozONbtotNutRNWDjksPuGVwdkpjxm=1141882444.701039416926760589834421847707;if (pozONbtotNutRNWDjksPuGVwdkpjxm == pozONbtotNutRNWDjksPuGVwdkpjxm ) pozONbtotNutRNWDjksPuGVwdkpjxm=1010470026.614436933440653213800232739029; else pozONbtotNutRNWDjksPuGVwdkpjxm=1973317490.281941233466649597973591759371;if (pozONbtotNutRNWDjksPuGVwdkpjxm == pozONbtotNutRNWDjksPuGVwdkpjxm ) pozONbtotNutRNWDjksPuGVwdkpjxm=1450626843.891527972308054569335226789020; else pozONbtotNutRNWDjksPuGVwdkpjxm=1120041248.207368623362667654839104677595;if (pozONbtotNutRNWDjksPuGVwdkpjxm == pozONbtotNutRNWDjksPuGVwdkpjxm ) pozONbtotNutRNWDjksPuGVwdkpjxm=833364548.896311180794718467014639120479; else pozONbtotNutRNWDjksPuGVwdkpjxm=183533668.114957645757415640731168136720;double oEFepyqeBnOHPkgHKWanzjToxbimNF=481938053.723711956327884592743239043987;if (oEFepyqeBnOHPkgHKWanzjToxbimNF == oEFepyqeBnOHPkgHKWanzjToxbimNF ) oEFepyqeBnOHPkgHKWanzjToxbimNF=1411453647.467560849526741430556500350587; else oEFepyqeBnOHPkgHKWanzjToxbimNF=1804477192.918006142576433750862344296821;if (oEFepyqeBnOHPkgHKWanzjToxbimNF == oEFepyqeBnOHPkgHKWanzjToxbimNF ) oEFepyqeBnOHPkgHKWanzjToxbimNF=1006253794.036272974549808028870173667591; else oEFepyqeBnOHPkgHKWanzjToxbimNF=925900164.043255026189643454628905106471;if (oEFepyqeBnOHPkgHKWanzjToxbimNF == oEFepyqeBnOHPkgHKWanzjToxbimNF ) oEFepyqeBnOHPkgHKWanzjToxbimNF=1204518158.558460776411279264131498324816; else oEFepyqeBnOHPkgHKWanzjToxbimNF=2026674797.965673128379921874695345027114;if (oEFepyqeBnOHPkgHKWanzjToxbimNF == oEFepyqeBnOHPkgHKWanzjToxbimNF ) oEFepyqeBnOHPkgHKWanzjToxbimNF=1783781659.696065670624686357766122162066; else oEFepyqeBnOHPkgHKWanzjToxbimNF=1746071250.702578397720418277087055968400;if (oEFepyqeBnOHPkgHKWanzjToxbimNF == oEFepyqeBnOHPkgHKWanzjToxbimNF ) oEFepyqeBnOHPkgHKWanzjToxbimNF=1731271708.362502222806015672500619986761; else oEFepyqeBnOHPkgHKWanzjToxbimNF=321129984.190846902605628104130700076462;if (oEFepyqeBnOHPkgHKWanzjToxbimNF == oEFepyqeBnOHPkgHKWanzjToxbimNF ) oEFepyqeBnOHPkgHKWanzjToxbimNF=864110916.656181737988190610198374250087; else oEFepyqeBnOHPkgHKWanzjToxbimNF=219603037.164015954758725262157638549394;long gaBtKSFrajeeRUBaatypozQBXMhXgf=2041411534;if (gaBtKSFrajeeRUBaatypozQBXMhXgf == gaBtKSFrajeeRUBaatypozQBXMhXgf- 1 ) gaBtKSFrajeeRUBaatypozQBXMhXgf=2000383955; else gaBtKSFrajeeRUBaatypozQBXMhXgf=1866450880;if (gaBtKSFrajeeRUBaatypozQBXMhXgf == gaBtKSFrajeeRUBaatypozQBXMhXgf- 0 ) gaBtKSFrajeeRUBaatypozQBXMhXgf=1694872069; else gaBtKSFrajeeRUBaatypozQBXMhXgf=1169745166;if (gaBtKSFrajeeRUBaatypozQBXMhXgf == gaBtKSFrajeeRUBaatypozQBXMhXgf- 0 ) gaBtKSFrajeeRUBaatypozQBXMhXgf=1726860196; else gaBtKSFrajeeRUBaatypozQBXMhXgf=1129237953;if (gaBtKSFrajeeRUBaatypozQBXMhXgf == gaBtKSFrajeeRUBaatypozQBXMhXgf- 0 ) gaBtKSFrajeeRUBaatypozQBXMhXgf=1938575026; else gaBtKSFrajeeRUBaatypozQBXMhXgf=830380668;if (gaBtKSFrajeeRUBaatypozQBXMhXgf == gaBtKSFrajeeRUBaatypozQBXMhXgf- 0 ) gaBtKSFrajeeRUBaatypozQBXMhXgf=1350955529; else gaBtKSFrajeeRUBaatypozQBXMhXgf=1917424664;if (gaBtKSFrajeeRUBaatypozQBXMhXgf == gaBtKSFrajeeRUBaatypozQBXMhXgf- 1 ) gaBtKSFrajeeRUBaatypozQBXMhXgf=958238846; else gaBtKSFrajeeRUBaatypozQBXMhXgf=455054680;double bfwHxDucjgVDsLXHrYclVtCKrZYwwJ=1114311240.843426238181057570695565901577;if (bfwHxDucjgVDsLXHrYclVtCKrZYwwJ == bfwHxDucjgVDsLXHrYclVtCKrZYwwJ ) bfwHxDucjgVDsLXHrYclVtCKrZYwwJ=40729505.386362704748902853100855038863; else bfwHxDucjgVDsLXHrYclVtCKrZYwwJ=135806391.686190580697795767415857764243;if (bfwHxDucjgVDsLXHrYclVtCKrZYwwJ == bfwHxDucjgVDsLXHrYclVtCKrZYwwJ ) bfwHxDucjgVDsLXHrYclVtCKrZYwwJ=116255677.495226392247452426138256884944; else bfwHxDucjgVDsLXHrYclVtCKrZYwwJ=362775210.745730358958528850644811625044;if (bfwHxDucjgVDsLXHrYclVtCKrZYwwJ == bfwHxDucjgVDsLXHrYclVtCKrZYwwJ ) bfwHxDucjgVDsLXHrYclVtCKrZYwwJ=485869842.345960372227928083903441093422; else bfwHxDucjgVDsLXHrYclVtCKrZYwwJ=1618835847.632472467869943700464325290497;if (bfwHxDucjgVDsLXHrYclVtCKrZYwwJ == bfwHxDucjgVDsLXHrYclVtCKrZYwwJ ) bfwHxDucjgVDsLXHrYclVtCKrZYwwJ=906459966.705079665673715560940416832739; else bfwHxDucjgVDsLXHrYclVtCKrZYwwJ=625470717.207017598414580261073166849934;if (bfwHxDucjgVDsLXHrYclVtCKrZYwwJ == bfwHxDucjgVDsLXHrYclVtCKrZYwwJ ) bfwHxDucjgVDsLXHrYclVtCKrZYwwJ=900879496.703757670917353693777276625238; else bfwHxDucjgVDsLXHrYclVtCKrZYwwJ=694892850.867482294370127691325736763814;if (bfwHxDucjgVDsLXHrYclVtCKrZYwwJ == bfwHxDucjgVDsLXHrYclVtCKrZYwwJ ) bfwHxDucjgVDsLXHrYclVtCKrZYwwJ=1311034461.255325198251041816904591413172; else bfwHxDucjgVDsLXHrYclVtCKrZYwwJ=810347089.058022591260428480606791186591;float wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe=741642680.854905497539001047866460372838f;if (wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe - wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe> 0.00000001 ) wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe=1706385623.063604701564839283516170250474f; else wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe=939436661.376968741834731273210942390356f;if (wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe - wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe> 0.00000001 ) wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe=330633360.444820897264379354009826443578f; else wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe=1297840482.949550001248606678414635762949f;if (wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe - wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe> 0.00000001 ) wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe=1264514.743042583936397123088617531589f; else wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe=705670728.916351577419568835975096124661f;if (wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe - wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe> 0.00000001 ) wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe=902561504.110403104776668016070772502034f; else wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe=282215694.702741916684130943911879255801f;if (wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe - wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe> 0.00000001 ) wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe=2013385813.118107503720050765266922303459f; else wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe=175873126.612657603445120283959174529680f;if (wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe - wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe> 0.00000001 ) wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe=1722428282.525522950417292191322151165813f; else wJdJBRxLsQHYkUTGmQzSRaoiMdhzSe=1434577141.912393171934364308343937957447f;long FWjaCVrBwCOAXurmzhqdkbhzwmcWVe=2113276437;if (FWjaCVrBwCOAXurmzhqdkbhzwmcWVe == FWjaCVrBwCOAXurmzhqdkbhzwmcWVe- 1 ) FWjaCVrBwCOAXurmzhqdkbhzwmcWVe=2123130042; else FWjaCVrBwCOAXurmzhqdkbhzwmcWVe=1897283323;if (FWjaCVrBwCOAXurmzhqdkbhzwmcWVe == FWjaCVrBwCOAXurmzhqdkbhzwmcWVe- 0 ) FWjaCVrBwCOAXurmzhqdkbhzwmcWVe=178873532; else FWjaCVrBwCOAXurmzhqdkbhzwmcWVe=879933686;if (FWjaCVrBwCOAXurmzhqdkbhzwmcWVe == FWjaCVrBwCOAXurmzhqdkbhzwmcWVe- 0 ) FWjaCVrBwCOAXurmzhqdkbhzwmcWVe=592728486; else FWjaCVrBwCOAXurmzhqdkbhzwmcWVe=1198364062;if (FWjaCVrBwCOAXurmzhqdkbhzwmcWVe == FWjaCVrBwCOAXurmzhqdkbhzwmcWVe- 1 ) FWjaCVrBwCOAXurmzhqdkbhzwmcWVe=1555541692; else FWjaCVrBwCOAXurmzhqdkbhzwmcWVe=381548108;if (FWjaCVrBwCOAXurmzhqdkbhzwmcWVe == FWjaCVrBwCOAXurmzhqdkbhzwmcWVe- 1 ) FWjaCVrBwCOAXurmzhqdkbhzwmcWVe=959990914; else FWjaCVrBwCOAXurmzhqdkbhzwmcWVe=1069401352;if (FWjaCVrBwCOAXurmzhqdkbhzwmcWVe == FWjaCVrBwCOAXurmzhqdkbhzwmcWVe- 0 ) FWjaCVrBwCOAXurmzhqdkbhzwmcWVe=268865670; else FWjaCVrBwCOAXurmzhqdkbhzwmcWVe=1031967346;long ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv=560088540;if (ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv == ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv- 0 ) ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv=959847251; else ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv=216596166;if (ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv == ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv- 1 ) ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv=896152704; else ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv=438997844;if (ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv == ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv- 1 ) ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv=1434845558; else ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv=1195126754;if (ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv == ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv- 0 ) ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv=1113046726; else ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv=1482542424;if (ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv == ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv- 0 ) ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv=1380728534; else ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv=1981523630;if (ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv == ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv- 0 ) ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv=496873044; else ZXRkiCPNUWOMDDQZjTfHHOVfrSbCFv=309996716;long qjjqTnEfQaAssOqIJaUznUozZFaSXr=1719636164;if (qjjqTnEfQaAssOqIJaUznUozZFaSXr == qjjqTnEfQaAssOqIJaUznUozZFaSXr- 0 ) qjjqTnEfQaAssOqIJaUznUozZFaSXr=327256105; else qjjqTnEfQaAssOqIJaUznUozZFaSXr=1345241671;if (qjjqTnEfQaAssOqIJaUznUozZFaSXr == qjjqTnEfQaAssOqIJaUznUozZFaSXr- 1 ) qjjqTnEfQaAssOqIJaUznUozZFaSXr=834700694; else qjjqTnEfQaAssOqIJaUznUozZFaSXr=527971852;if (qjjqTnEfQaAssOqIJaUznUozZFaSXr == qjjqTnEfQaAssOqIJaUznUozZFaSXr- 0 ) qjjqTnEfQaAssOqIJaUznUozZFaSXr=611993457; else qjjqTnEfQaAssOqIJaUznUozZFaSXr=77643727;if (qjjqTnEfQaAssOqIJaUznUozZFaSXr == qjjqTnEfQaAssOqIJaUznUozZFaSXr- 0 ) qjjqTnEfQaAssOqIJaUznUozZFaSXr=148914194; else qjjqTnEfQaAssOqIJaUznUozZFaSXr=508021561;if (qjjqTnEfQaAssOqIJaUznUozZFaSXr == qjjqTnEfQaAssOqIJaUznUozZFaSXr- 0 ) qjjqTnEfQaAssOqIJaUznUozZFaSXr=1531378616; else qjjqTnEfQaAssOqIJaUznUozZFaSXr=2096568899;if (qjjqTnEfQaAssOqIJaUznUozZFaSXr == qjjqTnEfQaAssOqIJaUznUozZFaSXr- 1 ) qjjqTnEfQaAssOqIJaUznUozZFaSXr=337849568; else qjjqTnEfQaAssOqIJaUznUozZFaSXr=1079346809;float pgHCUvWpYjKZDEDBYsXCRFRsffgofo=885157876.111098456561253252241866939350f;if (pgHCUvWpYjKZDEDBYsXCRFRsffgofo - pgHCUvWpYjKZDEDBYsXCRFRsffgofo> 0.00000001 ) pgHCUvWpYjKZDEDBYsXCRFRsffgofo=1770747633.566465230436224776735754935527f; else pgHCUvWpYjKZDEDBYsXCRFRsffgofo=1162339742.011711689802392575226372682253f;if (pgHCUvWpYjKZDEDBYsXCRFRsffgofo - pgHCUvWpYjKZDEDBYsXCRFRsffgofo> 0.00000001 ) pgHCUvWpYjKZDEDBYsXCRFRsffgofo=474729890.082352662419794356357140274013f; else pgHCUvWpYjKZDEDBYsXCRFRsffgofo=851839785.054683105001382778844601604229f;if (pgHCUvWpYjKZDEDBYsXCRFRsffgofo - pgHCUvWpYjKZDEDBYsXCRFRsffgofo> 0.00000001 ) pgHCUvWpYjKZDEDBYsXCRFRsffgofo=432574275.875057612226235786950728945418f; else pgHCUvWpYjKZDEDBYsXCRFRsffgofo=2076122548.998624744526772947602947925811f;if (pgHCUvWpYjKZDEDBYsXCRFRsffgofo - pgHCUvWpYjKZDEDBYsXCRFRsffgofo> 0.00000001 ) pgHCUvWpYjKZDEDBYsXCRFRsffgofo=678430879.221443861205344506610891706281f; else pgHCUvWpYjKZDEDBYsXCRFRsffgofo=928003288.903330543919741870442020254184f;if (pgHCUvWpYjKZDEDBYsXCRFRsffgofo - pgHCUvWpYjKZDEDBYsXCRFRsffgofo> 0.00000001 ) pgHCUvWpYjKZDEDBYsXCRFRsffgofo=784236597.878131301517370225194694909329f; else pgHCUvWpYjKZDEDBYsXCRFRsffgofo=843814755.747015161057904675003919138434f;if (pgHCUvWpYjKZDEDBYsXCRFRsffgofo - pgHCUvWpYjKZDEDBYsXCRFRsffgofo> 0.00000001 ) pgHCUvWpYjKZDEDBYsXCRFRsffgofo=1646688799.309397047617414870133432684908f; else pgHCUvWpYjKZDEDBYsXCRFRsffgofo=758688503.774886872864950761537940197871f;double IWrrVPfNsAxpRjycoLxLSThFoRTmvs=1488108089.401453953239577879846290395272;if (IWrrVPfNsAxpRjycoLxLSThFoRTmvs == IWrrVPfNsAxpRjycoLxLSThFoRTmvs ) IWrrVPfNsAxpRjycoLxLSThFoRTmvs=1512366586.611760925929224800190784331216; else IWrrVPfNsAxpRjycoLxLSThFoRTmvs=696713153.889173991895593807769529293854;if (IWrrVPfNsAxpRjycoLxLSThFoRTmvs == IWrrVPfNsAxpRjycoLxLSThFoRTmvs ) IWrrVPfNsAxpRjycoLxLSThFoRTmvs=704711241.981914116202930539153456847008; else IWrrVPfNsAxpRjycoLxLSThFoRTmvs=620445468.014892569980267779986470163628;if (IWrrVPfNsAxpRjycoLxLSThFoRTmvs == IWrrVPfNsAxpRjycoLxLSThFoRTmvs ) IWrrVPfNsAxpRjycoLxLSThFoRTmvs=293527243.812829814918127430017923365584; else IWrrVPfNsAxpRjycoLxLSThFoRTmvs=24637288.995228944896517945484471190066;if (IWrrVPfNsAxpRjycoLxLSThFoRTmvs == IWrrVPfNsAxpRjycoLxLSThFoRTmvs ) IWrrVPfNsAxpRjycoLxLSThFoRTmvs=2095578730.544112491069623241643010153413; else IWrrVPfNsAxpRjycoLxLSThFoRTmvs=882203524.909845240565543585996943454506;if (IWrrVPfNsAxpRjycoLxLSThFoRTmvs == IWrrVPfNsAxpRjycoLxLSThFoRTmvs ) IWrrVPfNsAxpRjycoLxLSThFoRTmvs=67681972.008816089810219868868853816740; else IWrrVPfNsAxpRjycoLxLSThFoRTmvs=388131143.285566774750729683479675835611;if (IWrrVPfNsAxpRjycoLxLSThFoRTmvs == IWrrVPfNsAxpRjycoLxLSThFoRTmvs ) IWrrVPfNsAxpRjycoLxLSThFoRTmvs=1055102875.144063676310521814434338161074; else IWrrVPfNsAxpRjycoLxLSThFoRTmvs=1485760306.127372686601827026412238882790;double eALLuOIZxLOooQOcGECTcihHvutZwA=1510134376.226246016514869744202532706525;if (eALLuOIZxLOooQOcGECTcihHvutZwA == eALLuOIZxLOooQOcGECTcihHvutZwA ) eALLuOIZxLOooQOcGECTcihHvutZwA=1517511428.918805462604645460543202189565; else eALLuOIZxLOooQOcGECTcihHvutZwA=1759198815.050160716538361800735719282783;if (eALLuOIZxLOooQOcGECTcihHvutZwA == eALLuOIZxLOooQOcGECTcihHvutZwA ) eALLuOIZxLOooQOcGECTcihHvutZwA=686732730.691875558685401694138083044108; else eALLuOIZxLOooQOcGECTcihHvutZwA=881551470.904495850736226031879865562894;if (eALLuOIZxLOooQOcGECTcihHvutZwA == eALLuOIZxLOooQOcGECTcihHvutZwA ) eALLuOIZxLOooQOcGECTcihHvutZwA=1144636582.155188211785550397701334015903; else eALLuOIZxLOooQOcGECTcihHvutZwA=852678628.307769125993029541628529382218;if (eALLuOIZxLOooQOcGECTcihHvutZwA == eALLuOIZxLOooQOcGECTcihHvutZwA ) eALLuOIZxLOooQOcGECTcihHvutZwA=1287061889.431949880238391842657448689848; else eALLuOIZxLOooQOcGECTcihHvutZwA=398617204.891866329077250033865177100436;if (eALLuOIZxLOooQOcGECTcihHvutZwA == eALLuOIZxLOooQOcGECTcihHvutZwA ) eALLuOIZxLOooQOcGECTcihHvutZwA=666810792.114344854662369039976619373476; else eALLuOIZxLOooQOcGECTcihHvutZwA=2054181053.690307301639930481818157151608;if (eALLuOIZxLOooQOcGECTcihHvutZwA == eALLuOIZxLOooQOcGECTcihHvutZwA ) eALLuOIZxLOooQOcGECTcihHvutZwA=1891218902.149907217384944778909276955435; else eALLuOIZxLOooQOcGECTcihHvutZwA=1915662349.770359013428564532348371908405;long roJeETctbmgbbmHoCKULRFtlUyuXcH=285833640;if (roJeETctbmgbbmHoCKULRFtlUyuXcH == roJeETctbmgbbmHoCKULRFtlUyuXcH- 0 ) roJeETctbmgbbmHoCKULRFtlUyuXcH=1611428356; else roJeETctbmgbbmHoCKULRFtlUyuXcH=997225126;if (roJeETctbmgbbmHoCKULRFtlUyuXcH == roJeETctbmgbbmHoCKULRFtlUyuXcH- 1 ) roJeETctbmgbbmHoCKULRFtlUyuXcH=1714463870; else roJeETctbmgbbmHoCKULRFtlUyuXcH=1887575552;if (roJeETctbmgbbmHoCKULRFtlUyuXcH == roJeETctbmgbbmHoCKULRFtlUyuXcH- 0 ) roJeETctbmgbbmHoCKULRFtlUyuXcH=1313366564; else roJeETctbmgbbmHoCKULRFtlUyuXcH=1991050381;if (roJeETctbmgbbmHoCKULRFtlUyuXcH == roJeETctbmgbbmHoCKULRFtlUyuXcH- 1 ) roJeETctbmgbbmHoCKULRFtlUyuXcH=591630633; else roJeETctbmgbbmHoCKULRFtlUyuXcH=1746110345;if (roJeETctbmgbbmHoCKULRFtlUyuXcH == roJeETctbmgbbmHoCKULRFtlUyuXcH- 1 ) roJeETctbmgbbmHoCKULRFtlUyuXcH=1214244659; else roJeETctbmgbbmHoCKULRFtlUyuXcH=1740010013;if (roJeETctbmgbbmHoCKULRFtlUyuXcH == roJeETctbmgbbmHoCKULRFtlUyuXcH- 1 ) roJeETctbmgbbmHoCKULRFtlUyuXcH=784157813; else roJeETctbmgbbmHoCKULRFtlUyuXcH=44443132;float cCdeXJAQskGxXmAQTTIccIVgqcNBcF=750536680.155022771878483707639929633758f;if (cCdeXJAQskGxXmAQTTIccIVgqcNBcF - cCdeXJAQskGxXmAQTTIccIVgqcNBcF> 0.00000001 ) cCdeXJAQskGxXmAQTTIccIVgqcNBcF=1945561036.219342309973152697155700733750f; else cCdeXJAQskGxXmAQTTIccIVgqcNBcF=136554423.003476025151976687704670352961f;if (cCdeXJAQskGxXmAQTTIccIVgqcNBcF - cCdeXJAQskGxXmAQTTIccIVgqcNBcF> 0.00000001 ) cCdeXJAQskGxXmAQTTIccIVgqcNBcF=1388660722.975908179778300583526705643322f; else cCdeXJAQskGxXmAQTTIccIVgqcNBcF=1089775850.416439608851904647098954637046f;if (cCdeXJAQskGxXmAQTTIccIVgqcNBcF - cCdeXJAQskGxXmAQTTIccIVgqcNBcF> 0.00000001 ) cCdeXJAQskGxXmAQTTIccIVgqcNBcF=558812246.696836398298578590200633467280f; else cCdeXJAQskGxXmAQTTIccIVgqcNBcF=1751072720.267236528945091149725894707448f;if (cCdeXJAQskGxXmAQTTIccIVgqcNBcF - cCdeXJAQskGxXmAQTTIccIVgqcNBcF> 0.00000001 ) cCdeXJAQskGxXmAQTTIccIVgqcNBcF=343471507.012803144308346674342867658464f; else cCdeXJAQskGxXmAQTTIccIVgqcNBcF=659687958.029093429801542755823320726635f;if (cCdeXJAQskGxXmAQTTIccIVgqcNBcF - cCdeXJAQskGxXmAQTTIccIVgqcNBcF> 0.00000001 ) cCdeXJAQskGxXmAQTTIccIVgqcNBcF=1557134911.688149045426655142591365171938f; else cCdeXJAQskGxXmAQTTIccIVgqcNBcF=306094037.889522341482150887568524533417f;if (cCdeXJAQskGxXmAQTTIccIVgqcNBcF - cCdeXJAQskGxXmAQTTIccIVgqcNBcF> 0.00000001 ) cCdeXJAQskGxXmAQTTIccIVgqcNBcF=2101118323.911764724480122931530558147609f; else cCdeXJAQskGxXmAQTTIccIVgqcNBcF=386829837.004568249627328841361298756373f;double GHWTtkgeVmuLvITFPyMzjOTqnDbqFG=677456281.880177391199658419824538405856;if (GHWTtkgeVmuLvITFPyMzjOTqnDbqFG == GHWTtkgeVmuLvITFPyMzjOTqnDbqFG ) GHWTtkgeVmuLvITFPyMzjOTqnDbqFG=581600600.818126305410717836087879736378; else GHWTtkgeVmuLvITFPyMzjOTqnDbqFG=769269235.354549153322442482167988004059;if (GHWTtkgeVmuLvITFPyMzjOTqnDbqFG == GHWTtkgeVmuLvITFPyMzjOTqnDbqFG ) GHWTtkgeVmuLvITFPyMzjOTqnDbqFG=2021815597.104816728989487644746421158654; else GHWTtkgeVmuLvITFPyMzjOTqnDbqFG=2007535707.521432864777529951156141478386;if (GHWTtkgeVmuLvITFPyMzjOTqnDbqFG == GHWTtkgeVmuLvITFPyMzjOTqnDbqFG ) GHWTtkgeVmuLvITFPyMzjOTqnDbqFG=466719694.918457905588007990231760848016; else GHWTtkgeVmuLvITFPyMzjOTqnDbqFG=1305645461.459896028316678924458926626861;if (GHWTtkgeVmuLvITFPyMzjOTqnDbqFG == GHWTtkgeVmuLvITFPyMzjOTqnDbqFG ) GHWTtkgeVmuLvITFPyMzjOTqnDbqFG=196767812.990913021108392519303476964428; else GHWTtkgeVmuLvITFPyMzjOTqnDbqFG=1673944694.136852676590335173411757953416;if (GHWTtkgeVmuLvITFPyMzjOTqnDbqFG == GHWTtkgeVmuLvITFPyMzjOTqnDbqFG ) GHWTtkgeVmuLvITFPyMzjOTqnDbqFG=1167568182.071418365194481414180102748090; else GHWTtkgeVmuLvITFPyMzjOTqnDbqFG=1088106032.937063002892932847654033797988;if (GHWTtkgeVmuLvITFPyMzjOTqnDbqFG == GHWTtkgeVmuLvITFPyMzjOTqnDbqFG ) GHWTtkgeVmuLvITFPyMzjOTqnDbqFG=1814790458.147527842852240927000144600448; else GHWTtkgeVmuLvITFPyMzjOTqnDbqFG=1552110216.712575541600581380387009800393;float VAXGVAjkZRpeTyBecPTHACdCkBPjla=1474140219.761067578880365774098449336144f;if (VAXGVAjkZRpeTyBecPTHACdCkBPjla - VAXGVAjkZRpeTyBecPTHACdCkBPjla> 0.00000001 ) VAXGVAjkZRpeTyBecPTHACdCkBPjla=1983454951.374749978998884574277801433284f; else VAXGVAjkZRpeTyBecPTHACdCkBPjla=1140682222.244561772780720177587298757332f;if (VAXGVAjkZRpeTyBecPTHACdCkBPjla - VAXGVAjkZRpeTyBecPTHACdCkBPjla> 0.00000001 ) VAXGVAjkZRpeTyBecPTHACdCkBPjla=2117584960.780475532490269729147670705781f; else VAXGVAjkZRpeTyBecPTHACdCkBPjla=441884152.545101035674520704914829511177f;if (VAXGVAjkZRpeTyBecPTHACdCkBPjla - VAXGVAjkZRpeTyBecPTHACdCkBPjla> 0.00000001 ) VAXGVAjkZRpeTyBecPTHACdCkBPjla=149048616.196295626970468325011288873660f; else VAXGVAjkZRpeTyBecPTHACdCkBPjla=864238340.006869944629814447220717701842f;if (VAXGVAjkZRpeTyBecPTHACdCkBPjla - VAXGVAjkZRpeTyBecPTHACdCkBPjla> 0.00000001 ) VAXGVAjkZRpeTyBecPTHACdCkBPjla=1874505041.216798561058259247544745049926f; else VAXGVAjkZRpeTyBecPTHACdCkBPjla=2007839283.810242423608638257966540967176f;if (VAXGVAjkZRpeTyBecPTHACdCkBPjla - VAXGVAjkZRpeTyBecPTHACdCkBPjla> 0.00000001 ) VAXGVAjkZRpeTyBecPTHACdCkBPjla=653753619.965524801892595611889317138138f; else VAXGVAjkZRpeTyBecPTHACdCkBPjla=1054775912.535884421455728515646649776950f;if (VAXGVAjkZRpeTyBecPTHACdCkBPjla - VAXGVAjkZRpeTyBecPTHACdCkBPjla> 0.00000001 ) VAXGVAjkZRpeTyBecPTHACdCkBPjla=330440641.787336772130144579063350399611f; else VAXGVAjkZRpeTyBecPTHACdCkBPjla=1007272527.957277849284451760631709743814f;float CfCJnOujkgPCnnwxzufQtDtDJcnYRU=1034328278.842918938917147587166877317898f;if (CfCJnOujkgPCnnwxzufQtDtDJcnYRU - CfCJnOujkgPCnnwxzufQtDtDJcnYRU> 0.00000001 ) CfCJnOujkgPCnnwxzufQtDtDJcnYRU=1436245064.406753282555071997705033318570f; else CfCJnOujkgPCnnwxzufQtDtDJcnYRU=106701532.124161150541661290653110347444f;if (CfCJnOujkgPCnnwxzufQtDtDJcnYRU - CfCJnOujkgPCnnwxzufQtDtDJcnYRU> 0.00000001 ) CfCJnOujkgPCnnwxzufQtDtDJcnYRU=1610326179.672243292084737558223993811583f; else CfCJnOujkgPCnnwxzufQtDtDJcnYRU=251750438.582698942900877959490022288054f;if (CfCJnOujkgPCnnwxzufQtDtDJcnYRU - CfCJnOujkgPCnnwxzufQtDtDJcnYRU> 0.00000001 ) CfCJnOujkgPCnnwxzufQtDtDJcnYRU=1465822270.372060314961643313696320466041f; else CfCJnOujkgPCnnwxzufQtDtDJcnYRU=210674042.009973991864554959008557806271f;if (CfCJnOujkgPCnnwxzufQtDtDJcnYRU - CfCJnOujkgPCnnwxzufQtDtDJcnYRU> 0.00000001 ) CfCJnOujkgPCnnwxzufQtDtDJcnYRU=283816986.447306388052393388752805456317f; else CfCJnOujkgPCnnwxzufQtDtDJcnYRU=639915575.873143725878115637453862074305f;if (CfCJnOujkgPCnnwxzufQtDtDJcnYRU - CfCJnOujkgPCnnwxzufQtDtDJcnYRU> 0.00000001 ) CfCJnOujkgPCnnwxzufQtDtDJcnYRU=2087270258.107229025177309570183006401218f; else CfCJnOujkgPCnnwxzufQtDtDJcnYRU=1862199675.053177109671249767360422321182f;if (CfCJnOujkgPCnnwxzufQtDtDJcnYRU - CfCJnOujkgPCnnwxzufQtDtDJcnYRU> 0.00000001 ) CfCJnOujkgPCnnwxzufQtDtDJcnYRU=2069691315.882813433348251296885596963196f; else CfCJnOujkgPCnnwxzufQtDtDJcnYRU=1618189861.703789043733502237214640840606f;int RWkfUcWkJMZOFbIzuShLfwoLGeLvTy=314516154;if (RWkfUcWkJMZOFbIzuShLfwoLGeLvTy == RWkfUcWkJMZOFbIzuShLfwoLGeLvTy- 1 ) RWkfUcWkJMZOFbIzuShLfwoLGeLvTy=928496506; else RWkfUcWkJMZOFbIzuShLfwoLGeLvTy=626978104;if (RWkfUcWkJMZOFbIzuShLfwoLGeLvTy == RWkfUcWkJMZOFbIzuShLfwoLGeLvTy- 1 ) RWkfUcWkJMZOFbIzuShLfwoLGeLvTy=550858533; else RWkfUcWkJMZOFbIzuShLfwoLGeLvTy=1287703458;if (RWkfUcWkJMZOFbIzuShLfwoLGeLvTy == RWkfUcWkJMZOFbIzuShLfwoLGeLvTy- 0 ) RWkfUcWkJMZOFbIzuShLfwoLGeLvTy=209815068; else RWkfUcWkJMZOFbIzuShLfwoLGeLvTy=2009364444;if (RWkfUcWkJMZOFbIzuShLfwoLGeLvTy == RWkfUcWkJMZOFbIzuShLfwoLGeLvTy- 0 ) RWkfUcWkJMZOFbIzuShLfwoLGeLvTy=528407665; else RWkfUcWkJMZOFbIzuShLfwoLGeLvTy=1864845907;if (RWkfUcWkJMZOFbIzuShLfwoLGeLvTy == RWkfUcWkJMZOFbIzuShLfwoLGeLvTy- 0 ) RWkfUcWkJMZOFbIzuShLfwoLGeLvTy=1042247841; else RWkfUcWkJMZOFbIzuShLfwoLGeLvTy=13976314;if (RWkfUcWkJMZOFbIzuShLfwoLGeLvTy == RWkfUcWkJMZOFbIzuShLfwoLGeLvTy- 1 ) RWkfUcWkJMZOFbIzuShLfwoLGeLvTy=1353782088; else RWkfUcWkJMZOFbIzuShLfwoLGeLvTy=1559427429;float uoubFxseyRXMqhHdEZwyKqRaMYReSu=962767386.072361522014019574799555596203f;if (uoubFxseyRXMqhHdEZwyKqRaMYReSu - uoubFxseyRXMqhHdEZwyKqRaMYReSu> 0.00000001 ) uoubFxseyRXMqhHdEZwyKqRaMYReSu=1770381367.665538959854295355038863062787f; else uoubFxseyRXMqhHdEZwyKqRaMYReSu=780462180.633045069173694637135137580268f;if (uoubFxseyRXMqhHdEZwyKqRaMYReSu - uoubFxseyRXMqhHdEZwyKqRaMYReSu> 0.00000001 ) uoubFxseyRXMqhHdEZwyKqRaMYReSu=115865152.637232125039707637043666361767f; else uoubFxseyRXMqhHdEZwyKqRaMYReSu=2109302930.691155646262221644105332617208f;if (uoubFxseyRXMqhHdEZwyKqRaMYReSu - uoubFxseyRXMqhHdEZwyKqRaMYReSu> 0.00000001 ) uoubFxseyRXMqhHdEZwyKqRaMYReSu=1779776225.247955865200188155672876237965f; else uoubFxseyRXMqhHdEZwyKqRaMYReSu=1273330740.854912638788991635567374748182f;if (uoubFxseyRXMqhHdEZwyKqRaMYReSu - uoubFxseyRXMqhHdEZwyKqRaMYReSu> 0.00000001 ) uoubFxseyRXMqhHdEZwyKqRaMYReSu=8317949.801155638652799408661350969128f; else uoubFxseyRXMqhHdEZwyKqRaMYReSu=1772780909.456699795610791568534162265139f;if (uoubFxseyRXMqhHdEZwyKqRaMYReSu - uoubFxseyRXMqhHdEZwyKqRaMYReSu> 0.00000001 ) uoubFxseyRXMqhHdEZwyKqRaMYReSu=856865236.751121721082406166040270449443f; else uoubFxseyRXMqhHdEZwyKqRaMYReSu=1477669385.467606977195755387721029059289f;if (uoubFxseyRXMqhHdEZwyKqRaMYReSu - uoubFxseyRXMqhHdEZwyKqRaMYReSu> 0.00000001 ) uoubFxseyRXMqhHdEZwyKqRaMYReSu=502097541.238972681129906563517383255865f; else uoubFxseyRXMqhHdEZwyKqRaMYReSu=434332588.880996899480660313460777295572f;int nJgPVtyByhlTGPllJQwTJWCYFEQmCE=1862392328;if (nJgPVtyByhlTGPllJQwTJWCYFEQmCE == nJgPVtyByhlTGPllJQwTJWCYFEQmCE- 0 ) nJgPVtyByhlTGPllJQwTJWCYFEQmCE=463608769; else nJgPVtyByhlTGPllJQwTJWCYFEQmCE=15924487;if (nJgPVtyByhlTGPllJQwTJWCYFEQmCE == nJgPVtyByhlTGPllJQwTJWCYFEQmCE- 1 ) nJgPVtyByhlTGPllJQwTJWCYFEQmCE=78776021; else nJgPVtyByhlTGPllJQwTJWCYFEQmCE=1410437601;if (nJgPVtyByhlTGPllJQwTJWCYFEQmCE == nJgPVtyByhlTGPllJQwTJWCYFEQmCE- 0 ) nJgPVtyByhlTGPllJQwTJWCYFEQmCE=1801425910; else nJgPVtyByhlTGPllJQwTJWCYFEQmCE=177504854;if (nJgPVtyByhlTGPllJQwTJWCYFEQmCE == nJgPVtyByhlTGPllJQwTJWCYFEQmCE- 1 ) nJgPVtyByhlTGPllJQwTJWCYFEQmCE=824513210; else nJgPVtyByhlTGPllJQwTJWCYFEQmCE=1574364267;if (nJgPVtyByhlTGPllJQwTJWCYFEQmCE == nJgPVtyByhlTGPllJQwTJWCYFEQmCE- 0 ) nJgPVtyByhlTGPllJQwTJWCYFEQmCE=1725777514; else nJgPVtyByhlTGPllJQwTJWCYFEQmCE=1892963919;if (nJgPVtyByhlTGPllJQwTJWCYFEQmCE == nJgPVtyByhlTGPllJQwTJWCYFEQmCE- 1 ) nJgPVtyByhlTGPllJQwTJWCYFEQmCE=1356986244; else nJgPVtyByhlTGPllJQwTJWCYFEQmCE=1451845956;double HcpPXshmWgHPJXPemmjIWbQNBMVQtB=750291326.744909771496209573182490410223;if (HcpPXshmWgHPJXPemmjIWbQNBMVQtB == HcpPXshmWgHPJXPemmjIWbQNBMVQtB ) HcpPXshmWgHPJXPemmjIWbQNBMVQtB=78187039.349082311109882661789658535459; else HcpPXshmWgHPJXPemmjIWbQNBMVQtB=1106923929.066925733827208472896955470266;if (HcpPXshmWgHPJXPemmjIWbQNBMVQtB == HcpPXshmWgHPJXPemmjIWbQNBMVQtB ) HcpPXshmWgHPJXPemmjIWbQNBMVQtB=1995292141.796224396894906345027467412155; else HcpPXshmWgHPJXPemmjIWbQNBMVQtB=1303303729.567823119418610316913525934092;if (HcpPXshmWgHPJXPemmjIWbQNBMVQtB == HcpPXshmWgHPJXPemmjIWbQNBMVQtB ) HcpPXshmWgHPJXPemmjIWbQNBMVQtB=822979786.334134719334761996720625305399; else HcpPXshmWgHPJXPemmjIWbQNBMVQtB=629347860.012829559760779096415947271093;if (HcpPXshmWgHPJXPemmjIWbQNBMVQtB == HcpPXshmWgHPJXPemmjIWbQNBMVQtB ) HcpPXshmWgHPJXPemmjIWbQNBMVQtB=682223576.943043305355716006632908527565; else HcpPXshmWgHPJXPemmjIWbQNBMVQtB=1802649413.956682884780217124228547215562;if (HcpPXshmWgHPJXPemmjIWbQNBMVQtB == HcpPXshmWgHPJXPemmjIWbQNBMVQtB ) HcpPXshmWgHPJXPemmjIWbQNBMVQtB=618660155.684150923432306289151445741047; else HcpPXshmWgHPJXPemmjIWbQNBMVQtB=925561271.732220717820180725114742124051;if (HcpPXshmWgHPJXPemmjIWbQNBMVQtB == HcpPXshmWgHPJXPemmjIWbQNBMVQtB ) HcpPXshmWgHPJXPemmjIWbQNBMVQtB=430838222.909680018767244237613604419110; else HcpPXshmWgHPJXPemmjIWbQNBMVQtB=879629330.777692151071755851909854941637;long wzrAtwfcFBoecUHvarffsNHDsNPmbJ=785932661;if (wzrAtwfcFBoecUHvarffsNHDsNPmbJ == wzrAtwfcFBoecUHvarffsNHDsNPmbJ- 1 ) wzrAtwfcFBoecUHvarffsNHDsNPmbJ=89902940; else wzrAtwfcFBoecUHvarffsNHDsNPmbJ=1638873407;if (wzrAtwfcFBoecUHvarffsNHDsNPmbJ == wzrAtwfcFBoecUHvarffsNHDsNPmbJ- 1 ) wzrAtwfcFBoecUHvarffsNHDsNPmbJ=1095818660; else wzrAtwfcFBoecUHvarffsNHDsNPmbJ=959280333;if (wzrAtwfcFBoecUHvarffsNHDsNPmbJ == wzrAtwfcFBoecUHvarffsNHDsNPmbJ- 0 ) wzrAtwfcFBoecUHvarffsNHDsNPmbJ=1258202911; else wzrAtwfcFBoecUHvarffsNHDsNPmbJ=373575035;if (wzrAtwfcFBoecUHvarffsNHDsNPmbJ == wzrAtwfcFBoecUHvarffsNHDsNPmbJ- 1 ) wzrAtwfcFBoecUHvarffsNHDsNPmbJ=1308825123; else wzrAtwfcFBoecUHvarffsNHDsNPmbJ=1592876513;if (wzrAtwfcFBoecUHvarffsNHDsNPmbJ == wzrAtwfcFBoecUHvarffsNHDsNPmbJ- 1 ) wzrAtwfcFBoecUHvarffsNHDsNPmbJ=2103432617; else wzrAtwfcFBoecUHvarffsNHDsNPmbJ=688984532;if (wzrAtwfcFBoecUHvarffsNHDsNPmbJ == wzrAtwfcFBoecUHvarffsNHDsNPmbJ- 1 ) wzrAtwfcFBoecUHvarffsNHDsNPmbJ=383525416; else wzrAtwfcFBoecUHvarffsNHDsNPmbJ=2034295953;int hmEnWlabmeJmsYaaeRqfADrAqAobNq=2132475679;if (hmEnWlabmeJmsYaaeRqfADrAqAobNq == hmEnWlabmeJmsYaaeRqfADrAqAobNq- 0 ) hmEnWlabmeJmsYaaeRqfADrAqAobNq=1812958630; else hmEnWlabmeJmsYaaeRqfADrAqAobNq=1899289862;if (hmEnWlabmeJmsYaaeRqfADrAqAobNq == hmEnWlabmeJmsYaaeRqfADrAqAobNq- 0 ) hmEnWlabmeJmsYaaeRqfADrAqAobNq=356145549; else hmEnWlabmeJmsYaaeRqfADrAqAobNq=1982590390;if (hmEnWlabmeJmsYaaeRqfADrAqAobNq == hmEnWlabmeJmsYaaeRqfADrAqAobNq- 0 ) hmEnWlabmeJmsYaaeRqfADrAqAobNq=408506686; else hmEnWlabmeJmsYaaeRqfADrAqAobNq=768253292;if (hmEnWlabmeJmsYaaeRqfADrAqAobNq == hmEnWlabmeJmsYaaeRqfADrAqAobNq- 1 ) hmEnWlabmeJmsYaaeRqfADrAqAobNq=1039412542; else hmEnWlabmeJmsYaaeRqfADrAqAobNq=1981744556;if (hmEnWlabmeJmsYaaeRqfADrAqAobNq == hmEnWlabmeJmsYaaeRqfADrAqAobNq- 0 ) hmEnWlabmeJmsYaaeRqfADrAqAobNq=1481184558; else hmEnWlabmeJmsYaaeRqfADrAqAobNq=423610255;if (hmEnWlabmeJmsYaaeRqfADrAqAobNq == hmEnWlabmeJmsYaaeRqfADrAqAobNq- 1 ) hmEnWlabmeJmsYaaeRqfADrAqAobNq=791777446; else hmEnWlabmeJmsYaaeRqfADrAqAobNq=875864639;float mcxpetWjFbagUCbNZrFFVfFeLYuSVa=1631702437.820735205576987449409468088034f;if (mcxpetWjFbagUCbNZrFFVfFeLYuSVa - mcxpetWjFbagUCbNZrFFVfFeLYuSVa> 0.00000001 ) mcxpetWjFbagUCbNZrFFVfFeLYuSVa=970148684.021511362382585599552555246386f; else mcxpetWjFbagUCbNZrFFVfFeLYuSVa=571053583.728155657619823809360069367638f;if (mcxpetWjFbagUCbNZrFFVfFeLYuSVa - mcxpetWjFbagUCbNZrFFVfFeLYuSVa> 0.00000001 ) mcxpetWjFbagUCbNZrFFVfFeLYuSVa=1361943016.619666543194394474254813777999f; else mcxpetWjFbagUCbNZrFFVfFeLYuSVa=1512985841.803011026737637687699458491709f;if (mcxpetWjFbagUCbNZrFFVfFeLYuSVa - mcxpetWjFbagUCbNZrFFVfFeLYuSVa> 0.00000001 ) mcxpetWjFbagUCbNZrFFVfFeLYuSVa=1923538191.076530239345863828610030056563f; else mcxpetWjFbagUCbNZrFFVfFeLYuSVa=852914069.143787120745936469677319014658f;if (mcxpetWjFbagUCbNZrFFVfFeLYuSVa - mcxpetWjFbagUCbNZrFFVfFeLYuSVa> 0.00000001 ) mcxpetWjFbagUCbNZrFFVfFeLYuSVa=1639572062.588630328771808611570700784587f; else mcxpetWjFbagUCbNZrFFVfFeLYuSVa=1852095651.368227730625200652151429998806f;if (mcxpetWjFbagUCbNZrFFVfFeLYuSVa - mcxpetWjFbagUCbNZrFFVfFeLYuSVa> 0.00000001 ) mcxpetWjFbagUCbNZrFFVfFeLYuSVa=1945582453.784242468198105661113344704943f; else mcxpetWjFbagUCbNZrFFVfFeLYuSVa=402415628.904505106830416333444880344155f;if (mcxpetWjFbagUCbNZrFFVfFeLYuSVa - mcxpetWjFbagUCbNZrFFVfFeLYuSVa> 0.00000001 ) mcxpetWjFbagUCbNZrFFVfFeLYuSVa=1347354759.853520561156639494857260022536f; else mcxpetWjFbagUCbNZrFFVfFeLYuSVa=133995254.116118799798340936195130960461f;int zPZzXwWMEmCYDduyLBFvgRWABkeMVr=2062188162;if (zPZzXwWMEmCYDduyLBFvgRWABkeMVr == zPZzXwWMEmCYDduyLBFvgRWABkeMVr- 0 ) zPZzXwWMEmCYDduyLBFvgRWABkeMVr=157155963; else zPZzXwWMEmCYDduyLBFvgRWABkeMVr=445846190;if (zPZzXwWMEmCYDduyLBFvgRWABkeMVr == zPZzXwWMEmCYDduyLBFvgRWABkeMVr- 0 ) zPZzXwWMEmCYDduyLBFvgRWABkeMVr=1214154347; else zPZzXwWMEmCYDduyLBFvgRWABkeMVr=1755460688;if (zPZzXwWMEmCYDduyLBFvgRWABkeMVr == zPZzXwWMEmCYDduyLBFvgRWABkeMVr- 0 ) zPZzXwWMEmCYDduyLBFvgRWABkeMVr=1885413175; else zPZzXwWMEmCYDduyLBFvgRWABkeMVr=1087708492;if (zPZzXwWMEmCYDduyLBFvgRWABkeMVr == zPZzXwWMEmCYDduyLBFvgRWABkeMVr- 0 ) zPZzXwWMEmCYDduyLBFvgRWABkeMVr=377425975; else zPZzXwWMEmCYDduyLBFvgRWABkeMVr=1469133295;if (zPZzXwWMEmCYDduyLBFvgRWABkeMVr == zPZzXwWMEmCYDduyLBFvgRWABkeMVr- 0 ) zPZzXwWMEmCYDduyLBFvgRWABkeMVr=599460865; else zPZzXwWMEmCYDduyLBFvgRWABkeMVr=1261557751;if (zPZzXwWMEmCYDduyLBFvgRWABkeMVr == zPZzXwWMEmCYDduyLBFvgRWABkeMVr- 0 ) zPZzXwWMEmCYDduyLBFvgRWABkeMVr=1625579422; else zPZzXwWMEmCYDduyLBFvgRWABkeMVr=834592656;float IKsgbhsuUmDRphkfZQicofKgHmmdkZ=11065090.158174401309364555681096129952f;if (IKsgbhsuUmDRphkfZQicofKgHmmdkZ - IKsgbhsuUmDRphkfZQicofKgHmmdkZ> 0.00000001 ) IKsgbhsuUmDRphkfZQicofKgHmmdkZ=621887168.224338332204572144971706726316f; else IKsgbhsuUmDRphkfZQicofKgHmmdkZ=899306626.665018614723458753418509162962f;if (IKsgbhsuUmDRphkfZQicofKgHmmdkZ - IKsgbhsuUmDRphkfZQicofKgHmmdkZ> 0.00000001 ) IKsgbhsuUmDRphkfZQicofKgHmmdkZ=1064147269.312675067454135836807491817695f; else IKsgbhsuUmDRphkfZQicofKgHmmdkZ=1492774560.685252288480616548443716503297f;if (IKsgbhsuUmDRphkfZQicofKgHmmdkZ - IKsgbhsuUmDRphkfZQicofKgHmmdkZ> 0.00000001 ) IKsgbhsuUmDRphkfZQicofKgHmmdkZ=1489496271.947288999375953888548936307085f; else IKsgbhsuUmDRphkfZQicofKgHmmdkZ=1402903114.000264354865917897483575902298f;if (IKsgbhsuUmDRphkfZQicofKgHmmdkZ - IKsgbhsuUmDRphkfZQicofKgHmmdkZ> 0.00000001 ) IKsgbhsuUmDRphkfZQicofKgHmmdkZ=208937488.202447670963589302952161363636f; else IKsgbhsuUmDRphkfZQicofKgHmmdkZ=960044561.316666468032752009770135725219f;if (IKsgbhsuUmDRphkfZQicofKgHmmdkZ - IKsgbhsuUmDRphkfZQicofKgHmmdkZ> 0.00000001 ) IKsgbhsuUmDRphkfZQicofKgHmmdkZ=259717800.515475702405493042946497729295f; else IKsgbhsuUmDRphkfZQicofKgHmmdkZ=651533545.236063848831738674388338412885f;if (IKsgbhsuUmDRphkfZQicofKgHmmdkZ - IKsgbhsuUmDRphkfZQicofKgHmmdkZ> 0.00000001 ) IKsgbhsuUmDRphkfZQicofKgHmmdkZ=37007937.761838954020421587926502790966f; else IKsgbhsuUmDRphkfZQicofKgHmmdkZ=6091058.207418101172136892794606036014f; }
 IKsgbhsuUmDRphkfZQicofKgHmmdkZy::IKsgbhsuUmDRphkfZQicofKgHmmdkZy()
 { this->uBseyLDbHcMm("mROmYqnmIbBtJQTOjRXzlhZXcALZpvuBseyLDbHcMmj", true, 1568382180, 905417288, 1735922112); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class HYQTIqNcHckGKchlsjCySXojcrUIXQy
 { 
public: bool dmYaqGwNUDQqJJbafIuxUGMijjgqRW; double dmYaqGwNUDQqJJbafIuxUGMijjgqRWHYQTIqNcHckGKchlsjCySXojcrUIXQ; HYQTIqNcHckGKchlsjCySXojcrUIXQy(); void ZdUDHEMFehsA(string dmYaqGwNUDQqJJbafIuxUGMijjgqRWZdUDHEMFehsA, bool FFqpHTlBrfvViuBhfaBQozdTSWaqqH, int aFwxyLzUCHpdROBzYRgCEGpAhFbWWk, float svLjFqZkwqhKXtbKiOSTyJJoCUQwPs, long jdyRPtShJLZnVqVuivZWXNECmjmWgk);
 protected: bool dmYaqGwNUDQqJJbafIuxUGMijjgqRWo; double dmYaqGwNUDQqJJbafIuxUGMijjgqRWHYQTIqNcHckGKchlsjCySXojcrUIXQf; void ZdUDHEMFehsAu(string dmYaqGwNUDQqJJbafIuxUGMijjgqRWZdUDHEMFehsAg, bool FFqpHTlBrfvViuBhfaBQozdTSWaqqHe, int aFwxyLzUCHpdROBzYRgCEGpAhFbWWkr, float svLjFqZkwqhKXtbKiOSTyJJoCUQwPsw, long jdyRPtShJLZnVqVuivZWXNECmjmWgkn);
 private: bool dmYaqGwNUDQqJJbafIuxUGMijjgqRWFFqpHTlBrfvViuBhfaBQozdTSWaqqH; double dmYaqGwNUDQqJJbafIuxUGMijjgqRWsvLjFqZkwqhKXtbKiOSTyJJoCUQwPsHYQTIqNcHckGKchlsjCySXojcrUIXQ;
 void ZdUDHEMFehsAv(string FFqpHTlBrfvViuBhfaBQozdTSWaqqHZdUDHEMFehsA, bool FFqpHTlBrfvViuBhfaBQozdTSWaqqHaFwxyLzUCHpdROBzYRgCEGpAhFbWWk, int aFwxyLzUCHpdROBzYRgCEGpAhFbWWkdmYaqGwNUDQqJJbafIuxUGMijjgqRW, float svLjFqZkwqhKXtbKiOSTyJJoCUQwPsjdyRPtShJLZnVqVuivZWXNECmjmWgk, long jdyRPtShJLZnVqVuivZWXNECmjmWgkFFqpHTlBrfvViuBhfaBQozdTSWaqqH); };
 void HYQTIqNcHckGKchlsjCySXojcrUIXQy::ZdUDHEMFehsA(string dmYaqGwNUDQqJJbafIuxUGMijjgqRWZdUDHEMFehsA, bool FFqpHTlBrfvViuBhfaBQozdTSWaqqH, int aFwxyLzUCHpdROBzYRgCEGpAhFbWWk, float svLjFqZkwqhKXtbKiOSTyJJoCUQwPs, long jdyRPtShJLZnVqVuivZWXNECmjmWgk)
 { long nDxlYYzsMgUmQzhezMmAFQujWSgxOo=1731514471;if (nDxlYYzsMgUmQzhezMmAFQujWSgxOo == nDxlYYzsMgUmQzhezMmAFQujWSgxOo- 1 ) nDxlYYzsMgUmQzhezMmAFQujWSgxOo=1025929140; else nDxlYYzsMgUmQzhezMmAFQujWSgxOo=2046392650;if (nDxlYYzsMgUmQzhezMmAFQujWSgxOo == nDxlYYzsMgUmQzhezMmAFQujWSgxOo- 0 ) nDxlYYzsMgUmQzhezMmAFQujWSgxOo=1476935152; else nDxlYYzsMgUmQzhezMmAFQujWSgxOo=698867588;if (nDxlYYzsMgUmQzhezMmAFQujWSgxOo == nDxlYYzsMgUmQzhezMmAFQujWSgxOo- 1 ) nDxlYYzsMgUmQzhezMmAFQujWSgxOo=167332501; else nDxlYYzsMgUmQzhezMmAFQujWSgxOo=150506750;if (nDxlYYzsMgUmQzhezMmAFQujWSgxOo == nDxlYYzsMgUmQzhezMmAFQujWSgxOo- 0 ) nDxlYYzsMgUmQzhezMmAFQujWSgxOo=446468115; else nDxlYYzsMgUmQzhezMmAFQujWSgxOo=172348232;if (nDxlYYzsMgUmQzhezMmAFQujWSgxOo == nDxlYYzsMgUmQzhezMmAFQujWSgxOo- 1 ) nDxlYYzsMgUmQzhezMmAFQujWSgxOo=1083156835; else nDxlYYzsMgUmQzhezMmAFQujWSgxOo=1377167792;if (nDxlYYzsMgUmQzhezMmAFQujWSgxOo == nDxlYYzsMgUmQzhezMmAFQujWSgxOo- 1 ) nDxlYYzsMgUmQzhezMmAFQujWSgxOo=1664113335; else nDxlYYzsMgUmQzhezMmAFQujWSgxOo=107060632;long uSunYidCLuNdtmXEhurMlIQLPavxlf=2055459471;if (uSunYidCLuNdtmXEhurMlIQLPavxlf == uSunYidCLuNdtmXEhurMlIQLPavxlf- 0 ) uSunYidCLuNdtmXEhurMlIQLPavxlf=1973698883; else uSunYidCLuNdtmXEhurMlIQLPavxlf=1899158005;if (uSunYidCLuNdtmXEhurMlIQLPavxlf == uSunYidCLuNdtmXEhurMlIQLPavxlf- 1 ) uSunYidCLuNdtmXEhurMlIQLPavxlf=826054125; else uSunYidCLuNdtmXEhurMlIQLPavxlf=1913547303;if (uSunYidCLuNdtmXEhurMlIQLPavxlf == uSunYidCLuNdtmXEhurMlIQLPavxlf- 1 ) uSunYidCLuNdtmXEhurMlIQLPavxlf=1027391169; else uSunYidCLuNdtmXEhurMlIQLPavxlf=1691979157;if (uSunYidCLuNdtmXEhurMlIQLPavxlf == uSunYidCLuNdtmXEhurMlIQLPavxlf- 0 ) uSunYidCLuNdtmXEhurMlIQLPavxlf=1059801124; else uSunYidCLuNdtmXEhurMlIQLPavxlf=1734145171;if (uSunYidCLuNdtmXEhurMlIQLPavxlf == uSunYidCLuNdtmXEhurMlIQLPavxlf- 1 ) uSunYidCLuNdtmXEhurMlIQLPavxlf=1714109590; else uSunYidCLuNdtmXEhurMlIQLPavxlf=1849588610;if (uSunYidCLuNdtmXEhurMlIQLPavxlf == uSunYidCLuNdtmXEhurMlIQLPavxlf- 1 ) uSunYidCLuNdtmXEhurMlIQLPavxlf=519371712; else uSunYidCLuNdtmXEhurMlIQLPavxlf=165223173;float dgEykJFjBDyhgGPoEvSeVQUufyUwqm=1179867389.513715554690110363204652203803f;if (dgEykJFjBDyhgGPoEvSeVQUufyUwqm - dgEykJFjBDyhgGPoEvSeVQUufyUwqm> 0.00000001 ) dgEykJFjBDyhgGPoEvSeVQUufyUwqm=1557902298.689338478212353910080733607151f; else dgEykJFjBDyhgGPoEvSeVQUufyUwqm=1936199477.275212189290610230940493405673f;if (dgEykJFjBDyhgGPoEvSeVQUufyUwqm - dgEykJFjBDyhgGPoEvSeVQUufyUwqm> 0.00000001 ) dgEykJFjBDyhgGPoEvSeVQUufyUwqm=237842611.232704824294199013448732289282f; else dgEykJFjBDyhgGPoEvSeVQUufyUwqm=2001141058.761617991651676740549223651163f;if (dgEykJFjBDyhgGPoEvSeVQUufyUwqm - dgEykJFjBDyhgGPoEvSeVQUufyUwqm> 0.00000001 ) dgEykJFjBDyhgGPoEvSeVQUufyUwqm=1454588851.664354359223290726755985454816f; else dgEykJFjBDyhgGPoEvSeVQUufyUwqm=1902058613.683919843349232714404651878900f;if (dgEykJFjBDyhgGPoEvSeVQUufyUwqm - dgEykJFjBDyhgGPoEvSeVQUufyUwqm> 0.00000001 ) dgEykJFjBDyhgGPoEvSeVQUufyUwqm=1912642743.055374048198352786375052314325f; else dgEykJFjBDyhgGPoEvSeVQUufyUwqm=427887162.195330407878631630010373611406f;if (dgEykJFjBDyhgGPoEvSeVQUufyUwqm - dgEykJFjBDyhgGPoEvSeVQUufyUwqm> 0.00000001 ) dgEykJFjBDyhgGPoEvSeVQUufyUwqm=966337120.182401413348060852179431593282f; else dgEykJFjBDyhgGPoEvSeVQUufyUwqm=1234808838.900180623033888241387033875102f;if (dgEykJFjBDyhgGPoEvSeVQUufyUwqm - dgEykJFjBDyhgGPoEvSeVQUufyUwqm> 0.00000001 ) dgEykJFjBDyhgGPoEvSeVQUufyUwqm=2032792084.923144898123925665446290608741f; else dgEykJFjBDyhgGPoEvSeVQUufyUwqm=801951127.553221084445337412921086773355f;long FZkbjylyXhpFxxEugTLudtLBamjaYl=498552083;if (FZkbjylyXhpFxxEugTLudtLBamjaYl == FZkbjylyXhpFxxEugTLudtLBamjaYl- 0 ) FZkbjylyXhpFxxEugTLudtLBamjaYl=706592750; else FZkbjylyXhpFxxEugTLudtLBamjaYl=1042272241;if (FZkbjylyXhpFxxEugTLudtLBamjaYl == FZkbjylyXhpFxxEugTLudtLBamjaYl- 1 ) FZkbjylyXhpFxxEugTLudtLBamjaYl=1793408414; else FZkbjylyXhpFxxEugTLudtLBamjaYl=1391190567;if (FZkbjylyXhpFxxEugTLudtLBamjaYl == FZkbjylyXhpFxxEugTLudtLBamjaYl- 0 ) FZkbjylyXhpFxxEugTLudtLBamjaYl=1408066113; else FZkbjylyXhpFxxEugTLudtLBamjaYl=1444253403;if (FZkbjylyXhpFxxEugTLudtLBamjaYl == FZkbjylyXhpFxxEugTLudtLBamjaYl- 0 ) FZkbjylyXhpFxxEugTLudtLBamjaYl=704123302; else FZkbjylyXhpFxxEugTLudtLBamjaYl=1481433630;if (FZkbjylyXhpFxxEugTLudtLBamjaYl == FZkbjylyXhpFxxEugTLudtLBamjaYl- 0 ) FZkbjylyXhpFxxEugTLudtLBamjaYl=986591526; else FZkbjylyXhpFxxEugTLudtLBamjaYl=1890273628;if (FZkbjylyXhpFxxEugTLudtLBamjaYl == FZkbjylyXhpFxxEugTLudtLBamjaYl- 0 ) FZkbjylyXhpFxxEugTLudtLBamjaYl=1132241645; else FZkbjylyXhpFxxEugTLudtLBamjaYl=465227538;float qAmUpLPgPPHjqkgjamsKvVkvSGEVfk=1825837561.872560605199154216888040550681f;if (qAmUpLPgPPHjqkgjamsKvVkvSGEVfk - qAmUpLPgPPHjqkgjamsKvVkvSGEVfk> 0.00000001 ) qAmUpLPgPPHjqkgjamsKvVkvSGEVfk=1486056735.094375936136968262183244693201f; else qAmUpLPgPPHjqkgjamsKvVkvSGEVfk=484245946.112510688939076219340851896088f;if (qAmUpLPgPPHjqkgjamsKvVkvSGEVfk - qAmUpLPgPPHjqkgjamsKvVkvSGEVfk> 0.00000001 ) qAmUpLPgPPHjqkgjamsKvVkvSGEVfk=696641981.283613545968525527949507236183f; else qAmUpLPgPPHjqkgjamsKvVkvSGEVfk=953053980.461085656387449343647925079749f;if (qAmUpLPgPPHjqkgjamsKvVkvSGEVfk - qAmUpLPgPPHjqkgjamsKvVkvSGEVfk> 0.00000001 ) qAmUpLPgPPHjqkgjamsKvVkvSGEVfk=949677978.747350755197267216909455665701f; else qAmUpLPgPPHjqkgjamsKvVkvSGEVfk=496001259.602874019960966086887495049370f;if (qAmUpLPgPPHjqkgjamsKvVkvSGEVfk - qAmUpLPgPPHjqkgjamsKvVkvSGEVfk> 0.00000001 ) qAmUpLPgPPHjqkgjamsKvVkvSGEVfk=600870620.515459640321704050978992531794f; else qAmUpLPgPPHjqkgjamsKvVkvSGEVfk=71640270.585046551744757679109041272392f;if (qAmUpLPgPPHjqkgjamsKvVkvSGEVfk - qAmUpLPgPPHjqkgjamsKvVkvSGEVfk> 0.00000001 ) qAmUpLPgPPHjqkgjamsKvVkvSGEVfk=2133998909.896795687961933434697360564588f; else qAmUpLPgPPHjqkgjamsKvVkvSGEVfk=398166963.426095917209335911884971563624f;if (qAmUpLPgPPHjqkgjamsKvVkvSGEVfk - qAmUpLPgPPHjqkgjamsKvVkvSGEVfk> 0.00000001 ) qAmUpLPgPPHjqkgjamsKvVkvSGEVfk=947210183.121954504862859804665380466653f; else qAmUpLPgPPHjqkgjamsKvVkvSGEVfk=1940065978.339419859152803223287016995260f;long rYFeLPSUCzZaXHReogIWPWmhMLQaMR=869638270;if (rYFeLPSUCzZaXHReogIWPWmhMLQaMR == rYFeLPSUCzZaXHReogIWPWmhMLQaMR- 0 ) rYFeLPSUCzZaXHReogIWPWmhMLQaMR=1119992431; else rYFeLPSUCzZaXHReogIWPWmhMLQaMR=20819629;if (rYFeLPSUCzZaXHReogIWPWmhMLQaMR == rYFeLPSUCzZaXHReogIWPWmhMLQaMR- 1 ) rYFeLPSUCzZaXHReogIWPWmhMLQaMR=1606114070; else rYFeLPSUCzZaXHReogIWPWmhMLQaMR=694557368;if (rYFeLPSUCzZaXHReogIWPWmhMLQaMR == rYFeLPSUCzZaXHReogIWPWmhMLQaMR- 1 ) rYFeLPSUCzZaXHReogIWPWmhMLQaMR=1758878154; else rYFeLPSUCzZaXHReogIWPWmhMLQaMR=647404137;if (rYFeLPSUCzZaXHReogIWPWmhMLQaMR == rYFeLPSUCzZaXHReogIWPWmhMLQaMR- 1 ) rYFeLPSUCzZaXHReogIWPWmhMLQaMR=1919284923; else rYFeLPSUCzZaXHReogIWPWmhMLQaMR=113648895;if (rYFeLPSUCzZaXHReogIWPWmhMLQaMR == rYFeLPSUCzZaXHReogIWPWmhMLQaMR- 1 ) rYFeLPSUCzZaXHReogIWPWmhMLQaMR=1232076175; else rYFeLPSUCzZaXHReogIWPWmhMLQaMR=903892628;if (rYFeLPSUCzZaXHReogIWPWmhMLQaMR == rYFeLPSUCzZaXHReogIWPWmhMLQaMR- 0 ) rYFeLPSUCzZaXHReogIWPWmhMLQaMR=1014549532; else rYFeLPSUCzZaXHReogIWPWmhMLQaMR=1711798870;double GJxYqUHjBvxLXYqtiNMwTceHzdaDCW=685092178.460095041917634214883624205051;if (GJxYqUHjBvxLXYqtiNMwTceHzdaDCW == GJxYqUHjBvxLXYqtiNMwTceHzdaDCW ) GJxYqUHjBvxLXYqtiNMwTceHzdaDCW=651806568.402258926180624477149900794790; else GJxYqUHjBvxLXYqtiNMwTceHzdaDCW=206954523.110061549649980401093801360367;if (GJxYqUHjBvxLXYqtiNMwTceHzdaDCW == GJxYqUHjBvxLXYqtiNMwTceHzdaDCW ) GJxYqUHjBvxLXYqtiNMwTceHzdaDCW=1463378039.534718067393951003139470732981; else GJxYqUHjBvxLXYqtiNMwTceHzdaDCW=460246910.239221638254571196136824826508;if (GJxYqUHjBvxLXYqtiNMwTceHzdaDCW == GJxYqUHjBvxLXYqtiNMwTceHzdaDCW ) GJxYqUHjBvxLXYqtiNMwTceHzdaDCW=147829339.729627573058698591974796551651; else GJxYqUHjBvxLXYqtiNMwTceHzdaDCW=1924025846.613164696563318824633664413318;if (GJxYqUHjBvxLXYqtiNMwTceHzdaDCW == GJxYqUHjBvxLXYqtiNMwTceHzdaDCW ) GJxYqUHjBvxLXYqtiNMwTceHzdaDCW=1696357752.693255563501551249873712915076; else GJxYqUHjBvxLXYqtiNMwTceHzdaDCW=210591491.138998016447214212327564509145;if (GJxYqUHjBvxLXYqtiNMwTceHzdaDCW == GJxYqUHjBvxLXYqtiNMwTceHzdaDCW ) GJxYqUHjBvxLXYqtiNMwTceHzdaDCW=441401621.069757376423354661970486978440; else GJxYqUHjBvxLXYqtiNMwTceHzdaDCW=1721768144.998248957704690522104937579398;if (GJxYqUHjBvxLXYqtiNMwTceHzdaDCW == GJxYqUHjBvxLXYqtiNMwTceHzdaDCW ) GJxYqUHjBvxLXYqtiNMwTceHzdaDCW=1404675305.094753611781428699656745617288; else GJxYqUHjBvxLXYqtiNMwTceHzdaDCW=1009899150.067981105891252452999184335914;float VQNhKWZRmauSqeXazNNiYOwwwpPkad=728741529.887916750135356960941050311491f;if (VQNhKWZRmauSqeXazNNiYOwwwpPkad - VQNhKWZRmauSqeXazNNiYOwwwpPkad> 0.00000001 ) VQNhKWZRmauSqeXazNNiYOwwwpPkad=1100621313.266986856093175115967558284870f; else VQNhKWZRmauSqeXazNNiYOwwwpPkad=262603375.637098957112506809531663571981f;if (VQNhKWZRmauSqeXazNNiYOwwwpPkad - VQNhKWZRmauSqeXazNNiYOwwwpPkad> 0.00000001 ) VQNhKWZRmauSqeXazNNiYOwwwpPkad=1492718754.219646728826886515349634008463f; else VQNhKWZRmauSqeXazNNiYOwwwpPkad=1757799056.718700407018182598807774278574f;if (VQNhKWZRmauSqeXazNNiYOwwwpPkad - VQNhKWZRmauSqeXazNNiYOwwwpPkad> 0.00000001 ) VQNhKWZRmauSqeXazNNiYOwwwpPkad=822614519.947612929622933156252166704454f; else VQNhKWZRmauSqeXazNNiYOwwwpPkad=2035928549.393328267177939594122332388102f;if (VQNhKWZRmauSqeXazNNiYOwwwpPkad - VQNhKWZRmauSqeXazNNiYOwwwpPkad> 0.00000001 ) VQNhKWZRmauSqeXazNNiYOwwwpPkad=66959407.857633972590929523822034206204f; else VQNhKWZRmauSqeXazNNiYOwwwpPkad=838652598.127632140773363252386548082444f;if (VQNhKWZRmauSqeXazNNiYOwwwpPkad - VQNhKWZRmauSqeXazNNiYOwwwpPkad> 0.00000001 ) VQNhKWZRmauSqeXazNNiYOwwwpPkad=712444670.777858430859316088216593868976f; else VQNhKWZRmauSqeXazNNiYOwwwpPkad=582993086.070700652592363823300534938409f;if (VQNhKWZRmauSqeXazNNiYOwwwpPkad - VQNhKWZRmauSqeXazNNiYOwwwpPkad> 0.00000001 ) VQNhKWZRmauSqeXazNNiYOwwwpPkad=1868461452.057489972643672434014677655798f; else VQNhKWZRmauSqeXazNNiYOwwwpPkad=45785350.236354598596286806250540559093f;long OpVSJmPHovWdbItUuHpdvVXpmpdozc=686583704;if (OpVSJmPHovWdbItUuHpdvVXpmpdozc == OpVSJmPHovWdbItUuHpdvVXpmpdozc- 1 ) OpVSJmPHovWdbItUuHpdvVXpmpdozc=1748898486; else OpVSJmPHovWdbItUuHpdvVXpmpdozc=262118005;if (OpVSJmPHovWdbItUuHpdvVXpmpdozc == OpVSJmPHovWdbItUuHpdvVXpmpdozc- 1 ) OpVSJmPHovWdbItUuHpdvVXpmpdozc=1288259410; else OpVSJmPHovWdbItUuHpdvVXpmpdozc=1599716197;if (OpVSJmPHovWdbItUuHpdvVXpmpdozc == OpVSJmPHovWdbItUuHpdvVXpmpdozc- 1 ) OpVSJmPHovWdbItUuHpdvVXpmpdozc=941955605; else OpVSJmPHovWdbItUuHpdvVXpmpdozc=329405405;if (OpVSJmPHovWdbItUuHpdvVXpmpdozc == OpVSJmPHovWdbItUuHpdvVXpmpdozc- 0 ) OpVSJmPHovWdbItUuHpdvVXpmpdozc=721808931; else OpVSJmPHovWdbItUuHpdvVXpmpdozc=1343450232;if (OpVSJmPHovWdbItUuHpdvVXpmpdozc == OpVSJmPHovWdbItUuHpdvVXpmpdozc- 0 ) OpVSJmPHovWdbItUuHpdvVXpmpdozc=471945524; else OpVSJmPHovWdbItUuHpdvVXpmpdozc=1395522579;if (OpVSJmPHovWdbItUuHpdvVXpmpdozc == OpVSJmPHovWdbItUuHpdvVXpmpdozc- 1 ) OpVSJmPHovWdbItUuHpdvVXpmpdozc=253155747; else OpVSJmPHovWdbItUuHpdvVXpmpdozc=37110010;int YgRagzQEaqFxQywstkpkrHbCUcpCyV=320535204;if (YgRagzQEaqFxQywstkpkrHbCUcpCyV == YgRagzQEaqFxQywstkpkrHbCUcpCyV- 0 ) YgRagzQEaqFxQywstkpkrHbCUcpCyV=1229129718; else YgRagzQEaqFxQywstkpkrHbCUcpCyV=285808003;if (YgRagzQEaqFxQywstkpkrHbCUcpCyV == YgRagzQEaqFxQywstkpkrHbCUcpCyV- 1 ) YgRagzQEaqFxQywstkpkrHbCUcpCyV=611177557; else YgRagzQEaqFxQywstkpkrHbCUcpCyV=507213593;if (YgRagzQEaqFxQywstkpkrHbCUcpCyV == YgRagzQEaqFxQywstkpkrHbCUcpCyV- 1 ) YgRagzQEaqFxQywstkpkrHbCUcpCyV=2102180098; else YgRagzQEaqFxQywstkpkrHbCUcpCyV=1186436715;if (YgRagzQEaqFxQywstkpkrHbCUcpCyV == YgRagzQEaqFxQywstkpkrHbCUcpCyV- 1 ) YgRagzQEaqFxQywstkpkrHbCUcpCyV=2009961306; else YgRagzQEaqFxQywstkpkrHbCUcpCyV=763361666;if (YgRagzQEaqFxQywstkpkrHbCUcpCyV == YgRagzQEaqFxQywstkpkrHbCUcpCyV- 0 ) YgRagzQEaqFxQywstkpkrHbCUcpCyV=139995116; else YgRagzQEaqFxQywstkpkrHbCUcpCyV=624725441;if (YgRagzQEaqFxQywstkpkrHbCUcpCyV == YgRagzQEaqFxQywstkpkrHbCUcpCyV- 0 ) YgRagzQEaqFxQywstkpkrHbCUcpCyV=1895285887; else YgRagzQEaqFxQywstkpkrHbCUcpCyV=1712319900;long tZBmqlbNFpvTkpFGXvxIetFPxnFeKi=1552329871;if (tZBmqlbNFpvTkpFGXvxIetFPxnFeKi == tZBmqlbNFpvTkpFGXvxIetFPxnFeKi- 0 ) tZBmqlbNFpvTkpFGXvxIetFPxnFeKi=272461195; else tZBmqlbNFpvTkpFGXvxIetFPxnFeKi=718091601;if (tZBmqlbNFpvTkpFGXvxIetFPxnFeKi == tZBmqlbNFpvTkpFGXvxIetFPxnFeKi- 0 ) tZBmqlbNFpvTkpFGXvxIetFPxnFeKi=1408484311; else tZBmqlbNFpvTkpFGXvxIetFPxnFeKi=234925215;if (tZBmqlbNFpvTkpFGXvxIetFPxnFeKi == tZBmqlbNFpvTkpFGXvxIetFPxnFeKi- 1 ) tZBmqlbNFpvTkpFGXvxIetFPxnFeKi=1770765208; else tZBmqlbNFpvTkpFGXvxIetFPxnFeKi=222530360;if (tZBmqlbNFpvTkpFGXvxIetFPxnFeKi == tZBmqlbNFpvTkpFGXvxIetFPxnFeKi- 0 ) tZBmqlbNFpvTkpFGXvxIetFPxnFeKi=1934269571; else tZBmqlbNFpvTkpFGXvxIetFPxnFeKi=771215908;if (tZBmqlbNFpvTkpFGXvxIetFPxnFeKi == tZBmqlbNFpvTkpFGXvxIetFPxnFeKi- 0 ) tZBmqlbNFpvTkpFGXvxIetFPxnFeKi=1688278091; else tZBmqlbNFpvTkpFGXvxIetFPxnFeKi=149268522;if (tZBmqlbNFpvTkpFGXvxIetFPxnFeKi == tZBmqlbNFpvTkpFGXvxIetFPxnFeKi- 0 ) tZBmqlbNFpvTkpFGXvxIetFPxnFeKi=1285853532; else tZBmqlbNFpvTkpFGXvxIetFPxnFeKi=1574575587;long heCDMZsjFglkZdJFSoAiorZUHaTinC=1950542544;if (heCDMZsjFglkZdJFSoAiorZUHaTinC == heCDMZsjFglkZdJFSoAiorZUHaTinC- 1 ) heCDMZsjFglkZdJFSoAiorZUHaTinC=966387411; else heCDMZsjFglkZdJFSoAiorZUHaTinC=1547926248;if (heCDMZsjFglkZdJFSoAiorZUHaTinC == heCDMZsjFglkZdJFSoAiorZUHaTinC- 1 ) heCDMZsjFglkZdJFSoAiorZUHaTinC=964139279; else heCDMZsjFglkZdJFSoAiorZUHaTinC=1343127371;if (heCDMZsjFglkZdJFSoAiorZUHaTinC == heCDMZsjFglkZdJFSoAiorZUHaTinC- 0 ) heCDMZsjFglkZdJFSoAiorZUHaTinC=425377840; else heCDMZsjFglkZdJFSoAiorZUHaTinC=942093356;if (heCDMZsjFglkZdJFSoAiorZUHaTinC == heCDMZsjFglkZdJFSoAiorZUHaTinC- 1 ) heCDMZsjFglkZdJFSoAiorZUHaTinC=731887253; else heCDMZsjFglkZdJFSoAiorZUHaTinC=562461771;if (heCDMZsjFglkZdJFSoAiorZUHaTinC == heCDMZsjFglkZdJFSoAiorZUHaTinC- 0 ) heCDMZsjFglkZdJFSoAiorZUHaTinC=399640346; else heCDMZsjFglkZdJFSoAiorZUHaTinC=524897744;if (heCDMZsjFglkZdJFSoAiorZUHaTinC == heCDMZsjFglkZdJFSoAiorZUHaTinC- 1 ) heCDMZsjFglkZdJFSoAiorZUHaTinC=1459721081; else heCDMZsjFglkZdJFSoAiorZUHaTinC=317230164;int puxIjUysrIUlyYMtIMkIgzvasYeWqX=968856769;if (puxIjUysrIUlyYMtIMkIgzvasYeWqX == puxIjUysrIUlyYMtIMkIgzvasYeWqX- 0 ) puxIjUysrIUlyYMtIMkIgzvasYeWqX=666382815; else puxIjUysrIUlyYMtIMkIgzvasYeWqX=848309523;if (puxIjUysrIUlyYMtIMkIgzvasYeWqX == puxIjUysrIUlyYMtIMkIgzvasYeWqX- 0 ) puxIjUysrIUlyYMtIMkIgzvasYeWqX=1912132462; else puxIjUysrIUlyYMtIMkIgzvasYeWqX=672120878;if (puxIjUysrIUlyYMtIMkIgzvasYeWqX == puxIjUysrIUlyYMtIMkIgzvasYeWqX- 0 ) puxIjUysrIUlyYMtIMkIgzvasYeWqX=1648385109; else puxIjUysrIUlyYMtIMkIgzvasYeWqX=1529164107;if (puxIjUysrIUlyYMtIMkIgzvasYeWqX == puxIjUysrIUlyYMtIMkIgzvasYeWqX- 1 ) puxIjUysrIUlyYMtIMkIgzvasYeWqX=697253643; else puxIjUysrIUlyYMtIMkIgzvasYeWqX=1006599358;if (puxIjUysrIUlyYMtIMkIgzvasYeWqX == puxIjUysrIUlyYMtIMkIgzvasYeWqX- 0 ) puxIjUysrIUlyYMtIMkIgzvasYeWqX=499022079; else puxIjUysrIUlyYMtIMkIgzvasYeWqX=1987445296;if (puxIjUysrIUlyYMtIMkIgzvasYeWqX == puxIjUysrIUlyYMtIMkIgzvasYeWqX- 0 ) puxIjUysrIUlyYMtIMkIgzvasYeWqX=966419149; else puxIjUysrIUlyYMtIMkIgzvasYeWqX=1952911576;double HWVhBHCevHxRuzKYyfeSkoEGXKNRvv=110145719.532737179262664727346657587591;if (HWVhBHCevHxRuzKYyfeSkoEGXKNRvv == HWVhBHCevHxRuzKYyfeSkoEGXKNRvv ) HWVhBHCevHxRuzKYyfeSkoEGXKNRvv=1176285322.524071191073450450614207508626; else HWVhBHCevHxRuzKYyfeSkoEGXKNRvv=2092226990.179985879364747431823708842857;if (HWVhBHCevHxRuzKYyfeSkoEGXKNRvv == HWVhBHCevHxRuzKYyfeSkoEGXKNRvv ) HWVhBHCevHxRuzKYyfeSkoEGXKNRvv=745932489.367601458669236057028881348215; else HWVhBHCevHxRuzKYyfeSkoEGXKNRvv=655121793.313210824608436567289683069636;if (HWVhBHCevHxRuzKYyfeSkoEGXKNRvv == HWVhBHCevHxRuzKYyfeSkoEGXKNRvv ) HWVhBHCevHxRuzKYyfeSkoEGXKNRvv=686433123.435335703634343156698104683036; else HWVhBHCevHxRuzKYyfeSkoEGXKNRvv=1533496040.220937410197575534101414933186;if (HWVhBHCevHxRuzKYyfeSkoEGXKNRvv == HWVhBHCevHxRuzKYyfeSkoEGXKNRvv ) HWVhBHCevHxRuzKYyfeSkoEGXKNRvv=1126952031.377751552684651274916904592610; else HWVhBHCevHxRuzKYyfeSkoEGXKNRvv=1477851486.534492082518352786377927148436;if (HWVhBHCevHxRuzKYyfeSkoEGXKNRvv == HWVhBHCevHxRuzKYyfeSkoEGXKNRvv ) HWVhBHCevHxRuzKYyfeSkoEGXKNRvv=2133687995.995225137029251875282744461746; else HWVhBHCevHxRuzKYyfeSkoEGXKNRvv=846022540.476762414876046878647425536296;if (HWVhBHCevHxRuzKYyfeSkoEGXKNRvv == HWVhBHCevHxRuzKYyfeSkoEGXKNRvv ) HWVhBHCevHxRuzKYyfeSkoEGXKNRvv=1982768516.449266460946252780969411915123; else HWVhBHCevHxRuzKYyfeSkoEGXKNRvv=1245867464.733843798042980238496633813835;double PUiMJQSFuyvItIlZSFksklSmBvwAMw=531305413.245211856111885886337673694289;if (PUiMJQSFuyvItIlZSFksklSmBvwAMw == PUiMJQSFuyvItIlZSFksklSmBvwAMw ) PUiMJQSFuyvItIlZSFksklSmBvwAMw=1008866331.146377700556193660934111678362; else PUiMJQSFuyvItIlZSFksklSmBvwAMw=316996763.878807562829698158104436514680;if (PUiMJQSFuyvItIlZSFksklSmBvwAMw == PUiMJQSFuyvItIlZSFksklSmBvwAMw ) PUiMJQSFuyvItIlZSFksklSmBvwAMw=908192772.044879939620911552352036944330; else PUiMJQSFuyvItIlZSFksklSmBvwAMw=934463279.409298582033059462517243902804;if (PUiMJQSFuyvItIlZSFksklSmBvwAMw == PUiMJQSFuyvItIlZSFksklSmBvwAMw ) PUiMJQSFuyvItIlZSFksklSmBvwAMw=265200582.561032173308142018759293336649; else PUiMJQSFuyvItIlZSFksklSmBvwAMw=923802888.779957203740021920290425538871;if (PUiMJQSFuyvItIlZSFksklSmBvwAMw == PUiMJQSFuyvItIlZSFksklSmBvwAMw ) PUiMJQSFuyvItIlZSFksklSmBvwAMw=302157435.653574929581602937342756936442; else PUiMJQSFuyvItIlZSFksklSmBvwAMw=1584706951.053229755810171982461163001864;if (PUiMJQSFuyvItIlZSFksklSmBvwAMw == PUiMJQSFuyvItIlZSFksklSmBvwAMw ) PUiMJQSFuyvItIlZSFksklSmBvwAMw=850672605.868864849338354268505518299132; else PUiMJQSFuyvItIlZSFksklSmBvwAMw=2105023568.909106242814314391085346935168;if (PUiMJQSFuyvItIlZSFksklSmBvwAMw == PUiMJQSFuyvItIlZSFksklSmBvwAMw ) PUiMJQSFuyvItIlZSFksklSmBvwAMw=844105292.582854925902779231590826766328; else PUiMJQSFuyvItIlZSFksklSmBvwAMw=585416191.129443356147718477781577877610;long ezoqsROSfqmYVbOIDnTvvfThgcFggf=2111738274;if (ezoqsROSfqmYVbOIDnTvvfThgcFggf == ezoqsROSfqmYVbOIDnTvvfThgcFggf- 0 ) ezoqsROSfqmYVbOIDnTvvfThgcFggf=1544308880; else ezoqsROSfqmYVbOIDnTvvfThgcFggf=1349575362;if (ezoqsROSfqmYVbOIDnTvvfThgcFggf == ezoqsROSfqmYVbOIDnTvvfThgcFggf- 0 ) ezoqsROSfqmYVbOIDnTvvfThgcFggf=1288428489; else ezoqsROSfqmYVbOIDnTvvfThgcFggf=636859822;if (ezoqsROSfqmYVbOIDnTvvfThgcFggf == ezoqsROSfqmYVbOIDnTvvfThgcFggf- 0 ) ezoqsROSfqmYVbOIDnTvvfThgcFggf=411040189; else ezoqsROSfqmYVbOIDnTvvfThgcFggf=632764045;if (ezoqsROSfqmYVbOIDnTvvfThgcFggf == ezoqsROSfqmYVbOIDnTvvfThgcFggf- 1 ) ezoqsROSfqmYVbOIDnTvvfThgcFggf=282423646; else ezoqsROSfqmYVbOIDnTvvfThgcFggf=1280370422;if (ezoqsROSfqmYVbOIDnTvvfThgcFggf == ezoqsROSfqmYVbOIDnTvvfThgcFggf- 1 ) ezoqsROSfqmYVbOIDnTvvfThgcFggf=1868841139; else ezoqsROSfqmYVbOIDnTvvfThgcFggf=434280976;if (ezoqsROSfqmYVbOIDnTvvfThgcFggf == ezoqsROSfqmYVbOIDnTvvfThgcFggf- 0 ) ezoqsROSfqmYVbOIDnTvvfThgcFggf=685916530; else ezoqsROSfqmYVbOIDnTvvfThgcFggf=802362569;int QWeAkvLNMLjllttfFPtEjgbIdeVVZJ=1029466319;if (QWeAkvLNMLjllttfFPtEjgbIdeVVZJ == QWeAkvLNMLjllttfFPtEjgbIdeVVZJ- 1 ) QWeAkvLNMLjllttfFPtEjgbIdeVVZJ=1797832553; else QWeAkvLNMLjllttfFPtEjgbIdeVVZJ=435113736;if (QWeAkvLNMLjllttfFPtEjgbIdeVVZJ == QWeAkvLNMLjllttfFPtEjgbIdeVVZJ- 0 ) QWeAkvLNMLjllttfFPtEjgbIdeVVZJ=944045245; else QWeAkvLNMLjllttfFPtEjgbIdeVVZJ=1731070067;if (QWeAkvLNMLjllttfFPtEjgbIdeVVZJ == QWeAkvLNMLjllttfFPtEjgbIdeVVZJ- 0 ) QWeAkvLNMLjllttfFPtEjgbIdeVVZJ=1674676594; else QWeAkvLNMLjllttfFPtEjgbIdeVVZJ=193609232;if (QWeAkvLNMLjllttfFPtEjgbIdeVVZJ == QWeAkvLNMLjllttfFPtEjgbIdeVVZJ- 1 ) QWeAkvLNMLjllttfFPtEjgbIdeVVZJ=1992428784; else QWeAkvLNMLjllttfFPtEjgbIdeVVZJ=252482434;if (QWeAkvLNMLjllttfFPtEjgbIdeVVZJ == QWeAkvLNMLjllttfFPtEjgbIdeVVZJ- 0 ) QWeAkvLNMLjllttfFPtEjgbIdeVVZJ=1790069555; else QWeAkvLNMLjllttfFPtEjgbIdeVVZJ=1308709185;if (QWeAkvLNMLjllttfFPtEjgbIdeVVZJ == QWeAkvLNMLjllttfFPtEjgbIdeVVZJ- 0 ) QWeAkvLNMLjllttfFPtEjgbIdeVVZJ=1951932835; else QWeAkvLNMLjllttfFPtEjgbIdeVVZJ=728893202;float FiseSXTQIJDEnegvnNizFhHnexOQDD=1207228671.896583411758655090122708186559f;if (FiseSXTQIJDEnegvnNizFhHnexOQDD - FiseSXTQIJDEnegvnNizFhHnexOQDD> 0.00000001 ) FiseSXTQIJDEnegvnNizFhHnexOQDD=184143952.256424292661328801170030620940f; else FiseSXTQIJDEnegvnNizFhHnexOQDD=2018513889.706250234291866148048549033690f;if (FiseSXTQIJDEnegvnNizFhHnexOQDD - FiseSXTQIJDEnegvnNizFhHnexOQDD> 0.00000001 ) FiseSXTQIJDEnegvnNizFhHnexOQDD=1849042231.435297517968808927558201236833f; else FiseSXTQIJDEnegvnNizFhHnexOQDD=1708201211.764487777064119185111624334416f;if (FiseSXTQIJDEnegvnNizFhHnexOQDD - FiseSXTQIJDEnegvnNizFhHnexOQDD> 0.00000001 ) FiseSXTQIJDEnegvnNizFhHnexOQDD=328610918.188520542236050519226273225026f; else FiseSXTQIJDEnegvnNizFhHnexOQDD=1700236568.336299670974499667582630101631f;if (FiseSXTQIJDEnegvnNizFhHnexOQDD - FiseSXTQIJDEnegvnNizFhHnexOQDD> 0.00000001 ) FiseSXTQIJDEnegvnNizFhHnexOQDD=120265224.653790097105946299466864481719f; else FiseSXTQIJDEnegvnNizFhHnexOQDD=376102286.904942270077159879492337589932f;if (FiseSXTQIJDEnegvnNizFhHnexOQDD - FiseSXTQIJDEnegvnNizFhHnexOQDD> 0.00000001 ) FiseSXTQIJDEnegvnNizFhHnexOQDD=34573117.949831100412950982672233867756f; else FiseSXTQIJDEnegvnNizFhHnexOQDD=1775305997.335914932877018159871837081617f;if (FiseSXTQIJDEnegvnNizFhHnexOQDD - FiseSXTQIJDEnegvnNizFhHnexOQDD> 0.00000001 ) FiseSXTQIJDEnegvnNizFhHnexOQDD=1213105787.446418704705081633572908661903f; else FiseSXTQIJDEnegvnNizFhHnexOQDD=1978403253.342553160145517696656925554996f;float rzDSiOYPmZgLVOkTwFwjaManYInAzt=1399286426.550106921944184255254020746118f;if (rzDSiOYPmZgLVOkTwFwjaManYInAzt - rzDSiOYPmZgLVOkTwFwjaManYInAzt> 0.00000001 ) rzDSiOYPmZgLVOkTwFwjaManYInAzt=2008717136.679439953739398584584043964046f; else rzDSiOYPmZgLVOkTwFwjaManYInAzt=1962122620.092338575640353583227364573668f;if (rzDSiOYPmZgLVOkTwFwjaManYInAzt - rzDSiOYPmZgLVOkTwFwjaManYInAzt> 0.00000001 ) rzDSiOYPmZgLVOkTwFwjaManYInAzt=935067285.592774817111905479148193084987f; else rzDSiOYPmZgLVOkTwFwjaManYInAzt=1688044118.502815979988712646911462051089f;if (rzDSiOYPmZgLVOkTwFwjaManYInAzt - rzDSiOYPmZgLVOkTwFwjaManYInAzt> 0.00000001 ) rzDSiOYPmZgLVOkTwFwjaManYInAzt=2129897726.003172010051127159145907624096f; else rzDSiOYPmZgLVOkTwFwjaManYInAzt=1078292168.509008575174607034690295521655f;if (rzDSiOYPmZgLVOkTwFwjaManYInAzt - rzDSiOYPmZgLVOkTwFwjaManYInAzt> 0.00000001 ) rzDSiOYPmZgLVOkTwFwjaManYInAzt=437061680.812466223979517131169141631558f; else rzDSiOYPmZgLVOkTwFwjaManYInAzt=1350699648.892047234191273289850169687284f;if (rzDSiOYPmZgLVOkTwFwjaManYInAzt - rzDSiOYPmZgLVOkTwFwjaManYInAzt> 0.00000001 ) rzDSiOYPmZgLVOkTwFwjaManYInAzt=1504630225.578628757191708908855380873447f; else rzDSiOYPmZgLVOkTwFwjaManYInAzt=1035946055.005172365564276046188044030986f;if (rzDSiOYPmZgLVOkTwFwjaManYInAzt - rzDSiOYPmZgLVOkTwFwjaManYInAzt> 0.00000001 ) rzDSiOYPmZgLVOkTwFwjaManYInAzt=994273914.632120774664349211594836916114f; else rzDSiOYPmZgLVOkTwFwjaManYInAzt=1249814651.359003216747362570593950281631f;long dexNGaSZhwxdoYjYkTRXfdPIVBrnFN=1327305299;if (dexNGaSZhwxdoYjYkTRXfdPIVBrnFN == dexNGaSZhwxdoYjYkTRXfdPIVBrnFN- 0 ) dexNGaSZhwxdoYjYkTRXfdPIVBrnFN=1079587344; else dexNGaSZhwxdoYjYkTRXfdPIVBrnFN=1626171506;if (dexNGaSZhwxdoYjYkTRXfdPIVBrnFN == dexNGaSZhwxdoYjYkTRXfdPIVBrnFN- 0 ) dexNGaSZhwxdoYjYkTRXfdPIVBrnFN=618218617; else dexNGaSZhwxdoYjYkTRXfdPIVBrnFN=1822848996;if (dexNGaSZhwxdoYjYkTRXfdPIVBrnFN == dexNGaSZhwxdoYjYkTRXfdPIVBrnFN- 0 ) dexNGaSZhwxdoYjYkTRXfdPIVBrnFN=1897311242; else dexNGaSZhwxdoYjYkTRXfdPIVBrnFN=535588868;if (dexNGaSZhwxdoYjYkTRXfdPIVBrnFN == dexNGaSZhwxdoYjYkTRXfdPIVBrnFN- 1 ) dexNGaSZhwxdoYjYkTRXfdPIVBrnFN=700855401; else dexNGaSZhwxdoYjYkTRXfdPIVBrnFN=97595985;if (dexNGaSZhwxdoYjYkTRXfdPIVBrnFN == dexNGaSZhwxdoYjYkTRXfdPIVBrnFN- 0 ) dexNGaSZhwxdoYjYkTRXfdPIVBrnFN=314848512; else dexNGaSZhwxdoYjYkTRXfdPIVBrnFN=567942959;if (dexNGaSZhwxdoYjYkTRXfdPIVBrnFN == dexNGaSZhwxdoYjYkTRXfdPIVBrnFN- 0 ) dexNGaSZhwxdoYjYkTRXfdPIVBrnFN=1696496950; else dexNGaSZhwxdoYjYkTRXfdPIVBrnFN=2046854244;double sZumGQgcREGabJSnUUlLftPtiMnkUO=142791042.717357614955608026211980455698;if (sZumGQgcREGabJSnUUlLftPtiMnkUO == sZumGQgcREGabJSnUUlLftPtiMnkUO ) sZumGQgcREGabJSnUUlLftPtiMnkUO=1417881472.463215546737545727301939293891; else sZumGQgcREGabJSnUUlLftPtiMnkUO=552646409.302760240358158445583840753413;if (sZumGQgcREGabJSnUUlLftPtiMnkUO == sZumGQgcREGabJSnUUlLftPtiMnkUO ) sZumGQgcREGabJSnUUlLftPtiMnkUO=867659713.376654598559805389185931857301; else sZumGQgcREGabJSnUUlLftPtiMnkUO=874751775.893718351595814414548444845580;if (sZumGQgcREGabJSnUUlLftPtiMnkUO == sZumGQgcREGabJSnUUlLftPtiMnkUO ) sZumGQgcREGabJSnUUlLftPtiMnkUO=1753952202.729777710714979022845496456007; else sZumGQgcREGabJSnUUlLftPtiMnkUO=1961616808.498197715781584567583374634443;if (sZumGQgcREGabJSnUUlLftPtiMnkUO == sZumGQgcREGabJSnUUlLftPtiMnkUO ) sZumGQgcREGabJSnUUlLftPtiMnkUO=1224814592.271343688241817501309952422466; else sZumGQgcREGabJSnUUlLftPtiMnkUO=1253335431.438262035298035356824314443993;if (sZumGQgcREGabJSnUUlLftPtiMnkUO == sZumGQgcREGabJSnUUlLftPtiMnkUO ) sZumGQgcREGabJSnUUlLftPtiMnkUO=1581452209.133391476557712203827664213483; else sZumGQgcREGabJSnUUlLftPtiMnkUO=498342583.914316574499137771335127658929;if (sZumGQgcREGabJSnUUlLftPtiMnkUO == sZumGQgcREGabJSnUUlLftPtiMnkUO ) sZumGQgcREGabJSnUUlLftPtiMnkUO=203570986.919375351773954863854326990876; else sZumGQgcREGabJSnUUlLftPtiMnkUO=1440148510.976659197353077238480527570448;long cqZpzjMvYokdYCJMYtmjJVqeRYvHsv=481042673;if (cqZpzjMvYokdYCJMYtmjJVqeRYvHsv == cqZpzjMvYokdYCJMYtmjJVqeRYvHsv- 0 ) cqZpzjMvYokdYCJMYtmjJVqeRYvHsv=1184326323; else cqZpzjMvYokdYCJMYtmjJVqeRYvHsv=2033284135;if (cqZpzjMvYokdYCJMYtmjJVqeRYvHsv == cqZpzjMvYokdYCJMYtmjJVqeRYvHsv- 0 ) cqZpzjMvYokdYCJMYtmjJVqeRYvHsv=898815909; else cqZpzjMvYokdYCJMYtmjJVqeRYvHsv=759198499;if (cqZpzjMvYokdYCJMYtmjJVqeRYvHsv == cqZpzjMvYokdYCJMYtmjJVqeRYvHsv- 1 ) cqZpzjMvYokdYCJMYtmjJVqeRYvHsv=1029059896; else cqZpzjMvYokdYCJMYtmjJVqeRYvHsv=215462364;if (cqZpzjMvYokdYCJMYtmjJVqeRYvHsv == cqZpzjMvYokdYCJMYtmjJVqeRYvHsv- 1 ) cqZpzjMvYokdYCJMYtmjJVqeRYvHsv=1649458831; else cqZpzjMvYokdYCJMYtmjJVqeRYvHsv=1473128268;if (cqZpzjMvYokdYCJMYtmjJVqeRYvHsv == cqZpzjMvYokdYCJMYtmjJVqeRYvHsv- 0 ) cqZpzjMvYokdYCJMYtmjJVqeRYvHsv=1261267219; else cqZpzjMvYokdYCJMYtmjJVqeRYvHsv=837471300;if (cqZpzjMvYokdYCJMYtmjJVqeRYvHsv == cqZpzjMvYokdYCJMYtmjJVqeRYvHsv- 0 ) cqZpzjMvYokdYCJMYtmjJVqeRYvHsv=1373195606; else cqZpzjMvYokdYCJMYtmjJVqeRYvHsv=1561954767;double slVZfuOlJBWelqSfWSSmMZpSdkTaTB=1490604694.823652344189869486889960946928;if (slVZfuOlJBWelqSfWSSmMZpSdkTaTB == slVZfuOlJBWelqSfWSSmMZpSdkTaTB ) slVZfuOlJBWelqSfWSSmMZpSdkTaTB=1021349979.843530066473786608518972397471; else slVZfuOlJBWelqSfWSSmMZpSdkTaTB=851482872.743635734043051729544941098234;if (slVZfuOlJBWelqSfWSSmMZpSdkTaTB == slVZfuOlJBWelqSfWSSmMZpSdkTaTB ) slVZfuOlJBWelqSfWSSmMZpSdkTaTB=1979416826.479583115108567602167894881264; else slVZfuOlJBWelqSfWSSmMZpSdkTaTB=275668448.850068457862445749450628868680;if (slVZfuOlJBWelqSfWSSmMZpSdkTaTB == slVZfuOlJBWelqSfWSSmMZpSdkTaTB ) slVZfuOlJBWelqSfWSSmMZpSdkTaTB=833354378.396145961925631646915392524330; else slVZfuOlJBWelqSfWSSmMZpSdkTaTB=1863711546.197867640199853509722278611462;if (slVZfuOlJBWelqSfWSSmMZpSdkTaTB == slVZfuOlJBWelqSfWSSmMZpSdkTaTB ) slVZfuOlJBWelqSfWSSmMZpSdkTaTB=1720836744.164194898411692654913661902025; else slVZfuOlJBWelqSfWSSmMZpSdkTaTB=1265454183.728913687072242179607866061330;if (slVZfuOlJBWelqSfWSSmMZpSdkTaTB == slVZfuOlJBWelqSfWSSmMZpSdkTaTB ) slVZfuOlJBWelqSfWSSmMZpSdkTaTB=401356914.297379489018805142083352997295; else slVZfuOlJBWelqSfWSSmMZpSdkTaTB=1512366833.346888697489170016202935704559;if (slVZfuOlJBWelqSfWSSmMZpSdkTaTB == slVZfuOlJBWelqSfWSSmMZpSdkTaTB ) slVZfuOlJBWelqSfWSSmMZpSdkTaTB=241396787.220477213765570539309119441119; else slVZfuOlJBWelqSfWSSmMZpSdkTaTB=1398968659.515926080481465757638636012103;double IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ=1444397939.552281651267738967194958919216;if (IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ == IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ ) IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ=1020155530.445620492356752584262682071538; else IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ=1215454277.355692311957584931643605487376;if (IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ == IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ ) IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ=862527921.906511116138332620915190575032; else IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ=1242186604.604605118482551648487856291704;if (IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ == IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ ) IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ=1262693269.628564474751354365904865695942; else IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ=1402881224.451497410727484651303781187398;if (IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ == IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ ) IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ=1261214793.064330197896628849214575987041; else IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ=1202419108.012287425457451727445829569946;if (IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ == IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ ) IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ=1050671225.416788545153486812526161630483; else IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ=1542015092.958914188339340355386853136616;if (IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ == IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ ) IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ=1760968203.916220567273429892248511720699; else IwTLaAeZTWWtuKtlbZhxUbqGrmwyDZ=916480902.958342313099821344435457851661;double shrOJaDpjYozHdumuAUmiLeUdAEdxG=1043761126.161870834140007284832827973667;if (shrOJaDpjYozHdumuAUmiLeUdAEdxG == shrOJaDpjYozHdumuAUmiLeUdAEdxG ) shrOJaDpjYozHdumuAUmiLeUdAEdxG=275630027.507313515023420475400921935500; else shrOJaDpjYozHdumuAUmiLeUdAEdxG=860449939.838874480577081950751927769749;if (shrOJaDpjYozHdumuAUmiLeUdAEdxG == shrOJaDpjYozHdumuAUmiLeUdAEdxG ) shrOJaDpjYozHdumuAUmiLeUdAEdxG=418798531.912530666609603405008585002112; else shrOJaDpjYozHdumuAUmiLeUdAEdxG=2001963429.684166665239525232446475154764;if (shrOJaDpjYozHdumuAUmiLeUdAEdxG == shrOJaDpjYozHdumuAUmiLeUdAEdxG ) shrOJaDpjYozHdumuAUmiLeUdAEdxG=1186001041.856413654894087907187078622012; else shrOJaDpjYozHdumuAUmiLeUdAEdxG=1419602628.291148411379876259825878899990;if (shrOJaDpjYozHdumuAUmiLeUdAEdxG == shrOJaDpjYozHdumuAUmiLeUdAEdxG ) shrOJaDpjYozHdumuAUmiLeUdAEdxG=1795171942.274360404069682820348339853436; else shrOJaDpjYozHdumuAUmiLeUdAEdxG=1468098424.063221662695597629617966204551;if (shrOJaDpjYozHdumuAUmiLeUdAEdxG == shrOJaDpjYozHdumuAUmiLeUdAEdxG ) shrOJaDpjYozHdumuAUmiLeUdAEdxG=312447391.817800757227975384323482020703; else shrOJaDpjYozHdumuAUmiLeUdAEdxG=1780845373.711199979708350110102134854699;if (shrOJaDpjYozHdumuAUmiLeUdAEdxG == shrOJaDpjYozHdumuAUmiLeUdAEdxG ) shrOJaDpjYozHdumuAUmiLeUdAEdxG=357841585.105562689751673623302170179835; else shrOJaDpjYozHdumuAUmiLeUdAEdxG=1664176710.549170380953146257233139804650;long rXUKFJXzQOTSXZqyMXuGKfBHVCyRic=2000560328;if (rXUKFJXzQOTSXZqyMXuGKfBHVCyRic == rXUKFJXzQOTSXZqyMXuGKfBHVCyRic- 0 ) rXUKFJXzQOTSXZqyMXuGKfBHVCyRic=1445837969; else rXUKFJXzQOTSXZqyMXuGKfBHVCyRic=2076281314;if (rXUKFJXzQOTSXZqyMXuGKfBHVCyRic == rXUKFJXzQOTSXZqyMXuGKfBHVCyRic- 1 ) rXUKFJXzQOTSXZqyMXuGKfBHVCyRic=541799237; else rXUKFJXzQOTSXZqyMXuGKfBHVCyRic=313824588;if (rXUKFJXzQOTSXZqyMXuGKfBHVCyRic == rXUKFJXzQOTSXZqyMXuGKfBHVCyRic- 0 ) rXUKFJXzQOTSXZqyMXuGKfBHVCyRic=1822646809; else rXUKFJXzQOTSXZqyMXuGKfBHVCyRic=1128009599;if (rXUKFJXzQOTSXZqyMXuGKfBHVCyRic == rXUKFJXzQOTSXZqyMXuGKfBHVCyRic- 1 ) rXUKFJXzQOTSXZqyMXuGKfBHVCyRic=227911425; else rXUKFJXzQOTSXZqyMXuGKfBHVCyRic=1765952402;if (rXUKFJXzQOTSXZqyMXuGKfBHVCyRic == rXUKFJXzQOTSXZqyMXuGKfBHVCyRic- 1 ) rXUKFJXzQOTSXZqyMXuGKfBHVCyRic=1737751726; else rXUKFJXzQOTSXZqyMXuGKfBHVCyRic=776997718;if (rXUKFJXzQOTSXZqyMXuGKfBHVCyRic == rXUKFJXzQOTSXZqyMXuGKfBHVCyRic- 1 ) rXUKFJXzQOTSXZqyMXuGKfBHVCyRic=1372480870; else rXUKFJXzQOTSXZqyMXuGKfBHVCyRic=1438822933;int JqfEVGvgVTUgimWHOIdfVulZVlueIy=738497089;if (JqfEVGvgVTUgimWHOIdfVulZVlueIy == JqfEVGvgVTUgimWHOIdfVulZVlueIy- 1 ) JqfEVGvgVTUgimWHOIdfVulZVlueIy=1788310267; else JqfEVGvgVTUgimWHOIdfVulZVlueIy=1345119308;if (JqfEVGvgVTUgimWHOIdfVulZVlueIy == JqfEVGvgVTUgimWHOIdfVulZVlueIy- 1 ) JqfEVGvgVTUgimWHOIdfVulZVlueIy=1123338632; else JqfEVGvgVTUgimWHOIdfVulZVlueIy=1619051590;if (JqfEVGvgVTUgimWHOIdfVulZVlueIy == JqfEVGvgVTUgimWHOIdfVulZVlueIy- 0 ) JqfEVGvgVTUgimWHOIdfVulZVlueIy=989392007; else JqfEVGvgVTUgimWHOIdfVulZVlueIy=1684637196;if (JqfEVGvgVTUgimWHOIdfVulZVlueIy == JqfEVGvgVTUgimWHOIdfVulZVlueIy- 0 ) JqfEVGvgVTUgimWHOIdfVulZVlueIy=258396898; else JqfEVGvgVTUgimWHOIdfVulZVlueIy=1748036549;if (JqfEVGvgVTUgimWHOIdfVulZVlueIy == JqfEVGvgVTUgimWHOIdfVulZVlueIy- 0 ) JqfEVGvgVTUgimWHOIdfVulZVlueIy=1567765982; else JqfEVGvgVTUgimWHOIdfVulZVlueIy=1541913144;if (JqfEVGvgVTUgimWHOIdfVulZVlueIy == JqfEVGvgVTUgimWHOIdfVulZVlueIy- 0 ) JqfEVGvgVTUgimWHOIdfVulZVlueIy=929739213; else JqfEVGvgVTUgimWHOIdfVulZVlueIy=1629331543;int dHGaHdEgpTAyxAogfHMeHNXvSSBPct=800181053;if (dHGaHdEgpTAyxAogfHMeHNXvSSBPct == dHGaHdEgpTAyxAogfHMeHNXvSSBPct- 1 ) dHGaHdEgpTAyxAogfHMeHNXvSSBPct=400904706; else dHGaHdEgpTAyxAogfHMeHNXvSSBPct=1907891522;if (dHGaHdEgpTAyxAogfHMeHNXvSSBPct == dHGaHdEgpTAyxAogfHMeHNXvSSBPct- 0 ) dHGaHdEgpTAyxAogfHMeHNXvSSBPct=1618126580; else dHGaHdEgpTAyxAogfHMeHNXvSSBPct=659623535;if (dHGaHdEgpTAyxAogfHMeHNXvSSBPct == dHGaHdEgpTAyxAogfHMeHNXvSSBPct- 1 ) dHGaHdEgpTAyxAogfHMeHNXvSSBPct=1268544235; else dHGaHdEgpTAyxAogfHMeHNXvSSBPct=1557715625;if (dHGaHdEgpTAyxAogfHMeHNXvSSBPct == dHGaHdEgpTAyxAogfHMeHNXvSSBPct- 1 ) dHGaHdEgpTAyxAogfHMeHNXvSSBPct=1368597964; else dHGaHdEgpTAyxAogfHMeHNXvSSBPct=1295104075;if (dHGaHdEgpTAyxAogfHMeHNXvSSBPct == dHGaHdEgpTAyxAogfHMeHNXvSSBPct- 1 ) dHGaHdEgpTAyxAogfHMeHNXvSSBPct=632538514; else dHGaHdEgpTAyxAogfHMeHNXvSSBPct=800329776;if (dHGaHdEgpTAyxAogfHMeHNXvSSBPct == dHGaHdEgpTAyxAogfHMeHNXvSSBPct- 0 ) dHGaHdEgpTAyxAogfHMeHNXvSSBPct=264211703; else dHGaHdEgpTAyxAogfHMeHNXvSSBPct=409003323;long NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv=1655575196;if (NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv == NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv- 1 ) NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv=1103812392; else NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv=1766828143;if (NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv == NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv- 1 ) NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv=2023350090; else NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv=776429916;if (NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv == NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv- 1 ) NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv=1274635187; else NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv=1669542443;if (NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv == NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv- 0 ) NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv=1011168321; else NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv=1908684420;if (NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv == NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv- 0 ) NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv=1817884416; else NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv=941246335;if (NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv == NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv- 0 ) NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv=893542253; else NUzCqvJyCpMUiSqAuIYjOdIiGgCcLv=280733665;double HYQTIqNcHckGKchlsjCySXojcrUIXQ=2140713643.129384428459455077201580447476;if (HYQTIqNcHckGKchlsjCySXojcrUIXQ == HYQTIqNcHckGKchlsjCySXojcrUIXQ ) HYQTIqNcHckGKchlsjCySXojcrUIXQ=1696127834.496520260415022678646050075095; else HYQTIqNcHckGKchlsjCySXojcrUIXQ=572299817.716217303681789667997084144756;if (HYQTIqNcHckGKchlsjCySXojcrUIXQ == HYQTIqNcHckGKchlsjCySXojcrUIXQ ) HYQTIqNcHckGKchlsjCySXojcrUIXQ=1037918227.637571091638516538151921884227; else HYQTIqNcHckGKchlsjCySXojcrUIXQ=342335468.376716432978646057871761572780;if (HYQTIqNcHckGKchlsjCySXojcrUIXQ == HYQTIqNcHckGKchlsjCySXojcrUIXQ ) HYQTIqNcHckGKchlsjCySXojcrUIXQ=1333712909.887437381111739439593756671534; else HYQTIqNcHckGKchlsjCySXojcrUIXQ=17409446.486229121987776114214429714297;if (HYQTIqNcHckGKchlsjCySXojcrUIXQ == HYQTIqNcHckGKchlsjCySXojcrUIXQ ) HYQTIqNcHckGKchlsjCySXojcrUIXQ=1617436501.786774652042862618066742016761; else HYQTIqNcHckGKchlsjCySXojcrUIXQ=230594642.694430100357338366811633105576;if (HYQTIqNcHckGKchlsjCySXojcrUIXQ == HYQTIqNcHckGKchlsjCySXojcrUIXQ ) HYQTIqNcHckGKchlsjCySXojcrUIXQ=2124004991.809146844251871051586095117143; else HYQTIqNcHckGKchlsjCySXojcrUIXQ=1975718204.673556829252276186326410181514;if (HYQTIqNcHckGKchlsjCySXojcrUIXQ == HYQTIqNcHckGKchlsjCySXojcrUIXQ ) HYQTIqNcHckGKchlsjCySXojcrUIXQ=986513076.971269842533333483903193736448; else HYQTIqNcHckGKchlsjCySXojcrUIXQ=189062231.642548211844480319880861288953; }
 HYQTIqNcHckGKchlsjCySXojcrUIXQy::HYQTIqNcHckGKchlsjCySXojcrUIXQy()
 { this->ZdUDHEMFehsA("dmYaqGwNUDQqJJbafIuxUGMijjgqRWZdUDHEMFehsAj", true, 1420425493, 1996877222, 1700914791); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class YBSQrDuaeXyYFFmNFulZQBuYszzmCay
 { 
public: bool CYifxjZWhwqXucGrEGbxIyweXGtYrE; double CYifxjZWhwqXucGrEGbxIyweXGtYrEYBSQrDuaeXyYFFmNFulZQBuYszzmCa; YBSQrDuaeXyYFFmNFulZQBuYszzmCay(); void QwEdkTTaxlEv(string CYifxjZWhwqXucGrEGbxIyweXGtYrEQwEdkTTaxlEv, bool IXEsvAWZpegXDdZdAwlIvfbcWEbDIq, int pAfdoSTBgGUeRwRluSFtGxmjIcwbWr, float YSRyofsZpGKyPfnROeLlJJyOjsOwVL, long WrfpiBNSIlsUdziIhGvrOSQliPUCPv);
 protected: bool CYifxjZWhwqXucGrEGbxIyweXGtYrEo; double CYifxjZWhwqXucGrEGbxIyweXGtYrEYBSQrDuaeXyYFFmNFulZQBuYszzmCaf; void QwEdkTTaxlEvu(string CYifxjZWhwqXucGrEGbxIyweXGtYrEQwEdkTTaxlEvg, bool IXEsvAWZpegXDdZdAwlIvfbcWEbDIqe, int pAfdoSTBgGUeRwRluSFtGxmjIcwbWrr, float YSRyofsZpGKyPfnROeLlJJyOjsOwVLw, long WrfpiBNSIlsUdziIhGvrOSQliPUCPvn);
 private: bool CYifxjZWhwqXucGrEGbxIyweXGtYrEIXEsvAWZpegXDdZdAwlIvfbcWEbDIq; double CYifxjZWhwqXucGrEGbxIyweXGtYrEYSRyofsZpGKyPfnROeLlJJyOjsOwVLYBSQrDuaeXyYFFmNFulZQBuYszzmCa;
 void QwEdkTTaxlEvv(string IXEsvAWZpegXDdZdAwlIvfbcWEbDIqQwEdkTTaxlEv, bool IXEsvAWZpegXDdZdAwlIvfbcWEbDIqpAfdoSTBgGUeRwRluSFtGxmjIcwbWr, int pAfdoSTBgGUeRwRluSFtGxmjIcwbWrCYifxjZWhwqXucGrEGbxIyweXGtYrE, float YSRyofsZpGKyPfnROeLlJJyOjsOwVLWrfpiBNSIlsUdziIhGvrOSQliPUCPv, long WrfpiBNSIlsUdziIhGvrOSQliPUCPvIXEsvAWZpegXDdZdAwlIvfbcWEbDIq); };
 void YBSQrDuaeXyYFFmNFulZQBuYszzmCay::QwEdkTTaxlEv(string CYifxjZWhwqXucGrEGbxIyweXGtYrEQwEdkTTaxlEv, bool IXEsvAWZpegXDdZdAwlIvfbcWEbDIq, int pAfdoSTBgGUeRwRluSFtGxmjIcwbWr, float YSRyofsZpGKyPfnROeLlJJyOjsOwVL, long WrfpiBNSIlsUdziIhGvrOSQliPUCPv)
 { double fIZGedxUZKGQOlAFtfzUFdXttBGmYp=1351778848.390824614492083098987290280788;if (fIZGedxUZKGQOlAFtfzUFdXttBGmYp == fIZGedxUZKGQOlAFtfzUFdXttBGmYp ) fIZGedxUZKGQOlAFtfzUFdXttBGmYp=140551732.555576545744367053695546020065; else fIZGedxUZKGQOlAFtfzUFdXttBGmYp=1663779726.403704862756334303423719301956;if (fIZGedxUZKGQOlAFtfzUFdXttBGmYp == fIZGedxUZKGQOlAFtfzUFdXttBGmYp ) fIZGedxUZKGQOlAFtfzUFdXttBGmYp=571601851.758134177330204932273765280441; else fIZGedxUZKGQOlAFtfzUFdXttBGmYp=1185249707.391611930134719462658079409306;if (fIZGedxUZKGQOlAFtfzUFdXttBGmYp == fIZGedxUZKGQOlAFtfzUFdXttBGmYp ) fIZGedxUZKGQOlAFtfzUFdXttBGmYp=1741287347.573615069401224906692990442232; else fIZGedxUZKGQOlAFtfzUFdXttBGmYp=2073812548.263566121540325785550482469337;if (fIZGedxUZKGQOlAFtfzUFdXttBGmYp == fIZGedxUZKGQOlAFtfzUFdXttBGmYp ) fIZGedxUZKGQOlAFtfzUFdXttBGmYp=1793800977.493735886999525368676779289221; else fIZGedxUZKGQOlAFtfzUFdXttBGmYp=718222733.890426870000077207720276272555;if (fIZGedxUZKGQOlAFtfzUFdXttBGmYp == fIZGedxUZKGQOlAFtfzUFdXttBGmYp ) fIZGedxUZKGQOlAFtfzUFdXttBGmYp=508997709.667620011960868780319350162435; else fIZGedxUZKGQOlAFtfzUFdXttBGmYp=283361578.442554445729787142960325741766;if (fIZGedxUZKGQOlAFtfzUFdXttBGmYp == fIZGedxUZKGQOlAFtfzUFdXttBGmYp ) fIZGedxUZKGQOlAFtfzUFdXttBGmYp=1755046945.968662942434679077403168622781; else fIZGedxUZKGQOlAFtfzUFdXttBGmYp=645407577.479087213530063686688436491114;float HgHAfaeqMpdVynPDAbwYdFxNvUSfxf=1869714141.770335898663937162693869891777f;if (HgHAfaeqMpdVynPDAbwYdFxNvUSfxf - HgHAfaeqMpdVynPDAbwYdFxNvUSfxf> 0.00000001 ) HgHAfaeqMpdVynPDAbwYdFxNvUSfxf=2100722690.442043207007737475852392792264f; else HgHAfaeqMpdVynPDAbwYdFxNvUSfxf=159477116.513132290596569724242793615145f;if (HgHAfaeqMpdVynPDAbwYdFxNvUSfxf - HgHAfaeqMpdVynPDAbwYdFxNvUSfxf> 0.00000001 ) HgHAfaeqMpdVynPDAbwYdFxNvUSfxf=911451981.215360560628546684317902392052f; else HgHAfaeqMpdVynPDAbwYdFxNvUSfxf=1517314611.716539231026473526814284941317f;if (HgHAfaeqMpdVynPDAbwYdFxNvUSfxf - HgHAfaeqMpdVynPDAbwYdFxNvUSfxf> 0.00000001 ) HgHAfaeqMpdVynPDAbwYdFxNvUSfxf=727278197.852891211746349154088734964270f; else HgHAfaeqMpdVynPDAbwYdFxNvUSfxf=518010147.892428569231517905689844012804f;if (HgHAfaeqMpdVynPDAbwYdFxNvUSfxf - HgHAfaeqMpdVynPDAbwYdFxNvUSfxf> 0.00000001 ) HgHAfaeqMpdVynPDAbwYdFxNvUSfxf=1540505866.815205503270695733707079206851f; else HgHAfaeqMpdVynPDAbwYdFxNvUSfxf=1264310364.676643642250351936039011894610f;if (HgHAfaeqMpdVynPDAbwYdFxNvUSfxf - HgHAfaeqMpdVynPDAbwYdFxNvUSfxf> 0.00000001 ) HgHAfaeqMpdVynPDAbwYdFxNvUSfxf=297075790.076261516210255773938192276585f; else HgHAfaeqMpdVynPDAbwYdFxNvUSfxf=1598653568.913239519152182086612289209409f;if (HgHAfaeqMpdVynPDAbwYdFxNvUSfxf - HgHAfaeqMpdVynPDAbwYdFxNvUSfxf> 0.00000001 ) HgHAfaeqMpdVynPDAbwYdFxNvUSfxf=1732384501.503141703737488602020108383513f; else HgHAfaeqMpdVynPDAbwYdFxNvUSfxf=2005646613.011175004148499691160849051597f;int JxAkIbQHOhClvqHlBujuLVYOOiPELY=927687559;if (JxAkIbQHOhClvqHlBujuLVYOOiPELY == JxAkIbQHOhClvqHlBujuLVYOOiPELY- 1 ) JxAkIbQHOhClvqHlBujuLVYOOiPELY=836771896; else JxAkIbQHOhClvqHlBujuLVYOOiPELY=1963261142;if (JxAkIbQHOhClvqHlBujuLVYOOiPELY == JxAkIbQHOhClvqHlBujuLVYOOiPELY- 0 ) JxAkIbQHOhClvqHlBujuLVYOOiPELY=1818208039; else JxAkIbQHOhClvqHlBujuLVYOOiPELY=294264556;if (JxAkIbQHOhClvqHlBujuLVYOOiPELY == JxAkIbQHOhClvqHlBujuLVYOOiPELY- 0 ) JxAkIbQHOhClvqHlBujuLVYOOiPELY=881225905; else JxAkIbQHOhClvqHlBujuLVYOOiPELY=379838476;if (JxAkIbQHOhClvqHlBujuLVYOOiPELY == JxAkIbQHOhClvqHlBujuLVYOOiPELY- 1 ) JxAkIbQHOhClvqHlBujuLVYOOiPELY=55688435; else JxAkIbQHOhClvqHlBujuLVYOOiPELY=1716332826;if (JxAkIbQHOhClvqHlBujuLVYOOiPELY == JxAkIbQHOhClvqHlBujuLVYOOiPELY- 1 ) JxAkIbQHOhClvqHlBujuLVYOOiPELY=733349947; else JxAkIbQHOhClvqHlBujuLVYOOiPELY=421514634;if (JxAkIbQHOhClvqHlBujuLVYOOiPELY == JxAkIbQHOhClvqHlBujuLVYOOiPELY- 0 ) JxAkIbQHOhClvqHlBujuLVYOOiPELY=1874163746; else JxAkIbQHOhClvqHlBujuLVYOOiPELY=585140545;int KLkzxIUdclxCDqmgNfQxIpkqNxnNpr=1329960611;if (KLkzxIUdclxCDqmgNfQxIpkqNxnNpr == KLkzxIUdclxCDqmgNfQxIpkqNxnNpr- 1 ) KLkzxIUdclxCDqmgNfQxIpkqNxnNpr=556087770; else KLkzxIUdclxCDqmgNfQxIpkqNxnNpr=472201402;if (KLkzxIUdclxCDqmgNfQxIpkqNxnNpr == KLkzxIUdclxCDqmgNfQxIpkqNxnNpr- 0 ) KLkzxIUdclxCDqmgNfQxIpkqNxnNpr=149882489; else KLkzxIUdclxCDqmgNfQxIpkqNxnNpr=1165186425;if (KLkzxIUdclxCDqmgNfQxIpkqNxnNpr == KLkzxIUdclxCDqmgNfQxIpkqNxnNpr- 1 ) KLkzxIUdclxCDqmgNfQxIpkqNxnNpr=2067536590; else KLkzxIUdclxCDqmgNfQxIpkqNxnNpr=1208168513;if (KLkzxIUdclxCDqmgNfQxIpkqNxnNpr == KLkzxIUdclxCDqmgNfQxIpkqNxnNpr- 1 ) KLkzxIUdclxCDqmgNfQxIpkqNxnNpr=624500651; else KLkzxIUdclxCDqmgNfQxIpkqNxnNpr=1770025232;if (KLkzxIUdclxCDqmgNfQxIpkqNxnNpr == KLkzxIUdclxCDqmgNfQxIpkqNxnNpr- 1 ) KLkzxIUdclxCDqmgNfQxIpkqNxnNpr=123273860; else KLkzxIUdclxCDqmgNfQxIpkqNxnNpr=1454775134;if (KLkzxIUdclxCDqmgNfQxIpkqNxnNpr == KLkzxIUdclxCDqmgNfQxIpkqNxnNpr- 0 ) KLkzxIUdclxCDqmgNfQxIpkqNxnNpr=888173917; else KLkzxIUdclxCDqmgNfQxIpkqNxnNpr=142633779;long DPNVucNHyrAADfUjwxOHFdXpUyzNuE=668663068;if (DPNVucNHyrAADfUjwxOHFdXpUyzNuE == DPNVucNHyrAADfUjwxOHFdXpUyzNuE- 0 ) DPNVucNHyrAADfUjwxOHFdXpUyzNuE=1073397263; else DPNVucNHyrAADfUjwxOHFdXpUyzNuE=827359386;if (DPNVucNHyrAADfUjwxOHFdXpUyzNuE == DPNVucNHyrAADfUjwxOHFdXpUyzNuE- 0 ) DPNVucNHyrAADfUjwxOHFdXpUyzNuE=1956119328; else DPNVucNHyrAADfUjwxOHFdXpUyzNuE=1102955865;if (DPNVucNHyrAADfUjwxOHFdXpUyzNuE == DPNVucNHyrAADfUjwxOHFdXpUyzNuE- 1 ) DPNVucNHyrAADfUjwxOHFdXpUyzNuE=1655236754; else DPNVucNHyrAADfUjwxOHFdXpUyzNuE=1708724634;if (DPNVucNHyrAADfUjwxOHFdXpUyzNuE == DPNVucNHyrAADfUjwxOHFdXpUyzNuE- 1 ) DPNVucNHyrAADfUjwxOHFdXpUyzNuE=988488236; else DPNVucNHyrAADfUjwxOHFdXpUyzNuE=1720884214;if (DPNVucNHyrAADfUjwxOHFdXpUyzNuE == DPNVucNHyrAADfUjwxOHFdXpUyzNuE- 1 ) DPNVucNHyrAADfUjwxOHFdXpUyzNuE=103788681; else DPNVucNHyrAADfUjwxOHFdXpUyzNuE=1342602802;if (DPNVucNHyrAADfUjwxOHFdXpUyzNuE == DPNVucNHyrAADfUjwxOHFdXpUyzNuE- 0 ) DPNVucNHyrAADfUjwxOHFdXpUyzNuE=783964664; else DPNVucNHyrAADfUjwxOHFdXpUyzNuE=305763563;long lnSQXjycqhePkPdtJyVavmvDJnQvbi=97825904;if (lnSQXjycqhePkPdtJyVavmvDJnQvbi == lnSQXjycqhePkPdtJyVavmvDJnQvbi- 1 ) lnSQXjycqhePkPdtJyVavmvDJnQvbi=1328564790; else lnSQXjycqhePkPdtJyVavmvDJnQvbi=402423890;if (lnSQXjycqhePkPdtJyVavmvDJnQvbi == lnSQXjycqhePkPdtJyVavmvDJnQvbi- 1 ) lnSQXjycqhePkPdtJyVavmvDJnQvbi=1449558843; else lnSQXjycqhePkPdtJyVavmvDJnQvbi=1627903121;if (lnSQXjycqhePkPdtJyVavmvDJnQvbi == lnSQXjycqhePkPdtJyVavmvDJnQvbi- 0 ) lnSQXjycqhePkPdtJyVavmvDJnQvbi=1837935799; else lnSQXjycqhePkPdtJyVavmvDJnQvbi=192261537;if (lnSQXjycqhePkPdtJyVavmvDJnQvbi == lnSQXjycqhePkPdtJyVavmvDJnQvbi- 1 ) lnSQXjycqhePkPdtJyVavmvDJnQvbi=1007634616; else lnSQXjycqhePkPdtJyVavmvDJnQvbi=1776087030;if (lnSQXjycqhePkPdtJyVavmvDJnQvbi == lnSQXjycqhePkPdtJyVavmvDJnQvbi- 0 ) lnSQXjycqhePkPdtJyVavmvDJnQvbi=1338760491; else lnSQXjycqhePkPdtJyVavmvDJnQvbi=48182807;if (lnSQXjycqhePkPdtJyVavmvDJnQvbi == lnSQXjycqhePkPdtJyVavmvDJnQvbi- 1 ) lnSQXjycqhePkPdtJyVavmvDJnQvbi=170990696; else lnSQXjycqhePkPdtJyVavmvDJnQvbi=1573934418;int mPwbmJTqeFQhFMkjYifaVNDsdbroit=945195583;if (mPwbmJTqeFQhFMkjYifaVNDsdbroit == mPwbmJTqeFQhFMkjYifaVNDsdbroit- 1 ) mPwbmJTqeFQhFMkjYifaVNDsdbroit=1803396424; else mPwbmJTqeFQhFMkjYifaVNDsdbroit=1205500678;if (mPwbmJTqeFQhFMkjYifaVNDsdbroit == mPwbmJTqeFQhFMkjYifaVNDsdbroit- 1 ) mPwbmJTqeFQhFMkjYifaVNDsdbroit=1659226929; else mPwbmJTqeFQhFMkjYifaVNDsdbroit=650223712;if (mPwbmJTqeFQhFMkjYifaVNDsdbroit == mPwbmJTqeFQhFMkjYifaVNDsdbroit- 1 ) mPwbmJTqeFQhFMkjYifaVNDsdbroit=390614205; else mPwbmJTqeFQhFMkjYifaVNDsdbroit=1314616560;if (mPwbmJTqeFQhFMkjYifaVNDsdbroit == mPwbmJTqeFQhFMkjYifaVNDsdbroit- 1 ) mPwbmJTqeFQhFMkjYifaVNDsdbroit=1822207504; else mPwbmJTqeFQhFMkjYifaVNDsdbroit=994846783;if (mPwbmJTqeFQhFMkjYifaVNDsdbroit == mPwbmJTqeFQhFMkjYifaVNDsdbroit- 0 ) mPwbmJTqeFQhFMkjYifaVNDsdbroit=1631196813; else mPwbmJTqeFQhFMkjYifaVNDsdbroit=576481922;if (mPwbmJTqeFQhFMkjYifaVNDsdbroit == mPwbmJTqeFQhFMkjYifaVNDsdbroit- 0 ) mPwbmJTqeFQhFMkjYifaVNDsdbroit=1061397744; else mPwbmJTqeFQhFMkjYifaVNDsdbroit=724933788;int khCsNTGlhWQFWRoWEnlzCaEuJmftPy=1315392186;if (khCsNTGlhWQFWRoWEnlzCaEuJmftPy == khCsNTGlhWQFWRoWEnlzCaEuJmftPy- 1 ) khCsNTGlhWQFWRoWEnlzCaEuJmftPy=1668047868; else khCsNTGlhWQFWRoWEnlzCaEuJmftPy=790348013;if (khCsNTGlhWQFWRoWEnlzCaEuJmftPy == khCsNTGlhWQFWRoWEnlzCaEuJmftPy- 0 ) khCsNTGlhWQFWRoWEnlzCaEuJmftPy=961552636; else khCsNTGlhWQFWRoWEnlzCaEuJmftPy=2086487804;if (khCsNTGlhWQFWRoWEnlzCaEuJmftPy == khCsNTGlhWQFWRoWEnlzCaEuJmftPy- 0 ) khCsNTGlhWQFWRoWEnlzCaEuJmftPy=486079168; else khCsNTGlhWQFWRoWEnlzCaEuJmftPy=1285182383;if (khCsNTGlhWQFWRoWEnlzCaEuJmftPy == khCsNTGlhWQFWRoWEnlzCaEuJmftPy- 0 ) khCsNTGlhWQFWRoWEnlzCaEuJmftPy=978210916; else khCsNTGlhWQFWRoWEnlzCaEuJmftPy=881135726;if (khCsNTGlhWQFWRoWEnlzCaEuJmftPy == khCsNTGlhWQFWRoWEnlzCaEuJmftPy- 1 ) khCsNTGlhWQFWRoWEnlzCaEuJmftPy=1967208417; else khCsNTGlhWQFWRoWEnlzCaEuJmftPy=180032298;if (khCsNTGlhWQFWRoWEnlzCaEuJmftPy == khCsNTGlhWQFWRoWEnlzCaEuJmftPy- 0 ) khCsNTGlhWQFWRoWEnlzCaEuJmftPy=1911679021; else khCsNTGlhWQFWRoWEnlzCaEuJmftPy=1607053947;double BDqJWGDYMzdqxCAgpOauXkbPXEmprG=190734025.804279770972134920080587688286;if (BDqJWGDYMzdqxCAgpOauXkbPXEmprG == BDqJWGDYMzdqxCAgpOauXkbPXEmprG ) BDqJWGDYMzdqxCAgpOauXkbPXEmprG=2115181670.330591616912587786709272742870; else BDqJWGDYMzdqxCAgpOauXkbPXEmprG=1986252728.169465375441517901190259826985;if (BDqJWGDYMzdqxCAgpOauXkbPXEmprG == BDqJWGDYMzdqxCAgpOauXkbPXEmprG ) BDqJWGDYMzdqxCAgpOauXkbPXEmprG=649850786.995786312252246886694278001075; else BDqJWGDYMzdqxCAgpOauXkbPXEmprG=1733313017.959155783160994912315130272779;if (BDqJWGDYMzdqxCAgpOauXkbPXEmprG == BDqJWGDYMzdqxCAgpOauXkbPXEmprG ) BDqJWGDYMzdqxCAgpOauXkbPXEmprG=1443622039.535841329210353538529077092098; else BDqJWGDYMzdqxCAgpOauXkbPXEmprG=1283987201.412574626460320689885436166986;if (BDqJWGDYMzdqxCAgpOauXkbPXEmprG == BDqJWGDYMzdqxCAgpOauXkbPXEmprG ) BDqJWGDYMzdqxCAgpOauXkbPXEmprG=1854695346.043181999991326975751332409751; else BDqJWGDYMzdqxCAgpOauXkbPXEmprG=13948230.884143835070199412020224672188;if (BDqJWGDYMzdqxCAgpOauXkbPXEmprG == BDqJWGDYMzdqxCAgpOauXkbPXEmprG ) BDqJWGDYMzdqxCAgpOauXkbPXEmprG=727700033.234822738324188125409874276157; else BDqJWGDYMzdqxCAgpOauXkbPXEmprG=454712060.286785647879480615656888956176;if (BDqJWGDYMzdqxCAgpOauXkbPXEmprG == BDqJWGDYMzdqxCAgpOauXkbPXEmprG ) BDqJWGDYMzdqxCAgpOauXkbPXEmprG=2144189955.668125950359863140476261240984; else BDqJWGDYMzdqxCAgpOauXkbPXEmprG=1261453877.067221146687631702591781051898;int AieTxJhWLyzTsuyfOzcVtUvONpiJnB=1020023403;if (AieTxJhWLyzTsuyfOzcVtUvONpiJnB == AieTxJhWLyzTsuyfOzcVtUvONpiJnB- 1 ) AieTxJhWLyzTsuyfOzcVtUvONpiJnB=1216971270; else AieTxJhWLyzTsuyfOzcVtUvONpiJnB=1003082157;if (AieTxJhWLyzTsuyfOzcVtUvONpiJnB == AieTxJhWLyzTsuyfOzcVtUvONpiJnB- 1 ) AieTxJhWLyzTsuyfOzcVtUvONpiJnB=2053370197; else AieTxJhWLyzTsuyfOzcVtUvONpiJnB=848800193;if (AieTxJhWLyzTsuyfOzcVtUvONpiJnB == AieTxJhWLyzTsuyfOzcVtUvONpiJnB- 1 ) AieTxJhWLyzTsuyfOzcVtUvONpiJnB=1098985667; else AieTxJhWLyzTsuyfOzcVtUvONpiJnB=1435900622;if (AieTxJhWLyzTsuyfOzcVtUvONpiJnB == AieTxJhWLyzTsuyfOzcVtUvONpiJnB- 1 ) AieTxJhWLyzTsuyfOzcVtUvONpiJnB=459116415; else AieTxJhWLyzTsuyfOzcVtUvONpiJnB=518214041;if (AieTxJhWLyzTsuyfOzcVtUvONpiJnB == AieTxJhWLyzTsuyfOzcVtUvONpiJnB- 0 ) AieTxJhWLyzTsuyfOzcVtUvONpiJnB=227289762; else AieTxJhWLyzTsuyfOzcVtUvONpiJnB=778091203;if (AieTxJhWLyzTsuyfOzcVtUvONpiJnB == AieTxJhWLyzTsuyfOzcVtUvONpiJnB- 1 ) AieTxJhWLyzTsuyfOzcVtUvONpiJnB=830498942; else AieTxJhWLyzTsuyfOzcVtUvONpiJnB=210581907;int bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ=69159348;if (bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ == bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ- 0 ) bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ=494745914; else bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ=870663719;if (bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ == bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ- 0 ) bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ=757235765; else bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ=585434768;if (bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ == bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ- 0 ) bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ=1816369949; else bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ=1392865814;if (bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ == bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ- 0 ) bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ=2019253794; else bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ=384060667;if (bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ == bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ- 1 ) bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ=1083136314; else bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ=947604406;if (bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ == bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ- 1 ) bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ=1358787771; else bxnVigVzkHVdPjWuZwWVNKOEJZnFFZ=31367108;int AOkQWFznXNLnaXdARCQAzAdDkwDtds=1684507589;if (AOkQWFznXNLnaXdARCQAzAdDkwDtds == AOkQWFznXNLnaXdARCQAzAdDkwDtds- 0 ) AOkQWFznXNLnaXdARCQAzAdDkwDtds=975632764; else AOkQWFznXNLnaXdARCQAzAdDkwDtds=891655618;if (AOkQWFznXNLnaXdARCQAzAdDkwDtds == AOkQWFznXNLnaXdARCQAzAdDkwDtds- 1 ) AOkQWFznXNLnaXdARCQAzAdDkwDtds=390082677; else AOkQWFznXNLnaXdARCQAzAdDkwDtds=534651781;if (AOkQWFznXNLnaXdARCQAzAdDkwDtds == AOkQWFznXNLnaXdARCQAzAdDkwDtds- 1 ) AOkQWFznXNLnaXdARCQAzAdDkwDtds=1656150915; else AOkQWFznXNLnaXdARCQAzAdDkwDtds=1380419462;if (AOkQWFznXNLnaXdARCQAzAdDkwDtds == AOkQWFznXNLnaXdARCQAzAdDkwDtds- 1 ) AOkQWFznXNLnaXdARCQAzAdDkwDtds=1239232005; else AOkQWFznXNLnaXdARCQAzAdDkwDtds=679281048;if (AOkQWFznXNLnaXdARCQAzAdDkwDtds == AOkQWFznXNLnaXdARCQAzAdDkwDtds- 1 ) AOkQWFznXNLnaXdARCQAzAdDkwDtds=1527136313; else AOkQWFznXNLnaXdARCQAzAdDkwDtds=131636745;if (AOkQWFznXNLnaXdARCQAzAdDkwDtds == AOkQWFznXNLnaXdARCQAzAdDkwDtds- 0 ) AOkQWFznXNLnaXdARCQAzAdDkwDtds=1506023255; else AOkQWFznXNLnaXdARCQAzAdDkwDtds=665530836;float DidTovgzFKQnUczYjjnFmABZwASnAB=512546476.185796053404013453686734321121f;if (DidTovgzFKQnUczYjjnFmABZwASnAB - DidTovgzFKQnUczYjjnFmABZwASnAB> 0.00000001 ) DidTovgzFKQnUczYjjnFmABZwASnAB=997599215.102849540138194438240782145158f; else DidTovgzFKQnUczYjjnFmABZwASnAB=2075030607.693330237502356875109399143171f;if (DidTovgzFKQnUczYjjnFmABZwASnAB - DidTovgzFKQnUczYjjnFmABZwASnAB> 0.00000001 ) DidTovgzFKQnUczYjjnFmABZwASnAB=766707963.440364586421285954648115030520f; else DidTovgzFKQnUczYjjnFmABZwASnAB=407683721.826716210339978822530064443534f;if (DidTovgzFKQnUczYjjnFmABZwASnAB - DidTovgzFKQnUczYjjnFmABZwASnAB> 0.00000001 ) DidTovgzFKQnUczYjjnFmABZwASnAB=1672948710.417761658974541934721910096248f; else DidTovgzFKQnUczYjjnFmABZwASnAB=766448413.389693322838566653451981351788f;if (DidTovgzFKQnUczYjjnFmABZwASnAB - DidTovgzFKQnUczYjjnFmABZwASnAB> 0.00000001 ) DidTovgzFKQnUczYjjnFmABZwASnAB=1351137101.542069695861309455444499741844f; else DidTovgzFKQnUczYjjnFmABZwASnAB=1971589103.365430651817869179301644426332f;if (DidTovgzFKQnUczYjjnFmABZwASnAB - DidTovgzFKQnUczYjjnFmABZwASnAB> 0.00000001 ) DidTovgzFKQnUczYjjnFmABZwASnAB=1131312010.996161797080574763078364483111f; else DidTovgzFKQnUczYjjnFmABZwASnAB=1669309530.542992271958233117077337574288f;if (DidTovgzFKQnUczYjjnFmABZwASnAB - DidTovgzFKQnUczYjjnFmABZwASnAB> 0.00000001 ) DidTovgzFKQnUczYjjnFmABZwASnAB=1913147526.869654878269222013237724521643f; else DidTovgzFKQnUczYjjnFmABZwASnAB=151381261.900310068935982822556115224524f;int MNcQdvuDOEbYPyMmRKjCxCylowYGwa=510532723;if (MNcQdvuDOEbYPyMmRKjCxCylowYGwa == MNcQdvuDOEbYPyMmRKjCxCylowYGwa- 0 ) MNcQdvuDOEbYPyMmRKjCxCylowYGwa=1175302917; else MNcQdvuDOEbYPyMmRKjCxCylowYGwa=1293475000;if (MNcQdvuDOEbYPyMmRKjCxCylowYGwa == MNcQdvuDOEbYPyMmRKjCxCylowYGwa- 1 ) MNcQdvuDOEbYPyMmRKjCxCylowYGwa=1382432790; else MNcQdvuDOEbYPyMmRKjCxCylowYGwa=658765568;if (MNcQdvuDOEbYPyMmRKjCxCylowYGwa == MNcQdvuDOEbYPyMmRKjCxCylowYGwa- 1 ) MNcQdvuDOEbYPyMmRKjCxCylowYGwa=1972554527; else MNcQdvuDOEbYPyMmRKjCxCylowYGwa=1069944711;if (MNcQdvuDOEbYPyMmRKjCxCylowYGwa == MNcQdvuDOEbYPyMmRKjCxCylowYGwa- 0 ) MNcQdvuDOEbYPyMmRKjCxCylowYGwa=560492080; else MNcQdvuDOEbYPyMmRKjCxCylowYGwa=1261810099;if (MNcQdvuDOEbYPyMmRKjCxCylowYGwa == MNcQdvuDOEbYPyMmRKjCxCylowYGwa- 1 ) MNcQdvuDOEbYPyMmRKjCxCylowYGwa=1778915361; else MNcQdvuDOEbYPyMmRKjCxCylowYGwa=77954717;if (MNcQdvuDOEbYPyMmRKjCxCylowYGwa == MNcQdvuDOEbYPyMmRKjCxCylowYGwa- 0 ) MNcQdvuDOEbYPyMmRKjCxCylowYGwa=1205782102; else MNcQdvuDOEbYPyMmRKjCxCylowYGwa=1684733204;long VCpNulLCIJlLwebxjrcpeATzKJWhGX=1586506522;if (VCpNulLCIJlLwebxjrcpeATzKJWhGX == VCpNulLCIJlLwebxjrcpeATzKJWhGX- 1 ) VCpNulLCIJlLwebxjrcpeATzKJWhGX=336593715; else VCpNulLCIJlLwebxjrcpeATzKJWhGX=846241295;if (VCpNulLCIJlLwebxjrcpeATzKJWhGX == VCpNulLCIJlLwebxjrcpeATzKJWhGX- 1 ) VCpNulLCIJlLwebxjrcpeATzKJWhGX=1181251540; else VCpNulLCIJlLwebxjrcpeATzKJWhGX=1360929115;if (VCpNulLCIJlLwebxjrcpeATzKJWhGX == VCpNulLCIJlLwebxjrcpeATzKJWhGX- 1 ) VCpNulLCIJlLwebxjrcpeATzKJWhGX=1097532723; else VCpNulLCIJlLwebxjrcpeATzKJWhGX=1342588212;if (VCpNulLCIJlLwebxjrcpeATzKJWhGX == VCpNulLCIJlLwebxjrcpeATzKJWhGX- 1 ) VCpNulLCIJlLwebxjrcpeATzKJWhGX=11558879; else VCpNulLCIJlLwebxjrcpeATzKJWhGX=209184351;if (VCpNulLCIJlLwebxjrcpeATzKJWhGX == VCpNulLCIJlLwebxjrcpeATzKJWhGX- 1 ) VCpNulLCIJlLwebxjrcpeATzKJWhGX=1688002164; else VCpNulLCIJlLwebxjrcpeATzKJWhGX=565977221;if (VCpNulLCIJlLwebxjrcpeATzKJWhGX == VCpNulLCIJlLwebxjrcpeATzKJWhGX- 0 ) VCpNulLCIJlLwebxjrcpeATzKJWhGX=1550823418; else VCpNulLCIJlLwebxjrcpeATzKJWhGX=2134325032;long IKkEZuMwSCyzsQIyQNEOhnuBrPBlug=1099387006;if (IKkEZuMwSCyzsQIyQNEOhnuBrPBlug == IKkEZuMwSCyzsQIyQNEOhnuBrPBlug- 1 ) IKkEZuMwSCyzsQIyQNEOhnuBrPBlug=901898779; else IKkEZuMwSCyzsQIyQNEOhnuBrPBlug=995490532;if (IKkEZuMwSCyzsQIyQNEOhnuBrPBlug == IKkEZuMwSCyzsQIyQNEOhnuBrPBlug- 0 ) IKkEZuMwSCyzsQIyQNEOhnuBrPBlug=1637711566; else IKkEZuMwSCyzsQIyQNEOhnuBrPBlug=1307496906;if (IKkEZuMwSCyzsQIyQNEOhnuBrPBlug == IKkEZuMwSCyzsQIyQNEOhnuBrPBlug- 0 ) IKkEZuMwSCyzsQIyQNEOhnuBrPBlug=261680649; else IKkEZuMwSCyzsQIyQNEOhnuBrPBlug=2099728770;if (IKkEZuMwSCyzsQIyQNEOhnuBrPBlug == IKkEZuMwSCyzsQIyQNEOhnuBrPBlug- 0 ) IKkEZuMwSCyzsQIyQNEOhnuBrPBlug=687475596; else IKkEZuMwSCyzsQIyQNEOhnuBrPBlug=2075138151;if (IKkEZuMwSCyzsQIyQNEOhnuBrPBlug == IKkEZuMwSCyzsQIyQNEOhnuBrPBlug- 0 ) IKkEZuMwSCyzsQIyQNEOhnuBrPBlug=1514538527; else IKkEZuMwSCyzsQIyQNEOhnuBrPBlug=1652381511;if (IKkEZuMwSCyzsQIyQNEOhnuBrPBlug == IKkEZuMwSCyzsQIyQNEOhnuBrPBlug- 1 ) IKkEZuMwSCyzsQIyQNEOhnuBrPBlug=776252007; else IKkEZuMwSCyzsQIyQNEOhnuBrPBlug=1594993993;int rprcJsqCLVKEBXaVUrzFVxzBTrnSuE=1925072699;if (rprcJsqCLVKEBXaVUrzFVxzBTrnSuE == rprcJsqCLVKEBXaVUrzFVxzBTrnSuE- 1 ) rprcJsqCLVKEBXaVUrzFVxzBTrnSuE=1738737122; else rprcJsqCLVKEBXaVUrzFVxzBTrnSuE=326641004;if (rprcJsqCLVKEBXaVUrzFVxzBTrnSuE == rprcJsqCLVKEBXaVUrzFVxzBTrnSuE- 0 ) rprcJsqCLVKEBXaVUrzFVxzBTrnSuE=1962271193; else rprcJsqCLVKEBXaVUrzFVxzBTrnSuE=1378355203;if (rprcJsqCLVKEBXaVUrzFVxzBTrnSuE == rprcJsqCLVKEBXaVUrzFVxzBTrnSuE- 0 ) rprcJsqCLVKEBXaVUrzFVxzBTrnSuE=1393981414; else rprcJsqCLVKEBXaVUrzFVxzBTrnSuE=16292498;if (rprcJsqCLVKEBXaVUrzFVxzBTrnSuE == rprcJsqCLVKEBXaVUrzFVxzBTrnSuE- 0 ) rprcJsqCLVKEBXaVUrzFVxzBTrnSuE=1560483647; else rprcJsqCLVKEBXaVUrzFVxzBTrnSuE=1980198352;if (rprcJsqCLVKEBXaVUrzFVxzBTrnSuE == rprcJsqCLVKEBXaVUrzFVxzBTrnSuE- 1 ) rprcJsqCLVKEBXaVUrzFVxzBTrnSuE=1281916121; else rprcJsqCLVKEBXaVUrzFVxzBTrnSuE=1173248439;if (rprcJsqCLVKEBXaVUrzFVxzBTrnSuE == rprcJsqCLVKEBXaVUrzFVxzBTrnSuE- 1 ) rprcJsqCLVKEBXaVUrzFVxzBTrnSuE=1118247051; else rprcJsqCLVKEBXaVUrzFVxzBTrnSuE=1406577306;float JlRFIGRoDqPNwppTrzigrgyBPUgaNM=691064617.861304680415274305116981984577f;if (JlRFIGRoDqPNwppTrzigrgyBPUgaNM - JlRFIGRoDqPNwppTrzigrgyBPUgaNM> 0.00000001 ) JlRFIGRoDqPNwppTrzigrgyBPUgaNM=1623270167.603176528067014302286434209153f; else JlRFIGRoDqPNwppTrzigrgyBPUgaNM=106395096.122481320998826098768239228998f;if (JlRFIGRoDqPNwppTrzigrgyBPUgaNM - JlRFIGRoDqPNwppTrzigrgyBPUgaNM> 0.00000001 ) JlRFIGRoDqPNwppTrzigrgyBPUgaNM=782834425.858594573318349871921249599327f; else JlRFIGRoDqPNwppTrzigrgyBPUgaNM=1038835674.443225133263546000155708861010f;if (JlRFIGRoDqPNwppTrzigrgyBPUgaNM - JlRFIGRoDqPNwppTrzigrgyBPUgaNM> 0.00000001 ) JlRFIGRoDqPNwppTrzigrgyBPUgaNM=1863495039.626707612574987971350038275656f; else JlRFIGRoDqPNwppTrzigrgyBPUgaNM=770559149.523374884059868292100076169854f;if (JlRFIGRoDqPNwppTrzigrgyBPUgaNM - JlRFIGRoDqPNwppTrzigrgyBPUgaNM> 0.00000001 ) JlRFIGRoDqPNwppTrzigrgyBPUgaNM=1324825873.207716262419741128119355755831f; else JlRFIGRoDqPNwppTrzigrgyBPUgaNM=384348592.503185326203189144505285595206f;if (JlRFIGRoDqPNwppTrzigrgyBPUgaNM - JlRFIGRoDqPNwppTrzigrgyBPUgaNM> 0.00000001 ) JlRFIGRoDqPNwppTrzigrgyBPUgaNM=158765699.254192666681291638772133732969f; else JlRFIGRoDqPNwppTrzigrgyBPUgaNM=1253597036.022016909758993510497854512156f;if (JlRFIGRoDqPNwppTrzigrgyBPUgaNM - JlRFIGRoDqPNwppTrzigrgyBPUgaNM> 0.00000001 ) JlRFIGRoDqPNwppTrzigrgyBPUgaNM=1993874235.080777939673902434133358337950f; else JlRFIGRoDqPNwppTrzigrgyBPUgaNM=1592634859.485487823679293496229813374355f;long xlrGupeMZdCHXfZIeZrVGkTGgmKBBT=2021598267;if (xlrGupeMZdCHXfZIeZrVGkTGgmKBBT == xlrGupeMZdCHXfZIeZrVGkTGgmKBBT- 1 ) xlrGupeMZdCHXfZIeZrVGkTGgmKBBT=628714324; else xlrGupeMZdCHXfZIeZrVGkTGgmKBBT=1773234366;if (xlrGupeMZdCHXfZIeZrVGkTGgmKBBT == xlrGupeMZdCHXfZIeZrVGkTGgmKBBT- 0 ) xlrGupeMZdCHXfZIeZrVGkTGgmKBBT=395587910; else xlrGupeMZdCHXfZIeZrVGkTGgmKBBT=1288114579;if (xlrGupeMZdCHXfZIeZrVGkTGgmKBBT == xlrGupeMZdCHXfZIeZrVGkTGgmKBBT- 0 ) xlrGupeMZdCHXfZIeZrVGkTGgmKBBT=1273063198; else xlrGupeMZdCHXfZIeZrVGkTGgmKBBT=1371296641;if (xlrGupeMZdCHXfZIeZrVGkTGgmKBBT == xlrGupeMZdCHXfZIeZrVGkTGgmKBBT- 1 ) xlrGupeMZdCHXfZIeZrVGkTGgmKBBT=1852889239; else xlrGupeMZdCHXfZIeZrVGkTGgmKBBT=885606281;if (xlrGupeMZdCHXfZIeZrVGkTGgmKBBT == xlrGupeMZdCHXfZIeZrVGkTGgmKBBT- 1 ) xlrGupeMZdCHXfZIeZrVGkTGgmKBBT=1582490532; else xlrGupeMZdCHXfZIeZrVGkTGgmKBBT=1804996861;if (xlrGupeMZdCHXfZIeZrVGkTGgmKBBT == xlrGupeMZdCHXfZIeZrVGkTGgmKBBT- 0 ) xlrGupeMZdCHXfZIeZrVGkTGgmKBBT=25580785; else xlrGupeMZdCHXfZIeZrVGkTGgmKBBT=1235915857;double ElzcnwcXtrTFmSKappQwVuuvoYTNKn=940887832.887970059507072813530301525565;if (ElzcnwcXtrTFmSKappQwVuuvoYTNKn == ElzcnwcXtrTFmSKappQwVuuvoYTNKn ) ElzcnwcXtrTFmSKappQwVuuvoYTNKn=2005326995.489224205260297465907840169645; else ElzcnwcXtrTFmSKappQwVuuvoYTNKn=85187390.669412996871938021516067975017;if (ElzcnwcXtrTFmSKappQwVuuvoYTNKn == ElzcnwcXtrTFmSKappQwVuuvoYTNKn ) ElzcnwcXtrTFmSKappQwVuuvoYTNKn=2119207473.667060189568024791225823971364; else ElzcnwcXtrTFmSKappQwVuuvoYTNKn=1601754862.708931597904306905328580385420;if (ElzcnwcXtrTFmSKappQwVuuvoYTNKn == ElzcnwcXtrTFmSKappQwVuuvoYTNKn ) ElzcnwcXtrTFmSKappQwVuuvoYTNKn=1031053119.082597119321442162817604632976; else ElzcnwcXtrTFmSKappQwVuuvoYTNKn=1045910870.718892392999611815832471449896;if (ElzcnwcXtrTFmSKappQwVuuvoYTNKn == ElzcnwcXtrTFmSKappQwVuuvoYTNKn ) ElzcnwcXtrTFmSKappQwVuuvoYTNKn=61577660.635471375188093810483715435746; else ElzcnwcXtrTFmSKappQwVuuvoYTNKn=968177973.450647535064175559964052492104;if (ElzcnwcXtrTFmSKappQwVuuvoYTNKn == ElzcnwcXtrTFmSKappQwVuuvoYTNKn ) ElzcnwcXtrTFmSKappQwVuuvoYTNKn=1149298778.763084945748643974109799350936; else ElzcnwcXtrTFmSKappQwVuuvoYTNKn=1577922601.554140220171432642962801608574;if (ElzcnwcXtrTFmSKappQwVuuvoYTNKn == ElzcnwcXtrTFmSKappQwVuuvoYTNKn ) ElzcnwcXtrTFmSKappQwVuuvoYTNKn=1219589504.412020550386504947771907475465; else ElzcnwcXtrTFmSKappQwVuuvoYTNKn=140384378.375677041320269740777291451575;int GxcCCHuQjvxdUaEnSplSXgroFsTJKd=717867588;if (GxcCCHuQjvxdUaEnSplSXgroFsTJKd == GxcCCHuQjvxdUaEnSplSXgroFsTJKd- 1 ) GxcCCHuQjvxdUaEnSplSXgroFsTJKd=524730399; else GxcCCHuQjvxdUaEnSplSXgroFsTJKd=1244132431;if (GxcCCHuQjvxdUaEnSplSXgroFsTJKd == GxcCCHuQjvxdUaEnSplSXgroFsTJKd- 0 ) GxcCCHuQjvxdUaEnSplSXgroFsTJKd=777862982; else GxcCCHuQjvxdUaEnSplSXgroFsTJKd=2040854221;if (GxcCCHuQjvxdUaEnSplSXgroFsTJKd == GxcCCHuQjvxdUaEnSplSXgroFsTJKd- 1 ) GxcCCHuQjvxdUaEnSplSXgroFsTJKd=178062785; else GxcCCHuQjvxdUaEnSplSXgroFsTJKd=506423584;if (GxcCCHuQjvxdUaEnSplSXgroFsTJKd == GxcCCHuQjvxdUaEnSplSXgroFsTJKd- 0 ) GxcCCHuQjvxdUaEnSplSXgroFsTJKd=1565485066; else GxcCCHuQjvxdUaEnSplSXgroFsTJKd=251973526;if (GxcCCHuQjvxdUaEnSplSXgroFsTJKd == GxcCCHuQjvxdUaEnSplSXgroFsTJKd- 0 ) GxcCCHuQjvxdUaEnSplSXgroFsTJKd=400989504; else GxcCCHuQjvxdUaEnSplSXgroFsTJKd=2044711791;if (GxcCCHuQjvxdUaEnSplSXgroFsTJKd == GxcCCHuQjvxdUaEnSplSXgroFsTJKd- 0 ) GxcCCHuQjvxdUaEnSplSXgroFsTJKd=1603828789; else GxcCCHuQjvxdUaEnSplSXgroFsTJKd=58498178;float ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw=877867409.816186011427800326851445471180f;if (ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw - ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw> 0.00000001 ) ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw=845063825.411905001725173113164005648181f; else ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw=1052986403.594101601532514647337812752747f;if (ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw - ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw> 0.00000001 ) ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw=1734791511.803124107262364269100195589356f; else ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw=481148815.907574950165018565950464772974f;if (ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw - ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw> 0.00000001 ) ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw=592324707.595236690041915528703440765380f; else ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw=1194246825.318667199307692956026528215558f;if (ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw - ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw> 0.00000001 ) ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw=990545148.132083435854050696991936580155f; else ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw=1730287101.264127671962228358863076731650f;if (ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw - ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw> 0.00000001 ) ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw=1711656706.439070093313604981934239339950f; else ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw=1574893584.712163426837566047385182720174f;if (ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw - ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw> 0.00000001 ) ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw=138815801.125471155167510778349367663044f; else ZBqqfqEQuxnAebEhsMkgEGiVDRQIBw=1842624244.336282329690322967316383696881f;int EFHpPEPPrUbslYvzTYlCdoPpCbMgtd=1614641744;if (EFHpPEPPrUbslYvzTYlCdoPpCbMgtd == EFHpPEPPrUbslYvzTYlCdoPpCbMgtd- 0 ) EFHpPEPPrUbslYvzTYlCdoPpCbMgtd=391134714; else EFHpPEPPrUbslYvzTYlCdoPpCbMgtd=1455196844;if (EFHpPEPPrUbslYvzTYlCdoPpCbMgtd == EFHpPEPPrUbslYvzTYlCdoPpCbMgtd- 0 ) EFHpPEPPrUbslYvzTYlCdoPpCbMgtd=711185458; else EFHpPEPPrUbslYvzTYlCdoPpCbMgtd=1884832935;if (EFHpPEPPrUbslYvzTYlCdoPpCbMgtd == EFHpPEPPrUbslYvzTYlCdoPpCbMgtd- 0 ) EFHpPEPPrUbslYvzTYlCdoPpCbMgtd=650518955; else EFHpPEPPrUbslYvzTYlCdoPpCbMgtd=515162637;if (EFHpPEPPrUbslYvzTYlCdoPpCbMgtd == EFHpPEPPrUbslYvzTYlCdoPpCbMgtd- 0 ) EFHpPEPPrUbslYvzTYlCdoPpCbMgtd=762825047; else EFHpPEPPrUbslYvzTYlCdoPpCbMgtd=1498903411;if (EFHpPEPPrUbslYvzTYlCdoPpCbMgtd == EFHpPEPPrUbslYvzTYlCdoPpCbMgtd- 1 ) EFHpPEPPrUbslYvzTYlCdoPpCbMgtd=667185971; else EFHpPEPPrUbslYvzTYlCdoPpCbMgtd=1867233947;if (EFHpPEPPrUbslYvzTYlCdoPpCbMgtd == EFHpPEPPrUbslYvzTYlCdoPpCbMgtd- 1 ) EFHpPEPPrUbslYvzTYlCdoPpCbMgtd=1200765358; else EFHpPEPPrUbslYvzTYlCdoPpCbMgtd=1133824975;float cjkzLmHWscpPXJlxiKmCWadrSxICUz=1060388762.355702363635401411801796124206f;if (cjkzLmHWscpPXJlxiKmCWadrSxICUz - cjkzLmHWscpPXJlxiKmCWadrSxICUz> 0.00000001 ) cjkzLmHWscpPXJlxiKmCWadrSxICUz=27572081.040252747045312196953984367968f; else cjkzLmHWscpPXJlxiKmCWadrSxICUz=341722095.816869635987596658404936327699f;if (cjkzLmHWscpPXJlxiKmCWadrSxICUz - cjkzLmHWscpPXJlxiKmCWadrSxICUz> 0.00000001 ) cjkzLmHWscpPXJlxiKmCWadrSxICUz=1442804200.688924066457108649882967509154f; else cjkzLmHWscpPXJlxiKmCWadrSxICUz=1264399154.214640977360316783878882873243f;if (cjkzLmHWscpPXJlxiKmCWadrSxICUz - cjkzLmHWscpPXJlxiKmCWadrSxICUz> 0.00000001 ) cjkzLmHWscpPXJlxiKmCWadrSxICUz=380540924.601225357888362275182639753397f; else cjkzLmHWscpPXJlxiKmCWadrSxICUz=932713332.873587919153710063061826207268f;if (cjkzLmHWscpPXJlxiKmCWadrSxICUz - cjkzLmHWscpPXJlxiKmCWadrSxICUz> 0.00000001 ) cjkzLmHWscpPXJlxiKmCWadrSxICUz=125542881.307111588546714903079134716848f; else cjkzLmHWscpPXJlxiKmCWadrSxICUz=1477967221.086195761958563511331539028450f;if (cjkzLmHWscpPXJlxiKmCWadrSxICUz - cjkzLmHWscpPXJlxiKmCWadrSxICUz> 0.00000001 ) cjkzLmHWscpPXJlxiKmCWadrSxICUz=253587283.585056163924212168120651755827f; else cjkzLmHWscpPXJlxiKmCWadrSxICUz=1195059528.842158461755941843797133103431f;if (cjkzLmHWscpPXJlxiKmCWadrSxICUz - cjkzLmHWscpPXJlxiKmCWadrSxICUz> 0.00000001 ) cjkzLmHWscpPXJlxiKmCWadrSxICUz=329197515.235629227580858672002314081015f; else cjkzLmHWscpPXJlxiKmCWadrSxICUz=750652848.503152133362631739663244903265f;double MHJzonjweYWtlRChmMRVMVbKOfuKct=835968290.029225326821848406038313200606;if (MHJzonjweYWtlRChmMRVMVbKOfuKct == MHJzonjweYWtlRChmMRVMVbKOfuKct ) MHJzonjweYWtlRChmMRVMVbKOfuKct=1329007420.568810764912539381887131071712; else MHJzonjweYWtlRChmMRVMVbKOfuKct=2136670692.683146413985213699592992044794;if (MHJzonjweYWtlRChmMRVMVbKOfuKct == MHJzonjweYWtlRChmMRVMVbKOfuKct ) MHJzonjweYWtlRChmMRVMVbKOfuKct=1814847111.905344040308991302849332070221; else MHJzonjweYWtlRChmMRVMVbKOfuKct=1437265167.373908008075020510174295678337;if (MHJzonjweYWtlRChmMRVMVbKOfuKct == MHJzonjweYWtlRChmMRVMVbKOfuKct ) MHJzonjweYWtlRChmMRVMVbKOfuKct=1525208205.593914609431498541367015718844; else MHJzonjweYWtlRChmMRVMVbKOfuKct=1813001232.077268212602175481724718158920;if (MHJzonjweYWtlRChmMRVMVbKOfuKct == MHJzonjweYWtlRChmMRVMVbKOfuKct ) MHJzonjweYWtlRChmMRVMVbKOfuKct=227348454.686646433033076604143823047011; else MHJzonjweYWtlRChmMRVMVbKOfuKct=329901188.149213654916879174266499897461;if (MHJzonjweYWtlRChmMRVMVbKOfuKct == MHJzonjweYWtlRChmMRVMVbKOfuKct ) MHJzonjweYWtlRChmMRVMVbKOfuKct=290161356.052200497405133805507486675153; else MHJzonjweYWtlRChmMRVMVbKOfuKct=235888100.629497478979886247600207477773;if (MHJzonjweYWtlRChmMRVMVbKOfuKct == MHJzonjweYWtlRChmMRVMVbKOfuKct ) MHJzonjweYWtlRChmMRVMVbKOfuKct=1961446491.544588684754763960154956353313; else MHJzonjweYWtlRChmMRVMVbKOfuKct=872574407.080544541867305754451762079924;long PkGknQLKyUOxRtigcBivxwcIPJqanX=1708577224;if (PkGknQLKyUOxRtigcBivxwcIPJqanX == PkGknQLKyUOxRtigcBivxwcIPJqanX- 0 ) PkGknQLKyUOxRtigcBivxwcIPJqanX=1351693975; else PkGknQLKyUOxRtigcBivxwcIPJqanX=1225910686;if (PkGknQLKyUOxRtigcBivxwcIPJqanX == PkGknQLKyUOxRtigcBivxwcIPJqanX- 1 ) PkGknQLKyUOxRtigcBivxwcIPJqanX=1815052163; else PkGknQLKyUOxRtigcBivxwcIPJqanX=1957468689;if (PkGknQLKyUOxRtigcBivxwcIPJqanX == PkGknQLKyUOxRtigcBivxwcIPJqanX- 1 ) PkGknQLKyUOxRtigcBivxwcIPJqanX=269700661; else PkGknQLKyUOxRtigcBivxwcIPJqanX=1598788864;if (PkGknQLKyUOxRtigcBivxwcIPJqanX == PkGknQLKyUOxRtigcBivxwcIPJqanX- 0 ) PkGknQLKyUOxRtigcBivxwcIPJqanX=1234100820; else PkGknQLKyUOxRtigcBivxwcIPJqanX=1605905029;if (PkGknQLKyUOxRtigcBivxwcIPJqanX == PkGknQLKyUOxRtigcBivxwcIPJqanX- 1 ) PkGknQLKyUOxRtigcBivxwcIPJqanX=1329653963; else PkGknQLKyUOxRtigcBivxwcIPJqanX=1380701884;if (PkGknQLKyUOxRtigcBivxwcIPJqanX == PkGknQLKyUOxRtigcBivxwcIPJqanX- 0 ) PkGknQLKyUOxRtigcBivxwcIPJqanX=1631245652; else PkGknQLKyUOxRtigcBivxwcIPJqanX=1602943074;double hhQsiKfLZXaMTvZvmptXpbapZQySbs=944325149.605277210508125996476968672519;if (hhQsiKfLZXaMTvZvmptXpbapZQySbs == hhQsiKfLZXaMTvZvmptXpbapZQySbs ) hhQsiKfLZXaMTvZvmptXpbapZQySbs=1766967558.032985955171469789050688475771; else hhQsiKfLZXaMTvZvmptXpbapZQySbs=364797068.681157624015230650099356945551;if (hhQsiKfLZXaMTvZvmptXpbapZQySbs == hhQsiKfLZXaMTvZvmptXpbapZQySbs ) hhQsiKfLZXaMTvZvmptXpbapZQySbs=1952301202.128469032216505655893589233531; else hhQsiKfLZXaMTvZvmptXpbapZQySbs=1600378683.227409788162988486240177965023;if (hhQsiKfLZXaMTvZvmptXpbapZQySbs == hhQsiKfLZXaMTvZvmptXpbapZQySbs ) hhQsiKfLZXaMTvZvmptXpbapZQySbs=335716018.751109921697783170196740226154; else hhQsiKfLZXaMTvZvmptXpbapZQySbs=933418089.486652298957665459580594802299;if (hhQsiKfLZXaMTvZvmptXpbapZQySbs == hhQsiKfLZXaMTvZvmptXpbapZQySbs ) hhQsiKfLZXaMTvZvmptXpbapZQySbs=1682664204.497731114299325928597778509395; else hhQsiKfLZXaMTvZvmptXpbapZQySbs=362054496.010066094434639238998471273034;if (hhQsiKfLZXaMTvZvmptXpbapZQySbs == hhQsiKfLZXaMTvZvmptXpbapZQySbs ) hhQsiKfLZXaMTvZvmptXpbapZQySbs=114373641.382929936904526414937251774980; else hhQsiKfLZXaMTvZvmptXpbapZQySbs=1112903012.406125281130209596696920724226;if (hhQsiKfLZXaMTvZvmptXpbapZQySbs == hhQsiKfLZXaMTvZvmptXpbapZQySbs ) hhQsiKfLZXaMTvZvmptXpbapZQySbs=974237798.506750850124491705238949469741; else hhQsiKfLZXaMTvZvmptXpbapZQySbs=144652824.217563556085658354012900977749;double FFzwBkrNnIXwNXORwHCEbjNrNGkhwi=396867110.308109842927867961298073996265;if (FFzwBkrNnIXwNXORwHCEbjNrNGkhwi == FFzwBkrNnIXwNXORwHCEbjNrNGkhwi ) FFzwBkrNnIXwNXORwHCEbjNrNGkhwi=1054338155.218334042162031582833617634622; else FFzwBkrNnIXwNXORwHCEbjNrNGkhwi=768103938.368782054302914615251553260181;if (FFzwBkrNnIXwNXORwHCEbjNrNGkhwi == FFzwBkrNnIXwNXORwHCEbjNrNGkhwi ) FFzwBkrNnIXwNXORwHCEbjNrNGkhwi=1546442520.142180436064864181313948192652; else FFzwBkrNnIXwNXORwHCEbjNrNGkhwi=1289180744.298426193538930870919088821500;if (FFzwBkrNnIXwNXORwHCEbjNrNGkhwi == FFzwBkrNnIXwNXORwHCEbjNrNGkhwi ) FFzwBkrNnIXwNXORwHCEbjNrNGkhwi=55292306.899487654919898512932374421160; else FFzwBkrNnIXwNXORwHCEbjNrNGkhwi=290281347.679374065323611030042968963516;if (FFzwBkrNnIXwNXORwHCEbjNrNGkhwi == FFzwBkrNnIXwNXORwHCEbjNrNGkhwi ) FFzwBkrNnIXwNXORwHCEbjNrNGkhwi=566267629.750535153565761086440222871252; else FFzwBkrNnIXwNXORwHCEbjNrNGkhwi=1877702203.265068329608769730709718664877;if (FFzwBkrNnIXwNXORwHCEbjNrNGkhwi == FFzwBkrNnIXwNXORwHCEbjNrNGkhwi ) FFzwBkrNnIXwNXORwHCEbjNrNGkhwi=902569872.491231181476884357414166455841; else FFzwBkrNnIXwNXORwHCEbjNrNGkhwi=208942082.466221863876033304442846344588;if (FFzwBkrNnIXwNXORwHCEbjNrNGkhwi == FFzwBkrNnIXwNXORwHCEbjNrNGkhwi ) FFzwBkrNnIXwNXORwHCEbjNrNGkhwi=107611204.645012166469475914326140972009; else FFzwBkrNnIXwNXORwHCEbjNrNGkhwi=144506321.838524202225818348153137350525;long FfkeQCuVFlEGIyHQRKeMSubeiAFMkU=277989157;if (FfkeQCuVFlEGIyHQRKeMSubeiAFMkU == FfkeQCuVFlEGIyHQRKeMSubeiAFMkU- 1 ) FfkeQCuVFlEGIyHQRKeMSubeiAFMkU=1252076119; else FfkeQCuVFlEGIyHQRKeMSubeiAFMkU=1017121342;if (FfkeQCuVFlEGIyHQRKeMSubeiAFMkU == FfkeQCuVFlEGIyHQRKeMSubeiAFMkU- 1 ) FfkeQCuVFlEGIyHQRKeMSubeiAFMkU=1253090496; else FfkeQCuVFlEGIyHQRKeMSubeiAFMkU=1776168046;if (FfkeQCuVFlEGIyHQRKeMSubeiAFMkU == FfkeQCuVFlEGIyHQRKeMSubeiAFMkU- 0 ) FfkeQCuVFlEGIyHQRKeMSubeiAFMkU=51902618; else FfkeQCuVFlEGIyHQRKeMSubeiAFMkU=687826337;if (FfkeQCuVFlEGIyHQRKeMSubeiAFMkU == FfkeQCuVFlEGIyHQRKeMSubeiAFMkU- 0 ) FfkeQCuVFlEGIyHQRKeMSubeiAFMkU=1372861206; else FfkeQCuVFlEGIyHQRKeMSubeiAFMkU=418275886;if (FfkeQCuVFlEGIyHQRKeMSubeiAFMkU == FfkeQCuVFlEGIyHQRKeMSubeiAFMkU- 1 ) FfkeQCuVFlEGIyHQRKeMSubeiAFMkU=1690730129; else FfkeQCuVFlEGIyHQRKeMSubeiAFMkU=1452997667;if (FfkeQCuVFlEGIyHQRKeMSubeiAFMkU == FfkeQCuVFlEGIyHQRKeMSubeiAFMkU- 0 ) FfkeQCuVFlEGIyHQRKeMSubeiAFMkU=1843095048; else FfkeQCuVFlEGIyHQRKeMSubeiAFMkU=1304281296;long YBSQrDuaeXyYFFmNFulZQBuYszzmCa=2076685489;if (YBSQrDuaeXyYFFmNFulZQBuYszzmCa == YBSQrDuaeXyYFFmNFulZQBuYszzmCa- 0 ) YBSQrDuaeXyYFFmNFulZQBuYszzmCa=2043699777; else YBSQrDuaeXyYFFmNFulZQBuYszzmCa=1234378542;if (YBSQrDuaeXyYFFmNFulZQBuYszzmCa == YBSQrDuaeXyYFFmNFulZQBuYszzmCa- 0 ) YBSQrDuaeXyYFFmNFulZQBuYszzmCa=548604919; else YBSQrDuaeXyYFFmNFulZQBuYszzmCa=1565344831;if (YBSQrDuaeXyYFFmNFulZQBuYszzmCa == YBSQrDuaeXyYFFmNFulZQBuYszzmCa- 1 ) YBSQrDuaeXyYFFmNFulZQBuYszzmCa=613213326; else YBSQrDuaeXyYFFmNFulZQBuYszzmCa=1989598531;if (YBSQrDuaeXyYFFmNFulZQBuYszzmCa == YBSQrDuaeXyYFFmNFulZQBuYszzmCa- 1 ) YBSQrDuaeXyYFFmNFulZQBuYszzmCa=889032843; else YBSQrDuaeXyYFFmNFulZQBuYszzmCa=1476686211;if (YBSQrDuaeXyYFFmNFulZQBuYszzmCa == YBSQrDuaeXyYFFmNFulZQBuYszzmCa- 1 ) YBSQrDuaeXyYFFmNFulZQBuYszzmCa=1946013086; else YBSQrDuaeXyYFFmNFulZQBuYszzmCa=74598999;if (YBSQrDuaeXyYFFmNFulZQBuYszzmCa == YBSQrDuaeXyYFFmNFulZQBuYszzmCa- 1 ) YBSQrDuaeXyYFFmNFulZQBuYszzmCa=697808811; else YBSQrDuaeXyYFFmNFulZQBuYszzmCa=126773936; }
 YBSQrDuaeXyYFFmNFulZQBuYszzmCay::YBSQrDuaeXyYFFmNFulZQBuYszzmCay()
 { this->QwEdkTTaxlEv("CYifxjZWhwqXucGrEGbxIyweXGtYrEQwEdkTTaxlEvj", true, 825806885, 1538157883, 180298916); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class fwyTiqLzoXcPfXaUXCPIVSmbMuHFUvy
 { 
public: bool ZgRDvCeWlJtDMmZGFSPZbujJNKXGoi; double ZgRDvCeWlJtDMmZGFSPZbujJNKXGoifwyTiqLzoXcPfXaUXCPIVSmbMuHFUv; fwyTiqLzoXcPfXaUXCPIVSmbMuHFUvy(); void PRDNbQstrGJH(string ZgRDvCeWlJtDMmZGFSPZbujJNKXGoiPRDNbQstrGJH, bool GfsJCiNUfRtgezcOapnnCmEWnewIqk, int fYhDCNKCjqtlmxguQjgykiSAfTaWWX, float mFeumUyawPEYFwuRzHpsNMbrAxFhXj, long nnfLEoHrLLvqnRuMcWFOTThdJZemup);
 protected: bool ZgRDvCeWlJtDMmZGFSPZbujJNKXGoio; double ZgRDvCeWlJtDMmZGFSPZbujJNKXGoifwyTiqLzoXcPfXaUXCPIVSmbMuHFUvf; void PRDNbQstrGJHu(string ZgRDvCeWlJtDMmZGFSPZbujJNKXGoiPRDNbQstrGJHg, bool GfsJCiNUfRtgezcOapnnCmEWnewIqke, int fYhDCNKCjqtlmxguQjgykiSAfTaWWXr, float mFeumUyawPEYFwuRzHpsNMbrAxFhXjw, long nnfLEoHrLLvqnRuMcWFOTThdJZemupn);
 private: bool ZgRDvCeWlJtDMmZGFSPZbujJNKXGoiGfsJCiNUfRtgezcOapnnCmEWnewIqk; double ZgRDvCeWlJtDMmZGFSPZbujJNKXGoimFeumUyawPEYFwuRzHpsNMbrAxFhXjfwyTiqLzoXcPfXaUXCPIVSmbMuHFUv;
 void PRDNbQstrGJHv(string GfsJCiNUfRtgezcOapnnCmEWnewIqkPRDNbQstrGJH, bool GfsJCiNUfRtgezcOapnnCmEWnewIqkfYhDCNKCjqtlmxguQjgykiSAfTaWWX, int fYhDCNKCjqtlmxguQjgykiSAfTaWWXZgRDvCeWlJtDMmZGFSPZbujJNKXGoi, float mFeumUyawPEYFwuRzHpsNMbrAxFhXjnnfLEoHrLLvqnRuMcWFOTThdJZemup, long nnfLEoHrLLvqnRuMcWFOTThdJZemupGfsJCiNUfRtgezcOapnnCmEWnewIqk); };
 void fwyTiqLzoXcPfXaUXCPIVSmbMuHFUvy::PRDNbQstrGJH(string ZgRDvCeWlJtDMmZGFSPZbujJNKXGoiPRDNbQstrGJH, bool GfsJCiNUfRtgezcOapnnCmEWnewIqk, int fYhDCNKCjqtlmxguQjgykiSAfTaWWX, float mFeumUyawPEYFwuRzHpsNMbrAxFhXj, long nnfLEoHrLLvqnRuMcWFOTThdJZemup)
 { double QdjimZdSkQFmQXIjQJBVFkFDqQSFvG=365615434.601857436904204087874334235128;if (QdjimZdSkQFmQXIjQJBVFkFDqQSFvG == QdjimZdSkQFmQXIjQJBVFkFDqQSFvG ) QdjimZdSkQFmQXIjQJBVFkFDqQSFvG=91395357.860572710259658938381584147763; else QdjimZdSkQFmQXIjQJBVFkFDqQSFvG=2070615496.341814584238762679641794849743;if (QdjimZdSkQFmQXIjQJBVFkFDqQSFvG == QdjimZdSkQFmQXIjQJBVFkFDqQSFvG ) QdjimZdSkQFmQXIjQJBVFkFDqQSFvG=1449053164.506892895556515382268922276707; else QdjimZdSkQFmQXIjQJBVFkFDqQSFvG=485240897.694173307199520638056759679463;if (QdjimZdSkQFmQXIjQJBVFkFDqQSFvG == QdjimZdSkQFmQXIjQJBVFkFDqQSFvG ) QdjimZdSkQFmQXIjQJBVFkFDqQSFvG=1092016398.619897245319617951364986204551; else QdjimZdSkQFmQXIjQJBVFkFDqQSFvG=427601786.594459073459384825273921983470;if (QdjimZdSkQFmQXIjQJBVFkFDqQSFvG == QdjimZdSkQFmQXIjQJBVFkFDqQSFvG ) QdjimZdSkQFmQXIjQJBVFkFDqQSFvG=1109401472.379583526998892661248948977467; else QdjimZdSkQFmQXIjQJBVFkFDqQSFvG=1175909558.671318090241939513943323244172;if (QdjimZdSkQFmQXIjQJBVFkFDqQSFvG == QdjimZdSkQFmQXIjQJBVFkFDqQSFvG ) QdjimZdSkQFmQXIjQJBVFkFDqQSFvG=1652900930.617043769568101811047552546827; else QdjimZdSkQFmQXIjQJBVFkFDqQSFvG=425167835.493973400685458622344423617550;if (QdjimZdSkQFmQXIjQJBVFkFDqQSFvG == QdjimZdSkQFmQXIjQJBVFkFDqQSFvG ) QdjimZdSkQFmQXIjQJBVFkFDqQSFvG=1809177508.705317534852147211403468408036; else QdjimZdSkQFmQXIjQJBVFkFDqQSFvG=1180391312.705109124119950248741615844702;int aztGXiCYnCulPvUZCoHjDRuYEvzObY=506053081;if (aztGXiCYnCulPvUZCoHjDRuYEvzObY == aztGXiCYnCulPvUZCoHjDRuYEvzObY- 1 ) aztGXiCYnCulPvUZCoHjDRuYEvzObY=1884102869; else aztGXiCYnCulPvUZCoHjDRuYEvzObY=280059011;if (aztGXiCYnCulPvUZCoHjDRuYEvzObY == aztGXiCYnCulPvUZCoHjDRuYEvzObY- 0 ) aztGXiCYnCulPvUZCoHjDRuYEvzObY=1835562969; else aztGXiCYnCulPvUZCoHjDRuYEvzObY=1661941187;if (aztGXiCYnCulPvUZCoHjDRuYEvzObY == aztGXiCYnCulPvUZCoHjDRuYEvzObY- 1 ) aztGXiCYnCulPvUZCoHjDRuYEvzObY=106705521; else aztGXiCYnCulPvUZCoHjDRuYEvzObY=220558774;if (aztGXiCYnCulPvUZCoHjDRuYEvzObY == aztGXiCYnCulPvUZCoHjDRuYEvzObY- 0 ) aztGXiCYnCulPvUZCoHjDRuYEvzObY=258272860; else aztGXiCYnCulPvUZCoHjDRuYEvzObY=1086925960;if (aztGXiCYnCulPvUZCoHjDRuYEvzObY == aztGXiCYnCulPvUZCoHjDRuYEvzObY- 1 ) aztGXiCYnCulPvUZCoHjDRuYEvzObY=129568954; else aztGXiCYnCulPvUZCoHjDRuYEvzObY=300774810;if (aztGXiCYnCulPvUZCoHjDRuYEvzObY == aztGXiCYnCulPvUZCoHjDRuYEvzObY- 1 ) aztGXiCYnCulPvUZCoHjDRuYEvzObY=427375455; else aztGXiCYnCulPvUZCoHjDRuYEvzObY=1352446127;long ZMmofvDoOhocoUsckDqyWTqcMsgyZn=391968862;if (ZMmofvDoOhocoUsckDqyWTqcMsgyZn == ZMmofvDoOhocoUsckDqyWTqcMsgyZn- 0 ) ZMmofvDoOhocoUsckDqyWTqcMsgyZn=1838481622; else ZMmofvDoOhocoUsckDqyWTqcMsgyZn=215891331;if (ZMmofvDoOhocoUsckDqyWTqcMsgyZn == ZMmofvDoOhocoUsckDqyWTqcMsgyZn- 0 ) ZMmofvDoOhocoUsckDqyWTqcMsgyZn=2081771997; else ZMmofvDoOhocoUsckDqyWTqcMsgyZn=1824027727;if (ZMmofvDoOhocoUsckDqyWTqcMsgyZn == ZMmofvDoOhocoUsckDqyWTqcMsgyZn- 1 ) ZMmofvDoOhocoUsckDqyWTqcMsgyZn=2042948516; else ZMmofvDoOhocoUsckDqyWTqcMsgyZn=2036025319;if (ZMmofvDoOhocoUsckDqyWTqcMsgyZn == ZMmofvDoOhocoUsckDqyWTqcMsgyZn- 1 ) ZMmofvDoOhocoUsckDqyWTqcMsgyZn=710642560; else ZMmofvDoOhocoUsckDqyWTqcMsgyZn=2073350963;if (ZMmofvDoOhocoUsckDqyWTqcMsgyZn == ZMmofvDoOhocoUsckDqyWTqcMsgyZn- 1 ) ZMmofvDoOhocoUsckDqyWTqcMsgyZn=264918687; else ZMmofvDoOhocoUsckDqyWTqcMsgyZn=271136512;if (ZMmofvDoOhocoUsckDqyWTqcMsgyZn == ZMmofvDoOhocoUsckDqyWTqcMsgyZn- 0 ) ZMmofvDoOhocoUsckDqyWTqcMsgyZn=902171732; else ZMmofvDoOhocoUsckDqyWTqcMsgyZn=92488444;long SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP=284895127;if (SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP == SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP- 1 ) SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP=898258218; else SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP=1443464902;if (SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP == SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP- 0 ) SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP=1560968513; else SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP=1489455458;if (SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP == SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP- 0 ) SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP=1694628322; else SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP=1846859405;if (SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP == SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP- 0 ) SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP=258909913; else SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP=2018320230;if (SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP == SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP- 1 ) SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP=1812342636; else SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP=362127204;if (SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP == SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP- 0 ) SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP=355671943; else SmuqkJPMIzIcIeVIJMKcCRRVYjYNSP=791241588;double mqiDHKAoqhklJSkyotAktQEpZtPRJz=1629653470.918504006100805429748203156035;if (mqiDHKAoqhklJSkyotAktQEpZtPRJz == mqiDHKAoqhklJSkyotAktQEpZtPRJz ) mqiDHKAoqhklJSkyotAktQEpZtPRJz=975040462.570455911445003017478259771646; else mqiDHKAoqhklJSkyotAktQEpZtPRJz=1417208646.917263045062931431014066742289;if (mqiDHKAoqhklJSkyotAktQEpZtPRJz == mqiDHKAoqhklJSkyotAktQEpZtPRJz ) mqiDHKAoqhklJSkyotAktQEpZtPRJz=1489393337.447664894008242742935063205263; else mqiDHKAoqhklJSkyotAktQEpZtPRJz=124253436.297762538765811783501035549456;if (mqiDHKAoqhklJSkyotAktQEpZtPRJz == mqiDHKAoqhklJSkyotAktQEpZtPRJz ) mqiDHKAoqhklJSkyotAktQEpZtPRJz=239689792.974292168121527036262386226232; else mqiDHKAoqhklJSkyotAktQEpZtPRJz=1799368227.318280272063548061952430570807;if (mqiDHKAoqhklJSkyotAktQEpZtPRJz == mqiDHKAoqhklJSkyotAktQEpZtPRJz ) mqiDHKAoqhklJSkyotAktQEpZtPRJz=610588212.604792334897430094826648199982; else mqiDHKAoqhklJSkyotAktQEpZtPRJz=1995561197.047546986774816610059718117660;if (mqiDHKAoqhklJSkyotAktQEpZtPRJz == mqiDHKAoqhklJSkyotAktQEpZtPRJz ) mqiDHKAoqhklJSkyotAktQEpZtPRJz=1716900098.355386099411915393269994343944; else mqiDHKAoqhklJSkyotAktQEpZtPRJz=1909695653.043653953062274881393441146205;if (mqiDHKAoqhklJSkyotAktQEpZtPRJz == mqiDHKAoqhklJSkyotAktQEpZtPRJz ) mqiDHKAoqhklJSkyotAktQEpZtPRJz=1397022500.326548936344020883056366260262; else mqiDHKAoqhklJSkyotAktQEpZtPRJz=1983052656.885985673706515952246335916148;long llWXiuDZdIGAPcBoaFMJpzIdZbAEiu=818028555;if (llWXiuDZdIGAPcBoaFMJpzIdZbAEiu == llWXiuDZdIGAPcBoaFMJpzIdZbAEiu- 1 ) llWXiuDZdIGAPcBoaFMJpzIdZbAEiu=1561124984; else llWXiuDZdIGAPcBoaFMJpzIdZbAEiu=142480328;if (llWXiuDZdIGAPcBoaFMJpzIdZbAEiu == llWXiuDZdIGAPcBoaFMJpzIdZbAEiu- 1 ) llWXiuDZdIGAPcBoaFMJpzIdZbAEiu=454187909; else llWXiuDZdIGAPcBoaFMJpzIdZbAEiu=529229345;if (llWXiuDZdIGAPcBoaFMJpzIdZbAEiu == llWXiuDZdIGAPcBoaFMJpzIdZbAEiu- 0 ) llWXiuDZdIGAPcBoaFMJpzIdZbAEiu=609762594; else llWXiuDZdIGAPcBoaFMJpzIdZbAEiu=108155562;if (llWXiuDZdIGAPcBoaFMJpzIdZbAEiu == llWXiuDZdIGAPcBoaFMJpzIdZbAEiu- 1 ) llWXiuDZdIGAPcBoaFMJpzIdZbAEiu=844824187; else llWXiuDZdIGAPcBoaFMJpzIdZbAEiu=2139105864;if (llWXiuDZdIGAPcBoaFMJpzIdZbAEiu == llWXiuDZdIGAPcBoaFMJpzIdZbAEiu- 1 ) llWXiuDZdIGAPcBoaFMJpzIdZbAEiu=2104465065; else llWXiuDZdIGAPcBoaFMJpzIdZbAEiu=63451767;if (llWXiuDZdIGAPcBoaFMJpzIdZbAEiu == llWXiuDZdIGAPcBoaFMJpzIdZbAEiu- 0 ) llWXiuDZdIGAPcBoaFMJpzIdZbAEiu=11685091; else llWXiuDZdIGAPcBoaFMJpzIdZbAEiu=1680821312;double ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi=507963342.883643213438612650578767415883;if (ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi == ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi ) ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi=1067921201.989796685941644566507847523623; else ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi=1420001909.943065092618673722406039615338;if (ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi == ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi ) ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi=1264931629.406210979058499027480133802445; else ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi=981038162.176531257396355864169895838442;if (ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi == ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi ) ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi=2117114356.953599207998110599814948638271; else ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi=762880037.381654396766728289635074802597;if (ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi == ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi ) ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi=45205335.113761766630315822156064742886; else ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi=1246373638.381976417852311576009054215913;if (ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi == ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi ) ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi=832876690.098135044951630476437699744787; else ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi=1712890963.251486538753315952484968741950;if (ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi == ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi ) ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi=1920039007.137881025595988449499240330385; else ZJtqyjxCsWMbPVpjWtcpNZgYJlwcJi=1932416316.669962753594700918131266100756;float kSFsRjxMdksdwzdaTDVIJLaRSBMqeL=1646558220.586899943012981062332472942065f;if (kSFsRjxMdksdwzdaTDVIJLaRSBMqeL - kSFsRjxMdksdwzdaTDVIJLaRSBMqeL> 0.00000001 ) kSFsRjxMdksdwzdaTDVIJLaRSBMqeL=1673448052.515236325059808517249422021406f; else kSFsRjxMdksdwzdaTDVIJLaRSBMqeL=1685127035.809951785675749467544592619332f;if (kSFsRjxMdksdwzdaTDVIJLaRSBMqeL - kSFsRjxMdksdwzdaTDVIJLaRSBMqeL> 0.00000001 ) kSFsRjxMdksdwzdaTDVIJLaRSBMqeL=1377600251.592025096783277588366406332744f; else kSFsRjxMdksdwzdaTDVIJLaRSBMqeL=2005229650.255318382448287142614117720670f;if (kSFsRjxMdksdwzdaTDVIJLaRSBMqeL - kSFsRjxMdksdwzdaTDVIJLaRSBMqeL> 0.00000001 ) kSFsRjxMdksdwzdaTDVIJLaRSBMqeL=1450251083.446264907930793932020023114870f; else kSFsRjxMdksdwzdaTDVIJLaRSBMqeL=821469613.971210213424679552599976153426f;if (kSFsRjxMdksdwzdaTDVIJLaRSBMqeL - kSFsRjxMdksdwzdaTDVIJLaRSBMqeL> 0.00000001 ) kSFsRjxMdksdwzdaTDVIJLaRSBMqeL=1019890876.013716059372060645476349100626f; else kSFsRjxMdksdwzdaTDVIJLaRSBMqeL=866884900.890742296451422892661806087472f;if (kSFsRjxMdksdwzdaTDVIJLaRSBMqeL - kSFsRjxMdksdwzdaTDVIJLaRSBMqeL> 0.00000001 ) kSFsRjxMdksdwzdaTDVIJLaRSBMqeL=572384459.654368332945012603741907500487f; else kSFsRjxMdksdwzdaTDVIJLaRSBMqeL=1497771120.683581716557082789449484175886f;if (kSFsRjxMdksdwzdaTDVIJLaRSBMqeL - kSFsRjxMdksdwzdaTDVIJLaRSBMqeL> 0.00000001 ) kSFsRjxMdksdwzdaTDVIJLaRSBMqeL=167272018.896647364199739503593214921941f; else kSFsRjxMdksdwzdaTDVIJLaRSBMqeL=176238025.703176245653546086567230479272f;double TIMWuffcQRfszWygksuWaTqWZoSmzk=1797499126.673990118181366077162805002141;if (TIMWuffcQRfszWygksuWaTqWZoSmzk == TIMWuffcQRfszWygksuWaTqWZoSmzk ) TIMWuffcQRfszWygksuWaTqWZoSmzk=1984054668.386983072022362718786446598899; else TIMWuffcQRfszWygksuWaTqWZoSmzk=889059158.818790648310525164439446695040;if (TIMWuffcQRfszWygksuWaTqWZoSmzk == TIMWuffcQRfszWygksuWaTqWZoSmzk ) TIMWuffcQRfszWygksuWaTqWZoSmzk=915131455.528421567228148603253456057221; else TIMWuffcQRfszWygksuWaTqWZoSmzk=45868780.570989310470722231910390717358;if (TIMWuffcQRfszWygksuWaTqWZoSmzk == TIMWuffcQRfszWygksuWaTqWZoSmzk ) TIMWuffcQRfszWygksuWaTqWZoSmzk=1048336434.535775836949389483482732138028; else TIMWuffcQRfszWygksuWaTqWZoSmzk=2002608284.307304615841507101244507690465;if (TIMWuffcQRfszWygksuWaTqWZoSmzk == TIMWuffcQRfszWygksuWaTqWZoSmzk ) TIMWuffcQRfszWygksuWaTqWZoSmzk=848397846.781974319588874224514899491072; else TIMWuffcQRfszWygksuWaTqWZoSmzk=798244947.435116174399286773457796100369;if (TIMWuffcQRfszWygksuWaTqWZoSmzk == TIMWuffcQRfszWygksuWaTqWZoSmzk ) TIMWuffcQRfszWygksuWaTqWZoSmzk=97274993.444816386825022562373873584902; else TIMWuffcQRfszWygksuWaTqWZoSmzk=1355297918.792606557149863288474909777244;if (TIMWuffcQRfszWygksuWaTqWZoSmzk == TIMWuffcQRfszWygksuWaTqWZoSmzk ) TIMWuffcQRfszWygksuWaTqWZoSmzk=1843836302.358524452702533411130654899304; else TIMWuffcQRfszWygksuWaTqWZoSmzk=1044355278.942490234094687347746621471484;double IatreAIYQHpkDWjAkjXYNdflMNAwIu=1681124161.717054766945562433233152997897;if (IatreAIYQHpkDWjAkjXYNdflMNAwIu == IatreAIYQHpkDWjAkjXYNdflMNAwIu ) IatreAIYQHpkDWjAkjXYNdflMNAwIu=1658485873.189008622740079997289951467617; else IatreAIYQHpkDWjAkjXYNdflMNAwIu=512610518.707986666581159428400172824964;if (IatreAIYQHpkDWjAkjXYNdflMNAwIu == IatreAIYQHpkDWjAkjXYNdflMNAwIu ) IatreAIYQHpkDWjAkjXYNdflMNAwIu=7373260.102614310334170990215031774627; else IatreAIYQHpkDWjAkjXYNdflMNAwIu=2142709988.380348065511918700304576928104;if (IatreAIYQHpkDWjAkjXYNdflMNAwIu == IatreAIYQHpkDWjAkjXYNdflMNAwIu ) IatreAIYQHpkDWjAkjXYNdflMNAwIu=689284368.180316484393680412407542481452; else IatreAIYQHpkDWjAkjXYNdflMNAwIu=67894982.196153261242601100661681875803;if (IatreAIYQHpkDWjAkjXYNdflMNAwIu == IatreAIYQHpkDWjAkjXYNdflMNAwIu ) IatreAIYQHpkDWjAkjXYNdflMNAwIu=1205195906.504331853836237694239289152649; else IatreAIYQHpkDWjAkjXYNdflMNAwIu=246451588.947874848145793655428277442528;if (IatreAIYQHpkDWjAkjXYNdflMNAwIu == IatreAIYQHpkDWjAkjXYNdflMNAwIu ) IatreAIYQHpkDWjAkjXYNdflMNAwIu=400111033.922500046486610560760722177044; else IatreAIYQHpkDWjAkjXYNdflMNAwIu=398046729.232132320471015267808397821143;if (IatreAIYQHpkDWjAkjXYNdflMNAwIu == IatreAIYQHpkDWjAkjXYNdflMNAwIu ) IatreAIYQHpkDWjAkjXYNdflMNAwIu=408653703.300225469824039081791125695631; else IatreAIYQHpkDWjAkjXYNdflMNAwIu=619343236.239717280091819536894045239673;long oPmhtaPLLnzHlECLHcFhssSjIwQcpr=1903109790;if (oPmhtaPLLnzHlECLHcFhssSjIwQcpr == oPmhtaPLLnzHlECLHcFhssSjIwQcpr- 0 ) oPmhtaPLLnzHlECLHcFhssSjIwQcpr=1397683142; else oPmhtaPLLnzHlECLHcFhssSjIwQcpr=122539881;if (oPmhtaPLLnzHlECLHcFhssSjIwQcpr == oPmhtaPLLnzHlECLHcFhssSjIwQcpr- 0 ) oPmhtaPLLnzHlECLHcFhssSjIwQcpr=2095845295; else oPmhtaPLLnzHlECLHcFhssSjIwQcpr=1708261394;if (oPmhtaPLLnzHlECLHcFhssSjIwQcpr == oPmhtaPLLnzHlECLHcFhssSjIwQcpr- 0 ) oPmhtaPLLnzHlECLHcFhssSjIwQcpr=1655693096; else oPmhtaPLLnzHlECLHcFhssSjIwQcpr=506580761;if (oPmhtaPLLnzHlECLHcFhssSjIwQcpr == oPmhtaPLLnzHlECLHcFhssSjIwQcpr- 0 ) oPmhtaPLLnzHlECLHcFhssSjIwQcpr=598221786; else oPmhtaPLLnzHlECLHcFhssSjIwQcpr=1818323415;if (oPmhtaPLLnzHlECLHcFhssSjIwQcpr == oPmhtaPLLnzHlECLHcFhssSjIwQcpr- 1 ) oPmhtaPLLnzHlECLHcFhssSjIwQcpr=836729189; else oPmhtaPLLnzHlECLHcFhssSjIwQcpr=579355304;if (oPmhtaPLLnzHlECLHcFhssSjIwQcpr == oPmhtaPLLnzHlECLHcFhssSjIwQcpr- 1 ) oPmhtaPLLnzHlECLHcFhssSjIwQcpr=1907954657; else oPmhtaPLLnzHlECLHcFhssSjIwQcpr=94953165;long fEhToJVzEXZpwtksvHqoBioxRbThgF=1659976588;if (fEhToJVzEXZpwtksvHqoBioxRbThgF == fEhToJVzEXZpwtksvHqoBioxRbThgF- 1 ) fEhToJVzEXZpwtksvHqoBioxRbThgF=1955985808; else fEhToJVzEXZpwtksvHqoBioxRbThgF=633631504;if (fEhToJVzEXZpwtksvHqoBioxRbThgF == fEhToJVzEXZpwtksvHqoBioxRbThgF- 0 ) fEhToJVzEXZpwtksvHqoBioxRbThgF=665235799; else fEhToJVzEXZpwtksvHqoBioxRbThgF=1275072618;if (fEhToJVzEXZpwtksvHqoBioxRbThgF == fEhToJVzEXZpwtksvHqoBioxRbThgF- 1 ) fEhToJVzEXZpwtksvHqoBioxRbThgF=1069877287; else fEhToJVzEXZpwtksvHqoBioxRbThgF=319981480;if (fEhToJVzEXZpwtksvHqoBioxRbThgF == fEhToJVzEXZpwtksvHqoBioxRbThgF- 1 ) fEhToJVzEXZpwtksvHqoBioxRbThgF=1108214758; else fEhToJVzEXZpwtksvHqoBioxRbThgF=1916159686;if (fEhToJVzEXZpwtksvHqoBioxRbThgF == fEhToJVzEXZpwtksvHqoBioxRbThgF- 1 ) fEhToJVzEXZpwtksvHqoBioxRbThgF=1831346899; else fEhToJVzEXZpwtksvHqoBioxRbThgF=668679867;if (fEhToJVzEXZpwtksvHqoBioxRbThgF == fEhToJVzEXZpwtksvHqoBioxRbThgF- 0 ) fEhToJVzEXZpwtksvHqoBioxRbThgF=1793241394; else fEhToJVzEXZpwtksvHqoBioxRbThgF=650289705;double iRYeYcfCBXCOAZRnVohgHZgigBCeMF=228307683.151462344012852570082124747933;if (iRYeYcfCBXCOAZRnVohgHZgigBCeMF == iRYeYcfCBXCOAZRnVohgHZgigBCeMF ) iRYeYcfCBXCOAZRnVohgHZgigBCeMF=1896607416.181835399740479146313270103772; else iRYeYcfCBXCOAZRnVohgHZgigBCeMF=2088210159.541203113839135385571272002273;if (iRYeYcfCBXCOAZRnVohgHZgigBCeMF == iRYeYcfCBXCOAZRnVohgHZgigBCeMF ) iRYeYcfCBXCOAZRnVohgHZgigBCeMF=1794155783.262820928714459514693821248030; else iRYeYcfCBXCOAZRnVohgHZgigBCeMF=213060321.204068741646896882459968154279;if (iRYeYcfCBXCOAZRnVohgHZgigBCeMF == iRYeYcfCBXCOAZRnVohgHZgigBCeMF ) iRYeYcfCBXCOAZRnVohgHZgigBCeMF=1111529870.200852049287316713557778244602; else iRYeYcfCBXCOAZRnVohgHZgigBCeMF=2128491212.458345605585428266345283363372;if (iRYeYcfCBXCOAZRnVohgHZgigBCeMF == iRYeYcfCBXCOAZRnVohgHZgigBCeMF ) iRYeYcfCBXCOAZRnVohgHZgigBCeMF=25431065.854793007121907245108837838618; else iRYeYcfCBXCOAZRnVohgHZgigBCeMF=1151905112.243373274537932438613928512707;if (iRYeYcfCBXCOAZRnVohgHZgigBCeMF == iRYeYcfCBXCOAZRnVohgHZgigBCeMF ) iRYeYcfCBXCOAZRnVohgHZgigBCeMF=2061872379.659008864417277101540101642697; else iRYeYcfCBXCOAZRnVohgHZgigBCeMF=336533492.257809804567520686141226765946;if (iRYeYcfCBXCOAZRnVohgHZgigBCeMF == iRYeYcfCBXCOAZRnVohgHZgigBCeMF ) iRYeYcfCBXCOAZRnVohgHZgigBCeMF=1305980799.738737148940819469107666613114; else iRYeYcfCBXCOAZRnVohgHZgigBCeMF=109929064.406884080464921251840785017213;long xRQcSWwOYUpujTQnWbXeLQkYjIEqol=424575435;if (xRQcSWwOYUpujTQnWbXeLQkYjIEqol == xRQcSWwOYUpujTQnWbXeLQkYjIEqol- 1 ) xRQcSWwOYUpujTQnWbXeLQkYjIEqol=2014245678; else xRQcSWwOYUpujTQnWbXeLQkYjIEqol=896160762;if (xRQcSWwOYUpujTQnWbXeLQkYjIEqol == xRQcSWwOYUpujTQnWbXeLQkYjIEqol- 1 ) xRQcSWwOYUpujTQnWbXeLQkYjIEqol=810841075; else xRQcSWwOYUpujTQnWbXeLQkYjIEqol=2109460162;if (xRQcSWwOYUpujTQnWbXeLQkYjIEqol == xRQcSWwOYUpujTQnWbXeLQkYjIEqol- 1 ) xRQcSWwOYUpujTQnWbXeLQkYjIEqol=1351215158; else xRQcSWwOYUpujTQnWbXeLQkYjIEqol=331101531;if (xRQcSWwOYUpujTQnWbXeLQkYjIEqol == xRQcSWwOYUpujTQnWbXeLQkYjIEqol- 1 ) xRQcSWwOYUpujTQnWbXeLQkYjIEqol=833232503; else xRQcSWwOYUpujTQnWbXeLQkYjIEqol=1077701662;if (xRQcSWwOYUpujTQnWbXeLQkYjIEqol == xRQcSWwOYUpujTQnWbXeLQkYjIEqol- 1 ) xRQcSWwOYUpujTQnWbXeLQkYjIEqol=1161808770; else xRQcSWwOYUpujTQnWbXeLQkYjIEqol=179685609;if (xRQcSWwOYUpujTQnWbXeLQkYjIEqol == xRQcSWwOYUpujTQnWbXeLQkYjIEqol- 1 ) xRQcSWwOYUpujTQnWbXeLQkYjIEqol=2024398142; else xRQcSWwOYUpujTQnWbXeLQkYjIEqol=987013229;float GzMoWTpNKxacBsGvPBbkBZkYsUbcGb=607674579.254756933608470239568745501695f;if (GzMoWTpNKxacBsGvPBbkBZkYsUbcGb - GzMoWTpNKxacBsGvPBbkBZkYsUbcGb> 0.00000001 ) GzMoWTpNKxacBsGvPBbkBZkYsUbcGb=376718376.531809102015801830785348551926f; else GzMoWTpNKxacBsGvPBbkBZkYsUbcGb=1679646974.535818877123865495587102139335f;if (GzMoWTpNKxacBsGvPBbkBZkYsUbcGb - GzMoWTpNKxacBsGvPBbkBZkYsUbcGb> 0.00000001 ) GzMoWTpNKxacBsGvPBbkBZkYsUbcGb=345829396.928658019307376259426755054489f; else GzMoWTpNKxacBsGvPBbkBZkYsUbcGb=1184390446.964230522749629084698101928470f;if (GzMoWTpNKxacBsGvPBbkBZkYsUbcGb - GzMoWTpNKxacBsGvPBbkBZkYsUbcGb> 0.00000001 ) GzMoWTpNKxacBsGvPBbkBZkYsUbcGb=328863462.473145314674779823717189296431f; else GzMoWTpNKxacBsGvPBbkBZkYsUbcGb=318224377.326247759477003543320687340506f;if (GzMoWTpNKxacBsGvPBbkBZkYsUbcGb - GzMoWTpNKxacBsGvPBbkBZkYsUbcGb> 0.00000001 ) GzMoWTpNKxacBsGvPBbkBZkYsUbcGb=548446718.909446954227534129775412174091f; else GzMoWTpNKxacBsGvPBbkBZkYsUbcGb=1974978243.826358966879011416332283169737f;if (GzMoWTpNKxacBsGvPBbkBZkYsUbcGb - GzMoWTpNKxacBsGvPBbkBZkYsUbcGb> 0.00000001 ) GzMoWTpNKxacBsGvPBbkBZkYsUbcGb=608200439.296450228726301641187977975330f; else GzMoWTpNKxacBsGvPBbkBZkYsUbcGb=1660814334.924968477230872699252876143058f;if (GzMoWTpNKxacBsGvPBbkBZkYsUbcGb - GzMoWTpNKxacBsGvPBbkBZkYsUbcGb> 0.00000001 ) GzMoWTpNKxacBsGvPBbkBZkYsUbcGb=1360683886.699980344438595720638629815993f; else GzMoWTpNKxacBsGvPBbkBZkYsUbcGb=733343795.542975100246592549152452108117f;double WbJvwbfKEeLyUvKEQiQlioTBHtzojk=721104158.370461006321647989343963924834;if (WbJvwbfKEeLyUvKEQiQlioTBHtzojk == WbJvwbfKEeLyUvKEQiQlioTBHtzojk ) WbJvwbfKEeLyUvKEQiQlioTBHtzojk=1547345237.467682522142103533235716842764; else WbJvwbfKEeLyUvKEQiQlioTBHtzojk=1368665959.828206516657720296159270076285;if (WbJvwbfKEeLyUvKEQiQlioTBHtzojk == WbJvwbfKEeLyUvKEQiQlioTBHtzojk ) WbJvwbfKEeLyUvKEQiQlioTBHtzojk=783527674.186932491703779909676672576933; else WbJvwbfKEeLyUvKEQiQlioTBHtzojk=697793819.144257190880645750117861952547;if (WbJvwbfKEeLyUvKEQiQlioTBHtzojk == WbJvwbfKEeLyUvKEQiQlioTBHtzojk ) WbJvwbfKEeLyUvKEQiQlioTBHtzojk=1565697182.767877962391089713792682648330; else WbJvwbfKEeLyUvKEQiQlioTBHtzojk=240660413.666323388889893698325085519533;if (WbJvwbfKEeLyUvKEQiQlioTBHtzojk == WbJvwbfKEeLyUvKEQiQlioTBHtzojk ) WbJvwbfKEeLyUvKEQiQlioTBHtzojk=1024576172.974373677743591499006011578181; else WbJvwbfKEeLyUvKEQiQlioTBHtzojk=1565505885.495602662012269359523053807381;if (WbJvwbfKEeLyUvKEQiQlioTBHtzojk == WbJvwbfKEeLyUvKEQiQlioTBHtzojk ) WbJvwbfKEeLyUvKEQiQlioTBHtzojk=1254977656.550636800083709495332076954381; else WbJvwbfKEeLyUvKEQiQlioTBHtzojk=716454121.549167363356965046541205069489;if (WbJvwbfKEeLyUvKEQiQlioTBHtzojk == WbJvwbfKEeLyUvKEQiQlioTBHtzojk ) WbJvwbfKEeLyUvKEQiQlioTBHtzojk=1198735198.262055565323309711291985044420; else WbJvwbfKEeLyUvKEQiQlioTBHtzojk=931844261.999429402168233890636839948811;long BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD=2113940298;if (BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD == BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD- 0 ) BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD=112164658; else BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD=698306220;if (BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD == BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD- 1 ) BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD=1880694335; else BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD=775260645;if (BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD == BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD- 0 ) BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD=764413345; else BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD=84427831;if (BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD == BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD- 0 ) BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD=95711973; else BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD=1696021301;if (BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD == BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD- 0 ) BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD=347714044; else BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD=983346479;if (BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD == BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD- 1 ) BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD=1501259723; else BVYSMXxsAPFvHQQkdJKYTpsVEjBaoD=1837884471;double BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz=163523076.307295874689255821555660995675;if (BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz == BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz ) BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz=718433542.648880538017443123683611184730; else BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz=1303293984.030914147649354922888310304781;if (BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz == BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz ) BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz=1587151639.781908764908604436726205383171; else BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz=1639640702.147607956637999071698816558663;if (BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz == BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz ) BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz=1311888054.670386201758110042731359444126; else BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz=1674058662.949417861388194070762794242105;if (BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz == BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz ) BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz=1189461044.229692981438145426548446435628; else BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz=136057963.576717961329044576082777899426;if (BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz == BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz ) BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz=655070802.747496243967533940207074690003; else BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz=927807158.864568375833413525595923088381;if (BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz == BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz ) BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz=2076896437.635010407568657476217651904269; else BGnRsAgsiyUwCRKrWrvGUKiEVdvGlz=1759892988.119521841322914848662228418421;int IYmUZrCMggOasuanXlEhjgvorxJFHQ=1569782603;if (IYmUZrCMggOasuanXlEhjgvorxJFHQ == IYmUZrCMggOasuanXlEhjgvorxJFHQ- 0 ) IYmUZrCMggOasuanXlEhjgvorxJFHQ=1369732236; else IYmUZrCMggOasuanXlEhjgvorxJFHQ=1394227235;if (IYmUZrCMggOasuanXlEhjgvorxJFHQ == IYmUZrCMggOasuanXlEhjgvorxJFHQ- 1 ) IYmUZrCMggOasuanXlEhjgvorxJFHQ=621179137; else IYmUZrCMggOasuanXlEhjgvorxJFHQ=463178108;if (IYmUZrCMggOasuanXlEhjgvorxJFHQ == IYmUZrCMggOasuanXlEhjgvorxJFHQ- 1 ) IYmUZrCMggOasuanXlEhjgvorxJFHQ=1265075006; else IYmUZrCMggOasuanXlEhjgvorxJFHQ=833475069;if (IYmUZrCMggOasuanXlEhjgvorxJFHQ == IYmUZrCMggOasuanXlEhjgvorxJFHQ- 0 ) IYmUZrCMggOasuanXlEhjgvorxJFHQ=2104174460; else IYmUZrCMggOasuanXlEhjgvorxJFHQ=1462917406;if (IYmUZrCMggOasuanXlEhjgvorxJFHQ == IYmUZrCMggOasuanXlEhjgvorxJFHQ- 0 ) IYmUZrCMggOasuanXlEhjgvorxJFHQ=1272953986; else IYmUZrCMggOasuanXlEhjgvorxJFHQ=1234990020;if (IYmUZrCMggOasuanXlEhjgvorxJFHQ == IYmUZrCMggOasuanXlEhjgvorxJFHQ- 0 ) IYmUZrCMggOasuanXlEhjgvorxJFHQ=350079775; else IYmUZrCMggOasuanXlEhjgvorxJFHQ=582350703;long DNNQGPLkmsZFlHXdIwqMybOZDwYilp=1155088948;if (DNNQGPLkmsZFlHXdIwqMybOZDwYilp == DNNQGPLkmsZFlHXdIwqMybOZDwYilp- 1 ) DNNQGPLkmsZFlHXdIwqMybOZDwYilp=800236787; else DNNQGPLkmsZFlHXdIwqMybOZDwYilp=1035212122;if (DNNQGPLkmsZFlHXdIwqMybOZDwYilp == DNNQGPLkmsZFlHXdIwqMybOZDwYilp- 1 ) DNNQGPLkmsZFlHXdIwqMybOZDwYilp=213410719; else DNNQGPLkmsZFlHXdIwqMybOZDwYilp=948282733;if (DNNQGPLkmsZFlHXdIwqMybOZDwYilp == DNNQGPLkmsZFlHXdIwqMybOZDwYilp- 0 ) DNNQGPLkmsZFlHXdIwqMybOZDwYilp=1746233491; else DNNQGPLkmsZFlHXdIwqMybOZDwYilp=798925043;if (DNNQGPLkmsZFlHXdIwqMybOZDwYilp == DNNQGPLkmsZFlHXdIwqMybOZDwYilp- 0 ) DNNQGPLkmsZFlHXdIwqMybOZDwYilp=802052244; else DNNQGPLkmsZFlHXdIwqMybOZDwYilp=585589643;if (DNNQGPLkmsZFlHXdIwqMybOZDwYilp == DNNQGPLkmsZFlHXdIwqMybOZDwYilp- 0 ) DNNQGPLkmsZFlHXdIwqMybOZDwYilp=1656328823; else DNNQGPLkmsZFlHXdIwqMybOZDwYilp=1744636372;if (DNNQGPLkmsZFlHXdIwqMybOZDwYilp == DNNQGPLkmsZFlHXdIwqMybOZDwYilp- 1 ) DNNQGPLkmsZFlHXdIwqMybOZDwYilp=120189843; else DNNQGPLkmsZFlHXdIwqMybOZDwYilp=1984089834;float kawDpcbwIczfSmljmULpVOrXRiHKoF=731111587.491162082753614915212506380727f;if (kawDpcbwIczfSmljmULpVOrXRiHKoF - kawDpcbwIczfSmljmULpVOrXRiHKoF> 0.00000001 ) kawDpcbwIczfSmljmULpVOrXRiHKoF=1259944953.382073476973694751124596890689f; else kawDpcbwIczfSmljmULpVOrXRiHKoF=2078960767.347547265255934901264961480728f;if (kawDpcbwIczfSmljmULpVOrXRiHKoF - kawDpcbwIczfSmljmULpVOrXRiHKoF> 0.00000001 ) kawDpcbwIczfSmljmULpVOrXRiHKoF=468152235.499874725869607966290952558716f; else kawDpcbwIczfSmljmULpVOrXRiHKoF=1871157704.855886627039651536463453964732f;if (kawDpcbwIczfSmljmULpVOrXRiHKoF - kawDpcbwIczfSmljmULpVOrXRiHKoF> 0.00000001 ) kawDpcbwIczfSmljmULpVOrXRiHKoF=1626193218.611901493145037131202336451203f; else kawDpcbwIczfSmljmULpVOrXRiHKoF=1600522873.299542885024432853783463148555f;if (kawDpcbwIczfSmljmULpVOrXRiHKoF - kawDpcbwIczfSmljmULpVOrXRiHKoF> 0.00000001 ) kawDpcbwIczfSmljmULpVOrXRiHKoF=1045931717.130095659546639891471862094750f; else kawDpcbwIczfSmljmULpVOrXRiHKoF=2032442120.543190544830525894436819547811f;if (kawDpcbwIczfSmljmULpVOrXRiHKoF - kawDpcbwIczfSmljmULpVOrXRiHKoF> 0.00000001 ) kawDpcbwIczfSmljmULpVOrXRiHKoF=1346603171.201910364046775135821133573466f; else kawDpcbwIczfSmljmULpVOrXRiHKoF=124234233.374463061554059114541219127944f;if (kawDpcbwIczfSmljmULpVOrXRiHKoF - kawDpcbwIczfSmljmULpVOrXRiHKoF> 0.00000001 ) kawDpcbwIczfSmljmULpVOrXRiHKoF=366686716.890877242228448819456902627535f; else kawDpcbwIczfSmljmULpVOrXRiHKoF=76898034.623777438398333497241950990836f;long kEheAZigESqGFhsekuKOASxTlgPtqk=1468379101;if (kEheAZigESqGFhsekuKOASxTlgPtqk == kEheAZigESqGFhsekuKOASxTlgPtqk- 0 ) kEheAZigESqGFhsekuKOASxTlgPtqk=480202565; else kEheAZigESqGFhsekuKOASxTlgPtqk=921807489;if (kEheAZigESqGFhsekuKOASxTlgPtqk == kEheAZigESqGFhsekuKOASxTlgPtqk- 0 ) kEheAZigESqGFhsekuKOASxTlgPtqk=959656201; else kEheAZigESqGFhsekuKOASxTlgPtqk=231760704;if (kEheAZigESqGFhsekuKOASxTlgPtqk == kEheAZigESqGFhsekuKOASxTlgPtqk- 0 ) kEheAZigESqGFhsekuKOASxTlgPtqk=1550675385; else kEheAZigESqGFhsekuKOASxTlgPtqk=922602440;if (kEheAZigESqGFhsekuKOASxTlgPtqk == kEheAZigESqGFhsekuKOASxTlgPtqk- 0 ) kEheAZigESqGFhsekuKOASxTlgPtqk=1971032759; else kEheAZigESqGFhsekuKOASxTlgPtqk=570807193;if (kEheAZigESqGFhsekuKOASxTlgPtqk == kEheAZigESqGFhsekuKOASxTlgPtqk- 0 ) kEheAZigESqGFhsekuKOASxTlgPtqk=592174991; else kEheAZigESqGFhsekuKOASxTlgPtqk=35589494;if (kEheAZigESqGFhsekuKOASxTlgPtqk == kEheAZigESqGFhsekuKOASxTlgPtqk- 1 ) kEheAZigESqGFhsekuKOASxTlgPtqk=954332932; else kEheAZigESqGFhsekuKOASxTlgPtqk=1667922281;long trTRepwMhVKqChUoXjMhXuzzmffeKa=789651354;if (trTRepwMhVKqChUoXjMhXuzzmffeKa == trTRepwMhVKqChUoXjMhXuzzmffeKa- 1 ) trTRepwMhVKqChUoXjMhXuzzmffeKa=805941545; else trTRepwMhVKqChUoXjMhXuzzmffeKa=1766451835;if (trTRepwMhVKqChUoXjMhXuzzmffeKa == trTRepwMhVKqChUoXjMhXuzzmffeKa- 1 ) trTRepwMhVKqChUoXjMhXuzzmffeKa=1469889397; else trTRepwMhVKqChUoXjMhXuzzmffeKa=955407217;if (trTRepwMhVKqChUoXjMhXuzzmffeKa == trTRepwMhVKqChUoXjMhXuzzmffeKa- 1 ) trTRepwMhVKqChUoXjMhXuzzmffeKa=866023113; else trTRepwMhVKqChUoXjMhXuzzmffeKa=723930536;if (trTRepwMhVKqChUoXjMhXuzzmffeKa == trTRepwMhVKqChUoXjMhXuzzmffeKa- 0 ) trTRepwMhVKqChUoXjMhXuzzmffeKa=1676379377; else trTRepwMhVKqChUoXjMhXuzzmffeKa=1347197561;if (trTRepwMhVKqChUoXjMhXuzzmffeKa == trTRepwMhVKqChUoXjMhXuzzmffeKa- 1 ) trTRepwMhVKqChUoXjMhXuzzmffeKa=2136764052; else trTRepwMhVKqChUoXjMhXuzzmffeKa=328452246;if (trTRepwMhVKqChUoXjMhXuzzmffeKa == trTRepwMhVKqChUoXjMhXuzzmffeKa- 0 ) trTRepwMhVKqChUoXjMhXuzzmffeKa=1749163209; else trTRepwMhVKqChUoXjMhXuzzmffeKa=1621999258;int gBKGAwpbTnXWhiPryKvCKaBKJFRdEx=1049218482;if (gBKGAwpbTnXWhiPryKvCKaBKJFRdEx == gBKGAwpbTnXWhiPryKvCKaBKJFRdEx- 1 ) gBKGAwpbTnXWhiPryKvCKaBKJFRdEx=347979099; else gBKGAwpbTnXWhiPryKvCKaBKJFRdEx=799294389;if (gBKGAwpbTnXWhiPryKvCKaBKJFRdEx == gBKGAwpbTnXWhiPryKvCKaBKJFRdEx- 0 ) gBKGAwpbTnXWhiPryKvCKaBKJFRdEx=1503887269; else gBKGAwpbTnXWhiPryKvCKaBKJFRdEx=1380691199;if (gBKGAwpbTnXWhiPryKvCKaBKJFRdEx == gBKGAwpbTnXWhiPryKvCKaBKJFRdEx- 1 ) gBKGAwpbTnXWhiPryKvCKaBKJFRdEx=1671130078; else gBKGAwpbTnXWhiPryKvCKaBKJFRdEx=197287771;if (gBKGAwpbTnXWhiPryKvCKaBKJFRdEx == gBKGAwpbTnXWhiPryKvCKaBKJFRdEx- 1 ) gBKGAwpbTnXWhiPryKvCKaBKJFRdEx=1327919849; else gBKGAwpbTnXWhiPryKvCKaBKJFRdEx=337342513;if (gBKGAwpbTnXWhiPryKvCKaBKJFRdEx == gBKGAwpbTnXWhiPryKvCKaBKJFRdEx- 1 ) gBKGAwpbTnXWhiPryKvCKaBKJFRdEx=107928008; else gBKGAwpbTnXWhiPryKvCKaBKJFRdEx=2044828689;if (gBKGAwpbTnXWhiPryKvCKaBKJFRdEx == gBKGAwpbTnXWhiPryKvCKaBKJFRdEx- 1 ) gBKGAwpbTnXWhiPryKvCKaBKJFRdEx=1278982713; else gBKGAwpbTnXWhiPryKvCKaBKJFRdEx=1590603724;float xdqZsWEICmYslqxNBfMcLnpVDAmQHf=1226518545.989968712753716488605677179978f;if (xdqZsWEICmYslqxNBfMcLnpVDAmQHf - xdqZsWEICmYslqxNBfMcLnpVDAmQHf> 0.00000001 ) xdqZsWEICmYslqxNBfMcLnpVDAmQHf=963815515.867131750534479574886987341389f; else xdqZsWEICmYslqxNBfMcLnpVDAmQHf=1724519009.994509779583917121960520399089f;if (xdqZsWEICmYslqxNBfMcLnpVDAmQHf - xdqZsWEICmYslqxNBfMcLnpVDAmQHf> 0.00000001 ) xdqZsWEICmYslqxNBfMcLnpVDAmQHf=1418440136.058175007091630408022152048421f; else xdqZsWEICmYslqxNBfMcLnpVDAmQHf=1705010699.179064694323598009100217800623f;if (xdqZsWEICmYslqxNBfMcLnpVDAmQHf - xdqZsWEICmYslqxNBfMcLnpVDAmQHf> 0.00000001 ) xdqZsWEICmYslqxNBfMcLnpVDAmQHf=1284704591.868602278782480066505943207336f; else xdqZsWEICmYslqxNBfMcLnpVDAmQHf=441250056.417266590684242883784412893640f;if (xdqZsWEICmYslqxNBfMcLnpVDAmQHf - xdqZsWEICmYslqxNBfMcLnpVDAmQHf> 0.00000001 ) xdqZsWEICmYslqxNBfMcLnpVDAmQHf=602355988.604248523568792271142465200647f; else xdqZsWEICmYslqxNBfMcLnpVDAmQHf=1903755676.495019000537132782172250934015f;if (xdqZsWEICmYslqxNBfMcLnpVDAmQHf - xdqZsWEICmYslqxNBfMcLnpVDAmQHf> 0.00000001 ) xdqZsWEICmYslqxNBfMcLnpVDAmQHf=1392911759.809627379437172634452868525528f; else xdqZsWEICmYslqxNBfMcLnpVDAmQHf=1759942287.510674961651474518931166315110f;if (xdqZsWEICmYslqxNBfMcLnpVDAmQHf - xdqZsWEICmYslqxNBfMcLnpVDAmQHf> 0.00000001 ) xdqZsWEICmYslqxNBfMcLnpVDAmQHf=242480299.286193861887391259412869845547f; else xdqZsWEICmYslqxNBfMcLnpVDAmQHf=1222223139.985248638286826498626681852745f;double NITtyKEYNfHCLLexYRWdhwMCPBWTrg=2014504428.176003019175158364479135808482;if (NITtyKEYNfHCLLexYRWdhwMCPBWTrg == NITtyKEYNfHCLLexYRWdhwMCPBWTrg ) NITtyKEYNfHCLLexYRWdhwMCPBWTrg=773978738.449696872304573330427393004121; else NITtyKEYNfHCLLexYRWdhwMCPBWTrg=2052598097.564874756360317577257917525679;if (NITtyKEYNfHCLLexYRWdhwMCPBWTrg == NITtyKEYNfHCLLexYRWdhwMCPBWTrg ) NITtyKEYNfHCLLexYRWdhwMCPBWTrg=1319943182.275194170079416818764807476836; else NITtyKEYNfHCLLexYRWdhwMCPBWTrg=2061474484.029720523350739950172701648983;if (NITtyKEYNfHCLLexYRWdhwMCPBWTrg == NITtyKEYNfHCLLexYRWdhwMCPBWTrg ) NITtyKEYNfHCLLexYRWdhwMCPBWTrg=763681004.934515180740651427862787533818; else NITtyKEYNfHCLLexYRWdhwMCPBWTrg=2074694813.274153623598154325133113136808;if (NITtyKEYNfHCLLexYRWdhwMCPBWTrg == NITtyKEYNfHCLLexYRWdhwMCPBWTrg ) NITtyKEYNfHCLLexYRWdhwMCPBWTrg=1266004923.337258661980447563332879627202; else NITtyKEYNfHCLLexYRWdhwMCPBWTrg=608653774.793526190676535487677358756874;if (NITtyKEYNfHCLLexYRWdhwMCPBWTrg == NITtyKEYNfHCLLexYRWdhwMCPBWTrg ) NITtyKEYNfHCLLexYRWdhwMCPBWTrg=438531986.257930803148486635525164644598; else NITtyKEYNfHCLLexYRWdhwMCPBWTrg=1132546884.132753071668553908223146019439;if (NITtyKEYNfHCLLexYRWdhwMCPBWTrg == NITtyKEYNfHCLLexYRWdhwMCPBWTrg ) NITtyKEYNfHCLLexYRWdhwMCPBWTrg=847479209.732612773476596771175638500784; else NITtyKEYNfHCLLexYRWdhwMCPBWTrg=968678071.464294705233695775045937520512;int zyrBCKNANwkhwUIxXNqNWlUvZRlnoS=611270969;if (zyrBCKNANwkhwUIxXNqNWlUvZRlnoS == zyrBCKNANwkhwUIxXNqNWlUvZRlnoS- 0 ) zyrBCKNANwkhwUIxXNqNWlUvZRlnoS=1156778999; else zyrBCKNANwkhwUIxXNqNWlUvZRlnoS=522644664;if (zyrBCKNANwkhwUIxXNqNWlUvZRlnoS == zyrBCKNANwkhwUIxXNqNWlUvZRlnoS- 0 ) zyrBCKNANwkhwUIxXNqNWlUvZRlnoS=658183743; else zyrBCKNANwkhwUIxXNqNWlUvZRlnoS=855202965;if (zyrBCKNANwkhwUIxXNqNWlUvZRlnoS == zyrBCKNANwkhwUIxXNqNWlUvZRlnoS- 1 ) zyrBCKNANwkhwUIxXNqNWlUvZRlnoS=1454197721; else zyrBCKNANwkhwUIxXNqNWlUvZRlnoS=1851567351;if (zyrBCKNANwkhwUIxXNqNWlUvZRlnoS == zyrBCKNANwkhwUIxXNqNWlUvZRlnoS- 0 ) zyrBCKNANwkhwUIxXNqNWlUvZRlnoS=1911997538; else zyrBCKNANwkhwUIxXNqNWlUvZRlnoS=2054212690;if (zyrBCKNANwkhwUIxXNqNWlUvZRlnoS == zyrBCKNANwkhwUIxXNqNWlUvZRlnoS- 0 ) zyrBCKNANwkhwUIxXNqNWlUvZRlnoS=196938401; else zyrBCKNANwkhwUIxXNqNWlUvZRlnoS=1747615240;if (zyrBCKNANwkhwUIxXNqNWlUvZRlnoS == zyrBCKNANwkhwUIxXNqNWlUvZRlnoS- 0 ) zyrBCKNANwkhwUIxXNqNWlUvZRlnoS=2135263087; else zyrBCKNANwkhwUIxXNqNWlUvZRlnoS=2058671438;float qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD=1906378154.253156660444856924520615529479f;if (qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD - qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD> 0.00000001 ) qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD=1906259823.965976437655326607814564865964f; else qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD=1411961932.730269009050807649903367640111f;if (qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD - qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD> 0.00000001 ) qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD=816624986.268493627861886487733606157462f; else qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD=741075491.999990341503324358127348866235f;if (qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD - qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD> 0.00000001 ) qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD=205549901.636598200724733048290934675114f; else qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD=1405166057.411698784716951372583805680302f;if (qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD - qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD> 0.00000001 ) qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD=462837541.329731424646568419431463161712f; else qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD=1036604349.859240832005805745737297206667f;if (qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD - qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD> 0.00000001 ) qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD=458514086.082389832969744677953608576205f; else qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD=809786362.527602452601497966474982548492f;if (qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD - qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD> 0.00000001 ) qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD=1266478713.463755224222545232262184265616f; else qSRyQEjfSSAsnBFGtDvxQFNvlSpnfD=152157707.517134984385236245742723048059f;double gAnkCYWEEPZnyqlqdqAJEroWpxTCvf=1307136106.551379454025302343473311213410;if (gAnkCYWEEPZnyqlqdqAJEroWpxTCvf == gAnkCYWEEPZnyqlqdqAJEroWpxTCvf ) gAnkCYWEEPZnyqlqdqAJEroWpxTCvf=1058934667.602715754141487964019982935126; else gAnkCYWEEPZnyqlqdqAJEroWpxTCvf=1778692977.705443379860044393446950282189;if (gAnkCYWEEPZnyqlqdqAJEroWpxTCvf == gAnkCYWEEPZnyqlqdqAJEroWpxTCvf ) gAnkCYWEEPZnyqlqdqAJEroWpxTCvf=65444140.174845614569525904864278186527; else gAnkCYWEEPZnyqlqdqAJEroWpxTCvf=798278214.129180721079998753539932590686;if (gAnkCYWEEPZnyqlqdqAJEroWpxTCvf == gAnkCYWEEPZnyqlqdqAJEroWpxTCvf ) gAnkCYWEEPZnyqlqdqAJEroWpxTCvf=1838333405.919855145638490638144410820779; else gAnkCYWEEPZnyqlqdqAJEroWpxTCvf=1430849548.025517114461800155306354261913;if (gAnkCYWEEPZnyqlqdqAJEroWpxTCvf == gAnkCYWEEPZnyqlqdqAJEroWpxTCvf ) gAnkCYWEEPZnyqlqdqAJEroWpxTCvf=560306707.548772408453489461078647387236; else gAnkCYWEEPZnyqlqdqAJEroWpxTCvf=1069895034.621292314986705497590004601149;if (gAnkCYWEEPZnyqlqdqAJEroWpxTCvf == gAnkCYWEEPZnyqlqdqAJEroWpxTCvf ) gAnkCYWEEPZnyqlqdqAJEroWpxTCvf=140600559.627384230715820295399074663869; else gAnkCYWEEPZnyqlqdqAJEroWpxTCvf=1413214139.227546943236899529441796066373;if (gAnkCYWEEPZnyqlqdqAJEroWpxTCvf == gAnkCYWEEPZnyqlqdqAJEroWpxTCvf ) gAnkCYWEEPZnyqlqdqAJEroWpxTCvf=1864536083.148159848436317438624552963259; else gAnkCYWEEPZnyqlqdqAJEroWpxTCvf=1163549411.178019975079101659973886970639;int fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv=332835276;if (fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv == fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv- 1 ) fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv=2116127249; else fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv=1088584702;if (fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv == fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv- 1 ) fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv=1209901895; else fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv=180469538;if (fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv == fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv- 1 ) fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv=1416634314; else fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv=2107415776;if (fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv == fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv- 0 ) fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv=405721068; else fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv=1899096589;if (fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv == fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv- 0 ) fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv=59807123; else fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv=1769063041;if (fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv == fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv- 0 ) fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv=396688879; else fwyTiqLzoXcPfXaUXCPIVSmbMuHFUv=644411359; }
 fwyTiqLzoXcPfXaUXCPIVSmbMuHFUvy::fwyTiqLzoXcPfXaUXCPIVSmbMuHFUvy()
 { this->PRDNbQstrGJH("ZgRDvCeWlJtDMmZGFSPZbujJNKXGoiPRDNbQstrGJHj", true, 585088638, 1759839831, 312958196); }
#pragma optimize("", off)
 // <delete/>

