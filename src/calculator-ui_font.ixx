export module calculator:ui_font;
import std;
import :win32;
import :raii;
import :error;

export namespace UI
{
	struct SystemFontLoader
	{
		auto Get() const -> Win32::HFONT
		{
			static Raii::FontUniquePtr font =
				[] -> Raii::FontUniquePtr
				{
					Win32::NONCLIENTMETRICS metrics{ .cbSize = sizeof(metrics) };
					Win32::SystemParametersInfoW(Win32::SpiGetNonClientMetrics, metrics.cbSize, &metrics, 0);
					// When you're done with the font, don't forget to call Win32::DeleteObject();
					Win32::HFONT font = Win32::CreateFontIndirectW(&metrics.lfCaptionFont);
					if (not font)
						throw Error::Win32Error(Win32::GetLastError(), "Failed to load system font.");
					return Raii::FontUniquePtr(font);
				}();

			return font.get();
		}
	};
	
	constexpr SystemFontLoader SystemFont;
}
