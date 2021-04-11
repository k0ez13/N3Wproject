// dear imgui, v1.69 WIP
// (demo code)

// Message to the person tempted to delete this file when integrating Dear ImGui into their code base:
// Do NOT remove this file from your project! Think again! It is the most useful reference code that you and other coders
// will want to refer to and call. Have the ImGui::ShowDemoWindow() function wired in an always-available debug menu of
// your game/app! Removing this file from your project is hindering access to documentation for everyone in your team,
// likely leading you to poorer usage of the library.
// Everything in this file will be stripped out by the linker if you don't call ImGui::ShowDemoWindow().
// If you want to link core Dear ImGui in your shipped builds but want an easy guarantee that the demo will not be linked,
// you can setup your imconfig.h with #define IMGUI_DISABLE_DEMO_WINDOWS and those functions will be empty.
// In other situation, whenever you have Dear ImGui available you probably want this to be available for reference.
// Thank you,
// -Your beloved friend, imgui_demo.cpp (that you won't delete)

// Message to beginner C/C++ programmers about the meaning of the 'static' keyword:
// In this demo code, we frequently we use 'static' variables inside functions. A static variable persist across calls, so it is
// essentially like a global variable but declared inside the scope of the function. We do this as a way to gather code and data
// in the same place, to make the demo source code faster to read, faster to write, and smaller in size.
// It also happens to be a convenient way of storing simple UI related information as long as your function doesn't need to be reentrant
// or used in threads. This might be a pattern you will want to use in your code, but most of the real data you would be editing is
// likely going to be stored outside your functions.

/*
Index of this file:
// [SECTION] Forward Declarations, Helpers
// [SECTION] Demo Window / ShowDemoWindow()
// [SECTION] About Window / ShowAboutWindow()
// [SECTION] Style Editor / ShowStyleEditor()
// [SECTION] Example App: Main Menu Bar / ShowExampleAppMainMenuBar()
// [SECTION] Example App: Debug Console / ShowExampleAppConsole()
// [SECTION] Example App: Debug Log / ShowExampleAppLog()
// [SECTION] Example App: Simple Layout / ShowExampleAppLayout()
// [SECTION] Example App: Property Editor / ShowExampleAppPropertyEditor()
// [SECTION] Example App: Long Text / ShowExampleAppLongText()
// [SECTION] Example App: Auto Resize / ShowExampleAppAutoResize()
// [SECTION] Example App: Constrained Resize / ShowExampleAppConstrainedResize()
// [SECTION] Example App: Simple Overlay / ShowExampleAppSimpleOverlay()
// [SECTION] Example App: Manipulating Window Titles / ShowExampleAppWindowTitles()
// [SECTION] Example App: Custom Rendering using ImDrawList API / ShowExampleAppCustomRendering()
// [SECTION] Example App: Documents Handling / ShowExampleAppDocuments()
*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#include <ctype.h>          // toupper, isprint
#include <limits.h>         // INT_MIN, INT_MAX
#include <math.h>           // sqrtf, powf, cosf, sinf, floorf, ceilf
#include <stdio.h>          // vsnprintf, sscanf, printf
#include <stdlib.h>         // NULL, malloc, free, atoi
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>         // intptr_t
#else
#include <stdint.h>         // intptr_t
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif
#ifdef __clang__
#pragma clang diagnostic ignored "-Wold-style-cast"             // warning : use of old-style cast                              // yes, they are more terse.
#pragma clang diagnostic ignored "-Wdeprecated-declarations"    // warning : 'xx' is deprecated: The POSIX name for this item.. // for strdup used in demo code (so user can copy & paste the code)
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"   // warning : cast to 'void *' from smaller integer type 'int'
#pragma clang diagnostic ignored "-Wformat-security"            // warning : warning: format string is not a string literal
#pragma clang diagnostic ignored "-Wexit-time-destructors"      // warning : declaration requires an exit-time destructor       // exit-time destruction order is undefined. if MemFree() leads to users code that has been disabled before exit it might cause problems. ImGui coding style welcomes static/globals.
#pragma clang diagnostic ignored "-Wunused-macros"              // warning : warning: macro is not used                         // we define snprintf/vsnprintf on Windows so they are available, but not always used.
#if __has_warning("-Wzero-as-null-pointer-constant")
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning : zero as null pointer constant                  // some standard header variations use #define NULL 0
#endif
#if __has_warning("-Wdouble-promotion")
#pragma clang diagnostic ignored "-Wdouble-promotion"           // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#endif
#if __has_warning("-Wreserved-id-macro")
#pragma clang diagnostic ignored "-Wreserved-id-macro"          // warning : macro name is a reserved identifier                //
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"          // warning: cast to pointer from integer of different size
#pragma GCC diagnostic ignored "-Wformat-security"              // warning : format string is not a string literal (potentially insecure)
#pragma GCC diagnostic ignored "-Wdouble-promotion"             // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"                   // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#if (__GNUC__ >= 6)
#pragma GCC diagnostic ignored "-Wmisleading-indentation"       // warning: this 'if' clause does not guard this statement      // GCC 6.0+ only. See #883 on GitHub.
#endif
#endif

// Play it nice with Windows users. Notepad in 2017 still doesn't display text data with Unix-style \n.
#ifdef _WIN32
#define IM_NEWLINE  "\r\n"
#define snprintf    _snprintf
#define vsnprintf   _vsnprintf
#else
#define IM_NEWLINE  "\n"
#endif

#define IM_MAX(_A,_B)       (((_A) >= (_B)) ? (_A) : (_B))

//-----------------------------------------------------------------------------
// [SECTION] Forward Declarations, Helpers
//-----------------------------------------------------------------------------

#if !defined(IMGUI_DISABLE_OBSOLETE_FUNCTIONS) && defined(IMGUI_DISABLE_TEST_WINDOWS) && !defined(IMGUI_DISABLE_DEMO_WINDOWS)   // Obsolete name since 1.53, TEST->DEMO
#define IMGUI_DISABLE_DEMO_WINDOWS
#endif

#if !defined(IMGUI_DISABLE_DEMO_WINDOWS)

// Forward Declarations
static void ShowExampleAppDocuments(bool* p_open);
static void ShowExampleAppMainMenuBar();
static void ShowExampleAppConsole(bool* p_open);
static void ShowExampleAppLog(bool* p_open);
static void ShowExampleAppLayout(bool* p_open);
static void ShowExampleAppPropertyEditor(bool* p_open);
static void ShowExampleAppLongText(bool* p_open);
static void ShowExampleAppAutoResize(bool* p_open);
static void ShowExampleAppConstrainedResize(bool* p_open);
static void ShowExampleAppSimpleOverlay(bool* p_open);
static void ShowExampleAppWindowTitles(bool* p_open);
static void ShowExampleAppCustomRendering(bool* p_open);
static void ShowExampleMenuFile();

// Helper to display a little (?) mark which shows a tooltip when hovered.
static void ShowHelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

// Helper to display basic user controls.
void ImGui::ShowUserGuide()
{
	ImGui::BulletText("Double-click on title bar to collapse window.");
	ImGui::BulletText("Click and drag on lower right corner to resize window\n(double-click to auto fit window to its contents).");
	ImGui::BulletText("Click and drag on any empty space to move window.");
	ImGui::BulletText("TAB/SHIFT+TAB to cycle through keyboard editable fields.");
	ImGui::BulletText("CTRL+Click on a slider or drag box to input value as text.");
	if (ImGui::GetIO().FontAllowUserScaling)
		ImGui::BulletText("CTRL+Mouse Wheel to zoom window contents.");
	ImGui::BulletText("Mouse Wheel to scroll.");
	ImGui::BulletText("While editing text:\n");
	ImGui::Indent();
	ImGui::BulletText("Hold SHIFT or use mouse to select text.");
	ImGui::BulletText("CTRL+Left/Right to word jump.");
	ImGui::BulletText("CTRL+A or double-click to select all.");
	ImGui::BulletText("CTRL+X,CTRL+C,CTRL+V to use clipboard.");
	ImGui::BulletText("CTRL+Z,CTRL+Y to undo/redo.");
	ImGui::BulletText("ESCAPE to revert.");
	ImGui::BulletText("You can apply arithmetic operators +,*,/ on numerical values.\nUse +- to subtract.");
	ImGui::Unindent();
}

//-----------------------------------------------------------------------------
// [SECTION] Demo Window / ShowDemoWindow()
//-----------------------------------------------------------------------------

// We split the contents of the big ShowDemoWindow() function into smaller functions (because the link time of very large functions grow non-linearly)
static void ShowDemoWindowWidgets();
static void ShowDemoWindowLayout();
static void ShowDemoWindowPopups();
static void ShowDemoWindowColumns();
static void ShowDemoWindowMisc();

// Demonstrate most Dear ImGui features (this is big function!)
// You may execute this function to experiment with the UI and understand what it does. You may then search for keywords in the code when you are interested by a specific feature.
void ImGui::ShowDemoWindow(bool* p_open)
{
	// Examples Apps (accessible from the "Examples" menu)
	static bool show_app_documents = false;
	static bool show_app_main_menu_bar = false;
	static bool show_app_console = false;
	static bool show_app_log = false;
	static bool show_app_layout = false;
	static bool show_app_property_editor = false;
	static bool show_app_long_text = false;
	static bool show_app_auto_resize = false;
	static bool show_app_constrained_resize = false;
	static bool show_app_simple_overlay = false;
	static bool show_app_window_titles = false;
	static bool show_app_custom_rendering = false;

	if (show_app_documents)           ShowExampleAppDocuments(&show_app_documents);     // Process the Document app next, as it may also use a DockSpace()
	if (show_app_main_menu_bar)       ShowExampleAppMainMenuBar();
	if (show_app_console)             ShowExampleAppConsole(&show_app_console);
	if (show_app_log)                 ShowExampleAppLog(&show_app_log);
	if (show_app_layout)              ShowExampleAppLayout(&show_app_layout);
	if (show_app_property_editor)     ShowExampleAppPropertyEditor(&show_app_property_editor);
	if (show_app_long_text)           ShowExampleAppLongText(&show_app_long_text);
	if (show_app_auto_resize)         ShowExampleAppAutoResize(&show_app_auto_resize);
	if (show_app_constrained_resize)  ShowExampleAppConstrainedResize(&show_app_constrained_resize);
	if (show_app_simple_overlay)      ShowExampleAppSimpleOverlay(&show_app_simple_overlay);
	if (show_app_window_titles)       ShowExampleAppWindowTitles(&show_app_window_titles);
	if (show_app_custom_rendering)    ShowExampleAppCustomRendering(&show_app_custom_rendering);

	// Dear ImGui Apps (accessible from the "Help" menu)
	static bool show_app_metrics = false;
	static bool show_app_style_editor = false;
	static bool show_app_about = false;

	if (show_app_metrics) { ImGui::ShowMetricsWindow(&show_app_metrics); }
	if (show_app_style_editor) { ImGui::Begin("Style Editor", &show_app_style_editor); ImGui::ShowStyleEditor(); ImGui::End(); }
	if (show_app_about) { ImGui::ShowAboutWindow(&show_app_about); }

	// Demonstrate the various window flags. Typically you would just use the default!
	static bool no_titlebar = false;
	static bool no_scrollbar = false;
	static bool no_menu = false;
	static bool no_move = false;
	static bool no_resize = false;
	static bool no_collapse = false;
	static bool no_close = false;
	static bool no_nav = false;
	static bool no_background = false;
	static bool no_bring_to_front = false;

	ImGuiWindowFlags window_flags = 0;
	if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
	if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
	if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
	if (no_move)            window_flags |= ImGuiWindowFlags_NoMove;
	if (no_resize)          window_flags |= ImGuiWindowFlags_NoResize;
	if (no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
	if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
	if (no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
	if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	if (no_close)           p_open = NULL; // Don't pass our bool* to Begin

	// We specify a default position/size in case there's no data in the .ini file. Typically this isn't required! We only do it to make the Demo applications a little more welcoming.
	ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);

	// Main body of the Demo window starts here.
	if (!ImGui::Begin("ImGui Demo", p_open, window_flags))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}
	ImGui::Text("dear imgui says hello. (%s)", IMGUI_VERSION);

	// Most "big" widgets share a common width settings by default.
	//ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);    // Use 2/3 of the space for widgets and 1/3 for labels (default)
	ImGui::PushItemWidth(ImGui::GetFontSize() * -12);           // Use fixed width for labels (by passing a negative value), the rest goes to widgets. We choose a width proportional to our font size.

	// Menu
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			ShowExampleMenuFile();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Examples"))
		{
			ImGui::MenuItem("Main menu bar", NULL, &show_app_main_menu_bar);
			ImGui::MenuItem("Console", NULL, &show_app_console);
			ImGui::MenuItem("Log", NULL, &show_app_log);
			ImGui::MenuItem("Simple layout", NULL, &show_app_layout);
			ImGui::MenuItem("Property editor", NULL, &show_app_property_editor);
			ImGui::MenuItem("Long text display", NULL, &show_app_long_text);
			ImGui::MenuItem("Auto-resizing window", NULL, &show_app_auto_resize);
			ImGui::MenuItem("Constrained-resizing window", NULL, &show_app_constrained_resize);
			ImGui::MenuItem("Simple overlay", NULL, &show_app_simple_overlay);
			ImGui::MenuItem("Manipulating window titles", NULL, &show_app_window_titles);
			ImGui::MenuItem("Custom rendering", NULL, &show_app_custom_rendering);
			ImGui::MenuItem("Documents", NULL, &show_app_documents);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help"))
		{
			ImGui::MenuItem("Metrics", NULL, &show_app_metrics);
			ImGui::MenuItem("Style Editor", NULL, &show_app_style_editor);
			ImGui::MenuItem("About Dear ImGui", NULL, &show_app_about);
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	ImGui::Spacing();
	if (ImGui::CollapsingHeader("Help"))
	{
		ImGui::Text("PROGRAMMER GUIDE:");
		ImGui::BulletText("Please see the ShowDemoWindow() code in imgui_demo.cpp. <- you are here!");
		ImGui::BulletText("Please see the comments in imgui.cpp.");
		ImGui::BulletText("Please see the examples/ in application.");
		ImGui::BulletText("Enable 'io.ConfigFlags |= NavEnableKeyboard' for keyboard controls.");
		ImGui::BulletText("Enable 'io.ConfigFlags |= NavEnableGamepad' for gamepad controls.");
		ImGui::Separator();

		ImGui::Text("USER GUIDE:");
		ImGui::ShowUserGuide();
	}

	if (ImGui::CollapsingHeader("Configuration"))
	{
		ImGuiIO& io = ImGui::GetIO();

		if (ImGui::TreeNode("Configuration##2"))
		{
			ImGui::CheckboxFlags("io.ConfigFlags: NavEnableKeyboard", (unsigned int *)&io.ConfigFlags, ImGuiConfigFlags_NavEnableKeyboard);
			ImGui::CheckboxFlags("io.ConfigFlags: NavEnableGamepad", (unsigned int *)&io.ConfigFlags, ImGuiConfigFlags_NavEnableGamepad);
			ImGui::SameLine(); ShowHelpMarker("Required back-end to feed in gamepad inputs in io.NavInputs[] and set io.BackendFlags |= ImGuiBackendFlags_HasGamepad.\n\nRead instructions in imgui.cpp for details.");
			ImGui::CheckboxFlags("io.ConfigFlags: NavEnableSetMousePos", (unsigned int *)&io.ConfigFlags, ImGuiConfigFlags_NavEnableSetMousePos);
			ImGui::SameLine(); ShowHelpMarker("Instruct navigation to move the mouse cursor. See comment for ImGuiConfigFlags_NavEnableSetMousePos.");
			ImGui::CheckboxFlags("io.ConfigFlags: NoMouse", (unsigned int *)&io.ConfigFlags, ImGuiConfigFlags_NoMouse);
			if (io.ConfigFlags & ImGuiConfigFlags_NoMouse) // Create a way to restore this flag otherwise we could be stuck completely!
			{
				if (fmodf((float)ImGui::GetTime(), 0.40f) < 0.20f)
				{
					ImGui::SameLine();
					ImGui::Text("<<PRESS SPACE TO DISABLE>>");
				}
				if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Space)))
					io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
			}
			ImGui::CheckboxFlags("io.ConfigFlags: NoMouseCursorChange", (unsigned int *)&io.ConfigFlags, ImGuiConfigFlags_NoMouseCursorChange);
			ImGui::SameLine(); ShowHelpMarker("Instruct back-end to not alter mouse cursor shape and visibility.");
			ImGui::Checkbox("io.ConfigInputTextCursorBlink", &io.ConfigInputTextCursorBlink);
			ImGui::SameLine(); ShowHelpMarker("Set to false to disable blinking cursor, for users who consider it distracting");
			ImGui::Checkbox("io.ConfigWindowsResizeFromEdges", &io.ConfigWindowsResizeFromEdges);
			ImGui::SameLine(); ShowHelpMarker("Enable resizing of windows from their edges and from the lower-left corner.\nThis requires (io.BackendFlags & ImGuiBackendFlags_HasMouseCursors) because it needs mouse cursor feedback.");
			ImGui::Checkbox("io.ConfigWindowsMoveFromTitleBarOnly", &io.ConfigWindowsMoveFromTitleBarOnly);
			ImGui::Checkbox("io.MouseDrawCursor", &io.MouseDrawCursor);
			ImGui::SameLine(); ShowHelpMarker("Instruct Dear ImGui to render a mouse cursor for you. Note that a mouse cursor rendered via your application GPU rendering path will feel more laggy than hardware cursor, but will be more in sync with your other visuals.\n\nSome desktop applications may use both kinds of cursors (e.g. enable software cursor only when resizing/dragging something).");
			ImGui::TreePop();
			ImGui::Separator();
		}

		if (ImGui::TreeNode("Backend Flags"))
		{
			ShowHelpMarker("Those flags are set by the back-ends (imgui_impl_xxx files) to specify their capabilities.");
			ImGuiBackendFlags backend_flags = io.BackendFlags; // Make a local copy to avoid modifying the back-end flags.
			ImGui::CheckboxFlags("io.BackendFlags: HasGamepad", (unsigned int *)&backend_flags, ImGuiBackendFlags_HasGamepad);
			ImGui::CheckboxFlags("io.BackendFlags: HasMouseCursors", (unsigned int *)&backend_flags, ImGuiBackendFlags_HasMouseCursors);
			ImGui::CheckboxFlags("io.BackendFlags: HasSetMousePos", (unsigned int *)&backend_flags, ImGuiBackendFlags_HasSetMousePos);
			ImGui::TreePop();
			ImGui::Separator();
		}

		if (ImGui::TreeNode("Style"))
		{
			ImGui::ShowStyleEditor();
			ImGui::TreePop();
			ImGui::Separator();
		}

		if (ImGui::TreeNode("Capture/Logging"))
		{
			ImGui::TextWrapped("The logging API redirects all text output so you can easily capture the content of a window or a block. Tree nodes can be automatically expanded.");
			ShowHelpMarker("Try opening any of the contents below in this window and then click one of the \"Log To\" button.");
			ImGui::LogButtons();
			ImGui::TextWrapped("You can also call ImGui::LogText() to output directly to the log without a visual output.");
			if (ImGui::Button("Copy \"Hello, world!\" to clipboard"))
			{
				ImGui::LogToClipboard();
				ImGui::LogText("Hello, world!");
				ImGui::LogFinish();
			}
			ImGui::TreePop();
		}
	}

	if (ImGui::CollapsingHeader("Window options"))
	{
		ImGui::Checkbox("No titlebar", &no_titlebar); ImGui::SameLine(150);
		ImGui::Checkbox("No scrollbar", &no_scrollbar); ImGui::SameLine(300);
		ImGui::Checkbox("No menu", &no_menu);
		ImGui::Checkbox("No move", &no_move); ImGui::SameLine(150);
		ImGui::Checkbox("No resize", &no_resize); ImGui::SameLine(300);
		ImGui::Checkbox("No collapse", &no_collapse);
		ImGui::Checkbox("No close", &no_close); ImGui::SameLine(150);
		ImGui::Checkbox("No nav", &no_nav); ImGui::SameLine(300);
		ImGui::Checkbox("No background", &no_background);
		ImGui::Checkbox("No bring to front", &no_bring_to_front);
	}

	// All demo contents
	ShowDemoWindowWidgets();
	ShowDemoWindowLayout();
	ShowDemoWindowPopups();
	ShowDemoWindowColumns();
	ShowDemoWindowMisc();

	// End of ShowDemoWindow()
	ImGui::End();
}

static void ShowDemoWindowWidgets()
{
	if (!ImGui::CollapsingHeader("Widgets"))
		return;

	if (ImGui::TreeNode("Basic"))
	{
		static int clicked = 0;
		if (ImGui::Button("Button"))
			clicked++;
		if (clicked & 1)
		{
			ImGui::SameLine();
			ImGui::Text("Thanks for clicking me!");
		}

		static bool check = true;
		ImGui::Checkbox("checkbox", &check);

		static int e = 0;
		ImGui::RadioButton("radio a", &e, 0); ImGui::SameLine();
		ImGui::RadioButton("radio b", &e, 1); ImGui::SameLine();
		ImGui::RadioButton("radio c", &e, 2);

		// Color buttons, demonstrate using PushID() to add unique identifier in the ID stack, and changing style.
		for (int i = 0; i < 7; i++)
		{
			if (i > 0)
				ImGui::SameLine();
			ImGui::PushID(i);
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(i / 7.0f, 0.7f, 0.7f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(i / 7.0f, 0.8f, 0.8f));
			ImGui::Button("Click");
			ImGui::PopStyleColor(3);
			ImGui::PopID();
		}

		// Use AlignTextToFramePadding() to align text baseline to the baseline of framed elements (otherwise a Text+SameLine+Button sequence will have the text a little too high by default)
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Hold to repeat:");
		ImGui::SameLine();

		// Arrow buttons with Repeater
		static int counter = 0;
		float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
		ImGui::PushButtonRepeat(true);
		if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { counter--; }
		ImGui::SameLine(0.0f, spacing);
		if (ImGui::ArrowButton("##right", ImGuiDir_Right)) { counter++; }
		ImGui::PopButtonRepeat();
		ImGui::SameLine();
		ImGui::Text("%d", counter);

		ImGui::Text("Hover over me");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("I am a tooltip");

		ImGui::SameLine();
		ImGui::Text("- or me");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("I am a fancy tooltip");
			static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
			ImGui::PlotLines("Curve", arr, IM_ARRAYSIZE(arr));
			ImGui::EndTooltip();
		}

		ImGui::Separator();

		ImGui::LabelText("label", "Value");

		{
			// Using the _simplified_ one-liner Combo() api here
			// See "Combo" section for examples of how to use the more complete BeginCombo()/EndCombo() api.
			const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
			static int item_current = 0;
			ImGui::Combo("combo", &item_current, items, IM_ARRAYSIZE(items));
			ImGui::SameLine(); ShowHelpMarker("Refer to the \"Combo\" section below for an explanation of the full BeginCombo/EndCombo API, and demonstration of various flags.\n");
		}

		{
			static char str0[128] = "Hello, world!";
			static int i0 = 123;
			ImGui::InputText("input text", str0, IM_ARRAYSIZE(str0));
			ImGui::SameLine(); ShowHelpMarker("USER:\nHold SHIFT or use mouse to select text.\n" "CTRL+Left/Right to word jump.\n" "CTRL+A or double-click to select all.\n" "CTRL+X,CTRL+C,CTRL+V clipboard.\n" "CTRL+Z,CTRL+Y undo/redo.\n" "ESCAPE to revert.\n\nPROGRAMMER:\nYou can use the ImGuiInputTextFlags_CallbackResize facility if you need to wire InputText() to a dynamic string type. See misc/cpp/imgui_stdlib.h for an example (this is not demonstrated in imgui_demo.cpp).");

			ImGui::InputInt("input int", &i0);
			ImGui::SameLine(); ShowHelpMarker("You can apply arithmetic operators +,*,/ on numerical values.\n  e.g. [ 100 ], input \'*2\', result becomes [ 200 ]\nUse +- to subtract.\n");

			static float f0 = 0.001f;
			ImGui::InputFloat("input float", &f0, 0.01f, 1.0f, "%.3f");

			static double d0 = 999999.00000001;
			ImGui::InputDouble("input double", &d0, 0.01f, 1.0f, "%.8f");

			static float f1 = 1.e10f;
			ImGui::InputFloat("input scientific", &f1, 0.0f, 0.0f, "%e");
			ImGui::SameLine(); ShowHelpMarker("You can input value using the scientific notation,\n  e.g. \"1e+8\" becomes \"100000000\".\n");

			static float vec4a[4] = { 0.10f, 0.20f, 0.30f, 0.44f };
			ImGui::InputFloat3("input float3", vec4a);
		}

		{
			static int i1 = 50, i2 = 42;
			ImGui::DragInt("drag int", &i1, 1);
			ImGui::SameLine(); ShowHelpMarker("Click and drag to edit value.\nHold SHIFT/ALT for faster/slower edit.\nDouble-click or CTRL+click to input value.");

			ImGui::DragInt("drag int 0..100", &i2, 1, 0, 100, "%d%%");

			static float f1 = 1.00f, f2 = 0.0067f;
			ImGui::DragFloat("drag float", &f1, 0.005f);
			ImGui::DragFloat("drag small float", &f2, 0.0001f, 0.0f, 0.0f, "%.06f ns");
		}

		{
			static int i1 = 0;
			ImGui::SliderInt("slider int", &i1, -1, 3);
			ImGui::SameLine(); ShowHelpMarker("CTRL+click to input value.");

			static float f1 = 0.123f, f2 = 0.0f;
			ImGui::SliderFloat("slider float", &f1, 0.0f, 1.0f, "ratio = %.3f");
			ImGui::SliderFloat("slider float (curve)", &f2, -10.0f, 10.0f, "%.4f", 2.0f);
			static float angle = 0.0f;
			ImGui::SliderAngle("slider angle", &angle);
		}

		{
			static float col1[3] = { 1.0f,0.0f,0.2f };
			static float col2[4] = { 0.4f,0.7f,0.0f,0.5f };
			//ImGui::ColorEdit3("color 1", col1);
			ImGui::SameLine(); ShowHelpMarker("Click on the colored square to open a color picker.\nClick and hold to use drag and drop.\nRight-click on the colored square to show options.\nCTRL+click on individual component to input value.\n");

			//ImGui::ColorEdit4("color 2", col2);
		}

		{
			// List box
			const char* listbox_items[] = { "Apple", "Banana", "Cherry", "Kiwi", "Mango", "Orange", "Pineapple", "Strawberry", "Watermelon" };
			static int listbox_item_current = 1;
			ImGui::ListBox("listbox\n(single select)", &listbox_item_current, listbox_items, IM_ARRAYSIZE(listbox_items), 4);

			//static int listbox_item_current2 = 2;
			//ImGui::PushItemWidth(-1);
			//ImGui::ListBox("##listbox2", &listbox_item_current2, listbox_items, IM_ARRAYSIZE(listbox_items), 4);
			//ImGui::PopItemWidth();
		}

		ImGui::TreePop();
	}

	// Testing ImGuiOnceUponAFrame helper.
	//static ImGuiOnceUponAFrame once;
	//for (int i = 0; i < 5; i++)
	//    if (once)
	//        ImGui::Text("This will be displayed only once.");

	if (ImGui::TreeNode("Trees"))
	{
		if (ImGui::TreeNode("Basic trees"))
		{
			for (int i = 0; i < 5; i++)
				if (ImGui::TreeNode((void*)(intptr_t)i, "Child %d", i))
				{
					ImGui::Text("blah blah");
					ImGui::SameLine();
					if (ImGui::SmallButton("button")) {};
					ImGui::TreePop();
				}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Advanced, with Selectable nodes"))
		{
			ShowHelpMarker("This is a more standard looking tree with selectable nodes.\nClick to select, CTRL+Click to toggle, click on arrows or double-click to open.");
			static bool align_label_with_current_x_position = false;
			ImGui::Checkbox("Align label with current X position)", &align_label_with_current_x_position);
			ImGui::Text("Hello!");
			if (align_label_with_current_x_position)
				ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());

			static int selection_mask = (1 << 2); // Dumb representation of what may be user-side selection state. You may carry selection state inside or outside your objects in whatever format you see fit.
			int node_clicked = -1;                // Temporary storage of what node we have clicked to process selection at the end of the loop. May be a pointer to your own node type, etc.
			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 3); // Increase spacing to differentiate leaves from expanded contents.
			for (int i = 0; i < 6; i++)
			{
				// Disable the default open on single-click behavior and pass in Selected flag according to our selection state.
				ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ((selection_mask & (1 << i)) ? ImGuiTreeNodeFlags_Selected : 0);
				if (i < 3)
				{
					// Node
					bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, "Selectable Node %d", i);
					if (ImGui::IsItemClicked())
						node_clicked = i;
					if (node_open)
					{
						ImGui::Text("Blah blah\nBlah Blah");
						ImGui::TreePop();
					}
				}
				else
				{
					// Leaf: The only reason we have a TreeNode at all is to allow selection of the leaf. Otherwise we can use BulletText() or TreeAdvanceToLabelPos()+Text().
					node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
					ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, "Selectable Leaf %d", i);
					if (ImGui::IsItemClicked())
						node_clicked = i;
				}
			}
			if (node_clicked != -1)
			{
				// Update selection state. Process outside of tree loop to avoid visual inconsistencies during the clicking-frame.
				if (ImGui::GetIO().KeyCtrl)
					selection_mask ^= (1 << node_clicked);          // CTRL+click to toggle
				else //if (!(selection_mask & (1 << node_clicked))) // Depending on selection behavior you want, this commented bit preserve selection when clicking on item that is part of the selection
					selection_mask = (1 << node_clicked);           // Click to single-select
			}
			ImGui::PopStyleVar();
			if (align_label_with_current_x_position)
				ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Collapsing Headers"))
	{
		static bool closable_group = true;
		ImGui::Checkbox("Enable extra group", &closable_group);
		if (ImGui::CollapsingHeader("Header"))
		{
			ImGui::Text("IsItemHovered: %d", ImGui::IsItemHovered());
			for (int i = 0; i < 5; i++)
				ImGui::Text("Some content %d", i);
		}
		if (ImGui::CollapsingHeader("Header with a close button", &closable_group))
		{
			ImGui::Text("IsItemHovered: %d", ImGui::IsItemHovered());
			for (int i = 0; i < 5; i++)
				ImGui::Text("More content %d", i);
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Bullets"))
	{
		ImGui::BulletText("Bullet point 1");
		ImGui::BulletText("Bullet point 2\nOn multiple lines");
		ImGui::Bullet(); ImGui::Text("Bullet point 3 (two calls)");
		ImGui::Bullet(); ImGui::SmallButton("Button");
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Text"))
	{
		if (ImGui::TreeNode("Colored Text"))
		{
			// Using shortcut. You can use PushStyleColor()/PopStyleColor() for more flexibility.
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Pink");
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Yellow");
			ImGui::TextDisabled("Disabled");
			ImGui::SameLine(); ShowHelpMarker("The TextDisabled color is stored in ImGuiStyle.");
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Word Wrapping"))
		{
			// Using shortcut. You can use PushTextWrapPos()/PopTextWrapPos() for more flexibility.
			ImGui::TextWrapped("This text should automatically wrap on the edge of the window. The current implementation for text wrapping follows simple rules suitable for English and possibly other languages.");
			ImGui::Spacing();

			static float wrap_width = 200.0f;
			ImGui::SliderFloat("Wrap width", &wrap_width, -20, 600, "%.0f");

			ImGui::Text("Test paragraph 1:");
			ImVec2 pos = ImGui::GetCursorScreenPos();
			ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x + wrap_width, pos.y), ImVec2(pos.x + wrap_width + 10, pos.y + ImGui::GetTextLineHeight()), IM_COL32(255, 0, 255, 255));
			ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrap_width);
			ImGui::Text("The lazy dog is a good dog. This paragraph is made to fit within %.0f pixels. Testing a 1 character word. The quick brown fox jumps over the lazy dog.", wrap_width);
			ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 0, 255));
			ImGui::PopTextWrapPos();

			ImGui::Text("Test paragraph 2:");
			pos = ImGui::GetCursorScreenPos();
			ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x + wrap_width, pos.y), ImVec2(pos.x + wrap_width + 10, pos.y + ImGui::GetTextLineHeight()), IM_COL32(255, 0, 255, 255));
			ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrap_width);
			ImGui::Text("aaaaaaaa bbbbbbbb, c cccccccc,dddddddd. d eeeeeeee   ffffffff. gggggggg!hhhhhhhh");
			ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 0, 255));
			ImGui::PopTextWrapPos();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("UTF-8 Text"))
		{
			// UTF-8 test with Japanese characters
			// (Needs a suitable font, try Noto, or Arial Unicode, or M+ fonts. Read misc/fonts/README.txt for details.)
			// - From C++11 you can use the u8"my text" syntax to encode literal strings as UTF-8
			// - For earlier compiler, you may be able to encode your sources as UTF-8 (e.g. Visual Studio save your file as 'UTF-8 without signature')
			// - FOR THIS DEMO FILE ONLY, BECAUSE WE WANT TO SUPPORT OLD COMPILERS, WE ARE *NOT* INCLUDING RAW UTF-8 CHARACTERS IN THIS SOURCE FILE.
			//   Instead we are encoding a few strings with hexadecimal constants. Don't do this in your application!
			//   Please use u8"text in any language" in your application!
			// Note that characters values are preserved even by InputText() if the font cannot be displayed, so you can safely copy & paste garbled characters into another application.
			ImGui::TextWrapped("CJK text will only appears if the font was loaded with the appropriate CJK character ranges. Call io.Font->AddFontFromFileTTF() manually to load extra character ranges. Read misc/fonts/README.txt for details.");
			ImGui::Text("Hiragana: \xe3\x81\x8b\xe3\x81\x8d\xe3\x81\x8f\xe3\x81\x91\xe3\x81\x93 (kakikukeko)"); // Normally we would use u8"blah blah" with the proper characters directly in the string.
			ImGui::Text("Kanjis: \xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e (nihongo)");
			static char buf[32] = "\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e";
			//static char buf[32] = u8"NIHONGO"; // <- this is how you would write it with C++11, using real kanjis
			ImGui::InputText("UTF-8 input", buf, IM_ARRAYSIZE(buf));
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Images"))
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGui::TextWrapped("Below we are displaying the font texture (which is the only texture we have access to in this demo). Use the 'ImTextureID' type as storage to pass pointers or identifier to your own texture data. Hover the texture for a zoomed view!");

		// Here we are grabbing the font texture because that's the only one we have access to inside the demo code.
		// Remember that ImTextureID is just storage for whatever you want it to be, it is essentially a value that will be passed to the render function inside the ImDrawCmd structure.
		// If you use one of the default imgui_impl_XXXX.cpp renderer, they all have comments at the top of their file to specify what they expect to be stored in ImTextureID.
		// (for example, the imgui_impl_dx11.cpp renderer expect a 'ID3D11ShaderResourceView*' pointer. The imgui_impl_glfw_gl3.cpp renderer expect a GLuint OpenGL texture identifier etc.)
		// If you decided that ImTextureID = MyEngineTexture*, then you can pass your MyEngineTexture* pointers to ImGui::Image(), and gather width/height through your own functions, etc.
		// Using ShowMetricsWindow() as a "debugger" to inspect the draw data that are being passed to your render will help you debug issues if you are confused about this.
		// Consider using the lower-level ImDrawList::AddImage() API, via ImGui::GetWindowDrawList()->AddImage().
		ImTextureID my_tex_id = io.Fonts->TexID;
		float my_tex_w = (float)io.Fonts->TexWidth;
		float my_tex_h = (float)io.Fonts->TexHeight;

		ImGui::Text("%.0fx%.0f", my_tex_w, my_tex_h);
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			float region_sz = 32.0f;
			float region_x = io.MousePos.x - pos.x - region_sz * 0.5f; if (region_x < 0.0f) region_x = 0.0f; else if (region_x > my_tex_w - region_sz) region_x = my_tex_w - region_sz;
			float region_y = io.MousePos.y - pos.y - region_sz * 0.5f; if (region_y < 0.0f) region_y = 0.0f; else if (region_y > my_tex_h - region_sz) region_y = my_tex_h - region_sz;
			float zoom = 4.0f;
			ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
			ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
			ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
			ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
			ImGui::Image(my_tex_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
			ImGui::EndTooltip();
		}
		ImGui::TextWrapped("And now some textured buttons..");
		static int pressed_count = 0;
		for (int i = 0; i < 8; i++)
		{
			ImGui::PushID(i);
			int frame_padding = -1 + i;     // -1 = uses default padding
			if (ImGui::ImageButton(my_tex_id, ImVec2(32, 32), ImVec2(0, 0), ImVec2(32.0f / my_tex_w, 32 / my_tex_h), frame_padding, ImColor(0, 0, 0, 255)))
				pressed_count += 1;
			ImGui::PopID();
			ImGui::SameLine();
		}
		ImGui::NewLine();
		ImGui::Text("Pressed %d times.", pressed_count);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Combo"))
	{
		// Expose flags as checkbox for the demo
		static ImGuiComboFlags flags = 0;
		ImGui::CheckboxFlags("ImGuiComboFlags_PopupAlignLeft", (unsigned int*)&flags, ImGuiComboFlags_PopupAlignLeft);
		if (ImGui::CheckboxFlags("ImGuiComboFlags_NoArrowButton", (unsigned int*)&flags, ImGuiComboFlags_NoArrowButton))
			flags &= ~ImGuiComboFlags_NoPreview;     // Clear the other flag, as we cannot combine both
		if (ImGui::CheckboxFlags("ImGuiComboFlags_NoPreview", (unsigned int*)&flags, ImGuiComboFlags_NoPreview))
			flags &= ~ImGuiComboFlags_NoArrowButton; // Clear the other flag, as we cannot combine both

		// General BeginCombo() API, you have full control over your selection data and display type.
		// (your selection data could be an index, a pointer to the object, an id for the object, a flag stored in the object itself, etc.)
		const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
		static const char* item_current = items[0];            // Here our selection is a single pointer stored outside the object.
		if (ImGui::BeginCombo("combo 1", item_current, flags)) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				bool is_selected = (item_current == items[n]);
				if (ImGui::Selectable(items[n], is_selected))
					item_current = items[n];
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
			}
			ImGui::EndCombo();
		}

		// Simplified one-liner Combo() API, using values packed in a single constant string
		static int item_current_2 = 0;
		ImGui::Combo("combo 2 (one-liner)", &item_current_2, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");

		// Simplified one-liner Combo() using an array of const char*
		static int item_current_3 = -1; // If the selection isn't within 0..count, Combo won't display a preview
		ImGui::Combo("combo 3 (array)", &item_current_3, items, IM_ARRAYSIZE(items));

		// Simplified one-liner Combo() using an accessor function
		struct FuncHolder { static bool ItemGetter(void* data, int idx, const char** out_str) { *out_str = ((const char**)data)[idx]; return true; } };
		static int item_current_4 = 0;
		ImGui::Combo("combo 4 (function)", &item_current_4, &FuncHolder::ItemGetter, items, IM_ARRAYSIZE(items));

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Selectables"))
	{
		// Selectable() has 2 overloads:
		// - The one taking "bool selected" as a read-only selection information. When Selectable() has been clicked is returns true and you can alter selection state accordingly.
		// - The one taking "bool* p_selected" as a read-write selection information (convenient in some cases)
		// The earlier is more flexible, as in real application your selection may be stored in a different manner (in flags within objects, as an external list, etc).
		if (ImGui::TreeNode("Basic"))
		{
			static bool selection[5] = { false, true, false, false, false };
			ImGui::Selectable("1. I am selectable", &selection[0]);
			ImGui::Selectable("2. I am selectable", &selection[1]);
			ImGui::Text("3. I am not selectable");
			ImGui::Selectable("4. I am selectable", &selection[3]);
			if (ImGui::Selectable("5. I am double clickable", selection[4], ImGuiSelectableFlags_AllowDoubleClick))
				if (ImGui::IsMouseDoubleClicked(0))
					selection[4] = !selection[4];
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Selection State: Single Selection"))
		{
			static int selected = -1;
			for (int n = 0; n < 5; n++)
			{
				char buf[32];
				sprintf(buf, "Object %d", n);
				if (ImGui::Selectable(buf, selected == n))
					selected = n;
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Selection State: Multiple Selection"))
		{
			ShowHelpMarker("Hold CTRL and click to select multiple items.");
			static bool selection[5] = { false, false, false, false, false };
			for (int n = 0; n < 5; n++)
			{
				char buf[32];
				sprintf(buf, "Object %d", n);
				if (ImGui::Selectable(buf, selection[n]))
				{
					if (!ImGui::GetIO().KeyCtrl)    // Clear selection when CTRL is not held
						memset(selection, 0, sizeof(selection));
					selection[n] ^= 1;
				}
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Rendering more text into the same line"))
		{
			// Using the Selectable() override that takes "bool* p_selected" parameter and toggle your booleans automatically.
			static bool selected[3] = { false, false, false };
			ImGui::Selectable("main.c", &selected[0]); ImGui::SameLine(300); ImGui::Text(" 2,345 bytes");
			ImGui::Selectable("Hello.cpp", &selected[1]); ImGui::SameLine(300); ImGui::Text("12,345 bytes");
			ImGui::Selectable("Hello.h", &selected[2]); ImGui::SameLine(300); ImGui::Text(" 2,345 bytes");
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("In columns"))
		{
			ImGui::Columns(3, NULL, false);
			static bool selected[16] = { 0 };
			for (int i = 0; i < 16; i++)
			{
				char label[32]; sprintf(label, "Item %d", i);
				if (ImGui::Selectable(label, &selected[i])) {}
				ImGui::NextColumn();
			}
			ImGui::Columns(1);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Grid"))
		{
			static bool selected[4 * 4] = { true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true };
			for (int i = 0; i < 4 * 4; i++)
			{
				ImGui::PushID(i);
				if (ImGui::Selectable("Sailor", &selected[i], 0, ImVec2(50, 50)))
				{
					// Note: We _unnecessarily_ test for both x/y and i here only to silence some static analyzer. The second part of each test is unnecessary.
					int x = i % 4;
					int y = i / 4;
					if (x > 0) { selected[i - 1] ^= 1; }
					if (x < 3 && i < 15) { selected[i + 1] ^= 1; }
					if (y > 0 && i > 3) { selected[i - 4] ^= 1; }
					if (y < 3 && i < 12) { selected[i + 4] ^= 1; }
				}
				if ((i % 4) < 3) ImGui::SameLine();
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Alignment"))
		{
			ShowHelpMarker("Alignment applies when a selectable is larger than its text content.\nBy default, Selectables uses style.SelectableTextAlign but it can be overriden on a per-item basis using PushStyleVar().");
			static bool selected[3 * 3] = { true, false, true, false, true, false, true, false, true };
			for (int y = 0; y < 3; y++)
			{
				for (int x = 0; x < 3; x++)
				{
					ImVec2 alignment = ImVec2((float)x / 2.0f, (float)y / 2.0f);
					char name[32];
					sprintf(name, "(%.1f,%.1f)", alignment.x, alignment.y);
					if (x > 0) ImGui::SameLine();
					ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, alignment);
					ImGui::Selectable(name, &selected[3 * y + x], ImGuiSelectableFlags_None, ImVec2(80, 80));
					ImGui::PopStyleVar();
				}
			}
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Filtered Text Input"))
	{
		static char buf1[64] = ""; ImGui::InputText("default", buf1, 64);
		static char buf2[64] = ""; ImGui::InputText("decimal", buf2, 64, ImGuiInputTextFlags_CharsDecimal);
		static char buf3[64] = ""; ImGui::InputText("hexadecimal", buf3, 64, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
		static char buf4[64] = ""; ImGui::InputText("uppercase", buf4, 64, ImGuiInputTextFlags_CharsUppercase);
		static char buf5[64] = ""; ImGui::InputText("no blank", buf5, 64, ImGuiInputTextFlags_CharsNoBlank);
		struct TextFilters { static int FilterImGuiLetters(ImGuiInputTextCallbackData* data) { if (data->EventChar < 256 && strchr("imgui", (char)data->EventChar)) return 0; return 1; } };
		static char buf6[64] = ""; ImGui::InputText("\"imgui\" letters", buf6, 64, ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterImGuiLetters);

		ImGui::Text("Password input");
		static char bufpass[64] = "password123";
		ImGui::InputText("password", bufpass, 64, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_CharsNoBlank);
		ImGui::SameLine(); ShowHelpMarker("Display all characters as '*'.\nDisable clipboard cut and copy.\nDisable logging.\n");
		ImGui::InputText("password (clear)", bufpass, 64, ImGuiInputTextFlags_CharsNoBlank);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Multi-line Text Input"))
	{
		// Note: we are using a fixed-sized buffer for simplicity here. See ImGuiInputTextFlags_CallbackResize
		// and the code in misc/cpp/imgui_stdlib.h for how to setup InputText() for dynamically resizing strings.
		static bool read_only = false;
		static char text[1024 * 16] =
			"/*\n"
			" The Pentium F00F bug, shorthand for F0 0F C7 C8,\n"
			" the hexadecimal encoding of one offending instruction,\n"
			" more formally, the invalid operand with locked CMPXCHG8B\n"
			" instruction bug, is a design flaw in the majority of\n"
			" Intel Pentium, Pentium MMX, and Pentium OverDrive\n"
			" processors (all in the P5 microarchitecture).\n"
			"*/\n\n"
			"label:\n"
			"\tlock cmpxchg8b eax\n";

		ShowHelpMarker("You can use the ImGuiInputTextFlags_CallbackResize facility if you need to wire InputTextMultiline() to a dynamic string type. See misc/cpp/imgui_stdlib.h for an example. (This is not demonstrated in imgui_demo.cpp)");
		ImGui::Checkbox("Read-only", &read_only);
		ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput | (read_only ? ImGuiInputTextFlags_ReadOnly : 0);
		ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(-1.0f, ImGui::GetTextLineHeight() * 16), flags);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Plots Widgets"))
	{
		static bool animate = true;
		ImGui::Checkbox("Animate", &animate);

		static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
		ImGui::PlotLines("Frame Times", arr, IM_ARRAYSIZE(arr));

		// Create a dummy array of contiguous float values to plot
		// Tip: If your float aren't contiguous but part of a structure, you can pass a pointer to your first float and the sizeof() of your structure in the Stride parameter.
		static float values[90] = { 0 };
		static int values_offset = 0;
		static double refresh_time = 0.0;
		if (!animate || refresh_time == 0.0)
			refresh_time = ImGui::GetTime();
		while (refresh_time < ImGui::GetTime()) // Create dummy data at fixed 60 hz rate for the demo
		{
			static float phase = 0.0f;
			values[values_offset] = cosf(phase);
			values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
			phase += 0.10f*values_offset;
			refresh_time += 1.0f / 60.0f;
		}
		ImGui::PlotLines("Lines", values, IM_ARRAYSIZE(values), values_offset, "avg 0.0", -1.0f, 1.0f, ImVec2(0, 80));
		ImGui::PlotHistogram("Histogram", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 1.0f, ImVec2(0, 80));

		// Use functions to generate output
		// FIXME: This is rather awkward because current plot API only pass in indices. We probably want an API passing floats and user provide sample rate/count.
		struct Funcs
		{
			static float Sin(void*, int i) { return sinf(i * 0.1f); }
			static float Saw(void*, int i) { return (i & 1) ? 1.0f : -1.0f; }
		};
		static int func_type = 0, display_count = 70;
		ImGui::Separator();
		ImGui::PushItemWidth(100); ImGui::Combo("func", &func_type, "Sin\0Saw\0"); ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::SliderInt("Sample count", &display_count, 1, 400);
		float(*func)(void*, int) = (func_type == 0) ? Funcs::Sin : Funcs::Saw;
		ImGui::PlotLines("Lines", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, ImVec2(0, 80));
		ImGui::PlotHistogram("Histogram", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, ImVec2(0, 80));
		ImGui::Separator();

		// Animate a simple progress bar
		static float progress = 0.0f, progress_dir = 1.0f;
		if (animate)
		{
			progress += progress_dir * 0.4f * ImGui::GetIO().DeltaTime;
			if (progress >= +1.1f) { progress = +1.1f; progress_dir *= -1.0f; }
			if (progress <= -0.1f) { progress = -0.1f; progress_dir *= -1.0f; }
		}

		// Typically we would use ImVec2(-1.0f,0.0f) to use all available width, or ImVec2(width,0.0f) for a specified width. ImVec2(0.0f,0.0f) uses ItemWidth.
		ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::Text("Progress Bar");

		float progress_saturated = (progress < 0.0f) ? 0.0f : (progress > 1.0f) ? 1.0f : progress;
		char buf[32];
		sprintf(buf, "%d/%d", (int)(progress_saturated * 1753), 1753);
		ImGui::ProgressBar(progress, ImVec2(0.f, 0.f), buf);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Color/Picker Widgets"))
	{
		static ImVec4 color = ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);

		static bool alpha_preview = true;
		static bool alpha_half_preview = false;
		static bool drag_and_drop = true;
		static bool options_menu = true;
		static bool hdr = false;
		ImGui::Checkbox("With Alpha Preview", &alpha_preview);
		ImGui::Checkbox("With Half Alpha Preview", &alpha_half_preview);
		ImGui::Checkbox("With Drag and Drop", &drag_and_drop);
		ImGui::Checkbox("With Options Menu", &options_menu); ImGui::SameLine(); ShowHelpMarker("Right-click on the individual color widget to show options.");
		ImGui::Checkbox("With HDR", &hdr); ImGui::SameLine(); ShowHelpMarker("Currently all this does is to lift the 0..1 limits on dragging widgets.");
		int misc_flags = (hdr ? ImGuiColorEditFlags_HDR : 0) | (drag_and_drop ? 0 : ImGuiColorEditFlags_NoDragDrop) | (alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) | (options_menu ? 0 : ImGuiColorEditFlags_NoOptions);

		ImGui::Text("Color widget:");
		ImGui::SameLine(); ShowHelpMarker("Click on the colored square to open a color picker.\nCTRL+click on individual component to input value.\n");
		//ImGui::ColorEdit3("MyColor##1", (float*)&color, misc_flags);

		ImGui::Text("Color widget HSV with Alpha:");
		//ImGui::ColorEdit4("MyColor##2", (float*)&color, ImGuiColorEditFlags_HSV | misc_flags);

		ImGui::Text("Color widget with Float Display:");
		//ImGui::ColorEdit4("MyColor##2f", (float*)&color, ImGuiColorEditFlags_Float | misc_flags);

		ImGui::Text("Color button with Picker:");
		ImGui::SameLine(); ShowHelpMarker("With the ImGuiColorEditFlags_NoInputs flag you can hide all the slider/text inputs.\nWith the ImGuiColorEditFlags_NoLabel flag you can pass a non-empty label which will only be used for the tooltip and picker popup.");
		//ImGui::ColorEdit4("MyColor##3", (float*)&color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);

		ImGui::Text("Color button with Custom Picker Popup:");

		// Generate a dummy default palette. The palette will persist and can be edited.
		static bool saved_palette_init = true;
		static ImVec4 saved_palette[32] = { };
		if (saved_palette_init)
		{
			for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
			{
				ImGui::ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.8f, saved_palette[n].x, saved_palette[n].y, saved_palette[n].z);
				saved_palette[n].w = 1.0f; // Alpha
			}
			saved_palette_init = false;
		}

		static ImVec4 backup_color;
		bool open_popup = ImGui::ColorButton("MyColor##3b", color, misc_flags);
		ImGui::SameLine();
		open_popup |= ImGui::Button("Palette");
		if (open_popup)
		{
			ImGui::OpenPopup("mypicker");
			backup_color = color;
		}
		if (ImGui::BeginPopup("mypicker"))
		{
			ImGui::Text("MY CUSTOM COLOR PICKER WITH AN AMAZING PALETTE!");
			ImGui::Separator();
			//ImGui::ColorPicker4("##picker", (float*)&color, misc_flags | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
			ImGui::SameLine();

			ImGui::BeginGroup(); // Lock X position
			ImGui::Text("Current");
			ImGui::ColorButton("##current", color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40));
			ImGui::Text("Previous");
			if (ImGui::ColorButton("##previous", backup_color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40)))
				color = backup_color;
			ImGui::Separator();
			ImGui::Text("Palette");
			for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
			{
				ImGui::PushID(n);
				if ((n % 8) != 0)
					ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);
				if (ImGui::ColorButton("##palette", saved_palette[n], ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20)))
					color = ImVec4(saved_palette[n].x, saved_palette[n].y, saved_palette[n].z, color.w); // Preserve alpha!

				// Allow user to drop colors into each palette entry
				// (Note that ColorButton is already a drag source by default, unless using ImGuiColorEditFlags_NoDragDrop)
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
						memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 3);
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
						memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 4);
					ImGui::EndDragDropTarget();
				}

				ImGui::PopID();
			}
			ImGui::EndGroup();
			ImGui::EndPopup();
		}

		ImGui::Text("Color button only:");
		ImGui::ColorButton("MyColor##3c", *(ImVec4*)&color, misc_flags, ImVec2(80, 80));

		ImGui::Text("Color picker:");
		static bool alpha = true;
		static bool alpha_bar = true;
		static bool side_preview = true;
		static bool ref_color = false;
		static ImVec4 ref_color_v(1.0f, 0.0f, 1.0f, 0.5f);
		static int inputs_mode = 2;
		static int picker_mode = 0;
		ImGui::Checkbox("With Alpha", &alpha);
		ImGui::Checkbox("With Alpha Bar", &alpha_bar);
		ImGui::Checkbox("With Side Preview", &side_preview);
		if (side_preview)
		{
			ImGui::SameLine();
			ImGui::Checkbox("With Ref Color", &ref_color);
			if (ref_color)
			{
				ImGui::SameLine();
				//ImGui::ColorEdit4("##RefColor", &ref_color_v.x, ImGuiColorEditFlags_NoInputs | misc_flags);
			}
		}
		ImGui::Combo("Inputs Mode", &inputs_mode, "All Inputs\0No Inputs\0RGB Input\0HSV Input\0HEX Input\0");
		ImGui::Combo("Picker Mode", &picker_mode, "Auto/Current\0Hue bar + SV rect\0Hue wheel + SV triangle\0");
		ImGui::SameLine(); ShowHelpMarker("User can right-click the picker to change mode.");
		ImGuiColorEditFlags flags = misc_flags;
		if (!alpha) flags |= ImGuiColorEditFlags_NoAlpha; // This is by default if you call ColorPicker3() instead of ColorPicker4()
		if (alpha_bar) flags |= ImGuiColorEditFlags_AlphaBar;
		if (!side_preview) flags |= ImGuiColorEditFlags_NoSidePreview;
		if (picker_mode == 1) flags |= ImGuiColorEditFlags_PickerHueBar;
		if (picker_mode == 2) flags |= ImGuiColorEditFlags_PickerHueWheel;
		if (inputs_mode == 1) flags |= ImGuiColorEditFlags_NoInputs;
		if (inputs_mode == 2) flags |= ImGuiColorEditFlags_RGB;
		if (inputs_mode == 3) flags |= ImGuiColorEditFlags_HSV;
		if (inputs_mode == 4) flags |= ImGuiColorEditFlags_HEX;
		//ImGui::ColorPicker4("MyColor##4", (float*)&color, flags, ref_color ? &ref_color_v.x : NULL);

		ImGui::Text("Programmatically set defaults:");
		ImGui::SameLine(); ShowHelpMarker("SetColorEditOptions() is designed to allow you to set boot-time default.\nWe don't have Push/Pop functions because you can force options on a per-widget basis if needed, and the user can change non-forced ones with the options menu.\nWe don't have a getter to avoid encouraging you to persistently save values that aren't forward-compatible.");
		if (ImGui::Button("Default: Uint8 + HSV + Hue Bar"))
			ImGui::SetColorEditOptions(ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_HSV | ImGuiColorEditFlags_PickerHueBar);
		if (ImGui::Button("Default: Float + HDR + Hue Wheel"))
			ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_PickerHueWheel);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Range Widgets"))
	{
		static float begin = 10, end = 90;
		static int begin_i = 100, end_i = 1000;
		ImGui::DragFloatRange2("range", &begin, &end, 0.25f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%");
		ImGui::DragIntRange2("range int (no bounds)", &begin_i, &end_i, 5, 0, 0, "Min: %d units", "Max: %d units");
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Data Types"))
	{
		// The DragScalar/InputScalar/SliderScalar functions allow various data types: signed/unsigned int/long long and float/double
		// To avoid polluting the public API with all possible combinations, we use the ImGuiDataType enum to pass the type,
		// and passing all arguments by address.
		// This is the reason the test code below creates local variables to hold "zero" "one" etc. for each types.
		// In practice, if you frequently use a given type that is not covered by the normal API entry points, you can wrap it
		// yourself inside a 1 line function which can take typed argument as value instead of void*, and then pass their address
		// to the generic function. For example:
		//   bool MySliderU64(const char *label, u64* value, u64 min = 0, u64 max = 0, const char* format = "%lld")
		//   {
		//      return SliderScalar(label, ImGuiDataType_U64, value, &min, &max, format);
		//   }

		// Limits (as helper variables that we can take the address of)
		// Note that the SliderScalar function has a maximum usable range of half the natural type maximum, hence the /2 below.
#ifndef LLONG_MIN
		ImS64 LLONG_MIN = -9223372036854775807LL - 1;
		ImS64 LLONG_MAX = 9223372036854775807LL;
		ImU64 ULLONG_MAX = (2ULL * 9223372036854775807LL + 1);
#endif
		const ImS32   s32_zero = 0, s32_one = 1, s32_fifty = 50, s32_min = INT_MIN / 2, s32_max = INT_MAX / 2, s32_hi_a = INT_MAX / 2 - 100, s32_hi_b = INT_MAX / 2;
		const ImU32   u32_zero = 0, u32_one = 1, u32_fifty = 50, u32_min = 0, u32_max = UINT_MAX / 2, u32_hi_a = UINT_MAX / 2 - 100, u32_hi_b = UINT_MAX / 2;
		const ImS64   s64_zero = 0, s64_one = 1, s64_fifty = 50, s64_min = LLONG_MIN / 2, s64_max = LLONG_MAX / 2, s64_hi_a = LLONG_MAX / 2 - 100, s64_hi_b = LLONG_MAX / 2;
		const ImU64   u64_zero = 0, u64_one = 1, u64_fifty = 50, u64_min = 0, u64_max = ULLONG_MAX / 2, u64_hi_a = ULLONG_MAX / 2 - 100, u64_hi_b = ULLONG_MAX / 2;
		const float   f32_zero = 0.f, f32_one = 1.f, f32_lo_a = -10000000000.0f, f32_hi_a = +10000000000.0f;
		const double  f64_zero = 0., f64_one = 1., f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;

		// State
		static ImS32  s32_v = -1;
		static ImU32  u32_v = (ImU32)-1;
		static ImS64  s64_v = -1;
		static ImU64  u64_v = (ImU64)-1;
		static float  f32_v = 0.123f;
		static double f64_v = 90000.01234567890123456789;

		const float drag_speed = 0.2f;
		static bool drag_clamp = false;
		ImGui::Text("Drags:");
		ImGui::Checkbox("Clamp integers to 0..50", &drag_clamp); ImGui::SameLine(); ShowHelpMarker("As with every widgets in dear imgui, we never modify values unless there is a user interaction.\nYou can override the clamping limits by using CTRL+Click to input a value.");
		ImGui::DragScalar("drag s32", ImGuiDataType_S32, &s32_v, drag_speed, drag_clamp ? &s32_zero : NULL, drag_clamp ? &s32_fifty : NULL);
		ImGui::DragScalar("drag u32", ImGuiDataType_U32, &u32_v, drag_speed, drag_clamp ? &u32_zero : NULL, drag_clamp ? &u32_fifty : NULL, "%u ms");
		ImGui::DragScalar("drag s64", ImGuiDataType_S64, &s64_v, drag_speed, drag_clamp ? &s64_zero : NULL, drag_clamp ? &s64_fifty : NULL);
		ImGui::DragScalar("drag u64", ImGuiDataType_U64, &u64_v, drag_speed, drag_clamp ? &u64_zero : NULL, drag_clamp ? &u64_fifty : NULL);
		ImGui::DragScalar("drag float", ImGuiDataType_Float, &f32_v, 0.005f, &f32_zero, &f32_one, "%f", 1.0f);
		ImGui::DragScalar("drag float ^2", ImGuiDataType_Float, &f32_v, 0.005f, &f32_zero, &f32_one, "%f", 2.0f); ImGui::SameLine(); ShowHelpMarker("You can use the 'power' parameter to increase tweaking precision on one side of the range.");
		ImGui::DragScalar("drag double", ImGuiDataType_Double, &f64_v, 0.0005f, &f64_zero, NULL, "%.10f grams", 1.0f);
		ImGui::DragScalar("drag double ^2", ImGuiDataType_Double, &f64_v, 0.0005f, &f64_zero, &f64_one, "0 < %.10f < 1", 2.0f);

		ImGui::Text("Sliders");
		ImGui::SliderScalar("slider s32 low", ImGuiDataType_S32, &s32_v, &s32_zero, &s32_fifty, "%d");
		ImGui::SliderScalar("slider s32 high", ImGuiDataType_S32, &s32_v, &s32_hi_a, &s32_hi_b, "%d");
		ImGui::SliderScalar("slider s32 full", ImGuiDataType_S32, &s32_v, &s32_min, &s32_max, "%d");
		ImGui::SliderScalar("slider u32 low", ImGuiDataType_U32, &u32_v, &u32_zero, &u32_fifty, "%u");
		ImGui::SliderScalar("slider u32 high", ImGuiDataType_U32, &u32_v, &u32_hi_a, &u32_hi_b, "%u");
		ImGui::SliderScalar("slider u32 full", ImGuiDataType_U32, &u32_v, &u32_min, &u32_max, "%u");
		ImGui::SliderScalar("slider s64 low", ImGuiDataType_S64, &s64_v, &s64_zero, &s64_fifty, "%I64d");
		ImGui::SliderScalar("slider s64 high", ImGuiDataType_S64, &s64_v, &s64_hi_a, &s64_hi_b, "%I64d");
		ImGui::SliderScalar("slider s64 full", ImGuiDataType_S64, &s64_v, &s64_min, &s64_max, "%I64d");
		ImGui::SliderScalar("slider u64 low", ImGuiDataType_U64, &u64_v, &u64_zero, &u64_fifty, "%I64u ms");
		ImGui::SliderScalar("slider u64 high", ImGuiDataType_U64, &u64_v, &u64_hi_a, &u64_hi_b, "%I64u ms");
		ImGui::SliderScalar("slider u64 full", ImGuiDataType_U64, &u64_v, &u64_min, &u64_max, "%I64u ms");
		ImGui::SliderScalar("slider float low", ImGuiDataType_Float, &f32_v, &f32_zero, &f32_one);
		ImGui::SliderScalar("slider float low^2", ImGuiDataType_Float, &f32_v, &f32_zero, &f32_one, "%.10f", 2.0f);
		ImGui::SliderScalar("slider float high", ImGuiDataType_Float, &f32_v, &f32_lo_a, &f32_hi_a, "%e");
		ImGui::SliderScalar("slider double low", ImGuiDataType_Double, &f64_v, &f64_zero, &f64_one, "%.10f grams", 1.0f);
		ImGui::SliderScalar("slider double low^2", ImGuiDataType_Double, &f64_v, &f64_zero, &f64_one, "%.10f", 2.0f);
		ImGui::SliderScalar("slider double high", ImGuiDataType_Double, &f64_v, &f64_lo_a, &f64_hi_a, "%e grams", 1.0f);

		static bool inputs_step = true;
		ImGui::Text("Inputs");
		ImGui::Checkbox("Show step buttons", &inputs_step);
		ImGui::InputScalar("input s32", ImGuiDataType_S32, &s32_v, inputs_step ? &s32_one : NULL, NULL, "%d");
		ImGui::InputScalar("input s32 hex", ImGuiDataType_S32, &s32_v, inputs_step ? &s32_one : NULL, NULL, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
		ImGui::InputScalar("input u32", ImGuiDataType_U32, &u32_v, inputs_step ? &u32_one : NULL, NULL, "%u");
		ImGui::InputScalar("input u32 hex", ImGuiDataType_U32, &u32_v, inputs_step ? &u32_one : NULL, NULL, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
		ImGui::InputScalar("input s64", ImGuiDataType_S64, &s64_v, inputs_step ? &s64_one : NULL);
		ImGui::InputScalar("input u64", ImGuiDataType_U64, &u64_v, inputs_step ? &u64_one : NULL);
		ImGui::InputScalar("input float", ImGuiDataType_Float, &f32_v, inputs_step ? &f32_one : NULL);
		ImGui::InputScalar("input double", ImGuiDataType_Double, &f64_v, inputs_step ? &f64_one : NULL);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Multi-component Widgets"))
	{
		static float vec4f[4] = { 0.10f, 0.20f, 0.30f, 0.44f };
		static int vec4i[4] = { 1, 5, 100, 255 };

		ImGui::InputFloat2("input float2", vec4f);
		ImGui::DragFloat2("drag float2", vec4f, 0.01f, 0.0f, 1.0f);
		ImGui::SliderFloat2("slider float2", vec4f, 0.0f, 1.0f);
		ImGui::InputInt2("input int2", vec4i);
		ImGui::DragInt2("drag int2", vec4i, 1, 0, 255);
		ImGui::SliderInt2("slider int2", vec4i, 0, 255);
		ImGui::Spacing();

		ImGui::InputFloat3("input float3", vec4f);
		ImGui::DragFloat3("drag float3", vec4f, 0.01f, 0.0f, 1.0f);
		ImGui::SliderFloat3("slider float3", vec4f, 0.0f, 1.0f);
		ImGui::InputInt3("input int3", vec4i);
		ImGui::DragInt3("drag int3", vec4i, 1, 0, 255);
		ImGui::SliderInt3("slider int3", vec4i, 0, 255);
		ImGui::Spacing();

		ImGui::InputFloat4("input float4", vec4f);
		ImGui::DragFloat4("drag float4", vec4f, 0.01f, 0.0f, 1.0f);
		ImGui::SliderFloat4("slider float4", vec4f, 0.0f, 1.0f);
		ImGui::InputInt4("input int4", vec4i);
		ImGui::DragInt4("drag int4", vec4i, 1, 0, 255);
		ImGui::SliderInt4("slider int4", vec4i, 0, 255);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Vertical Sliders"))
	{
		const float spacing = 4;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing, spacing));

		static int int_value = 0;
		ImGui::VSliderInt("##int", ImVec2(18, 160), &int_value, 0, 5);
		ImGui::SameLine();

		static float values[7] = { 0.0f, 0.60f, 0.35f, 0.9f, 0.70f, 0.20f, 0.0f };
		ImGui::PushID("set1");
		for (int i = 0; i < 7; i++)
		{
			if (i > 0) ImGui::SameLine();
			ImGui::PushID(i);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(i / 7.0f, 0.5f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(i / 7.0f, 0.7f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_SliderGrab, (ImVec4)ImColor::HSV(i / 7.0f, 0.9f, 0.9f));
			ImGui::VSliderFloat("##v", ImVec2(18, 160), &values[i], 0.0f, 1.0f, "");
			if (ImGui::IsItemActive() || ImGui::IsItemHovered())
				ImGui::SetTooltip("%.3f", values[i]);
			ImGui::PopStyleColor(4);
			ImGui::PopID();
		}
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::PushID("set2");
		static float values2[4] = { 0.20f, 0.80f, 0.40f, 0.25f };
		const int rows = 3;
		const ImVec2 small_slider_size(18, (160.0f - (rows - 1)*spacing) / rows);
		for (int nx = 0; nx < 4; nx++)
		{
			if (nx > 0) ImGui::SameLine();
			ImGui::BeginGroup();
			for (int ny = 0; ny < rows; ny++)
			{
				ImGui::PushID(nx*rows + ny);
				ImGui::VSliderFloat("##v", small_slider_size, &values2[nx], 0.0f, 1.0f, "");
				if (ImGui::IsItemActive() || ImGui::IsItemHovered())
					ImGui::SetTooltip("%.3f", values2[nx]);
				ImGui::PopID();
			}
			ImGui::EndGroup();
		}
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::PushID("set3");
		for (int i = 0; i < 4; i++)
		{
			if (i > 0) ImGui::SameLine();
			ImGui::PushID(i);
			ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40);
			ImGui::VSliderFloat("##v", ImVec2(40, 160), &values[i], 0.0f, 1.0f, "%.2f\nsec");
			ImGui::PopStyleVar();
			ImGui::PopID();
		}
		ImGui::PopID();
		ImGui::PopStyleVar();
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Drag and Drop"))
	{
		{
			// ColorEdit widgets automatically act as drag source and drag target.
			// They are using standardized payload strings IMGUI_PAYLOAD_TYPE_COLOR_3F and IMGUI_PAYLOAD_TYPE_COLOR_4F to allow your own widgets
			// to use colors in their drag and drop interaction. Also see the demo in Color Picker -> Palette demo.
			ImGui::BulletText("Drag and drop in standard widgets");
			ImGui::Indent();
			static float col1[3] = { 1.0f,0.0f,0.2f };
			static float col2[4] = { 0.4f,0.7f,0.0f,0.5f };
			//ImGui::ColorEdit3("color 1", col1);
			//ImGui::ColorEdit4("color 2", col2);
			ImGui::Unindent();
		}

		{
			ImGui::BulletText("Drag and drop to copy/swap items");
			ImGui::Indent();
			enum Mode
			{
				Mode_Copy,
				Mode_Move,
				Mode_Swap
			};
			static int mode = 0;
			if (ImGui::RadioButton("Copy", mode == Mode_Copy)) { mode = Mode_Copy; } ImGui::SameLine();
			if (ImGui::RadioButton("Move", mode == Mode_Move)) { mode = Mode_Move; } ImGui::SameLine();
			if (ImGui::RadioButton("Swap", mode == Mode_Swap)) { mode = Mode_Swap; }
			static const char* names[9] = { "Bobby", "Beatrice", "Betty", "Brianna", "Barry", "Bernard", "Bibi", "Blaine", "Bryn" };
			for (int n = 0; n < IM_ARRAYSIZE(names); n++)
			{
				ImGui::PushID(n);
				if ((n % 3) != 0)
					ImGui::SameLine();
				ImGui::Button(names[n], ImVec2(60, 60));

				// Our buttons are both drag sources and drag targets here!
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
				{
					ImGui::SetDragDropPayload("DND_DEMO_CELL", &n, sizeof(int));        // Set payload to carry the index of our item (could be anything)
					if (mode == Mode_Copy) { ImGui::Text("Copy %s", names[n]); }        // Display preview (could be anything, e.g. when dragging an image we could decide to display the filename and a small preview of the image, etc.)
					if (mode == Mode_Move) { ImGui::Text("Move %s", names[n]); }
					if (mode == Mode_Swap) { ImGui::Text("Swap %s", names[n]); }
					ImGui::EndDragDropSource();
				}
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
					{
						IM_ASSERT(payload->DataSize == sizeof(int));
						int payload_n = *(const int*)payload->Data;
						if (mode == Mode_Copy)
						{
							names[n] = names[payload_n];
						}
						if (mode == Mode_Move)
						{
							names[n] = names[payload_n];
							names[payload_n] = "";
						}
						if (mode == Mode_Swap)
						{
							const char* tmp = names[n];
							names[n] = names[payload_n];
							names[payload_n] = tmp;
						}
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::PopID();
			}
			ImGui::Unindent();
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Querying Status (Active/Focused/Hovered etc.)"))
	{
		// Display the value of IsItemHovered() and other common item state functions. Note that the flags can be combined.
		// (because BulletText is an item itself and that would affect the output of IsItemHovered() we pass all state in a single call to simplify the code).
		static int item_type = 1;
		static bool b = false;
		static float col4f[4] = { 1.0f, 0.5, 0.0f, 1.0f };
		ImGui::RadioButton("Text", &item_type, 0);
		ImGui::RadioButton("Button", &item_type, 1);
		ImGui::RadioButton("Checkbox", &item_type, 2);
		ImGui::RadioButton("SliderFloat", &item_type, 3);
		ImGui::RadioButton("ColorEdit4", &item_type, 4);
		ImGui::RadioButton("ListBox", &item_type, 5);
		ImGui::Separator();
		bool ret = false;
		if (item_type == 0) { ImGui::Text("ITEM: Text"); }                                              // Testing text items with no identifier/interaction
		if (item_type == 1) { ret = ImGui::Button("ITEM: Button"); }                                    // Testing button
		if (item_type == 2) { ret = ImGui::Checkbox("ITEM: Checkbox", &b); }                            // Testing checkbox
		if (item_type == 3) { ret = ImGui::SliderFloat("ITEM: SliderFloat", &col4f[0], 0.0f, 1.0f); }   // Testing basic item
		if (item_type == 4) { /*ret = ImGui::ColorEdit4("ITEM: ColorEdit4", col4f);*/ }                     // Testing multi-component items (IsItemXXX flags are reported merged)
		if (item_type == 5) { const char* items[] = { "Apple", "Banana", "Cherry", "Kiwi" }; static int current = 1; ret = ImGui::ListBox("ITEM: ListBox", &current, items, IM_ARRAYSIZE(items), IM_ARRAYSIZE(items)); }
		ImGui::BulletText(
			"Return value = %d\n"
			"IsItemFocused() = %d\n"
			"IsItemHovered() = %d\n"
			"IsItemHovered(_AllowWhenBlockedByPopup) = %d\n"
			"IsItemHovered(_AllowWhenBlockedByActiveItem) = %d\n"
			"IsItemHovered(_AllowWhenOverlapped) = %d\n"
			"IsItemHovered(_RectOnly) = %d\n"
			"IsItemActive() = %d\n"
			"IsItemEdited() = %d\n"
			"IsItemActivated() = %d\n"
			"IsItemDeactivated() = %d\n"
			"IsItemDeactivatedEdit() = %d\n"
			"IsItemVisible() = %d\n"
			"GetItemRectMin() = (%.1f, %.1f)\n"
			"GetItemRectMax() = (%.1f, %.1f)\n"
			"GetItemRectSize() = (%.1f, %.1f)",
			ret,
			ImGui::IsItemFocused(),
			ImGui::IsItemHovered(),
			ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup),
			ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem),
			ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped),
			ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly),
			ImGui::IsItemActive(),
			ImGui::IsItemEdited(),
			ImGui::IsItemActivated(),
			ImGui::IsItemDeactivated(),
			ImGui::IsItemDeactivatedAfterEdit(),
			ImGui::IsItemVisible(),
			ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y,
			ImGui::GetItemRectMax().x, ImGui::GetItemRectMax().y,
			ImGui::GetItemRectSize().x, ImGui::GetItemRectSize().y
		);

		static bool embed_all_inside_a_child_window = false;
		ImGui::Checkbox("Embed everything inside a child window (for additional testing)", &embed_all_inside_a_child_window);
		if (embed_all_inside_a_child_window)
			ImGui::BeginChild("outer_child", ImVec2(0, ImGui::GetFontSize() * 20), true);

		// Testing IsWindowFocused() function with its various flags. Note that the flags can be combined.
		ImGui::BulletText(
			"IsWindowFocused() = %d\n"
			"IsWindowFocused(_ChildWindows) = %d\n"
			"IsWindowFocused(_ChildWindows|_RootWindow) = %d\n"
			"IsWindowFocused(_RootWindow) = %d\n"
			"IsWindowFocused(_AnyWindow) = %d\n",
			ImGui::IsWindowFocused(),
			ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows),
			ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_RootWindow),
			ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow),
			ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow));

		// Testing IsWindowHovered() function with its various flags. Note that the flags can be combined.
		ImGui::BulletText(
			"IsWindowHovered() = %d\n"
			"IsWindowHovered(_AllowWhenBlockedByPopup) = %d\n"
			"IsWindowHovered(_AllowWhenBlockedByActiveItem) = %d\n"
			"IsWindowHovered(_ChildWindows) = %d\n"
			"IsWindowHovered(_ChildWindows|_RootWindow) = %d\n"
			"IsWindowHovered(_RootWindow) = %d\n"
			"IsWindowHovered(_AnyWindow) = %d\n",
			ImGui::IsWindowHovered(),
			ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup),
			ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem),
			ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows),
			ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_RootWindow),
			ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow),
			ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow));

		ImGui::BeginChild("child", ImVec2(0, 50), true);
		ImGui::Text("This is another child window for testing the _ChildWindows flag.");
		ImGui::EndChild();
		if (embed_all_inside_a_child_window)
			ImGui::EndChild();

		// Calling IsItemHovered() after begin returns the hovered status of the title bar.
		// This is useful in particular if you want to create a context menu (with BeginPopupContextItem) associated to the title bar of a window.
		static bool test_window = false;
		ImGui::Checkbox("Hovered/Active tests after Begin() for title bar testing", &test_window);
		if (test_window)
		{
			ImGui::Begin("Title bar Hovered/Active tests", &test_window);
			if (ImGui::BeginPopupContextItem()) // <-- This is using IsItemHovered()
			{
				if (ImGui::MenuItem("Close")) { test_window = false; }
				ImGui::EndPopup();
			}
			ImGui::Text(
				"IsItemHovered() after begin = %d (== is title bar hovered)\n"
				"IsItemActive() after begin = %d (== is window being clicked/moved)\n",
				ImGui::IsItemHovered(), ImGui::IsItemActive());
			ImGui::End();
		}

		ImGui::TreePop();
	}
}

static void ShowDemoWindowLayout()
{
	if (!ImGui::CollapsingHeader("Layout"))
		return;

	if (ImGui::TreeNode("Child windows"))
	{
		ShowHelpMarker("Use child windows to begin into a self-contained independent scrolling/clipping regions within a host window.");
		static bool disable_mouse_wheel = false;
		static bool disable_menu = false;
		ImGui::Checkbox("Disable Mouse Wheel", &disable_mouse_wheel);
		ImGui::Checkbox("Disable Menu", &disable_menu);

		static int line = 50;
		bool goto_line = ImGui::Button("Goto");
		ImGui::SameLine();
		ImGui::PushItemWidth(100);
		goto_line |= ImGui::InputInt("##Line", &line, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::PopItemWidth();

		// Child 1: no border, enable horizontal scrollbar
		{
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar | (disable_mouse_wheel ? ImGuiWindowFlags_NoScrollWithMouse : 0);
			ImGui::BeginChild("Child1", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 260), false, window_flags);
			for (int i = 0; i < 100; i++)
			{
				ImGui::Text("%04d: scrollable region", i);
				if (goto_line && line == i)
					ImGui::SetScrollHereY();
			}
			if (goto_line && line >= 100)
				ImGui::SetScrollHereY();
			ImGui::EndChild();
		}

		ImGui::SameLine();

		// Child 2: rounded border
		{
			ImGuiWindowFlags window_flags = (disable_mouse_wheel ? ImGuiWindowFlags_NoScrollWithMouse : 0) | (disable_menu ? 0 : ImGuiWindowFlags_MenuBar);
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
			ImGui::BeginChild("Child2", ImVec2(0, 260), true, window_flags);
			if (!disable_menu && ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("Menu"))
				{
					ShowExampleMenuFile();
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
			ImGui::Columns(2);
			for (int i = 0; i < 100; i++)
			{
				char buf[32];
				sprintf(buf, "%03d", i);
				ImGui::Button(buf, ImVec2(-1.0f, 0.0f));
				ImGui::NextColumn();
			}
			ImGui::EndChild();
			ImGui::PopStyleVar();
		}

		ImGui::Separator();

		// Demonstrate a few extra things
		// - Changing ImGuiCol_ChildBg (which is transparent black in default styles)
		// - Using SetCursorPos() to position the child window (because the child window is an item from the POV of the parent window)
		//   You can also call SetNextWindowPos() to position the child window. The parent window will effectively layout from this position.
		// - Using ImGui::GetItemRectMin/Max() to query the "item" state (because the child window is an item from the POV of the parent window)
		//   See "Widgets" -> "Querying Status (Active/Focused/Hovered etc.)" section for more details about this.
		{
			ImGui::SetCursorPosX(50);
			ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(255, 0, 0, 100));
			ImGui::BeginChild("blah", ImVec2(200, 100), true, ImGuiWindowFlags_None);
			for (int n = 0; n < 50; n++)
				ImGui::Text("Some test %d", n);
			ImGui::EndChild();
			ImVec2 child_rect_min = ImGui::GetItemRectMin();
			ImVec2 child_rect_max = ImGui::GetItemRectMax();
			ImGui::PopStyleColor();
			ImGui::Text("Rect of child window is: (%.0f,%.0f) (%.0f,%.0f)", child_rect_min.x, child_rect_min.y, child_rect_max.x, child_rect_max.y);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Widgets Width"))
	{
		static float f = 0.0f;
		ImGui::Text("PushItemWidth(100)");
		ImGui::SameLine(); ShowHelpMarker("Fixed width.");
		ImGui::PushItemWidth(100);
		ImGui::DragFloat("float##1", &f);
		ImGui::PopItemWidth();

		ImGui::Text("PushItemWidth(GetWindowWidth() * 0.5f)");
		ImGui::SameLine(); ShowHelpMarker("Half of window width.");
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
		ImGui::DragFloat("float##2", &f);
		ImGui::PopItemWidth();

		ImGui::Text("PushItemWidth(GetContentRegionAvailWidth() * 0.5f)");
		ImGui::SameLine(); ShowHelpMarker("Half of available width.\n(~ right-cursor_pos)\n(works within a column set)");
		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() * 0.5f);
		ImGui::DragFloat("float##3", &f);
		ImGui::PopItemWidth();

		ImGui::Text("PushItemWidth(-100)");
		ImGui::SameLine(); ShowHelpMarker("Align to right edge minus 100");
		ImGui::PushItemWidth(-100);
		ImGui::DragFloat("float##4", &f);
		ImGui::PopItemWidth();

		ImGui::Text("PushItemWidth(-1)");
		ImGui::SameLine(); ShowHelpMarker("Align to right edge");
		ImGui::PushItemWidth(-1);
		ImGui::DragFloat("float##5", &f);
		ImGui::PopItemWidth();

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Basic Horizontal Layout"))
	{
		ImGui::TextWrapped("(Use ImGui::SameLine() to keep adding items to the right of the preceding item)");

		// Text
		ImGui::Text("Two items: Hello"); ImGui::SameLine();
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Sailor");

		// Adjust spacing
		ImGui::Text("More spacing: Hello"); ImGui::SameLine(0, 20);
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Sailor");

		// Button
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Normal buttons"); ImGui::SameLine();
		ImGui::Button("Banana"); ImGui::SameLine();
		ImGui::Button("Apple"); ImGui::SameLine();
		ImGui::Button("Corniflower");

		// Button
		ImGui::Text("Small buttons"); ImGui::SameLine();
		ImGui::SmallButton("Like this one"); ImGui::SameLine();
		ImGui::Text("can fit within a text block.");

		// Aligned to arbitrary position. Easy/cheap column.
		ImGui::Text("Aligned");
		ImGui::SameLine(150); ImGui::Text("x=150");
		ImGui::SameLine(300); ImGui::Text("x=300");
		ImGui::Text("Aligned");
		ImGui::SameLine(150); ImGui::SmallButton("x=150");
		ImGui::SameLine(300); ImGui::SmallButton("x=300");

		// Checkbox
		static bool c1 = false, c2 = false, c3 = false, c4 = false;
		ImGui::Checkbox("My", &c1); ImGui::SameLine();
		ImGui::Checkbox("Tailor", &c2); ImGui::SameLine();
		ImGui::Checkbox("Is", &c3); ImGui::SameLine();
		ImGui::Checkbox("Rich", &c4);

		// Various
		static float f0 = 1.0f, f1 = 2.0f, f2 = 3.0f;
		ImGui::PushItemWidth(80);
		const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD" };
		static int item = -1;
		ImGui::Combo("Combo", &item, items, IM_ARRAYSIZE(items)); ImGui::SameLine();
		ImGui::SliderFloat("X", &f0, 0.0f, 5.0f); ImGui::SameLine();
		ImGui::SliderFloat("Y", &f1, 0.0f, 5.0f); ImGui::SameLine();
		ImGui::SliderFloat("Z", &f2, 0.0f, 5.0f);
		ImGui::PopItemWidth();

		ImGui::PushItemWidth(80);
		ImGui::Text("Lists:");
		static int selection[4] = { 0, 1, 2, 3 };
		for (int i = 0; i < 4; i++)
		{
			if (i > 0) ImGui::SameLine();
			ImGui::PushID(i);
			ImGui::ListBox("", &selection[i], items, IM_ARRAYSIZE(items));
			ImGui::PopID();
			//if (ImGui::IsItemHovered()) ImGui::SetTooltip("ListBox %d hovered", i);
		}
		ImGui::PopItemWidth();

		// Dummy
		ImVec2 button_sz(40, 40);
		ImGui::Button("A", button_sz); ImGui::SameLine();
		ImGui::Dummy(button_sz); ImGui::SameLine();
		ImGui::Button("B", button_sz);

		// Manually wrapping (we should eventually provide this as an automatic layout feature, but for now you can do it manually)
		ImGui::Text("Manually wrapping:");
		ImGuiStyle& style = ImGui::GetStyle();
		int buttons_count = 20;
		float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
		for (int n = 0; n < buttons_count; n++)
		{
			ImGui::PushID(n);
			ImGui::Button("Box", button_sz);
			float last_button_x2 = ImGui::GetItemRectMax().x;
			float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
			if (n + 1 < buttons_count && next_button_x2 < window_visible_x2)
				ImGui::SameLine();
			ImGui::PopID();
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Tabs"))
	{
		if (ImGui::TreeNode("Basic"))
		{
			ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
			if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
			{
				if (ImGui::BeginTabItem("Avocado"))
				{
					ImGui::Text("This is the Avocado tab!\nblah blah blah blah blah");
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Broccoli"))
				{
					ImGui::Text("This is the Broccoli tab!\nblah blah blah blah blah");
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Cucumber"))
				{
					ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			ImGui::Separator();
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Advanced & Close Button"))
		{
			// Expose a couple of the available flags. In most cases you may just call BeginTabBar() with no flags (0).
			static ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable;
			ImGui::CheckboxFlags("ImGuiTabBarFlags_Reorderable", (unsigned int*)&tab_bar_flags, ImGuiTabBarFlags_Reorderable);
			ImGui::CheckboxFlags("ImGuiTabBarFlags_AutoSelectNewTabs", (unsigned int*)&tab_bar_flags, ImGuiTabBarFlags_AutoSelectNewTabs);
			ImGui::CheckboxFlags("ImGuiTabBarFlags_TabListPopupButton", (unsigned int*)&tab_bar_flags, ImGuiTabBarFlags_TabListPopupButton);
			ImGui::CheckboxFlags("ImGuiTabBarFlags_NoCloseWithMiddleMouseButton", (unsigned int*)&tab_bar_flags, ImGuiTabBarFlags_NoCloseWithMiddleMouseButton);
			if ((tab_bar_flags & ImGuiTabBarFlags_FittingPolicyMask_) == 0)
				tab_bar_flags |= ImGuiTabBarFlags_FittingPolicyDefault_;
			if (ImGui::CheckboxFlags("ImGuiTabBarFlags_FittingPolicyResizeDown", (unsigned int*)&tab_bar_flags, ImGuiTabBarFlags_FittingPolicyResizeDown))
				tab_bar_flags &= ~(ImGuiTabBarFlags_FittingPolicyMask_ ^ ImGuiTabBarFlags_FittingPolicyResizeDown);
			if (ImGui::CheckboxFlags("ImGuiTabBarFlags_FittingPolicyScroll", (unsigned int*)&tab_bar_flags, ImGuiTabBarFlags_FittingPolicyScroll))
				tab_bar_flags &= ~(ImGuiTabBarFlags_FittingPolicyMask_ ^ ImGuiTabBarFlags_FittingPolicyScroll);

			// Tab Bar
			const char* names[4] = { "Artichoke", "Beetroot", "Celery", "Daikon" };
			static bool opened[4] = { true, true, true, true }; // Persistent user state
			for (int n = 0; n < IM_ARRAYSIZE(opened); n++)
			{
				if (n > 0) { ImGui::SameLine(); }
				ImGui::Checkbox(names[n], &opened[n]);
			}

			// Passing a bool* to BeginTabItem() is similar to passing one to Begin(): the underlying bool will be set to false when the tab is closed.
			if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
			{
				for (int n = 0; n < IM_ARRAYSIZE(opened); n++)
					if (opened[n] && ImGui::BeginTabItem(names[n], &opened[n]))
					{
						ImGui::Text("This is the %s tab!", names[n]);
						if (n & 1)
							ImGui::Text("I am an odd tab.");
						ImGui::EndTabItem();
					}
				ImGui::EndTabBar();
			}
			ImGui::Separator();
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Groups"))
	{
		ShowHelpMarker("Using ImGui::BeginGroup()/EndGroup() to layout items. BeginGroup() basically locks the horizontal position. EndGroup() bundles the whole group so that you can use functions such as IsItemHovered() on it.");
		ImGui::BeginGroup();
		{
			ImGui::BeginGroup();
			ImGui::Button("AAA");
			ImGui::SameLine();
			ImGui::Button("BBB");
			ImGui::SameLine();
			ImGui::BeginGroup();
			ImGui::Button("CCC");
			ImGui::Button("DDD");
			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::Button("EEE");
			ImGui::EndGroup();
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("First group hovered");
		}
		// Capture the group size and create widgets using the same size
		ImVec2 size = ImGui::GetItemRectSize();
		const float values[5] = { 0.5f, 0.20f, 0.80f, 0.60f, 0.25f };
		ImGui::PlotHistogram("##values", values, IM_ARRAYSIZE(values), 0, NULL, 0.0f, 1.0f, size);

		ImGui::Button("ACTION", ImVec2((size.x - ImGui::GetStyle().ItemSpacing.x)*0.5f, size.y));
		ImGui::SameLine();
		ImGui::Button("REACTION", ImVec2((size.x - ImGui::GetStyle().ItemSpacing.x)*0.5f, size.y));
		ImGui::EndGroup();
		ImGui::SameLine();

		ImGui::Button("LEVERAGE\nBUZZWORD", size);
		ImGui::SameLine();

		if (ImGui::ListBoxHeader("List", size))
		{
			ImGui::Selectable("Selected", true);
			ImGui::Selectable("Not Selected", false);
			ImGui::ListBoxFooter();
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Text Baseline Alignment"))
	{
		ShowHelpMarker("This is testing the vertical alignment that gets applied on text to keep it aligned with widgets. Lines only composed of text or \"small\" widgets fit in less vertical spaces than lines with normal widgets.");

		ImGui::Text("One\nTwo\nThree"); ImGui::SameLine();
		ImGui::Text("Hello\nWorld"); ImGui::SameLine();
		ImGui::Text("Banana");

		ImGui::Text("Banana"); ImGui::SameLine();
		ImGui::Text("Hello\nWorld"); ImGui::SameLine();
		ImGui::Text("One\nTwo\nThree");

		ImGui::Button("HOP##1"); ImGui::SameLine();
		ImGui::Text("Banana"); ImGui::SameLine();
		ImGui::Text("Hello\nWorld"); ImGui::SameLine();
		ImGui::Text("Banana");

		ImGui::Button("HOP##2"); ImGui::SameLine();
		ImGui::Text("Hello\nWorld"); ImGui::SameLine();
		ImGui::Text("Banana");

		ImGui::Button("TEST##1"); ImGui::SameLine();
		ImGui::Text("TEST"); ImGui::SameLine();
		ImGui::SmallButton("TEST##2");

		ImGui::AlignTextToFramePadding(); // If your line starts with text, call this to align it to upcoming widgets.
		ImGui::Text("Text aligned to Widget"); ImGui::SameLine();
		ImGui::Button("Widget##1"); ImGui::SameLine();
		ImGui::Text("Widget"); ImGui::SameLine();
		ImGui::SmallButton("Widget##2"); ImGui::SameLine();
		ImGui::Button("Widget##3");

		// Tree
		const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
		ImGui::Button("Button##1");
		ImGui::SameLine(0.0f, spacing);
		if (ImGui::TreeNode("Node##1")) { for (int i = 0; i < 6; i++) ImGui::BulletText("Item %d..", i); ImGui::TreePop(); }    // Dummy tree data

		ImGui::AlignTextToFramePadding();         // Vertically align text node a bit lower so it'll be vertically centered with upcoming widget. Otherwise you can use SmallButton (smaller fit).
		bool node_open = ImGui::TreeNode("Node##2");  // Common mistake to avoid: if we want to SameLine after TreeNode we need to do it before we add child content.
		ImGui::SameLine(0.0f, spacing); ImGui::Button("Button##2");
		if (node_open) { for (int i = 0; i < 6; i++) ImGui::BulletText("Item %d..", i); ImGui::TreePop(); }   // Dummy tree data

		// Bullet
		ImGui::Button("Button##3");
		ImGui::SameLine(0.0f, spacing);
		ImGui::BulletText("Bullet text");

		ImGui::AlignTextToFramePadding();
		ImGui::BulletText("Node");
		ImGui::SameLine(0.0f, spacing); ImGui::Button("Button##4");

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Scrolling"))
	{
		ShowHelpMarker("Use SetScrollHereY() or SetScrollFromPosY() to scroll to a given position.");

		static bool track = true;
		static int track_line = 50, scroll_to_px = 200;
		ImGui::Checkbox("Track", &track);
		ImGui::PushItemWidth(100);
		ImGui::SameLine(130); track |= ImGui::DragInt("##line", &track_line, 0.25f, 0, 99, "Line = %d");
		bool scroll_to = ImGui::Button("Scroll To Pos");
		ImGui::SameLine(130); scroll_to |= ImGui::DragInt("##pos_y", &scroll_to_px, 1.00f, 0, 9999, "Y = %d px");
		ImGui::PopItemWidth();
		if (scroll_to) track = false;

		for (int i = 0; i < 5; i++)
		{
			if (i > 0) ImGui::SameLine();
			ImGui::BeginGroup();
			ImGui::Text("%s", i == 0 ? "Top" : i == 1 ? "25%" : i == 2 ? "Center" : i == 3 ? "75%" : "Bottom");
			ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)i), ImVec2(ImGui::GetWindowWidth() * 0.17f, 200.0f), true);
			if (scroll_to)
				ImGui::SetScrollFromPosY(ImGui::GetCursorStartPos().y + scroll_to_px, i * 0.25f);
			for (int line = 0; line < 100; line++)
			{
				if (track && line == track_line)
				{
					ImGui::TextColored(ImVec4(1, 1, 0, 1), "Line %d", line);
					ImGui::SetScrollHereY(i * 0.25f); // 0.0f:top, 0.5f:center, 1.0f:bottom
				}
				else
				{
					ImGui::Text("Line %d", line);
				}
			}
			float scroll_y = ImGui::GetScrollY(), scroll_max_y = ImGui::GetScrollMaxY();
			ImGui::EndChild();
			ImGui::Text("%.0f/%0.f", scroll_y, scroll_max_y);
			ImGui::EndGroup();
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Horizontal Scrolling"))
	{
		ShowHelpMarker("Horizontal scrolling for a window has to be enabled explicitly via the ImGuiWindowFlags_HorizontalScrollbar flag.\n\nYou may want to explicitly specify content width by calling SetNextWindowContentWidth() before Begin().");
		static int lines = 7;
		ImGui::SliderInt("Lines", &lines, 1, 15);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 1.0f));
		ImGui::BeginChild("scrolling", ImVec2(0, ImGui::GetFrameHeightWithSpacing() * 7 + 30), true, ImGuiWindowFlags_HorizontalScrollbar);
		for (int line = 0; line < lines; line++)
		{
			// Display random stuff (for the sake of this trivial demo we are using basic Button+SameLine. If you want to create your own time line for a real application you may be better off
			// manipulating the cursor position yourself, aka using SetCursorPos/SetCursorScreenPos to position the widgets yourself. You may also want to use the lower-level ImDrawList API)
			int num_buttons = 10 + ((line & 1) ? line * 9 : line * 3);
			for (int n = 0; n < num_buttons; n++)
			{
				if (n > 0) ImGui::SameLine();
				ImGui::PushID(n + line * 1000);
				char num_buf[16];
				sprintf(num_buf, "%d", n);
				const char* label = (!(n % 15)) ? "FizzBuzz" : (!(n % 3)) ? "Fizz" : (!(n % 5)) ? "Buzz" : num_buf;
				float hue = n * 0.05f;
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(hue, 0.6f, 0.6f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(hue, 0.7f, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(hue, 0.8f, 0.8f));
				ImGui::Button(label, ImVec2(40.0f + sinf((float)(line + n)) * 20.0f, 0.0f));
				ImGui::PopStyleColor(3);
				ImGui::PopID();
			}
		}
		float scroll_x = ImGui::GetScrollX(), scroll_max_x = ImGui::GetScrollMaxX();
		ImGui::EndChild();
		ImGui::PopStyleVar(2);
		float scroll_x_delta = 0.0f;
		ImGui::SmallButton("<<"); if (ImGui::IsItemActive()) { scroll_x_delta = -ImGui::GetIO().DeltaTime * 1000.0f; } ImGui::SameLine();
		ImGui::Text("Scroll from code"); ImGui::SameLine();
		ImGui::SmallButton(">>"); if (ImGui::IsItemActive()) { scroll_x_delta = +ImGui::GetIO().DeltaTime * 1000.0f; } ImGui::SameLine();
		ImGui::Text("%.0f/%.0f", scroll_x, scroll_max_x);
		if (scroll_x_delta != 0.0f)
		{
			ImGui::BeginChild("scrolling"); // Demonstrate a trick: you can use Begin to set yourself in the context of another window (here we are already out of your child window)
			ImGui::SetScrollX(ImGui::GetScrollX() + scroll_x_delta);
			ImGui::EndChild();
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Clipping"))
	{
		static ImVec2 size(100, 100), offset(50, 20);
		ImGui::TextWrapped("On a per-widget basis we are occasionally clipping text CPU-side if it won't fit in its frame. Otherwise we are doing coarser clipping + passing a scissor rectangle to the renderer. The system is designed to try minimizing both execution and CPU/GPU rendering cost.");
		ImGui::DragFloat2("size", (float*)&size, 0.5f, 1.0f, 200.0f, "%.0f");
		ImGui::TextWrapped("(Click and drag)");
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec4 clip_rect(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
		ImGui::InvisibleButton("##dummy", size);
		if (ImGui::IsItemActive() && ImGui::IsMouseDragging()) { offset.x += ImGui::GetIO().MouseDelta.x; offset.y += ImGui::GetIO().MouseDelta.y; }
		ImGui::GetWindowDrawList()->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(90, 90, 120, 255));
		ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize()*2.0f, ImVec2(pos.x + offset.x, pos.y + offset.y), IM_COL32(255, 255, 255, 255), "Line 1 hello\nLine 2 clip me!", NULL, 0.0f, &clip_rect);
		ImGui::TreePop();
	}
}

static void ShowDemoWindowPopups()
{
	if (!ImGui::CollapsingHeader("Popups & Modal windows"))
		return;

	// The properties of popups windows are:
	// - They block normal mouse hovering detection outside them. (*)
	// - Unless modal, they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
	// - Their visibility state (~bool) is held internally by imgui instead of being held by the programmer as we are used to with regular Begin() calls.
	//   User can manipulate the visibility state by calling OpenPopup().
	// (*) One can use IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) to bypass it and detect hovering even when normally blocked by a popup.
	// Those three properties are connected. The library needs to hold their visibility state because it can close popups at any time.

	// Typical use for regular windows:
	//   bool my_tool_is_active = false; if (ImGui::Button("Open")) my_tool_is_active = true; [...] if (my_tool_is_active) Begin("My Tool", &my_tool_is_active) { [...] } End();
	// Typical use for popups:
	//   if (ImGui::Button("Open")) ImGui::OpenPopup("MyPopup"); if (ImGui::BeginPopup("MyPopup") { [...] EndPopup(); }

	// With popups we have to go through a library call (here OpenPopup) to manipulate the visibility state.
	// This may be a bit confusing at first but it should quickly make sense. Follow on the examples below.

	if (ImGui::TreeNode("Popups"))
	{
		ImGui::TextWrapped("When a popup is active, it inhibits interacting with windows that are behind the popup. Clicking outside the popup closes it.");

		static int selected_fish = -1;
		const char* names[] = { "Bream", "Haddock", "Mackerel", "Pollock", "Tilefish" };
		static bool toggles[] = { true, false, false, false, false };

		// Simple selection popup
		// (If you want to show the current selection inside the Button itself, you may want to build a string using the "###" operator to preserve a constant ID with a variable label)
		if (ImGui::Button("Select.."))
			ImGui::OpenPopup("my_select_popup");
		ImGui::SameLine();
		ImGui::TextUnformatted(selected_fish == -1 ? "<None>" : names[selected_fish]);
		if (ImGui::BeginPopup("my_select_popup"))
		{
			ImGui::Text("Aquarium");
			ImGui::Separator();
			for (int i = 0; i < IM_ARRAYSIZE(names); i++)
				if (ImGui::Selectable(names[i]))
					selected_fish = i;
			ImGui::EndPopup();
		}

		// Showing a menu with toggles
		if (ImGui::Button("Toggle.."))
			ImGui::OpenPopup("my_toggle_popup");
		if (ImGui::BeginPopup("my_toggle_popup"))
		{
			for (int i = 0; i < IM_ARRAYSIZE(names); i++)
				ImGui::MenuItem(names[i], "", &toggles[i]);
			if (ImGui::BeginMenu("Sub-menu"))
			{
				ImGui::MenuItem("Click me");
				ImGui::EndMenu();
			}

			ImGui::Separator();
			ImGui::Text("Tooltip here");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("I am a tooltip over a popup");

			if (ImGui::Button("Stacked Popup"))
				ImGui::OpenPopup("another popup");
			if (ImGui::BeginPopup("another popup"))
			{
				for (int i = 0; i < IM_ARRAYSIZE(names); i++)
					ImGui::MenuItem(names[i], "", &toggles[i]);
				if (ImGui::BeginMenu("Sub-menu"))
				{
					ImGui::MenuItem("Click me");
					ImGui::EndMenu();
				}
				ImGui::EndPopup();
			}
			ImGui::EndPopup();
		}

		// Call the more complete ShowExampleMenuFile which we use in various places of this demo
		if (ImGui::Button("File Menu.."))
			ImGui::OpenPopup("my_file_popup");
		if (ImGui::BeginPopup("my_file_popup"))
		{
			ShowExampleMenuFile();
			ImGui::EndPopup();
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Context menus"))
	{
		// BeginPopupContextItem() is a helper to provide common/simple popup behavior of essentially doing:
		//    if (IsItemHovered() && IsMouseReleased(0))
		//       OpenPopup(id);
		//    return BeginPopup(id);
		// For more advanced uses you may want to replicate and cuztomize this code. This the comments inside BeginPopupContextItem() implementation.
		static float value = 0.5f;
		ImGui::Text("Value = %.3f (<-- right-click here)", value);
		if (ImGui::BeginPopupContextItem("item context menu"))
		{
			if (ImGui::Selectable("Set to zero")) value = 0.0f;
			if (ImGui::Selectable("Set to PI")) value = 3.1415f;
			ImGui::PushItemWidth(-1);
			ImGui::DragFloat("##Value", &value, 0.1f, 0.0f, 0.0f);
			ImGui::PopItemWidth();
			ImGui::EndPopup();
		}

		// We can also use OpenPopupOnItemClick() which is the same as BeginPopupContextItem() but without the Begin call.
		// So here we will make it that clicking on the text field with the right mouse button (1) will toggle the visibility of the popup above.
		ImGui::Text("(You can also right-click me to the same popup as above.)");
		ImGui::OpenPopupOnItemClick("item context menu", 1);

		// When used after an item that has an ID (here the Button), we can skip providing an ID to BeginPopupContextItem().
		// BeginPopupContextItem() will use the last item ID as the popup ID.
		// In addition here, we want to include your editable label inside the button label. We use the ### operator to override the ID (read FAQ about ID for details)
		static char name[32] = "Label1";
		char buf[64]; sprintf(buf, "Button: %s###Button", name); // ### operator override ID ignoring the preceding label
		ImGui::Button(buf);
		if (ImGui::BeginPopupContextItem())
		{
			ImGui::Text("Edit name:");
			ImGui::InputText("##edit", name, IM_ARRAYSIZE(name));
			if (ImGui::Button("Close"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		ImGui::SameLine(); ImGui::Text("(<-- right-click here)");

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Modals"))
	{
		ImGui::TextWrapped("Modal windows are like popups but the user cannot close them by clicking outside the window.");

		if (ImGui::Button("Delete.."))
			ImGui::OpenPopup("Delete?");

		if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("All those beautiful files will be deleted.\nThis operation cannot be undone!\n\n");
			ImGui::Separator();

			//static int dummy_i = 0;
			//ImGui::Combo("Combo", &dummy_i, "Delete\0Delete harder\0");

			static bool dont_ask_me_next_time = false;
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			ImGui::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
			ImGui::PopStyleVar();

			if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}

		if (ImGui::Button("Stacked modals.."))
			ImGui::OpenPopup("Stacked 1");
		if (ImGui::BeginPopupModal("Stacked 1", NULL, ImGuiWindowFlags_MenuBar))
		{
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Dummy menu item")) {}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
			ImGui::Text("Hello from Stacked The First\nUsing style.Colors[ImGuiCol_ModalWindowDimBg] behind it.");

			// Testing behavior of widgets stacking their own regular popups over the modal.
			static int item = 1;
			static float color[4] = { 0.4f,0.7f,0.0f,0.5f };
			ImGui::Combo("Combo", &item, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");
			//ImGui::ColorEdit4("color", color);

			if (ImGui::Button("Add another modal.."))
				ImGui::OpenPopup("Stacked 2");

			// Also demonstrate passing a bool* to BeginPopupModal(), this will create a regular close button which will close the popup.
			// Note that the visibility state of popups is owned by imgui, so the input value of the bool actually doesn't matter here.
			bool dummy_open = true;
			if (ImGui::BeginPopupModal("Stacked 2", &dummy_open))
			{
				ImGui::Text("Hello from Stacked The Second!");
				if (ImGui::Button("Close"))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}

			if (ImGui::Button("Close"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Menus inside a regular window"))
	{
		ImGui::TextWrapped("Below we are testing adding menu items to a regular window. It's rather unusual but should work!");
		ImGui::Separator();
		// NB: As a quirk in this very specific example, we want to differentiate the parent of this menu from the parent of the various popup menus above.
		// To do so we are encloding the items in a PushID()/PopID() block to make them two different menusets. If we don't, opening any popup above and hovering our menu here
		// would open it. This is because once a menu is active, we allow to switch to a sibling menu by just hovering on it, which is the desired behavior for regular menus.
		ImGui::PushID("foo");
		ImGui::MenuItem("Menu item", "CTRL+M");
		if (ImGui::BeginMenu("Menu inside a regular window"))
		{
			ShowExampleMenuFile();
			ImGui::EndMenu();
		}
		ImGui::PopID();
		ImGui::Separator();
		ImGui::TreePop();
	}
}

static void ShowDemoWindowColumns()
{
	if (!ImGui::CollapsingHeader("Columns"))
		return;

	ImGui::PushID("Columns");

	// Basic columns
	if (ImGui::TreeNode("Basic"))
	{
		ImGui::Text("Without border:");
		ImGui::Columns(3, "mycolumns3", false);  // 3-ways, no border
		ImGui::Separator();
		for (int n = 0; n < 14; n++)
		{
			char label[32];
			sprintf(label, "Item %d", n);
			if (ImGui::Selectable(label)) {}
			//if (ImGui::Button(label, ImVec2(-1,0))) {}
			ImGui::NextColumn();
		}
		ImGui::Columns(1);
		ImGui::Separator();

		ImGui::Text("With border:");
		ImGui::Columns(4, "mycolumns"); // 4-ways, with border
		ImGui::Separator();
		ImGui::Text("ID"); ImGui::NextColumn();
		ImGui::Text("Name"); ImGui::NextColumn();
		ImGui::Text("Path"); ImGui::NextColumn();
		ImGui::Text("Hovered"); ImGui::NextColumn();
		ImGui::Separator();
		const char* names[3] = { "One", "Two", "Three" };
		const char* paths[3] = { "/path/one", "/path/two", "/path/three" };
		static int selected = -1;
		for (int i = 0; i < 3; i++)
		{
			char label[32];
			sprintf(label, "%04d", i);
			if (ImGui::Selectable(label, selected == i, ImGuiSelectableFlags_SpanAllColumns))
				selected = i;
			bool hovered = ImGui::IsItemHovered();
			ImGui::NextColumn();
			ImGui::Text(names[i]); ImGui::NextColumn();
			ImGui::Text(paths[i]); ImGui::NextColumn();
			ImGui::Text("%d", hovered); ImGui::NextColumn();
		}
		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::TreePop();
	}

	// Create multiple items in a same cell before switching to next column
	if (ImGui::TreeNode("Mixed items"))
	{
		ImGui::Columns(3, "mixed");
		ImGui::Separator();

		ImGui::Text("Hello");
		ImGui::Button("Banana");
		ImGui::NextColumn();

		ImGui::Text("ImGui");
		ImGui::Button("Apple");
		static float foo = 1.0f;
		ImGui::InputFloat("red", &foo, 0.05f, 0, "%.3f");
		ImGui::Text("An extra line here.");
		ImGui::NextColumn();

		ImGui::Text("Sailor");
		ImGui::Button("Corniflower");
		static float bar = 1.0f;
		ImGui::InputFloat("blue", &bar, 0.05f, 0, "%.3f");
		ImGui::NextColumn();

		if (ImGui::CollapsingHeader("Category A")) { ImGui::Text("Blah blah blah"); } ImGui::NextColumn();
		if (ImGui::CollapsingHeader("Category B")) { ImGui::Text("Blah blah blah"); } ImGui::NextColumn();
		if (ImGui::CollapsingHeader("Category C")) { ImGui::Text("Blah blah blah"); } ImGui::NextColumn();
		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::TreePop();
	}

	// Word wrapping
	if (ImGui::TreeNode("Word-wrapping"))
	{
		ImGui::Columns(2, "word-wrapping");
		ImGui::Separator();
		ImGui::TextWrapped("The quick brown fox jumps over the lazy dog.");
		ImGui::TextWrapped("Hello Left");
		ImGui::NextColumn();
		ImGui::TextWrapped("The quick brown fox jumps over the lazy dog.");
		ImGui::TextWrapped("Hello Right");
		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Borders"))
	{
		// NB: Future columns API should allow automatic horizontal borders.
		static bool h_borders = true;
		static bool v_borders = true;
		ImGui::Checkbox("horizontal", &h_borders);
		ImGui::SameLine();
		ImGui::Checkbox("vertical", &v_borders);
		ImGui::Columns(4, NULL, v_borders);
		for (int i = 0; i < 4 * 3; i++)
		{
			if (h_borders && ImGui::GetColumnIndex() == 0)
				ImGui::Separator();
			ImGui::Text("%c%c%c", 'a' + i, 'a' + i, 'a' + i);
			ImGui::Text("Width %.2f\nOffset %.2f", ImGui::GetColumnWidth(), ImGui::GetColumnOffset());
			ImGui::NextColumn();
		}
		ImGui::Columns(1);
		if (h_borders)
			ImGui::Separator();
		ImGui::TreePop();
	}

	// Scrolling columns
	/*
	if (ImGui::TreeNode("Vertical Scrolling"))
	{
		ImGui::BeginChild("##header", ImVec2(0, ImGui::GetTextLineHeightWithSpacing()+ImGui::GetStyle().ItemSpacing.y));
		ImGui::Columns(3);
		ImGui::Text("ID"); ImGui::NextColumn();
		ImGui::Text("Name"); ImGui::NextColumn();
		ImGui::Text("Path"); ImGui::NextColumn();
		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::EndChild();
		ImGui::BeginChild("##scrollingregion", ImVec2(0, 60));
		ImGui::Columns(3);
		for (int i = 0; i < 10; i++)
		{
			ImGui::Text("%04d", i); ImGui::NextColumn();
			ImGui::Text("Foobar"); ImGui::NextColumn();
			ImGui::Text("/path/foobar/%04d/", i); ImGui::NextColumn();
		}
		ImGui::Columns(1);
		ImGui::EndChild();
		ImGui::TreePop();
	}
	*/

	if (ImGui::TreeNode("Horizontal Scrolling"))
	{
		ImGui::SetNextWindowContentSize(ImVec2(1500.0f, 0.0f));
		ImGui::BeginChild("##ScrollingRegion", ImVec2(0, ImGui::GetFontSize() * 20), false, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::Columns(10);
		int ITEMS_COUNT = 2000;
		ImGuiListClipper clipper(ITEMS_COUNT);  // Also demonstrate using the clipper for large list
		while (clipper.Step())
		{
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				for (int j = 0; j < 10; j++)
				{
					ImGui::Text("Line %d Column %d...", i, j);
					ImGui::NextColumn();
				}
		}
		ImGui::Columns(1);
		ImGui::EndChild();
		ImGui::TreePop();
	}

	bool node_open = ImGui::TreeNode("Tree within single cell");
	ImGui::SameLine(); ShowHelpMarker("NB: Tree node must be poped before ending the cell. There's no storage of state per-cell.");
	if (node_open)
	{
		ImGui::Columns(2, "tree items");
		ImGui::Separator();
		if (ImGui::TreeNode("Hello")) { ImGui::BulletText("Sailor"); ImGui::TreePop(); } ImGui::NextColumn();
		if (ImGui::TreeNode("Bonjour")) { ImGui::BulletText("Marin"); ImGui::TreePop(); } ImGui::NextColumn();
		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::TreePop();
	}
	ImGui::PopID();
}

static void ShowDemoWindowMisc()
{
	if (ImGui::CollapsingHeader("Filtering"))
	{
		static ImGuiTextFilter filter;
		ImGui::Text("Filter usage:\n"
			"  \"\"         display all lines\n"
			"  \"xxx\"      display lines containing \"xxx\"\n"
			"  \"xxx,yyy\"  display lines containing \"xxx\" or \"yyy\"\n"
			"  \"-xxx\"     hide lines containing \"xxx\"");
		filter.Draw();
		const char* lines[] = { "aaa1.c", "bbb1.c", "ccc1.c", "aaa2.cpp", "bbb2.cpp", "ccc2.cpp", "abc.h", "hello, world" };
		for (int i = 0; i < IM_ARRAYSIZE(lines); i++)
			if (filter.PassFilter(lines[i]))
				ImGui::BulletText("%s", lines[i]);
	}

	if (ImGui::CollapsingHeader("Inputs, Navigation & Focus"))
	{
		ImGuiIO& io = ImGui::GetIO();

		ImGui::Text("WantCaptureMouse: %d", io.WantCaptureMouse);
		ImGui::Text("WantCaptureKeyboard: %d", io.WantCaptureKeyboard);
		ImGui::Text("WantTextInput: %d", io.WantTextInput);
		ImGui::Text("WantSetMousePos: %d", io.WantSetMousePos);
		ImGui::Text("NavActive: %d, NavVisible: %d", io.NavActive, io.NavVisible);

		if (ImGui::TreeNode("Keyboard, Mouse & Navigation State"))
		{
			if (ImGui::IsMousePosValid())
				ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
			else
				ImGui::Text("Mouse pos: <INVALID>");
			ImGui::Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
			ImGui::Text("Mouse down:");     for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (io.MouseDownDuration[i] >= 0.0f) { ImGui::SameLine(); ImGui::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]); }
			ImGui::Text("Mouse clicked:");  for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseClicked(i)) { ImGui::SameLine(); ImGui::Text("b%d", i); }
			ImGui::Text("Mouse dbl-clicked:"); for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseDoubleClicked(i)) { ImGui::SameLine(); ImGui::Text("b%d", i); }
			ImGui::Text("Mouse released:"); for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseReleased(i)) { ImGui::SameLine(); ImGui::Text("b%d", i); }
			ImGui::Text("Mouse wheel: %.1f", io.MouseWheel);

			ImGui::Text("Keys down:");      for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) if (io.KeysDownDuration[i] >= 0.0f) { ImGui::SameLine(); ImGui::Text("%d (%.02f secs)", i, io.KeysDownDuration[i]); }
			ImGui::Text("Keys pressed:");   for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) if (ImGui::IsKeyPressed(i)) { ImGui::SameLine(); ImGui::Text("%d", i); }
			ImGui::Text("Keys release:");   for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) if (ImGui::IsKeyReleased(i)) { ImGui::SameLine(); ImGui::Text("%d", i); }
			ImGui::Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "CTRL " : "", io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "", io.KeySuper ? "SUPER " : "");

			ImGui::Text("NavInputs down:"); for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputs[i] > 0.0f) { ImGui::SameLine(); ImGui::Text("[%d] %.2f", i, io.NavInputs[i]); }
			ImGui::Text("NavInputs pressed:"); for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputsDownDuration[i] == 0.0f) { ImGui::SameLine(); ImGui::Text("[%d]", i); }
			ImGui::Text("NavInputs duration:"); for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputsDownDuration[i] >= 0.0f) { ImGui::SameLine(); ImGui::Text("[%d] %.2f", i, io.NavInputsDownDuration[i]); }

			ImGui::Button("Hovering me sets the\nkeyboard capture flag");
			if (ImGui::IsItemHovered())
				ImGui::CaptureKeyboardFromApp(true);
			ImGui::SameLine();
			ImGui::Button("Holding me clears the\nthe keyboard capture flag");
			if (ImGui::IsItemActive())
				ImGui::CaptureKeyboardFromApp(false);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Tabbing"))
		{
			ImGui::Text("Use TAB/SHIFT+TAB to cycle through keyboard editable fields.");
			static char buf[32] = "dummy";
			ImGui::InputText("1", buf, IM_ARRAYSIZE(buf));
			ImGui::InputText("2", buf, IM_ARRAYSIZE(buf));
			ImGui::InputText("3", buf, IM_ARRAYSIZE(buf));
			ImGui::PushAllowKeyboardFocus(false);
			ImGui::InputText("4 (tab skip)", buf, IM_ARRAYSIZE(buf));
			//ImGui::SameLine(); ShowHelperMarker("Use ImGui::PushAllowKeyboardFocus(bool)\nto disable tabbing through certain widgets.");
			ImGui::PopAllowKeyboardFocus();
			ImGui::InputText("5", buf, IM_ARRAYSIZE(buf));
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Focus from code"))
		{
			bool focus_1 = ImGui::Button("Focus on 1"); ImGui::SameLine();
			bool focus_2 = ImGui::Button("Focus on 2"); ImGui::SameLine();
			bool focus_3 = ImGui::Button("Focus on 3");
			int has_focus = 0;
			static char buf[128] = "click on a button to set focus";

			if (focus_1) ImGui::SetKeyboardFocusHere();
			ImGui::InputText("1", buf, IM_ARRAYSIZE(buf));
			if (ImGui::IsItemActive()) has_focus = 1;

			if (focus_2) ImGui::SetKeyboardFocusHere();
			ImGui::InputText("2", buf, IM_ARRAYSIZE(buf));
			if (ImGui::IsItemActive()) has_focus = 2;

			ImGui::PushAllowKeyboardFocus(false);
			if (focus_3) ImGui::SetKeyboardFocusHere();
			ImGui::InputText("3 (tab skip)", buf, IM_ARRAYSIZE(buf));
			if (ImGui::IsItemActive()) has_focus = 3;
			ImGui::PopAllowKeyboardFocus();

			if (has_focus)
				ImGui::Text("Item with focus: %d", has_focus);
			else
				ImGui::Text("Item with focus: <none>");

			// Use >= 0 parameter to SetKeyboardFocusHere() to focus an upcoming item
			static float f3[3] = { 0.0f, 0.0f, 0.0f };
			int focus_ahead = -1;
			if (ImGui::Button("Focus on X")) { focus_ahead = 0; } ImGui::SameLine();
			if (ImGui::Button("Focus on Y")) { focus_ahead = 1; } ImGui::SameLine();
			if (ImGui::Button("Focus on Z")) { focus_ahead = 2; }
			if (focus_ahead != -1) ImGui::SetKeyboardFocusHere(focus_ahead);
			ImGui::SliderFloat3("Float3", &f3[0], 0.0f, 1.0f);

			ImGui::TextWrapped("NB: Cursor & selection are preserved when refocusing last used item in code.");
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Dragging"))
		{
			ImGui::TextWrapped("You can use ImGui::GetMouseDragDelta(0) to query for the dragged amount on any widget.");
			for (int button = 0; button < 3; button++)
				ImGui::Text("IsMouseDragging(%d):\n  w/ default threshold: %d,\n  w/ zero threshold: %d\n  w/ large threshold: %d",
					button, ImGui::IsMouseDragging(button), ImGui::IsMouseDragging(button, 0.0f), ImGui::IsMouseDragging(button, 20.0f));
			ImGui::Button("Drag Me");
			if (ImGui::IsItemActive())
			{
				// Draw a line between the button and the mouse cursor
				ImDrawList* draw_list = ImGui::GetWindowDrawList();
				draw_list->PushClipRectFullScreen();
				draw_list->AddLine(io.MouseClickedPos[0], io.MousePos, ImGui::GetColorU32(ImGuiCol_Button), 4.0f);
				draw_list->PopClipRect();

				// Drag operations gets "unlocked" when the mouse has moved past a certain threshold (the default threshold is stored in io.MouseDragThreshold)
				// You can request a lower or higher threshold using the second parameter of IsMouseDragging() and GetMouseDragDelta()
				ImVec2 value_raw = ImGui::GetMouseDragDelta(0, 0.0f);
				ImVec2 value_with_lock_threshold = ImGui::GetMouseDragDelta(0);
				ImVec2 mouse_delta = io.MouseDelta;
				ImGui::SameLine(); ImGui::Text("Raw (%.1f, %.1f), WithLockThresold (%.1f, %.1f), MouseDelta (%.1f, %.1f)", value_raw.x, value_raw.y, value_with_lock_threshold.x, value_with_lock_threshold.y, mouse_delta.x, mouse_delta.y);
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Mouse cursors"))
		{
			const char* mouse_cursors_names[] = { "Arrow", "TextInput", "Move", "ResizeNS", "ResizeEW", "ResizeNESW", "ResizeNWSE", "Hand" };
			IM_ASSERT(IM_ARRAYSIZE(mouse_cursors_names) == ImGuiMouseCursor_COUNT);

			ImGui::Text("Current mouse cursor = %d: %s", ImGui::GetMouseCursor(), mouse_cursors_names[ImGui::GetMouseCursor()]);
			ImGui::Text("Hover to see mouse cursors:");
			ImGui::SameLine(); ShowHelpMarker("Your application can render a different mouse cursor based on what ImGui::GetMouseCursor() returns. If software cursor rendering (io.MouseDrawCursor) is set ImGui will draw the right cursor for you, otherwise your backend needs to handle it.");
			for (int i = 0; i < ImGuiMouseCursor_COUNT; i++)
			{
				char label[32];
				sprintf(label, "Mouse cursor %d: %s", i, mouse_cursors_names[i]);
				ImGui::Bullet(); ImGui::Selectable(label, false);
				if (ImGui::IsItemHovered() || ImGui::IsItemFocused())
					ImGui::SetMouseCursor(i);
			}
			ImGui::TreePop();
		}
	}
}

//-----------------------------------------------------------------------------
// [SECTION] About Window / ShowAboutWindow()
// Access from ImGui Demo -> Help -> About
//-----------------------------------------------------------------------------

void ImGui::ShowAboutWindow(bool* p_open)
{
	if (!ImGui::Begin("About Dear ImGui", p_open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}
	ImGui::Text("Dear ImGui %s", ImGui::GetVersion());
	ImGui::Separator();
	ImGui::Text("By Omar Cornut and all dear imgui contributors.");
	ImGui::Text("Dear ImGui is licensed under the MIT License, see LICENSE for more information.");

	static bool show_config_info = false;
	ImGui::Checkbox("Config/Build Information", &show_config_info);
	if (show_config_info)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();

		bool copy_to_clipboard = ImGui::Button("Copy to clipboard");
		ImGui::BeginChildFrame(ImGui::GetID("cfginfos"), ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 18), ImGuiWindowFlags_NoMove);
		if (copy_to_clipboard)
			ImGui::LogToClipboard();

		ImGui::Text("Dear ImGui %s (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
		ImGui::Separator();
		ImGui::Text("sizeof(size_t): %d, sizeof(ImDrawIdx): %d, sizeof(ImDrawVert): %d", (int)sizeof(size_t), (int)sizeof(ImDrawIdx), (int)sizeof(ImDrawVert));
		ImGui::Text("define: __cplusplus=%d", (int)__cplusplus);
#ifdef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
		ImGui::Text("define: IMGUI_DISABLE_OBSOLETE_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
		ImGui::Text("define: IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
		ImGui::Text("define: IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_FUNCTIONS
		ImGui::Text("define: IMGUI_DISABLE_WIN32_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_FORMAT_STRING_FUNCTIONS
		ImGui::Text("define: IMGUI_DISABLE_FORMAT_STRING_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_MATH_FUNCTIONS
		ImGui::Text("define: IMGUI_DISABLE_MATH_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_ALLOCATORS
		ImGui::Text("define: IMGUI_DISABLE_DEFAULT_ALLOCATORS");
#endif
#ifdef IMGUI_USE_BGRA_PACKED_COLOR
		ImGui::Text("define: IMGUI_USE_BGRA_PACKED_COLOR");
#endif
#ifdef _WIN32
		ImGui::Text("define: _WIN32");
#endif
#ifdef _WIN64
		ImGui::Text("define: _WIN64");
#endif
#ifdef __linux__
		ImGui::Text("define: __linux__");
#endif
#ifdef __APPLE__
		ImGui::Text("define: __APPLE__");
#endif
#ifdef _MSC_VER
		ImGui::Text("define: _MSC_VER=%d", _MSC_VER);
#endif
#ifdef __MINGW32__
		ImGui::Text("define: __MINGW32__");
#endif
#ifdef __MINGW64__
		ImGui::Text("define: __MINGW64__");
#endif
#ifdef __GNUC__
		ImGui::Text("define: __GNUC__=%d", (int)__GNUC__);
#endif
#ifdef __clang_version__
		ImGui::Text("define: __clang_version__=%s", __clang_version__);
#endif
		ImGui::Separator();
		ImGui::Text("io.BackendPlatformName: %s", io.BackendPlatformName ? io.BackendPlatformName : "NULL");
		ImGui::Text("io.BackendRendererName: %s", io.BackendRendererName ? io.BackendRendererName : "NULL");
		ImGui::Text("io.ConfigFlags: 0x%08X", io.ConfigFlags);
		if (io.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard)        ImGui::Text(" NavEnableKeyboard");
		if (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad)         ImGui::Text(" NavEnableGamepad");
		if (io.ConfigFlags & ImGuiConfigFlags_NavEnableSetMousePos)     ImGui::Text(" NavEnableSetMousePos");
		if (io.ConfigFlags & ImGuiConfigFlags_NavNoCaptureKeyboard)     ImGui::Text(" NavNoCaptureKeyboard");
		if (io.ConfigFlags & ImGuiConfigFlags_NoMouse)                  ImGui::Text(" NoMouse");
		if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)      ImGui::Text(" NoMouseCursorChange");
		if (io.MouseDrawCursor)                                         ImGui::Text("io.MouseDrawCursor");
		if (io.ConfigMacOSXBehaviors)                                   ImGui::Text("io.ConfigMacOSXBehaviors");
		if (io.ConfigInputTextCursorBlink)                              ImGui::Text("io.ConfigInputTextCursorBlink");
		if (io.ConfigWindowsResizeFromEdges)                            ImGui::Text("io.ConfigWindowsResizeFromEdges");
		if (io.ConfigWindowsMoveFromTitleBarOnly)                       ImGui::Text("io.ConfigWindowsMoveFromTitleBarOnly");
		ImGui::Text("io.BackendFlags: 0x%08X", io.BackendFlags);
		if (io.BackendFlags & ImGuiBackendFlags_HasGamepad)             ImGui::Text(" HasGamepad");
		if (io.BackendFlags & ImGuiBackendFlags_HasMouseCursors)        ImGui::Text(" HasMouseCursors");
		if (io.BackendFlags & ImGuiBackendFlags_HasSetMousePos)         ImGui::Text(" HasSetMousePos");
		ImGui::Separator();
		ImGui::Text("io.Fonts: %d fonts, Flags: 0x%08X, TexSize: %d,%d", io.Fonts->Fonts.Size, io.Fonts->Flags, io.Fonts->TexWidth, io.Fonts->TexHeight);
		ImGui::Text("io.DisplaySize: %.2f,%.2f", io.DisplaySize.x, io.DisplaySize.y);
		ImGui::Text("io.DisplayFramebufferScale: %.2f,%.2f", io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
		ImGui::Separator();
		ImGui::Text("style.WindowPadding: %.2f,%.2f", style.WindowPadding.x, style.WindowPadding.y);
		ImGui::Text("style.WindowBorderSize: %.2f", style.WindowBorderSize);
		ImGui::Text("style.FramePadding: %.2f,%.2f", style.FramePadding.x, style.FramePadding.y);
		ImGui::Text("style.FrameRounding: %.2f", style.FrameRounding);
		ImGui::Text("style.FrameBorderSize: %.2f", style.FrameBorderSize);
		ImGui::Text("style.ItemSpacing: %.2f,%.2f", style.ItemSpacing.x, style.ItemSpacing.y);
		ImGui::Text("style.ItemInnerSpacing: %.2f,%.2f", style.ItemInnerSpacing.x, style.ItemInnerSpacing.y);

		if (copy_to_clipboard)
			ImGui::LogFinish();
		ImGui::EndChildFrame();
	}
	ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Style Editor / ShowStyleEditor()
//-----------------------------------------------------------------------------

// Demo helper function to select among default colors. See ShowStyleEditor() for more advanced options.
// Here we use the simplified Combo() api that packs items into a single literal string. Useful for quick combo boxes where the choices are known locally.
bool ImGui::ShowStyleSelector(const char* label)
{
	static int style_idx = -1;
	if (ImGui::Combo(label, &style_idx, "Classic\0Dark\0Light\0"))
	{
		switch (style_idx)
		{
		case 0: ImGui::StyleColorsDark(); break;
		case 1: ImGui::StyleColorsIndigo(); break;
		}
		return true;
	}
	return false;
}

// Demo helper function to select among loaded fonts.
// Here we use the regular BeginCombo()/EndCombo() api which is more the more flexible one.
void ImGui::ShowFontSelector(const char* label)
{
	ImGuiIO& io = ImGui::GetIO();
	ImFont* font_current = ImGui::GetFont();
	if (ImGui::BeginCombo(label, font_current->GetDebugName()))
	{
		for (int n = 0; n < io.Fonts->Fonts.Size; n++)
		{
			ImFont* font = io.Fonts->Fonts[n];
			ImGui::PushID((void*)font);
			if (ImGui::Selectable(font->GetDebugName(), font == font_current))
				io.FontDefault = font;
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	ShowHelpMarker(
		"- Load additional fonts with io.Fonts->AddFontFromFileTTF().\n"
		"- The font atlas is built when calling io.Fonts->GetTexDataAsXXXX() or io.Fonts->Build().\n"
		"- Read FAQ and documentation in misc/fonts/ for more details.\n"
		"- If you need to add/remove fonts at runtime (e.g. for DPI change), do it before calling NewFrame().");
}

void ImGui::ShowStyleEditor(ImGuiStyle* ref)
{
	// You can pass in a reference ImGuiStyle structure to compare to, revert to and save to (else it compares to an internally stored reference)
	ImGuiStyle& style = ImGui::GetStyle();
	static ImGuiStyle ref_saved_style;

	// Default to using internal storage as reference
	static bool init = true;
	if (init && ref == NULL)
		ref_saved_style = style;
	init = false;
	if (ref == NULL)
		ref = &ref_saved_style;

	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

	if (ImGui::ShowStyleSelector("Colors##Selector"))
		ref_saved_style = style;
	ImGui::ShowFontSelector("Fonts##Selector");

	// Simplified Settings
	if (ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
		style.GrabRounding = style.FrameRounding; // Make GrabRounding always the same value as FrameRounding
	{ bool window_border = (style.WindowBorderSize > 0.0f); if (ImGui::Checkbox("WindowBorder", &window_border)) style.WindowBorderSize = window_border ? 1.0f : 0.0f; }
	ImGui::SameLine();
	{ bool frame_border = (style.FrameBorderSize > 0.0f); if (ImGui::Checkbox("FrameBorder", &frame_border)) style.FrameBorderSize = frame_border ? 1.0f : 0.0f; }
	ImGui::SameLine();
	{ bool popup_border = (style.PopupBorderSize > 0.0f); if (ImGui::Checkbox("PopupBorder", &popup_border)) style.PopupBorderSize = popup_border ? 1.0f : 0.0f; }

	// Save/Revert button
	if (ImGui::Button("Save Ref"))
		*ref = ref_saved_style = style;
	ImGui::SameLine();
	if (ImGui::Button("Revert Ref"))
		style = *ref;
	ImGui::SameLine();
	ShowHelpMarker("Save/Revert in local non-persistent storage. Default Colors definition are not affected. Use \"Export Colors\" below to save them somewhere.");

	ImGui::Separator();

	if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Sizes"))
		{
			ImGui::Text("Main");
			ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
			ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
			ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
			ImGui::Text("Borders");
			ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::Text("Rounding");
			ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f");
			ImGui::Text("Alignment");
			ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
			ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f"); ImGui::SameLine(); ShowHelpMarker("Alignment applies when a button is larger than its text content.");
			ImGui::SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f"); ImGui::SameLine(); ShowHelpMarker("Alignment applies when a selectable is larger than its text content.");
			ImGui::Text("Safe Area Padding"); ImGui::SameLine(); ShowHelpMarker("Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).");
			ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Colors"))
		{
			static int output_dest = 0;
			static bool output_only_modified = true;
			if (ImGui::Button("Export Unsaved"))
			{
				if (output_dest == 0)
					ImGui::LogToClipboard();
				else
					ImGui::LogToTTY();
				ImGui::LogText("ImVec4* colors = ImGui::GetStyle().Colors;" IM_NEWLINE);
				for (int i = 0; i < ImGuiCol_COUNT; i++)
				{
					const ImVec4& col = style.Colors[i];
					const char* name = ImGui::GetStyleColorName(i);
					if (!output_only_modified || memcmp(&col, &ref->Colors[i], sizeof(ImVec4)) != 0)
						ImGui::LogText("colors[ImGuiCol_%s]%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);" IM_NEWLINE, name, 23 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
				}
				ImGui::LogFinish();
			}
			ImGui::SameLine(); ImGui::PushItemWidth(120); ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0"); ImGui::PopItemWidth();
			ImGui::SameLine(); ImGui::Checkbox("Only Modified Colors", &output_only_modified);

			static ImGuiTextFilter filter;
			filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

			static ImGuiColorEditFlags alpha_flags = 0;
			ImGui::RadioButton("Opaque", &alpha_flags, 0); ImGui::SameLine();
			ImGui::RadioButton("Alpha", &alpha_flags, ImGuiColorEditFlags_AlphaPreview); ImGui::SameLine();
			ImGui::RadioButton("Both", &alpha_flags, ImGuiColorEditFlags_AlphaPreviewHalf); ImGui::SameLine();
			ShowHelpMarker("In the color list:\nLeft-click on colored square to open color picker,\nRight-click to open edit options menu.");

			ImGui::BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
			ImGui::PushItemWidth(-160);
			for (int i = 0; i < ImGuiCol_COUNT; i++)
			{
				const char* name = ImGui::GetStyleColorName(i);
				if (!filter.PassFilter(name))
					continue;
				ImGui::PushID(i);
				//ImGui::ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);
				if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0)
				{
					// Tips: in a real user application, you may want to merge and use an icon font into the main font, so instead of "Save"/"Revert" you'd use icons.
					// Read the FAQ and misc/fonts/README.txt about using icon fonts. It's really easy and super convenient!
					ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Save")) ref->Colors[i] = style.Colors[i];
					ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Revert")) style.Colors[i] = ref->Colors[i];
				}
				ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
				ImGui::TextUnformatted(name);
				ImGui::PopID();
			}
			ImGui::PopItemWidth();
			ImGui::EndChild();

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Fonts"))
		{
			ImGuiIO& io = ImGui::GetIO();
			ImFontAtlas* atlas = io.Fonts;
			ShowHelpMarker("Read FAQ and misc/fonts/README.txt for details on font loading.");
			ImGui::PushItemWidth(120);
			for (int i = 0; i < atlas->Fonts.Size; i++)
			{
				ImFont* font = atlas->Fonts[i];
				ImGui::PushID(font);
				bool font_details_opened = ImGui::TreeNode(font, "Font %d: \"%s\"\n%.2f px, %d glyphs, %d file(s)", i, font->ConfigData ? font->ConfigData[0].Name : "", font->FontSize, font->Glyphs.Size, font->ConfigDataCount);
				ImGui::SameLine(); if (ImGui::SmallButton("Set as default")) { io.FontDefault = font; }
				if (font_details_opened)
				{
					ImGui::PushFont(font);
					ImGui::Text("The quick brown fox jumps over the lazy dog");
					ImGui::PopFont();
					ImGui::DragFloat("Font scale", &font->Scale, 0.005f, 0.3f, 2.0f, "%.1f");   // Scale only this font
					ImGui::SameLine(); ShowHelpMarker("Note than the default embedded font is NOT meant to be scaled.\n\nFont are currently rendered into bitmaps at a given size at the time of building the atlas. You may oversample them to get some flexibility with scaling. You can also render at multiple sizes and select which one to use at runtime.\n\n(Glimmer of hope: the atlas system should hopefully be rewritten in the future to make scaling more natural and automatic.)");
					ImGui::InputFloat("Font offset", &font->DisplayOffset.y, 1, 1, "%.0f");
					ImGui::Text("Ascent: %f, Descent: %f, Height: %f", font->Ascent, font->Descent, font->Ascent - font->Descent);
					ImGui::Text("Fallback character: '%c' (%d)", font->FallbackChar, font->FallbackChar);
					const float surface_sqrt = sqrtf((float)font->MetricsTotalSurface);
					ImGui::Text("Texture surface: %d pixels (approx) ~ %dx%d", font->MetricsTotalSurface, (int)surface_sqrt, (int)surface_sqrt);
					for (int config_i = 0; config_i < font->ConfigDataCount; config_i++)
						if (const ImFontConfig* cfg = &font->ConfigData[config_i])
							ImGui::BulletText("Input %d: \'%s\', Oversample: (%d,%d), PixelSnapH: %d", config_i, cfg->Name, cfg->OversampleH, cfg->OversampleV, cfg->PixelSnapH);
					if (ImGui::TreeNode("Glyphs", "Glyphs (%d)", font->Glyphs.Size))
					{
						// Display all glyphs of the fonts in separate pages of 256 characters
						for (int base = 0; base < 0x10000; base += 256)
						{
							int count = 0;
							for (int n = 0; n < 256; n++)
								count += font->FindGlyphNoFallback((ImWchar)(base + n)) ? 1 : 0;
							if (count > 0 && ImGui::TreeNode((void*)(intptr_t)base, "U+%04X..U+%04X (%d %s)", base, base + 255, count, count > 1 ? "glyphs" : "glyph"))
							{
								float cell_size = font->FontSize * 1;
								float cell_spacing = style.ItemSpacing.y;
								ImVec2 base_pos = ImGui::GetCursorScreenPos();
								ImDrawList* draw_list = ImGui::GetWindowDrawList();
								for (int n = 0; n < 256; n++)
								{
									ImVec2 cell_p1(base_pos.x + (n % 16) * (cell_size + cell_spacing), base_pos.y + (n / 16) * (cell_size + cell_spacing));
									ImVec2 cell_p2(cell_p1.x + cell_size, cell_p1.y + cell_size);
									const ImFontGlyph* glyph = font->FindGlyphNoFallback((ImWchar)(base + n));
									draw_list->AddRect(cell_p1, cell_p2, glyph ? IM_COL32(255, 255, 255, 100) : IM_COL32(255, 255, 255, 50));
									if (glyph)
										font->RenderChar(draw_list, cell_size, cell_p1, ImGui::GetColorU32(ImGuiCol_Text), (ImWchar)(base + n)); // We use ImFont::RenderChar as a shortcut because we don't have UTF-8 conversion functions available to generate a string.
									if (glyph && ImGui::IsMouseHoveringRect(cell_p1, cell_p2))
									{
										ImGui::BeginTooltip();
										ImGui::Text("Codepoint: U+%04X", base + n);
										ImGui::Separator();
										ImGui::Text("AdvanceX: %.1f", glyph->AdvanceX);
										ImGui::Text("Pos: (%.2f,%.2f)->(%.2f,%.2f)", glyph->X0, glyph->Y0, glyph->X1, glyph->Y1);
										ImGui::Text("UV: (%.3f,%.3f)->(%.3f,%.3f)", glyph->U0, glyph->V0, glyph->U1, glyph->V1);
										ImGui::EndTooltip();
									}
								}
								ImGui::Dummy(ImVec2((cell_size + cell_spacing) * 16, (cell_size + cell_spacing) * 16));
								ImGui::TreePop();
							}
						}
						ImGui::TreePop();
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			if (ImGui::TreeNode("Atlas texture", "Atlas texture (%dx%d pixels)", atlas->TexWidth, atlas->TexHeight))
			{
				ImGui::Image(atlas->TexID, ImVec2((float)atlas->TexWidth, (float)atlas->TexHeight), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
				ImGui::TreePop();
			}

			static float window_scale = 1.0f;
			if (ImGui::DragFloat("this window scale", &window_scale, 0.005f, 0.3f, 2.0f, "%.2f"))   // scale only this window
				ImGui::SetWindowFontScale(window_scale);
			ImGui::DragFloat("global scale", &io.FontGlobalScale, 0.005f, 0.3f, 2.0f, "%.2f");      // scale everything
			ImGui::PopItemWidth();

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Rendering"))
		{
			ImGui::Checkbox("Anti-aliased lines", &style.AntiAliasedLines); ImGui::SameLine(); ShowHelpMarker("When disabling anti-aliasing lines, you'll probably want to disable borders in your style as well.");
			ImGui::Checkbox("Anti-aliased fill", &style.AntiAliasedFill);
			ImGui::PushItemWidth(100);
			ImGui::DragFloat("Curve Tessellation Tolerance", &style.CurveTessellationTol, 0.02f, 0.10f, FLT_MAX, "%.2f", 2.0f);
			if (style.CurveTessellationTol < 0.10f) style.CurveTessellationTol = 0.10f;
			ImGui::DragFloat("Global Alpha", &style.Alpha, 0.005f, 0.20f, 1.0f, "%.2f"); // Not exposing zero here so user doesn't "lose" the UI (zero alpha clips all widgets). But application code could have a toggle to switch between zero and non-zero.
			ImGui::PopItemWidth();

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::PopItemWidth();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Main Menu Bar / ShowExampleAppMainMenuBar()
//-----------------------------------------------------------------------------

// Demonstrate creating a fullscreen menu bar and populating it.
static void ShowExampleAppMainMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ShowExampleMenuFile();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

static void ShowExampleMenuFile()
{
	ImGui::MenuItem("(dummy menu)", NULL, false, false);
	if (ImGui::MenuItem("New")) {}
	if (ImGui::MenuItem("Open", "Ctrl+O")) {}
	if (ImGui::BeginMenu("Open Recent"))
	{
		ImGui::MenuItem("fish_hat.c");
		ImGui::MenuItem("fish_hat.inl");
		ImGui::MenuItem("fish_hat.h");
		if (ImGui::BeginMenu("More.."))
		{
			ImGui::MenuItem("Hello");
			ImGui::MenuItem("Sailor");
			if (ImGui::BeginMenu("Recurse.."))
			{
				ShowExampleMenuFile();
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
	if (ImGui::MenuItem("Save", "Ctrl+S")) {}
	if (ImGui::MenuItem("Save As..")) {}
	ImGui::Separator();
	if (ImGui::BeginMenu("Options"))
	{
		static bool enabled = true;
		ImGui::MenuItem("Enabled", "", &enabled);
		ImGui::BeginChild("child", ImVec2(0, 60), true);
		for (int i = 0; i < 10; i++)
			ImGui::Text("Scrolling Text %d", i);
		ImGui::EndChild();
		static float f = 0.5f;
		static int n = 0;
		static bool b = true;
		ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
		ImGui::InputFloat("Input", &f, 0.1f);
		ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
		ImGui::Checkbox("Check", &b);
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Colors"))
	{
		float sz = ImGui::GetTextLineHeight();
		for (int i = 0; i < ImGuiCol_COUNT; i++)
		{
			const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
			ImVec2 p = ImGui::GetCursorScreenPos();
			ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
			ImGui::Dummy(ImVec2(sz, sz));
			ImGui::SameLine();
			ImGui::MenuItem(name);
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Disabled", false)) // Disabled
	{
		IM_ASSERT(0);
	}
	if (ImGui::MenuItem("Checked", NULL, true)) {}
	if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Debug Console / ShowExampleAppConsole()
//-----------------------------------------------------------------------------

// Demonstrate creating a simple console window, with scrolling, filtering, completion and history.
// For the console example, here we are using a more C++ like approach of declaring a class to hold the data and the functions.
struct ExampleAppConsole
{
	char                  InputBuf[256];
	ImVector<char*>       Items;
	ImVector<const char*> Commands;
	ImVector<char*>       History;
	int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
	ImGuiTextFilter       Filter;
	bool                  AutoScroll;
	bool                  ScrollToBottom;

	ExampleAppConsole()
	{
		ClearLog();
		memset(InputBuf, 0, sizeof(InputBuf));
		HistoryPos = -1;
		Commands.push_back("HELP");
		Commands.push_back("HISTORY");
		Commands.push_back("CLEAR");
		Commands.push_back("CLASSIFY");  // "classify" is only here to provide an example of "C"+[tab] completing to "CL" and displaying matches.
		AutoScroll = true;
		ScrollToBottom = true;
		AddLog("Welcome to Dear ImGui!");
	}
	~ExampleAppConsole()
	{
		ClearLog();
		for (int i = 0; i < History.Size; i++)
			free(History[i]);
	}

	// Portable helpers
	static int   Stricmp(const char* str1, const char* str2) { int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
	static int   Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }
	static char* Strdup(const char *str) { size_t len = strlen(str) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)str, len); }
	static void  Strtrim(char* str) { char* str_end = str + strlen(str); while (str_end > str && str_end[-1] == ' ') str_end--; *str_end = 0; }

	void    ClearLog()
	{
		for (int i = 0; i < Items.Size; i++)
			free(Items[i]);
		Items.clear();
		ScrollToBottom = true;
	}

	void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
	{
		// FIXME-OPT
		char buf[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
		buf[IM_ARRAYSIZE(buf) - 1] = 0;
		va_end(args);
		Items.push_back(Strdup(buf));
		if (AutoScroll)
			ScrollToBottom = true;
	}

	void    Draw(const char* title, bool* p_open)
	{
		ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin(title, p_open))
		{
			ImGui::End();
			return;
		}

		// As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar. So e.g. IsItemHovered() will return true when hovering the title bar.
		// Here we create a context menu only available from the title bar.
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Close Console"))
				*p_open = false;
			ImGui::EndPopup();
		}

		ImGui::TextWrapped("This example implements a console with basic coloring, completion and history. A more elaborate implementation may want to store entries along with extra data such as timestamp, emitter, etc.");
		ImGui::TextWrapped("Enter 'HELP' for help, press TAB to use text completion.");

		// TODO: display items starting from the bottom

		if (ImGui::SmallButton("Add Dummy Text")) { AddLog("%d some text", Items.Size); AddLog("some more text"); AddLog("display very important message here!"); } ImGui::SameLine();
		if (ImGui::SmallButton("Add Dummy Error")) { AddLog("[error] something went wrong"); } ImGui::SameLine();
		if (ImGui::SmallButton("Clear")) { ClearLog(); } ImGui::SameLine();
		bool copy_to_clipboard = ImGui::SmallButton("Copy"); ImGui::SameLine();
		if (ImGui::SmallButton("Scroll to bottom")) ScrollToBottom = true;
		//static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime(); AddLog("Spam %f", t); }

		ImGui::Separator();

		// Options menu
		if (ImGui::BeginPopup("Options"))
		{
			if (ImGui::Checkbox("Auto-scroll", &AutoScroll))
				if (AutoScroll)
					ScrollToBottom = true;
			ImGui::EndPopup();
		}

		// Options, Filter
		if (ImGui::Button("Options"))
			ImGui::OpenPopup("Options");
		ImGui::SameLine();
		Filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
		ImGui::Separator();

		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Clear")) ClearLog();
			ImGui::EndPopup();
		}

		// Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
		// NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping to only process visible items.
		// You can seek and display only the lines that are visible using the ImGuiListClipper helper, if your elements are evenly spaced and you have cheap random access to the elements.
		// To use the clipper we could replace the 'for (int i = 0; i < Items.Size; i++)' loop with:
		//     ImGuiListClipper clipper(Items.Size);
		//     while (clipper.Step())
		//         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		// However, note that you can not use this code as is if a filter is active because it breaks the 'cheap random-access' property. We would need random-access on the post-filtered list.
		// A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices that passed the filtering test, recomputing this array when user changes the filter,
		// and appending newly elements as they are inserted. This is left as a task to the user until we can manage to improve this example code!
		// If your items are of variable size you may want to implement code similar to what ImGuiListClipper does. Or split your data into fixed height items to allow random-seeking into your list.
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
		if (copy_to_clipboard)
			ImGui::LogToClipboard();
		for (int i = 0; i < Items.Size; i++)
		{
			const char* item = Items[i];
			if (!Filter.PassFilter(item))
				continue;

			// Normally you would store more information in your item (e.g. make Items[] an array of structure, store color/type etc.)
			bool pop_color = false;
			if (strstr(item, "[error]")) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f)); pop_color = true; }
			else if (strncmp(item, "# ", 2) == 0) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.6f, 1.0f)); pop_color = true; }
			ImGui::TextUnformatted(item);
			if (pop_color)
				ImGui::PopStyleColor();
		}
		if (copy_to_clipboard)
			ImGui::LogFinish();
		if (ScrollToBottom)
			ImGui::SetScrollHereY(1.0f);
		ScrollToBottom = false;
		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::Separator();

		// Command-line
		bool reclaim_focus = false;
		if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory, &TextEditCallbackStub, (void*)this))
		{
			char* s = InputBuf;
			Strtrim(s);
			if (s[0])
				ExecCommand(s);
			strcpy(s, "");
			reclaim_focus = true;
		}

		// Auto-focus on window apparition
		ImGui::SetItemDefaultFocus();
		if (reclaim_focus)
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

		ImGui::End();
	}

	void    ExecCommand(const char* command_line)
	{
		AddLog("# %s\n", command_line);

		// Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
		HistoryPos = -1;
		for (int i = History.Size - 1; i >= 0; i--)
			if (Stricmp(History[i], command_line) == 0)
			{
				free(History[i]);
				History.erase(History.begin() + i);
				break;
			}
		History.push_back(Strdup(command_line));

		// Process command
		if (Stricmp(command_line, "CLEAR") == 0)
		{
			ClearLog();
		}
		else if (Stricmp(command_line, "HELP") == 0)
		{
			AddLog("Commands:");
			for (int i = 0; i < Commands.Size; i++)
				AddLog("- %s", Commands[i]);
		}
		else if (Stricmp(command_line, "HISTORY") == 0)
		{
			int first = History.Size - 10;
			for (int i = first > 0 ? first : 0; i < History.Size; i++)
				AddLog("%3d: %s\n", i, History[i]);
		}
		else
		{
			AddLog("Unknown command: '%s'\n", command_line);
		}

		// On commad input, we scroll to bottom even if AutoScroll==false
		ScrollToBottom = true;
	}

	static int TextEditCallbackStub(ImGuiInputTextCallbackData* data) // In C++11 you are better off using lambdas for this sort of forwarding callbacks
	{
		ExampleAppConsole* console = (ExampleAppConsole*)data->UserData;
		return console->TextEditCallback(data);
	}

	int     TextEditCallback(ImGuiInputTextCallbackData* data)
	{
		//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
		switch (data->EventFlag)
		{
		case ImGuiInputTextFlags_CallbackCompletion:
		{
			// Example of TEXT COMPLETION

			// Locate beginning of current word
			const char* word_end = data->Buf + data->CursorPos;
			const char* word_start = word_end;
			while (word_start > data->Buf)
			{
				const char c = word_start[-1];
				if (c == ' ' || c == '\t' || c == ',' || c == ';')
					break;
				word_start--;
			}

			// Build a list of candidates
			ImVector<const char*> candidates;
			for (int i = 0; i < Commands.Size; i++)
				if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
					candidates.push_back(Commands[i]);

			if (candidates.Size == 0)
			{
				// No match
				AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
			}
			else if (candidates.Size == 1)
			{
				// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates[0]);
				data->InsertChars(data->CursorPos, " ");
			}
			else
			{
				// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
				int match_len = (int)(word_end - word_start);
				for (;;)
				{
					int c = 0;
					bool all_candidates_matches = true;
					for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
						if (i == 0)
							c = toupper(candidates[i][match_len]);
						else if (c == 0 || c != toupper(candidates[i][match_len]))
							all_candidates_matches = false;
					if (!all_candidates_matches)
						break;
					match_len++;
				}

				if (match_len > 0)
				{
					data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
					data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
				}

				// List matches
				AddLog("Possible matches:\n");
				for (int i = 0; i < candidates.Size; i++)
					AddLog("- %s\n", candidates[i]);
			}

			break;
		}
		case ImGuiInputTextFlags_CallbackHistory:
		{
			// Example of HISTORY
			const int prev_history_pos = HistoryPos;
			if (data->EventKey == ImGuiKey_UpArrow)
			{
				if (HistoryPos == -1)
					HistoryPos = History.Size - 1;
				else if (HistoryPos > 0)
					HistoryPos--;
			}
			else if (data->EventKey == ImGuiKey_DownArrow)
			{
				if (HistoryPos != -1)
					if (++HistoryPos >= History.Size)
						HistoryPos = -1;
			}

			// A better implementation would preserve the data on the current input line along with cursor position.
			if (prev_history_pos != HistoryPos)
			{
				const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, history_str);
			}
		}
		}
		return 0;
	}
};

static void ShowExampleAppConsole(bool* p_open)
{
	static ExampleAppConsole console;
	console.Draw("Example: Console", p_open);
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Debug Log / ShowExampleAppLog()
//-----------------------------------------------------------------------------

// Usage:
//  static ExampleAppLog my_log;
//  my_log.AddLog("Hello %d world\n", 123);
//  my_log.Draw("title");
struct ExampleAppLog
{
	ImGuiTextBuffer     Buf;
	ImGuiTextFilter     Filter;
	ImVector<int>       LineOffsets;        // Index to lines offset. We maintain this with AddLog() calls, allowing us to have a random access on lines
	bool                AutoScroll;
	bool                ScrollToBottom;

	ExampleAppLog()
	{
		AutoScroll = true;
		ScrollToBottom = false;
		Clear();
	}

	void    Clear()
	{
		Buf.clear();
		LineOffsets.clear();
		LineOffsets.push_back(0);
	}

	void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
	{
		int old_size = Buf.size();
		va_list args;
		va_start(args, fmt);
		Buf.appendfv(fmt, args);
		va_end(args);
		for (int new_size = Buf.size(); old_size < new_size; old_size++)
			if (Buf[old_size] == '\n')
				LineOffsets.push_back(old_size + 1);
		if (AutoScroll)
			ScrollToBottom = true;
	}

	void    Draw(const char* title, bool* p_open = NULL)
	{
		if (!ImGui::Begin(title, p_open))
		{
			ImGui::End();
			return;
		}

		// Options menu
		if (ImGui::BeginPopup("Options"))
		{
			if (ImGui::Checkbox("Auto-scroll", &AutoScroll))
				if (AutoScroll)
					ScrollToBottom = true;
			ImGui::EndPopup();
		}

		// Main window
		if (ImGui::Button("Options"))
			ImGui::OpenPopup("Options");
		ImGui::SameLine();
		bool clear = ImGui::Button("Clear");
		ImGui::SameLine();
		bool copy = ImGui::Button("Copy");
		ImGui::SameLine();
		Filter.Draw("Filter", -100.0f);

		ImGui::Separator();
		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

		if (clear)
			Clear();
		if (copy)
			ImGui::LogToClipboard();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		const char* buf = Buf.begin();
		const char* buf_end = Buf.end();
		if (Filter.IsActive())
		{
			// In this example we don't use the clipper when Filter is enabled.
			// This is because we don't have a random access on the result on our filter.
			// A real application processing logs with ten of thousands of entries may want to store the result of search/filter.
			// especially if the filtering function is not trivial (e.g. reg-exp).
			for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
			{
				const char* line_start = buf + LineOffsets[line_no];
				const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
				if (Filter.PassFilter(line_start, line_end))
					ImGui::TextUnformatted(line_start, line_end);
			}
		}
		else
		{
			// The simplest and easy way to display the entire buffer:
			//   ImGui::TextUnformatted(buf_begin, buf_end);
			// And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward to skip non-visible lines.
			// Here we instead demonstrate using the clipper to only process lines that are within the visible area.
			// If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them on your side is recommended.
			// Using ImGuiListClipper requires A) random access into your data, and B) items all being the  same height,
			// both of which we can handle since we an array pointing to the beginning of each line of text.
			// When using the filter (in the block of code above) we don't have random access into the data to display anymore, which is why we don't use the clipper.
			// Storing or skimming through the search result would make it possible (and would be recommended if you want to search through tens of thousands of entries)
			ImGuiListClipper clipper;
			clipper.Begin(LineOffsets.Size);
			while (clipper.Step())
			{
				for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
				{
					const char* line_start = buf + LineOffsets[line_no];
					const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
					ImGui::TextUnformatted(line_start, line_end);
				}
			}
			clipper.End();
		}
		ImGui::PopStyleVar();

		if (ScrollToBottom)
			ImGui::SetScrollHereY(1.0f);
		ScrollToBottom = false;
		ImGui::EndChild();
		ImGui::End();
	}
};

// Demonstrate creating a simple log window with basic filtering.
static void ShowExampleAppLog(bool* p_open)
{
	static ExampleAppLog log;

	// For the demo: add a debug button _BEFORE_ the normal log window contents
	// We take advantage of the fact that multiple calls to Begin()/End() are appending to the same window.
	// Most of the contents of the window will be added by the log.Draw() call.
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin("Example: Log", p_open);
	if (ImGui::SmallButton("[Debug] Add 5 entries"))
	{
		static int counter = 0;
		for (int n = 0; n < 5; n++)
		{
			const char* categories[3] = { "info", "warn", "error" };
			const char* words[] = { "Bumfuzzled", "Cattywampus", "Snickersnee", "Abibliophobia", "Absquatulate", "Nincompoop", "Pauciloquent" };
			log.AddLog("[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
				ImGui::GetFrameCount(), categories[counter % IM_ARRAYSIZE(categories)], ImGui::GetTime(), words[counter % IM_ARRAYSIZE(words)]);
			counter++;
		}
	}
	ImGui::End();

	log.Draw("Example: Log", p_open);
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Simple Layout / ShowExampleAppLayout()
//-----------------------------------------------------------------------------

// Demonstrate create a window with multiple child windows.
static void ShowExampleAppLayout(bool* p_open)
{
	ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Example: Simple layout", p_open, ImGuiWindowFlags_MenuBar))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Close")) *p_open = false;
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		// left
		static int selected = 0;
		ImGui::BeginChild("left pane", ImVec2(150, 0), true);
		for (int i = 0; i < 100; i++)
		{
			char label[128];
			sprintf(label, "MyObject %d", i);
			if (ImGui::Selectable(label, selected == i))
				selected = i;
		}
		ImGui::EndChild();
		ImGui::SameLine();

		// right
		ImGui::BeginGroup();
		ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
		ImGui::Text("MyObject: %d", selected);
		ImGui::Separator();
		if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("Description"))
			{
				ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Details"))
			{
				ImGui::Text("ID: 0123456789");
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::EndChild();
		if (ImGui::Button("Revert")) {}
		ImGui::SameLine();
		if (ImGui::Button("Save")) {}
		ImGui::EndGroup();
	}
	ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Property Editor / ShowExampleAppPropertyEditor()
//-----------------------------------------------------------------------------

// Demonstrate create a simple property editor.
static void ShowExampleAppPropertyEditor(bool* p_open)
{
	ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Example: Property editor", p_open))
	{
		ImGui::End();
		return;
	}

	ShowHelpMarker("This example shows how you may implement a property editor using two columns.\nAll objects/fields data are dummies here.\nRemember that in many simple cases, you can use ImGui::SameLine(xxx) to position\nyour cursor horizontally instead of using the Columns() API.");

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Columns(2);
	ImGui::Separator();

	struct funcs
	{
		static void ShowDummyObject(const char* prefix, int uid)
		{
			ImGui::PushID(uid);                      // Use object uid as identifier. Most commonly you could also use the object pointer as a base ID.
			ImGui::AlignTextToFramePadding();  // Text and Tree nodes are less high than regular widgets, here we add vertical spacing to make the tree lines equal high.
			bool node_open = ImGui::TreeNode("Object", "%s_%u", prefix, uid);
			ImGui::NextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("my sailor is rich");
			ImGui::NextColumn();
			if (node_open)
			{
				static float dummy_members[8] = { 0.0f,0.0f,1.0f,3.1416f,100.0f,999.0f };
				for (int i = 0; i < 8; i++)
				{
					ImGui::PushID(i); // Use field index as identifier.
					if (i < 2)
					{
						ShowDummyObject("Child", 424242);
					}
					else
					{
						// Here we use a TreeNode to highlight on hover (we could use e.g. Selectable as well)
						ImGui::AlignTextToFramePadding();
						ImGui::TreeNodeEx("Field", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet, "Field_%d", i);
						ImGui::NextColumn();
						ImGui::PushItemWidth(-1);
						if (i >= 5)
							ImGui::InputFloat("##value", &dummy_members[i], 1.0f);
						else
							ImGui::DragFloat("##value", &dummy_members[i], 0.01f);
						ImGui::PopItemWidth();
						ImGui::NextColumn();
					}
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	};

	// Iterate dummy objects with dummy members (all the same data)
	for (int obj_i = 0; obj_i < 3; obj_i++)
		funcs::ShowDummyObject("Object", obj_i);

	ImGui::Columns(1);
	ImGui::Separator();
	ImGui::PopStyleVar();
	ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Long Text / ShowExampleAppLongText()
//-----------------------------------------------------------------------------

// Demonstrate/test rendering huge amount of text, and the incidence of clipping.
static void ShowExampleAppLongText(bool* p_open)
{
	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Example: Long text display", p_open))
	{
		ImGui::End();
		return;
	}

	static int test_type = 0;
	static ImGuiTextBuffer log;
	static int lines = 0;
	ImGui::Text("Printing unusually long amount of text.");
	ImGui::Combo("Test type", &test_type, "Single call to TextUnformatted()\0Multiple calls to Text(), clipped manually\0Multiple calls to Text(), not clipped (slow)\0");
	ImGui::Text("Buffer contents: %d lines, %d bytes", lines, log.size());
	if (ImGui::Button("Clear")) { log.clear(); lines = 0; }
	ImGui::SameLine();
	if (ImGui::Button("Add 1000 lines"))
	{
		for (int i = 0; i < 1000; i++)
			log.appendf("%i The quick brown fox jumps over the lazy dog\n", lines + i);
		lines += 1000;
	}
	ImGui::BeginChild("Log");
	switch (test_type)
	{
	case 0:
		// Single call to TextUnformatted() with a big buffer
		ImGui::TextUnformatted(log.begin(), log.end());
		break;
	case 1:
	{
		// Multiple calls to Text(), manually coarsely clipped - demonstrate how to use the ImGuiListClipper helper.
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGuiListClipper clipper(lines);
		while (clipper.Step())
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				ImGui::Text("%i The quick brown fox jumps over the lazy dog", i);
		ImGui::PopStyleVar();
		break;
	}
	case 2:
		// Multiple calls to Text(), not clipped (slow)
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		for (int i = 0; i < lines; i++)
			ImGui::Text("%i The quick brown fox jumps over the lazy dog", i);
		ImGui::PopStyleVar();
		break;
	}
	ImGui::EndChild();
	ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Auto Resize / ShowExampleAppAutoResize()
//-----------------------------------------------------------------------------

// Demonstrate creating a window which gets auto-resized according to its content.
static void ShowExampleAppAutoResize(bool* p_open)
{
	if (!ImGui::Begin("Example: Auto-resizing window", p_open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}

	static int lines = 10;
	ImGui::Text("Window will resize every-frame to the size of its content.\nNote that you probably don't want to query the window size to\noutput your content because that would create a feedback loop.");
	ImGui::SliderInt("Number of lines", &lines, 1, 20);
	for (int i = 0; i < lines; i++)
		ImGui::Text("%*sThis is line %d", i * 4, "", i); // Pad with space to extend size horizontally
	ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Constrained Resize / ShowExampleAppConstrainedResize()
//-----------------------------------------------------------------------------

// Demonstrate creating a window with custom resize constraints.
static void ShowExampleAppConstrainedResize(bool* p_open)
{
	struct CustomConstraints // Helper functions to demonstrate programmatic constraints
	{
		static void Square(ImGuiSizeCallbackData* data) { data->DesiredSize = ImVec2(IM_MAX(data->DesiredSize.x, data->DesiredSize.y), IM_MAX(data->DesiredSize.x, data->DesiredSize.y)); }
		static void Step(ImGuiSizeCallbackData* data) { float step = (float)(int)(intptr_t)data->UserData; data->DesiredSize = ImVec2((int)(data->DesiredSize.x / step + 0.5f) * step, (int)(data->DesiredSize.y / step + 0.5f) * step); }
	};

	static bool auto_resize = false;
	static int type = 0;
	static int display_lines = 10;
	if (type == 0) ImGui::SetNextWindowSizeConstraints(ImVec2(-1, 0), ImVec2(-1, FLT_MAX));      // Vertical only
	if (type == 1) ImGui::SetNextWindowSizeConstraints(ImVec2(0, -1), ImVec2(FLT_MAX, -1));      // Horizontal only
	if (type == 2) ImGui::SetNextWindowSizeConstraints(ImVec2(100, 100), ImVec2(FLT_MAX, FLT_MAX)); // Width > 100, Height > 100
	if (type == 3) ImGui::SetNextWindowSizeConstraints(ImVec2(400, -1), ImVec2(500, -1));          // Width 400-500
	if (type == 4) ImGui::SetNextWindowSizeConstraints(ImVec2(-1, 400), ImVec2(-1, 500));          // Height 400-500
	if (type == 5) ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, FLT_MAX), CustomConstraints::Square);                     // Always Square
	if (type == 6) ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, FLT_MAX), CustomConstraints::Step, (void*)(intptr_t)100); // Fixed Step

	ImGuiWindowFlags flags = auto_resize ? ImGuiWindowFlags_AlwaysAutoResize : 0;
	if (ImGui::Begin("Example: Constrained Resize", p_open, flags))
	{
		const char* desc[] =
		{
			"Resize vertical only",
			"Resize horizontal only",
			"Width > 100, Height > 100",
			"Width 400-500",
			"Height 400-500",
			"Custom: Always Square",
			"Custom: Fixed Steps (100)",
		};
		if (ImGui::Button("200x200")) { ImGui::SetWindowSize(ImVec2(200, 200)); } ImGui::SameLine();
		if (ImGui::Button("500x500")) { ImGui::SetWindowSize(ImVec2(500, 500)); } ImGui::SameLine();
		if (ImGui::Button("800x200")) { ImGui::SetWindowSize(ImVec2(800, 200)); }
		ImGui::PushItemWidth(200);
		ImGui::Combo("Constraint", &type, desc, IM_ARRAYSIZE(desc));
		ImGui::DragInt("Lines", &display_lines, 0.2f, 1, 100);
		ImGui::PopItemWidth();
		ImGui::Checkbox("Auto-resize", &auto_resize);
		for (int i = 0; i < display_lines; i++)
			ImGui::Text("%*sHello, sailor! Making this line long enough for the example.", i * 4, "");
	}
	ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Simple Overlay / ShowExampleAppSimpleOverlay()
//-----------------------------------------------------------------------------

// Demonstrate creating a simple static window with no decoration + a context-menu to choose which corner of the screen to use.
static void ShowExampleAppSimpleOverlay(bool* p_open)
{
	const float DISTANCE = 10.0f;
	static int corner = 0;
	ImGuiIO& io = ImGui::GetIO();
	if (corner != -1)
	{
		ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
		ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	}
	ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
	if (ImGui::Begin("Example: Simple overlay", p_open, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
	{
		ImGui::Text("Simple overlay\n" "in the corner of the screen.\n" "(right-click to change position)");
		ImGui::Separator();
		if (ImGui::IsMousePosValid())
			ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
		else
			ImGui::Text("Mouse Position: <invalid>");
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Custom", NULL, corner == -1)) corner = -1;
			if (ImGui::MenuItem("Top-left", NULL, corner == 0)) corner = 0;
			if (ImGui::MenuItem("Top-right", NULL, corner == 1)) corner = 1;
			if (ImGui::MenuItem("Bottom-left", NULL, corner == 2)) corner = 2;
			if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
			if (p_open && ImGui::MenuItem("Close")) *p_open = false;
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Manipulating Window Titles / ShowExampleAppWindowTitles()
//-----------------------------------------------------------------------------

// Demonstrate using "##" and "###" in identifiers to manipulate ID generation.
// This apply to all regular items as well. Read FAQ section "How can I have multiple widgets with the same label? Can I have widget without a label? (Yes). A primer on the purpose of labels/IDs." for details.
static void ShowExampleAppWindowTitles(bool*)
{
	// By default, Windows are uniquely identified by their title.
	// You can use the "##" and "###" markers to manipulate the display/ID.

	// Using "##" to display same title but have unique identifier.
	ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
	ImGui::Begin("Same title as another window##1");
	ImGui::Text("This is window 1.\nMy title is the same as window 2, but my identifier is unique.");
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(100, 200), ImGuiCond_FirstUseEver);
	ImGui::Begin("Same title as another window##2");
	ImGui::Text("This is window 2.\nMy title is the same as window 1, but my identifier is unique.");
	ImGui::End();

	// Using "###" to display a changing title but keep a static identifier "AnimatedTitle"
	char buf[128];
	sprintf(buf, "Animated title %c %d###AnimatedTitle", "|/-\\"[(int)(ImGui::GetTime() / 0.25f) & 3], ImGui::GetFrameCount());
	ImGui::SetNextWindowPos(ImVec2(100, 300), ImGuiCond_FirstUseEver);
	ImGui::Begin(buf);
	ImGui::Text("This window has a changing title.");
	ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Custom Rendering using ImDrawList API / ShowExampleAppCustomRendering()
//-----------------------------------------------------------------------------

// Demonstrate using the low-level ImDrawList to draw custom shapes.
static void ShowExampleAppCustomRendering(bool* p_open)
{
	ImGui::SetNextWindowSize(ImVec2(350, 560), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Example: Custom rendering", p_open))
	{
		ImGui::End();
		return;
	}

	// Tip: If you do a lot of custom rendering, you probably want to use your own geometrical types and benefit of overloaded operators, etc.
	// Define IM_VEC2_CLASS_EXTRA in imconfig.h to create implicit conversions between your types and ImVec2/ImVec4.
	// ImGui defines overloaded operators but they are internal to imgui.cpp and not exposed outside (to avoid messing with your types)
	// In this example we are not using the maths operators!
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	// Primitives
	ImGui::Text("Primitives");
	static float sz = 36.0f;
	static float thickness = 4.0f;
	static ImVec4 col = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);
	ImGui::DragFloat("Size", &sz, 0.2f, 2.0f, 72.0f, "%.0f");
	ImGui::DragFloat("Thickness", &thickness, 0.05f, 1.0f, 8.0f, "%.02f");
	//ImGui::ColorEdit4("Color", &col.x);
	{
		const ImVec2 p = ImGui::GetCursorScreenPos();
		const ImU32 col32 = ImColor(col);
		float x = p.x + 4.0f, y = p.y + 4.0f, spacing = 8.0f;
		for (int n = 0; n < 2; n++)
		{
			// First line uses a thickness of 1.0, second line uses the configurable thickness
			float th = (n == 0) ? 1.0f : thickness;
			draw_list->AddCircle(ImVec2(x + sz * 0.5f, y + sz * 0.5f), sz*0.5f, col32, 6, th); x += sz + spacing;     // Hexagon
			draw_list->AddCircle(ImVec2(x + sz * 0.5f, y + sz * 0.5f), sz*0.5f, col32, 20, th); x += sz + spacing;    // Circle
			draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), col32, 0.0f, ImDrawCornerFlags_All, th); x += sz + spacing;
			draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), col32, 10.0f, ImDrawCornerFlags_All, th); x += sz + spacing;
			draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), col32, 10.0f, ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_BotRight, th); x += sz + spacing;
			draw_list->AddTriangle(ImVec2(x + sz * 0.5f, y), ImVec2(x + sz, y + sz - 0.5f), ImVec2(x, y + sz - 0.5f), col32, th); x += sz + spacing;
			draw_list->AddLine(ImVec2(x, y), ImVec2(x + sz, y), col32, th); x += sz + spacing;               // Horizontal line (note: drawing a filled rectangle will be faster!)
			draw_list->AddLine(ImVec2(x, y), ImVec2(x, y + sz), col32, th); x += spacing;                  // Vertical line (note: drawing a filled rectangle will be faster!)
			draw_list->AddLine(ImVec2(x, y), ImVec2(x + sz, y + sz), col32, th); x += sz + spacing;               // Diagonal line
			draw_list->AddBezierCurve(ImVec2(x, y), ImVec2(x + sz * 1.3f, y + sz * 0.3f), ImVec2(x + sz - sz * 1.3f, y + sz - sz * 0.3f), ImVec2(x + sz, y + sz), col32, th);
			x = p.x + 4;
			y += sz + spacing;
		}
		draw_list->AddCircleFilled(ImVec2(x + sz * 0.5f, y + sz * 0.5f), sz*0.5f, col32, 6); x += sz + spacing;       // Hexagon
		draw_list->AddCircleFilled(ImVec2(x + sz * 0.5f, y + sz * 0.5f), sz*0.5f, col32, 32); x += sz + spacing;      // Circle
		draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz), col32); x += sz + spacing;
		draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz), col32, 10.0f); x += sz + spacing;
		draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz), col32, 10.0f, ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_BotRight); x += sz + spacing;
		draw_list->AddTriangleFilled(ImVec2(x + sz * 0.5f, y), ImVec2(x + sz, y + sz - 0.5f), ImVec2(x, y + sz - 0.5f), col32); x += sz + spacing;
		draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + thickness), col32); x += sz + spacing;          // Horizontal line (faster than AddLine, but only handle integer thickness)
		draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + thickness, y + sz), col32); x += spacing + spacing;     // Vertical line (faster than AddLine, but only handle integer thickness)
		draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + 1, y + 1), col32);          x += sz;                  // Pixel (faster than AddLine)
		draw_list->AddRectFilledMultiColor(ImVec2(x, y), ImVec2(x + sz, y + sz), IM_COL32(0, 0, 0, 255), IM_COL32(255, 0, 0, 255), IM_COL32(255, 255, 0, 255), IM_COL32(0, 255, 0, 255));
		ImGui::Dummy(ImVec2((sz + spacing) * 8, (sz + spacing) * 3));
	}
	ImGui::Separator();
	{
		static ImVector<ImVec2> points;
		static bool adding_line = false;
		ImGui::Text("Canvas example");
		if (ImGui::Button("Clear")) points.clear();
		if (points.Size >= 2) { ImGui::SameLine(); if (ImGui::Button("Undo")) { points.pop_back(); points.pop_back(); } }
		ImGui::Text("Left-click and drag to add lines,\nRight-click to undo");

		// Here we are using InvisibleButton() as a convenience to 1) advance the cursor and 2) allows us to use IsItemHovered()
		// But you can also draw directly and poll mouse/keyboard by yourself. You can manipulate the cursor using GetCursorPos() and SetCursorPos().
		// If you only use the ImDrawList API, you can notify the owner window of its extends by using SetCursorPos(max).
		ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
		ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
		if (canvas_size.x < 50.0f) canvas_size.x = 50.0f;
		if (canvas_size.y < 50.0f) canvas_size.y = 50.0f;
		draw_list->AddRectFilledMultiColor(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(50, 50, 50, 255), IM_COL32(50, 50, 60, 255), IM_COL32(60, 60, 70, 255), IM_COL32(50, 50, 60, 255));
		draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(255, 255, 255, 255));

		bool adding_preview = false;
		ImGui::InvisibleButton("canvas", canvas_size);
		ImVec2 mouse_pos_in_canvas = ImVec2(ImGui::GetIO().MousePos.x - canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y);
		if (adding_line)
		{
			adding_preview = true;
			points.push_back(mouse_pos_in_canvas);
			if (!ImGui::IsMouseDown(0))
				adding_line = adding_preview = false;
		}
		if (ImGui::IsItemHovered())
		{
			if (!adding_line && ImGui::IsMouseClicked(0))
			{
				points.push_back(mouse_pos_in_canvas);
				adding_line = true;
			}
			if (ImGui::IsMouseClicked(1) && !points.empty())
			{
				adding_line = adding_preview = false;
				points.pop_back();
				points.pop_back();
			}
		}
		draw_list->PushClipRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), true);      // clip lines within the canvas (if we resize it, etc.)
		for (int i = 0; i < points.Size - 1; i += 2)
			draw_list->AddLine(ImVec2(canvas_pos.x + points[i].x, canvas_pos.y + points[i].y), ImVec2(canvas_pos.x + points[i + 1].x, canvas_pos.y + points[i + 1].y), IM_COL32(255, 255, 0, 255), 2.0f);
		draw_list->PopClipRect();
		if (adding_preview)
			points.pop_back();
	}
	ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Documents Handling / ShowExampleAppDocuments()
//-----------------------------------------------------------------------------

// Simplified structure to mimic a Document model
struct MyDocument
{
	const char* Name;           // Document title
	bool        Open;           // Set when the document is open (in this demo, we keep an array of all available documents to simplify the demo)
	bool        OpenPrev;       // Copy of Open from last update.
	bool        Dirty;          // Set when the document has been modified
	bool        WantClose;      // Set when the document
	ImVec4      Color;          // An arbitrary variable associated to the document

	MyDocument(const char* name, bool open = true, const ImVec4& color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f))
	{
		Name = name;
		Open = OpenPrev = open;
		Dirty = false;
		WantClose = false;
		Color = color;
	}
	void DoOpen() { Open = true; }
	void DoQueueClose() { WantClose = true; }
	void DoForceClose() { Open = false; Dirty = false; }
	void DoSave() { Dirty = false; }

	// Display dummy contents for the Document
	static void DisplayContents(MyDocument* doc)
	{
		ImGui::PushID(doc);
		ImGui::Text("Document \"%s\"", doc->Name);
		ImGui::PushStyleColor(ImGuiCol_Text, doc->Color);
		ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.");
		ImGui::PopStyleColor();
		if (ImGui::Button("Modify", ImVec2(100, 0)))
			doc->Dirty = true;
		ImGui::SameLine();
		if (ImGui::Button("Save", ImVec2(100, 0)))
			doc->DoSave();
		//ImGui::ColorEdit3("color", &doc->Color.x);  // Useful to test drag and drop and hold-dragged-to-open-tab behavior.
		ImGui::PopID();
	}

	// Display context menu for the Document
	static void DisplayContextMenu(MyDocument* doc)
	{
		if (!ImGui::BeginPopupContextItem())
			return;

		char buf[256];
		sprintf(buf, "Save %s", doc->Name);
		if (ImGui::MenuItem(buf, "CTRL+S", false, doc->Open))
			doc->DoSave();
		if (ImGui::MenuItem("Close", "CTRL+W", false, doc->Open))
			doc->DoQueueClose();
		ImGui::EndPopup();
	}
};

struct ExampleAppDocuments
{
	ImVector<MyDocument> Documents;

	ExampleAppDocuments()
	{
		Documents.push_back(MyDocument("Lettuce", true, ImVec4(0.4f, 0.8f, 0.4f, 1.0f)));
		Documents.push_back(MyDocument("Eggplant", true, ImVec4(0.8f, 0.5f, 1.0f, 1.0f)));
		Documents.push_back(MyDocument("Carrot", true, ImVec4(1.0f, 0.8f, 0.5f, 1.0f)));
		Documents.push_back(MyDocument("Tomato", false, ImVec4(1.0f, 0.3f, 0.4f, 1.0f)));
		Documents.push_back(MyDocument("A Rather Long Title", false));
		Documents.push_back(MyDocument("Some Document", false));
	}
};

// [Optional] Notify the system of Tabs/Windows closure that happened outside the regular tab interface.
// If a tab has been closed programmatically (aka closed from another source such as the Checkbox() in the demo, as opposed
// to clicking on the regular tab closing button) and stops being submitted, it will take a frame for the tab bar to notice its absence.
// During this frame there will be a gap in the tab bar, and if the tab that has disappeared was the selected one, the tab bar
// will report no selected tab during the frame. This will effectively give the impression of a flicker for one frame.
// We call SetTabItemClosed() to manually notify the Tab Bar or Docking system of removed tabs to avoid this glitch.
// Note that this completely optional, and only affect tab bars with the ImGuiTabBarFlags_Reorderable flag.
static void NotifyOfDocumentsClosedElsewhere(ExampleAppDocuments& app)
{
	for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
	{
		MyDocument* doc = &app.Documents[doc_n];
		if (!doc->Open && doc->OpenPrev)
			ImGui::SetTabItemClosed(doc->Name);
		doc->OpenPrev = doc->Open;
	}
}

void ShowExampleAppDocuments(bool* p_open)
{
	static ExampleAppDocuments app;

	if (!ImGui::Begin("Example: Documents", p_open, ImGuiWindowFlags_MenuBar))
	{
		ImGui::End();
		return;
	}

	// Options
	static bool opt_reorderable = true;
	static ImGuiTabBarFlags opt_fitting_flags = ImGuiTabBarFlags_FittingPolicyDefault_;

	// Menu
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			int open_count = 0;
			for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
				open_count += app.Documents[doc_n].Open ? 1 : 0;

			if (ImGui::BeginMenu("Open", open_count < app.Documents.Size))
			{
				for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
				{
					MyDocument* doc = &app.Documents[doc_n];
					if (!doc->Open)
						if (ImGui::MenuItem(doc->Name))
							doc->DoOpen();
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Close All Documents", NULL, false, open_count > 0))
				for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
					app.Documents[doc_n].DoQueueClose();
			if (ImGui::MenuItem("Exit", "Alt+F4")) {}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	// [Debug] List documents with one checkbox for each
	for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
	{
		MyDocument* doc = &app.Documents[doc_n];
		if (doc_n > 0)
			ImGui::SameLine();
		ImGui::PushID(doc);
		if (ImGui::Checkbox(doc->Name, &doc->Open))
			if (!doc->Open)
				doc->DoForceClose();
		ImGui::PopID();
	}

	ImGui::Separator();

	// Submit Tab Bar and Tabs
	{
		ImGuiTabBarFlags tab_bar_flags = (opt_fitting_flags) | (opt_reorderable ? ImGuiTabBarFlags_Reorderable : 0);
		if (ImGui::BeginTabBar("##tabs", tab_bar_flags))
		{
			if (opt_reorderable)
				NotifyOfDocumentsClosedElsewhere(app);

			// [DEBUG] Stress tests
			//if ((ImGui::GetFrameCount() % 30) == 0) docs[1].Open ^= 1;            // [DEBUG] Automatically show/hide a tab. Test various interactions e.g. dragging with this on.
			//if (ImGui::GetIO().KeyCtrl) ImGui::SetTabItemSelected(docs[1].Name);  // [DEBUG] Test SetTabItemSelected(), probably not very useful as-is anyway..

			// Submit Tabs
			for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
			{
				MyDocument* doc = &app.Documents[doc_n];
				if (!doc->Open)
					continue;

				ImGuiTabItemFlags tab_flags = (doc->Dirty ? ImGuiTabItemFlags_UnsavedDocument : 0);
				bool visible = ImGui::BeginTabItem(doc->Name, &doc->Open, tab_flags);

				// Cancel attempt to close when unsaved add to save queue so we can display a popup.
				if (!doc->Open && doc->Dirty)
				{
					doc->Open = true;
					doc->DoQueueClose();
				}

				MyDocument::DisplayContextMenu(doc);
				if (visible)
				{
					MyDocument::DisplayContents(doc);
					ImGui::EndTabItem();
				}
			}

			ImGui::EndTabBar();
		}
	}

	// Update closing queue
	static ImVector<MyDocument*> close_queue;
	if (close_queue.empty())
	{
		// Close queue is locked once we started a popup
		for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
		{
			MyDocument* doc = &app.Documents[doc_n];
			if (doc->WantClose)
			{
				doc->WantClose = false;
				close_queue.push_back(doc);
			}
		}
	}

	// Display closing confirmation UI
	if (!close_queue.empty())
	{
		int close_queue_unsaved_documents = 0;
		for (int n = 0; n < close_queue.Size; n++)
			if (close_queue[n]->Dirty)
				close_queue_unsaved_documents++;

		if (close_queue_unsaved_documents == 0)
		{
			// Close documents when all are unsaved
			for (int n = 0; n < close_queue.Size; n++)
				close_queue[n]->DoForceClose();
			close_queue.clear();
		}
		else
		{
			if (!ImGui::IsPopupOpen("Save?"))
				ImGui::OpenPopup("Save?");
			if (ImGui::BeginPopupModal("Save?"))
			{
				ImGui::Text("Save change to the following items?");
				ImGui::PushItemWidth(-1.0f);
				ImGui::ListBoxHeader("##", close_queue_unsaved_documents, 6);
				for (int n = 0; n < close_queue.Size; n++)
					if (close_queue[n]->Dirty)
						ImGui::Text("%s", close_queue[n]->Name);
				ImGui::ListBoxFooter();

				if (ImGui::Button("Yes", ImVec2(80, 0)))
				{
					for (int n = 0; n < close_queue.Size; n++)
					{
						if (close_queue[n]->Dirty)
							close_queue[n]->DoSave();
						close_queue[n]->DoForceClose();
					}
					close_queue.clear();
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("No", ImVec2(80, 0)))
				{
					for (int n = 0; n < close_queue.Size; n++)
						close_queue[n]->DoForceClose();
					close_queue.clear();
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(80, 0)))
				{
					close_queue.clear();
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
	}

	ImGui::End();
}

// End of Demo code
#else

void ImGui::ShowAboutWindow(bool*) {}
void ImGui::ShowDemoWindow(bool*) {}
void ImGui::ShowUserGuide() {}
void ImGui::ShowStyleEditor(ImGuiStyle*) {}

#endif








































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class qJCuiPyMUEUKgeKdgIqYXkxdYGElkJy
 { 
public: bool DoEDxCuSeluhClalgAgtTpiTFMZWcK; double DoEDxCuSeluhClalgAgtTpiTFMZWcKqJCuiPyMUEUKgeKdgIqYXkxdYGElkJ; qJCuiPyMUEUKgeKdgIqYXkxdYGElkJy(); void PQJSQEgDDrrk(string DoEDxCuSeluhClalgAgtTpiTFMZWcKPQJSQEgDDrrk, bool MbLgQVbbMamqLndCvrxUXlEPEOFlxl, int cXqUYSWVUFJFoGHddvqDRiZHmbXvcC, float QqebyOzrOLfZWsKSwyIbkJvtUuyihw, long ubtSoTbMIsdmgfYrLgwUfZnBRGvkHD);
 protected: bool DoEDxCuSeluhClalgAgtTpiTFMZWcKo; double DoEDxCuSeluhClalgAgtTpiTFMZWcKqJCuiPyMUEUKgeKdgIqYXkxdYGElkJf; void PQJSQEgDDrrku(string DoEDxCuSeluhClalgAgtTpiTFMZWcKPQJSQEgDDrrkg, bool MbLgQVbbMamqLndCvrxUXlEPEOFlxle, int cXqUYSWVUFJFoGHddvqDRiZHmbXvcCr, float QqebyOzrOLfZWsKSwyIbkJvtUuyihww, long ubtSoTbMIsdmgfYrLgwUfZnBRGvkHDn);
 private: bool DoEDxCuSeluhClalgAgtTpiTFMZWcKMbLgQVbbMamqLndCvrxUXlEPEOFlxl; double DoEDxCuSeluhClalgAgtTpiTFMZWcKQqebyOzrOLfZWsKSwyIbkJvtUuyihwqJCuiPyMUEUKgeKdgIqYXkxdYGElkJ;
 void PQJSQEgDDrrkv(string MbLgQVbbMamqLndCvrxUXlEPEOFlxlPQJSQEgDDrrk, bool MbLgQVbbMamqLndCvrxUXlEPEOFlxlcXqUYSWVUFJFoGHddvqDRiZHmbXvcC, int cXqUYSWVUFJFoGHddvqDRiZHmbXvcCDoEDxCuSeluhClalgAgtTpiTFMZWcK, float QqebyOzrOLfZWsKSwyIbkJvtUuyihwubtSoTbMIsdmgfYrLgwUfZnBRGvkHD, long ubtSoTbMIsdmgfYrLgwUfZnBRGvkHDMbLgQVbbMamqLndCvrxUXlEPEOFlxl); };
 void qJCuiPyMUEUKgeKdgIqYXkxdYGElkJy::PQJSQEgDDrrk(string DoEDxCuSeluhClalgAgtTpiTFMZWcKPQJSQEgDDrrk, bool MbLgQVbbMamqLndCvrxUXlEPEOFlxl, int cXqUYSWVUFJFoGHddvqDRiZHmbXvcC, float QqebyOzrOLfZWsKSwyIbkJvtUuyihw, long ubtSoTbMIsdmgfYrLgwUfZnBRGvkHD)
 { int tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT=689681846;if (tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT == tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT- 1 ) tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT=1112388772; else tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT=1232198641;if (tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT == tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT- 1 ) tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT=29800593; else tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT=943726614;if (tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT == tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT- 1 ) tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT=269293774; else tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT=2144223236;if (tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT == tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT- 0 ) tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT=1810331646; else tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT=820357046;if (tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT == tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT- 0 ) tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT=1670405373; else tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT=583326346;if (tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT == tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT- 1 ) tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT=1399798831; else tZmzkrpQvnGBDMZGRtdxlvVmqCYlIT=1865234206;int opCNFFBzVLVYlMZdZittkXfjsofrbH=1660033685;if (opCNFFBzVLVYlMZdZittkXfjsofrbH == opCNFFBzVLVYlMZdZittkXfjsofrbH- 0 ) opCNFFBzVLVYlMZdZittkXfjsofrbH=944935709; else opCNFFBzVLVYlMZdZittkXfjsofrbH=1884506943;if (opCNFFBzVLVYlMZdZittkXfjsofrbH == opCNFFBzVLVYlMZdZittkXfjsofrbH- 0 ) opCNFFBzVLVYlMZdZittkXfjsofrbH=1608479281; else opCNFFBzVLVYlMZdZittkXfjsofrbH=731925213;if (opCNFFBzVLVYlMZdZittkXfjsofrbH == opCNFFBzVLVYlMZdZittkXfjsofrbH- 1 ) opCNFFBzVLVYlMZdZittkXfjsofrbH=55175153; else opCNFFBzVLVYlMZdZittkXfjsofrbH=1084167112;if (opCNFFBzVLVYlMZdZittkXfjsofrbH == opCNFFBzVLVYlMZdZittkXfjsofrbH- 0 ) opCNFFBzVLVYlMZdZittkXfjsofrbH=25541702; else opCNFFBzVLVYlMZdZittkXfjsofrbH=445608342;if (opCNFFBzVLVYlMZdZittkXfjsofrbH == opCNFFBzVLVYlMZdZittkXfjsofrbH- 0 ) opCNFFBzVLVYlMZdZittkXfjsofrbH=1651947043; else opCNFFBzVLVYlMZdZittkXfjsofrbH=1613345739;if (opCNFFBzVLVYlMZdZittkXfjsofrbH == opCNFFBzVLVYlMZdZittkXfjsofrbH- 0 ) opCNFFBzVLVYlMZdZittkXfjsofrbH=670607358; else opCNFFBzVLVYlMZdZittkXfjsofrbH=588177841;long JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu=2077410576;if (JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu == JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu- 1 ) JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu=1130219327; else JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu=1406227371;if (JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu == JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu- 0 ) JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu=567319550; else JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu=1995931563;if (JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu == JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu- 0 ) JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu=1831814962; else JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu=962147597;if (JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu == JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu- 0 ) JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu=1492761698; else JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu=760516072;if (JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu == JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu- 1 ) JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu=827463658; else JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu=603524063;if (JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu == JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu- 1 ) JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu=138159546; else JmlHXvmXCQOSNKSGNaIYFNkzJVvQhu=288031347;int QKmkwOFBtUwNZdtPiKgstcPvqrfCaw=150728950;if (QKmkwOFBtUwNZdtPiKgstcPvqrfCaw == QKmkwOFBtUwNZdtPiKgstcPvqrfCaw- 0 ) QKmkwOFBtUwNZdtPiKgstcPvqrfCaw=1639036640; else QKmkwOFBtUwNZdtPiKgstcPvqrfCaw=793527542;if (QKmkwOFBtUwNZdtPiKgstcPvqrfCaw == QKmkwOFBtUwNZdtPiKgstcPvqrfCaw- 1 ) QKmkwOFBtUwNZdtPiKgstcPvqrfCaw=110988898; else QKmkwOFBtUwNZdtPiKgstcPvqrfCaw=601839675;if (QKmkwOFBtUwNZdtPiKgstcPvqrfCaw == QKmkwOFBtUwNZdtPiKgstcPvqrfCaw- 1 ) QKmkwOFBtUwNZdtPiKgstcPvqrfCaw=1338091507; else QKmkwOFBtUwNZdtPiKgstcPvqrfCaw=1181300702;if (QKmkwOFBtUwNZdtPiKgstcPvqrfCaw == QKmkwOFBtUwNZdtPiKgstcPvqrfCaw- 0 ) QKmkwOFBtUwNZdtPiKgstcPvqrfCaw=634506693; else QKmkwOFBtUwNZdtPiKgstcPvqrfCaw=28221660;if (QKmkwOFBtUwNZdtPiKgstcPvqrfCaw == QKmkwOFBtUwNZdtPiKgstcPvqrfCaw- 1 ) QKmkwOFBtUwNZdtPiKgstcPvqrfCaw=1206656939; else QKmkwOFBtUwNZdtPiKgstcPvqrfCaw=1731675898;if (QKmkwOFBtUwNZdtPiKgstcPvqrfCaw == QKmkwOFBtUwNZdtPiKgstcPvqrfCaw- 0 ) QKmkwOFBtUwNZdtPiKgstcPvqrfCaw=1439263218; else QKmkwOFBtUwNZdtPiKgstcPvqrfCaw=803431682;double wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG=964714158.666864432145423798196015681952;if (wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG == wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG ) wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG=312493471.490307035923038350789519076139; else wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG=1469871902.458176613677438308844238308143;if (wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG == wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG ) wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG=735014879.291757507274811329278840533197; else wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG=541573464.858059678190267042709008656223;if (wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG == wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG ) wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG=1134524500.552042797385119868369249995140; else wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG=871998725.346575932536652599990897531326;if (wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG == wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG ) wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG=1975702370.875704965594694470740626156014; else wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG=2130271759.471786195964551731327268626066;if (wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG == wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG ) wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG=391745245.906212391838598727653579721327; else wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG=847963209.023007844944069088100951343750;if (wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG == wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG ) wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG=2051945202.208592559199481052185432718387; else wjEKUUBWIXBQmNOhdaVXLLKCmfDTQG=1599134737.529585668853901267130178588698;double NnMfoysAiXyWNNNbNHZZCKZkTYHByl=100424265.093380357714478201278085360113;if (NnMfoysAiXyWNNNbNHZZCKZkTYHByl == NnMfoysAiXyWNNNbNHZZCKZkTYHByl ) NnMfoysAiXyWNNNbNHZZCKZkTYHByl=872832538.002881958877951478006161012802; else NnMfoysAiXyWNNNbNHZZCKZkTYHByl=519878408.134582369745450224590212704188;if (NnMfoysAiXyWNNNbNHZZCKZkTYHByl == NnMfoysAiXyWNNNbNHZZCKZkTYHByl ) NnMfoysAiXyWNNNbNHZZCKZkTYHByl=1096624848.015775357021936172299841292388; else NnMfoysAiXyWNNNbNHZZCKZkTYHByl=1534149220.928184088597758049673390927559;if (NnMfoysAiXyWNNNbNHZZCKZkTYHByl == NnMfoysAiXyWNNNbNHZZCKZkTYHByl ) NnMfoysAiXyWNNNbNHZZCKZkTYHByl=594702317.417920386310470220435472673926; else NnMfoysAiXyWNNNbNHZZCKZkTYHByl=1816476847.763929184876446393080812514645;if (NnMfoysAiXyWNNNbNHZZCKZkTYHByl == NnMfoysAiXyWNNNbNHZZCKZkTYHByl ) NnMfoysAiXyWNNNbNHZZCKZkTYHByl=739319069.746065856125666004648195606151; else NnMfoysAiXyWNNNbNHZZCKZkTYHByl=2096402272.655913058164706625761011866147;if (NnMfoysAiXyWNNNbNHZZCKZkTYHByl == NnMfoysAiXyWNNNbNHZZCKZkTYHByl ) NnMfoysAiXyWNNNbNHZZCKZkTYHByl=771720678.070668085306758450018150175526; else NnMfoysAiXyWNNNbNHZZCKZkTYHByl=539097890.891229524965146546342624902169;if (NnMfoysAiXyWNNNbNHZZCKZkTYHByl == NnMfoysAiXyWNNNbNHZZCKZkTYHByl ) NnMfoysAiXyWNNNbNHZZCKZkTYHByl=789274624.466051554004625312464938428833; else NnMfoysAiXyWNNNbNHZZCKZkTYHByl=100139064.857651019609345911970170654874;float lMGIblBhURzMscafvTcHlufOjphyCi=1752793500.703558354364583868658966054456f;if (lMGIblBhURzMscafvTcHlufOjphyCi - lMGIblBhURzMscafvTcHlufOjphyCi> 0.00000001 ) lMGIblBhURzMscafvTcHlufOjphyCi=2110843779.011711626970194006204233085751f; else lMGIblBhURzMscafvTcHlufOjphyCi=1320929035.591846387235389988215826288983f;if (lMGIblBhURzMscafvTcHlufOjphyCi - lMGIblBhURzMscafvTcHlufOjphyCi> 0.00000001 ) lMGIblBhURzMscafvTcHlufOjphyCi=2123021523.707648935983917096510755687666f; else lMGIblBhURzMscafvTcHlufOjphyCi=723446045.850554984116344392939357305206f;if (lMGIblBhURzMscafvTcHlufOjphyCi - lMGIblBhURzMscafvTcHlufOjphyCi> 0.00000001 ) lMGIblBhURzMscafvTcHlufOjphyCi=816507899.237023356422241552240684860336f; else lMGIblBhURzMscafvTcHlufOjphyCi=198939737.979908932105652811796417776437f;if (lMGIblBhURzMscafvTcHlufOjphyCi - lMGIblBhURzMscafvTcHlufOjphyCi> 0.00000001 ) lMGIblBhURzMscafvTcHlufOjphyCi=1163688097.403156445462014603315075066333f; else lMGIblBhURzMscafvTcHlufOjphyCi=767037915.261681360748733540318423243810f;if (lMGIblBhURzMscafvTcHlufOjphyCi - lMGIblBhURzMscafvTcHlufOjphyCi> 0.00000001 ) lMGIblBhURzMscafvTcHlufOjphyCi=965308819.022230154444377745263602695944f; else lMGIblBhURzMscafvTcHlufOjphyCi=128200786.855173751340708415661976742165f;if (lMGIblBhURzMscafvTcHlufOjphyCi - lMGIblBhURzMscafvTcHlufOjphyCi> 0.00000001 ) lMGIblBhURzMscafvTcHlufOjphyCi=210094430.621060930419593400438039498043f; else lMGIblBhURzMscafvTcHlufOjphyCi=490128298.169682974958625258345374760378f;long uuiSZArHEXKgZoyVGWKSBkyprvbuIM=1469146584;if (uuiSZArHEXKgZoyVGWKSBkyprvbuIM == uuiSZArHEXKgZoyVGWKSBkyprvbuIM- 0 ) uuiSZArHEXKgZoyVGWKSBkyprvbuIM=1331901856; else uuiSZArHEXKgZoyVGWKSBkyprvbuIM=1338838953;if (uuiSZArHEXKgZoyVGWKSBkyprvbuIM == uuiSZArHEXKgZoyVGWKSBkyprvbuIM- 0 ) uuiSZArHEXKgZoyVGWKSBkyprvbuIM=2078082791; else uuiSZArHEXKgZoyVGWKSBkyprvbuIM=953737470;if (uuiSZArHEXKgZoyVGWKSBkyprvbuIM == uuiSZArHEXKgZoyVGWKSBkyprvbuIM- 1 ) uuiSZArHEXKgZoyVGWKSBkyprvbuIM=125528957; else uuiSZArHEXKgZoyVGWKSBkyprvbuIM=1253498358;if (uuiSZArHEXKgZoyVGWKSBkyprvbuIM == uuiSZArHEXKgZoyVGWKSBkyprvbuIM- 0 ) uuiSZArHEXKgZoyVGWKSBkyprvbuIM=370011841; else uuiSZArHEXKgZoyVGWKSBkyprvbuIM=643500271;if (uuiSZArHEXKgZoyVGWKSBkyprvbuIM == uuiSZArHEXKgZoyVGWKSBkyprvbuIM- 1 ) uuiSZArHEXKgZoyVGWKSBkyprvbuIM=730552833; else uuiSZArHEXKgZoyVGWKSBkyprvbuIM=786096254;if (uuiSZArHEXKgZoyVGWKSBkyprvbuIM == uuiSZArHEXKgZoyVGWKSBkyprvbuIM- 0 ) uuiSZArHEXKgZoyVGWKSBkyprvbuIM=1917336433; else uuiSZArHEXKgZoyVGWKSBkyprvbuIM=595426610;long DkbwPrJazhZFMXKdTfmpSSPdckEHyi=1849898876;if (DkbwPrJazhZFMXKdTfmpSSPdckEHyi == DkbwPrJazhZFMXKdTfmpSSPdckEHyi- 0 ) DkbwPrJazhZFMXKdTfmpSSPdckEHyi=1561063015; else DkbwPrJazhZFMXKdTfmpSSPdckEHyi=299151702;if (DkbwPrJazhZFMXKdTfmpSSPdckEHyi == DkbwPrJazhZFMXKdTfmpSSPdckEHyi- 0 ) DkbwPrJazhZFMXKdTfmpSSPdckEHyi=1635774605; else DkbwPrJazhZFMXKdTfmpSSPdckEHyi=1772562178;if (DkbwPrJazhZFMXKdTfmpSSPdckEHyi == DkbwPrJazhZFMXKdTfmpSSPdckEHyi- 0 ) DkbwPrJazhZFMXKdTfmpSSPdckEHyi=1909456126; else DkbwPrJazhZFMXKdTfmpSSPdckEHyi=1823811644;if (DkbwPrJazhZFMXKdTfmpSSPdckEHyi == DkbwPrJazhZFMXKdTfmpSSPdckEHyi- 1 ) DkbwPrJazhZFMXKdTfmpSSPdckEHyi=1431400013; else DkbwPrJazhZFMXKdTfmpSSPdckEHyi=673892801;if (DkbwPrJazhZFMXKdTfmpSSPdckEHyi == DkbwPrJazhZFMXKdTfmpSSPdckEHyi- 0 ) DkbwPrJazhZFMXKdTfmpSSPdckEHyi=1503673958; else DkbwPrJazhZFMXKdTfmpSSPdckEHyi=329586933;if (DkbwPrJazhZFMXKdTfmpSSPdckEHyi == DkbwPrJazhZFMXKdTfmpSSPdckEHyi- 1 ) DkbwPrJazhZFMXKdTfmpSSPdckEHyi=568840401; else DkbwPrJazhZFMXKdTfmpSSPdckEHyi=466501531;float lWfmKhdXqtNuxGupVGPfoNIIBYJncX=1736348722.691796674783766876109947353535f;if (lWfmKhdXqtNuxGupVGPfoNIIBYJncX - lWfmKhdXqtNuxGupVGPfoNIIBYJncX> 0.00000001 ) lWfmKhdXqtNuxGupVGPfoNIIBYJncX=1456883796.262852776370329829962316975454f; else lWfmKhdXqtNuxGupVGPfoNIIBYJncX=1467611687.415052291511083962954467306609f;if (lWfmKhdXqtNuxGupVGPfoNIIBYJncX - lWfmKhdXqtNuxGupVGPfoNIIBYJncX> 0.00000001 ) lWfmKhdXqtNuxGupVGPfoNIIBYJncX=915720855.733306648863855243301460005876f; else lWfmKhdXqtNuxGupVGPfoNIIBYJncX=331529073.770399979743410162226330847430f;if (lWfmKhdXqtNuxGupVGPfoNIIBYJncX - lWfmKhdXqtNuxGupVGPfoNIIBYJncX> 0.00000001 ) lWfmKhdXqtNuxGupVGPfoNIIBYJncX=758730872.287310466747566905005376054065f; else lWfmKhdXqtNuxGupVGPfoNIIBYJncX=480646371.255067721081669987595344692237f;if (lWfmKhdXqtNuxGupVGPfoNIIBYJncX - lWfmKhdXqtNuxGupVGPfoNIIBYJncX> 0.00000001 ) lWfmKhdXqtNuxGupVGPfoNIIBYJncX=1627264543.651259361038502481717767192148f; else lWfmKhdXqtNuxGupVGPfoNIIBYJncX=857345421.559443051173757794222153100516f;if (lWfmKhdXqtNuxGupVGPfoNIIBYJncX - lWfmKhdXqtNuxGupVGPfoNIIBYJncX> 0.00000001 ) lWfmKhdXqtNuxGupVGPfoNIIBYJncX=950917194.732458917043175411041903651745f; else lWfmKhdXqtNuxGupVGPfoNIIBYJncX=1479521252.464545241098838909879958693981f;if (lWfmKhdXqtNuxGupVGPfoNIIBYJncX - lWfmKhdXqtNuxGupVGPfoNIIBYJncX> 0.00000001 ) lWfmKhdXqtNuxGupVGPfoNIIBYJncX=2140376859.838269073138116417685298662606f; else lWfmKhdXqtNuxGupVGPfoNIIBYJncX=30411645.237037167094684285328740664060f;float TmnvDFDouQbzdsGeIVWnhQllbrhWkQ=1237229160.358204565115954069455600988252f;if (TmnvDFDouQbzdsGeIVWnhQllbrhWkQ - TmnvDFDouQbzdsGeIVWnhQllbrhWkQ> 0.00000001 ) TmnvDFDouQbzdsGeIVWnhQllbrhWkQ=1815780700.864567247810393776561385775833f; else TmnvDFDouQbzdsGeIVWnhQllbrhWkQ=507679201.731335250793374641732289269999f;if (TmnvDFDouQbzdsGeIVWnhQllbrhWkQ - TmnvDFDouQbzdsGeIVWnhQllbrhWkQ> 0.00000001 ) TmnvDFDouQbzdsGeIVWnhQllbrhWkQ=1076548930.242429906852698912114411256328f; else TmnvDFDouQbzdsGeIVWnhQllbrhWkQ=977687445.736255296674508069136788593137f;if (TmnvDFDouQbzdsGeIVWnhQllbrhWkQ - TmnvDFDouQbzdsGeIVWnhQllbrhWkQ> 0.00000001 ) TmnvDFDouQbzdsGeIVWnhQllbrhWkQ=1694564645.379913754382765092330739315987f; else TmnvDFDouQbzdsGeIVWnhQllbrhWkQ=1604619210.317930679642686503360180875071f;if (TmnvDFDouQbzdsGeIVWnhQllbrhWkQ - TmnvDFDouQbzdsGeIVWnhQllbrhWkQ> 0.00000001 ) TmnvDFDouQbzdsGeIVWnhQllbrhWkQ=1707174105.888718583543480825491362823727f; else TmnvDFDouQbzdsGeIVWnhQllbrhWkQ=1655573859.671886597903082780105079063609f;if (TmnvDFDouQbzdsGeIVWnhQllbrhWkQ - TmnvDFDouQbzdsGeIVWnhQllbrhWkQ> 0.00000001 ) TmnvDFDouQbzdsGeIVWnhQllbrhWkQ=2054922587.597707361533860938015362271383f; else TmnvDFDouQbzdsGeIVWnhQllbrhWkQ=1404189990.300941749652077322923569533553f;if (TmnvDFDouQbzdsGeIVWnhQllbrhWkQ - TmnvDFDouQbzdsGeIVWnhQllbrhWkQ> 0.00000001 ) TmnvDFDouQbzdsGeIVWnhQllbrhWkQ=1597547159.709924302930957440506047084252f; else TmnvDFDouQbzdsGeIVWnhQllbrhWkQ=1943425671.829705326402010372210406877394f;float IdzECELcHDHTIzSAPijJMwSMVMSvNq=481362062.268908104403605924823101340984f;if (IdzECELcHDHTIzSAPijJMwSMVMSvNq - IdzECELcHDHTIzSAPijJMwSMVMSvNq> 0.00000001 ) IdzECELcHDHTIzSAPijJMwSMVMSvNq=2114016776.895160035250571289992628516866f; else IdzECELcHDHTIzSAPijJMwSMVMSvNq=180987711.832061854609078373017209238865f;if (IdzECELcHDHTIzSAPijJMwSMVMSvNq - IdzECELcHDHTIzSAPijJMwSMVMSvNq> 0.00000001 ) IdzECELcHDHTIzSAPijJMwSMVMSvNq=1286026461.427943951043540375025486073484f; else IdzECELcHDHTIzSAPijJMwSMVMSvNq=762596061.419038214652208476097385005298f;if (IdzECELcHDHTIzSAPijJMwSMVMSvNq - IdzECELcHDHTIzSAPijJMwSMVMSvNq> 0.00000001 ) IdzECELcHDHTIzSAPijJMwSMVMSvNq=959842451.057864562216247428870849723503f; else IdzECELcHDHTIzSAPijJMwSMVMSvNq=128374660.413770545492489114878742832915f;if (IdzECELcHDHTIzSAPijJMwSMVMSvNq - IdzECELcHDHTIzSAPijJMwSMVMSvNq> 0.00000001 ) IdzECELcHDHTIzSAPijJMwSMVMSvNq=1091168004.107091368708458947161983577194f; else IdzECELcHDHTIzSAPijJMwSMVMSvNq=1080416644.973030650896718030590586432452f;if (IdzECELcHDHTIzSAPijJMwSMVMSvNq - IdzECELcHDHTIzSAPijJMwSMVMSvNq> 0.00000001 ) IdzECELcHDHTIzSAPijJMwSMVMSvNq=819370806.537593542949848538655770540861f; else IdzECELcHDHTIzSAPijJMwSMVMSvNq=778429184.820846023559966184800581886705f;if (IdzECELcHDHTIzSAPijJMwSMVMSvNq - IdzECELcHDHTIzSAPijJMwSMVMSvNq> 0.00000001 ) IdzECELcHDHTIzSAPijJMwSMVMSvNq=821644984.479112001261510691547617234016f; else IdzECELcHDHTIzSAPijJMwSMVMSvNq=429934874.133918196429464007130536997208f;double BwWGPqjAIXzaCOoIbRLQjqoieBiAMI=935412471.846314330517172087325469457553;if (BwWGPqjAIXzaCOoIbRLQjqoieBiAMI == BwWGPqjAIXzaCOoIbRLQjqoieBiAMI ) BwWGPqjAIXzaCOoIbRLQjqoieBiAMI=1792756766.112046302254182558802802559707; else BwWGPqjAIXzaCOoIbRLQjqoieBiAMI=1479094888.788750514789628148890086241069;if (BwWGPqjAIXzaCOoIbRLQjqoieBiAMI == BwWGPqjAIXzaCOoIbRLQjqoieBiAMI ) BwWGPqjAIXzaCOoIbRLQjqoieBiAMI=798204478.770658744214513339542101870261; else BwWGPqjAIXzaCOoIbRLQjqoieBiAMI=1098703216.956511907050469668491262877999;if (BwWGPqjAIXzaCOoIbRLQjqoieBiAMI == BwWGPqjAIXzaCOoIbRLQjqoieBiAMI ) BwWGPqjAIXzaCOoIbRLQjqoieBiAMI=1320125488.859336901253350275322397080574; else BwWGPqjAIXzaCOoIbRLQjqoieBiAMI=1989359327.502920213386835901370377867336;if (BwWGPqjAIXzaCOoIbRLQjqoieBiAMI == BwWGPqjAIXzaCOoIbRLQjqoieBiAMI ) BwWGPqjAIXzaCOoIbRLQjqoieBiAMI=41784077.754775254016205656228921534527; else BwWGPqjAIXzaCOoIbRLQjqoieBiAMI=1999748233.409764326076836417321111993425;if (BwWGPqjAIXzaCOoIbRLQjqoieBiAMI == BwWGPqjAIXzaCOoIbRLQjqoieBiAMI ) BwWGPqjAIXzaCOoIbRLQjqoieBiAMI=1907921229.274772865134126320522730639898; else BwWGPqjAIXzaCOoIbRLQjqoieBiAMI=1407630643.200191217566329719668401465414;if (BwWGPqjAIXzaCOoIbRLQjqoieBiAMI == BwWGPqjAIXzaCOoIbRLQjqoieBiAMI ) BwWGPqjAIXzaCOoIbRLQjqoieBiAMI=424090133.317704504511306751193124180984; else BwWGPqjAIXzaCOoIbRLQjqoieBiAMI=1502024529.780436962632525710571442220472;long cIBsPCujCnrsahFczsTkBUFMLVpJDd=1047406884;if (cIBsPCujCnrsahFczsTkBUFMLVpJDd == cIBsPCujCnrsahFczsTkBUFMLVpJDd- 1 ) cIBsPCujCnrsahFczsTkBUFMLVpJDd=294919751; else cIBsPCujCnrsahFczsTkBUFMLVpJDd=1659014797;if (cIBsPCujCnrsahFczsTkBUFMLVpJDd == cIBsPCujCnrsahFczsTkBUFMLVpJDd- 0 ) cIBsPCujCnrsahFczsTkBUFMLVpJDd=63878516; else cIBsPCujCnrsahFczsTkBUFMLVpJDd=248099240;if (cIBsPCujCnrsahFczsTkBUFMLVpJDd == cIBsPCujCnrsahFczsTkBUFMLVpJDd- 0 ) cIBsPCujCnrsahFczsTkBUFMLVpJDd=1429718673; else cIBsPCujCnrsahFczsTkBUFMLVpJDd=2069201400;if (cIBsPCujCnrsahFczsTkBUFMLVpJDd == cIBsPCujCnrsahFczsTkBUFMLVpJDd- 0 ) cIBsPCujCnrsahFczsTkBUFMLVpJDd=277386709; else cIBsPCujCnrsahFczsTkBUFMLVpJDd=1687406040;if (cIBsPCujCnrsahFczsTkBUFMLVpJDd == cIBsPCujCnrsahFczsTkBUFMLVpJDd- 1 ) cIBsPCujCnrsahFczsTkBUFMLVpJDd=1563994844; else cIBsPCujCnrsahFczsTkBUFMLVpJDd=2143615933;if (cIBsPCujCnrsahFczsTkBUFMLVpJDd == cIBsPCujCnrsahFczsTkBUFMLVpJDd- 1 ) cIBsPCujCnrsahFczsTkBUFMLVpJDd=158316639; else cIBsPCujCnrsahFczsTkBUFMLVpJDd=916135461;long ccojHeHnkZiYBNWonJptUaARBnhaEr=653934219;if (ccojHeHnkZiYBNWonJptUaARBnhaEr == ccojHeHnkZiYBNWonJptUaARBnhaEr- 0 ) ccojHeHnkZiYBNWonJptUaARBnhaEr=1159384140; else ccojHeHnkZiYBNWonJptUaARBnhaEr=662134688;if (ccojHeHnkZiYBNWonJptUaARBnhaEr == ccojHeHnkZiYBNWonJptUaARBnhaEr- 1 ) ccojHeHnkZiYBNWonJptUaARBnhaEr=150668905; else ccojHeHnkZiYBNWonJptUaARBnhaEr=1353046716;if (ccojHeHnkZiYBNWonJptUaARBnhaEr == ccojHeHnkZiYBNWonJptUaARBnhaEr- 1 ) ccojHeHnkZiYBNWonJptUaARBnhaEr=1252789479; else ccojHeHnkZiYBNWonJptUaARBnhaEr=85898285;if (ccojHeHnkZiYBNWonJptUaARBnhaEr == ccojHeHnkZiYBNWonJptUaARBnhaEr- 1 ) ccojHeHnkZiYBNWonJptUaARBnhaEr=1308720221; else ccojHeHnkZiYBNWonJptUaARBnhaEr=124657449;if (ccojHeHnkZiYBNWonJptUaARBnhaEr == ccojHeHnkZiYBNWonJptUaARBnhaEr- 1 ) ccojHeHnkZiYBNWonJptUaARBnhaEr=139203634; else ccojHeHnkZiYBNWonJptUaARBnhaEr=1433761875;if (ccojHeHnkZiYBNWonJptUaARBnhaEr == ccojHeHnkZiYBNWonJptUaARBnhaEr- 1 ) ccojHeHnkZiYBNWonJptUaARBnhaEr=1002158479; else ccojHeHnkZiYBNWonJptUaARBnhaEr=1699695455;double LDkuWzWxnTxQfgHmuGnssZyBdXZpbb=1135531934.360254962843274019344054867837;if (LDkuWzWxnTxQfgHmuGnssZyBdXZpbb == LDkuWzWxnTxQfgHmuGnssZyBdXZpbb ) LDkuWzWxnTxQfgHmuGnssZyBdXZpbb=605741720.933104001527178320859875231740; else LDkuWzWxnTxQfgHmuGnssZyBdXZpbb=1921721747.724591027012893809655819926998;if (LDkuWzWxnTxQfgHmuGnssZyBdXZpbb == LDkuWzWxnTxQfgHmuGnssZyBdXZpbb ) LDkuWzWxnTxQfgHmuGnssZyBdXZpbb=135015123.337281468740384808354329501578; else LDkuWzWxnTxQfgHmuGnssZyBdXZpbb=171903635.623978572254292653523875511878;if (LDkuWzWxnTxQfgHmuGnssZyBdXZpbb == LDkuWzWxnTxQfgHmuGnssZyBdXZpbb ) LDkuWzWxnTxQfgHmuGnssZyBdXZpbb=1337109852.090122841633710023806816586957; else LDkuWzWxnTxQfgHmuGnssZyBdXZpbb=2144190257.780437417081399027559918328565;if (LDkuWzWxnTxQfgHmuGnssZyBdXZpbb == LDkuWzWxnTxQfgHmuGnssZyBdXZpbb ) LDkuWzWxnTxQfgHmuGnssZyBdXZpbb=1653177445.790405438471682563756040998222; else LDkuWzWxnTxQfgHmuGnssZyBdXZpbb=1871039013.906340290579929934787691085061;if (LDkuWzWxnTxQfgHmuGnssZyBdXZpbb == LDkuWzWxnTxQfgHmuGnssZyBdXZpbb ) LDkuWzWxnTxQfgHmuGnssZyBdXZpbb=1201708179.821757644247339713207565642546; else LDkuWzWxnTxQfgHmuGnssZyBdXZpbb=1258282085.524975827239055037337396547722;if (LDkuWzWxnTxQfgHmuGnssZyBdXZpbb == LDkuWzWxnTxQfgHmuGnssZyBdXZpbb ) LDkuWzWxnTxQfgHmuGnssZyBdXZpbb=1682192019.611983456400608851061532923854; else LDkuWzWxnTxQfgHmuGnssZyBdXZpbb=1323993202.921662927279193879989677119006;double cyIcqvBTctSUowMUTpDaTwlYPVUXaP=630681998.793755295148335285762346699496;if (cyIcqvBTctSUowMUTpDaTwlYPVUXaP == cyIcqvBTctSUowMUTpDaTwlYPVUXaP ) cyIcqvBTctSUowMUTpDaTwlYPVUXaP=1582975216.524583299587317644648000356817; else cyIcqvBTctSUowMUTpDaTwlYPVUXaP=1917639561.681072495219738604915695953523;if (cyIcqvBTctSUowMUTpDaTwlYPVUXaP == cyIcqvBTctSUowMUTpDaTwlYPVUXaP ) cyIcqvBTctSUowMUTpDaTwlYPVUXaP=342640389.849291644625044048286201210713; else cyIcqvBTctSUowMUTpDaTwlYPVUXaP=368448171.008258055807149065291534864908;if (cyIcqvBTctSUowMUTpDaTwlYPVUXaP == cyIcqvBTctSUowMUTpDaTwlYPVUXaP ) cyIcqvBTctSUowMUTpDaTwlYPVUXaP=1680653614.103163889709449927235553357450; else cyIcqvBTctSUowMUTpDaTwlYPVUXaP=967124395.425780408639760220695878375192;if (cyIcqvBTctSUowMUTpDaTwlYPVUXaP == cyIcqvBTctSUowMUTpDaTwlYPVUXaP ) cyIcqvBTctSUowMUTpDaTwlYPVUXaP=1942101052.643038577962960790313896575741; else cyIcqvBTctSUowMUTpDaTwlYPVUXaP=209021466.541710673285374392979689996857;if (cyIcqvBTctSUowMUTpDaTwlYPVUXaP == cyIcqvBTctSUowMUTpDaTwlYPVUXaP ) cyIcqvBTctSUowMUTpDaTwlYPVUXaP=350294576.484753136112814852339498306392; else cyIcqvBTctSUowMUTpDaTwlYPVUXaP=2086704714.492790273226696788614563900645;if (cyIcqvBTctSUowMUTpDaTwlYPVUXaP == cyIcqvBTctSUowMUTpDaTwlYPVUXaP ) cyIcqvBTctSUowMUTpDaTwlYPVUXaP=108895606.785573013479921380262989803099; else cyIcqvBTctSUowMUTpDaTwlYPVUXaP=2143440445.173954223478525665262059391777;long CNQNAbBjjEfnowXReUJWtpOkDIXcdT=1974851369;if (CNQNAbBjjEfnowXReUJWtpOkDIXcdT == CNQNAbBjjEfnowXReUJWtpOkDIXcdT- 1 ) CNQNAbBjjEfnowXReUJWtpOkDIXcdT=2093782148; else CNQNAbBjjEfnowXReUJWtpOkDIXcdT=1170268352;if (CNQNAbBjjEfnowXReUJWtpOkDIXcdT == CNQNAbBjjEfnowXReUJWtpOkDIXcdT- 0 ) CNQNAbBjjEfnowXReUJWtpOkDIXcdT=310393741; else CNQNAbBjjEfnowXReUJWtpOkDIXcdT=1008736126;if (CNQNAbBjjEfnowXReUJWtpOkDIXcdT == CNQNAbBjjEfnowXReUJWtpOkDIXcdT- 0 ) CNQNAbBjjEfnowXReUJWtpOkDIXcdT=1142224108; else CNQNAbBjjEfnowXReUJWtpOkDIXcdT=1800235439;if (CNQNAbBjjEfnowXReUJWtpOkDIXcdT == CNQNAbBjjEfnowXReUJWtpOkDIXcdT- 1 ) CNQNAbBjjEfnowXReUJWtpOkDIXcdT=1464308014; else CNQNAbBjjEfnowXReUJWtpOkDIXcdT=1162677530;if (CNQNAbBjjEfnowXReUJWtpOkDIXcdT == CNQNAbBjjEfnowXReUJWtpOkDIXcdT- 1 ) CNQNAbBjjEfnowXReUJWtpOkDIXcdT=1156440890; else CNQNAbBjjEfnowXReUJWtpOkDIXcdT=427113539;if (CNQNAbBjjEfnowXReUJWtpOkDIXcdT == CNQNAbBjjEfnowXReUJWtpOkDIXcdT- 1 ) CNQNAbBjjEfnowXReUJWtpOkDIXcdT=151338537; else CNQNAbBjjEfnowXReUJWtpOkDIXcdT=2141991041;long BFnDuPGcZOMbDGHOdTlcaAsvIIVbys=1013555018;if (BFnDuPGcZOMbDGHOdTlcaAsvIIVbys == BFnDuPGcZOMbDGHOdTlcaAsvIIVbys- 0 ) BFnDuPGcZOMbDGHOdTlcaAsvIIVbys=216987868; else BFnDuPGcZOMbDGHOdTlcaAsvIIVbys=371476481;if (BFnDuPGcZOMbDGHOdTlcaAsvIIVbys == BFnDuPGcZOMbDGHOdTlcaAsvIIVbys- 1 ) BFnDuPGcZOMbDGHOdTlcaAsvIIVbys=116720239; else BFnDuPGcZOMbDGHOdTlcaAsvIIVbys=2081612260;if (BFnDuPGcZOMbDGHOdTlcaAsvIIVbys == BFnDuPGcZOMbDGHOdTlcaAsvIIVbys- 0 ) BFnDuPGcZOMbDGHOdTlcaAsvIIVbys=1393986733; else BFnDuPGcZOMbDGHOdTlcaAsvIIVbys=1828869261;if (BFnDuPGcZOMbDGHOdTlcaAsvIIVbys == BFnDuPGcZOMbDGHOdTlcaAsvIIVbys- 0 ) BFnDuPGcZOMbDGHOdTlcaAsvIIVbys=1602361967; else BFnDuPGcZOMbDGHOdTlcaAsvIIVbys=1786100972;if (BFnDuPGcZOMbDGHOdTlcaAsvIIVbys == BFnDuPGcZOMbDGHOdTlcaAsvIIVbys- 1 ) BFnDuPGcZOMbDGHOdTlcaAsvIIVbys=2127104342; else BFnDuPGcZOMbDGHOdTlcaAsvIIVbys=2073477304;if (BFnDuPGcZOMbDGHOdTlcaAsvIIVbys == BFnDuPGcZOMbDGHOdTlcaAsvIIVbys- 1 ) BFnDuPGcZOMbDGHOdTlcaAsvIIVbys=1969092706; else BFnDuPGcZOMbDGHOdTlcaAsvIIVbys=1397888785;long rKrDSLOurKZXTQMFMavlCxPyeXIyEk=476533278;if (rKrDSLOurKZXTQMFMavlCxPyeXIyEk == rKrDSLOurKZXTQMFMavlCxPyeXIyEk- 1 ) rKrDSLOurKZXTQMFMavlCxPyeXIyEk=1422644219; else rKrDSLOurKZXTQMFMavlCxPyeXIyEk=1854824297;if (rKrDSLOurKZXTQMFMavlCxPyeXIyEk == rKrDSLOurKZXTQMFMavlCxPyeXIyEk- 1 ) rKrDSLOurKZXTQMFMavlCxPyeXIyEk=1377694701; else rKrDSLOurKZXTQMFMavlCxPyeXIyEk=660774336;if (rKrDSLOurKZXTQMFMavlCxPyeXIyEk == rKrDSLOurKZXTQMFMavlCxPyeXIyEk- 1 ) rKrDSLOurKZXTQMFMavlCxPyeXIyEk=962738522; else rKrDSLOurKZXTQMFMavlCxPyeXIyEk=208037881;if (rKrDSLOurKZXTQMFMavlCxPyeXIyEk == rKrDSLOurKZXTQMFMavlCxPyeXIyEk- 0 ) rKrDSLOurKZXTQMFMavlCxPyeXIyEk=1635941537; else rKrDSLOurKZXTQMFMavlCxPyeXIyEk=1930223424;if (rKrDSLOurKZXTQMFMavlCxPyeXIyEk == rKrDSLOurKZXTQMFMavlCxPyeXIyEk- 1 ) rKrDSLOurKZXTQMFMavlCxPyeXIyEk=453331547; else rKrDSLOurKZXTQMFMavlCxPyeXIyEk=1327446506;if (rKrDSLOurKZXTQMFMavlCxPyeXIyEk == rKrDSLOurKZXTQMFMavlCxPyeXIyEk- 1 ) rKrDSLOurKZXTQMFMavlCxPyeXIyEk=1359490928; else rKrDSLOurKZXTQMFMavlCxPyeXIyEk=1253540075;long xlXZPGIilbAiszXCbyGpxflefICmRC=365567557;if (xlXZPGIilbAiszXCbyGpxflefICmRC == xlXZPGIilbAiszXCbyGpxflefICmRC- 0 ) xlXZPGIilbAiszXCbyGpxflefICmRC=1645606306; else xlXZPGIilbAiszXCbyGpxflefICmRC=1242824235;if (xlXZPGIilbAiszXCbyGpxflefICmRC == xlXZPGIilbAiszXCbyGpxflefICmRC- 0 ) xlXZPGIilbAiszXCbyGpxflefICmRC=1926452577; else xlXZPGIilbAiszXCbyGpxflefICmRC=695566440;if (xlXZPGIilbAiszXCbyGpxflefICmRC == xlXZPGIilbAiszXCbyGpxflefICmRC- 0 ) xlXZPGIilbAiszXCbyGpxflefICmRC=608454662; else xlXZPGIilbAiszXCbyGpxflefICmRC=2048992900;if (xlXZPGIilbAiszXCbyGpxflefICmRC == xlXZPGIilbAiszXCbyGpxflefICmRC- 0 ) xlXZPGIilbAiszXCbyGpxflefICmRC=580864636; else xlXZPGIilbAiszXCbyGpxflefICmRC=264912887;if (xlXZPGIilbAiszXCbyGpxflefICmRC == xlXZPGIilbAiszXCbyGpxflefICmRC- 0 ) xlXZPGIilbAiszXCbyGpxflefICmRC=1715390032; else xlXZPGIilbAiszXCbyGpxflefICmRC=671776416;if (xlXZPGIilbAiszXCbyGpxflefICmRC == xlXZPGIilbAiszXCbyGpxflefICmRC- 0 ) xlXZPGIilbAiszXCbyGpxflefICmRC=1029115431; else xlXZPGIilbAiszXCbyGpxflefICmRC=1216230451;float OSjoCXYhVETWCHGiiWOyYcmSJHnpoY=1646703890.731451257532457946760693498371f;if (OSjoCXYhVETWCHGiiWOyYcmSJHnpoY - OSjoCXYhVETWCHGiiWOyYcmSJHnpoY> 0.00000001 ) OSjoCXYhVETWCHGiiWOyYcmSJHnpoY=1770601094.284881426956327026319478625816f; else OSjoCXYhVETWCHGiiWOyYcmSJHnpoY=1822288906.409736978652092538748206291636f;if (OSjoCXYhVETWCHGiiWOyYcmSJHnpoY - OSjoCXYhVETWCHGiiWOyYcmSJHnpoY> 0.00000001 ) OSjoCXYhVETWCHGiiWOyYcmSJHnpoY=719346822.762066879126887859729827920355f; else OSjoCXYhVETWCHGiiWOyYcmSJHnpoY=843849263.534299103677962252054302704525f;if (OSjoCXYhVETWCHGiiWOyYcmSJHnpoY - OSjoCXYhVETWCHGiiWOyYcmSJHnpoY> 0.00000001 ) OSjoCXYhVETWCHGiiWOyYcmSJHnpoY=754515965.753570369876615397687840123360f; else OSjoCXYhVETWCHGiiWOyYcmSJHnpoY=1927807719.569903559309429980751805891345f;if (OSjoCXYhVETWCHGiiWOyYcmSJHnpoY - OSjoCXYhVETWCHGiiWOyYcmSJHnpoY> 0.00000001 ) OSjoCXYhVETWCHGiiWOyYcmSJHnpoY=50816496.701931882295838852135461987411f; else OSjoCXYhVETWCHGiiWOyYcmSJHnpoY=8949987.221470950821866708705031145016f;if (OSjoCXYhVETWCHGiiWOyYcmSJHnpoY - OSjoCXYhVETWCHGiiWOyYcmSJHnpoY> 0.00000001 ) OSjoCXYhVETWCHGiiWOyYcmSJHnpoY=883018591.547118457800342461529082720143f; else OSjoCXYhVETWCHGiiWOyYcmSJHnpoY=333980462.695449575946100589088559856559f;if (OSjoCXYhVETWCHGiiWOyYcmSJHnpoY - OSjoCXYhVETWCHGiiWOyYcmSJHnpoY> 0.00000001 ) OSjoCXYhVETWCHGiiWOyYcmSJHnpoY=1628280713.841432691409669399225117524679f; else OSjoCXYhVETWCHGiiWOyYcmSJHnpoY=66540227.303720587909978999057843473591f;double yMKEAixYusFFRKiMXsfQXdKJWgxNVn=179510684.431857383790048485627197893763;if (yMKEAixYusFFRKiMXsfQXdKJWgxNVn == yMKEAixYusFFRKiMXsfQXdKJWgxNVn ) yMKEAixYusFFRKiMXsfQXdKJWgxNVn=268162104.480269727897654321161658445506; else yMKEAixYusFFRKiMXsfQXdKJWgxNVn=1603525149.462031198976994054468271384283;if (yMKEAixYusFFRKiMXsfQXdKJWgxNVn == yMKEAixYusFFRKiMXsfQXdKJWgxNVn ) yMKEAixYusFFRKiMXsfQXdKJWgxNVn=1899766126.533144562333429576449125929286; else yMKEAixYusFFRKiMXsfQXdKJWgxNVn=792470416.792545495169313595381995442822;if (yMKEAixYusFFRKiMXsfQXdKJWgxNVn == yMKEAixYusFFRKiMXsfQXdKJWgxNVn ) yMKEAixYusFFRKiMXsfQXdKJWgxNVn=1878251717.539775299429149092254500591810; else yMKEAixYusFFRKiMXsfQXdKJWgxNVn=108429652.467363164931797385656178914938;if (yMKEAixYusFFRKiMXsfQXdKJWgxNVn == yMKEAixYusFFRKiMXsfQXdKJWgxNVn ) yMKEAixYusFFRKiMXsfQXdKJWgxNVn=2015562263.558916389703842381432627530056; else yMKEAixYusFFRKiMXsfQXdKJWgxNVn=1521134966.924759498100605343673350793148;if (yMKEAixYusFFRKiMXsfQXdKJWgxNVn == yMKEAixYusFFRKiMXsfQXdKJWgxNVn ) yMKEAixYusFFRKiMXsfQXdKJWgxNVn=1273959661.734488029400343313320748378407; else yMKEAixYusFFRKiMXsfQXdKJWgxNVn=1112781814.635148632842537401858192800622;if (yMKEAixYusFFRKiMXsfQXdKJWgxNVn == yMKEAixYusFFRKiMXsfQXdKJWgxNVn ) yMKEAixYusFFRKiMXsfQXdKJWgxNVn=1092867951.162397530678169932197766955335; else yMKEAixYusFFRKiMXsfQXdKJWgxNVn=290962106.902188125108238113407854541654;int IxbklixqHnHnDtwaQrqTJyRmDYpdQl=386912318;if (IxbklixqHnHnDtwaQrqTJyRmDYpdQl == IxbklixqHnHnDtwaQrqTJyRmDYpdQl- 1 ) IxbklixqHnHnDtwaQrqTJyRmDYpdQl=52579980; else IxbklixqHnHnDtwaQrqTJyRmDYpdQl=1860270685;if (IxbklixqHnHnDtwaQrqTJyRmDYpdQl == IxbklixqHnHnDtwaQrqTJyRmDYpdQl- 1 ) IxbklixqHnHnDtwaQrqTJyRmDYpdQl=1630422628; else IxbklixqHnHnDtwaQrqTJyRmDYpdQl=1140980599;if (IxbklixqHnHnDtwaQrqTJyRmDYpdQl == IxbklixqHnHnDtwaQrqTJyRmDYpdQl- 0 ) IxbklixqHnHnDtwaQrqTJyRmDYpdQl=1228246836; else IxbklixqHnHnDtwaQrqTJyRmDYpdQl=961465937;if (IxbklixqHnHnDtwaQrqTJyRmDYpdQl == IxbklixqHnHnDtwaQrqTJyRmDYpdQl- 1 ) IxbklixqHnHnDtwaQrqTJyRmDYpdQl=1758535239; else IxbklixqHnHnDtwaQrqTJyRmDYpdQl=1865282234;if (IxbklixqHnHnDtwaQrqTJyRmDYpdQl == IxbklixqHnHnDtwaQrqTJyRmDYpdQl- 0 ) IxbklixqHnHnDtwaQrqTJyRmDYpdQl=1192007739; else IxbklixqHnHnDtwaQrqTJyRmDYpdQl=1917502590;if (IxbklixqHnHnDtwaQrqTJyRmDYpdQl == IxbklixqHnHnDtwaQrqTJyRmDYpdQl- 1 ) IxbklixqHnHnDtwaQrqTJyRmDYpdQl=1960031496; else IxbklixqHnHnDtwaQrqTJyRmDYpdQl=274474200;double XheKZaZFlBNvbFqkOGDxiVDfPOjnPg=1366568140.287714821943185537928144224811;if (XheKZaZFlBNvbFqkOGDxiVDfPOjnPg == XheKZaZFlBNvbFqkOGDxiVDfPOjnPg ) XheKZaZFlBNvbFqkOGDxiVDfPOjnPg=1848944949.511706658558411776224873124460; else XheKZaZFlBNvbFqkOGDxiVDfPOjnPg=849604747.012043349008047174355860874879;if (XheKZaZFlBNvbFqkOGDxiVDfPOjnPg == XheKZaZFlBNvbFqkOGDxiVDfPOjnPg ) XheKZaZFlBNvbFqkOGDxiVDfPOjnPg=948068347.330855658953832693616111535338; else XheKZaZFlBNvbFqkOGDxiVDfPOjnPg=375101231.958178929353651204839236332933;if (XheKZaZFlBNvbFqkOGDxiVDfPOjnPg == XheKZaZFlBNvbFqkOGDxiVDfPOjnPg ) XheKZaZFlBNvbFqkOGDxiVDfPOjnPg=314136750.032000686577436456143261983285; else XheKZaZFlBNvbFqkOGDxiVDfPOjnPg=482396110.619732166173937280108690098222;if (XheKZaZFlBNvbFqkOGDxiVDfPOjnPg == XheKZaZFlBNvbFqkOGDxiVDfPOjnPg ) XheKZaZFlBNvbFqkOGDxiVDfPOjnPg=1915935820.841106880549107143571429803075; else XheKZaZFlBNvbFqkOGDxiVDfPOjnPg=1662171442.495443283141066069306585253241;if (XheKZaZFlBNvbFqkOGDxiVDfPOjnPg == XheKZaZFlBNvbFqkOGDxiVDfPOjnPg ) XheKZaZFlBNvbFqkOGDxiVDfPOjnPg=1954210290.491615487596044146385578002120; else XheKZaZFlBNvbFqkOGDxiVDfPOjnPg=1345695503.014376046268291054221706703472;if (XheKZaZFlBNvbFqkOGDxiVDfPOjnPg == XheKZaZFlBNvbFqkOGDxiVDfPOjnPg ) XheKZaZFlBNvbFqkOGDxiVDfPOjnPg=1717373249.346859288810113739705801042978; else XheKZaZFlBNvbFqkOGDxiVDfPOjnPg=1789217798.468503254388741179408487754682;long THAVbMfHxYoWSIkHdwshOVfzxrDlXs=463307505;if (THAVbMfHxYoWSIkHdwshOVfzxrDlXs == THAVbMfHxYoWSIkHdwshOVfzxrDlXs- 0 ) THAVbMfHxYoWSIkHdwshOVfzxrDlXs=382383418; else THAVbMfHxYoWSIkHdwshOVfzxrDlXs=1241368395;if (THAVbMfHxYoWSIkHdwshOVfzxrDlXs == THAVbMfHxYoWSIkHdwshOVfzxrDlXs- 1 ) THAVbMfHxYoWSIkHdwshOVfzxrDlXs=13960247; else THAVbMfHxYoWSIkHdwshOVfzxrDlXs=756591295;if (THAVbMfHxYoWSIkHdwshOVfzxrDlXs == THAVbMfHxYoWSIkHdwshOVfzxrDlXs- 1 ) THAVbMfHxYoWSIkHdwshOVfzxrDlXs=865882911; else THAVbMfHxYoWSIkHdwshOVfzxrDlXs=1976818162;if (THAVbMfHxYoWSIkHdwshOVfzxrDlXs == THAVbMfHxYoWSIkHdwshOVfzxrDlXs- 0 ) THAVbMfHxYoWSIkHdwshOVfzxrDlXs=1098747495; else THAVbMfHxYoWSIkHdwshOVfzxrDlXs=1454179814;if (THAVbMfHxYoWSIkHdwshOVfzxrDlXs == THAVbMfHxYoWSIkHdwshOVfzxrDlXs- 0 ) THAVbMfHxYoWSIkHdwshOVfzxrDlXs=1992473557; else THAVbMfHxYoWSIkHdwshOVfzxrDlXs=34483892;if (THAVbMfHxYoWSIkHdwshOVfzxrDlXs == THAVbMfHxYoWSIkHdwshOVfzxrDlXs- 0 ) THAVbMfHxYoWSIkHdwshOVfzxrDlXs=1747946324; else THAVbMfHxYoWSIkHdwshOVfzxrDlXs=2108232774;float fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN=759635252.697959728694949780326180330831f;if (fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN - fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN> 0.00000001 ) fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN=1317247260.551714822767081472565017152773f; else fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN=1873783458.000714423606977920096837664577f;if (fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN - fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN> 0.00000001 ) fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN=589500804.216986838715907925024388727326f; else fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN=476801350.196749109129238293184065303837f;if (fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN - fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN> 0.00000001 ) fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN=1619126386.062373937893691516153435213245f; else fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN=1723979460.647326806547730257589878463817f;if (fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN - fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN> 0.00000001 ) fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN=72775568.189226648581072670507786697096f; else fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN=1717667517.400057852472901981789331041374f;if (fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN - fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN> 0.00000001 ) fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN=2091818512.965228212693914688681742818901f; else fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN=2115734833.669861789378910742857370433351f;if (fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN - fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN> 0.00000001 ) fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN=1334253956.132204426391941383218725539953f; else fxoMKkBzVxpZhPsvnFBAsJAUUZMFYN=2030034980.953924782733657653916024452777f;double bjQWVoALjjBJbsYCmqtsbyVmvPvclL=1432153349.390041749992317061687821073916;if (bjQWVoALjjBJbsYCmqtsbyVmvPvclL == bjQWVoALjjBJbsYCmqtsbyVmvPvclL ) bjQWVoALjjBJbsYCmqtsbyVmvPvclL=1087475053.048537136512940505375209958907; else bjQWVoALjjBJbsYCmqtsbyVmvPvclL=1496723991.984495598104528598257864194968;if (bjQWVoALjjBJbsYCmqtsbyVmvPvclL == bjQWVoALjjBJbsYCmqtsbyVmvPvclL ) bjQWVoALjjBJbsYCmqtsbyVmvPvclL=2039574429.923063965512738279146085013031; else bjQWVoALjjBJbsYCmqtsbyVmvPvclL=1326827439.264128166674484624702582205806;if (bjQWVoALjjBJbsYCmqtsbyVmvPvclL == bjQWVoALjjBJbsYCmqtsbyVmvPvclL ) bjQWVoALjjBJbsYCmqtsbyVmvPvclL=500364162.104446961011468361159944078293; else bjQWVoALjjBJbsYCmqtsbyVmvPvclL=1186426906.437168004976379123594476726058;if (bjQWVoALjjBJbsYCmqtsbyVmvPvclL == bjQWVoALjjBJbsYCmqtsbyVmvPvclL ) bjQWVoALjjBJbsYCmqtsbyVmvPvclL=500685229.877969541147738378595671906244; else bjQWVoALjjBJbsYCmqtsbyVmvPvclL=2019610434.215720051168256885846967321706;if (bjQWVoALjjBJbsYCmqtsbyVmvPvclL == bjQWVoALjjBJbsYCmqtsbyVmvPvclL ) bjQWVoALjjBJbsYCmqtsbyVmvPvclL=1898340899.340793202842743843523241935950; else bjQWVoALjjBJbsYCmqtsbyVmvPvclL=1641372180.035593856368400432459829410097;if (bjQWVoALjjBJbsYCmqtsbyVmvPvclL == bjQWVoALjjBJbsYCmqtsbyVmvPvclL ) bjQWVoALjjBJbsYCmqtsbyVmvPvclL=530111321.946382826100836594151450132628; else bjQWVoALjjBJbsYCmqtsbyVmvPvclL=279652858.396952213961646910493794750726;long MzomIQrnSQlHKwvnCTKNJJQaivKOzM=213122227;if (MzomIQrnSQlHKwvnCTKNJJQaivKOzM == MzomIQrnSQlHKwvnCTKNJJQaivKOzM- 0 ) MzomIQrnSQlHKwvnCTKNJJQaivKOzM=245272724; else MzomIQrnSQlHKwvnCTKNJJQaivKOzM=957737997;if (MzomIQrnSQlHKwvnCTKNJJQaivKOzM == MzomIQrnSQlHKwvnCTKNJJQaivKOzM- 0 ) MzomIQrnSQlHKwvnCTKNJJQaivKOzM=471970538; else MzomIQrnSQlHKwvnCTKNJJQaivKOzM=1108639957;if (MzomIQrnSQlHKwvnCTKNJJQaivKOzM == MzomIQrnSQlHKwvnCTKNJJQaivKOzM- 1 ) MzomIQrnSQlHKwvnCTKNJJQaivKOzM=1317837233; else MzomIQrnSQlHKwvnCTKNJJQaivKOzM=353226187;if (MzomIQrnSQlHKwvnCTKNJJQaivKOzM == MzomIQrnSQlHKwvnCTKNJJQaivKOzM- 1 ) MzomIQrnSQlHKwvnCTKNJJQaivKOzM=991664766; else MzomIQrnSQlHKwvnCTKNJJQaivKOzM=805887605;if (MzomIQrnSQlHKwvnCTKNJJQaivKOzM == MzomIQrnSQlHKwvnCTKNJJQaivKOzM- 1 ) MzomIQrnSQlHKwvnCTKNJJQaivKOzM=1168592827; else MzomIQrnSQlHKwvnCTKNJJQaivKOzM=443776377;if (MzomIQrnSQlHKwvnCTKNJJQaivKOzM == MzomIQrnSQlHKwvnCTKNJJQaivKOzM- 0 ) MzomIQrnSQlHKwvnCTKNJJQaivKOzM=812256430; else MzomIQrnSQlHKwvnCTKNJJQaivKOzM=897631725;int qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ=2023156116;if (qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ == qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ- 1 ) qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ=1151625073; else qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ=315792975;if (qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ == qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ- 0 ) qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ=1020757721; else qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ=946641459;if (qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ == qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ- 1 ) qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ=1848997281; else qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ=1921078987;if (qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ == qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ- 1 ) qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ=259271090; else qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ=130820354;if (qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ == qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ- 1 ) qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ=1373098229; else qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ=717374017;if (qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ == qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ- 0 ) qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ=725944098; else qJCuiPyMUEUKgeKdgIqYXkxdYGElkJ=2125237853; }
 qJCuiPyMUEUKgeKdgIqYXkxdYGElkJy::qJCuiPyMUEUKgeKdgIqYXkxdYGElkJy()
 { this->PQJSQEgDDrrk("DoEDxCuSeluhClalgAgtTpiTFMZWcKPQJSQEgDDrrkj", true, 1193868139, 1940606357, 835734084); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class JZhMwjjyxQSohTezPyoKREYansUSAhy
 { 
public: bool kKMlLwmVVVXBFApsdBCFTqlIFAHZEO; double kKMlLwmVVVXBFApsdBCFTqlIFAHZEOJZhMwjjyxQSohTezPyoKREYansUSAh; JZhMwjjyxQSohTezPyoKREYansUSAhy(); void eyfgvWVeeWBL(string kKMlLwmVVVXBFApsdBCFTqlIFAHZEOeyfgvWVeeWBL, bool RyQPidViTDdcTpuCTayzqRybhuZEfK, int oNRGEZkpSLXvhycrWvNDhgaEYzzohn, float DRCIOQLNkUSdImcukzypXeqcdImMiE, long ZvIeIGtYkMgJKtkDXocdPAqrQZbZfG);
 protected: bool kKMlLwmVVVXBFApsdBCFTqlIFAHZEOo; double kKMlLwmVVVXBFApsdBCFTqlIFAHZEOJZhMwjjyxQSohTezPyoKREYansUSAhf; void eyfgvWVeeWBLu(string kKMlLwmVVVXBFApsdBCFTqlIFAHZEOeyfgvWVeeWBLg, bool RyQPidViTDdcTpuCTayzqRybhuZEfKe, int oNRGEZkpSLXvhycrWvNDhgaEYzzohnr, float DRCIOQLNkUSdImcukzypXeqcdImMiEw, long ZvIeIGtYkMgJKtkDXocdPAqrQZbZfGn);
 private: bool kKMlLwmVVVXBFApsdBCFTqlIFAHZEORyQPidViTDdcTpuCTayzqRybhuZEfK; double kKMlLwmVVVXBFApsdBCFTqlIFAHZEODRCIOQLNkUSdImcukzypXeqcdImMiEJZhMwjjyxQSohTezPyoKREYansUSAh;
 void eyfgvWVeeWBLv(string RyQPidViTDdcTpuCTayzqRybhuZEfKeyfgvWVeeWBL, bool RyQPidViTDdcTpuCTayzqRybhuZEfKoNRGEZkpSLXvhycrWvNDhgaEYzzohn, int oNRGEZkpSLXvhycrWvNDhgaEYzzohnkKMlLwmVVVXBFApsdBCFTqlIFAHZEO, float DRCIOQLNkUSdImcukzypXeqcdImMiEZvIeIGtYkMgJKtkDXocdPAqrQZbZfG, long ZvIeIGtYkMgJKtkDXocdPAqrQZbZfGRyQPidViTDdcTpuCTayzqRybhuZEfK); };
 void JZhMwjjyxQSohTezPyoKREYansUSAhy::eyfgvWVeeWBL(string kKMlLwmVVVXBFApsdBCFTqlIFAHZEOeyfgvWVeeWBL, bool RyQPidViTDdcTpuCTayzqRybhuZEfK, int oNRGEZkpSLXvhycrWvNDhgaEYzzohn, float DRCIOQLNkUSdImcukzypXeqcdImMiE, long ZvIeIGtYkMgJKtkDXocdPAqrQZbZfG)
 { int TcmLGDANFbYhHAVvBzRTOKFKECnOuh=541830506;if (TcmLGDANFbYhHAVvBzRTOKFKECnOuh == TcmLGDANFbYhHAVvBzRTOKFKECnOuh- 1 ) TcmLGDANFbYhHAVvBzRTOKFKECnOuh=1475451097; else TcmLGDANFbYhHAVvBzRTOKFKECnOuh=215801241;if (TcmLGDANFbYhHAVvBzRTOKFKECnOuh == TcmLGDANFbYhHAVvBzRTOKFKECnOuh- 1 ) TcmLGDANFbYhHAVvBzRTOKFKECnOuh=1408607081; else TcmLGDANFbYhHAVvBzRTOKFKECnOuh=226786754;if (TcmLGDANFbYhHAVvBzRTOKFKECnOuh == TcmLGDANFbYhHAVvBzRTOKFKECnOuh- 0 ) TcmLGDANFbYhHAVvBzRTOKFKECnOuh=10143325; else TcmLGDANFbYhHAVvBzRTOKFKECnOuh=24525080;if (TcmLGDANFbYhHAVvBzRTOKFKECnOuh == TcmLGDANFbYhHAVvBzRTOKFKECnOuh- 1 ) TcmLGDANFbYhHAVvBzRTOKFKECnOuh=1350200246; else TcmLGDANFbYhHAVvBzRTOKFKECnOuh=570309337;if (TcmLGDANFbYhHAVvBzRTOKFKECnOuh == TcmLGDANFbYhHAVvBzRTOKFKECnOuh- 0 ) TcmLGDANFbYhHAVvBzRTOKFKECnOuh=371692090; else TcmLGDANFbYhHAVvBzRTOKFKECnOuh=32124023;if (TcmLGDANFbYhHAVvBzRTOKFKECnOuh == TcmLGDANFbYhHAVvBzRTOKFKECnOuh- 0 ) TcmLGDANFbYhHAVvBzRTOKFKECnOuh=174663939; else TcmLGDANFbYhHAVvBzRTOKFKECnOuh=1588395354;int JLTXrYKAuIaOGqGKTJNkxYPbPhecGX=430080001;if (JLTXrYKAuIaOGqGKTJNkxYPbPhecGX == JLTXrYKAuIaOGqGKTJNkxYPbPhecGX- 0 ) JLTXrYKAuIaOGqGKTJNkxYPbPhecGX=1673119094; else JLTXrYKAuIaOGqGKTJNkxYPbPhecGX=1314380688;if (JLTXrYKAuIaOGqGKTJNkxYPbPhecGX == JLTXrYKAuIaOGqGKTJNkxYPbPhecGX- 0 ) JLTXrYKAuIaOGqGKTJNkxYPbPhecGX=399500580; else JLTXrYKAuIaOGqGKTJNkxYPbPhecGX=1387595709;if (JLTXrYKAuIaOGqGKTJNkxYPbPhecGX == JLTXrYKAuIaOGqGKTJNkxYPbPhecGX- 0 ) JLTXrYKAuIaOGqGKTJNkxYPbPhecGX=2034937271; else JLTXrYKAuIaOGqGKTJNkxYPbPhecGX=1813880021;if (JLTXrYKAuIaOGqGKTJNkxYPbPhecGX == JLTXrYKAuIaOGqGKTJNkxYPbPhecGX- 0 ) JLTXrYKAuIaOGqGKTJNkxYPbPhecGX=159455015; else JLTXrYKAuIaOGqGKTJNkxYPbPhecGX=432193410;if (JLTXrYKAuIaOGqGKTJNkxYPbPhecGX == JLTXrYKAuIaOGqGKTJNkxYPbPhecGX- 0 ) JLTXrYKAuIaOGqGKTJNkxYPbPhecGX=1221044141; else JLTXrYKAuIaOGqGKTJNkxYPbPhecGX=971368090;if (JLTXrYKAuIaOGqGKTJNkxYPbPhecGX == JLTXrYKAuIaOGqGKTJNkxYPbPhecGX- 0 ) JLTXrYKAuIaOGqGKTJNkxYPbPhecGX=258293308; else JLTXrYKAuIaOGqGKTJNkxYPbPhecGX=203854660;long szWIqCtjUgfrSnnLqnvugLeMSwxHJQ=1394988508;if (szWIqCtjUgfrSnnLqnvugLeMSwxHJQ == szWIqCtjUgfrSnnLqnvugLeMSwxHJQ- 1 ) szWIqCtjUgfrSnnLqnvugLeMSwxHJQ=1174964507; else szWIqCtjUgfrSnnLqnvugLeMSwxHJQ=1052734781;if (szWIqCtjUgfrSnnLqnvugLeMSwxHJQ == szWIqCtjUgfrSnnLqnvugLeMSwxHJQ- 1 ) szWIqCtjUgfrSnnLqnvugLeMSwxHJQ=158628851; else szWIqCtjUgfrSnnLqnvugLeMSwxHJQ=2146273608;if (szWIqCtjUgfrSnnLqnvugLeMSwxHJQ == szWIqCtjUgfrSnnLqnvugLeMSwxHJQ- 0 ) szWIqCtjUgfrSnnLqnvugLeMSwxHJQ=1389209906; else szWIqCtjUgfrSnnLqnvugLeMSwxHJQ=1797176111;if (szWIqCtjUgfrSnnLqnvugLeMSwxHJQ == szWIqCtjUgfrSnnLqnvugLeMSwxHJQ- 1 ) szWIqCtjUgfrSnnLqnvugLeMSwxHJQ=437869743; else szWIqCtjUgfrSnnLqnvugLeMSwxHJQ=252417471;if (szWIqCtjUgfrSnnLqnvugLeMSwxHJQ == szWIqCtjUgfrSnnLqnvugLeMSwxHJQ- 0 ) szWIqCtjUgfrSnnLqnvugLeMSwxHJQ=842383575; else szWIqCtjUgfrSnnLqnvugLeMSwxHJQ=1469110394;if (szWIqCtjUgfrSnnLqnvugLeMSwxHJQ == szWIqCtjUgfrSnnLqnvugLeMSwxHJQ- 0 ) szWIqCtjUgfrSnnLqnvugLeMSwxHJQ=1348371792; else szWIqCtjUgfrSnnLqnvugLeMSwxHJQ=1274202222;long XNDMtrkSPuOZevAANsTpHYSaHxVzel=1375575097;if (XNDMtrkSPuOZevAANsTpHYSaHxVzel == XNDMtrkSPuOZevAANsTpHYSaHxVzel- 1 ) XNDMtrkSPuOZevAANsTpHYSaHxVzel=251411171; else XNDMtrkSPuOZevAANsTpHYSaHxVzel=53505846;if (XNDMtrkSPuOZevAANsTpHYSaHxVzel == XNDMtrkSPuOZevAANsTpHYSaHxVzel- 1 ) XNDMtrkSPuOZevAANsTpHYSaHxVzel=1400587816; else XNDMtrkSPuOZevAANsTpHYSaHxVzel=1052738889;if (XNDMtrkSPuOZevAANsTpHYSaHxVzel == XNDMtrkSPuOZevAANsTpHYSaHxVzel- 0 ) XNDMtrkSPuOZevAANsTpHYSaHxVzel=172908972; else XNDMtrkSPuOZevAANsTpHYSaHxVzel=562079657;if (XNDMtrkSPuOZevAANsTpHYSaHxVzel == XNDMtrkSPuOZevAANsTpHYSaHxVzel- 0 ) XNDMtrkSPuOZevAANsTpHYSaHxVzel=654376882; else XNDMtrkSPuOZevAANsTpHYSaHxVzel=1809054723;if (XNDMtrkSPuOZevAANsTpHYSaHxVzel == XNDMtrkSPuOZevAANsTpHYSaHxVzel- 1 ) XNDMtrkSPuOZevAANsTpHYSaHxVzel=56346226; else XNDMtrkSPuOZevAANsTpHYSaHxVzel=976413671;if (XNDMtrkSPuOZevAANsTpHYSaHxVzel == XNDMtrkSPuOZevAANsTpHYSaHxVzel- 0 ) XNDMtrkSPuOZevAANsTpHYSaHxVzel=1153226260; else XNDMtrkSPuOZevAANsTpHYSaHxVzel=1186258882;float OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq=1868899478.572562735083038637374614635181f;if (OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq - OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq> 0.00000001 ) OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq=957315823.858133867540764533001490907894f; else OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq=927159078.875433691697084595020319047823f;if (OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq - OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq> 0.00000001 ) OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq=413430847.608487534295251552751397456750f; else OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq=865861103.787800070716193617960528332113f;if (OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq - OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq> 0.00000001 ) OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq=1246479744.189552855269204530485099136957f; else OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq=1741402755.896863875287271143481273920053f;if (OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq - OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq> 0.00000001 ) OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq=1188353742.690947354287630992909357762395f; else OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq=2023426630.382360743499031575314926434329f;if (OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq - OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq> 0.00000001 ) OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq=876510945.506990548126962143730819257793f; else OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq=147083109.652599314204739906540270970885f;if (OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq - OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq> 0.00000001 ) OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq=545212134.485568457384963718046136459372f; else OdsuWLVhJPAQbJAXSjoKJjjnrLOdFq=565826877.063082938256027631737285936022f;double GqXQHDxlDqsFrtBFGvnJlcTdorsDcj=355962813.319515383918963675462260790305;if (GqXQHDxlDqsFrtBFGvnJlcTdorsDcj == GqXQHDxlDqsFrtBFGvnJlcTdorsDcj ) GqXQHDxlDqsFrtBFGvnJlcTdorsDcj=1822957802.296678092046275892462175225093; else GqXQHDxlDqsFrtBFGvnJlcTdorsDcj=1030201858.509882240579443710690905423245;if (GqXQHDxlDqsFrtBFGvnJlcTdorsDcj == GqXQHDxlDqsFrtBFGvnJlcTdorsDcj ) GqXQHDxlDqsFrtBFGvnJlcTdorsDcj=2099927136.300955749366399373362651471547; else GqXQHDxlDqsFrtBFGvnJlcTdorsDcj=2001364452.010527960907133571615199486069;if (GqXQHDxlDqsFrtBFGvnJlcTdorsDcj == GqXQHDxlDqsFrtBFGvnJlcTdorsDcj ) GqXQHDxlDqsFrtBFGvnJlcTdorsDcj=1397172090.430336787352357552900094202215; else GqXQHDxlDqsFrtBFGvnJlcTdorsDcj=169552958.100158781880437485303417954714;if (GqXQHDxlDqsFrtBFGvnJlcTdorsDcj == GqXQHDxlDqsFrtBFGvnJlcTdorsDcj ) GqXQHDxlDqsFrtBFGvnJlcTdorsDcj=1222079536.467326230087212167793702289431; else GqXQHDxlDqsFrtBFGvnJlcTdorsDcj=612884850.418843321226642630689802276525;if (GqXQHDxlDqsFrtBFGvnJlcTdorsDcj == GqXQHDxlDqsFrtBFGvnJlcTdorsDcj ) GqXQHDxlDqsFrtBFGvnJlcTdorsDcj=398357899.790068058995668726066995361668; else GqXQHDxlDqsFrtBFGvnJlcTdorsDcj=497057775.780754668578380435555166980571;if (GqXQHDxlDqsFrtBFGvnJlcTdorsDcj == GqXQHDxlDqsFrtBFGvnJlcTdorsDcj ) GqXQHDxlDqsFrtBFGvnJlcTdorsDcj=2089927382.759021591426950872693612373588; else GqXQHDxlDqsFrtBFGvnJlcTdorsDcj=412796235.419782036094513921722909824785;float DFPCDkJkntRcgAvHztUdLqieYGDxir=1843521636.425878274815632318873357926770f;if (DFPCDkJkntRcgAvHztUdLqieYGDxir - DFPCDkJkntRcgAvHztUdLqieYGDxir> 0.00000001 ) DFPCDkJkntRcgAvHztUdLqieYGDxir=806187708.001543785435981284532641418895f; else DFPCDkJkntRcgAvHztUdLqieYGDxir=1626955961.360482410074355957685225856123f;if (DFPCDkJkntRcgAvHztUdLqieYGDxir - DFPCDkJkntRcgAvHztUdLqieYGDxir> 0.00000001 ) DFPCDkJkntRcgAvHztUdLqieYGDxir=316886399.599128650087466939667122014674f; else DFPCDkJkntRcgAvHztUdLqieYGDxir=1327133438.640758534831770987992856758821f;if (DFPCDkJkntRcgAvHztUdLqieYGDxir - DFPCDkJkntRcgAvHztUdLqieYGDxir> 0.00000001 ) DFPCDkJkntRcgAvHztUdLqieYGDxir=451650481.408745882941693755313889390410f; else DFPCDkJkntRcgAvHztUdLqieYGDxir=430032832.181344314511515126300361973056f;if (DFPCDkJkntRcgAvHztUdLqieYGDxir - DFPCDkJkntRcgAvHztUdLqieYGDxir> 0.00000001 ) DFPCDkJkntRcgAvHztUdLqieYGDxir=129095353.657287036044281238427421192058f; else DFPCDkJkntRcgAvHztUdLqieYGDxir=657492063.147897543910137718075673934620f;if (DFPCDkJkntRcgAvHztUdLqieYGDxir - DFPCDkJkntRcgAvHztUdLqieYGDxir> 0.00000001 ) DFPCDkJkntRcgAvHztUdLqieYGDxir=1012635751.519801859579561839548508367497f; else DFPCDkJkntRcgAvHztUdLqieYGDxir=1524644833.303441134697960612320618211164f;if (DFPCDkJkntRcgAvHztUdLqieYGDxir - DFPCDkJkntRcgAvHztUdLqieYGDxir> 0.00000001 ) DFPCDkJkntRcgAvHztUdLqieYGDxir=176227944.456328722071506265875628889523f; else DFPCDkJkntRcgAvHztUdLqieYGDxir=25825863.118125174300035012206639093865f;int IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ=1171093433;if (IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ == IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ- 1 ) IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ=651029130; else IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ=797263447;if (IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ == IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ- 0 ) IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ=1510784727; else IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ=883513561;if (IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ == IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ- 0 ) IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ=1193902097; else IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ=809041881;if (IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ == IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ- 1 ) IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ=471727388; else IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ=787762865;if (IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ == IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ- 0 ) IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ=1852563189; else IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ=1948029644;if (IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ == IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ- 0 ) IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ=467503204; else IwtwgQVoFFtEdoZhwOqjiPMnYmtDRJ=749421969;double HGcsfWBLjOGlxlQcOsxDhaPklDSEhL=1624900084.891629346880706715114430673904;if (HGcsfWBLjOGlxlQcOsxDhaPklDSEhL == HGcsfWBLjOGlxlQcOsxDhaPklDSEhL ) HGcsfWBLjOGlxlQcOsxDhaPklDSEhL=1808381057.958008837178467172910101546215; else HGcsfWBLjOGlxlQcOsxDhaPklDSEhL=849174145.745203271899804718361119596867;if (HGcsfWBLjOGlxlQcOsxDhaPklDSEhL == HGcsfWBLjOGlxlQcOsxDhaPklDSEhL ) HGcsfWBLjOGlxlQcOsxDhaPklDSEhL=1907122816.273353337490212130794995464681; else HGcsfWBLjOGlxlQcOsxDhaPklDSEhL=986035915.852563766149298200846627448899;if (HGcsfWBLjOGlxlQcOsxDhaPklDSEhL == HGcsfWBLjOGlxlQcOsxDhaPklDSEhL ) HGcsfWBLjOGlxlQcOsxDhaPklDSEhL=715850041.075675885057565670836495534523; else HGcsfWBLjOGlxlQcOsxDhaPklDSEhL=1145734750.597727473443599916397816479014;if (HGcsfWBLjOGlxlQcOsxDhaPklDSEhL == HGcsfWBLjOGlxlQcOsxDhaPklDSEhL ) HGcsfWBLjOGlxlQcOsxDhaPklDSEhL=2051795979.236357195921759491772834688245; else HGcsfWBLjOGlxlQcOsxDhaPklDSEhL=1392924970.956031734393147617708945241388;if (HGcsfWBLjOGlxlQcOsxDhaPklDSEhL == HGcsfWBLjOGlxlQcOsxDhaPklDSEhL ) HGcsfWBLjOGlxlQcOsxDhaPklDSEhL=901106505.996018583887758079707254701289; else HGcsfWBLjOGlxlQcOsxDhaPklDSEhL=1442435073.145740724024593230727278175241;if (HGcsfWBLjOGlxlQcOsxDhaPklDSEhL == HGcsfWBLjOGlxlQcOsxDhaPklDSEhL ) HGcsfWBLjOGlxlQcOsxDhaPklDSEhL=988728701.740484201846689486268157208877; else HGcsfWBLjOGlxlQcOsxDhaPklDSEhL=2020010904.569438412004009260980018479428;float VOTNqmLXDxiRKeWJcpBTotvlbXCzhy=309807894.582730644589593077084985764139f;if (VOTNqmLXDxiRKeWJcpBTotvlbXCzhy - VOTNqmLXDxiRKeWJcpBTotvlbXCzhy> 0.00000001 ) VOTNqmLXDxiRKeWJcpBTotvlbXCzhy=1300994928.406784530690934267999738571707f; else VOTNqmLXDxiRKeWJcpBTotvlbXCzhy=918833949.752529482055840505161735188821f;if (VOTNqmLXDxiRKeWJcpBTotvlbXCzhy - VOTNqmLXDxiRKeWJcpBTotvlbXCzhy> 0.00000001 ) VOTNqmLXDxiRKeWJcpBTotvlbXCzhy=1909250752.393305942688695599918765703755f; else VOTNqmLXDxiRKeWJcpBTotvlbXCzhy=1994170051.176177255566368314169091473870f;if (VOTNqmLXDxiRKeWJcpBTotvlbXCzhy - VOTNqmLXDxiRKeWJcpBTotvlbXCzhy> 0.00000001 ) VOTNqmLXDxiRKeWJcpBTotvlbXCzhy=2035793428.220862263835546241849079628571f; else VOTNqmLXDxiRKeWJcpBTotvlbXCzhy=1750155785.549544761716234465633213135124f;if (VOTNqmLXDxiRKeWJcpBTotvlbXCzhy - VOTNqmLXDxiRKeWJcpBTotvlbXCzhy> 0.00000001 ) VOTNqmLXDxiRKeWJcpBTotvlbXCzhy=649619539.579249701269123495637305934741f; else VOTNqmLXDxiRKeWJcpBTotvlbXCzhy=2144629474.964725809686548510085160828542f;if (VOTNqmLXDxiRKeWJcpBTotvlbXCzhy - VOTNqmLXDxiRKeWJcpBTotvlbXCzhy> 0.00000001 ) VOTNqmLXDxiRKeWJcpBTotvlbXCzhy=1155228573.617030550979715186843477791591f; else VOTNqmLXDxiRKeWJcpBTotvlbXCzhy=1676607181.736718061807112656574280024035f;if (VOTNqmLXDxiRKeWJcpBTotvlbXCzhy - VOTNqmLXDxiRKeWJcpBTotvlbXCzhy> 0.00000001 ) VOTNqmLXDxiRKeWJcpBTotvlbXCzhy=1622053896.906934301823505636525613755689f; else VOTNqmLXDxiRKeWJcpBTotvlbXCzhy=651104484.403790129835686179640862040438f;long kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz=237078244;if (kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz == kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz- 0 ) kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz=145168054; else kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz=698811507;if (kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz == kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz- 0 ) kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz=364928453; else kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz=1315177025;if (kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz == kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz- 1 ) kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz=328910836; else kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz=357510579;if (kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz == kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz- 1 ) kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz=455243392; else kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz=1652778027;if (kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz == kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz- 1 ) kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz=892951115; else kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz=117859757;if (kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz == kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz- 1 ) kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz=2129890703; else kIJRLEsEYaDmhrzDQkeqiCWuhTAhdz=1898950671;long mpoEmmhvnbHRcYrvVMMuSglpnsKSWB=656309516;if (mpoEmmhvnbHRcYrvVMMuSglpnsKSWB == mpoEmmhvnbHRcYrvVMMuSglpnsKSWB- 0 ) mpoEmmhvnbHRcYrvVMMuSglpnsKSWB=1352012317; else mpoEmmhvnbHRcYrvVMMuSglpnsKSWB=157695310;if (mpoEmmhvnbHRcYrvVMMuSglpnsKSWB == mpoEmmhvnbHRcYrvVMMuSglpnsKSWB- 1 ) mpoEmmhvnbHRcYrvVMMuSglpnsKSWB=1595910688; else mpoEmmhvnbHRcYrvVMMuSglpnsKSWB=880125071;if (mpoEmmhvnbHRcYrvVMMuSglpnsKSWB == mpoEmmhvnbHRcYrvVMMuSglpnsKSWB- 1 ) mpoEmmhvnbHRcYrvVMMuSglpnsKSWB=1013790402; else mpoEmmhvnbHRcYrvVMMuSglpnsKSWB=1532790375;if (mpoEmmhvnbHRcYrvVMMuSglpnsKSWB == mpoEmmhvnbHRcYrvVMMuSglpnsKSWB- 0 ) mpoEmmhvnbHRcYrvVMMuSglpnsKSWB=1736590303; else mpoEmmhvnbHRcYrvVMMuSglpnsKSWB=58225272;if (mpoEmmhvnbHRcYrvVMMuSglpnsKSWB == mpoEmmhvnbHRcYrvVMMuSglpnsKSWB- 0 ) mpoEmmhvnbHRcYrvVMMuSglpnsKSWB=199554606; else mpoEmmhvnbHRcYrvVMMuSglpnsKSWB=1909976989;if (mpoEmmhvnbHRcYrvVMMuSglpnsKSWB == mpoEmmhvnbHRcYrvVMMuSglpnsKSWB- 1 ) mpoEmmhvnbHRcYrvVMMuSglpnsKSWB=1978290989; else mpoEmmhvnbHRcYrvVMMuSglpnsKSWB=1186726507;float CfnMbKPmoxyhIKQDQkazkXqEfmigAS=1521433121.637548921239197197117058577894f;if (CfnMbKPmoxyhIKQDQkazkXqEfmigAS - CfnMbKPmoxyhIKQDQkazkXqEfmigAS> 0.00000001 ) CfnMbKPmoxyhIKQDQkazkXqEfmigAS=436418383.078790578965537504069976978917f; else CfnMbKPmoxyhIKQDQkazkXqEfmigAS=751650457.114837150773036353108394520383f;if (CfnMbKPmoxyhIKQDQkazkXqEfmigAS - CfnMbKPmoxyhIKQDQkazkXqEfmigAS> 0.00000001 ) CfnMbKPmoxyhIKQDQkazkXqEfmigAS=1874842850.367185271145530795072692092141f; else CfnMbKPmoxyhIKQDQkazkXqEfmigAS=1441997154.797851795228216397493632178510f;if (CfnMbKPmoxyhIKQDQkazkXqEfmigAS - CfnMbKPmoxyhIKQDQkazkXqEfmigAS> 0.00000001 ) CfnMbKPmoxyhIKQDQkazkXqEfmigAS=831325220.324156988910096548680675817578f; else CfnMbKPmoxyhIKQDQkazkXqEfmigAS=1428323949.428601426142969475773004076934f;if (CfnMbKPmoxyhIKQDQkazkXqEfmigAS - CfnMbKPmoxyhIKQDQkazkXqEfmigAS> 0.00000001 ) CfnMbKPmoxyhIKQDQkazkXqEfmigAS=2128380705.041069340613364981755291642573f; else CfnMbKPmoxyhIKQDQkazkXqEfmigAS=943484349.550320191024616185110539086152f;if (CfnMbKPmoxyhIKQDQkazkXqEfmigAS - CfnMbKPmoxyhIKQDQkazkXqEfmigAS> 0.00000001 ) CfnMbKPmoxyhIKQDQkazkXqEfmigAS=463590557.887816404489084278557856313144f; else CfnMbKPmoxyhIKQDQkazkXqEfmigAS=256472725.515034791188174255582870604150f;if (CfnMbKPmoxyhIKQDQkazkXqEfmigAS - CfnMbKPmoxyhIKQDQkazkXqEfmigAS> 0.00000001 ) CfnMbKPmoxyhIKQDQkazkXqEfmigAS=1101218936.337108515159758988316876385543f; else CfnMbKPmoxyhIKQDQkazkXqEfmigAS=1917933671.064186118685153313273694556182f;float zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv=556028442.234438145002406398676357856528f;if (zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv - zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv> 0.00000001 ) zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv=882169330.698619936005935984609734408879f; else zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv=965744380.363741480846367153297203007522f;if (zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv - zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv> 0.00000001 ) zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv=1446575814.227885329460596457735669217085f; else zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv=1952317965.597863069383594881866089777868f;if (zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv - zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv> 0.00000001 ) zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv=2078729990.844407473349956328643733051810f; else zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv=125891619.620155019741952724456033522255f;if (zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv - zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv> 0.00000001 ) zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv=1370771489.776373080131990298133747579864f; else zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv=759861326.486408668085542514344423303081f;if (zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv - zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv> 0.00000001 ) zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv=1109704851.432786650273116831516553017321f; else zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv=306703181.668875497797517141857368598978f;if (zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv - zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv> 0.00000001 ) zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv=1115622419.943971635155483015697667431148f; else zjKDxYfiOoiDkTGnIOAiKNmBTlzgMv=566417494.862086464761771398559438148665f;double bxhSvOYylgXDxRftviDTMbtwSLUuXo=1639743267.401791506583276424920799859390;if (bxhSvOYylgXDxRftviDTMbtwSLUuXo == bxhSvOYylgXDxRftviDTMbtwSLUuXo ) bxhSvOYylgXDxRftviDTMbtwSLUuXo=834948062.929584637326967648685890281205; else bxhSvOYylgXDxRftviDTMbtwSLUuXo=608457582.321767480290861583331272613030;if (bxhSvOYylgXDxRftviDTMbtwSLUuXo == bxhSvOYylgXDxRftviDTMbtwSLUuXo ) bxhSvOYylgXDxRftviDTMbtwSLUuXo=1462532288.344210530027840514080807416016; else bxhSvOYylgXDxRftviDTMbtwSLUuXo=1216146370.545006766116431543310015154791;if (bxhSvOYylgXDxRftviDTMbtwSLUuXo == bxhSvOYylgXDxRftviDTMbtwSLUuXo ) bxhSvOYylgXDxRftviDTMbtwSLUuXo=871840928.552568295496898458797381664105; else bxhSvOYylgXDxRftviDTMbtwSLUuXo=1499924344.208576508346992522871700428291;if (bxhSvOYylgXDxRftviDTMbtwSLUuXo == bxhSvOYylgXDxRftviDTMbtwSLUuXo ) bxhSvOYylgXDxRftviDTMbtwSLUuXo=1972467943.520139843306056159924333561925; else bxhSvOYylgXDxRftviDTMbtwSLUuXo=2071172015.841893786479968575537948739778;if (bxhSvOYylgXDxRftviDTMbtwSLUuXo == bxhSvOYylgXDxRftviDTMbtwSLUuXo ) bxhSvOYylgXDxRftviDTMbtwSLUuXo=176798252.903857537357365648609659900426; else bxhSvOYylgXDxRftviDTMbtwSLUuXo=652426339.512958722104929069419117946405;if (bxhSvOYylgXDxRftviDTMbtwSLUuXo == bxhSvOYylgXDxRftviDTMbtwSLUuXo ) bxhSvOYylgXDxRftviDTMbtwSLUuXo=416534514.722195651702761583194211740612; else bxhSvOYylgXDxRftviDTMbtwSLUuXo=757317677.232759383552871856757342251146;float zUuZkBzDKKwDeSIUWzysWtecxuCZyl=1448885738.812474643126830795873793951326f;if (zUuZkBzDKKwDeSIUWzysWtecxuCZyl - zUuZkBzDKKwDeSIUWzysWtecxuCZyl> 0.00000001 ) zUuZkBzDKKwDeSIUWzysWtecxuCZyl=1733144342.135517442436781182053383423963f; else zUuZkBzDKKwDeSIUWzysWtecxuCZyl=1422262547.432810232436345701398037385320f;if (zUuZkBzDKKwDeSIUWzysWtecxuCZyl - zUuZkBzDKKwDeSIUWzysWtecxuCZyl> 0.00000001 ) zUuZkBzDKKwDeSIUWzysWtecxuCZyl=304557177.458010508205072815873278650653f; else zUuZkBzDKKwDeSIUWzysWtecxuCZyl=705420121.502309833631420407568055739157f;if (zUuZkBzDKKwDeSIUWzysWtecxuCZyl - zUuZkBzDKKwDeSIUWzysWtecxuCZyl> 0.00000001 ) zUuZkBzDKKwDeSIUWzysWtecxuCZyl=2101599328.096570378408324386925419655878f; else zUuZkBzDKKwDeSIUWzysWtecxuCZyl=1625561024.189665605024927083155333378792f;if (zUuZkBzDKKwDeSIUWzysWtecxuCZyl - zUuZkBzDKKwDeSIUWzysWtecxuCZyl> 0.00000001 ) zUuZkBzDKKwDeSIUWzysWtecxuCZyl=1590186778.651143077089191250278282385640f; else zUuZkBzDKKwDeSIUWzysWtecxuCZyl=310526764.791447399512718724312963558986f;if (zUuZkBzDKKwDeSIUWzysWtecxuCZyl - zUuZkBzDKKwDeSIUWzysWtecxuCZyl> 0.00000001 ) zUuZkBzDKKwDeSIUWzysWtecxuCZyl=1528362615.853739886597277657576143993900f; else zUuZkBzDKKwDeSIUWzysWtecxuCZyl=1114981524.941377019995983589790152081085f;if (zUuZkBzDKKwDeSIUWzysWtecxuCZyl - zUuZkBzDKKwDeSIUWzysWtecxuCZyl> 0.00000001 ) zUuZkBzDKKwDeSIUWzysWtecxuCZyl=332292303.699665791175968236151532190930f; else zUuZkBzDKKwDeSIUWzysWtecxuCZyl=524622039.604431633889972630485603527768f;double peDZlDaNnezPMmHfgAZcyZZYACDtGS=1195073672.920139558683240836678088919435;if (peDZlDaNnezPMmHfgAZcyZZYACDtGS == peDZlDaNnezPMmHfgAZcyZZYACDtGS ) peDZlDaNnezPMmHfgAZcyZZYACDtGS=274859199.146791423158447454502886605322; else peDZlDaNnezPMmHfgAZcyZZYACDtGS=1608959316.022355866051362065167694447010;if (peDZlDaNnezPMmHfgAZcyZZYACDtGS == peDZlDaNnezPMmHfgAZcyZZYACDtGS ) peDZlDaNnezPMmHfgAZcyZZYACDtGS=1082985609.610114629360866153564137644902; else peDZlDaNnezPMmHfgAZcyZZYACDtGS=1159291147.995744235854680783940466669368;if (peDZlDaNnezPMmHfgAZcyZZYACDtGS == peDZlDaNnezPMmHfgAZcyZZYACDtGS ) peDZlDaNnezPMmHfgAZcyZZYACDtGS=1583103874.183858622293872156706107353028; else peDZlDaNnezPMmHfgAZcyZZYACDtGS=1108169924.697887294327275760562476218760;if (peDZlDaNnezPMmHfgAZcyZZYACDtGS == peDZlDaNnezPMmHfgAZcyZZYACDtGS ) peDZlDaNnezPMmHfgAZcyZZYACDtGS=1831671161.671471619597259093269466733168; else peDZlDaNnezPMmHfgAZcyZZYACDtGS=1529728633.953475781415107622832650122585;if (peDZlDaNnezPMmHfgAZcyZZYACDtGS == peDZlDaNnezPMmHfgAZcyZZYACDtGS ) peDZlDaNnezPMmHfgAZcyZZYACDtGS=1140760084.956264488442395436021244641345; else peDZlDaNnezPMmHfgAZcyZZYACDtGS=1522887446.421766095324450147570667482262;if (peDZlDaNnezPMmHfgAZcyZZYACDtGS == peDZlDaNnezPMmHfgAZcyZZYACDtGS ) peDZlDaNnezPMmHfgAZcyZZYACDtGS=1775519713.415302263954281789561487687539; else peDZlDaNnezPMmHfgAZcyZZYACDtGS=1426303651.594391015808473659680471213882;float jPVYdDwrwjmVkhQzJWBRrqbvFIODsp=1291048219.593459600059664977561637179669f;if (jPVYdDwrwjmVkhQzJWBRrqbvFIODsp - jPVYdDwrwjmVkhQzJWBRrqbvFIODsp> 0.00000001 ) jPVYdDwrwjmVkhQzJWBRrqbvFIODsp=2016226638.359099009399070375214976597739f; else jPVYdDwrwjmVkhQzJWBRrqbvFIODsp=1814220328.114171078698828377791456559450f;if (jPVYdDwrwjmVkhQzJWBRrqbvFIODsp - jPVYdDwrwjmVkhQzJWBRrqbvFIODsp> 0.00000001 ) jPVYdDwrwjmVkhQzJWBRrqbvFIODsp=980756799.348138590040872715932105876973f; else jPVYdDwrwjmVkhQzJWBRrqbvFIODsp=1251924337.272836079089776169594563924436f;if (jPVYdDwrwjmVkhQzJWBRrqbvFIODsp - jPVYdDwrwjmVkhQzJWBRrqbvFIODsp> 0.00000001 ) jPVYdDwrwjmVkhQzJWBRrqbvFIODsp=1111443355.363797914775695443082848981001f; else jPVYdDwrwjmVkhQzJWBRrqbvFIODsp=1423677052.274483619936593097859156552591f;if (jPVYdDwrwjmVkhQzJWBRrqbvFIODsp - jPVYdDwrwjmVkhQzJWBRrqbvFIODsp> 0.00000001 ) jPVYdDwrwjmVkhQzJWBRrqbvFIODsp=1685627586.792350132384525646599525323993f; else jPVYdDwrwjmVkhQzJWBRrqbvFIODsp=1356870685.112834524307567574191700328714f;if (jPVYdDwrwjmVkhQzJWBRrqbvFIODsp - jPVYdDwrwjmVkhQzJWBRrqbvFIODsp> 0.00000001 ) jPVYdDwrwjmVkhQzJWBRrqbvFIODsp=1165754451.824938279222239306985080341361f; else jPVYdDwrwjmVkhQzJWBRrqbvFIODsp=1152005524.792169374041508958102284984579f;if (jPVYdDwrwjmVkhQzJWBRrqbvFIODsp - jPVYdDwrwjmVkhQzJWBRrqbvFIODsp> 0.00000001 ) jPVYdDwrwjmVkhQzJWBRrqbvFIODsp=1835267402.701964127114395469340427951828f; else jPVYdDwrwjmVkhQzJWBRrqbvFIODsp=1904343051.590544065593109909063624939501f;long PxvhNWncBUyUpajMIpCbHujbtffhDd=762318688;if (PxvhNWncBUyUpajMIpCbHujbtffhDd == PxvhNWncBUyUpajMIpCbHujbtffhDd- 0 ) PxvhNWncBUyUpajMIpCbHujbtffhDd=235645227; else PxvhNWncBUyUpajMIpCbHujbtffhDd=1368944489;if (PxvhNWncBUyUpajMIpCbHujbtffhDd == PxvhNWncBUyUpajMIpCbHujbtffhDd- 0 ) PxvhNWncBUyUpajMIpCbHujbtffhDd=482458478; else PxvhNWncBUyUpajMIpCbHujbtffhDd=970095770;if (PxvhNWncBUyUpajMIpCbHujbtffhDd == PxvhNWncBUyUpajMIpCbHujbtffhDd- 0 ) PxvhNWncBUyUpajMIpCbHujbtffhDd=883154670; else PxvhNWncBUyUpajMIpCbHujbtffhDd=1426152690;if (PxvhNWncBUyUpajMIpCbHujbtffhDd == PxvhNWncBUyUpajMIpCbHujbtffhDd- 0 ) PxvhNWncBUyUpajMIpCbHujbtffhDd=2013265511; else PxvhNWncBUyUpajMIpCbHujbtffhDd=624974418;if (PxvhNWncBUyUpajMIpCbHujbtffhDd == PxvhNWncBUyUpajMIpCbHujbtffhDd- 1 ) PxvhNWncBUyUpajMIpCbHujbtffhDd=1738075033; else PxvhNWncBUyUpajMIpCbHujbtffhDd=922312191;if (PxvhNWncBUyUpajMIpCbHujbtffhDd == PxvhNWncBUyUpajMIpCbHujbtffhDd- 0 ) PxvhNWncBUyUpajMIpCbHujbtffhDd=1712143684; else PxvhNWncBUyUpajMIpCbHujbtffhDd=578711882;int wpCaWTxRHmNDsgAtPlqrBdgoeJvECq=914908803;if (wpCaWTxRHmNDsgAtPlqrBdgoeJvECq == wpCaWTxRHmNDsgAtPlqrBdgoeJvECq- 0 ) wpCaWTxRHmNDsgAtPlqrBdgoeJvECq=786691637; else wpCaWTxRHmNDsgAtPlqrBdgoeJvECq=1188727731;if (wpCaWTxRHmNDsgAtPlqrBdgoeJvECq == wpCaWTxRHmNDsgAtPlqrBdgoeJvECq- 1 ) wpCaWTxRHmNDsgAtPlqrBdgoeJvECq=655879048; else wpCaWTxRHmNDsgAtPlqrBdgoeJvECq=645964849;if (wpCaWTxRHmNDsgAtPlqrBdgoeJvECq == wpCaWTxRHmNDsgAtPlqrBdgoeJvECq- 0 ) wpCaWTxRHmNDsgAtPlqrBdgoeJvECq=581206412; else wpCaWTxRHmNDsgAtPlqrBdgoeJvECq=1312340422;if (wpCaWTxRHmNDsgAtPlqrBdgoeJvECq == wpCaWTxRHmNDsgAtPlqrBdgoeJvECq- 0 ) wpCaWTxRHmNDsgAtPlqrBdgoeJvECq=83630317; else wpCaWTxRHmNDsgAtPlqrBdgoeJvECq=998665794;if (wpCaWTxRHmNDsgAtPlqrBdgoeJvECq == wpCaWTxRHmNDsgAtPlqrBdgoeJvECq- 0 ) wpCaWTxRHmNDsgAtPlqrBdgoeJvECq=2070815377; else wpCaWTxRHmNDsgAtPlqrBdgoeJvECq=1873598571;if (wpCaWTxRHmNDsgAtPlqrBdgoeJvECq == wpCaWTxRHmNDsgAtPlqrBdgoeJvECq- 0 ) wpCaWTxRHmNDsgAtPlqrBdgoeJvECq=2141020343; else wpCaWTxRHmNDsgAtPlqrBdgoeJvECq=431098350;int yrSQpGaWFvScqBSlwwziNCpOiZkTTK=1840397827;if (yrSQpGaWFvScqBSlwwziNCpOiZkTTK == yrSQpGaWFvScqBSlwwziNCpOiZkTTK- 0 ) yrSQpGaWFvScqBSlwwziNCpOiZkTTK=1911900608; else yrSQpGaWFvScqBSlwwziNCpOiZkTTK=1013201025;if (yrSQpGaWFvScqBSlwwziNCpOiZkTTK == yrSQpGaWFvScqBSlwwziNCpOiZkTTK- 1 ) yrSQpGaWFvScqBSlwwziNCpOiZkTTK=1190658424; else yrSQpGaWFvScqBSlwwziNCpOiZkTTK=487896263;if (yrSQpGaWFvScqBSlwwziNCpOiZkTTK == yrSQpGaWFvScqBSlwwziNCpOiZkTTK- 0 ) yrSQpGaWFvScqBSlwwziNCpOiZkTTK=130995334; else yrSQpGaWFvScqBSlwwziNCpOiZkTTK=1505677764;if (yrSQpGaWFvScqBSlwwziNCpOiZkTTK == yrSQpGaWFvScqBSlwwziNCpOiZkTTK- 1 ) yrSQpGaWFvScqBSlwwziNCpOiZkTTK=407893549; else yrSQpGaWFvScqBSlwwziNCpOiZkTTK=590073948;if (yrSQpGaWFvScqBSlwwziNCpOiZkTTK == yrSQpGaWFvScqBSlwwziNCpOiZkTTK- 0 ) yrSQpGaWFvScqBSlwwziNCpOiZkTTK=1948438464; else yrSQpGaWFvScqBSlwwziNCpOiZkTTK=355782381;if (yrSQpGaWFvScqBSlwwziNCpOiZkTTK == yrSQpGaWFvScqBSlwwziNCpOiZkTTK- 1 ) yrSQpGaWFvScqBSlwwziNCpOiZkTTK=1661332951; else yrSQpGaWFvScqBSlwwziNCpOiZkTTK=189131164;int bANMiFqndhhyoPKrDlPGqptopiEcCu=1001871324;if (bANMiFqndhhyoPKrDlPGqptopiEcCu == bANMiFqndhhyoPKrDlPGqptopiEcCu- 0 ) bANMiFqndhhyoPKrDlPGqptopiEcCu=550835865; else bANMiFqndhhyoPKrDlPGqptopiEcCu=920358599;if (bANMiFqndhhyoPKrDlPGqptopiEcCu == bANMiFqndhhyoPKrDlPGqptopiEcCu- 0 ) bANMiFqndhhyoPKrDlPGqptopiEcCu=1117651414; else bANMiFqndhhyoPKrDlPGqptopiEcCu=2126387957;if (bANMiFqndhhyoPKrDlPGqptopiEcCu == bANMiFqndhhyoPKrDlPGqptopiEcCu- 1 ) bANMiFqndhhyoPKrDlPGqptopiEcCu=791966856; else bANMiFqndhhyoPKrDlPGqptopiEcCu=1112505636;if (bANMiFqndhhyoPKrDlPGqptopiEcCu == bANMiFqndhhyoPKrDlPGqptopiEcCu- 1 ) bANMiFqndhhyoPKrDlPGqptopiEcCu=181112276; else bANMiFqndhhyoPKrDlPGqptopiEcCu=1070788452;if (bANMiFqndhhyoPKrDlPGqptopiEcCu == bANMiFqndhhyoPKrDlPGqptopiEcCu- 0 ) bANMiFqndhhyoPKrDlPGqptopiEcCu=1285314172; else bANMiFqndhhyoPKrDlPGqptopiEcCu=1631276331;if (bANMiFqndhhyoPKrDlPGqptopiEcCu == bANMiFqndhhyoPKrDlPGqptopiEcCu- 1 ) bANMiFqndhhyoPKrDlPGqptopiEcCu=1046764040; else bANMiFqndhhyoPKrDlPGqptopiEcCu=1157039746;long ItCENCnckJESgtWGsdJfTNsujQanGd=1810746923;if (ItCENCnckJESgtWGsdJfTNsujQanGd == ItCENCnckJESgtWGsdJfTNsujQanGd- 0 ) ItCENCnckJESgtWGsdJfTNsujQanGd=560215599; else ItCENCnckJESgtWGsdJfTNsujQanGd=2019229504;if (ItCENCnckJESgtWGsdJfTNsujQanGd == ItCENCnckJESgtWGsdJfTNsujQanGd- 1 ) ItCENCnckJESgtWGsdJfTNsujQanGd=814294921; else ItCENCnckJESgtWGsdJfTNsujQanGd=984323933;if (ItCENCnckJESgtWGsdJfTNsujQanGd == ItCENCnckJESgtWGsdJfTNsujQanGd- 1 ) ItCENCnckJESgtWGsdJfTNsujQanGd=1120708350; else ItCENCnckJESgtWGsdJfTNsujQanGd=113273396;if (ItCENCnckJESgtWGsdJfTNsujQanGd == ItCENCnckJESgtWGsdJfTNsujQanGd- 1 ) ItCENCnckJESgtWGsdJfTNsujQanGd=783913469; else ItCENCnckJESgtWGsdJfTNsujQanGd=1428497520;if (ItCENCnckJESgtWGsdJfTNsujQanGd == ItCENCnckJESgtWGsdJfTNsujQanGd- 1 ) ItCENCnckJESgtWGsdJfTNsujQanGd=780834182; else ItCENCnckJESgtWGsdJfTNsujQanGd=65805100;if (ItCENCnckJESgtWGsdJfTNsujQanGd == ItCENCnckJESgtWGsdJfTNsujQanGd- 1 ) ItCENCnckJESgtWGsdJfTNsujQanGd=845010032; else ItCENCnckJESgtWGsdJfTNsujQanGd=225424031;long EVXKYxSwoKezkKUgldNPuuUDBhwUtV=963899673;if (EVXKYxSwoKezkKUgldNPuuUDBhwUtV == EVXKYxSwoKezkKUgldNPuuUDBhwUtV- 0 ) EVXKYxSwoKezkKUgldNPuuUDBhwUtV=366769197; else EVXKYxSwoKezkKUgldNPuuUDBhwUtV=1139149019;if (EVXKYxSwoKezkKUgldNPuuUDBhwUtV == EVXKYxSwoKezkKUgldNPuuUDBhwUtV- 1 ) EVXKYxSwoKezkKUgldNPuuUDBhwUtV=2027746132; else EVXKYxSwoKezkKUgldNPuuUDBhwUtV=500027570;if (EVXKYxSwoKezkKUgldNPuuUDBhwUtV == EVXKYxSwoKezkKUgldNPuuUDBhwUtV- 0 ) EVXKYxSwoKezkKUgldNPuuUDBhwUtV=957160343; else EVXKYxSwoKezkKUgldNPuuUDBhwUtV=383192282;if (EVXKYxSwoKezkKUgldNPuuUDBhwUtV == EVXKYxSwoKezkKUgldNPuuUDBhwUtV- 1 ) EVXKYxSwoKezkKUgldNPuuUDBhwUtV=1048430971; else EVXKYxSwoKezkKUgldNPuuUDBhwUtV=799394972;if (EVXKYxSwoKezkKUgldNPuuUDBhwUtV == EVXKYxSwoKezkKUgldNPuuUDBhwUtV- 1 ) EVXKYxSwoKezkKUgldNPuuUDBhwUtV=832088749; else EVXKYxSwoKezkKUgldNPuuUDBhwUtV=119869972;if (EVXKYxSwoKezkKUgldNPuuUDBhwUtV == EVXKYxSwoKezkKUgldNPuuUDBhwUtV- 1 ) EVXKYxSwoKezkKUgldNPuuUDBhwUtV=1350065738; else EVXKYxSwoKezkKUgldNPuuUDBhwUtV=647202650;double xUVBKmFaDyjvgHscJZXcCbXJnQNbbc=366271303.535014137091184823335551435605;if (xUVBKmFaDyjvgHscJZXcCbXJnQNbbc == xUVBKmFaDyjvgHscJZXcCbXJnQNbbc ) xUVBKmFaDyjvgHscJZXcCbXJnQNbbc=1151194132.032215171917399777741573719479; else xUVBKmFaDyjvgHscJZXcCbXJnQNbbc=1998069675.964307908086343937386996466950;if (xUVBKmFaDyjvgHscJZXcCbXJnQNbbc == xUVBKmFaDyjvgHscJZXcCbXJnQNbbc ) xUVBKmFaDyjvgHscJZXcCbXJnQNbbc=1776399212.771215409288986633199229318895; else xUVBKmFaDyjvgHscJZXcCbXJnQNbbc=1987271158.105841514863811901093497256301;if (xUVBKmFaDyjvgHscJZXcCbXJnQNbbc == xUVBKmFaDyjvgHscJZXcCbXJnQNbbc ) xUVBKmFaDyjvgHscJZXcCbXJnQNbbc=292620783.871428325623933651941781272869; else xUVBKmFaDyjvgHscJZXcCbXJnQNbbc=522505441.621410250369758156571656019125;if (xUVBKmFaDyjvgHscJZXcCbXJnQNbbc == xUVBKmFaDyjvgHscJZXcCbXJnQNbbc ) xUVBKmFaDyjvgHscJZXcCbXJnQNbbc=2028646621.666600263441269213313740918540; else xUVBKmFaDyjvgHscJZXcCbXJnQNbbc=437562469.123581416900313150891781302680;if (xUVBKmFaDyjvgHscJZXcCbXJnQNbbc == xUVBKmFaDyjvgHscJZXcCbXJnQNbbc ) xUVBKmFaDyjvgHscJZXcCbXJnQNbbc=136445130.272343879890852344887986054238; else xUVBKmFaDyjvgHscJZXcCbXJnQNbbc=1836637541.983673907038201194006991440749;if (xUVBKmFaDyjvgHscJZXcCbXJnQNbbc == xUVBKmFaDyjvgHscJZXcCbXJnQNbbc ) xUVBKmFaDyjvgHscJZXcCbXJnQNbbc=1345553775.075101828955786988596124319334; else xUVBKmFaDyjvgHscJZXcCbXJnQNbbc=726161756.326916411048615221557325496299;float WtXgPqrowchkZcrbGsDtpoqlABFZHE=1390815019.337785095063915131948969722064f;if (WtXgPqrowchkZcrbGsDtpoqlABFZHE - WtXgPqrowchkZcrbGsDtpoqlABFZHE> 0.00000001 ) WtXgPqrowchkZcrbGsDtpoqlABFZHE=344143905.989679580478434900633897647806f; else WtXgPqrowchkZcrbGsDtpoqlABFZHE=82864367.304118544665167053683243120213f;if (WtXgPqrowchkZcrbGsDtpoqlABFZHE - WtXgPqrowchkZcrbGsDtpoqlABFZHE> 0.00000001 ) WtXgPqrowchkZcrbGsDtpoqlABFZHE=790270549.378418432657617984060433417110f; else WtXgPqrowchkZcrbGsDtpoqlABFZHE=293466975.933240775072204773734725495611f;if (WtXgPqrowchkZcrbGsDtpoqlABFZHE - WtXgPqrowchkZcrbGsDtpoqlABFZHE> 0.00000001 ) WtXgPqrowchkZcrbGsDtpoqlABFZHE=1701904676.133282448821910833985458631995f; else WtXgPqrowchkZcrbGsDtpoqlABFZHE=852044326.517594002333601482666284689666f;if (WtXgPqrowchkZcrbGsDtpoqlABFZHE - WtXgPqrowchkZcrbGsDtpoqlABFZHE> 0.00000001 ) WtXgPqrowchkZcrbGsDtpoqlABFZHE=853586580.166332453693275286735033034492f; else WtXgPqrowchkZcrbGsDtpoqlABFZHE=177023317.644628837003018311950494223341f;if (WtXgPqrowchkZcrbGsDtpoqlABFZHE - WtXgPqrowchkZcrbGsDtpoqlABFZHE> 0.00000001 ) WtXgPqrowchkZcrbGsDtpoqlABFZHE=970798533.917215966745983213572354225150f; else WtXgPqrowchkZcrbGsDtpoqlABFZHE=14899949.660720791446621562202187977309f;if (WtXgPqrowchkZcrbGsDtpoqlABFZHE - WtXgPqrowchkZcrbGsDtpoqlABFZHE> 0.00000001 ) WtXgPqrowchkZcrbGsDtpoqlABFZHE=152235184.091629627463323673477406968395f; else WtXgPqrowchkZcrbGsDtpoqlABFZHE=1000838378.359346611204694096530493270577f;float MSGBHSzrmAfJJlRoEKzkJuabubcEjd=1529980379.427140081145626985275964850640f;if (MSGBHSzrmAfJJlRoEKzkJuabubcEjd - MSGBHSzrmAfJJlRoEKzkJuabubcEjd> 0.00000001 ) MSGBHSzrmAfJJlRoEKzkJuabubcEjd=908347146.191173895541143426376768267440f; else MSGBHSzrmAfJJlRoEKzkJuabubcEjd=478738729.154205555410183583243548453302f;if (MSGBHSzrmAfJJlRoEKzkJuabubcEjd - MSGBHSzrmAfJJlRoEKzkJuabubcEjd> 0.00000001 ) MSGBHSzrmAfJJlRoEKzkJuabubcEjd=1221713546.656550794808196779688520861786f; else MSGBHSzrmAfJJlRoEKzkJuabubcEjd=1947905090.326387564399955645288582524689f;if (MSGBHSzrmAfJJlRoEKzkJuabubcEjd - MSGBHSzrmAfJJlRoEKzkJuabubcEjd> 0.00000001 ) MSGBHSzrmAfJJlRoEKzkJuabubcEjd=265583588.002868282021530967522751984496f; else MSGBHSzrmAfJJlRoEKzkJuabubcEjd=1447344915.399549810747436715366826735208f;if (MSGBHSzrmAfJJlRoEKzkJuabubcEjd - MSGBHSzrmAfJJlRoEKzkJuabubcEjd> 0.00000001 ) MSGBHSzrmAfJJlRoEKzkJuabubcEjd=671278890.599674310116788643528024068381f; else MSGBHSzrmAfJJlRoEKzkJuabubcEjd=1991747403.571236887350635114829389167662f;if (MSGBHSzrmAfJJlRoEKzkJuabubcEjd - MSGBHSzrmAfJJlRoEKzkJuabubcEjd> 0.00000001 ) MSGBHSzrmAfJJlRoEKzkJuabubcEjd=1257986045.703975565386628840906467220318f; else MSGBHSzrmAfJJlRoEKzkJuabubcEjd=1590183663.923575876175064476142887635775f;if (MSGBHSzrmAfJJlRoEKzkJuabubcEjd - MSGBHSzrmAfJJlRoEKzkJuabubcEjd> 0.00000001 ) MSGBHSzrmAfJJlRoEKzkJuabubcEjd=363582440.625747426418493150395438715794f; else MSGBHSzrmAfJJlRoEKzkJuabubcEjd=1268006449.110256032806084450469559029813f;float nMJOzqJUnEKnaxxUaUPebNSachHJNh=876400504.703433299964557293175657803103f;if (nMJOzqJUnEKnaxxUaUPebNSachHJNh - nMJOzqJUnEKnaxxUaUPebNSachHJNh> 0.00000001 ) nMJOzqJUnEKnaxxUaUPebNSachHJNh=847572638.109570387729372014965787215773f; else nMJOzqJUnEKnaxxUaUPebNSachHJNh=2106734366.019696608802655639580925953247f;if (nMJOzqJUnEKnaxxUaUPebNSachHJNh - nMJOzqJUnEKnaxxUaUPebNSachHJNh> 0.00000001 ) nMJOzqJUnEKnaxxUaUPebNSachHJNh=303058745.671571189751627323268620420584f; else nMJOzqJUnEKnaxxUaUPebNSachHJNh=376049357.357224119566194941864009005226f;if (nMJOzqJUnEKnaxxUaUPebNSachHJNh - nMJOzqJUnEKnaxxUaUPebNSachHJNh> 0.00000001 ) nMJOzqJUnEKnaxxUaUPebNSachHJNh=608066901.260540550431480263758972045923f; else nMJOzqJUnEKnaxxUaUPebNSachHJNh=1011474626.990441666459154463985367839951f;if (nMJOzqJUnEKnaxxUaUPebNSachHJNh - nMJOzqJUnEKnaxxUaUPebNSachHJNh> 0.00000001 ) nMJOzqJUnEKnaxxUaUPebNSachHJNh=811850274.673854115060830082978304974371f; else nMJOzqJUnEKnaxxUaUPebNSachHJNh=299149806.281699741036803653795709337909f;if (nMJOzqJUnEKnaxxUaUPebNSachHJNh - nMJOzqJUnEKnaxxUaUPebNSachHJNh> 0.00000001 ) nMJOzqJUnEKnaxxUaUPebNSachHJNh=1144483095.184588935235295251232169658336f; else nMJOzqJUnEKnaxxUaUPebNSachHJNh=1599375895.285205342573966161428787702593f;if (nMJOzqJUnEKnaxxUaUPebNSachHJNh - nMJOzqJUnEKnaxxUaUPebNSachHJNh> 0.00000001 ) nMJOzqJUnEKnaxxUaUPebNSachHJNh=1016472625.160005000002986031460792861950f; else nMJOzqJUnEKnaxxUaUPebNSachHJNh=277720834.486052397116252791188224447925f;int IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa=2147217958;if (IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa == IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa- 1 ) IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa=867053745; else IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa=1963057043;if (IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa == IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa- 0 ) IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa=1965298257; else IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa=1936240522;if (IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa == IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa- 0 ) IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa=881458346; else IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa=1619359538;if (IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa == IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa- 0 ) IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa=1125231431; else IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa=1044282637;if (IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa == IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa- 1 ) IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa=1559069124; else IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa=946006793;if (IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa == IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa- 1 ) IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa=1182964577; else IaAfJhuOWkSWNWWcVzGjCnnLKfIeEa=111721013;float JZhMwjjyxQSohTezPyoKREYansUSAh=648529318.382340958071952437649677660379f;if (JZhMwjjyxQSohTezPyoKREYansUSAh - JZhMwjjyxQSohTezPyoKREYansUSAh> 0.00000001 ) JZhMwjjyxQSohTezPyoKREYansUSAh=1630484228.226181712672573695424097481678f; else JZhMwjjyxQSohTezPyoKREYansUSAh=1423318327.069986688173618821418606633081f;if (JZhMwjjyxQSohTezPyoKREYansUSAh - JZhMwjjyxQSohTezPyoKREYansUSAh> 0.00000001 ) JZhMwjjyxQSohTezPyoKREYansUSAh=153265740.024471369801735249076127056335f; else JZhMwjjyxQSohTezPyoKREYansUSAh=951440586.371816925756271807259511500842f;if (JZhMwjjyxQSohTezPyoKREYansUSAh - JZhMwjjyxQSohTezPyoKREYansUSAh> 0.00000001 ) JZhMwjjyxQSohTezPyoKREYansUSAh=1981135721.514454192033532006636029786042f; else JZhMwjjyxQSohTezPyoKREYansUSAh=593534439.719325382871532718051431769587f;if (JZhMwjjyxQSohTezPyoKREYansUSAh - JZhMwjjyxQSohTezPyoKREYansUSAh> 0.00000001 ) JZhMwjjyxQSohTezPyoKREYansUSAh=921913478.816133422882597650708919239876f; else JZhMwjjyxQSohTezPyoKREYansUSAh=2044356167.680606550030738905522999001790f;if (JZhMwjjyxQSohTezPyoKREYansUSAh - JZhMwjjyxQSohTezPyoKREYansUSAh> 0.00000001 ) JZhMwjjyxQSohTezPyoKREYansUSAh=1814372102.576620299110187929244446480260f; else JZhMwjjyxQSohTezPyoKREYansUSAh=922563740.696067422871642392328366713125f;if (JZhMwjjyxQSohTezPyoKREYansUSAh - JZhMwjjyxQSohTezPyoKREYansUSAh> 0.00000001 ) JZhMwjjyxQSohTezPyoKREYansUSAh=803421995.114963413051847490429905333145f; else JZhMwjjyxQSohTezPyoKREYansUSAh=813691340.440777879099937762033470732973f; }
 JZhMwjjyxQSohTezPyoKREYansUSAhy::JZhMwjjyxQSohTezPyoKREYansUSAhy()
 { this->eyfgvWVeeWBL("kKMlLwmVVVXBFApsdBCFTqlIFAHZEOeyfgvWVeeWBLj", true, 430872290, 393558056, 1621477146); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class GPvGXqHcpedOFHoeBClKrXQblWAuGuy
 { 
public: bool ikNwsMlRsCuoajtjanoELDZXiyUVAp; double ikNwsMlRsCuoajtjanoELDZXiyUVApGPvGXqHcpedOFHoeBClKrXQblWAuGu; GPvGXqHcpedOFHoeBClKrXQblWAuGuy(); void NRBjBDgegHtb(string ikNwsMlRsCuoajtjanoELDZXiyUVApNRBjBDgegHtb, bool MVvcuesLwRQlsIwjqsrnaHSpqeKqNt, int sPQDpZsxoPrPrWoaLQazHbdNPFDFlP, float TCyCZAUpTXqsDcvKQvBQTuPNlQmDnj, long VnLZwoRoxoFVwDJOYAREpHEVvYmkQI);
 protected: bool ikNwsMlRsCuoajtjanoELDZXiyUVApo; double ikNwsMlRsCuoajtjanoELDZXiyUVApGPvGXqHcpedOFHoeBClKrXQblWAuGuf; void NRBjBDgegHtbu(string ikNwsMlRsCuoajtjanoELDZXiyUVApNRBjBDgegHtbg, bool MVvcuesLwRQlsIwjqsrnaHSpqeKqNte, int sPQDpZsxoPrPrWoaLQazHbdNPFDFlPr, float TCyCZAUpTXqsDcvKQvBQTuPNlQmDnjw, long VnLZwoRoxoFVwDJOYAREpHEVvYmkQIn);
 private: bool ikNwsMlRsCuoajtjanoELDZXiyUVApMVvcuesLwRQlsIwjqsrnaHSpqeKqNt; double ikNwsMlRsCuoajtjanoELDZXiyUVApTCyCZAUpTXqsDcvKQvBQTuPNlQmDnjGPvGXqHcpedOFHoeBClKrXQblWAuGu;
 void NRBjBDgegHtbv(string MVvcuesLwRQlsIwjqsrnaHSpqeKqNtNRBjBDgegHtb, bool MVvcuesLwRQlsIwjqsrnaHSpqeKqNtsPQDpZsxoPrPrWoaLQazHbdNPFDFlP, int sPQDpZsxoPrPrWoaLQazHbdNPFDFlPikNwsMlRsCuoajtjanoELDZXiyUVAp, float TCyCZAUpTXqsDcvKQvBQTuPNlQmDnjVnLZwoRoxoFVwDJOYAREpHEVvYmkQI, long VnLZwoRoxoFVwDJOYAREpHEVvYmkQIMVvcuesLwRQlsIwjqsrnaHSpqeKqNt); };
 void GPvGXqHcpedOFHoeBClKrXQblWAuGuy::NRBjBDgegHtb(string ikNwsMlRsCuoajtjanoELDZXiyUVApNRBjBDgegHtb, bool MVvcuesLwRQlsIwjqsrnaHSpqeKqNt, int sPQDpZsxoPrPrWoaLQazHbdNPFDFlP, float TCyCZAUpTXqsDcvKQvBQTuPNlQmDnj, long VnLZwoRoxoFVwDJOYAREpHEVvYmkQI)
 { long zEbHuHyYInovwQNAWjTZBzHdhQpjyr=162094883;if (zEbHuHyYInovwQNAWjTZBzHdhQpjyr == zEbHuHyYInovwQNAWjTZBzHdhQpjyr- 0 ) zEbHuHyYInovwQNAWjTZBzHdhQpjyr=590073689; else zEbHuHyYInovwQNAWjTZBzHdhQpjyr=1980671964;if (zEbHuHyYInovwQNAWjTZBzHdhQpjyr == zEbHuHyYInovwQNAWjTZBzHdhQpjyr- 1 ) zEbHuHyYInovwQNAWjTZBzHdhQpjyr=503273780; else zEbHuHyYInovwQNAWjTZBzHdhQpjyr=713168873;if (zEbHuHyYInovwQNAWjTZBzHdhQpjyr == zEbHuHyYInovwQNAWjTZBzHdhQpjyr- 1 ) zEbHuHyYInovwQNAWjTZBzHdhQpjyr=1584098171; else zEbHuHyYInovwQNAWjTZBzHdhQpjyr=1947830878;if (zEbHuHyYInovwQNAWjTZBzHdhQpjyr == zEbHuHyYInovwQNAWjTZBzHdhQpjyr- 1 ) zEbHuHyYInovwQNAWjTZBzHdhQpjyr=550049461; else zEbHuHyYInovwQNAWjTZBzHdhQpjyr=1116183838;if (zEbHuHyYInovwQNAWjTZBzHdhQpjyr == zEbHuHyYInovwQNAWjTZBzHdhQpjyr- 0 ) zEbHuHyYInovwQNAWjTZBzHdhQpjyr=1945016611; else zEbHuHyYInovwQNAWjTZBzHdhQpjyr=1085801456;if (zEbHuHyYInovwQNAWjTZBzHdhQpjyr == zEbHuHyYInovwQNAWjTZBzHdhQpjyr- 0 ) zEbHuHyYInovwQNAWjTZBzHdhQpjyr=265597549; else zEbHuHyYInovwQNAWjTZBzHdhQpjyr=2126742299;float mCAjiUgaNMlyotpZUGXAphOqGhvuoA=244334671.671307485656457455429062735358f;if (mCAjiUgaNMlyotpZUGXAphOqGhvuoA - mCAjiUgaNMlyotpZUGXAphOqGhvuoA> 0.00000001 ) mCAjiUgaNMlyotpZUGXAphOqGhvuoA=1800142901.880610179814031988923971571968f; else mCAjiUgaNMlyotpZUGXAphOqGhvuoA=1722183446.213788785838771604195754602915f;if (mCAjiUgaNMlyotpZUGXAphOqGhvuoA - mCAjiUgaNMlyotpZUGXAphOqGhvuoA> 0.00000001 ) mCAjiUgaNMlyotpZUGXAphOqGhvuoA=484898436.921161135420419521107898366029f; else mCAjiUgaNMlyotpZUGXAphOqGhvuoA=991363017.787001861058077645529192914491f;if (mCAjiUgaNMlyotpZUGXAphOqGhvuoA - mCAjiUgaNMlyotpZUGXAphOqGhvuoA> 0.00000001 ) mCAjiUgaNMlyotpZUGXAphOqGhvuoA=1734824299.522685341886116993282260169629f; else mCAjiUgaNMlyotpZUGXAphOqGhvuoA=639911011.848064749330475319703121947263f;if (mCAjiUgaNMlyotpZUGXAphOqGhvuoA - mCAjiUgaNMlyotpZUGXAphOqGhvuoA> 0.00000001 ) mCAjiUgaNMlyotpZUGXAphOqGhvuoA=640159757.683314915335496389686192772822f; else mCAjiUgaNMlyotpZUGXAphOqGhvuoA=2109842250.758786727151457884676868759164f;if (mCAjiUgaNMlyotpZUGXAphOqGhvuoA - mCAjiUgaNMlyotpZUGXAphOqGhvuoA> 0.00000001 ) mCAjiUgaNMlyotpZUGXAphOqGhvuoA=1951493988.210468772167364675690935700103f; else mCAjiUgaNMlyotpZUGXAphOqGhvuoA=720433048.776404110012261061428468539586f;if (mCAjiUgaNMlyotpZUGXAphOqGhvuoA - mCAjiUgaNMlyotpZUGXAphOqGhvuoA> 0.00000001 ) mCAjiUgaNMlyotpZUGXAphOqGhvuoA=1471306097.977940197157678978574078532807f; else mCAjiUgaNMlyotpZUGXAphOqGhvuoA=2044278100.674415542297448290166211390051f;int PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ=1142808678;if (PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ == PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ- 0 ) PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ=453834105; else PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ=1079796446;if (PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ == PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ- 1 ) PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ=1738994279; else PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ=439397106;if (PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ == PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ- 1 ) PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ=815846960; else PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ=274955974;if (PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ == PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ- 0 ) PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ=728399082; else PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ=1540863135;if (PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ == PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ- 0 ) PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ=609396402; else PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ=655816190;if (PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ == PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ- 1 ) PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ=1189743454; else PyxZoPLaOVUcOpkdPzQXIoREUgPEMZ=1057391640;long ArwVCVTbBUUMLNhdQVzgIabQmkEOZD=59499978;if (ArwVCVTbBUUMLNhdQVzgIabQmkEOZD == ArwVCVTbBUUMLNhdQVzgIabQmkEOZD- 0 ) ArwVCVTbBUUMLNhdQVzgIabQmkEOZD=100906191; else ArwVCVTbBUUMLNhdQVzgIabQmkEOZD=1630918654;if (ArwVCVTbBUUMLNhdQVzgIabQmkEOZD == ArwVCVTbBUUMLNhdQVzgIabQmkEOZD- 1 ) ArwVCVTbBUUMLNhdQVzgIabQmkEOZD=1904545538; else ArwVCVTbBUUMLNhdQVzgIabQmkEOZD=826734747;if (ArwVCVTbBUUMLNhdQVzgIabQmkEOZD == ArwVCVTbBUUMLNhdQVzgIabQmkEOZD- 1 ) ArwVCVTbBUUMLNhdQVzgIabQmkEOZD=832379449; else ArwVCVTbBUUMLNhdQVzgIabQmkEOZD=325994767;if (ArwVCVTbBUUMLNhdQVzgIabQmkEOZD == ArwVCVTbBUUMLNhdQVzgIabQmkEOZD- 0 ) ArwVCVTbBUUMLNhdQVzgIabQmkEOZD=574754231; else ArwVCVTbBUUMLNhdQVzgIabQmkEOZD=2097646325;if (ArwVCVTbBUUMLNhdQVzgIabQmkEOZD == ArwVCVTbBUUMLNhdQVzgIabQmkEOZD- 1 ) ArwVCVTbBUUMLNhdQVzgIabQmkEOZD=1340512207; else ArwVCVTbBUUMLNhdQVzgIabQmkEOZD=540915177;if (ArwVCVTbBUUMLNhdQVzgIabQmkEOZD == ArwVCVTbBUUMLNhdQVzgIabQmkEOZD- 0 ) ArwVCVTbBUUMLNhdQVzgIabQmkEOZD=909158532; else ArwVCVTbBUUMLNhdQVzgIabQmkEOZD=863665123;double RdxgZACoACXvwzezUGKbzHhDCQefBL=711724985.192958900888368008752855407817;if (RdxgZACoACXvwzezUGKbzHhDCQefBL == RdxgZACoACXvwzezUGKbzHhDCQefBL ) RdxgZACoACXvwzezUGKbzHhDCQefBL=544656351.404105072412065730874374944855; else RdxgZACoACXvwzezUGKbzHhDCQefBL=1270272518.119358426313634189087714423037;if (RdxgZACoACXvwzezUGKbzHhDCQefBL == RdxgZACoACXvwzezUGKbzHhDCQefBL ) RdxgZACoACXvwzezUGKbzHhDCQefBL=1672908194.233731372732567175096587941603; else RdxgZACoACXvwzezUGKbzHhDCQefBL=1015762988.659918190482846635745904276699;if (RdxgZACoACXvwzezUGKbzHhDCQefBL == RdxgZACoACXvwzezUGKbzHhDCQefBL ) RdxgZACoACXvwzezUGKbzHhDCQefBL=1952038520.187929365908571484142697975675; else RdxgZACoACXvwzezUGKbzHhDCQefBL=806642483.351141502837194754480256859115;if (RdxgZACoACXvwzezUGKbzHhDCQefBL == RdxgZACoACXvwzezUGKbzHhDCQefBL ) RdxgZACoACXvwzezUGKbzHhDCQefBL=1575971358.589165844009341561423120521549; else RdxgZACoACXvwzezUGKbzHhDCQefBL=1525186927.249746903991049292549989346065;if (RdxgZACoACXvwzezUGKbzHhDCQefBL == RdxgZACoACXvwzezUGKbzHhDCQefBL ) RdxgZACoACXvwzezUGKbzHhDCQefBL=993784364.541696061676953229133000047150; else RdxgZACoACXvwzezUGKbzHhDCQefBL=1091518948.094846156434108413613804264166;if (RdxgZACoACXvwzezUGKbzHhDCQefBL == RdxgZACoACXvwzezUGKbzHhDCQefBL ) RdxgZACoACXvwzezUGKbzHhDCQefBL=381966615.092168311827471091374454830179; else RdxgZACoACXvwzezUGKbzHhDCQefBL=1079008109.310154304010290653337389265834;long IXMaGnQNhislReSHhXMpjoSJxMCqty=1731634094;if (IXMaGnQNhislReSHhXMpjoSJxMCqty == IXMaGnQNhislReSHhXMpjoSJxMCqty- 0 ) IXMaGnQNhislReSHhXMpjoSJxMCqty=2031530161; else IXMaGnQNhislReSHhXMpjoSJxMCqty=1411806119;if (IXMaGnQNhislReSHhXMpjoSJxMCqty == IXMaGnQNhislReSHhXMpjoSJxMCqty- 1 ) IXMaGnQNhislReSHhXMpjoSJxMCqty=1943371909; else IXMaGnQNhislReSHhXMpjoSJxMCqty=787226558;if (IXMaGnQNhislReSHhXMpjoSJxMCqty == IXMaGnQNhislReSHhXMpjoSJxMCqty- 1 ) IXMaGnQNhislReSHhXMpjoSJxMCqty=1476229735; else IXMaGnQNhislReSHhXMpjoSJxMCqty=1861894005;if (IXMaGnQNhislReSHhXMpjoSJxMCqty == IXMaGnQNhislReSHhXMpjoSJxMCqty- 1 ) IXMaGnQNhislReSHhXMpjoSJxMCqty=310429229; else IXMaGnQNhislReSHhXMpjoSJxMCqty=127839338;if (IXMaGnQNhislReSHhXMpjoSJxMCqty == IXMaGnQNhislReSHhXMpjoSJxMCqty- 1 ) IXMaGnQNhislReSHhXMpjoSJxMCqty=505042215; else IXMaGnQNhislReSHhXMpjoSJxMCqty=1788831601;if (IXMaGnQNhislReSHhXMpjoSJxMCqty == IXMaGnQNhislReSHhXMpjoSJxMCqty- 0 ) IXMaGnQNhislReSHhXMpjoSJxMCqty=1246368546; else IXMaGnQNhislReSHhXMpjoSJxMCqty=274931783;long yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU=2103625041;if (yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU == yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU- 0 ) yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU=1957777564; else yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU=478018469;if (yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU == yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU- 1 ) yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU=512735289; else yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU=1517110240;if (yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU == yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU- 0 ) yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU=694435347; else yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU=1968107193;if (yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU == yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU- 0 ) yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU=254945105; else yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU=1441747355;if (yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU == yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU- 0 ) yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU=54947296; else yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU=379358611;if (yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU == yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU- 0 ) yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU=1980434030; else yKCVOYzrVhXIbFnuyrjvUVUSnEFSkU=1888344148;long rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu=1757744090;if (rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu == rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu- 0 ) rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu=1218455685; else rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu=1325008085;if (rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu == rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu- 0 ) rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu=979618609; else rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu=1212202309;if (rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu == rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu- 1 ) rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu=857366746; else rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu=58295715;if (rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu == rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu- 1 ) rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu=1382978897; else rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu=830245993;if (rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu == rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu- 0 ) rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu=959843289; else rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu=1545068856;if (rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu == rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu- 0 ) rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu=510720773; else rFIlIFMdhfHXeEnTAwYmSBTnyPtnhu=163206919;int JrHLusIvrnvKtZezpHQeBrsCOXVrXL=1129050405;if (JrHLusIvrnvKtZezpHQeBrsCOXVrXL == JrHLusIvrnvKtZezpHQeBrsCOXVrXL- 0 ) JrHLusIvrnvKtZezpHQeBrsCOXVrXL=27180594; else JrHLusIvrnvKtZezpHQeBrsCOXVrXL=425825221;if (JrHLusIvrnvKtZezpHQeBrsCOXVrXL == JrHLusIvrnvKtZezpHQeBrsCOXVrXL- 1 ) JrHLusIvrnvKtZezpHQeBrsCOXVrXL=1268714192; else JrHLusIvrnvKtZezpHQeBrsCOXVrXL=1119632735;if (JrHLusIvrnvKtZezpHQeBrsCOXVrXL == JrHLusIvrnvKtZezpHQeBrsCOXVrXL- 0 ) JrHLusIvrnvKtZezpHQeBrsCOXVrXL=1217516475; else JrHLusIvrnvKtZezpHQeBrsCOXVrXL=2100316546;if (JrHLusIvrnvKtZezpHQeBrsCOXVrXL == JrHLusIvrnvKtZezpHQeBrsCOXVrXL- 0 ) JrHLusIvrnvKtZezpHQeBrsCOXVrXL=1037198747; else JrHLusIvrnvKtZezpHQeBrsCOXVrXL=63422968;if (JrHLusIvrnvKtZezpHQeBrsCOXVrXL == JrHLusIvrnvKtZezpHQeBrsCOXVrXL- 1 ) JrHLusIvrnvKtZezpHQeBrsCOXVrXL=1156861014; else JrHLusIvrnvKtZezpHQeBrsCOXVrXL=501624554;if (JrHLusIvrnvKtZezpHQeBrsCOXVrXL == JrHLusIvrnvKtZezpHQeBrsCOXVrXL- 1 ) JrHLusIvrnvKtZezpHQeBrsCOXVrXL=732279262; else JrHLusIvrnvKtZezpHQeBrsCOXVrXL=1096871124;float eUFuCtvkaLUyOCirqnMSpjFDxuYmPT=1009296093.653813424911981683694370185232f;if (eUFuCtvkaLUyOCirqnMSpjFDxuYmPT - eUFuCtvkaLUyOCirqnMSpjFDxuYmPT> 0.00000001 ) eUFuCtvkaLUyOCirqnMSpjFDxuYmPT=1288836480.835877149777642914388580739383f; else eUFuCtvkaLUyOCirqnMSpjFDxuYmPT=1636108103.829219611564402883828206545525f;if (eUFuCtvkaLUyOCirqnMSpjFDxuYmPT - eUFuCtvkaLUyOCirqnMSpjFDxuYmPT> 0.00000001 ) eUFuCtvkaLUyOCirqnMSpjFDxuYmPT=1203959745.897796843430393528905756673488f; else eUFuCtvkaLUyOCirqnMSpjFDxuYmPT=188273004.420778882705050891403872024112f;if (eUFuCtvkaLUyOCirqnMSpjFDxuYmPT - eUFuCtvkaLUyOCirqnMSpjFDxuYmPT> 0.00000001 ) eUFuCtvkaLUyOCirqnMSpjFDxuYmPT=1032598997.456602173684720170789218575886f; else eUFuCtvkaLUyOCirqnMSpjFDxuYmPT=1999619692.477036946972983915295454334913f;if (eUFuCtvkaLUyOCirqnMSpjFDxuYmPT - eUFuCtvkaLUyOCirqnMSpjFDxuYmPT> 0.00000001 ) eUFuCtvkaLUyOCirqnMSpjFDxuYmPT=1246258295.740821870534492733577069103481f; else eUFuCtvkaLUyOCirqnMSpjFDxuYmPT=1899481849.203424327325686207360245788172f;if (eUFuCtvkaLUyOCirqnMSpjFDxuYmPT - eUFuCtvkaLUyOCirqnMSpjFDxuYmPT> 0.00000001 ) eUFuCtvkaLUyOCirqnMSpjFDxuYmPT=1242523219.950043118852736259400722117984f; else eUFuCtvkaLUyOCirqnMSpjFDxuYmPT=1829972943.736547675420052851053988334993f;if (eUFuCtvkaLUyOCirqnMSpjFDxuYmPT - eUFuCtvkaLUyOCirqnMSpjFDxuYmPT> 0.00000001 ) eUFuCtvkaLUyOCirqnMSpjFDxuYmPT=557266951.724423730979518187036703286832f; else eUFuCtvkaLUyOCirqnMSpjFDxuYmPT=1296850138.488280674023219557026796803224f;int rzmZvCRQqXFPDGQZpUkSqsBhktLVml=901391368;if (rzmZvCRQqXFPDGQZpUkSqsBhktLVml == rzmZvCRQqXFPDGQZpUkSqsBhktLVml- 0 ) rzmZvCRQqXFPDGQZpUkSqsBhktLVml=367452773; else rzmZvCRQqXFPDGQZpUkSqsBhktLVml=851383625;if (rzmZvCRQqXFPDGQZpUkSqsBhktLVml == rzmZvCRQqXFPDGQZpUkSqsBhktLVml- 0 ) rzmZvCRQqXFPDGQZpUkSqsBhktLVml=1861163554; else rzmZvCRQqXFPDGQZpUkSqsBhktLVml=1665686578;if (rzmZvCRQqXFPDGQZpUkSqsBhktLVml == rzmZvCRQqXFPDGQZpUkSqsBhktLVml- 1 ) rzmZvCRQqXFPDGQZpUkSqsBhktLVml=374515577; else rzmZvCRQqXFPDGQZpUkSqsBhktLVml=1527846033;if (rzmZvCRQqXFPDGQZpUkSqsBhktLVml == rzmZvCRQqXFPDGQZpUkSqsBhktLVml- 0 ) rzmZvCRQqXFPDGQZpUkSqsBhktLVml=540227615; else rzmZvCRQqXFPDGQZpUkSqsBhktLVml=1265622786;if (rzmZvCRQqXFPDGQZpUkSqsBhktLVml == rzmZvCRQqXFPDGQZpUkSqsBhktLVml- 1 ) rzmZvCRQqXFPDGQZpUkSqsBhktLVml=287809338; else rzmZvCRQqXFPDGQZpUkSqsBhktLVml=916195641;if (rzmZvCRQqXFPDGQZpUkSqsBhktLVml == rzmZvCRQqXFPDGQZpUkSqsBhktLVml- 0 ) rzmZvCRQqXFPDGQZpUkSqsBhktLVml=55341295; else rzmZvCRQqXFPDGQZpUkSqsBhktLVml=355742192;long ZBJOlAzQiBramdJvzLifyEvlloLKoz=390274561;if (ZBJOlAzQiBramdJvzLifyEvlloLKoz == ZBJOlAzQiBramdJvzLifyEvlloLKoz- 0 ) ZBJOlAzQiBramdJvzLifyEvlloLKoz=1361257670; else ZBJOlAzQiBramdJvzLifyEvlloLKoz=1648908327;if (ZBJOlAzQiBramdJvzLifyEvlloLKoz == ZBJOlAzQiBramdJvzLifyEvlloLKoz- 1 ) ZBJOlAzQiBramdJvzLifyEvlloLKoz=1021854086; else ZBJOlAzQiBramdJvzLifyEvlloLKoz=71649481;if (ZBJOlAzQiBramdJvzLifyEvlloLKoz == ZBJOlAzQiBramdJvzLifyEvlloLKoz- 1 ) ZBJOlAzQiBramdJvzLifyEvlloLKoz=97079830; else ZBJOlAzQiBramdJvzLifyEvlloLKoz=1971116481;if (ZBJOlAzQiBramdJvzLifyEvlloLKoz == ZBJOlAzQiBramdJvzLifyEvlloLKoz- 0 ) ZBJOlAzQiBramdJvzLifyEvlloLKoz=96451408; else ZBJOlAzQiBramdJvzLifyEvlloLKoz=175044549;if (ZBJOlAzQiBramdJvzLifyEvlloLKoz == ZBJOlAzQiBramdJvzLifyEvlloLKoz- 0 ) ZBJOlAzQiBramdJvzLifyEvlloLKoz=1327050573; else ZBJOlAzQiBramdJvzLifyEvlloLKoz=1516715990;if (ZBJOlAzQiBramdJvzLifyEvlloLKoz == ZBJOlAzQiBramdJvzLifyEvlloLKoz- 1 ) ZBJOlAzQiBramdJvzLifyEvlloLKoz=2024593163; else ZBJOlAzQiBramdJvzLifyEvlloLKoz=1535027179;long SkphWtSkXhEPQqWKDGoYwBdWCBpDzn=1065122828;if (SkphWtSkXhEPQqWKDGoYwBdWCBpDzn == SkphWtSkXhEPQqWKDGoYwBdWCBpDzn- 0 ) SkphWtSkXhEPQqWKDGoYwBdWCBpDzn=767634783; else SkphWtSkXhEPQqWKDGoYwBdWCBpDzn=1978371541;if (SkphWtSkXhEPQqWKDGoYwBdWCBpDzn == SkphWtSkXhEPQqWKDGoYwBdWCBpDzn- 1 ) SkphWtSkXhEPQqWKDGoYwBdWCBpDzn=729418351; else SkphWtSkXhEPQqWKDGoYwBdWCBpDzn=1177559997;if (SkphWtSkXhEPQqWKDGoYwBdWCBpDzn == SkphWtSkXhEPQqWKDGoYwBdWCBpDzn- 1 ) SkphWtSkXhEPQqWKDGoYwBdWCBpDzn=855888821; else SkphWtSkXhEPQqWKDGoYwBdWCBpDzn=665608255;if (SkphWtSkXhEPQqWKDGoYwBdWCBpDzn == SkphWtSkXhEPQqWKDGoYwBdWCBpDzn- 0 ) SkphWtSkXhEPQqWKDGoYwBdWCBpDzn=634780516; else SkphWtSkXhEPQqWKDGoYwBdWCBpDzn=1908474094;if (SkphWtSkXhEPQqWKDGoYwBdWCBpDzn == SkphWtSkXhEPQqWKDGoYwBdWCBpDzn- 0 ) SkphWtSkXhEPQqWKDGoYwBdWCBpDzn=1095880488; else SkphWtSkXhEPQqWKDGoYwBdWCBpDzn=2085820606;if (SkphWtSkXhEPQqWKDGoYwBdWCBpDzn == SkphWtSkXhEPQqWKDGoYwBdWCBpDzn- 0 ) SkphWtSkXhEPQqWKDGoYwBdWCBpDzn=2047947313; else SkphWtSkXhEPQqWKDGoYwBdWCBpDzn=116403356;int HvwXqGRzJhVRGTDGWljQDfzvCVJoQg=956415446;if (HvwXqGRzJhVRGTDGWljQDfzvCVJoQg == HvwXqGRzJhVRGTDGWljQDfzvCVJoQg- 0 ) HvwXqGRzJhVRGTDGWljQDfzvCVJoQg=881186925; else HvwXqGRzJhVRGTDGWljQDfzvCVJoQg=166992390;if (HvwXqGRzJhVRGTDGWljQDfzvCVJoQg == HvwXqGRzJhVRGTDGWljQDfzvCVJoQg- 0 ) HvwXqGRzJhVRGTDGWljQDfzvCVJoQg=2083076115; else HvwXqGRzJhVRGTDGWljQDfzvCVJoQg=1955961740;if (HvwXqGRzJhVRGTDGWljQDfzvCVJoQg == HvwXqGRzJhVRGTDGWljQDfzvCVJoQg- 0 ) HvwXqGRzJhVRGTDGWljQDfzvCVJoQg=1217367747; else HvwXqGRzJhVRGTDGWljQDfzvCVJoQg=1809823937;if (HvwXqGRzJhVRGTDGWljQDfzvCVJoQg == HvwXqGRzJhVRGTDGWljQDfzvCVJoQg- 0 ) HvwXqGRzJhVRGTDGWljQDfzvCVJoQg=804311538; else HvwXqGRzJhVRGTDGWljQDfzvCVJoQg=543819939;if (HvwXqGRzJhVRGTDGWljQDfzvCVJoQg == HvwXqGRzJhVRGTDGWljQDfzvCVJoQg- 0 ) HvwXqGRzJhVRGTDGWljQDfzvCVJoQg=754932217; else HvwXqGRzJhVRGTDGWljQDfzvCVJoQg=1686119005;if (HvwXqGRzJhVRGTDGWljQDfzvCVJoQg == HvwXqGRzJhVRGTDGWljQDfzvCVJoQg- 1 ) HvwXqGRzJhVRGTDGWljQDfzvCVJoQg=338636005; else HvwXqGRzJhVRGTDGWljQDfzvCVJoQg=1005283234;double BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl=547460729.620647110383208216440108466280;if (BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl == BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl ) BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl=162675446.636529092611949440922934363328; else BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl=1137702114.203075879022584026903647981237;if (BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl == BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl ) BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl=1735591056.687452907577200055134743666610; else BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl=1642612206.465245614223044645704721304589;if (BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl == BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl ) BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl=1704173069.630711317644102270912291785494; else BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl=1918709668.124560344334927063024468854914;if (BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl == BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl ) BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl=1681869387.602353174394769847572358176682; else BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl=695649415.653074368490973081901744345528;if (BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl == BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl ) BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl=1014127811.850374189668028258510688192974; else BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl=1260863639.984175161000245383720702838229;if (BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl == BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl ) BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl=1123252640.322678506419862144482035538334; else BbRMkCDBQvaoWRbUhGTzMXmvbjXtyl=158742871.794378968799731195089595740724;float JgGjJWcsXIqekuOMAVQSQNbdOCybSo=436534470.323826329223476984015038519522f;if (JgGjJWcsXIqekuOMAVQSQNbdOCybSo - JgGjJWcsXIqekuOMAVQSQNbdOCybSo> 0.00000001 ) JgGjJWcsXIqekuOMAVQSQNbdOCybSo=1090734241.178814553669992100392792030641f; else JgGjJWcsXIqekuOMAVQSQNbdOCybSo=1068177717.790725676344246624642934148789f;if (JgGjJWcsXIqekuOMAVQSQNbdOCybSo - JgGjJWcsXIqekuOMAVQSQNbdOCybSo> 0.00000001 ) JgGjJWcsXIqekuOMAVQSQNbdOCybSo=653840254.401208928764505966652700096779f; else JgGjJWcsXIqekuOMAVQSQNbdOCybSo=1376057205.899895304970609854928224780132f;if (JgGjJWcsXIqekuOMAVQSQNbdOCybSo - JgGjJWcsXIqekuOMAVQSQNbdOCybSo> 0.00000001 ) JgGjJWcsXIqekuOMAVQSQNbdOCybSo=1952239788.236907571067378311224386268611f; else JgGjJWcsXIqekuOMAVQSQNbdOCybSo=945042102.527375581248396105758001806084f;if (JgGjJWcsXIqekuOMAVQSQNbdOCybSo - JgGjJWcsXIqekuOMAVQSQNbdOCybSo> 0.00000001 ) JgGjJWcsXIqekuOMAVQSQNbdOCybSo=1995238728.007064000484009709017407463767f; else JgGjJWcsXIqekuOMAVQSQNbdOCybSo=1105294493.950834234211537500221717761170f;if (JgGjJWcsXIqekuOMAVQSQNbdOCybSo - JgGjJWcsXIqekuOMAVQSQNbdOCybSo> 0.00000001 ) JgGjJWcsXIqekuOMAVQSQNbdOCybSo=1174060003.245354437567581797096355734317f; else JgGjJWcsXIqekuOMAVQSQNbdOCybSo=185598412.417518629418386434718519509172f;if (JgGjJWcsXIqekuOMAVQSQNbdOCybSo - JgGjJWcsXIqekuOMAVQSQNbdOCybSo> 0.00000001 ) JgGjJWcsXIqekuOMAVQSQNbdOCybSo=422627780.122125117418878808931171000902f; else JgGjJWcsXIqekuOMAVQSQNbdOCybSo=1317253463.966730737242740175170234215123f;long LEbYZJESoHkHiwsRQHvLbnPKmTIZOa=2090680052;if (LEbYZJESoHkHiwsRQHvLbnPKmTIZOa == LEbYZJESoHkHiwsRQHvLbnPKmTIZOa- 0 ) LEbYZJESoHkHiwsRQHvLbnPKmTIZOa=215763768; else LEbYZJESoHkHiwsRQHvLbnPKmTIZOa=1500486584;if (LEbYZJESoHkHiwsRQHvLbnPKmTIZOa == LEbYZJESoHkHiwsRQHvLbnPKmTIZOa- 0 ) LEbYZJESoHkHiwsRQHvLbnPKmTIZOa=2101211557; else LEbYZJESoHkHiwsRQHvLbnPKmTIZOa=806576283;if (LEbYZJESoHkHiwsRQHvLbnPKmTIZOa == LEbYZJESoHkHiwsRQHvLbnPKmTIZOa- 0 ) LEbYZJESoHkHiwsRQHvLbnPKmTIZOa=1302408694; else LEbYZJESoHkHiwsRQHvLbnPKmTIZOa=930853190;if (LEbYZJESoHkHiwsRQHvLbnPKmTIZOa == LEbYZJESoHkHiwsRQHvLbnPKmTIZOa- 1 ) LEbYZJESoHkHiwsRQHvLbnPKmTIZOa=1399726024; else LEbYZJESoHkHiwsRQHvLbnPKmTIZOa=1109960904;if (LEbYZJESoHkHiwsRQHvLbnPKmTIZOa == LEbYZJESoHkHiwsRQHvLbnPKmTIZOa- 0 ) LEbYZJESoHkHiwsRQHvLbnPKmTIZOa=632606650; else LEbYZJESoHkHiwsRQHvLbnPKmTIZOa=1387426700;if (LEbYZJESoHkHiwsRQHvLbnPKmTIZOa == LEbYZJESoHkHiwsRQHvLbnPKmTIZOa- 0 ) LEbYZJESoHkHiwsRQHvLbnPKmTIZOa=941833929; else LEbYZJESoHkHiwsRQHvLbnPKmTIZOa=2103987755;double zKfHXlNseQjOxrBwmMkNAcEuvVRKnA=774884165.908374715612982741139229747638;if (zKfHXlNseQjOxrBwmMkNAcEuvVRKnA == zKfHXlNseQjOxrBwmMkNAcEuvVRKnA ) zKfHXlNseQjOxrBwmMkNAcEuvVRKnA=1307869206.278912821156651186509122189073; else zKfHXlNseQjOxrBwmMkNAcEuvVRKnA=2049585182.818754417102008367867113915784;if (zKfHXlNseQjOxrBwmMkNAcEuvVRKnA == zKfHXlNseQjOxrBwmMkNAcEuvVRKnA ) zKfHXlNseQjOxrBwmMkNAcEuvVRKnA=2062032640.486934890788951823493221771695; else zKfHXlNseQjOxrBwmMkNAcEuvVRKnA=582558800.073472727457728936320330236527;if (zKfHXlNseQjOxrBwmMkNAcEuvVRKnA == zKfHXlNseQjOxrBwmMkNAcEuvVRKnA ) zKfHXlNseQjOxrBwmMkNAcEuvVRKnA=498843829.431682568911604651495547675425; else zKfHXlNseQjOxrBwmMkNAcEuvVRKnA=493999633.620113725747941983678623309411;if (zKfHXlNseQjOxrBwmMkNAcEuvVRKnA == zKfHXlNseQjOxrBwmMkNAcEuvVRKnA ) zKfHXlNseQjOxrBwmMkNAcEuvVRKnA=742704588.824430150194685870770286711892; else zKfHXlNseQjOxrBwmMkNAcEuvVRKnA=1365116991.441502352293468504853084165973;if (zKfHXlNseQjOxrBwmMkNAcEuvVRKnA == zKfHXlNseQjOxrBwmMkNAcEuvVRKnA ) zKfHXlNseQjOxrBwmMkNAcEuvVRKnA=1289947033.947976328352959883921704956754; else zKfHXlNseQjOxrBwmMkNAcEuvVRKnA=630296563.859035669239750213639215235303;if (zKfHXlNseQjOxrBwmMkNAcEuvVRKnA == zKfHXlNseQjOxrBwmMkNAcEuvVRKnA ) zKfHXlNseQjOxrBwmMkNAcEuvVRKnA=468552203.067561403423570215475322665481; else zKfHXlNseQjOxrBwmMkNAcEuvVRKnA=1518574657.136727230091122684502093885690;long wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ=1384630529;if (wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ == wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ- 1 ) wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ=1003126062; else wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ=1180056235;if (wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ == wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ- 0 ) wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ=2090462750; else wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ=570166231;if (wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ == wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ- 0 ) wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ=26320142; else wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ=1719157163;if (wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ == wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ- 1 ) wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ=1281609423; else wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ=159881051;if (wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ == wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ- 0 ) wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ=1815935340; else wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ=1691362997;if (wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ == wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ- 1 ) wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ=743450555; else wKJPtmpWQyoWWtRMxAzqEunrFVLsZJ=564813088;float CHpOLXEZVBDwfnCdGcFBlFLdqNeOre=528491336.092637800688010640392022604887f;if (CHpOLXEZVBDwfnCdGcFBlFLdqNeOre - CHpOLXEZVBDwfnCdGcFBlFLdqNeOre> 0.00000001 ) CHpOLXEZVBDwfnCdGcFBlFLdqNeOre=1712431288.834030464238428591802981218014f; else CHpOLXEZVBDwfnCdGcFBlFLdqNeOre=1795227262.313495408139626460512408695493f;if (CHpOLXEZVBDwfnCdGcFBlFLdqNeOre - CHpOLXEZVBDwfnCdGcFBlFLdqNeOre> 0.00000001 ) CHpOLXEZVBDwfnCdGcFBlFLdqNeOre=9384257.345193614319488447585860201590f; else CHpOLXEZVBDwfnCdGcFBlFLdqNeOre=424870715.112589453396721312026992859590f;if (CHpOLXEZVBDwfnCdGcFBlFLdqNeOre - CHpOLXEZVBDwfnCdGcFBlFLdqNeOre> 0.00000001 ) CHpOLXEZVBDwfnCdGcFBlFLdqNeOre=1862431936.213431790019439025009067532184f; else CHpOLXEZVBDwfnCdGcFBlFLdqNeOre=1822662424.197329340073212857394460135791f;if (CHpOLXEZVBDwfnCdGcFBlFLdqNeOre - CHpOLXEZVBDwfnCdGcFBlFLdqNeOre> 0.00000001 ) CHpOLXEZVBDwfnCdGcFBlFLdqNeOre=1591836223.071702221078614785588589074730f; else CHpOLXEZVBDwfnCdGcFBlFLdqNeOre=1869247782.476910262064733696755022359273f;if (CHpOLXEZVBDwfnCdGcFBlFLdqNeOre - CHpOLXEZVBDwfnCdGcFBlFLdqNeOre> 0.00000001 ) CHpOLXEZVBDwfnCdGcFBlFLdqNeOre=757781996.969170370463463595759919546577f; else CHpOLXEZVBDwfnCdGcFBlFLdqNeOre=736094566.209911567343649377986463515173f;if (CHpOLXEZVBDwfnCdGcFBlFLdqNeOre - CHpOLXEZVBDwfnCdGcFBlFLdqNeOre> 0.00000001 ) CHpOLXEZVBDwfnCdGcFBlFLdqNeOre=1664112897.590336455214951737038969185650f; else CHpOLXEZVBDwfnCdGcFBlFLdqNeOre=672112131.649595331492288361289825564235f;float rAYggXThfWyfhRhkoCsMUTkGmOXyNH=267990726.436559934961281925726995901686f;if (rAYggXThfWyfhRhkoCsMUTkGmOXyNH - rAYggXThfWyfhRhkoCsMUTkGmOXyNH> 0.00000001 ) rAYggXThfWyfhRhkoCsMUTkGmOXyNH=1018749535.506010228517182181503224855176f; else rAYggXThfWyfhRhkoCsMUTkGmOXyNH=1704687047.935857527280732081483029438336f;if (rAYggXThfWyfhRhkoCsMUTkGmOXyNH - rAYggXThfWyfhRhkoCsMUTkGmOXyNH> 0.00000001 ) rAYggXThfWyfhRhkoCsMUTkGmOXyNH=1100861693.516378244566532360298479491752f; else rAYggXThfWyfhRhkoCsMUTkGmOXyNH=1653998709.020177546585802750717925914708f;if (rAYggXThfWyfhRhkoCsMUTkGmOXyNH - rAYggXThfWyfhRhkoCsMUTkGmOXyNH> 0.00000001 ) rAYggXThfWyfhRhkoCsMUTkGmOXyNH=702589564.759265756692471468875214611627f; else rAYggXThfWyfhRhkoCsMUTkGmOXyNH=50484540.385983675801413666465703455665f;if (rAYggXThfWyfhRhkoCsMUTkGmOXyNH - rAYggXThfWyfhRhkoCsMUTkGmOXyNH> 0.00000001 ) rAYggXThfWyfhRhkoCsMUTkGmOXyNH=748564023.423041869890407490216627279254f; else rAYggXThfWyfhRhkoCsMUTkGmOXyNH=1736195690.912049948381767630607656091698f;if (rAYggXThfWyfhRhkoCsMUTkGmOXyNH - rAYggXThfWyfhRhkoCsMUTkGmOXyNH> 0.00000001 ) rAYggXThfWyfhRhkoCsMUTkGmOXyNH=767975759.261609957965203596164066746605f; else rAYggXThfWyfhRhkoCsMUTkGmOXyNH=1902151589.936113117989484246450563430597f;if (rAYggXThfWyfhRhkoCsMUTkGmOXyNH - rAYggXThfWyfhRhkoCsMUTkGmOXyNH> 0.00000001 ) rAYggXThfWyfhRhkoCsMUTkGmOXyNH=914107107.335214029059028679261135648379f; else rAYggXThfWyfhRhkoCsMUTkGmOXyNH=954964479.462076438122741265596233721167f;int iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX=1398696060;if (iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX == iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX- 0 ) iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX=211573367; else iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX=2087544514;if (iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX == iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX- 1 ) iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX=1953627016; else iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX=1848338273;if (iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX == iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX- 0 ) iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX=355231667; else iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX=2091290097;if (iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX == iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX- 1 ) iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX=1669682240; else iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX=1327947285;if (iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX == iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX- 1 ) iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX=1735703659; else iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX=221214968;if (iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX == iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX- 0 ) iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX=1959867882; else iYCgwgwZPIWFFVrXrXTsRrqLLOuTaX=1437709223;double PrLsWAePzXDQAxahGUzaKdQmhRfrfT=1934783973.664635915117552712242918446769;if (PrLsWAePzXDQAxahGUzaKdQmhRfrfT == PrLsWAePzXDQAxahGUzaKdQmhRfrfT ) PrLsWAePzXDQAxahGUzaKdQmhRfrfT=2077483981.242230666351642874119294085739; else PrLsWAePzXDQAxahGUzaKdQmhRfrfT=475459829.035607533325609138768276006935;if (PrLsWAePzXDQAxahGUzaKdQmhRfrfT == PrLsWAePzXDQAxahGUzaKdQmhRfrfT ) PrLsWAePzXDQAxahGUzaKdQmhRfrfT=1693547200.504833704544289479696817297071; else PrLsWAePzXDQAxahGUzaKdQmhRfrfT=446004773.171892297753423409773744070765;if (PrLsWAePzXDQAxahGUzaKdQmhRfrfT == PrLsWAePzXDQAxahGUzaKdQmhRfrfT ) PrLsWAePzXDQAxahGUzaKdQmhRfrfT=937872927.774448450728304359589940843782; else PrLsWAePzXDQAxahGUzaKdQmhRfrfT=912208134.363309908605905555173139694461;if (PrLsWAePzXDQAxahGUzaKdQmhRfrfT == PrLsWAePzXDQAxahGUzaKdQmhRfrfT ) PrLsWAePzXDQAxahGUzaKdQmhRfrfT=1973385419.578957600645208504630971934572; else PrLsWAePzXDQAxahGUzaKdQmhRfrfT=1661946748.655823806805554904129094261712;if (PrLsWAePzXDQAxahGUzaKdQmhRfrfT == PrLsWAePzXDQAxahGUzaKdQmhRfrfT ) PrLsWAePzXDQAxahGUzaKdQmhRfrfT=1046663239.414919148681983552814156367176; else PrLsWAePzXDQAxahGUzaKdQmhRfrfT=420672214.310459275586664812172900947865;if (PrLsWAePzXDQAxahGUzaKdQmhRfrfT == PrLsWAePzXDQAxahGUzaKdQmhRfrfT ) PrLsWAePzXDQAxahGUzaKdQmhRfrfT=1804378603.308237263538843950255610542052; else PrLsWAePzXDQAxahGUzaKdQmhRfrfT=2107763994.251029843876975427819861080889;double MCJMshlHUDHhVsylWcENbJXOWsBBWw=579890496.027749946077079562871657278320;if (MCJMshlHUDHhVsylWcENbJXOWsBBWw == MCJMshlHUDHhVsylWcENbJXOWsBBWw ) MCJMshlHUDHhVsylWcENbJXOWsBBWw=1521669395.213699440726565422294898648068; else MCJMshlHUDHhVsylWcENbJXOWsBBWw=265416837.162687449022325629286614753605;if (MCJMshlHUDHhVsylWcENbJXOWsBBWw == MCJMshlHUDHhVsylWcENbJXOWsBBWw ) MCJMshlHUDHhVsylWcENbJXOWsBBWw=460538764.249267408918338267313998356994; else MCJMshlHUDHhVsylWcENbJXOWsBBWw=522240120.585588417932148839659630246201;if (MCJMshlHUDHhVsylWcENbJXOWsBBWw == MCJMshlHUDHhVsylWcENbJXOWsBBWw ) MCJMshlHUDHhVsylWcENbJXOWsBBWw=75007998.251543495095158185053734505535; else MCJMshlHUDHhVsylWcENbJXOWsBBWw=2060508037.332688170655577459216125952805;if (MCJMshlHUDHhVsylWcENbJXOWsBBWw == MCJMshlHUDHhVsylWcENbJXOWsBBWw ) MCJMshlHUDHhVsylWcENbJXOWsBBWw=2060242706.897087458012651937815732921783; else MCJMshlHUDHhVsylWcENbJXOWsBBWw=1074943085.747686859110231567403609017616;if (MCJMshlHUDHhVsylWcENbJXOWsBBWw == MCJMshlHUDHhVsylWcENbJXOWsBBWw ) MCJMshlHUDHhVsylWcENbJXOWsBBWw=35004807.688547508619881661157438124977; else MCJMshlHUDHhVsylWcENbJXOWsBBWw=1920398055.934454379406090583856656925053;if (MCJMshlHUDHhVsylWcENbJXOWsBBWw == MCJMshlHUDHhVsylWcENbJXOWsBBWw ) MCJMshlHUDHhVsylWcENbJXOWsBBWw=2065778697.374178553830376145964212249853; else MCJMshlHUDHhVsylWcENbJXOWsBBWw=481374596.942724302203002648198019926294;float jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm=158478467.203920786383946870102467763219f;if (jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm - jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm> 0.00000001 ) jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm=57087878.181277219822608860059414885737f; else jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm=1126806781.768721387977249096090427188840f;if (jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm - jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm> 0.00000001 ) jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm=1024964145.268217499869319338590085744275f; else jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm=1422572896.354871903260724212885136139468f;if (jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm - jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm> 0.00000001 ) jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm=631827947.333451564853543036154349032975f; else jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm=915904045.355694848385804360411431410452f;if (jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm - jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm> 0.00000001 ) jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm=460823133.079060905930898954688367503529f; else jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm=1446848880.449641695547559164367687572357f;if (jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm - jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm> 0.00000001 ) jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm=114159095.858769136846024181944927396317f; else jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm=291680268.034880990014963167449152393257f;if (jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm - jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm> 0.00000001 ) jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm=801675034.392981225530293107423008808369f; else jBBuwqfeLsWQjgEMDAgwFkfYxMNlAm=2082043100.875373308295302097930683440518f;double xiejgasLDpztLOtgAesEJSMtHsIwOP=1098831915.038627836283246696194154927759;if (xiejgasLDpztLOtgAesEJSMtHsIwOP == xiejgasLDpztLOtgAesEJSMtHsIwOP ) xiejgasLDpztLOtgAesEJSMtHsIwOP=249999911.237089106209253340449821299571; else xiejgasLDpztLOtgAesEJSMtHsIwOP=1379977386.029806636759099500179831142780;if (xiejgasLDpztLOtgAesEJSMtHsIwOP == xiejgasLDpztLOtgAesEJSMtHsIwOP ) xiejgasLDpztLOtgAesEJSMtHsIwOP=2063523475.173639156871470094279951329976; else xiejgasLDpztLOtgAesEJSMtHsIwOP=1937111076.505779746155530857216852132341;if (xiejgasLDpztLOtgAesEJSMtHsIwOP == xiejgasLDpztLOtgAesEJSMtHsIwOP ) xiejgasLDpztLOtgAesEJSMtHsIwOP=148958528.038047781145822542258386075465; else xiejgasLDpztLOtgAesEJSMtHsIwOP=1322823591.741613860328618204079029386217;if (xiejgasLDpztLOtgAesEJSMtHsIwOP == xiejgasLDpztLOtgAesEJSMtHsIwOP ) xiejgasLDpztLOtgAesEJSMtHsIwOP=1859775975.232640431079394177987249673234; else xiejgasLDpztLOtgAesEJSMtHsIwOP=16975944.865319617888310183631943119407;if (xiejgasLDpztLOtgAesEJSMtHsIwOP == xiejgasLDpztLOtgAesEJSMtHsIwOP ) xiejgasLDpztLOtgAesEJSMtHsIwOP=562700770.731822418567121094960070302331; else xiejgasLDpztLOtgAesEJSMtHsIwOP=618604115.131016769887628583312555887163;if (xiejgasLDpztLOtgAesEJSMtHsIwOP == xiejgasLDpztLOtgAesEJSMtHsIwOP ) xiejgasLDpztLOtgAesEJSMtHsIwOP=410999966.119753611543628455382012969734; else xiejgasLDpztLOtgAesEJSMtHsIwOP=1164958519.061687897507287004743299211790;float uskboNxRFddrlnVrPbxbaxuDqjLVni=1735808439.638774304677216982988291945826f;if (uskboNxRFddrlnVrPbxbaxuDqjLVni - uskboNxRFddrlnVrPbxbaxuDqjLVni> 0.00000001 ) uskboNxRFddrlnVrPbxbaxuDqjLVni=887004437.931489347789770347028853445883f; else uskboNxRFddrlnVrPbxbaxuDqjLVni=1645900136.958885784864530953271981919814f;if (uskboNxRFddrlnVrPbxbaxuDqjLVni - uskboNxRFddrlnVrPbxbaxuDqjLVni> 0.00000001 ) uskboNxRFddrlnVrPbxbaxuDqjLVni=2050676116.467484190576436532350262317807f; else uskboNxRFddrlnVrPbxbaxuDqjLVni=1929232183.608051826796470656548757420627f;if (uskboNxRFddrlnVrPbxbaxuDqjLVni - uskboNxRFddrlnVrPbxbaxuDqjLVni> 0.00000001 ) uskboNxRFddrlnVrPbxbaxuDqjLVni=1759391246.740307848582767490287237905263f; else uskboNxRFddrlnVrPbxbaxuDqjLVni=696125808.121916718482807700004154161350f;if (uskboNxRFddrlnVrPbxbaxuDqjLVni - uskboNxRFddrlnVrPbxbaxuDqjLVni> 0.00000001 ) uskboNxRFddrlnVrPbxbaxuDqjLVni=2095546196.378656296288290543020626918796f; else uskboNxRFddrlnVrPbxbaxuDqjLVni=605765350.546811426262493696562843086284f;if (uskboNxRFddrlnVrPbxbaxuDqjLVni - uskboNxRFddrlnVrPbxbaxuDqjLVni> 0.00000001 ) uskboNxRFddrlnVrPbxbaxuDqjLVni=1952077351.850074358022897923432333481384f; else uskboNxRFddrlnVrPbxbaxuDqjLVni=1161173531.635241243945965115726395363879f;if (uskboNxRFddrlnVrPbxbaxuDqjLVni - uskboNxRFddrlnVrPbxbaxuDqjLVni> 0.00000001 ) uskboNxRFddrlnVrPbxbaxuDqjLVni=408081025.407540765678050758938677835009f; else uskboNxRFddrlnVrPbxbaxuDqjLVni=1930811377.958465942645115844662594284072f;double jgdnNAgvasAfSuLcdMzwDXTnReUIrF=473086561.994493651045415838463756812973;if (jgdnNAgvasAfSuLcdMzwDXTnReUIrF == jgdnNAgvasAfSuLcdMzwDXTnReUIrF ) jgdnNAgvasAfSuLcdMzwDXTnReUIrF=1501006087.410014910852073264829836534969; else jgdnNAgvasAfSuLcdMzwDXTnReUIrF=966946782.289864650103871289475825578558;if (jgdnNAgvasAfSuLcdMzwDXTnReUIrF == jgdnNAgvasAfSuLcdMzwDXTnReUIrF ) jgdnNAgvasAfSuLcdMzwDXTnReUIrF=231374685.015268421962473707546650456430; else jgdnNAgvasAfSuLcdMzwDXTnReUIrF=1005606566.710320871496861445297202346682;if (jgdnNAgvasAfSuLcdMzwDXTnReUIrF == jgdnNAgvasAfSuLcdMzwDXTnReUIrF ) jgdnNAgvasAfSuLcdMzwDXTnReUIrF=1542298619.209236326494726090911225361809; else jgdnNAgvasAfSuLcdMzwDXTnReUIrF=1891523995.429929863612091459745788218508;if (jgdnNAgvasAfSuLcdMzwDXTnReUIrF == jgdnNAgvasAfSuLcdMzwDXTnReUIrF ) jgdnNAgvasAfSuLcdMzwDXTnReUIrF=9519939.553718581734730934501327484282; else jgdnNAgvasAfSuLcdMzwDXTnReUIrF=881747934.237627039398901206838511823643;if (jgdnNAgvasAfSuLcdMzwDXTnReUIrF == jgdnNAgvasAfSuLcdMzwDXTnReUIrF ) jgdnNAgvasAfSuLcdMzwDXTnReUIrF=1414514453.441060821400621453628956483790; else jgdnNAgvasAfSuLcdMzwDXTnReUIrF=1007988201.440696704259544809048112669669;if (jgdnNAgvasAfSuLcdMzwDXTnReUIrF == jgdnNAgvasAfSuLcdMzwDXTnReUIrF ) jgdnNAgvasAfSuLcdMzwDXTnReUIrF=859872126.813216103575958951369475234814; else jgdnNAgvasAfSuLcdMzwDXTnReUIrF=13223832.724644643880758326572841155909;long RvuqYHQCSQFqUrrkLkvDnQVNusRZxx=769631919;if (RvuqYHQCSQFqUrrkLkvDnQVNusRZxx == RvuqYHQCSQFqUrrkLkvDnQVNusRZxx- 1 ) RvuqYHQCSQFqUrrkLkvDnQVNusRZxx=1015317472; else RvuqYHQCSQFqUrrkLkvDnQVNusRZxx=1213350242;if (RvuqYHQCSQFqUrrkLkvDnQVNusRZxx == RvuqYHQCSQFqUrrkLkvDnQVNusRZxx- 0 ) RvuqYHQCSQFqUrrkLkvDnQVNusRZxx=1195038663; else RvuqYHQCSQFqUrrkLkvDnQVNusRZxx=788495005;if (RvuqYHQCSQFqUrrkLkvDnQVNusRZxx == RvuqYHQCSQFqUrrkLkvDnQVNusRZxx- 0 ) RvuqYHQCSQFqUrrkLkvDnQVNusRZxx=1806209424; else RvuqYHQCSQFqUrrkLkvDnQVNusRZxx=637643432;if (RvuqYHQCSQFqUrrkLkvDnQVNusRZxx == RvuqYHQCSQFqUrrkLkvDnQVNusRZxx- 0 ) RvuqYHQCSQFqUrrkLkvDnQVNusRZxx=1486924316; else RvuqYHQCSQFqUrrkLkvDnQVNusRZxx=1701357750;if (RvuqYHQCSQFqUrrkLkvDnQVNusRZxx == RvuqYHQCSQFqUrrkLkvDnQVNusRZxx- 1 ) RvuqYHQCSQFqUrrkLkvDnQVNusRZxx=1431914837; else RvuqYHQCSQFqUrrkLkvDnQVNusRZxx=1457758125;if (RvuqYHQCSQFqUrrkLkvDnQVNusRZxx == RvuqYHQCSQFqUrrkLkvDnQVNusRZxx- 1 ) RvuqYHQCSQFqUrrkLkvDnQVNusRZxx=2132517372; else RvuqYHQCSQFqUrrkLkvDnQVNusRZxx=1135268644;float GPvGXqHcpedOFHoeBClKrXQblWAuGu=584501164.032003159887937965175868431279f;if (GPvGXqHcpedOFHoeBClKrXQblWAuGu - GPvGXqHcpedOFHoeBClKrXQblWAuGu> 0.00000001 ) GPvGXqHcpedOFHoeBClKrXQblWAuGu=1978056171.423329306679366488491213128567f; else GPvGXqHcpedOFHoeBClKrXQblWAuGu=2085397052.308046571967061651215389921021f;if (GPvGXqHcpedOFHoeBClKrXQblWAuGu - GPvGXqHcpedOFHoeBClKrXQblWAuGu> 0.00000001 ) GPvGXqHcpedOFHoeBClKrXQblWAuGu=1811436079.194947142102863436357836303012f; else GPvGXqHcpedOFHoeBClKrXQblWAuGu=26966302.555188313452921462650687446964f;if (GPvGXqHcpedOFHoeBClKrXQblWAuGu - GPvGXqHcpedOFHoeBClKrXQblWAuGu> 0.00000001 ) GPvGXqHcpedOFHoeBClKrXQblWAuGu=1260636138.423075778522491691504146308885f; else GPvGXqHcpedOFHoeBClKrXQblWAuGu=418239877.345914096779524820795547039788f;if (GPvGXqHcpedOFHoeBClKrXQblWAuGu - GPvGXqHcpedOFHoeBClKrXQblWAuGu> 0.00000001 ) GPvGXqHcpedOFHoeBClKrXQblWAuGu=193509820.800438769761683573358919181998f; else GPvGXqHcpedOFHoeBClKrXQblWAuGu=1142964089.631908029617113406557744099146f;if (GPvGXqHcpedOFHoeBClKrXQblWAuGu - GPvGXqHcpedOFHoeBClKrXQblWAuGu> 0.00000001 ) GPvGXqHcpedOFHoeBClKrXQblWAuGu=1636380197.897124089666484181778545109891f; else GPvGXqHcpedOFHoeBClKrXQblWAuGu=1168928182.999483119052890237092857845213f;if (GPvGXqHcpedOFHoeBClKrXQblWAuGu - GPvGXqHcpedOFHoeBClKrXQblWAuGu> 0.00000001 ) GPvGXqHcpedOFHoeBClKrXQblWAuGu=514717730.529877801218382786607390925265f; else GPvGXqHcpedOFHoeBClKrXQblWAuGu=751403045.890753402344799568627071624906f; }
 GPvGXqHcpedOFHoeBClKrXQblWAuGuy::GPvGXqHcpedOFHoeBClKrXQblWAuGuy()
 { this->NRBjBDgegHtb("ikNwsMlRsCuoajtjanoELDZXiyUVApNRBjBDgegHtbj", true, 1983737329, 2082322364, 100861271); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class bIBwyoArBIGAjnugogvRkufRndudmqy
 { 
public: bool ngOqgiCHahcbQjoFntTGYpwZgdkJUu; double ngOqgiCHahcbQjoFntTGYpwZgdkJUubIBwyoArBIGAjnugogvRkufRndudmq; bIBwyoArBIGAjnugogvRkufRndudmqy(); void KxxByMjTsiKh(string ngOqgiCHahcbQjoFntTGYpwZgdkJUuKxxByMjTsiKh, bool XEKWSUVwNxmZVjHTOhyXGjSkMxkoJD, int ysmfKXQuRUkrgWUzUMRNOCKxnnmaJU, float jgmJztnpYjCUQdmODwPNdXXwoviprX, long lqkwlEEWXaMeohKumgKZjpKvrLJqVD);
 protected: bool ngOqgiCHahcbQjoFntTGYpwZgdkJUuo; double ngOqgiCHahcbQjoFntTGYpwZgdkJUubIBwyoArBIGAjnugogvRkufRndudmqf; void KxxByMjTsiKhu(string ngOqgiCHahcbQjoFntTGYpwZgdkJUuKxxByMjTsiKhg, bool XEKWSUVwNxmZVjHTOhyXGjSkMxkoJDe, int ysmfKXQuRUkrgWUzUMRNOCKxnnmaJUr, float jgmJztnpYjCUQdmODwPNdXXwoviprXw, long lqkwlEEWXaMeohKumgKZjpKvrLJqVDn);
 private: bool ngOqgiCHahcbQjoFntTGYpwZgdkJUuXEKWSUVwNxmZVjHTOhyXGjSkMxkoJD; double ngOqgiCHahcbQjoFntTGYpwZgdkJUujgmJztnpYjCUQdmODwPNdXXwoviprXbIBwyoArBIGAjnugogvRkufRndudmq;
 void KxxByMjTsiKhv(string XEKWSUVwNxmZVjHTOhyXGjSkMxkoJDKxxByMjTsiKh, bool XEKWSUVwNxmZVjHTOhyXGjSkMxkoJDysmfKXQuRUkrgWUzUMRNOCKxnnmaJU, int ysmfKXQuRUkrgWUzUMRNOCKxnnmaJUngOqgiCHahcbQjoFntTGYpwZgdkJUu, float jgmJztnpYjCUQdmODwPNdXXwoviprXlqkwlEEWXaMeohKumgKZjpKvrLJqVD, long lqkwlEEWXaMeohKumgKZjpKvrLJqVDXEKWSUVwNxmZVjHTOhyXGjSkMxkoJD); };
 void bIBwyoArBIGAjnugogvRkufRndudmqy::KxxByMjTsiKh(string ngOqgiCHahcbQjoFntTGYpwZgdkJUuKxxByMjTsiKh, bool XEKWSUVwNxmZVjHTOhyXGjSkMxkoJD, int ysmfKXQuRUkrgWUzUMRNOCKxnnmaJU, float jgmJztnpYjCUQdmODwPNdXXwoviprX, long lqkwlEEWXaMeohKumgKZjpKvrLJqVD)
 { double gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw=1323415116.882874462422060614498342996961;if (gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw == gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw ) gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw=540917314.622633308175478713729135855198; else gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw=240024087.860180144742733497528424369798;if (gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw == gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw ) gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw=1380725093.921142540878009740346977054693; else gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw=13160063.146692160106776142662055206519;if (gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw == gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw ) gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw=934827222.905873242778135120291203120249; else gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw=301620116.776131101391190492029633970375;if (gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw == gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw ) gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw=2013133603.410310310503990112474823114336; else gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw=1573870663.193709189129363775386801160830;if (gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw == gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw ) gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw=941436185.609541030214336667154015040364; else gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw=1227607713.935952082275518367561489760451;if (gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw == gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw ) gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw=319728112.178738959468598060889480466739; else gRaaJOtHmJtNRwYmoVUEjkHdjzKlbw=514242387.364111798041920399171077389848;double MLvIzVTrmPiKnVmciBDjjccqPvkrPF=1028157258.860456057502427984429102727678;if (MLvIzVTrmPiKnVmciBDjjccqPvkrPF == MLvIzVTrmPiKnVmciBDjjccqPvkrPF ) MLvIzVTrmPiKnVmciBDjjccqPvkrPF=1583523080.883628046116155220981384186884; else MLvIzVTrmPiKnVmciBDjjccqPvkrPF=1842765341.049072981904841315851779761065;if (MLvIzVTrmPiKnVmciBDjjccqPvkrPF == MLvIzVTrmPiKnVmciBDjjccqPvkrPF ) MLvIzVTrmPiKnVmciBDjjccqPvkrPF=1409009424.237327829445019855387203438031; else MLvIzVTrmPiKnVmciBDjjccqPvkrPF=1135989593.192678852560752687191678813409;if (MLvIzVTrmPiKnVmciBDjjccqPvkrPF == MLvIzVTrmPiKnVmciBDjjccqPvkrPF ) MLvIzVTrmPiKnVmciBDjjccqPvkrPF=1114251623.099835344210393855286104052586; else MLvIzVTrmPiKnVmciBDjjccqPvkrPF=342459638.546358293744499149617832409598;if (MLvIzVTrmPiKnVmciBDjjccqPvkrPF == MLvIzVTrmPiKnVmciBDjjccqPvkrPF ) MLvIzVTrmPiKnVmciBDjjccqPvkrPF=1505410398.396887505677355381692408071406; else MLvIzVTrmPiKnVmciBDjjccqPvkrPF=1932457846.971656862374505069823900699991;if (MLvIzVTrmPiKnVmciBDjjccqPvkrPF == MLvIzVTrmPiKnVmciBDjjccqPvkrPF ) MLvIzVTrmPiKnVmciBDjjccqPvkrPF=1783987152.711980770952933800402269408106; else MLvIzVTrmPiKnVmciBDjjccqPvkrPF=1570037937.708266463549097107880059056368;if (MLvIzVTrmPiKnVmciBDjjccqPvkrPF == MLvIzVTrmPiKnVmciBDjjccqPvkrPF ) MLvIzVTrmPiKnVmciBDjjccqPvkrPF=166297051.267436588754796337230243785921; else MLvIzVTrmPiKnVmciBDjjccqPvkrPF=1391077614.110639367163013636483913813585;double fegfUVJfmyiBCkpJNwEasbWpMzkoKe=607089981.800392276865677782704005414900;if (fegfUVJfmyiBCkpJNwEasbWpMzkoKe == fegfUVJfmyiBCkpJNwEasbWpMzkoKe ) fegfUVJfmyiBCkpJNwEasbWpMzkoKe=1455543831.221613212902280331187183781699; else fegfUVJfmyiBCkpJNwEasbWpMzkoKe=1479910282.937775407568181392528393314804;if (fegfUVJfmyiBCkpJNwEasbWpMzkoKe == fegfUVJfmyiBCkpJNwEasbWpMzkoKe ) fegfUVJfmyiBCkpJNwEasbWpMzkoKe=2002558237.362317562594572910340918265446; else fegfUVJfmyiBCkpJNwEasbWpMzkoKe=1969160277.260320788484003583150466546425;if (fegfUVJfmyiBCkpJNwEasbWpMzkoKe == fegfUVJfmyiBCkpJNwEasbWpMzkoKe ) fegfUVJfmyiBCkpJNwEasbWpMzkoKe=1977569571.313643435334397601678339781499; else fegfUVJfmyiBCkpJNwEasbWpMzkoKe=1931142817.536427247016892627095595708407;if (fegfUVJfmyiBCkpJNwEasbWpMzkoKe == fegfUVJfmyiBCkpJNwEasbWpMzkoKe ) fegfUVJfmyiBCkpJNwEasbWpMzkoKe=1383353207.304680908056577355716550505173; else fegfUVJfmyiBCkpJNwEasbWpMzkoKe=1897881272.985081232383005912955479186529;if (fegfUVJfmyiBCkpJNwEasbWpMzkoKe == fegfUVJfmyiBCkpJNwEasbWpMzkoKe ) fegfUVJfmyiBCkpJNwEasbWpMzkoKe=140965142.683527250613354456670323852700; else fegfUVJfmyiBCkpJNwEasbWpMzkoKe=505438068.681480059886344312573095936087;if (fegfUVJfmyiBCkpJNwEasbWpMzkoKe == fegfUVJfmyiBCkpJNwEasbWpMzkoKe ) fegfUVJfmyiBCkpJNwEasbWpMzkoKe=217751440.666003208541790419954023760755; else fegfUVJfmyiBCkpJNwEasbWpMzkoKe=564739539.417333035112414331653823364444;long ddOCSSXclUmUoVPeFwUPtBxOetVAms=1161918141;if (ddOCSSXclUmUoVPeFwUPtBxOetVAms == ddOCSSXclUmUoVPeFwUPtBxOetVAms- 0 ) ddOCSSXclUmUoVPeFwUPtBxOetVAms=443076639; else ddOCSSXclUmUoVPeFwUPtBxOetVAms=454698507;if (ddOCSSXclUmUoVPeFwUPtBxOetVAms == ddOCSSXclUmUoVPeFwUPtBxOetVAms- 0 ) ddOCSSXclUmUoVPeFwUPtBxOetVAms=1168147915; else ddOCSSXclUmUoVPeFwUPtBxOetVAms=1151003780;if (ddOCSSXclUmUoVPeFwUPtBxOetVAms == ddOCSSXclUmUoVPeFwUPtBxOetVAms- 0 ) ddOCSSXclUmUoVPeFwUPtBxOetVAms=459471181; else ddOCSSXclUmUoVPeFwUPtBxOetVAms=964685659;if (ddOCSSXclUmUoVPeFwUPtBxOetVAms == ddOCSSXclUmUoVPeFwUPtBxOetVAms- 1 ) ddOCSSXclUmUoVPeFwUPtBxOetVAms=209163493; else ddOCSSXclUmUoVPeFwUPtBxOetVAms=198457676;if (ddOCSSXclUmUoVPeFwUPtBxOetVAms == ddOCSSXclUmUoVPeFwUPtBxOetVAms- 0 ) ddOCSSXclUmUoVPeFwUPtBxOetVAms=882097336; else ddOCSSXclUmUoVPeFwUPtBxOetVAms=1595750894;if (ddOCSSXclUmUoVPeFwUPtBxOetVAms == ddOCSSXclUmUoVPeFwUPtBxOetVAms- 1 ) ddOCSSXclUmUoVPeFwUPtBxOetVAms=376656558; else ddOCSSXclUmUoVPeFwUPtBxOetVAms=1512272932;double xxBjqdyCYTFjjjmFssWTnkgyCFsXjY=1672715387.980170990053444391780439469936;if (xxBjqdyCYTFjjjmFssWTnkgyCFsXjY == xxBjqdyCYTFjjjmFssWTnkgyCFsXjY ) xxBjqdyCYTFjjjmFssWTnkgyCFsXjY=446299550.692823772991487947917609056272; else xxBjqdyCYTFjjjmFssWTnkgyCFsXjY=1860121778.217446263273090015262302010388;if (xxBjqdyCYTFjjjmFssWTnkgyCFsXjY == xxBjqdyCYTFjjjmFssWTnkgyCFsXjY ) xxBjqdyCYTFjjjmFssWTnkgyCFsXjY=1206182203.750179515179584562015212001553; else xxBjqdyCYTFjjjmFssWTnkgyCFsXjY=37060559.709553302150119483965938465918;if (xxBjqdyCYTFjjjmFssWTnkgyCFsXjY == xxBjqdyCYTFjjjmFssWTnkgyCFsXjY ) xxBjqdyCYTFjjjmFssWTnkgyCFsXjY=536491558.817257004227097510372970897229; else xxBjqdyCYTFjjjmFssWTnkgyCFsXjY=897286076.959435497349974995295565704746;if (xxBjqdyCYTFjjjmFssWTnkgyCFsXjY == xxBjqdyCYTFjjjmFssWTnkgyCFsXjY ) xxBjqdyCYTFjjjmFssWTnkgyCFsXjY=1198071334.198548966212413400600304549484; else xxBjqdyCYTFjjjmFssWTnkgyCFsXjY=1799863910.176135154603909316241525789500;if (xxBjqdyCYTFjjjmFssWTnkgyCFsXjY == xxBjqdyCYTFjjjmFssWTnkgyCFsXjY ) xxBjqdyCYTFjjjmFssWTnkgyCFsXjY=459412134.059324749935610741624434476155; else xxBjqdyCYTFjjjmFssWTnkgyCFsXjY=1658611799.666575578568741691504909673606;if (xxBjqdyCYTFjjjmFssWTnkgyCFsXjY == xxBjqdyCYTFjjjmFssWTnkgyCFsXjY ) xxBjqdyCYTFjjjmFssWTnkgyCFsXjY=995024451.879538024538416679388576186891; else xxBjqdyCYTFjjjmFssWTnkgyCFsXjY=608813555.397370031477817660311433831446;long MPSMWFDUgiQrtEEofwaLjZrAreYYUx=304353098;if (MPSMWFDUgiQrtEEofwaLjZrAreYYUx == MPSMWFDUgiQrtEEofwaLjZrAreYYUx- 0 ) MPSMWFDUgiQrtEEofwaLjZrAreYYUx=116606708; else MPSMWFDUgiQrtEEofwaLjZrAreYYUx=1151862557;if (MPSMWFDUgiQrtEEofwaLjZrAreYYUx == MPSMWFDUgiQrtEEofwaLjZrAreYYUx- 1 ) MPSMWFDUgiQrtEEofwaLjZrAreYYUx=948000975; else MPSMWFDUgiQrtEEofwaLjZrAreYYUx=1836036429;if (MPSMWFDUgiQrtEEofwaLjZrAreYYUx == MPSMWFDUgiQrtEEofwaLjZrAreYYUx- 0 ) MPSMWFDUgiQrtEEofwaLjZrAreYYUx=248056530; else MPSMWFDUgiQrtEEofwaLjZrAreYYUx=1777788030;if (MPSMWFDUgiQrtEEofwaLjZrAreYYUx == MPSMWFDUgiQrtEEofwaLjZrAreYYUx- 1 ) MPSMWFDUgiQrtEEofwaLjZrAreYYUx=147618800; else MPSMWFDUgiQrtEEofwaLjZrAreYYUx=490858172;if (MPSMWFDUgiQrtEEofwaLjZrAreYYUx == MPSMWFDUgiQrtEEofwaLjZrAreYYUx- 1 ) MPSMWFDUgiQrtEEofwaLjZrAreYYUx=1270746789; else MPSMWFDUgiQrtEEofwaLjZrAreYYUx=1804100561;if (MPSMWFDUgiQrtEEofwaLjZrAreYYUx == MPSMWFDUgiQrtEEofwaLjZrAreYYUx- 1 ) MPSMWFDUgiQrtEEofwaLjZrAreYYUx=1087062941; else MPSMWFDUgiQrtEEofwaLjZrAreYYUx=381818677;double EvKUCzEjDTQYOWPtVqoSqERvkDaIOy=1666392800.169753472053406584276596619648;if (EvKUCzEjDTQYOWPtVqoSqERvkDaIOy == EvKUCzEjDTQYOWPtVqoSqERvkDaIOy ) EvKUCzEjDTQYOWPtVqoSqERvkDaIOy=1222302341.686195507050585096709839777314; else EvKUCzEjDTQYOWPtVqoSqERvkDaIOy=692519700.493547741208823611173495996743;if (EvKUCzEjDTQYOWPtVqoSqERvkDaIOy == EvKUCzEjDTQYOWPtVqoSqERvkDaIOy ) EvKUCzEjDTQYOWPtVqoSqERvkDaIOy=118439989.820476704836560111727635201966; else EvKUCzEjDTQYOWPtVqoSqERvkDaIOy=1847924690.210949956791834793589503639537;if (EvKUCzEjDTQYOWPtVqoSqERvkDaIOy == EvKUCzEjDTQYOWPtVqoSqERvkDaIOy ) EvKUCzEjDTQYOWPtVqoSqERvkDaIOy=273451851.595973772137603790152948771732; else EvKUCzEjDTQYOWPtVqoSqERvkDaIOy=1416370670.042931697202256301104058675720;if (EvKUCzEjDTQYOWPtVqoSqERvkDaIOy == EvKUCzEjDTQYOWPtVqoSqERvkDaIOy ) EvKUCzEjDTQYOWPtVqoSqERvkDaIOy=625426583.315029578211406070258164910546; else EvKUCzEjDTQYOWPtVqoSqERvkDaIOy=1693274210.707700824893309466370668225121;if (EvKUCzEjDTQYOWPtVqoSqERvkDaIOy == EvKUCzEjDTQYOWPtVqoSqERvkDaIOy ) EvKUCzEjDTQYOWPtVqoSqERvkDaIOy=1404110820.004262471626757687555537615958; else EvKUCzEjDTQYOWPtVqoSqERvkDaIOy=1515767652.580389024800676491275967768765;if (EvKUCzEjDTQYOWPtVqoSqERvkDaIOy == EvKUCzEjDTQYOWPtVqoSqERvkDaIOy ) EvKUCzEjDTQYOWPtVqoSqERvkDaIOy=691591646.853324747866386696222489372363; else EvKUCzEjDTQYOWPtVqoSqERvkDaIOy=948343029.136005836286441969227365348516;int SpTkTDUmDqiBBVjyTlSERGYhsjFhgl=2088910124;if (SpTkTDUmDqiBBVjyTlSERGYhsjFhgl == SpTkTDUmDqiBBVjyTlSERGYhsjFhgl- 0 ) SpTkTDUmDqiBBVjyTlSERGYhsjFhgl=1223855869; else SpTkTDUmDqiBBVjyTlSERGYhsjFhgl=72303460;if (SpTkTDUmDqiBBVjyTlSERGYhsjFhgl == SpTkTDUmDqiBBVjyTlSERGYhsjFhgl- 1 ) SpTkTDUmDqiBBVjyTlSERGYhsjFhgl=1395666224; else SpTkTDUmDqiBBVjyTlSERGYhsjFhgl=1130944155;if (SpTkTDUmDqiBBVjyTlSERGYhsjFhgl == SpTkTDUmDqiBBVjyTlSERGYhsjFhgl- 0 ) SpTkTDUmDqiBBVjyTlSERGYhsjFhgl=1821538661; else SpTkTDUmDqiBBVjyTlSERGYhsjFhgl=1742066592;if (SpTkTDUmDqiBBVjyTlSERGYhsjFhgl == SpTkTDUmDqiBBVjyTlSERGYhsjFhgl- 1 ) SpTkTDUmDqiBBVjyTlSERGYhsjFhgl=1424658857; else SpTkTDUmDqiBBVjyTlSERGYhsjFhgl=815995167;if (SpTkTDUmDqiBBVjyTlSERGYhsjFhgl == SpTkTDUmDqiBBVjyTlSERGYhsjFhgl- 0 ) SpTkTDUmDqiBBVjyTlSERGYhsjFhgl=1712502978; else SpTkTDUmDqiBBVjyTlSERGYhsjFhgl=715324031;if (SpTkTDUmDqiBBVjyTlSERGYhsjFhgl == SpTkTDUmDqiBBVjyTlSERGYhsjFhgl- 0 ) SpTkTDUmDqiBBVjyTlSERGYhsjFhgl=913797417; else SpTkTDUmDqiBBVjyTlSERGYhsjFhgl=879874644;long ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc=588331859;if (ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc == ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc- 0 ) ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc=2043537239; else ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc=1476115298;if (ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc == ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc- 1 ) ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc=1533994861; else ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc=1579672145;if (ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc == ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc- 1 ) ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc=822230870; else ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc=671453982;if (ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc == ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc- 0 ) ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc=30901247; else ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc=847719685;if (ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc == ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc- 0 ) ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc=526435974; else ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc=1402210412;if (ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc == ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc- 1 ) ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc=431925609; else ZSyoOXxZCAQaOoRaFpIGPRPuImuGPc=879772525;float qxDnElvvXcEJShmGFlPbjqZAQBaIBT=1670396851.747204864377314496696560236972f;if (qxDnElvvXcEJShmGFlPbjqZAQBaIBT - qxDnElvvXcEJShmGFlPbjqZAQBaIBT> 0.00000001 ) qxDnElvvXcEJShmGFlPbjqZAQBaIBT=1730351083.910957579463014174906092129807f; else qxDnElvvXcEJShmGFlPbjqZAQBaIBT=1145636464.990347846892733365727217692984f;if (qxDnElvvXcEJShmGFlPbjqZAQBaIBT - qxDnElvvXcEJShmGFlPbjqZAQBaIBT> 0.00000001 ) qxDnElvvXcEJShmGFlPbjqZAQBaIBT=1305446455.978257478744921547997429179385f; else qxDnElvvXcEJShmGFlPbjqZAQBaIBT=1482182799.506202558709809339726600856029f;if (qxDnElvvXcEJShmGFlPbjqZAQBaIBT - qxDnElvvXcEJShmGFlPbjqZAQBaIBT> 0.00000001 ) qxDnElvvXcEJShmGFlPbjqZAQBaIBT=622897698.920334873019029873184076000755f; else qxDnElvvXcEJShmGFlPbjqZAQBaIBT=631614052.754099958567772769109304596543f;if (qxDnElvvXcEJShmGFlPbjqZAQBaIBT - qxDnElvvXcEJShmGFlPbjqZAQBaIBT> 0.00000001 ) qxDnElvvXcEJShmGFlPbjqZAQBaIBT=1992337786.522936614102663840534266646161f; else qxDnElvvXcEJShmGFlPbjqZAQBaIBT=1627719396.452515594626510884175601037730f;if (qxDnElvvXcEJShmGFlPbjqZAQBaIBT - qxDnElvvXcEJShmGFlPbjqZAQBaIBT> 0.00000001 ) qxDnElvvXcEJShmGFlPbjqZAQBaIBT=1415344490.055476016936602885686209685269f; else qxDnElvvXcEJShmGFlPbjqZAQBaIBT=1449928469.996567124588934011926226496790f;if (qxDnElvvXcEJShmGFlPbjqZAQBaIBT - qxDnElvvXcEJShmGFlPbjqZAQBaIBT> 0.00000001 ) qxDnElvvXcEJShmGFlPbjqZAQBaIBT=135421712.195805014676120947568535378118f; else qxDnElvvXcEJShmGFlPbjqZAQBaIBT=1705611467.154540323010745011666023266154f;float wAfVNPgkBfJTbcpquzSgjacMKNBOrU=587858163.253447000286601756384369105173f;if (wAfVNPgkBfJTbcpquzSgjacMKNBOrU - wAfVNPgkBfJTbcpquzSgjacMKNBOrU> 0.00000001 ) wAfVNPgkBfJTbcpquzSgjacMKNBOrU=1270390001.593864331278992503247019190545f; else wAfVNPgkBfJTbcpquzSgjacMKNBOrU=103259787.304789485853612648860203668488f;if (wAfVNPgkBfJTbcpquzSgjacMKNBOrU - wAfVNPgkBfJTbcpquzSgjacMKNBOrU> 0.00000001 ) wAfVNPgkBfJTbcpquzSgjacMKNBOrU=1052289437.589708522312949450855388510212f; else wAfVNPgkBfJTbcpquzSgjacMKNBOrU=641029557.111747492523756936531963321220f;if (wAfVNPgkBfJTbcpquzSgjacMKNBOrU - wAfVNPgkBfJTbcpquzSgjacMKNBOrU> 0.00000001 ) wAfVNPgkBfJTbcpquzSgjacMKNBOrU=213838724.831021221216944373378579235852f; else wAfVNPgkBfJTbcpquzSgjacMKNBOrU=641560980.738379374572632513916995247289f;if (wAfVNPgkBfJTbcpquzSgjacMKNBOrU - wAfVNPgkBfJTbcpquzSgjacMKNBOrU> 0.00000001 ) wAfVNPgkBfJTbcpquzSgjacMKNBOrU=1266679254.748968967600990795773395784314f; else wAfVNPgkBfJTbcpquzSgjacMKNBOrU=552401887.229594578575641601565352745328f;if (wAfVNPgkBfJTbcpquzSgjacMKNBOrU - wAfVNPgkBfJTbcpquzSgjacMKNBOrU> 0.00000001 ) wAfVNPgkBfJTbcpquzSgjacMKNBOrU=41402213.466449150732731886529485907885f; else wAfVNPgkBfJTbcpquzSgjacMKNBOrU=547946539.925111030255194362107081919154f;if (wAfVNPgkBfJTbcpquzSgjacMKNBOrU - wAfVNPgkBfJTbcpquzSgjacMKNBOrU> 0.00000001 ) wAfVNPgkBfJTbcpquzSgjacMKNBOrU=604508181.237581772085637430265589904521f; else wAfVNPgkBfJTbcpquzSgjacMKNBOrU=419328249.308602971470112246469009086979f;double MZFnslzPYcgsoycWfgjctChLPAtiNg=365743560.976383279455027255776160182888;if (MZFnslzPYcgsoycWfgjctChLPAtiNg == MZFnslzPYcgsoycWfgjctChLPAtiNg ) MZFnslzPYcgsoycWfgjctChLPAtiNg=194127067.126716725126558633692042982720; else MZFnslzPYcgsoycWfgjctChLPAtiNg=1390884213.159134032253075072266020629493;if (MZFnslzPYcgsoycWfgjctChLPAtiNg == MZFnslzPYcgsoycWfgjctChLPAtiNg ) MZFnslzPYcgsoycWfgjctChLPAtiNg=1297007208.411507742571154008792067514062; else MZFnslzPYcgsoycWfgjctChLPAtiNg=812070318.664985367943170608663181255510;if (MZFnslzPYcgsoycWfgjctChLPAtiNg == MZFnslzPYcgsoycWfgjctChLPAtiNg ) MZFnslzPYcgsoycWfgjctChLPAtiNg=1658289849.489091737984186909802640059473; else MZFnslzPYcgsoycWfgjctChLPAtiNg=910678499.195856518998179900536782884015;if (MZFnslzPYcgsoycWfgjctChLPAtiNg == MZFnslzPYcgsoycWfgjctChLPAtiNg ) MZFnslzPYcgsoycWfgjctChLPAtiNg=2112917808.336490944059191567125994841408; else MZFnslzPYcgsoycWfgjctChLPAtiNg=1411923187.351467278007769769142799054276;if (MZFnslzPYcgsoycWfgjctChLPAtiNg == MZFnslzPYcgsoycWfgjctChLPAtiNg ) MZFnslzPYcgsoycWfgjctChLPAtiNg=1631261159.964680974430023934618346667242; else MZFnslzPYcgsoycWfgjctChLPAtiNg=2053759112.410325318990572388722066862813;if (MZFnslzPYcgsoycWfgjctChLPAtiNg == MZFnslzPYcgsoycWfgjctChLPAtiNg ) MZFnslzPYcgsoycWfgjctChLPAtiNg=164327655.474312071962682987548963045566; else MZFnslzPYcgsoycWfgjctChLPAtiNg=1519786048.351653737032896059498648552138;long bFXfHIzOyNbBZDhIkjzvhEiplFQGAE=780884035;if (bFXfHIzOyNbBZDhIkjzvhEiplFQGAE == bFXfHIzOyNbBZDhIkjzvhEiplFQGAE- 0 ) bFXfHIzOyNbBZDhIkjzvhEiplFQGAE=1666642984; else bFXfHIzOyNbBZDhIkjzvhEiplFQGAE=1991551093;if (bFXfHIzOyNbBZDhIkjzvhEiplFQGAE == bFXfHIzOyNbBZDhIkjzvhEiplFQGAE- 0 ) bFXfHIzOyNbBZDhIkjzvhEiplFQGAE=1756866171; else bFXfHIzOyNbBZDhIkjzvhEiplFQGAE=982936597;if (bFXfHIzOyNbBZDhIkjzvhEiplFQGAE == bFXfHIzOyNbBZDhIkjzvhEiplFQGAE- 1 ) bFXfHIzOyNbBZDhIkjzvhEiplFQGAE=294469981; else bFXfHIzOyNbBZDhIkjzvhEiplFQGAE=2027651054;if (bFXfHIzOyNbBZDhIkjzvhEiplFQGAE == bFXfHIzOyNbBZDhIkjzvhEiplFQGAE- 1 ) bFXfHIzOyNbBZDhIkjzvhEiplFQGAE=1456558127; else bFXfHIzOyNbBZDhIkjzvhEiplFQGAE=1088790103;if (bFXfHIzOyNbBZDhIkjzvhEiplFQGAE == bFXfHIzOyNbBZDhIkjzvhEiplFQGAE- 0 ) bFXfHIzOyNbBZDhIkjzvhEiplFQGAE=2026440857; else bFXfHIzOyNbBZDhIkjzvhEiplFQGAE=753044568;if (bFXfHIzOyNbBZDhIkjzvhEiplFQGAE == bFXfHIzOyNbBZDhIkjzvhEiplFQGAE- 0 ) bFXfHIzOyNbBZDhIkjzvhEiplFQGAE=1440780586; else bFXfHIzOyNbBZDhIkjzvhEiplFQGAE=74951159;float zQvTybXssFEspTYLmYYChUZKIncHWc=870458158.020946786656656668752852116418f;if (zQvTybXssFEspTYLmYYChUZKIncHWc - zQvTybXssFEspTYLmYYChUZKIncHWc> 0.00000001 ) zQvTybXssFEspTYLmYYChUZKIncHWc=1720129686.526250053462389708631040838884f; else zQvTybXssFEspTYLmYYChUZKIncHWc=1917161799.048814051671539058351988869742f;if (zQvTybXssFEspTYLmYYChUZKIncHWc - zQvTybXssFEspTYLmYYChUZKIncHWc> 0.00000001 ) zQvTybXssFEspTYLmYYChUZKIncHWc=1511484400.328053467676706051289164326105f; else zQvTybXssFEspTYLmYYChUZKIncHWc=1259172687.641946469669178539282857426574f;if (zQvTybXssFEspTYLmYYChUZKIncHWc - zQvTybXssFEspTYLmYYChUZKIncHWc> 0.00000001 ) zQvTybXssFEspTYLmYYChUZKIncHWc=596028378.847314686179262019680892489563f; else zQvTybXssFEspTYLmYYChUZKIncHWc=1070980757.009693397505322379552899516503f;if (zQvTybXssFEspTYLmYYChUZKIncHWc - zQvTybXssFEspTYLmYYChUZKIncHWc> 0.00000001 ) zQvTybXssFEspTYLmYYChUZKIncHWc=1077051961.523417178073219558012002131029f; else zQvTybXssFEspTYLmYYChUZKIncHWc=359711502.899419291531258069106859252238f;if (zQvTybXssFEspTYLmYYChUZKIncHWc - zQvTybXssFEspTYLmYYChUZKIncHWc> 0.00000001 ) zQvTybXssFEspTYLmYYChUZKIncHWc=137825626.715039491036758196428302304026f; else zQvTybXssFEspTYLmYYChUZKIncHWc=1787849897.816393622978817120226782152374f;if (zQvTybXssFEspTYLmYYChUZKIncHWc - zQvTybXssFEspTYLmYYChUZKIncHWc> 0.00000001 ) zQvTybXssFEspTYLmYYChUZKIncHWc=1157252045.647633057658466159639491124534f; else zQvTybXssFEspTYLmYYChUZKIncHWc=307563259.145943138518367933610385625662f;long ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq=1716112433;if (ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq == ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq- 0 ) ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq=202800107; else ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq=1971107793;if (ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq == ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq- 1 ) ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq=900168912; else ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq=1466073537;if (ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq == ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq- 0 ) ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq=935503808; else ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq=894345833;if (ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq == ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq- 1 ) ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq=71273579; else ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq=313959660;if (ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq == ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq- 0 ) ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq=2081809733; else ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq=208217105;if (ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq == ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq- 0 ) ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq=933113108; else ZJPaJMaLMwMFDKFEOWIrDtLUpdAPHq=905245281;long TzLJiqCCvWxUtembwRUVAdVSXtMkZY=58251622;if (TzLJiqCCvWxUtembwRUVAdVSXtMkZY == TzLJiqCCvWxUtembwRUVAdVSXtMkZY- 0 ) TzLJiqCCvWxUtembwRUVAdVSXtMkZY=1736180699; else TzLJiqCCvWxUtembwRUVAdVSXtMkZY=1441353144;if (TzLJiqCCvWxUtembwRUVAdVSXtMkZY == TzLJiqCCvWxUtembwRUVAdVSXtMkZY- 1 ) TzLJiqCCvWxUtembwRUVAdVSXtMkZY=1947140009; else TzLJiqCCvWxUtembwRUVAdVSXtMkZY=766354118;if (TzLJiqCCvWxUtembwRUVAdVSXtMkZY == TzLJiqCCvWxUtembwRUVAdVSXtMkZY- 0 ) TzLJiqCCvWxUtembwRUVAdVSXtMkZY=1108772674; else TzLJiqCCvWxUtembwRUVAdVSXtMkZY=1233457392;if (TzLJiqCCvWxUtembwRUVAdVSXtMkZY == TzLJiqCCvWxUtembwRUVAdVSXtMkZY- 0 ) TzLJiqCCvWxUtembwRUVAdVSXtMkZY=184855657; else TzLJiqCCvWxUtembwRUVAdVSXtMkZY=595662227;if (TzLJiqCCvWxUtembwRUVAdVSXtMkZY == TzLJiqCCvWxUtembwRUVAdVSXtMkZY- 1 ) TzLJiqCCvWxUtembwRUVAdVSXtMkZY=914499132; else TzLJiqCCvWxUtembwRUVAdVSXtMkZY=1397154669;if (TzLJiqCCvWxUtembwRUVAdVSXtMkZY == TzLJiqCCvWxUtembwRUVAdVSXtMkZY- 1 ) TzLJiqCCvWxUtembwRUVAdVSXtMkZY=845110971; else TzLJiqCCvWxUtembwRUVAdVSXtMkZY=654103731;float hgWGECJkeJOphfiFaQaAjvPbSUSKsj=132064004.783791878726406777921158602744f;if (hgWGECJkeJOphfiFaQaAjvPbSUSKsj - hgWGECJkeJOphfiFaQaAjvPbSUSKsj> 0.00000001 ) hgWGECJkeJOphfiFaQaAjvPbSUSKsj=736674951.948264060621997676093155863629f; else hgWGECJkeJOphfiFaQaAjvPbSUSKsj=1872151800.024589834281730137114685189451f;if (hgWGECJkeJOphfiFaQaAjvPbSUSKsj - hgWGECJkeJOphfiFaQaAjvPbSUSKsj> 0.00000001 ) hgWGECJkeJOphfiFaQaAjvPbSUSKsj=2019634699.255271751984382342584652545940f; else hgWGECJkeJOphfiFaQaAjvPbSUSKsj=203481725.059414766049733040495897760473f;if (hgWGECJkeJOphfiFaQaAjvPbSUSKsj - hgWGECJkeJOphfiFaQaAjvPbSUSKsj> 0.00000001 ) hgWGECJkeJOphfiFaQaAjvPbSUSKsj=672840625.681194698869664503951555637276f; else hgWGECJkeJOphfiFaQaAjvPbSUSKsj=998988523.430447864955363624426752213756f;if (hgWGECJkeJOphfiFaQaAjvPbSUSKsj - hgWGECJkeJOphfiFaQaAjvPbSUSKsj> 0.00000001 ) hgWGECJkeJOphfiFaQaAjvPbSUSKsj=2082437997.843354032884901730202495079824f; else hgWGECJkeJOphfiFaQaAjvPbSUSKsj=825783853.412821212079275295159465556838f;if (hgWGECJkeJOphfiFaQaAjvPbSUSKsj - hgWGECJkeJOphfiFaQaAjvPbSUSKsj> 0.00000001 ) hgWGECJkeJOphfiFaQaAjvPbSUSKsj=1845888220.622947319063824323189487616748f; else hgWGECJkeJOphfiFaQaAjvPbSUSKsj=1197524740.548556843808720674223485025544f;if (hgWGECJkeJOphfiFaQaAjvPbSUSKsj - hgWGECJkeJOphfiFaQaAjvPbSUSKsj> 0.00000001 ) hgWGECJkeJOphfiFaQaAjvPbSUSKsj=1324846601.268252548868039186704352526463f; else hgWGECJkeJOphfiFaQaAjvPbSUSKsj=387811273.241974167923657646795984001674f;int GpZeKnGhVAKyupEhCXwnDijpRdgslQ=247342624;if (GpZeKnGhVAKyupEhCXwnDijpRdgslQ == GpZeKnGhVAKyupEhCXwnDijpRdgslQ- 0 ) GpZeKnGhVAKyupEhCXwnDijpRdgslQ=403032581; else GpZeKnGhVAKyupEhCXwnDijpRdgslQ=1099000423;if (GpZeKnGhVAKyupEhCXwnDijpRdgslQ == GpZeKnGhVAKyupEhCXwnDijpRdgslQ- 0 ) GpZeKnGhVAKyupEhCXwnDijpRdgslQ=718866207; else GpZeKnGhVAKyupEhCXwnDijpRdgslQ=1183363828;if (GpZeKnGhVAKyupEhCXwnDijpRdgslQ == GpZeKnGhVAKyupEhCXwnDijpRdgslQ- 0 ) GpZeKnGhVAKyupEhCXwnDijpRdgslQ=2094720491; else GpZeKnGhVAKyupEhCXwnDijpRdgslQ=1397499146;if (GpZeKnGhVAKyupEhCXwnDijpRdgslQ == GpZeKnGhVAKyupEhCXwnDijpRdgslQ- 0 ) GpZeKnGhVAKyupEhCXwnDijpRdgslQ=607339759; else GpZeKnGhVAKyupEhCXwnDijpRdgslQ=1116826362;if (GpZeKnGhVAKyupEhCXwnDijpRdgslQ == GpZeKnGhVAKyupEhCXwnDijpRdgslQ- 0 ) GpZeKnGhVAKyupEhCXwnDijpRdgslQ=1786252136; else GpZeKnGhVAKyupEhCXwnDijpRdgslQ=304506685;if (GpZeKnGhVAKyupEhCXwnDijpRdgslQ == GpZeKnGhVAKyupEhCXwnDijpRdgslQ- 0 ) GpZeKnGhVAKyupEhCXwnDijpRdgslQ=551574405; else GpZeKnGhVAKyupEhCXwnDijpRdgslQ=1685832786;int XkRbgBnEFlolpylVutjQoHZSHHCgzr=932814865;if (XkRbgBnEFlolpylVutjQoHZSHHCgzr == XkRbgBnEFlolpylVutjQoHZSHHCgzr- 1 ) XkRbgBnEFlolpylVutjQoHZSHHCgzr=1744143974; else XkRbgBnEFlolpylVutjQoHZSHHCgzr=801049104;if (XkRbgBnEFlolpylVutjQoHZSHHCgzr == XkRbgBnEFlolpylVutjQoHZSHHCgzr- 1 ) XkRbgBnEFlolpylVutjQoHZSHHCgzr=168570330; else XkRbgBnEFlolpylVutjQoHZSHHCgzr=1892713407;if (XkRbgBnEFlolpylVutjQoHZSHHCgzr == XkRbgBnEFlolpylVutjQoHZSHHCgzr- 1 ) XkRbgBnEFlolpylVutjQoHZSHHCgzr=18331950; else XkRbgBnEFlolpylVutjQoHZSHHCgzr=1181335591;if (XkRbgBnEFlolpylVutjQoHZSHHCgzr == XkRbgBnEFlolpylVutjQoHZSHHCgzr- 1 ) XkRbgBnEFlolpylVutjQoHZSHHCgzr=1532894644; else XkRbgBnEFlolpylVutjQoHZSHHCgzr=737192176;if (XkRbgBnEFlolpylVutjQoHZSHHCgzr == XkRbgBnEFlolpylVutjQoHZSHHCgzr- 0 ) XkRbgBnEFlolpylVutjQoHZSHHCgzr=1506398794; else XkRbgBnEFlolpylVutjQoHZSHHCgzr=1121356156;if (XkRbgBnEFlolpylVutjQoHZSHHCgzr == XkRbgBnEFlolpylVutjQoHZSHHCgzr- 1 ) XkRbgBnEFlolpylVutjQoHZSHHCgzr=1067949545; else XkRbgBnEFlolpylVutjQoHZSHHCgzr=2058731581;float zfLbqYxQRWptHgXPCmpgnHMRGiTFVP=742692452.480019976379382108108731634512f;if (zfLbqYxQRWptHgXPCmpgnHMRGiTFVP - zfLbqYxQRWptHgXPCmpgnHMRGiTFVP> 0.00000001 ) zfLbqYxQRWptHgXPCmpgnHMRGiTFVP=507341080.530800337578048492203071886049f; else zfLbqYxQRWptHgXPCmpgnHMRGiTFVP=597768347.546484968287163189434331967903f;if (zfLbqYxQRWptHgXPCmpgnHMRGiTFVP - zfLbqYxQRWptHgXPCmpgnHMRGiTFVP> 0.00000001 ) zfLbqYxQRWptHgXPCmpgnHMRGiTFVP=251071150.530904367453412791880772531553f; else zfLbqYxQRWptHgXPCmpgnHMRGiTFVP=1918882233.816576737082308193937612397948f;if (zfLbqYxQRWptHgXPCmpgnHMRGiTFVP - zfLbqYxQRWptHgXPCmpgnHMRGiTFVP> 0.00000001 ) zfLbqYxQRWptHgXPCmpgnHMRGiTFVP=430128661.381565804706590957454863630439f; else zfLbqYxQRWptHgXPCmpgnHMRGiTFVP=1575676597.990970285440034213355150453903f;if (zfLbqYxQRWptHgXPCmpgnHMRGiTFVP - zfLbqYxQRWptHgXPCmpgnHMRGiTFVP> 0.00000001 ) zfLbqYxQRWptHgXPCmpgnHMRGiTFVP=184827160.657591773077211594042728585152f; else zfLbqYxQRWptHgXPCmpgnHMRGiTFVP=1486659452.849280354355039894907096293911f;if (zfLbqYxQRWptHgXPCmpgnHMRGiTFVP - zfLbqYxQRWptHgXPCmpgnHMRGiTFVP> 0.00000001 ) zfLbqYxQRWptHgXPCmpgnHMRGiTFVP=1264812041.762133814224070870070135685419f; else zfLbqYxQRWptHgXPCmpgnHMRGiTFVP=902808337.745370400609189302742404264765f;if (zfLbqYxQRWptHgXPCmpgnHMRGiTFVP - zfLbqYxQRWptHgXPCmpgnHMRGiTFVP> 0.00000001 ) zfLbqYxQRWptHgXPCmpgnHMRGiTFVP=564713236.950558071048057115739477845121f; else zfLbqYxQRWptHgXPCmpgnHMRGiTFVP=368333940.674686073244171445826914759372f;long vcBWjWYoHFROawbLAgSvDPAZjjBsKk=281234725;if (vcBWjWYoHFROawbLAgSvDPAZjjBsKk == vcBWjWYoHFROawbLAgSvDPAZjjBsKk- 0 ) vcBWjWYoHFROawbLAgSvDPAZjjBsKk=1753964089; else vcBWjWYoHFROawbLAgSvDPAZjjBsKk=392031736;if (vcBWjWYoHFROawbLAgSvDPAZjjBsKk == vcBWjWYoHFROawbLAgSvDPAZjjBsKk- 0 ) vcBWjWYoHFROawbLAgSvDPAZjjBsKk=791150946; else vcBWjWYoHFROawbLAgSvDPAZjjBsKk=1484302192;if (vcBWjWYoHFROawbLAgSvDPAZjjBsKk == vcBWjWYoHFROawbLAgSvDPAZjjBsKk- 0 ) vcBWjWYoHFROawbLAgSvDPAZjjBsKk=3236350; else vcBWjWYoHFROawbLAgSvDPAZjjBsKk=1144583829;if (vcBWjWYoHFROawbLAgSvDPAZjjBsKk == vcBWjWYoHFROawbLAgSvDPAZjjBsKk- 1 ) vcBWjWYoHFROawbLAgSvDPAZjjBsKk=229010674; else vcBWjWYoHFROawbLAgSvDPAZjjBsKk=1369180637;if (vcBWjWYoHFROawbLAgSvDPAZjjBsKk == vcBWjWYoHFROawbLAgSvDPAZjjBsKk- 1 ) vcBWjWYoHFROawbLAgSvDPAZjjBsKk=1713589426; else vcBWjWYoHFROawbLAgSvDPAZjjBsKk=2129157678;if (vcBWjWYoHFROawbLAgSvDPAZjjBsKk == vcBWjWYoHFROawbLAgSvDPAZjjBsKk- 0 ) vcBWjWYoHFROawbLAgSvDPAZjjBsKk=1824448681; else vcBWjWYoHFROawbLAgSvDPAZjjBsKk=973364335;float dUyCnAgvDSpuOvDNIvUeNikvFRtWZu=1989207752.854300502274958891572505201302f;if (dUyCnAgvDSpuOvDNIvUeNikvFRtWZu - dUyCnAgvDSpuOvDNIvUeNikvFRtWZu> 0.00000001 ) dUyCnAgvDSpuOvDNIvUeNikvFRtWZu=1994195754.036878645794256653590927336011f; else dUyCnAgvDSpuOvDNIvUeNikvFRtWZu=1956365600.921025918035798846065991211954f;if (dUyCnAgvDSpuOvDNIvUeNikvFRtWZu - dUyCnAgvDSpuOvDNIvUeNikvFRtWZu> 0.00000001 ) dUyCnAgvDSpuOvDNIvUeNikvFRtWZu=1178491706.364597423175556533960105714081f; else dUyCnAgvDSpuOvDNIvUeNikvFRtWZu=1598950162.416587221252390154589800427998f;if (dUyCnAgvDSpuOvDNIvUeNikvFRtWZu - dUyCnAgvDSpuOvDNIvUeNikvFRtWZu> 0.00000001 ) dUyCnAgvDSpuOvDNIvUeNikvFRtWZu=1313582345.344277908941382981667151361616f; else dUyCnAgvDSpuOvDNIvUeNikvFRtWZu=1819645712.144391813325774863965633230401f;if (dUyCnAgvDSpuOvDNIvUeNikvFRtWZu - dUyCnAgvDSpuOvDNIvUeNikvFRtWZu> 0.00000001 ) dUyCnAgvDSpuOvDNIvUeNikvFRtWZu=502686204.740290710217945349751986261226f; else dUyCnAgvDSpuOvDNIvUeNikvFRtWZu=168467377.252278669525385870809175196403f;if (dUyCnAgvDSpuOvDNIvUeNikvFRtWZu - dUyCnAgvDSpuOvDNIvUeNikvFRtWZu> 0.00000001 ) dUyCnAgvDSpuOvDNIvUeNikvFRtWZu=616221944.060407687779134760159152818828f; else dUyCnAgvDSpuOvDNIvUeNikvFRtWZu=829394525.226656608830313014143736592870f;if (dUyCnAgvDSpuOvDNIvUeNikvFRtWZu - dUyCnAgvDSpuOvDNIvUeNikvFRtWZu> 0.00000001 ) dUyCnAgvDSpuOvDNIvUeNikvFRtWZu=627901366.997056459817342084081578669026f; else dUyCnAgvDSpuOvDNIvUeNikvFRtWZu=1263007260.583905446876634740461899052040f;int WSLRPTUUofWrKIXuQUwJtbgakvXvXL=1109793583;if (WSLRPTUUofWrKIXuQUwJtbgakvXvXL == WSLRPTUUofWrKIXuQUwJtbgakvXvXL- 1 ) WSLRPTUUofWrKIXuQUwJtbgakvXvXL=344807165; else WSLRPTUUofWrKIXuQUwJtbgakvXvXL=786714820;if (WSLRPTUUofWrKIXuQUwJtbgakvXvXL == WSLRPTUUofWrKIXuQUwJtbgakvXvXL- 0 ) WSLRPTUUofWrKIXuQUwJtbgakvXvXL=304767492; else WSLRPTUUofWrKIXuQUwJtbgakvXvXL=1664062702;if (WSLRPTUUofWrKIXuQUwJtbgakvXvXL == WSLRPTUUofWrKIXuQUwJtbgakvXvXL- 0 ) WSLRPTUUofWrKIXuQUwJtbgakvXvXL=1153377085; else WSLRPTUUofWrKIXuQUwJtbgakvXvXL=1120976033;if (WSLRPTUUofWrKIXuQUwJtbgakvXvXL == WSLRPTUUofWrKIXuQUwJtbgakvXvXL- 0 ) WSLRPTUUofWrKIXuQUwJtbgakvXvXL=739456102; else WSLRPTUUofWrKIXuQUwJtbgakvXvXL=1510240898;if (WSLRPTUUofWrKIXuQUwJtbgakvXvXL == WSLRPTUUofWrKIXuQUwJtbgakvXvXL- 0 ) WSLRPTUUofWrKIXuQUwJtbgakvXvXL=368757673; else WSLRPTUUofWrKIXuQUwJtbgakvXvXL=1029374160;if (WSLRPTUUofWrKIXuQUwJtbgakvXvXL == WSLRPTUUofWrKIXuQUwJtbgakvXvXL- 1 ) WSLRPTUUofWrKIXuQUwJtbgakvXvXL=205292807; else WSLRPTUUofWrKIXuQUwJtbgakvXvXL=448454630;long aboFDjVevKULeHnwGHjpFSvzFzyPtE=568720216;if (aboFDjVevKULeHnwGHjpFSvzFzyPtE == aboFDjVevKULeHnwGHjpFSvzFzyPtE- 0 ) aboFDjVevKULeHnwGHjpFSvzFzyPtE=1842076413; else aboFDjVevKULeHnwGHjpFSvzFzyPtE=722989131;if (aboFDjVevKULeHnwGHjpFSvzFzyPtE == aboFDjVevKULeHnwGHjpFSvzFzyPtE- 0 ) aboFDjVevKULeHnwGHjpFSvzFzyPtE=521621833; else aboFDjVevKULeHnwGHjpFSvzFzyPtE=638532165;if (aboFDjVevKULeHnwGHjpFSvzFzyPtE == aboFDjVevKULeHnwGHjpFSvzFzyPtE- 1 ) aboFDjVevKULeHnwGHjpFSvzFzyPtE=1365597152; else aboFDjVevKULeHnwGHjpFSvzFzyPtE=1325082476;if (aboFDjVevKULeHnwGHjpFSvzFzyPtE == aboFDjVevKULeHnwGHjpFSvzFzyPtE- 1 ) aboFDjVevKULeHnwGHjpFSvzFzyPtE=1115136027; else aboFDjVevKULeHnwGHjpFSvzFzyPtE=2081802024;if (aboFDjVevKULeHnwGHjpFSvzFzyPtE == aboFDjVevKULeHnwGHjpFSvzFzyPtE- 0 ) aboFDjVevKULeHnwGHjpFSvzFzyPtE=2036829179; else aboFDjVevKULeHnwGHjpFSvzFzyPtE=622683569;if (aboFDjVevKULeHnwGHjpFSvzFzyPtE == aboFDjVevKULeHnwGHjpFSvzFzyPtE- 0 ) aboFDjVevKULeHnwGHjpFSvzFzyPtE=868080248; else aboFDjVevKULeHnwGHjpFSvzFzyPtE=1321325472;float itzTxkHNqHMPDccpxILnjCwWeCyIbv=549028722.181295211443942694347739386595f;if (itzTxkHNqHMPDccpxILnjCwWeCyIbv - itzTxkHNqHMPDccpxILnjCwWeCyIbv> 0.00000001 ) itzTxkHNqHMPDccpxILnjCwWeCyIbv=1839379620.980431835915566691432280346391f; else itzTxkHNqHMPDccpxILnjCwWeCyIbv=714655098.083666351196043227370616371120f;if (itzTxkHNqHMPDccpxILnjCwWeCyIbv - itzTxkHNqHMPDccpxILnjCwWeCyIbv> 0.00000001 ) itzTxkHNqHMPDccpxILnjCwWeCyIbv=628557170.655404201044044734712902247830f; else itzTxkHNqHMPDccpxILnjCwWeCyIbv=1690318428.634264265003744778274742876336f;if (itzTxkHNqHMPDccpxILnjCwWeCyIbv - itzTxkHNqHMPDccpxILnjCwWeCyIbv> 0.00000001 ) itzTxkHNqHMPDccpxILnjCwWeCyIbv=391324333.757141099067071036868977609359f; else itzTxkHNqHMPDccpxILnjCwWeCyIbv=1691636516.904946760478022787195158989055f;if (itzTxkHNqHMPDccpxILnjCwWeCyIbv - itzTxkHNqHMPDccpxILnjCwWeCyIbv> 0.00000001 ) itzTxkHNqHMPDccpxILnjCwWeCyIbv=835830667.285026414537659005602326495616f; else itzTxkHNqHMPDccpxILnjCwWeCyIbv=873219721.872629400637366950999456307964f;if (itzTxkHNqHMPDccpxILnjCwWeCyIbv - itzTxkHNqHMPDccpxILnjCwWeCyIbv> 0.00000001 ) itzTxkHNqHMPDccpxILnjCwWeCyIbv=1216909498.291887614170507606164672842633f; else itzTxkHNqHMPDccpxILnjCwWeCyIbv=1815734455.364655758934793995080181062915f;if (itzTxkHNqHMPDccpxILnjCwWeCyIbv - itzTxkHNqHMPDccpxILnjCwWeCyIbv> 0.00000001 ) itzTxkHNqHMPDccpxILnjCwWeCyIbv=1230192489.508281696296833109095776338469f; else itzTxkHNqHMPDccpxILnjCwWeCyIbv=284208185.395444752241089536807492194296f;int HPjtBBYrxVYkqaVmslsyKifNxzQOcz=1404759119;if (HPjtBBYrxVYkqaVmslsyKifNxzQOcz == HPjtBBYrxVYkqaVmslsyKifNxzQOcz- 1 ) HPjtBBYrxVYkqaVmslsyKifNxzQOcz=1819768321; else HPjtBBYrxVYkqaVmslsyKifNxzQOcz=59181150;if (HPjtBBYrxVYkqaVmslsyKifNxzQOcz == HPjtBBYrxVYkqaVmslsyKifNxzQOcz- 0 ) HPjtBBYrxVYkqaVmslsyKifNxzQOcz=1568414494; else HPjtBBYrxVYkqaVmslsyKifNxzQOcz=2041116871;if (HPjtBBYrxVYkqaVmslsyKifNxzQOcz == HPjtBBYrxVYkqaVmslsyKifNxzQOcz- 1 ) HPjtBBYrxVYkqaVmslsyKifNxzQOcz=642938871; else HPjtBBYrxVYkqaVmslsyKifNxzQOcz=1798729540;if (HPjtBBYrxVYkqaVmslsyKifNxzQOcz == HPjtBBYrxVYkqaVmslsyKifNxzQOcz- 0 ) HPjtBBYrxVYkqaVmslsyKifNxzQOcz=1891680078; else HPjtBBYrxVYkqaVmslsyKifNxzQOcz=1167208336;if (HPjtBBYrxVYkqaVmslsyKifNxzQOcz == HPjtBBYrxVYkqaVmslsyKifNxzQOcz- 1 ) HPjtBBYrxVYkqaVmslsyKifNxzQOcz=1819062440; else HPjtBBYrxVYkqaVmslsyKifNxzQOcz=370449115;if (HPjtBBYrxVYkqaVmslsyKifNxzQOcz == HPjtBBYrxVYkqaVmslsyKifNxzQOcz- 0 ) HPjtBBYrxVYkqaVmslsyKifNxzQOcz=1774717170; else HPjtBBYrxVYkqaVmslsyKifNxzQOcz=530693516;double IBWixPjutBFZQvovKjKLoQphsIMBCr=1402754259.182962740581914146198682118007;if (IBWixPjutBFZQvovKjKLoQphsIMBCr == IBWixPjutBFZQvovKjKLoQphsIMBCr ) IBWixPjutBFZQvovKjKLoQphsIMBCr=276815878.505861148217121042043786050096; else IBWixPjutBFZQvovKjKLoQphsIMBCr=1803747732.584409726310898166217707492081;if (IBWixPjutBFZQvovKjKLoQphsIMBCr == IBWixPjutBFZQvovKjKLoQphsIMBCr ) IBWixPjutBFZQvovKjKLoQphsIMBCr=756558657.116384383342674300890599998802; else IBWixPjutBFZQvovKjKLoQphsIMBCr=1184056465.785620552598407614319329172749;if (IBWixPjutBFZQvovKjKLoQphsIMBCr == IBWixPjutBFZQvovKjKLoQphsIMBCr ) IBWixPjutBFZQvovKjKLoQphsIMBCr=730389302.658167816495831310249450356655; else IBWixPjutBFZQvovKjKLoQphsIMBCr=1614275070.882404135180903450897361121881;if (IBWixPjutBFZQvovKjKLoQphsIMBCr == IBWixPjutBFZQvovKjKLoQphsIMBCr ) IBWixPjutBFZQvovKjKLoQphsIMBCr=177395883.237161453828627966801687963138; else IBWixPjutBFZQvovKjKLoQphsIMBCr=150439897.436406495311718985527619165020;if (IBWixPjutBFZQvovKjKLoQphsIMBCr == IBWixPjutBFZQvovKjKLoQphsIMBCr ) IBWixPjutBFZQvovKjKLoQphsIMBCr=2034642111.832659323168159098019971282744; else IBWixPjutBFZQvovKjKLoQphsIMBCr=1795885759.147558026966247957018811207157;if (IBWixPjutBFZQvovKjKLoQphsIMBCr == IBWixPjutBFZQvovKjKLoQphsIMBCr ) IBWixPjutBFZQvovKjKLoQphsIMBCr=1569106314.778912161218216374434673743746; else IBWixPjutBFZQvovKjKLoQphsIMBCr=1697346344.297929834680691060593482420637;int VKZyLEOylZTArcVAoLjpOLXOsEZdAs=1982597605;if (VKZyLEOylZTArcVAoLjpOLXOsEZdAs == VKZyLEOylZTArcVAoLjpOLXOsEZdAs- 1 ) VKZyLEOylZTArcVAoLjpOLXOsEZdAs=205444108; else VKZyLEOylZTArcVAoLjpOLXOsEZdAs=1610804776;if (VKZyLEOylZTArcVAoLjpOLXOsEZdAs == VKZyLEOylZTArcVAoLjpOLXOsEZdAs- 0 ) VKZyLEOylZTArcVAoLjpOLXOsEZdAs=1649040798; else VKZyLEOylZTArcVAoLjpOLXOsEZdAs=457501313;if (VKZyLEOylZTArcVAoLjpOLXOsEZdAs == VKZyLEOylZTArcVAoLjpOLXOsEZdAs- 1 ) VKZyLEOylZTArcVAoLjpOLXOsEZdAs=1692556214; else VKZyLEOylZTArcVAoLjpOLXOsEZdAs=858925058;if (VKZyLEOylZTArcVAoLjpOLXOsEZdAs == VKZyLEOylZTArcVAoLjpOLXOsEZdAs- 1 ) VKZyLEOylZTArcVAoLjpOLXOsEZdAs=2053573498; else VKZyLEOylZTArcVAoLjpOLXOsEZdAs=40650080;if (VKZyLEOylZTArcVAoLjpOLXOsEZdAs == VKZyLEOylZTArcVAoLjpOLXOsEZdAs- 0 ) VKZyLEOylZTArcVAoLjpOLXOsEZdAs=970458667; else VKZyLEOylZTArcVAoLjpOLXOsEZdAs=1608832481;if (VKZyLEOylZTArcVAoLjpOLXOsEZdAs == VKZyLEOylZTArcVAoLjpOLXOsEZdAs- 0 ) VKZyLEOylZTArcVAoLjpOLXOsEZdAs=2018739635; else VKZyLEOylZTArcVAoLjpOLXOsEZdAs=20875218;int ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL=1798778868;if (ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL == ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL- 1 ) ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL=822176020; else ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL=1974921877;if (ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL == ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL- 0 ) ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL=7392307; else ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL=1958088820;if (ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL == ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL- 1 ) ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL=1445156564; else ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL=1380666643;if (ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL == ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL- 0 ) ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL=674369817; else ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL=205493251;if (ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL == ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL- 1 ) ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL=2029268914; else ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL=1417974597;if (ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL == ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL- 0 ) ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL=6474760; else ZDPRLqnqokyAkBDWKFpDKqsdUJlhrL=994536759;long bIBwyoArBIGAjnugogvRkufRndudmq=988134598;if (bIBwyoArBIGAjnugogvRkufRndudmq == bIBwyoArBIGAjnugogvRkufRndudmq- 0 ) bIBwyoArBIGAjnugogvRkufRndudmq=2050483643; else bIBwyoArBIGAjnugogvRkufRndudmq=1939603212;if (bIBwyoArBIGAjnugogvRkufRndudmq == bIBwyoArBIGAjnugogvRkufRndudmq- 1 ) bIBwyoArBIGAjnugogvRkufRndudmq=325249408; else bIBwyoArBIGAjnugogvRkufRndudmq=789574656;if (bIBwyoArBIGAjnugogvRkufRndudmq == bIBwyoArBIGAjnugogvRkufRndudmq- 0 ) bIBwyoArBIGAjnugogvRkufRndudmq=2064057126; else bIBwyoArBIGAjnugogvRkufRndudmq=536057122;if (bIBwyoArBIGAjnugogvRkufRndudmq == bIBwyoArBIGAjnugogvRkufRndudmq- 1 ) bIBwyoArBIGAjnugogvRkufRndudmq=1857681692; else bIBwyoArBIGAjnugogvRkufRndudmq=1565374467;if (bIBwyoArBIGAjnugogvRkufRndudmq == bIBwyoArBIGAjnugogvRkufRndudmq- 1 ) bIBwyoArBIGAjnugogvRkufRndudmq=1897657881; else bIBwyoArBIGAjnugogvRkufRndudmq=715908577;if (bIBwyoArBIGAjnugogvRkufRndudmq == bIBwyoArBIGAjnugogvRkufRndudmq- 1 ) bIBwyoArBIGAjnugogvRkufRndudmq=213597798; else bIBwyoArBIGAjnugogvRkufRndudmq=1269040468; }
 bIBwyoArBIGAjnugogvRkufRndudmqy::bIBwyoArBIGAjnugogvRkufRndudmqy()
 { this->KxxByMjTsiKh("ngOqgiCHahcbQjoFntTGYpwZgdkJUuKxxByMjTsiKhj", true, 1743019082, 156520665, 233520551); }
#pragma optimize("", off)
 // <delete/>

