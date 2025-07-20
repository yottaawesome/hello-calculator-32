export module calculator:ui_common;
import std;
import :win32;
import :raii;

export namespace UI
{
	//
	// Strongly-typed message.
	template<Win32::DWORD VMsg>
	struct Win32Message
	{
		static constexpr Win32::UINT uMsg = VMsg;
		Win32::HWND Hwnd = nullptr;
		Win32::WPARAM wParam = 0;
		Win32::LPARAM lParam = 0;
	};

	struct GenericWin32Message
	{
		Win32::HWND Hwnd = nullptr;
		Win32::UINT uMsg = 0;
		Win32::WPARAM wParam = 0;
		Win32::LPARAM lParam = 0;
	};

	struct SimpleWindow
	{
	protected:
		Raii::HwndUniquePtr m_window = nullptr;
	};
}
