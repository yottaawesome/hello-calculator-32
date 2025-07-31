export module calculator:ui_mainwindow;
import std;
import :win32;
import :ui_common;
import :ui_window;
import :ui_control;
import :ui_font;

export namespace UI
{
	constexpr auto ButtonWidth = 100;
	constexpr auto ButtonHeight = 50;
	constexpr auto StartX = 50;
	constexpr auto StartY = 50;
	constexpr auto RowWidth = ButtonWidth * 4;

	// 1st row
	using OutputWindow = Output<1, StartX + ButtonWidth * 0, StartY + ButtonHeight * 0, RowWidth, ButtonHeight>;
	// 2nd row
	using Button1 = NumberButton<1, 101, StartX + ButtonWidth * 0, StartY + ButtonHeight * 1, ButtonWidth, ButtonHeight>;
	using Button2 = NumberButton<2, 102, StartX + ButtonWidth * 1, StartY + ButtonHeight * 1, ButtonWidth, ButtonHeight>;
	using Button3 = NumberButton<3, 103, StartX + ButtonWidth * 2, StartY + ButtonHeight * 1, ButtonWidth, ButtonHeight>;
	using ButtonPlus = OperationButton<L"+", 111, StartX + ButtonWidth * 3, StartY + ButtonHeight * 1, ButtonWidth, ButtonHeight>;
	// 3rd row
	using Button4 = NumberButton<4, 104, StartX + ButtonWidth * 0, StartY + ButtonHeight * 2, ButtonWidth, ButtonHeight>;
	using Button5 = NumberButton<5, 105, StartX + ButtonWidth * 1, StartY + ButtonHeight * 2, ButtonWidth, ButtonHeight>;
	using Button6 = NumberButton<6, 106, StartX + ButtonWidth * 2, StartY + ButtonHeight * 2, ButtonWidth, ButtonHeight>;
	using ButtonMinus = OperationButton<L"-", 112, StartX + ButtonWidth * 3, StartY + ButtonHeight * 2, ButtonWidth, ButtonHeight>;
	// 4th row
	using Button7 = NumberButton<7, 107, StartX + ButtonWidth * 0, StartY + ButtonHeight * 3, ButtonWidth, ButtonHeight>;
	using Button8 = NumberButton<8, 108, StartX + ButtonWidth * 1, StartY + ButtonHeight * 3, ButtonWidth, ButtonHeight>;
	using Button9 = NumberButton<9, 109, StartX + ButtonWidth * 2, StartY + ButtonHeight * 3, ButtonWidth, ButtonHeight>;
	using ButtonTimes = OperationButton<L"x", 113, StartX + ButtonWidth * 3, StartY + ButtonHeight * 3, ButtonWidth, ButtonHeight>;
	// 5th row
	using Button0 = NumberButton<0, 100, StartX + ButtonWidth * 0, StartY + ButtonHeight * 4, ButtonWidth, ButtonHeight>;
	using ButtonDecimal = OperationButton<L".", 114, StartX + ButtonWidth * 1, StartY + ButtonHeight * 4, ButtonWidth, ButtonHeight>;
	using ButtonClear = OperationButton<L"CE", 115, StartX + ButtonWidth * 2, StartY + ButtonHeight * 4, ButtonWidth, ButtonHeight>;
	using ButtonDivide = OperationButton<L"%", 116, StartX + ButtonWidth * 3, StartY + ButtonHeight * 4, ButtonWidth, ButtonHeight>;
	// 6th row
	using ButtonEquals = OperationButton<L"=", 117, StartX + ButtonWidth * 0, StartY + ButtonHeight * 5, RowWidth, ButtonHeight>;

	template<typename...Ts>
	struct Overload : Ts...
	{
		using Ts::operator()...;
	};

	struct ButtonGroup
	{
		using TupleType = std::tuple<
			OutputWindow,
			Button1,
			Button2,
			Button3,
			ButtonPlus,
			Button4,
			Button5,
			Button6,
			ButtonMinus,
			Button7,
			Button8,
			Button9,
			ButtonTimes,
			Button0,
			ButtonDecimal,
			ButtonClear,
			ButtonDivide,
			ButtonEquals
		>;

		auto Get(this auto& self, unsigned id, auto&&...invocable) -> auto
		{
			[id]<typename...TArgs>(std::tuple<TArgs...>& tuple, auto&&...invocable)
			{
				((std::get<TArgs>(tuple).GetId() == id ? (Overload{ invocable... }(std::get<TArgs>(tuple)), true) : false) or ...);
			}(self.Buttons, std::forward<decltype(invocable)>(invocable)...);
		}

		auto RunOn(this auto& self, auto&&...invocable)
		{
			[]<typename...TArgs>(std::tuple<TArgs...>&tuple, auto&& overload)
			{
				([&overload, &tuple]<typename T = TArgs>
				{
					if constexpr (std::invocable<decltype(overload), T&>)
						std::invoke(overload, std::get<T>(tuple));
					//if constexpr (requires { std::invoke(overload, std::get<T>(tuple)); })
						//std::invoke(overload, std::get<T>(tuple));
				}(), ...);
			}(self.Buttons, Overload{ std::forward<decltype(invocable)>(invocable)... });
		}

		TupleType Buttons;
	};

	struct MainWindow : Window
	{
		using Window::Process;

		auto Process(this auto& self, Win32Message<Win32::Messages::Paint> message) -> Win32::LRESULT
		{
			return Win32::DefWindowProcW(message.Hwnd, message.uMsg, message.wParam, message.lParam);
		}

		auto Process(this auto& self, Win32Message<Win32::Messages::Command> message) -> Win32::LRESULT
		{
			self.m_buttons.Get(
				Win32::LoWord(message.wParam),
				[&self](Button0& control) { Log::Info("Zero was pressed!"); },
				[](auto& control) { }
			);

			self.m_buttons.RunOn([](Button0&) {Log::Info("Got it"); });

			Log::Info("{}", Win32::LoWord(message.wParam));
			return Win32::DefWindowProcW(message.Hwnd, message.uMsg, message.wParam, message.lParam);
		}

		auto Process(this auto& self, Win32Message<Win32::Messages::KeyUp> message) -> Win32::LRESULT
		{
			Win32::SendMessageW(std::get<1>(self.m_buttons.Buttons).GetHandle(), Win32::Messages::ButtonClick, 0, 0);
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
			}(self.m_window.get(), self.m_buttons.Buttons);
		}

	protected:
		ButtonGroup m_buttons{};
	};
}
