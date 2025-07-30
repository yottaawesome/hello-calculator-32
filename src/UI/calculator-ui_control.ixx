export module calculator:ui_control;
import std;
import :win32;
import :ui_common;
import :error;
import :string;
import :log;

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

	struct TextableCapability
	{
		auto GetText(this auto& self) -> std::wstring
		{
			auto handle = self.GetHandle();
			if (not handle)
				return {};
			std::wstring buffer{ Win32::GetWindowTextLengthW(handle), '\0' };
			Win32::GetWindowTextW(self.GetHandle(), buffer.data(), static_cast<int>(buffer.size()));
		}
		auto SetText(this auto& self, std::wstring_view text)
		{
			if (auto handle = self.GetHandle())
				Win32::SetWindowTextW(handle, text.data());
		}
		auto AppendText(this auto& self, std::wstring_view text) 
		{
			self.SetText(std::format(L"{}{}"), self.GetText(), text);
		}
	};

	struct Control : SimpleWindow
	{
		Control() = default;
		Control(ControlProperties properties) : m_properties(properties) {}

		auto Create(this auto&& self, Win32::HWND parent) -> void
		{
			static_assert(
				requires { { self.GetClass().data() } -> std::same_as<const wchar_t*>; }, 
				"This type needs a GetClass() member that returns a wstring or wstring_view!");
			
			Win32::HWND window = Win32::CreateWindowExW(
				self.m_properties.ExtendedStyles,
				self.GetClass().data(),
				self.m_properties.Text.empty() ? nullptr : self.m_properties.Text.data(),
				self.m_properties.Styles,
				self.m_properties.X,
				self.m_properties.Y,
				self.m_properties.Width,
				self.m_properties.Height,
				parent,
				(Win32::HMENU)(Win32::UINT_PTR)(self.m_properties.Id),
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
			Win32::UINT msg,
			Win32::WPARAM wParam,
			Win32::LPARAM lParam,
			Win32::UINT_PTR uIdSubclass,
			Win32::DWORD_PTR dwRefData
		) -> Win32::LRESULT
		{
			//Log::Info("Control {:X} {:X}", msg, wParam);
			if (msg == Win32::Messages::LeftButtonUp)
				return self.Process(Win32Message<Win32::Messages::LeftButtonUp>{ hwnd, wParam, lParam });
			if (msg == Win32::Messages::ButtonClick)
				return self.Process(Win32Message<Win32::Messages::ButtonClick>{ hwnd, wParam, lParam });
			return self.Process(GenericWin32Message{ .Hwnd = hwnd, .uMsg = msg, .wParam = wParam, .lParam = lParam });
		}

		auto Process(this Control& self, auto&& msg) -> Win32::LRESULT
		{
			return Win32::DefSubclassProc(msg.Hwnd, msg.uMsg, msg.wParam, msg.lParam);
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

		auto GetId(this const auto& self) noexcept -> unsigned { return m_properties.Id; }

	protected:
		ControlProperties m_properties;
	};

	struct Button : Control
	{
		using Control::Process;

		Button() : Control(GetDefaultProperties()) {}

		Button(ControlProperties properties)
			: Control(properties)
		{ }

		void Init(this auto&& self)
		{
			//Win32::SetWindowRgn(self.m_window.get(), Win32::CreateRoundRectRgn(10, 10, 60, 110, 50, 50), true);
		}

		auto GetClass(this auto&&) noexcept -> std::wstring_view
		{
			return L"Button";
		}

		auto GetDefaultProperties(this auto&& self) -> ControlProperties
		{
			return {
				.Id = 100,
				.Class = L"Button",
				.Text = L"Hello",
				.Styles = Win32::Styles::PushButton | Win32::Styles::Child | Win32::Styles::Visible,
				.ExtendedStyles = Win32::WindowStyles::WindowEdge,
				.X = 10,
				.Y = 10,
				.Width = 100,
				.Height = 50
			};
		};
	};

	template<unsigned VValue, unsigned VId, int VX, int VY, int VWidth, int VHeight>
	struct NumberButton : Button, TextableCapability
	{
		using Control::Process;

		NumberButton() : Button(GetDefaultProperties()) {}

		auto GetSubclassId(this auto&) noexcept { return VId; }

		auto Process(this auto&& self, Win32Message<Win32::Messages::LeftButtonUp> msg) -> Win32::LRESULT
		{
			Log::Info("Pressed: {}.", VValue);
			return Win32::DefSubclassProc(msg.Hwnd, msg.uMsg, msg.wParam, msg.lParam);
		}

		auto Process(this auto&& self, Win32Message<Win32::Messages::ButtonClick> msg) -> Win32::LRESULT
		{
			Log::Info("Pressed: {}.", VValue);
			return Win32::DefSubclassProc(msg.Hwnd, msg.uMsg, msg.wParam, msg.lParam);
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

	template<String::FixedString VText, unsigned VId, int VX, int VY, int VWidth, int VHeight>
	struct OperationButton : Button
	{
		using Control::Process;

		OperationButton() : Button(GetDefaultProperties()) {}
		
		auto GetSubclassId(this auto&) noexcept { return VId; }

		auto GetDefaultProperties(this auto&& self) -> ControlProperties
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
	};

	template<unsigned VId, int VX, int VY, int VWidth, int VHeight>
	struct Output : Control, TextableCapability
	{
		using Control::Process;

		Output() : Control(GetDefaultProperties()) {}

		auto GetSubclassId(this auto&) noexcept { return VId; }

		auto GetDefaultProperties(this auto&& self) -> ControlProperties
		{
			return {
				.Id = VId,
				.Text = L"Hello",
				.Styles = Win32::Styles::Child | Win32::Styles::Visible | Win32::Styles::Border,
				.X = VX,
				.Y = VY,
				.Width = VWidth,
				.Height = VHeight
			};
		};

		auto GetClass(this auto&&) noexcept -> std::wstring_view
		{
			return L"Static";
		}
	};
}