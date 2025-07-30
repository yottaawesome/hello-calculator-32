module;

#include <stdio.h>
#include <stdlib.h>

export module calculator:log;
import std;
import :win32;

namespace
{
	struct WindowsConsole
	{
		~WindowsConsole()
		{
			Win32::FreeConsole();
		}

		WindowsConsole()
		{
			Win32::AllocConsole();
			FILE* fDummy;
			freopen_s(&fDummy, "CONIN$", "r", stdin);
			freopen_s(&fDummy, "CONOUT$", "w", stderr);
			freopen_s(&fDummy, "CONOUT$", "w", stdout);
		}
	} Console;
}

export namespace Log
{
	template<typename...TArgs>
	void Info(std::format_string<TArgs...> fmt, TArgs&&...args) noexcept
	{
		std::println(fmt, std::forward<TArgs>(args)...);
	}

	template<typename...TArgs>
	void Warn(std::format_string<TArgs...> fmt, TArgs&&...args) noexcept
	{
		std::println(fmt, std::forward<TArgs>(args)...);
	}

	template<typename...TArgs>
	void Error(std::format_string<TArgs...> fmt, TArgs&&...args) noexcept
	{
		std::println(fmt, std::forward<TArgs>(args)...);
	}
}
