export module calculator:ui;
import std;
import :win32;
import :error;
import :raii;

export namespace UI
{
	//
	// Strongly-typed message.
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

	struct SimpleWindow
	{
	protected:
		Raii::HwndUniquePtr m_window = nullptr;
	};
}

// Controls
export namespace UI
{
	struct ControlProperties
	{
		Win32::DWORD Id;
		std::wstring Class;
		std::wstring Text;
		Win32::DWORD Styles = 0;
		Win32::DWORD ExtendedStyles = 0;
		int X = 0;
		int Y = 0;
		int Width = 0;
		int Height = 0;
	};

	struct Control : SimpleWindow
	{
		Control() = default;
		Control(ControlProperties properties) : m_properties(properties) {}

		auto Create(this auto&& self, Win32::HWND parent) -> void
		{
			Win32::HWND window = Win32::CreateWindowExW(
				self.m_properties.ExtendedStyles,
				self.GetClass().data(),
				self.m_properties.Text.empty() ? nullptr : self.m_properties.Text.data(),
				self.m_properties.Styles,
				self.m_properties.X,
				self.m_properties.Y,
				self.m_properties.Width,
				self.m_properties.Height,
				parent,
				(Win32::HMENU)(Win32::UINT_PTR)(self.m_properties.Id),
				Win32::GetModuleHandleW(nullptr),
				nullptr
			);
			if (self.m_window = Raii::HwndUniquePtr(window); not self.m_window)
				throw Error::Win32Error(Win32::GetLastError(), "Failed creating button.");
			if (not Win32::SetWindowSubclass(self.m_window.get(), SubclassProc<std::remove_cvref_t<decltype(self)>>, 5, reinterpret_cast<Win32::DWORD_PTR>(&self)))
				throw Error::Win32Error(Win32::GetLastError(), "Failed creating button.");

			self.Init();
		}

		auto HandleMessage(
			this auto&& self,
			Win32::HWND hwnd,
			Win32::UINT msg,
			Win32::WPARAM wParam,
			Win32::LPARAM lParam,
			Win32::UINT_PTR uIdSubclass,
			Win32::DWORD_PTR dwRefData
		) -> Win32::LRESULT
		{
			if (msg == Win32::Messages::LeftButtonUp)
				return self.Process(Win32Message<Win32::Messages::LeftButtonUp>{ hwnd, wParam, lParam });
			return self.Process(GenericWin32Message{ .Hwnd = hwnd, .uMsg = msg, .wParam = wParam, .lParam = lParam });
		}

		auto Process(this auto&& self, auto&& msg) -> Win32::LRESULT
		{
			return Win32::DefSubclassProc(msg.Hwnd, msg.uMsg, msg.wParam, msg.lParam);
		}

		template<typename TControl>
		static auto SubclassProc(
			Win32::HWND hwnd,
			Win32::UINT msg,
			Win32::WPARAM wParam,
			Win32::LPARAM lParam,
			Win32::UINT_PTR idSubclass,
			Win32::DWORD_PTR refData
		) -> Win32::LRESULT
		{
			TControl* pThis = reinterpret_cast<TControl*>(refData);
			return pThis
				? pThis->HandleMessage(hwnd, msg, wParam, lParam, idSubclass, refData)
				: Win32::DefSubclassProc(hwnd, msg, wParam, lParam);
		}

	protected:
		ControlProperties m_properties;
	};

	struct Button : Control
	{
		Button() : Control(GetDefaultProperties()) {}

		Button(ControlProperties properties)
			: Control(properties)
		{ }

		void Init(this auto&& self)
		{
			//Win32::SetWindowRgn(self.m_window.get(), Win32::CreateRoundRectRgn(10, 10, 60, 110, 50, 50), true);
		}

		auto GetClass(this auto&&) noexcept -> std::wstring_view
		{
			return L"Button";
		}

		auto GetDefaultProperties(this auto&& self) -> ControlProperties
		{
			return {
				.Id = 100,
				.Class = L"Button",
				.Text = L"Hello",
				.Styles = Win32::Styles::PushButton | Win32::Styles::Child | Win32::Styles::Visible,
				.X = 10,
				.Y = 10,
				.Width = 100,
				.Height = 50
			};
		};
	};

	template<unsigned VValue, unsigned VId, int VX, int VY, int VWidth, int VHeight>
	struct NumberButton : Button
	{
		using Control::HandleMessage;

		NumberButton() : Button(GetDefaultProperties()) {}

		auto HandleMessage(this auto&& self, Win32Message<Win32::Messages::LeftButtonUp>)
		{
			return 0;
		}

		auto GetDefaultProperties(this auto&& self) -> ControlProperties
		{
			return {
				.Id = VId,
				.Class = L"Button",
				.Text = std::to_wstring(VValue),
				.Styles = Win32::Styles::PushButton | Win32::Styles::Child | Win32::Styles::Visible,
				.X = VX,
				.Y = VY,
				.Width = VWidth,
				.Height = VHeight
			};
		};
	};
}

// Windows
export namespace UI
{
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

	struct Window : SimpleWindow
	{
		void Destroy(this auto&& self) noexcept
		{
			self.m_window.reset();
		}

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
			CreateWindowArgs args = self.CreateArgs();
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
			self.m_window = Raii::HwndUniquePtr(hwnd);
			self.Init();
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
		auto Show(this auto&& self) noexcept -> decltype(self)
		{
			if (self.m_window)
				Win32::ShowWindow(self.m_window.get(), Win32::ShowWindowOptions::ShowNormal);
			return self;
		}

		auto Hide(this auto&& self) noexcept -> decltype(self)
		{
			if (self.m_hwnd)
				Win32::ShowWindow(self.m_window.get(), Win32::ShowWindowOptions::Hide);
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

				pThis->m_window = Raii::HwndUniquePtr(hwnd);
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
	};

	struct MainWindow : Window
	{
		auto ClassName(this auto&&) noexcept -> std::wstring_view { return L"Calculator-Gui"; }

		auto CreateArgs(this auto&& self) -> CreateWindowArgs
		{
			return { 
				.WindowName = L"Win32 Calculator", 
				.Style = Win32::WindowStyles::WsOverlappedWindow, 
				.Width = 500, 
				.Height = 500 
			};
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

		auto Init(this auto& self)
		{
			[&self]<typename...TArgs>(std::tuple<TArgs...>& tuple)
			{
				(std::get<TArgs>(tuple).Create(self.m_window.get()), ...);
			}(self.m_buttons);
		}

	protected:
		static constexpr auto NumberButtonWidth = 100;
		static constexpr auto NumberButtonHeight = 50;
		static constexpr auto PaddingX = 10;
		static constexpr auto PaddingY = 10;

		using ButtonGroup = std::tuple<
			NumberButton<1, 101, PaddingX + NumberButtonWidth * 0, PaddingY + NumberButtonHeight * 0, NumberButtonWidth, NumberButtonHeight>,
			NumberButton<2, 102, PaddingX + NumberButtonWidth * 1, PaddingY + NumberButtonHeight * 0, NumberButtonWidth, NumberButtonHeight>,
			NumberButton<3, 103, PaddingX + NumberButtonWidth * 2, PaddingY + NumberButtonHeight * 0, NumberButtonWidth, NumberButtonHeight>,
			NumberButton<4, 104, PaddingX + NumberButtonWidth * 0, PaddingY + NumberButtonHeight * 1, NumberButtonWidth, NumberButtonHeight>,
			NumberButton<5, 105, PaddingX + NumberButtonWidth * 1, PaddingY + NumberButtonHeight * 1, NumberButtonWidth, NumberButtonHeight>,
			NumberButton<6, 106, PaddingX + NumberButtonWidth * 2, PaddingY + NumberButtonHeight * 1, NumberButtonWidth, NumberButtonHeight>,
			NumberButton<7, 107, PaddingX + NumberButtonWidth * 0, PaddingY + NumberButtonHeight * 2, NumberButtonWidth, NumberButtonHeight>,
			NumberButton<8, 108, PaddingX + NumberButtonWidth * 1, PaddingY + NumberButtonHeight * 2, NumberButtonWidth, NumberButtonHeight>,
			NumberButton<9, 109, PaddingX + NumberButtonWidth * 2, PaddingY + NumberButtonHeight * 2, NumberButtonWidth, NumberButtonHeight>,
			NumberButton<0, 100, PaddingX + NumberButtonWidth * 0, PaddingY + NumberButtonHeight * 3, NumberButtonWidth, NumberButtonHeight>
		>;

		ButtonGroup m_buttons{};
	};
}
