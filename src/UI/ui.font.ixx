export module calculator:ui.font;
import std;
import :win32;
import :raii;
import :error;

namespace UI
{
	struct Font
	{
		auto Get(this const auto self) -> Win32::HFONT
		{
			static Raii::FontUniquePtr font{ self.Create() };
			return font.get();
		}

		operator Win32::HFONT(this const auto self)
		{
			return self.Get();
		}
	};

	struct SystemFontLoader : Font
	{
		auto Create(this const auto) -> Win32::HFONT
		{
			Win32::NONCLIENTMETRICS metrics{ .cbSize = sizeof(metrics) };
			Win32::SystemParametersInfoW(Win32::SpiGetNonClientMetrics, metrics.cbSize, &metrics, 0);
			// When you're done with the font, don't forget to call Win32::DeleteObject();
			Win32::HFONT font = Win32::CreateFontIndirectW(&metrics.lfCaptionFont);
			if (not font)
				throw Error::Win32Error(Win32::GetLastError(), "Failed to load system font.");
			return font;
		}
	};

	struct SegoeFontLoader : Font
	{
		auto Create(this const auto) -> Win32::HFONT
		{
			Win32::HFONT font = Win32::CreateFontW(
				-32, // Font height (negative for desired height in logical units)
				0,   // Width (0 for default)
				0,   // Escapement
				0,   // Orientation
				Win32::FontWeight::Normal, // Font weight
				false,     // Italic
				false,     // Underline
				false,     // Strikeout
				Win32::DefaultCharset, // Character set
				Win32::OutPrecision::OutOutlinePrecision, // Output precision
				Win32::ClipPrecision::DefaultPrecision, // Clipping precision
				Win32::FontQuality::ClearType, // Quality
				Win32::Pitch::Variable | Win32::FontFamily::Swiss, // Pitch and family
				L"Segoe UI" // Font face name
			);
			if (not font)
				throw Error::Win32Error(Win32::GetLastError(), "Failed to Segoe font.");
			return font;
		}
	};
}

export namespace UI
{
	constexpr SystemFontLoader SystemFont;
	constexpr SegoeFontLoader SegoeFont;
}
