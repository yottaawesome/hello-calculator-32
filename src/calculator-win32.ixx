module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wingdi.h>
#include <CommCtrl.h>

export module calculator:win32;
import std;

export namespace Win32
{
	template<auto VValue>
	struct Win32Constant
	{
		operator decltype(VValue)(this auto&&) noexcept
			requires (not std::invocable<decltype(VValue)>)
		{
			return VValue;
		}

		operator decltype(VValue)(this auto&&) noexcept
			requires std::invocable<decltype(VValue)>
		{
			return VValue();
		}
	};

	using
		::BOOL,
		::HMENU,
		::HANDLE,
		::UINT,
		::ATOM,
		::HINSTANCE,
		::HMODULE,
		::HWND,
		::PCWSTR,
		::PWSTR,
		::WNDCLASSW,
		::WNDCLASSEXW,
		::DWORD,
		::HLOCAL,
		::LRESULT,
		::LPARAM,
		::WPARAM,
		::LONG_PTR,
		::CREATESTRUCT,
		::HGDIOBJ,
		::HBRUSH,
		::MSG,
		::DestroyWindow,
		::PostQuitMessage,
		::PeekMessageW,
		::TranslateMessage,
		::DispatchMessageW,
		::GetMessageW,
		::TranslateMessage,
		::DispatchMessageW,
		::GetWindowLongPtrW,
		::SetWindowLongPtrW,
		::CloseHandle,
		::WaitForSingleObject,
		::WaitForSingleObjectEx,
		::WaitForMultipleObjects,
		::WaitForMultipleObjectsEx,
		::RegisterClassW,
		::RegisterClassExW,
		::ShowWindow,
		::ShowWindowAsync,
		::GetModuleHandleW,
		::GetStockObject,
		::LoadIconW,
		::LoadCursorW,
		::GetLastError,
		::FormatMessageA,
		::FormatMessageW,
		::__fastfail,
		::ExitProcess,
		::FatalAppExitW,
		::FatalExit,
		::LocalFree,
		::DefWindowProcW,
		::CreateWindowExW,
		::ShowWindow
		;

	constexpr auto Gwlp_UserData = GWLP_USERDATA;
	constexpr auto CwUseDefault = CW_USEDEFAULT;

	namespace ShowWindowOptions
	{
		enum
		{
			Hide = SW_HIDE,
			ShowNormal = SW_SHOWNORMAL
		};
	}

	namespace PeekMessageOptions
	{
		enum
		{
			NoRemove = PM_NOREMOVE,
			Remove = PM_REMOVE,
			NoYield = PM_NOYIELD
		};
	}

	namespace Color
	{
		enum
		{
			ColorWindow = COLOR_WINDOW
		};
	}

	namespace Messages
	{
		enum
		{
			Quit = WM_QUIT,
			Close = WM_CLOSE,
			Destroy = WM_DESTROY,
			Size = WM_SIZE,
			LeftButtonUp = WM_LBUTTONUP,
			NonClientCreate = WM_NCCREATE,
			Paint = WM_PAINT,
			KeyUp = WM_KEYUP,
			Create = WM_CREATE,
			Command = WM_COMMAND,
			CtlColorBtn = WM_CTLCOLORBTN,
			Notify = WM_NOTIFY,
			CustomDraw = NM_CUSTOMDRAW
		};
	}

	namespace Brushes
	{
		enum
		{
			White = WHITE_BRUSH
		};
	}

	namespace FailFast
	{
		enum
		{
			Fatal = FAST_FAIL_FATAL_APP_EXIT
		};
	}

	namespace FormatMessageOptions
	{
		enum
		{
			AllocateBuffer = FORMAT_MESSAGE_ALLOCATE_BUFFER,
			FromSystem = FORMAT_MESSAGE_FROM_SYSTEM,
			IgnoreInserts = FORMAT_MESSAGE_IGNORE_INSERTS,
			FromHModule = FORMAT_MESSAGE_FROM_HMODULE
		};
	}

	namespace WindowStyles
	{
		enum
		{
			WsOverlappedWindow = WS_OVERLAPPEDWINDOW
		};
	}

	constexpr Win32Constant<IDI_APPLICATION> IdiApplication;
	constexpr Win32Constant<IDC_ARROW> IdcArrow;
}
