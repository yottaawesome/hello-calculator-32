module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>
#include <wingdi.h>

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
		::UINT32,
		::UINT_PTR,
		::DWORD_PTR,
		::MessageBoxA,
		::MessageBoxW,
		::DefSubclassProc,
		::SetWindowSubclass,
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
		::ShowWindow,
		::InitCommonControls,
		::AllocConsole,
		::WriteConsoleA,
		::WriteConsoleW,
		::FreeConsole,
		::INITCOMMONCONTROLSEX,
		::InitCommonControlsEx,
		::SetWindowRgn,
		::CreateRectRgn,
		::CreateRoundRectRgn
		;

	constexpr auto Gwlp_UserData = GWLP_USERDATA;
	constexpr auto CwUseDefault = CW_USEDEFAULT;

	namespace InitCommonControlsFlag
	{
		enum
		{
			Animate = ICC_ANIMATE_CLASS,
			NativeClass = ICC_NATIVEFNTCTL_CLASS,
			Standard = ICC_STANDARD_CLASSES,
			Cool = ICC_COOL_CLASSES,
			Bar = ICC_BAR_CLASSES
		};
	}


	namespace MessageBoxStyle
	{
		enum
		{
			OK = MB_OK
		};
	}

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

	namespace Styles
	{
		constexpr auto Child = WS_CHILD;
		constexpr auto ClipSiblings = WS_CLIPSIBLINGS;
		constexpr auto Visible = WS_VISIBLE;
		constexpr auto OverlappedWindow = WS_OVERLAPPEDWINDOW;
		constexpr auto HScroll = WS_HSCROLL;
		constexpr auto VScroll = WS_VSCROLL;
		constexpr auto PushButton = BS_DEFPUSHBUTTON;
		constexpr auto VRedraw = CS_VREDRAW;
		constexpr auto HRedraw = CS_HREDRAW;
		constexpr auto AutoCheckBox = BS_AUTOCHECKBOX;
		constexpr auto PushLike = BS_PUSHLIKE;
	}

	constexpr Win32Constant<IDI_APPLICATION> IdiApplication;
	constexpr Win32Constant<IDC_ARROW> IdcArrow;
}
