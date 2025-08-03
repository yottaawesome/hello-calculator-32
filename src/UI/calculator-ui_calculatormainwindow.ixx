export module calculator:ui_calculatormainwindow;
import std;
import :win32;
import :string;
import :ui_common;
import :ui_toplevelwindow;
import :ui_control;
import :ui_font;
import :misc;

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

	using ButtonGroup = Misc::TypeSequence<
		Button0, Button1, Button2, Button3, Button4, Button5, Button6, Button7, Button8, Button9,
		OutputWindow, ButtonPlus, ButtonMinus, ButtonTimes, ButtonDecimal, ButtonClear, ButtonDivide, ButtonEquals>;

	template<typename T>
	concept NumberInput = requires(T t, const T c)
	{
		{ c.Value() } noexcept -> std::convertible_to<unsigned>;
		{ c.ValueString() } noexcept -> std::convertible_to<std::wstring>;
	};
	// This is just a sanity check.
	static_assert(NumberInput<Button0>, "Expected Button0 to conform to NumberInput.");
}

export namespace UI
{
	struct Calculator
	{
		using OperatorTypes = std::variant<std::plus<double>, std::minus<double>, std::multiplies<double>, std::divides<double>>;
		std::optional<double> Left = 0;
		std::optional<double> Right = 0;
		OperatorTypes Operator;
		
		constexpr auto InsertOperand(this auto&& self, double value)
		{
			if (not self.Left)
				self.Left = value;
			if (not self.Right)
				self.Right = value;
		}
		
		constexpr auto BothSpecified(this auto&& self) noexcept -> bool { return self.Left and self.Right; }

		constexpr auto Result(this auto&& self, auto mathOperator) noexcept -> double
		{
			if (not self.BothSpecified) 
				return;
			double result = mathOperator(Left, Right);
			self.Left = result;
			self.Right = std::nullopt;
			return result;
		}
	};

	struct CalculatorMainWindow : TopLevelWindow
	{
		using TopLevelWindow::Process;

		auto Process(this auto&& self, Win32Message<Win32::Messages::Paint> message) -> Win32::LRESULT
		{
			return Win32::DefWindowProcW(message.Hwnd, message.uMsg, message.wParam, message.lParam);
		}

		auto Process(this auto&& self, Win32Message<Win32::Messages::Command> message) -> Win32::LRESULT
		{
			self.m_buttons.Find(
				[id = Win32::LoWord(message.wParam)](auto&& control) { return control.GetId() == id; },
				[&self](NumberInput auto& btn) 
				{ 
					self.m_buttons.GetByType<OutputWindow>().AppendText(btn.ValueString());
				},
				[](ButtonPlus& btn) { Log::Info("{} was pressed!", btn.Operator()); },
				[](ButtonMinus& btn) { Log::Info("{} was pressed!", btn.Operator()); },
				[](ButtonTimes& btn) { Log::Info("{} was pressed!", btn.Operator()); },
				[](ButtonDivide& btn) { Log::Info("{} was pressed!", btn.Operator()); },
				[](ButtonEquals& btn) { Log::Info("{} was pressed!", btn.Operator()); },
				[](ButtonDecimal& btn) { Log::Info("{} was pressed!", btn.Operator()); },
				[&self](ButtonClear& btn) 
				{
					self.m_buttons.GetByType<OutputWindow>().ClearText();
				},
				[](auto& control) { Log::Info("Other"); }
			);
			return Win32::DefWindowProcW(message.Hwnd, message.uMsg, message.wParam, message.lParam);
		}

		auto Process(this auto&& self, Win32Message<Win32::Messages::KeyUp> message) -> Win32::LRESULT
		{
			Win32::SendMessageW(self.m_buttons.GetByType<Button0>().GetHandle(), Win32::Messages::ButtonClick, 0, 0);
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

		auto Init(this auto&& self)
		{
			self.SetFont(UI::SystemFont);
			// Create child windows and set their font.
			self.m_buttons.RunAll(
				[&self](auto&& control)
				{
					control.Create(self.m_window.get());
					control.SetFont(UI::SystemFont);
				}
			);
		}

	protected:
		ButtonGroup m_buttons{};
	};
}
