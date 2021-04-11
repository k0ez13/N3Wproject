#pragma once

#define NOMINMAX
#include <Windows.h>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <d3d9.h>

enum class KeyState
{
    None = 1,
    Down,
    Up,
    Pressed /*Down and then up*/
};

DEFINE_ENUM_FLAG_OPERATORS(KeyState);

namespace input_sys
{

    HWND            m_hTargetWindow;
    LONG_PTR        m_ulOldWndProc;
    KeyState       m_iKeyMap[256];

    std::function<void(void)> m_Hotkeys[256];

    void remove();

    void initialize();

    HWND get_main_window() { return m_hTargetWindow; }

    KeyState      get_key_state(uint32_t vk);
    bool          is_key_down(uint32_t vk);
    bool          was_key_pressed(uint32_t vk);

    void register_hotkey(uint32_t vk, std::function<void(void)> f);
    void remove_hotkey(uint32_t vk);

    static LRESULT WINAPI wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool process_message(UINT uMsg, WPARAM wParam, LPARAM lParam);
    bool process_mouse_message(UINT uMsg, WPARAM wParam, LPARAM lParam);
    bool process_keybd_message(UINT uMsg, WPARAM wParam, LPARAM lParam);
}