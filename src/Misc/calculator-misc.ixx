export module calculator:misc;
import std;

export namespace Misc
{
	constexpr bool IsDebug =
#ifdef _DEBUG
		true;
#else
		false;
#endif
	constexpr bool IsRelease = not IsDebug;

	template<typename...Ts>
	struct Overload : Ts...
	{
		using Ts::operator()...;
	};

	template<typename T, typename...TTypes>
	concept OneOf = (std::same_as<std::remove_cvref_t<T>, TTypes> or ...);

	template<typename...TTypes>
	struct TypeSequence
	{
		using TupleType = std::tuple<TTypes...>;
		using VariantType = std::variant<const TTypes*...>;

		constexpr void Find(this auto&& self, auto&& filterFn, auto&&...invocable)
		{
			static_assert((std::invocable<decltype(filterFn), TTypes> or ...), "The filter function must be invocable with at least one type in the sequence.");
			static_assert(sizeof...(invocable) > 0, "You must pass at least one invocable.");
			[]<typename...TArgs>(std::tuple<TArgs...>& tuple, auto&& filterFn, auto&&...invocable)
			{
				// Returns false if the filter function cannot be invoked on the 
				// tuple element type, or the overload does not accept the tuple
				// element type. This allows client code to specify filter and
				// and invocable functions with only what they care about.
				Overload overload{ std::forward<decltype(invocable)>(invocable)... };
				([&filterFn, &overload, &tuple]<typename TArg = TArgs>
				{
					if constexpr (not std::invocable<decltype(filterFn), TArg>) 
						return false;
					else if constexpr (not std::invocable<decltype(overload), decltype(std::get<TArg>(tuple))>)
						return false;
					else return std::invoke(filterFn, std::get<TArg>(tuple))
						? (std::invoke(overload, std::get<TArg>(tuple)), true)
						: false;
				}() or ...);
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
