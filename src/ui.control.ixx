export module calculator:ui.control;
import std;
import :win32;
import :ui.common;
import :error;
import :string;
import :log;
import :ui.capability;

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
		Win32::Messages::Command,
		Win32::Messages::Notify,
		Win32::Messages::DrawItem,
		Win32::Messages::MouseHover,
		Win32::Messages::MouseLeave,
		Win32::Messages::MouseMove,
		Win32::Messages::EraseBackground,
		Win32::Messages::Destroy,
		Win32::Messages::NonClientDestroy
	};

	struct Control : Window
	{
		constexpr Control() = default;

		~Control()
		{
			// Do not dispatch to a derived type once its destructor has finished.
			if (m_window and m_subclassProc)
				Win32::RemoveWindowSubclass(m_window.get(), m_subclassProc, m_subclassId);
		}

		// Call through the concrete type, never a base reference: self determines callback dispatch.
		auto Create(this auto& self, Win32::HWND parent) -> void
		{
			static_assert(
				requires { { self.ClassName.data() } -> std::same_as<const wchar_t*>; }, 
				"This type needs a ClassName member that returns a wstring or wstring_view!");
			
			if (self.m_window)
				throw std::logic_error{"Control has already been created."};

			auto properties = self.GetDefaultProperties();
			auto subclassId = self.GetSubclassId();
			// WM_CREATE is sent inside CreateWindowExW, before this subclass is installed.
			// Use Init() for post-creation initialization; OnMessage(WM_CREATE) cannot intercept it.
			auto window = Win32::CreateWindowExW(
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
			if (not window)
				throw Error::Win32Error{Win32::GetLastError(), "CreateWindowExW failed for control."};

			if (self.m_window = Raii::HwndUniquePtr(window); not self.m_window)
				throw Error::Win32Error{Win32::GetLastError(), "Failed to adopt control HWND."};

			self.m_subclassProc = SubclassProc<std::remove_cvref_t<decltype(self)>>;
			self.m_subclassId = subclassId;
			if (not Win32::SetWindowSubclass(window, self.m_subclassProc, subclassId, reinterpret_cast<Win32::DWORD_PTR>(&self)))
			{
				auto error = Win32::GetLastError();
				self.Destroy();
				throw Error::Win32Error{error, "SetWindowSubclass failed for control."};
			}

			try
			{
				if constexpr (requires { self.Init(); })
					self.Init();
			}
			catch (...)
			{
				self.Destroy();
				throw;
			}
		}

		auto HandleMessage(
			this auto&& self,
			Win32::HWND hwnd,
			Win32::UINT msgType,
			Win32::WPARAM wParam,
			Win32::LPARAM lParam
		) -> MessageResult
		{
			return[&self, hwnd, msgType, wParam, lParam]<size_t...Is>(std::index_sequence<Is...>)
			{
				auto result = MessageResult{};
				auto handled = (... or
					[&result, msgType]<Win32::DWORD VRawMsgType>(auto&& self, Win32Message<VRawMsgType> msg)
					{
						// Pass self as an argument to work around MSVC's concept evaluation when captured.
						if constexpr (Handles<decltype(self), std::remove_cvref_t<decltype(msg)>>)
						{
							return msg == msgType ? (result = self.OnMessage(msg), true) : false;
						}
						else
						{
							return false;
						}
					}(self, Win32Message<std::get<Is>(HandledControlMessages)>{ hwnd, wParam, lParam }));

				return handled ? result : std::nullopt;
			}(std::make_index_sequence<HandledControlMessages.size()>());
		}

		template<typename TControl>
		static auto __stdcall SubclassProc(
			Win32::HWND hwnd,
			Win32::UINT msg,
			Win32::WPARAM wParam,
			Win32::LPARAM lParam,
			Win32::UINT_PTR idSubclass,
			Win32::DWORD_PTR refData
		) noexcept -> Win32::LRESULT
		{
			auto pThis = reinterpret_cast<TControl*>(refData);
			// Cleanup precedes user handlers, including handlers that throw.
			if (msg == Win32::Messages::NonClientDestroy)
			{
				Win32::RemoveWindowSubclass(hwnd, SubclassProc<TControl>, idSubclass);
				if (pThis and pThis->m_window.get() == hwnd)
				{
					pThis->m_window.release();
					pThis->m_subclassProc = nullptr;
				}
			}

			try
			{
				auto result = pThis ? pThis->HandleMessage(hwnd, msg, wParam, lParam) : MessageResult{};
				return result ? *result : Win32::DefSubclassProc(hwnd, msg, wParam, lParam);
			}
			catch (const std::exception& ex)
			{
				ReportCallbackException(ex.what());
			}
			catch (...)
			{
				ReportCallbackException("Unknown exception");
			}
			return 0;
		}

		auto GetId(this auto&& self) -> unsigned { return self.GetDefaultProperties().Id; }

		// Default subclass ID: reuse the control ID unless overridden by derived controls.
		auto GetSubclassId(this auto&& self) -> Win32::UINT_PTR { return self.GetId(); }

	private:
		Win32::SUBCLASSPROC m_subclassProc = nullptr;
		Win32::UINT_PTR m_subclassId = 0;
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

		constexpr static auto ClassName = std::wstring_view{L"Static"};
	};

	struct Button : Control
	{
		constexpr static auto ClassName = std::wstring_view{L"Button"};

		// Derived overloads must retain this one with "using Button::OnMessage".
		auto OnMessage(this auto&& self, Win32Message<Win32::Messages::Command> msg) -> MessageResult
		{
			if (NotificationSource(msg) == self.GetHandle() and Win32::HiWord(msg.wParam) == Win32::Messages::ButtonClicked)
			{
				if constexpr (requires { self.OnClick(); })
				{
					self.OnClick();
					return 0;
				}
			}
			return std::nullopt;
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
