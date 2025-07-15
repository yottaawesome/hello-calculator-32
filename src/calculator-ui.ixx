export module calculator:ui;
import std;
import :win32;
import :error;

export namespace UI
{
	struct CreateArgs
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
	// Strongly typed message handler.
	template<Win32::DWORD VMsg>
	struct Win32Message
	{
		static constexpr Win32::UINT uMsg = VMsg;
		Win32::HWND Hwnd = nullptr;
		Win32::WPARAM wParam = 0;
		Win32::LPARAM lParam = 0;
	};

	struct GenericWin32Message
	{
		Win32::HWND Hwnd = nullptr;
		Win32::UINT uMsg = 0;
		Win32::WPARAM wParam = 0;
		Win32::LPARAM lParam = 0;
	};

	struct Window
	{
		//
		// Registers the window class.
		auto Register(this auto&& self) -> decltype(self)
		{
			auto wndClass = self.GetClass();
			wndClass.lpfnWndProc = WindowProc<std::remove_cvref_t<decltype(self)>>;
			if (Win32::ATOM atom = Win32::RegisterClassExW(&wndClass); atom)
				return self;
			throw Error::Win32Error{};
		}

		//
		// Creates the window.
		auto Create(this auto& self) -> decltype(self)
		{
			CreateArgs args = self.CreateArgs();
			self.m_hwnd = Win32::CreateWindowExW(
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
			return self;
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
					if (Win32::PeekMessageW(&msg, self.m_hwnd, 0, 0, Win32::PeekMessageOptions::Remove))
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

		//
		// Shows or hide the window, if present.
		auto ShowOrHide(this auto&& self, bool show) noexcept -> decltype(self)
		{
			if (self.m_hwnd)
				Win32::ShowWindow(self.m_hwnd, show ? Win32::ShowWindowOptions::ShowNormal : Win32::ShowWindowOptions::Hide);
			return self;
		}

	protected:
		//
		// The main Window proc.
		template<typename TWindow>
		static auto WindowProc(Win32::HWND hwnd, unsigned uMsg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
		{
			TWindow* pThis = nullptr;

			if (uMsg == Win32::Messages::NonClientCreate)
			{
				Win32::CREATESTRUCT* pCreate = (Win32::CREATESTRUCT*)lParam;
				pThis = (TWindow*)pCreate->lpCreateParams;
				Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp_UserData, (Win32::LONG_PTR)pThis);

				pThis->m_hwnd = hwnd;
			}
			else
			{
				pThis = (TWindow*)GetWindowLongPtrW(hwnd, Win32::Gwlp_UserData);
			}

			return pThis
				? pThis->HandleMessage(hwnd, uMsg, wParam, lParam)
				: Win32::DefWindowProcW(hwnd, uMsg, wParam, lParam);
		}

		//
		// Called by WindowProc, which then dispatches the message to either the generic handler
		// or specific handlers by subclasses.
		auto HandleMessage(this auto&& self, Win32::HWND hwnd, unsigned uMsg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
		{
			if (uMsg == Win32::Messages::Destroy)
				return self.Process(Win32Message<Win32::Messages::Destroy>{ hwnd, wParam, lParam });
			if (uMsg == Win32::Messages::Paint)
				return self.Process(Win32Message<Win32::Messages::Paint>{ hwnd, wParam, lParam });
			return self.Process(GenericWin32Message{ hwnd, uMsg, wParam, lParam });
		}

		//
		// The generic message handler.
		auto Process(this Window& self, auto&& args) noexcept -> Win32::LRESULT
		{
			if (args.uMsg == Win32::Messages::Destroy)
			{
				Win32::PostQuitMessage(0);
				return 0;
			}
			return Win32::DefWindowProcW(args.Hwnd, args.uMsg, args.wParam, args.lParam);
		}

		Win32::HWND m_hwnd = nullptr;
	};

	struct MainWindow : Window
	{
		constexpr auto ClassName(this auto&&) noexcept -> std::wstring_view { return L"Calculator-Gui"; }

		auto CreateArgs() -> CreateArgs
		{
			return { L"This is test!", Win32::WindowStyles::WsOverlappedWindow };
		}

		auto GetClass(this auto&& self) noexcept -> Win32::WNDCLASSEXW
		{
			return Win32::WNDCLASSEXW{
				.cbSize = sizeof(Win32::WNDCLASSEXW),
				.lpfnWndProc = nullptr,
				.hInstance = Win32::GetModuleHandleW(nullptr),
				.hIcon = Win32::LoadIconW(nullptr, Win32::IdiApplication),
				.hCursor = Win32::LoadCursorW(nullptr, Win32::IdcArrow),
				.hbrBackground = static_cast<Win32::HBRUSH>(Win32::GetStockObject(Win32::Brushes::White)),
				.lpszClassName = self.ClassName().data()
			};
		}
	};
}
