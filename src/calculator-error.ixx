export module calculator:error;
import std;
import :win32;
import :raii;

export namespace Error
{
	struct SystemMessage
	{
		SystemMessage(Win32::DWORD code)
		{

		}

		static auto Format(Win32::DWORD errorCode) noexcept -> std::string
		{
			void* messageBuffer = nullptr;

			constexpr auto Flags =
				Win32::FormatMessageOptions::AllocateBuffer
				| Win32::FormatMessageOptions::FromSystem
				| Win32::FormatMessageOptions::IgnoreInserts;
			Win32::FormatMessageA(
				Flags,
				nullptr, //module
				errorCode,
				0,
				reinterpret_cast<char*>(&messageBuffer),
				0,
				nullptr
			);
			if (not messageBuffer)
			{
				auto lastError = Win32::GetLastError();
				return std::format("Failed formatting code {} with error {}.", errorCode, lastError);
			}
			Raii::LocalHeapUniquePtr ptr(messageBuffer);
			std::string_view msg(static_cast<char*>(messageBuffer));
			// This should never happen
			// See also https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-raisefailfastexception
			if (Win32::LocalFree(messageBuffer))
				Win32::__fastfail(Win32::FailFast::Fatal);

			return msg | std::ranges::views::filter([](auto c) { return c != '\n' and c != '\r'; }) | std::ranges::to<std::string>();
		}

		std::string Message;
	};

	struct Error : std::exception
	{
		Error(std::string_view error) : std::exception(error.data()) {}
	};

	struct Win32Error : Error
	{
		Win32Error(Win32::DWORD errorCode = Win32::GetLastError(), std::source_location loc = std::source_location::current())
			: Error(Format(errorCode, "", loc))
		{
		}

		Win32Error(Win32::DWORD errorCode, std::string_view message, std::source_location loc = std::source_location::current())
			: Error(Format(errorCode, "", loc))
		{
		}

		auto Format(
			this auto&& self,
			Win32::DWORD errorCode,
			std::string_view message,
			const std::source_location& loc
		) -> std::string
		{
			return std::format("{}", errorCode);
		}
	};
}
