export module calculator:ui_control;
import std;
import :win32;
import :ui_common;
import :error;
import :string;
import :log;
import :ui_capability;

template<typename T>
concept Handles2 = requires(T t) { t.Poo(); };

// Controls
export namespace UI
{
	

	struct ControlProperties
	{
		Win32::DWORD Id;
		std::wstring Class;
		std::wstring Text;
		Win32::DWORD Styles = 0;
		Win32::DWORD ExtendedStyles = 0;
		int X = 0;
		int Y = 0;
		int Width = 0;
		int Height = 0;
	};

	constexpr std::array HandledControlMessages{
		Win32::Messages::LeftButtonUp,
		Win32::Messages::Paint,
		Win32::Messages::Notify,
		Win32::Messages::DrawItem,
		Win32::Messages::MouseHover,
		Win32::Messages::MouseLeave,
		Win32::Messages::MouseMove,
		Win32::Messages::EraseBackground,
		Win32::Messages::Create
	};

	struct Control : Window
	{
		constexpr Control() = default;

		auto Create(this auto&& self, Win32::HWND parent) -> void
		{
			static_assert(
				requires { { self.ClassName.data() } -> std::same_as<const wchar_t*>; }, 
				"This type needs a ClassName member that returns a wstring or wstring_view!");
			
			auto properties = self.GetDefaultProperties();
			Win32::HWND window = Win32::CreateWindowExW(
				properties.ExtendedStyles,
				self.ClassName.data(),
				properties.Text.empty() ? nullptr : properties.Text.data(),
				properties.Styles,
				properties.X,
				properties.Y,
				properties.Width,
				properties.Height,
				parent,
				(Win32::HMENU)(Win32::UINT_PTR)(properties.Id),
				Win32::GetModuleHandleW(nullptr),
				nullptr
			);
			if (self.m_window = Raii::HwndUniquePtr(window); not self.m_window)
				throw Error::Win32Error(Win32::GetLastError(), "Failed creating button.");
			if (not Win32::SetWindowSubclass(self.m_window.get(), SubclassProc<std::remove_cvref_t<decltype(self)>>, self.GetSubclassId(), reinterpret_cast<Win32::DWORD_PTR>(&self)))
				throw Error::Win32Error(Win32::GetLastError(), "Failed creating button.");

			if constexpr (requires { self.Init(); })
				self.Init();
		}

		auto HandleMessage(
			this auto&& self,
			Win32::HWND hwnd,
			Win32::UINT msgType,
			Win32::WPARAM wParam,
			Win32::LPARAM lParam,
			Win32::UINT_PTR uIdSubclass,
			Win32::DWORD_PTR dwRefData
		) -> Win32::LRESULT
		{
			return[&self, hwnd, msgType, wParam, lParam]<size_t...Is>(std::index_sequence<Is...>)
			{
				Win32::LRESULT result;
				// MSVC BUG NOTE
				// CAPTURING SELF IN THE CAPTURE CLAUSE CAUSES THE CONCEPT EVALUATION TO ALWAYS BE TRUE,
				// NO MATTER WHAT THE CONCEPT ACTUALLY IS, THUS CAUSING A COMPILE ERROR WHERE IT SHOULD 
				// BE FINE. IT'S NOT CLEAR WHY, BECAUSE SIMILAR CODE WORKS FINE IN THE  WIN32-EXPERIMENTS
				// REPO. THIS IS IS BECAUSE MICROSOFT IS RETARDED AND CAN'T FIX THEIR FUCKING COMPILER.
				// THIS IS A REMINDER TO MAKE A MINIMAL REPRODUCIBLE SAMPLE OF THIS BUG AND REPORT IT TO
				// THE TWATS AT MICROSOFT, SO THEY CAN IGNORE IT ALONG THE OTHER CONSTEXPR BUGS I'VE
				// REPORTED.
				// END MSVC BUG NOTE
				bool handled = (... or
					[&result, msgType]<Win32::DWORD VRawMsgType>(auto&& self, Win32Message<VRawMsgType> msg)
					{
						//if constexpr (requires { self.On(winMsg); }) // doesn't work
						//if constexpr (requires { self.OnMessage(Win32Message<VRawMsgType>{}); })
						if constexpr (Handles<decltype(self), std::remove_cvref_t<decltype(msg)>>)
						{
							return msg == msgType ? (result = self.OnMessage(msg), true) : false;
						}
						else
						{
							return false;
						}
					}(self, Win32Message<std::get<Is>(HandledControlMessages)>{ hwnd, wParam, lParam }));
				return handled ? result : Win32::DefSubclassProc(hwnd, msgType, wParam, lParam);
			}(std::make_index_sequence<HandledControlMessages.size()>());
		}

		template<typename TControl>
		static auto SubclassProc(
			Win32::HWND hwnd,
			Win32::UINT msg,
			Win32::WPARAM wParam,
			Win32::LPARAM lParam,
			Win32::UINT_PTR idSubclass,
			Win32::DWORD_PTR refData
		) -> Win32::LRESULT
		{
			TControl* pThis = reinterpret_cast<TControl*>(refData);
			return pThis
				? pThis->HandleMessage(hwnd, msg, wParam, lParam, idSubclass, refData)
				: Win32::DefSubclassProc(hwnd, msg, wParam, lParam);
		}

		auto GetId(this auto&& self) noexcept -> unsigned { return self.GetDefaultProperties().Id; }
	};

	template<unsigned VId, int VX, int VY, int VWidth, int VHeight>
	struct Output : Control, Textable
	{
		auto GetSubclassId(this auto&&) noexcept -> unsigned 
		{ 
			return VId; 
		}

		auto GetDefaultProperties(this auto&& self) -> ControlProperties
		{
			return {
				.Id = VId,
				.Text = L"", // initial window text
				.Styles = Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::Border | Win32::Styles::Static::Right,
				.X = VX,
				.Y = VY,
				.Width = VWidth,
				.Height = VHeight
			};
		};

		constexpr static std::wstring_view ClassName = L"Static";
	};

	struct Button : Control
	{
		constexpr static std::wstring_view ClassName = L"Button";

		auto OnMessage(this auto&& self, Win32Message<Win32::Messages::LeftButtonUp> msg) -> Win32::LRESULT
		{
			if constexpr (requires { self.OnClick(); })
				self.OnClick();
			return Win32::DefSubclassProc(msg.Hwnd, msg.uMsg, msg.wParam, msg.lParam);
		}

		auto GetDefaultProperties(this auto&& self) -> ControlProperties
		{
			return {
				.Id = 100,
				.Class = L"Button",
				.Text = L"", // window text
				.Styles = Win32::Styles::PushButton | Win32::Styles::Child | Win32::Styles::Visible,
				.ExtendedStyles = Win32::WindowStyles::WindowEdge,
				.X = 10,
				.Y = 10,
				.Width = 100,
				.Height = 50
			};
		};

		void Click(this auto&& self) noexcept
		{
			if (self.GetHandle())
				Win32::SendMessageW(self.GetHandle(), Win32::Messages::ButtonClick, 0, 0);
		}
	};

	template<unsigned VValue, unsigned VId, int VX, int VY, int VWidth, int VHeight, unsigned VKeyCode>
	struct NumberButton : Button, Textable, KeyBindable<VKeyCode>
	{
		auto GetSubclassId(this auto&) noexcept { return VId; }

		constexpr auto Value(this const auto&) noexcept -> unsigned
		{
			return VValue;
		}

		constexpr auto ValueString(this const auto&) noexcept -> std::wstring
		{
			return std::to_wstring(VValue);
		}

		auto GetDefaultProperties(this auto&& self) -> ControlProperties
		{
			return {
				.Id = VId,
				.Class = L"Button",
				.Text = std::to_wstring(VValue),
				.Styles = Win32::Styles::DefPushButton | Win32::Styles::Child | Win32::Styles::Visible,
				.X = VX,
				.Y = VY,
				.Width = VWidth,
				.Height = VHeight
			};
		};
	};

	template<String::FixedString VText, unsigned VId, int VX, int VY, int VWidth, int VHeight, unsigned VKeyCode>
	struct OperationButton : Button, KeyBindable<VKeyCode>
	{
		auto GetSubclassId(this auto&&) noexcept { return VId; }

		auto GetDefaultProperties(this const auto&) -> ControlProperties
		{
			return {
				.Id = VId,
				.Class = L"Button",
				.Text = VText.Buffer,
				.Styles = Win32::Styles::DefPushButton | Win32::Styles::Child | Win32::Styles::Visible,
				.X = VX,
				.Y = VY,
				.Width = VWidth,
				.Height = VHeight
			};
		};

		constexpr auto Operator(this const auto&) noexcept -> std::wstring_view
		{ 
			return VText.ToView(); 
		}
	};
}
