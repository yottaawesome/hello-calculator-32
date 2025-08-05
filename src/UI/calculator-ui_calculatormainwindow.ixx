export module calculator:ui_calculatormainwindow;
import std;
import :win32;
import :string;
import :ui_common;
import :ui_toplevelwindow;
import :ui_control;
import :ui_font;
import :misc;
import :math;

export namespace UI
{
	constexpr auto ButtonWidth = 100;
	constexpr auto ButtonHeight = 50;
	constexpr auto StartX = 15;
	constexpr auto StartY = 15;
	constexpr auto RowWidth = ButtonWidth * 4;

	// 1st row
	using OutputWindow = Output<1, StartX + ButtonWidth * 0, StartY + ButtonHeight * 0, RowWidth, ButtonHeight>;
	// 2nd row
	using Button1 = 
		NumberButton<1, 101, StartX + ButtonWidth * 0, StartY + ButtonHeight * 1, ButtonWidth, ButtonHeight, Win32::VirtualKeyCodes::Numpad1>;
	using Button2 = 
		NumberButton<2, 102, StartX + ButtonWidth * 1, StartY + ButtonHeight * 1, ButtonWidth, ButtonHeight, Win32::VirtualKeyCodes::Numpad2>;
	using Button3 = 
		NumberButton<3, 103, StartX + ButtonWidth * 2, StartY + ButtonHeight * 1, ButtonWidth, ButtonHeight, Win32::VirtualKeyCodes::Numpad3>;
	using ButtonPlus = 
		OperationButton<L"+", 111, StartX + ButtonWidth * 3, StartY + ButtonHeight * 1, ButtonWidth, ButtonHeight, Win32::VirtualKeyCodes::Add>;
	// 3rd row
	using Button4 = 
		NumberButton<4, 104, StartX + ButtonWidth * 0, StartY + ButtonHeight * 2, ButtonWidth, ButtonHeight, Win32::VirtualKeyCodes::Numpad4>;
	using Button5 = 
		NumberButton<5, 105, StartX + ButtonWidth * 1, StartY + ButtonHeight * 2, ButtonWidth, ButtonHeight, Win32::VirtualKeyCodes::Numpad5>;
	using Button6 = 
		NumberButton<6, 106, StartX + ButtonWidth * 2, StartY + ButtonHeight * 2, ButtonWidth, ButtonHeight, Win32::VirtualKeyCodes::Numpad6>;
	using ButtonMinus = 
		OperationButton<L"-", 112, StartX + ButtonWidth * 3, StartY + ButtonHeight * 2, ButtonWidth, ButtonHeight, Win32::VirtualKeyCodes::Subtract>;
	// 4th row
	using Button7 = 
		NumberButton<7, 107, StartX + ButtonWidth * 0, StartY + ButtonHeight * 3, ButtonWidth, ButtonHeight, Win32::VirtualKeyCodes::Numpad7>;
	using Button8 = 
		NumberButton<8, 108, StartX + ButtonWidth * 1, StartY + ButtonHeight * 3, ButtonWidth, ButtonHeight, Win32::VirtualKeyCodes::Numpad8>;
	using Button9 = 
		NumberButton<9, 109, StartX + ButtonWidth * 2, StartY + ButtonHeight * 3, ButtonWidth, ButtonHeight, Win32::VirtualKeyCodes::Numpad9>;
	using ButtonTimes = 
		OperationButton<L"x", 113, StartX + ButtonWidth * 3, StartY + ButtonHeight * 3, ButtonWidth, ButtonHeight, Win32::VirtualKeyCodes::Multiply>;
	// 5th row
	using Button0 = 
		NumberButton<0, 100, StartX + ButtonWidth * 0, StartY + ButtonHeight * 4, ButtonWidth, ButtonHeight, Win32::VirtualKeyCodes::Numpad0>;
	using ButtonDecimal = 
		OperationButton<L".", 114, StartX + ButtonWidth * 1, StartY + ButtonHeight * 4, ButtonWidth, ButtonHeight, Win32::VirtualKeyCodes::Decimal>;
	using ButtonClear = 
		OperationButton<L"CE", 115, StartX + ButtonWidth * 2, StartY + ButtonHeight * 4, ButtonWidth, ButtonHeight, Win32::VirtualKeyCodes::Delete>;
	using ButtonDivide = 
		OperationButton<L"%", 116, StartX + ButtonWidth * 3, StartY + ButtonHeight * 4, ButtonWidth, ButtonHeight, Win32::VirtualKeyCodes::Divide>;
	// 6th row
	using ButtonEquals = 
		OperationButton<L"=", 117, StartX + ButtonWidth * 0, StartY + ButtonHeight * 5, RowWidth, ButtonHeight, Win32::VirtualKeyCodes::NumpadEnter>;

	using ButtonGroup = Misc::TypeSequence<
		Button0, Button1, Button2, Button3, Button4, Button5, Button6, Button7, Button8, Button9,
		OutputWindow, ButtonPlus, ButtonMinus, ButtonTimes, ButtonDecimal, ButtonClear, ButtonDivide, ButtonEquals>;

	template<typename T>
	concept NumberInput = requires(T t, const T c)
	{
		{ c.Value() } noexcept -> std::convertible_to<unsigned>;
		{ c.ValueString() } noexcept -> std::convertible_to<std::wstring>;
	};

	template<typename T>
	concept OperatorInput = Misc::OneOf<T, ButtonPlus, ButtonMinus, ButtonTimes, ButtonDivide>;
	
		// This is just a sanity check.
	static_assert(NumberInput<Button0>, "Expected Button0 to conform to NumberInput.");

	template<typename T>
	concept KeyCodeButton = requires(T t)
	{
		requires std::derived_from<T, Button>;
		{ t.KeyCode() } -> std::convertible_to<unsigned>;
	};
}

export namespace UI
{
	struct CalculatorMainWindow : TopLevelWindow
	{
		using TopLevelWindow::Process;

		auto Process(this auto&& self, Win32Message<Win32::Messages::Paint> message) -> Win32::LRESULT
		{
			return Win32::DefWindowProcW(message.Hwnd, message.uMsg, message.wParam, message.lParam);
		}

		auto Clicked(this auto&& self, NumberInput auto& btn)
		{
			self.m_buttons.GetByType<OutputWindow>().AppendText(btn.ValueString());
		}

		auto Clicked(this auto&& self, OperatorInput auto& btn)
		{
			Log::Info("{} was pressed!", btn.Operator());
			auto& textWindow = self.m_buttons.GetByType<OutputWindow>();
			try
			{
				Misc::Overload{
					[&self](ButtonPlus&) {self.m_calculator.Operator = std::plus<double>{}; },
					[&self](ButtonMinus&) {self.m_calculator.Operator = std::minus<double>{}; },
					[&self](ButtonTimes&) {self.m_calculator.Operator = std::multiplies<double>{}; },
					[&self](ButtonDivide&) {self.m_calculator.Operator = std::divides<double>{}; }
				}(btn);

				auto text = textWindow.GetText();
				if (text.empty())
					return;

				double value = std::stod(text);
				if (std::isnan(value))
					throw std::out_of_range("NaN");
				self.m_calculator.Left = value;
				textWindow.ClearText();
			}
			catch (const std::exception& ex)
			{
				Log::Error("Error: {}", ex.what());
				textWindow.ClearText();
				self.m_calculator.Clear();
			}
		}

		auto Clicked(this auto&& self, ButtonEquals& btn)
		{
			Log::Info("{} was pressed!", btn.Operator());
			auto& textWindow = self.m_buttons.GetByType<OutputWindow>();
			try
			{
				auto text = textWindow.GetText();
				if (text.empty())
					return;
				double value = std::stod(text);
				if (std::isnan(value))
					throw std::out_of_range("NaN");
				self.m_calculator.InsertOperand(value);
				self.m_calculator.Calculate();
				textWindow.SetText(self.m_calculator.GetCalculationAsString());
				self.m_calculator.Clear();
			}
			catch (const std::exception& ex)
			{
				Log::Error("Error: {}", ex.what());
				textWindow.ClearText();
				self.m_calculator.Clear();
			}
		}

		auto Clicked(this auto&& self, ButtonDecimal& btn)
		{
			Log::Info("{} was pressed!", btn.Operator());

			auto& output = self.m_buttons.GetByType<OutputWindow>();
			std::wstring out = output.GetText();
			if (not out.contains(L"."))
				output.AppendText(L".");
		}

		auto Clicked(this auto&& self, ButtonClear& btn)
		{
			self.m_buttons.GetByType<OutputWindow>().ClearText();
			self.m_calculator.Clear();
		}

		auto Process(this auto&& self, Win32Message<Win32::Messages::Command> message) -> Win32::LRESULT
		{
			self.m_buttons.Find(
				[id = Win32::LoWord(message.wParam)](auto&& control) { return control.GetId() == id; },
				[&self](NumberInput auto& btn) { self.Clicked(btn); },
				[&self](OperatorInput auto& btn) { self.Clicked(btn); },
				[&self](ButtonEquals& btn) { self.Clicked(btn); },
				[&self](ButtonDecimal& btn) { self.Clicked(btn); },
				[&self](ButtonClear& btn) { self.Clicked(btn); },
				[](auto& control) { Log::Info("Other"); }
			);
			// Focus needs to be set back to the window, because it goes to the button
			self.TakeFocus();
			return Win32::DefWindowProcW(message.Hwnd, message.uMsg, message.wParam, message.lParam);
		}

		auto Process(this auto&& self, Win32Message<Win32::Messages::KeyUp> message) -> Win32::LRESULT
		{
			self.m_buttons.Find(
				[keyCode = message.wParam](const KeyCodeButton auto& btn) -> bool
				{
					return keyCode == btn.KeyCode(); 
				},
				[](const KeyCodeButton auto& btn)
				{
					Log::Info("Keystroke virtual key code: {}", btn.KeyCode());
					btn.Click();
				}
			);
			// Focus needs to be set back to the window, because it goes to the button
			self.TakeFocus();
			return Win32::DefWindowProcW(message.Hwnd, message.uMsg, message.wParam, message.lParam);
		}

		auto ClassName(this const auto&) noexcept -> std::wstring_view { return L"Calculator-Gui"; }

		auto CreateArgs(this const auto&) -> CreateWindowArgs
		{
			return {
				.WindowName = L"Win32 Calculator",
				// disable resize
				.Style = Win32::WindowStyles::WsOverlappedWindow & ~Win32::WindowStyles::ThickFrame,
				.Width = 450,
				.Height = 370
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

		auto Init(this auto&& self)
		{
			self.SetFont(UI::SystemFont);
			// Create child windows and set their font.
			self.m_buttons.RunAll(
				[&self](OutputWindow& control)
				{
					control.Create(self.m_window.get());
					control.SetFont(UI::SegoeFont);
				},
				[&self](auto& control)
				{
					control.Create(self.m_window.get());
					control.SetFont(UI::SystemFont);
				}
			);
		}

	protected:
		ButtonGroup m_buttons{};
		Math::Calculator m_calculator;
	};
}
