export module calculator:ui_capability;
import std;
import :win32;

export namespace UI
{
	struct Textable
	{
		auto GetText(this const auto& self) -> std::wstring
		{
			auto handle = self.GetHandle();
			if (not handle)
				return {};
			std::wstring buffer(Win32::GetWindowTextLengthW(handle), '\0');
			Win32::GetWindowTextW(self.GetHandle(), buffer.data(), static_cast<int>(buffer.size()));
			return buffer;
		}
		auto SetText(this auto&& self, std::wstring_view text)
		{
			if (auto handle = self.GetHandle())
				Win32::SetWindowTextW(handle, text.data());
		}
		auto AppendText(this auto&& self, std::wstring_view text)
		{
			self.SetText(std::format(L"{}{}"), self.GetText(), text);
		}
	};
}