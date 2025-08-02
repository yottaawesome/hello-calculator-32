export module calculator:raii;
import std;
import :win32;

export namespace Raii
{
	template<auto VDeleteFn>
	struct Deleter
	{
		static void operator()(auto resource) noexcept { VDeleteFn(resource); }
	};
	template<typename TResource, auto VDeleteFn>
	using UniquePtr = std::unique_ptr<TResource, Deleter<VDeleteFn>>;

	template<typename TResource, auto VDeleteFn>
	using DirectUniquePtr = std::unique_ptr<TResource, Deleter<VDeleteFn>>;
	template<typename TResource, auto VDeleteFn>
	using IndirectUniquePtr = std::unique_ptr<std::remove_pointer_t<TResource>, Deleter<VDeleteFn>>;

	using LocalHeapUniquePtr = IndirectUniquePtr<Win32::HLOCAL, Win32::LocalFree>;
	using HwndUniquePtr = IndirectUniquePtr<Win32::HWND, Win32::DestroyWindow>;
	using FontUniquePtr = IndirectUniquePtr<Win32::HFONT, Win32::DeleteObject>;
}
