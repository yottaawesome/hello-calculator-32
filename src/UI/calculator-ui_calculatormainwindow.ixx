export module calculator:ui_calculatormainwindow;
import std;
import :win32;
import :string;
import :misc;
import :math;
import :ui_common;
import :ui_toplevelwindow;
import :ui_calculatorcontrols;
import :ui_control;
import :ui_font;

export namespace UI
{
	struct CalculatorMainWindow : TopLevelWindow
	{
		using TopLevelWindow::OnMessage;

		auto OnMessage(this auto&& self, Win32Message<Win32::Messages::Paint> message) -> Win32::LRESULT
		{
			return Win32::DefWindowProcW(message.Hwnd, message.uMsg, message.wParam, message.lParam);
		}

		auto OnMessage(this auto&& self, Win32Message<Win32::Messages::KeyUp> message) -> Win32::LRESULT
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

		auto OnMessage(this auto&& self, Win32Message<Win32::Messages::Command> message) -> Win32::LRESULT
		{
			self.m_buttons.Find(
				[id = Win32::LoWord(message.wParam)](AnyButton auto&& control) { return control.GetId() == id; },
				[&self](AnyButton auto&& btn) { self.Clicked(btn); }
			);
			// Focus needs to be set back to the window, because it goes to the button
			self.TakeFocus();
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

		auto ClassName(this const auto&) noexcept -> std::wstring_view { return L"Calculator-Gui"; }

		auto GetCreationArgs(this const auto&) -> CreateWindowArgs
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
