export module calculator:ui.common;
import std;
import :win32;
import :raii;

export namespace UI
{
	// A value handles the message (including zero); nullopt requests default processing.
	using MessageResult = std::optional<Win32::LRESULT>;

	template<typename T, typename M>
	concept Handles = requires(T t, M m)
	{
		{ t.OnMessage(m) } -> std::convertible_to<MessageResult>;
	};

	inline void ReportCallbackException(const char* detail) noexcept
	{
		Win32::OutputDebugStringA("Unhandled window callback exception: ");
		Win32::OutputDebugStringA(detail);
		Win32::OutputDebugStringA("\n");
		Win32::PostQuitMessage(1);
	}

	struct CreateWindowArgs
	{
		Win32::PCWSTR WindowName = nullptr;
		Win32::DWORD Style = 0;
		Win32::DWORD ExtendedStyle = 0;
		int X = Win32::CwUseDefault;
		int Y = Win32::CwUseDefault;
		int Width = Win32::CwUseDefault;
		int Height = Win32::CwUseDefault;
		Win32::HWND hWndParent = 0;
		Win32::HMENU Menu = 0;
	};

	//
	// Strongly-typed message.
	template<Win32::DWORD VMsg>
	struct Win32Message
	{
		static constexpr std::uint32_t uMsg = VMsg;
		Win32::HWND Hwnd = nullptr;
		Win32::WPARAM wParam = 0;
		Win32::LPARAM lParam = 0;
		auto operator==(this auto&&, std::uint32_t msg) noexcept -> bool 
		{ 
			return VMsg == msg; 
		}
	};

	struct GenericWin32Message
	{
		Win32::HWND Hwnd = nullptr;
		Win32::UINT uMsg = 0;
		Win32::WPARAM wParam = 0;
		Win32::LPARAM lParam = 0;
	};

	template<Win32::DWORD VMsg>
		requires (VMsg == Win32::Messages::Command or VMsg == Win32::Messages::Notify or VMsg == Win32::Messages::DrawItem)
	auto NotificationSource(Win32Message<VMsg> message) noexcept -> Win32::HWND
	{
		if constexpr (VMsg == Win32::Messages::Command)
			return reinterpret_cast<Win32::HWND>(message.lParam);
		else if constexpr (VMsg == Win32::Messages::Notify)
			return message.lParam ? reinterpret_cast<const Win32::NMHDR*>(message.lParam)->hwndFrom : nullptr;
		else
			return message.lParam and reinterpret_cast<const Win32::DRAWITEMSTRUCT*>(message.lParam)->CtlType != Win32::OdtMenu
				? reinterpret_cast<const Win32::DRAWITEMSTRUCT*>(message.lParam)->hwndItem
				: nullptr;
	}

	class Window
	{
	public:
		constexpr Window() = default;
		Window(const Window&) = delete;
		Window(Window&&) = delete;
		auto operator=(const Window&) -> Window& = delete;
		auto operator=(Window&&) -> Window& = delete;

		auto GetHandle(this auto&& self) noexcept { return self.m_window.get(); }
		void Destroy(this auto&& self) noexcept { self.m_window.reset(); }
		void SetFont(this auto&& self, Win32::HFONT font)
		{
			if (self.m_window)
				Win32::SendMessageW(self.m_window.get(), Win32::Messages::SetFont, reinterpret_cast<Win32::WPARAM>(font), true);
		}
		//
		// Shows or hide the window, if present.
		auto Show(this auto&& self) noexcept -> decltype(auto)
		{
			if (self.m_window)
				Win32::ShowWindow(self.m_window.get(), Win32::ShowWindowOptions::ShowNormal);
			return std::forward<decltype(self)>(self);
		}

		auto Hide(this auto&& self) noexcept -> decltype(auto)
		{
			if (self.m_window)
				Win32::ShowWindow(self.m_window.get(), Win32::ShowWindowOptions::Hide);
			return std::forward<decltype(self)>(self);
		}

		auto TakeFocus(this auto&& self) -> decltype(auto)
		{
			if (self.m_window)
				Win32::SetFocus(self.m_window.get());
			return std::forward<decltype(self)>(self);
		}
	protected:
		Raii::HwndUniquePtr m_window = nullptr;
	};
}
