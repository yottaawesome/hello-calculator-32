export module calculator:ui_toplevelwindow;
import std;
import :win32;
import :error;
import :raii;
import :log;
import :ui_common;
import :ui_control;
import :ui_font;
import :ui_capability;

export namespace UI
{
	struct TopLevelWindow : Window, Textable
	{
		//
		// Creates the window.
		auto Create(this auto&& self) -> decltype(auto)
		{
			static bool registered =
				[&self]<typename T = std::remove_cvref_t<decltype(self)>>()
				{
					Win32::WNDCLASSEXW wndClass = self.GetClass();
					wndClass.lpfnWndProc = WindowProc<T>;
					if (auto atom = Win32::RegisterClassExW(&wndClass); atom != 0)
						return true;

					const auto lastError = Win32::GetLastError();
					if (lastError == Win32::ErrorCodes::ClassAlreadyExists)
						return true;

					throw Error::Win32Error(lastError, "RegisterClassExW failed.");
				}();

			CreateWindowArgs args = self.GetCreationArgs();
			Win32::HWND hwnd = Win32::CreateWindowExW(
				args.ExtendedStyle,
				self.ClassName().data(),
				args.WindowName,
				args.Style,
				args.X,
				args.Y,
				args.Width,
				args.Height,
				args.hWndParent,
				args.Menu,
				Win32::GetModuleHandleW(nullptr),
				&self
			);
			if (not hwnd)
				throw Error::Win32Error(Win32::GetLastError(), "CreateWindowExW failed.");

			// Ownership of hwnd is established in WM_NCCREATE. Do not reassign here.
			if constexpr (requires { self.Init(); })
				self.Init();

			return std::forward<decltype(self)>(self);
		}

		//
		// Applications call this to retrieve messages.
		auto MainLoop(this auto&& self) -> Win32::LRESULT
		{
			if constexpr (true) // Basic loop
			{
				Win32::MSG msg{};
				while (Win32::GetMessageW(&msg, nullptr, 0, 0))
				{
					Win32::TranslateMessage(&msg);
					Win32::DispatchMessageW(&msg);
				}
				return msg.wParam;
			}
			else // Optional processing loop
			{
				Win32::MSG msg{};
				while (msg.message != Win32::Messages::Quit)
				{
					if (Win32::PeekMessageW(&msg, self.GetHandle(), 0, 0, Win32::PeekMessageOptions::Remove))
					{
						Win32::TranslateMessage(&msg);
						Win32::DispatchMessageW(&msg);
					}
					else
					{
						// idle, add processing here
					}
				}
			}
		}

	protected:
		static constexpr std::array HandledMessages{
			Win32::Messages::Destroy,
			Win32::Messages::Paint,
			Win32::Messages::KeyUp,
			Win32::Messages::Command,
			Win32::Messages::NonClientDestroy
		};

		//
		// The main Window proc.
		template<typename TWindow>
		static auto WindowProc(Win32::HWND hwnd, unsigned uMsg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
		{
			TWindow* pThis = nullptr;

			if (uMsg == Win32::Messages::NonClientCreate)
			{
				auto* pCreate = reinterpret_cast<Win32::CREATESTRUCT*>(lParam);
				pThis = reinterpret_cast<TWindow*>(pCreate->lpCreateParams);
				Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp_UserData, reinterpret_cast<Win32::LONG_PTR>(pThis));

				// Adopt ownership once, during creation
				pThis->m_window = Raii::HwndUniquePtr(hwnd);
			}
			else
			{
				pThis = reinterpret_cast<TWindow*>(Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp_UserData));

				// Detach before the OS destroys the HWND to avoid double-destroy later.
				// Caught by AI, more information here https://learn.microsoft.com/en-us/cpp/mfc/tn017-destroying-window-objects?view=msvc-170
				// under the "Auto cleanup with CWnd::PostNcDestroy" header.
				if (pThis and uMsg == Win32::Messages::NonClientDestroy)
				{
					Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp_UserData, 0);
					pThis->m_window.release();
				}
			}

			return pThis
				? pThis->HandleMessage(hwnd, uMsg, wParam, lParam)
				: Win32::DefWindowProcW(hwnd, uMsg, wParam, lParam);
		}

		//
		// Called by WindowProc, which then dispatches the message to either the generic handler
		// or specific handlers by subclasses.
		auto HandleMessage(
			this auto&& self,
			Win32::HWND hwnd,
			unsigned msgType,
			Win32::WPARAM wParam,
			Win32::LPARAM lParam
		) -> Win32::LRESULT
		{
			return[&self, hwnd, msgType, wParam, lParam]<size_t...Is>(std::index_sequence<Is...>)
			{
				Win32::LRESULT result;
				bool handled = (... or
					[=, &self, &result]<typename TMsg = Win32Message<std::get<Is>(HandledMessages)>>()
					{
						if constexpr (Handles<decltype(self), TMsg>)
							return TMsg::uMsg == msgType ? (result = self.OnMessage(TMsg{ hwnd, wParam, lParam }), true) : false;
						return false;
					}());
				if (handled)
					return result;
				return msgType == Win32::Messages::Destroy
					? (Win32::PostQuitMessage(0), 0)
					: Win32::DefWindowProcW(hwnd, msgType, wParam, lParam);
			}(std::make_index_sequence<HandledMessages.size()>());
		}
	};
}
