export module calculator:ui_mainwindow;
import std;
import :win32;
import :ui_common;
import :ui_window;
import :ui_control;
import :ui_font;

export namespace UI
{
	struct MainWindow : Window
	{
		using Window::Process;

		auto Process(this auto& self, Win32Message<Win32::Messages::Paint> message) -> Win32::LRESULT
		{
			return Win32::DefWindowProcW(message.Hwnd, message.uMsg, message.wParam, message.lParam);
		}

		auto Process(this auto& self, Win32Message<Win32::Messages::KeyUp> message) -> Win32::LRESULT
		{
			Win32::SendMessageW(std::get<0>(self.m_buttons).GetHandle(), Win32::Messages::ButtonClick, 0, 0);
			// Focus needs to be set back to the window, because it goes to the button
			self.TakeFocus();

			return Win32::DefWindowProcW(message.Hwnd, message.uMsg, message.wParam, message.lParam);
		}

		auto ClassName(this const auto&) noexcept -> std::wstring_view { return L"Calculator-Gui"; }

		auto CreateArgs(this const auto&) -> CreateWindowArgs
		{
			return {
				.WindowName = L"Win32 Calculator",
				.Style = Win32::WindowStyles::WsOverlappedWindow,
				.Width = 500,
				.Height = 500
			};
		}

		auto GetClass(this const auto& self) noexcept -> Win32::WNDCLASSEXW
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
			self.SetFont(UI::SystemFont.Get());
			// Create child windows and set their font.
			[]<typename...TArgs>(Win32::HWND parent, std::tuple<TArgs...>&tuple)
			{
				([parent, &control = std::get<TArgs>(tuple)]
				{
					control.Create(parent);
					control.SetFont(UI::SystemFont.Get());
				}(), ...);
			}(self.m_window.get(), self.m_buttons);
		}

	protected:
		static constexpr auto ButtonWidth = 100;
		static constexpr auto ButtonHeight = 50;
		static constexpr auto StartX = 50;
		static constexpr auto StartY = 50;

		using ButtonGroup = std::tuple<
			NumberButton<1, 101, StartX + ButtonWidth * 0, StartY + ButtonHeight * 0, ButtonWidth, ButtonHeight>,
			NumberButton<2, 102, StartX + ButtonWidth * 1, StartY + ButtonHeight * 0, ButtonWidth, ButtonHeight>,
			NumberButton<3, 103, StartX + ButtonWidth * 2, StartY + ButtonHeight * 0, ButtonWidth, ButtonHeight>,
			OperationButton<L"+", 111, StartX + ButtonWidth * 3, StartY + ButtonHeight * 0, ButtonWidth, ButtonHeight>,
			NumberButton<4, 104, StartX + ButtonWidth * 0, StartY + ButtonHeight * 1, ButtonWidth, ButtonHeight>,
			NumberButton<5, 105, StartX + ButtonWidth * 1, StartY + ButtonHeight * 1, ButtonWidth, ButtonHeight>,
			NumberButton<6, 106, StartX + ButtonWidth * 2, StartY + ButtonHeight * 1, ButtonWidth, ButtonHeight>,
			OperationButton<L"-", 112, StartX + ButtonWidth * 3, StartY + ButtonHeight * 1, ButtonWidth, ButtonHeight>,
			NumberButton<7, 107, StartX + ButtonWidth * 0, StartY + ButtonHeight * 2, ButtonWidth, ButtonHeight>,
			NumberButton<8, 108, StartX + ButtonWidth * 1, StartY + ButtonHeight * 2, ButtonWidth, ButtonHeight>,
			NumberButton<9, 109, StartX + ButtonWidth * 2, StartY + ButtonHeight * 2, ButtonWidth, ButtonHeight>,
			OperationButton<L"x", 113, StartX + ButtonWidth * 3, StartY + ButtonHeight * 2, ButtonWidth, ButtonHeight>,
			NumberButton<0, 100, StartX + ButtonWidth * 0, StartY + ButtonHeight * 3, ButtonWidth, ButtonHeight>,
			OperationButton<L".", 114, StartX + ButtonWidth * 1, StartY + ButtonHeight * 3, ButtonWidth, ButtonHeight>,
			OperationButton<L"=", 115, StartX + ButtonWidth * 2, StartY + ButtonHeight * 3, ButtonWidth, ButtonHeight>,
			OperationButton<L"%", 116, StartX + ButtonWidth * 3, StartY + ButtonHeight * 3, ButtonWidth, ButtonHeight>
		>;

		ButtonGroup m_buttons{};
	};
}
