export module calculator:string;
import std;
import :win32;

export namespace String
{
	template<size_t N, typename TChar>
	struct FixedString
	{
		using TStringView = std::basic_string_view<TChar, std::char_traits<TChar>>;
		using TString = std::basic_string<TChar, std::char_traits<TChar>>;

		TChar Buffer[N];

		consteval FixedString(const TChar(&buffer)[N]) noexcept
		{
			std::copy_n(buffer, N, Buffer);
		}

		constexpr auto ToView(this const auto& self) noexcept -> TStringView
		{
			return TStringView{ self.Buffer };
		}

		constexpr auto ToString(this const auto& self) noexcept -> TString
		{
			return TString{ self.Buffer };
		}
	};
	template<size_t N>
	FixedString(const char(&)[N]) -> FixedString<N, char>;
	template<size_t N>
	FixedString(const wchar_t(&)[N]) -> FixedString<N, wchar_t>;

    auto ConvertString(const std::wstring_view wstr) -> std::string
    {
        if (wstr.empty())
            return {};

        // https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-widechartomultibyte
        // Returns the size in bytes, this differs from MultiByteToWideChar, which returns the size in characters
        int sizeInBytes = Win32::WideCharToMultiByte(
            Win32::CpUtf8,										// CodePage
            Win32::WcNoBestFitChars,							// dwFlags 
            &wstr[0],										// lpWideCharStr
            static_cast<int>(wstr.size()),					// cchWideChar 
            nullptr,										// lpMultiByteStr
            0,												// cbMultiByte
            nullptr,										// lpDefaultChar
            nullptr											// lpUsedDefaultChar
        );
        if (sizeInBytes == 0)
            throw std::runtime_error("WideCharToMultiByte() [1] failed");

        std::string strTo(sizeInBytes / sizeof(char), '\0');
        int status = Win32::WideCharToMultiByte(
            Win32::CpUtf8,										// CodePage
            Win32::WcNoBestFitChars,							// dwFlags 
            &wstr[0],										// lpWideCharStr
            static_cast<int>(wstr.size()),					// cchWideChar 
            &strTo[0],										// lpMultiByteStr
            static_cast<int>(strTo.size() * sizeof(char)),	// cbMultiByte
            nullptr,										// lpDefaultChar
            nullptr											// lpUsedDefaultChar
        );
        if (status == 0)
            throw std::runtime_error("WideCharToMultiByte() [2] failed");

        return strTo;
    }

    auto ConvertString(const std::string_view str) -> std::wstring
    {
        if (str.empty())
            return {};

        // https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
        // Returns the size in characters, this differs from WideCharToMultiByte, which returns the size in bytes
        int sizeInCharacters = Win32::MultiByteToWideChar(
            Win32::CpUtf8,									// CodePage
            0,											// dwFlags
            &str[0],									// lpMultiByteStr
            static_cast<int>(str.size() * sizeof(char)),// cbMultiByte
            nullptr,									// lpWideCharStr
            0											// cchWideChar
        );
        if (sizeInCharacters == 0)
            throw std::runtime_error("MultiByteToWideChar() [1] failed");

        std::wstring wstrTo(sizeInCharacters, '\0');
        int status = Win32::MultiByteToWideChar(
            Win32::CpUtf8,									// CodePage
            0,											// dwFlags
            &str[0],									// lpMultiByteStr
            static_cast<int>(str.size() * sizeof(char)),	// cbMultiByte
            &wstrTo[0],									// lpWideCharStr
            static_cast<int>(wstrTo.size())				// cchWideChar
        );
        if (status == 0)
            throw std::runtime_error("MultiByteToWideChar() [2] failed");

        return wstrTo;
    }
}
