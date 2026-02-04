export module calculator:math;
import std;

export namespace Math
{
	auto HasOnlyZeroesForDecimalPart(double num) -> bool
	{
		double integerPart;
		double fractionalPart = std::modf(num, &integerPart);
		// Due to floating-point precision, comparing directly to 0.0 might be unreliable.
		// Instead, compare the absolute value of the fractional part to a small epsilon.
		constexpr double Epsilon = 1e-9; // A small value to account for precision issues
		return std::abs(fractionalPart) < Epsilon;
	}

	struct Calculator
	{
		using OperatorTypes = std::variant<std::plus<double>, std::minus<double>, std::multiplies<double>, std::divides<double>>;
		std::optional<double> Left = std::nullopt;
		std::optional<double> Right = std::nullopt;
		std::optional<double> Calculation = std::nullopt;
		std::optional<OperatorTypes> Operator = std::nullopt;

		constexpr auto InsertOperand(this auto&& self, double value)
		{
			if (not self.Left)
				self.Left = value;
			else if (not self.Right)
				self.Right = value;
		}

		constexpr auto OperandsSpecified(this auto&& self) noexcept -> bool
		{
			return self.Left.has_value() and self.Right.has_value();
		}

		constexpr auto LeftAndOperator(this auto&& self) noexcept -> bool
		{
			return self.Left.has_value() and self.Operator;
		}

		constexpr auto AllSpecified(this auto&& self) noexcept -> bool
		{
			return self.OperandsSpecified() and self.Operator;
		}

		constexpr auto Clear(this auto&& self) noexcept
		{
			self.Left = std::nullopt;
			self.Right = std::nullopt;
			self.Operator = std::nullopt;
		}

		constexpr auto Calculate(this auto&& self) -> double
		{
			if (not self.AllSpecified())
				throw std::runtime_error("Both operands and an operator are not specified.");

			self.Calculation = std::visit(
				[left = *self.Left, right = *self.Right](const auto& mathOperator)
					{ return mathOperator(left, right); },
				*self.Operator
			);
			return *self.Calculation;
		}

		constexpr auto GetCalculationAsString(this auto&& self) -> std::wstring
		{
			if (not self.Calculation)
				return {};
			if (HasOnlyZeroesForDecimalPart(*self.Calculation))
				return std::to_wstring(static_cast<int>(*self.Calculation));
			return std::to_wstring(*self.Calculation);
		}
	};
}
