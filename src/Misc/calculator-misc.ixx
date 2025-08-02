export module calculator:misc;
import std;

export namespace Misc
{
	template<typename...Ts>
	struct Overload : Ts...
	{
		using Ts::operator()...;
	};

	template<typename...TTypes>
	struct TypeSequence
	{
		using TupleType = std::tuple<TTypes...>;
		using VariantType = std::variant<const TTypes*...>;

		constexpr void Find(this auto&& self, auto&& filterFn, auto&&...invocable)
		{
			static_assert((std::invocable<decltype(filterFn), TTypes> and ...), "The filter function must be invocable with all types in the sequence.");
			[]<typename...TArgs>(std::tuple<TArgs...>& tuple, auto&& filterFn, auto&&...invocable)
			{
				Overload overload{ std::forward<decltype(invocable)>(invocable)... };
				((std::invoke(filterFn, std::get<TArgs>(tuple)) 
					? (std::invoke(overload, std::get<TArgs>(tuple)), true)
					: false
				) or ...);
			}(self.AllTypes, std::forward<decltype(filterFn)>(filterFn), std::forward<decltype(invocable)>(invocable)...);
		}

		constexpr void RunAll(this auto&& self, auto&&...invocable)
		{
			[]<typename...TArgs>(std::tuple<TArgs...> &tuple, auto&& overload)
			{
				([&overload, &tuple]<typename T = TArgs>
				{
					//if constexpr (requires { std::invoke(overload, std::get<T>(tuple)); })
					if constexpr (std::invocable<decltype(overload), T&>)
						std::invoke(overload, std::get<T>(tuple));
				}(), ...);
			}(self.AllTypes, Overload{ std::forward<decltype(invocable)>(invocable)... });
		}

		constexpr void RunOne(this auto&& self, auto&& invocable)
		{
			[]<typename...TArgs>(std::tuple<TArgs...>&tuple, auto&& invocable)
			{
				([&invocable, &tuple]<typename T = TArgs>
				{
					//if constexpr (requires { std::invoke(invocable, std::get<T>(tuple)); })
					if constexpr (std::invocable<decltype(invocable), T&>)
					{
						std::invoke(invocable, std::get<T>(tuple));
						return true;
					}
					return false;
				}() or ...);
			}(self.AllTypes, invocable);
		}

		template<typename T>
		constexpr auto GetByType(this auto&& self) noexcept -> decltype(auto) { return std::get<T>(self.AllTypes); }

		TupleType AllTypes;
	};
}
