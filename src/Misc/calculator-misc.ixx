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

		constexpr void Get(this auto&& self, unsigned id, auto&&...invocable)
		{
			[id]<typename...TArgs>(std::tuple<TArgs...>&tuple, auto&&...invocable)
			{
				((std::get<TArgs>(tuple).GetId() == id ? (Overload{ invocable... }(std::get<TArgs>(tuple)), true) : false) or ...);
			}(self.AllTypes, std::forward<decltype(invocable)>(invocable)...);
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
			[] <typename...TArgs>(std::tuple<TArgs...>&tuple, auto&& invocable)
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
