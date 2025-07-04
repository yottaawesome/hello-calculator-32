module;

#include <Windows.h>

export module win32;

export namespace Win32
{
	using
		::HANDLE,
		::ATOM,
		::HINSTANCE,
		::HMODULE,
		::HWND,
		::PCWSTR,
		::PWSTR,
		::WNDCLASSW,
		::WNDCLASSEXW,
		::CloseHandle,
		::WaitForSingleObject,
		::WaitForSingleObjectEx,
		::WaitForMultipleObjects,
		::WaitForMultipleObjectsEx,
		::RegisterClassW,
		::RegisterClassExW,
		::ShowWindow,
		::ShowWindowAsync,
		::GetModuleHandleW
		;
}
