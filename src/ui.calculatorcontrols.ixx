export module calculator:ui.calculatorcontrols;
import std;
import :win32;
import :ui.control;
import :misc;

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
		OperationButton<L"/", 116, StartX + ButtonWidth * 3, StartY + ButtonHeight * 4, ButtonWidth, ButtonHeight, Win32::VirtualKeyCodes::Divide>;
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

	template<typename T>
	concept KeyCodeButton = requires(T t)
	{
		requires std::derived_from<std::remove_cvref_t<T>, Button>;
		{ t.KeyCode() } -> std::convertible_to<unsigned>;
	};

	template<typename T>
	concept AnyButton = std::derived_from<std::remove_cvref_t<T>, Button>;

	// This is just a sanity check.
	static_assert(NumberInput<Button0>, "Expected Button0 to conform to NumberInput.");
}
