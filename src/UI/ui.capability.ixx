export module calculator:ui.capability;
import std;
import :win32;

export namespace UI
{
	template<unsigned VVirtualKeyCode>
	struct KeyBindable
	{
		constexpr auto KeyCode(this auto&&) noexcept -> unsigned { return VVirtualKeyCode; }
	};

	struct Textable
	{
		auto GetText(this const auto& self) -> std::wstring
		{
			auto handle = self.GetHandle();
			if (not handle)
				return {};
			std::wstring buffer(Win32::GetWindowTextLengthW(handle)+1, '\0');
			Win32::GetWindowTextW(self.GetHandle(), buffer.data(), static_cast<int>(buffer.size()));
			// Remove trailing null character.
			return buffer.empty() ? buffer : (buffer.pop_back(), buffer);
		}

		auto SetText(this auto&& self, std::wstring_view text)
		{
			// Beware of embedded nulls -- these will cut the displayed text short.
			if (auto handle = self.GetHandle())
				Win32::SetWindowTextW(handle, text.data());
		}

		auto AppendText(this auto&& self, std::wstring_view text)
		{
			self.SetText(std::format(L"{}{}", self.GetText(), text));
		}

		auto ClearText(this auto&& self)
		{
			self.SetText(L"");
		}
	};
}
