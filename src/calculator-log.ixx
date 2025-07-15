export module calculator:log;
import std;

export namespace Log
{
	template<typename...TArgs>
	void Info(std::format_string<TArgs...> fmt, TArgs&&...args) noexcept
	{
		std::format(fmt, std::forward<TArgs>(args)...);
	}

	template<typename...TArgs>
	void Warn(std::format_string<TArgs...> fmt, TArgs&&...args) noexcept
	{
		std::format(fmt, std::forward<TArgs>(args)...);
	}

	template<typename...TArgs>
	void Error(std::format_string<TArgs...> fmt, TArgs&&...args) noexcept
	{
		std::format(fmt, std::forward<TArgs>(args)...);
	}
}
