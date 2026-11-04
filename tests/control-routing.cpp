#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <CommCtrl.h>

import std;
import calculator;

namespace
{
	void Check(bool condition, std::string_view message)
	{
		if (not condition)
			throw std::runtime_error{std::string{message}};
	}

	int NativeMouseMoves = 0;
	int NativeErases = 0;

	LRESULT CALLBACK NativeControlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (msg == WM_MOUSEMOVE)
		{
			++NativeMouseMoves;
			return 42;
		}
		if (msg == WM_ERASEBKGND)
		{
			++NativeErases;
			return 91;
		}
		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}

	struct ProbeControl : UI::Control
	{
		static constexpr auto ClassName = std::wstring_view{L"Calculator-RoutingTestControl"};
		unsigned SubclassId = 501;
		int IdQueries = 0;
		int Destroyed = 0;
		int ThrowOnMove = 0;
		bool ThrowOnDestroy = false;
		bool RejectInit = false;
		HWND NotificationParent = nullptr;

		~ProbeControl() { this->Destroy(); }

		auto GetDefaultProperties(this auto&&) -> UI::ControlProperties
		{
			return { .Id = 201, .Styles = WS_CHILD, .Width = 30, .Height = 30 };
		}

		auto GetSubclassId(this auto& self) -> unsigned
		{
			++self.IdQueries;
			return self.SubclassId;
		}

		void Init(this auto& self)
		{
			if (self.RejectInit)
				throw std::runtime_error{"Init failed"};
		}

		auto OnMessage(this auto& self, UI::Win32Message<WM_MOUSEMOVE>) -> UI::MessageResult
		{
			if (self.ThrowOnMove == 1)
				throw std::runtime_error{"Mouse handler failed"};
			if (self.ThrowOnMove == 2)
				throw 42;
			return std::nullopt;
		}

		auto OnMessage(this auto&, UI::Win32Message<WM_ERASEBKGND>) -> LRESULT { return 0; }

		auto OnMessage(this auto& self, UI::Win32Message<WM_NOTIFY> message) -> LRESULT
		{
			self.NotificationParent = message.Hwnd;
			return 73;
		}

		auto OnMessage(this auto& self, UI::Win32Message<WM_DRAWITEM> message) -> LRESULT
		{
			self.NotificationParent = message.Hwnd;
			return 1;
		}

		auto OnMessage(this auto& self, UI::Win32Message<WM_NCDESTROY>) -> UI::MessageResult
		{
			++self.Destroyed;
			if (self.ThrowOnDestroy)
				throw std::runtime_error{"Destruction handler failed"};
			return std::nullopt;
		}
	};

	struct HookButton : UI::Button
	{
		using UI::Button::OnMessage;
		int Clicks = 0;

		~HookButton() { this->Destroy(); }
		void OnClick(this auto& self) { ++self.Clicks; }
		auto OnMessage(this auto&, UI::Win32Message<WM_PAINT>) -> UI::MessageResult { return std::nullopt; }
	};

	struct PassiveButton : UI::Button
	{
		using UI::Button::OnMessage;
		int* DestructionCallbacks;

		explicit PassiveButton(int& callbacks) : DestructionCallbacks(&callbacks) {}
		auto OnMessage(this auto& self, UI::Win32Message<WM_NCDESTROY>) -> UI::MessageResult
		{
			++*self.DestructionCallbacks;
			return std::nullopt;
		}
	};

	struct NotificationWindow : UI::TopLevelWindow
	{
		ProbeControl Probe;
		HookButton Button;
		bool ThrowOnPaint = false;

		~NotificationWindow() { this->Destroy(); }

		auto ClassName(this auto&&) noexcept -> std::wstring_view { return L"Calculator-RoutingTestParent"; }
		auto GetCreationArgs(this auto&&) -> UI::CreateWindowArgs { return { .Width = 200, .Height = 200 }; }
		auto GetClass(this auto&& self) -> WNDCLASSEXW
		{
			return {
				.cbSize = sizeof(WNDCLASSEXW),
				.hInstance = GetModuleHandleW(nullptr),
				.lpszClassName = self.ClassName().data()
			};
		}

		void Init(this auto& self)
		{
			self.Probe.Create(self.GetHandle());
			self.Button.Create(self.GetHandle());
		}

		auto OnMessage(this auto& self, UI::Win32Message<WM_PAINT>) -> UI::MessageResult
		{
			if (self.ThrowOnPaint)
				throw std::runtime_error{"Parent paint handler failed"};
			return std::nullopt;
		}

		template<Win32::DWORD VMsg>
			requires (VMsg == WM_COMMAND or VMsg == WM_NOTIFY or VMsg == WM_DRAWITEM)
		auto OnMessage(this auto& self, UI::Win32Message<VMsg> message) -> UI::MessageResult
		{
			auto source = UI::NotificationSource(message);
			if (source == self.Probe.GetHandle())
				return self.Probe.HandleMessage(message.Hwnd, message.uMsg, message.wParam, message.lParam);
			if (source == self.Button.GetHandle())
				return self.Button.HandleMessage(message.Hwnd, message.uMsg, message.wParam, message.lParam);
			return std::nullopt;
		}
	};

	static_assert(not std::is_move_constructible_v<ProbeControl>);
	static_assert(not std::is_copy_constructible_v<NotificationWindow>);
	static_assert(not std::is_move_constructible_v<UI::CalculatorMainWindow>);
	static_assert(std::convertible_to<decltype(&UI::Control::SubclassProc<ProbeControl>), SUBCLASSPROC>);

	auto DrainMessages() -> std::optional<WPARAM>
	{
		auto result = std::optional<WPARAM>{};
		MSG message{};
		// WM_QUIT is synthesized only after other posted messages have drained.
		while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
		{
			if (message.message == WM_QUIT)
				result = message.wParam;
			else if (message.message == WM_PAINT)
				ValidateRect(message.hwnd, nullptr);
		}
		return result;
	}

	void CheckFailureQuit(std::string_view stage)
	{
		auto result = DrainMessages();
		Check(result and *result == 1,
			std::format("{}: expected WM_QUIT(1), got {}", stage, result ? std::to_string(*result) : "none"));
	}

	void TestForwardingAndLifetime()
	{
		NotificationWindow parent;
		parent.Create();
		NativeMouseMoves = NativeErases = 0;
		auto hwnd = parent.Probe.GetHandle();
		Check(SendMessageW(hwnd, WM_MOUSEMOVE, 0, 0) == 42 and NativeMouseMoves == 1,
			"An observer must forward to the native procedure exactly once");
		Check(SendMessageW(hwnd, WM_ERASEBKGND, 0, 0) == 0 and NativeErases == 0,
			"A zero result must suppress native processing");

		auto destructionCallbacks = 0;
		HWND passiveHandle = nullptr;
		{
			PassiveButton passive{destructionCallbacks};
			passive.Create(parent.GetHandle());
			passiveHandle = passive.GetHandle();
		}
		Check(not IsWindow(passiveHandle) and destructionCallbacks == 0,
			"Base teardown must disconnect callbacks before destroying a dead derived object");

		parent.Probe.SubclassId = 999;
		parent.Destroy();
		Check(not parent.GetHandle() and not parent.Probe.GetHandle() and not parent.Button.GetHandle(),
			"Parent destruction must clear every owned child HWND");
		Check(parent.Probe.Destroyed == 1 and parent.Probe.IdQueries == 1,
			"Destruction must dispatch once without recomputing the subclass ID");
		DrainMessages();
	}

	void TestNotificationsAndClickHook()
	{
		NotificationWindow parent;
		parent.Create();
		NMHDR notification{parent.Probe.GetHandle(), 201, NM_CUSTOMDRAW};
		Check(SendMessageW(parent.GetHandle(), WM_NOTIFY, 201, reinterpret_cast<LPARAM>(&notification)) == 73,
			"WM_NOTIFY must reach the typed control and preserve its result");
		Check(parent.Probe.NotificationParent == parent.GetHandle(), "Reflection must retain the parent HWND");

		DRAWITEMSTRUCT drawing{ .CtlType = ODT_BUTTON, .CtlID = 201, .hwndItem = parent.Probe.GetHandle() };
		Check(SendMessageW(parent.GetHandle(), WM_DRAWITEM, 201, reinterpret_cast<LPARAM>(&drawing)) == 1,
			"WM_DRAWITEM must reach the typed control");
		drawing.CtlID = 0;
		Check(UI::NotificationSource(UI::Win32Message<WM_DRAWITEM>{
			parent.GetHandle(), 0, reinterpret_cast<LPARAM>(&drawing)}) == parent.Probe.GetHandle(),
			"Control ID zero must not be mistaken for an owner-drawn menu");
		drawing.CtlType = ODT_MENU;
		Check(not UI::NotificationSource(UI::Win32Message<WM_DRAWITEM>{
			parent.GetHandle(), 0, reinterpret_cast<LPARAM>(&drawing)}),
			"Menu drawing must not be reflected to a control");

		parent.Button.Click();
		Check(parent.Button.Clicks == 1, "Inherited BN_CLICKED handling must invoke OnClick once");
		SendMessageW(parent.GetHandle(), WM_COMMAND, MAKEWPARAM(100, BN_SETFOCUS),
			reinterpret_cast<LPARAM>(parent.Button.GetHandle()));
		Check(parent.Button.Clicks == 1, "Other button notifications must not invoke OnClick");
		parent.Destroy();
		DrainMessages();
	}

	void TestCallbackExceptionsAndInitialization()
	{
		NotificationWindow parent;
		parent.Create();
		for (auto failure : {1, 2})
		{
			parent.Probe.ThrowOnMove = failure;
			Check(SendMessageW(parent.Probe.GetHandle(), WM_MOUSEMOVE, 0, 0) == 0,
				"Callback exceptions must not escape SendMessage");
			CheckFailureQuit(std::format("Control exception {}", failure));
		}
		parent.ThrowOnPaint = true;
		SendMessageW(parent.GetHandle(), WM_PAINT, 0, 0);
		CheckFailureQuit("Parent exception");
		parent.ThrowOnPaint = false;

		parent.Probe.ThrowOnDestroy = true;
		DestroyWindow(parent.Probe.GetHandle());
		Check(not parent.Probe.GetHandle(), "Ownership cleanup must precede a throwing destruction handler");
		CheckFailureQuit("Destruction exception");

		ProbeControl retry;
		retry.RejectInit = true;
		auto rejected = false;
		try { retry.Create(parent.GetHandle()); }
		catch (const std::runtime_error&) { rejected = true; }
		Check(rejected and not retry.GetHandle(), "Failed Init must destroy the partially initialized control");
		retry.RejectInit = false;
		retry.Create(parent.GetHandle());
		Check(retry.GetHandle() != nullptr, "A failed initialization must permit a later retry");
		parent.Destroy();
		DrainMessages();
	}

	void TestCalculatorActivation()
	{
		UI::CalculatorMainWindow calculator;
		calculator.Create();
		auto parent = calculator.GetHandle();
		auto button = GetDlgItem(parent, 101);
		auto output = GetDlgItem(parent, 1);
		auto text = [output]
		{
			wchar_t buffer[32]{};
			GetWindowTextW(output, buffer, 32);
			return std::wstring{buffer};
		};

		SendMessageW(button, BM_CLICK, 0, 0);
		Check(text() == L"1", "A programmatic click must append exactly one digit");
		SendMessageW(parent, WM_COMMAND, MAKEWPARAM(101, BN_SETFOCUS), reinterpret_cast<LPARAM>(button));
		SendMessageW(parent, WM_COMMAND, MAKEWPARAM(101, BN_CLICKED), 0);
		Check(text() == L"1", "Focus and menu commands must not trigger calculator input");
		SendMessageW(button, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(2, 2));
		SendMessageW(button, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(500, 500));
		SendMessageW(button, WM_LBUTTONUP, 0, MAKELPARAM(500, 500));
		Check(text() == L"1", "A canceled mouse press must not activate the button");
		SendMessageW(button, WM_KEYDOWN, VK_SPACE, 0);
		SendMessageW(button, WM_KEYUP, VK_SPACE, 0);
		Check(text() == L"11", "Native keyboard activation must follow the same click route");
		SendMessageW(parent, WM_KEYUP, VK_NUMPAD3, 0);
		Check(text() == L"113", "Existing calculator keyboard bindings must still work");
		calculator.Destroy();
		DrainMessages();
	}
}

int main(int argc, char** argv)
{
	auto report = [argc, argv](std::string_view message)
	{
		if (argc > 1)
			std::ofstream{argv[1]} << message << '\n';
		std::println("{}", message);
	};
	try
	{
		INITCOMMONCONTROLSEX controls{sizeof(INITCOMMONCONTROLSEX), ICC_STANDARD_CLASSES};
		Check(InitCommonControlsEx(&controls), "Common control initialization failed");
		WNDCLASSEXW windowClass{
			.cbSize = sizeof(WNDCLASSEXW),
			.lpfnWndProc = NativeControlProc,
			.hInstance = GetModuleHandleW(nullptr),
			.lpszClassName = ProbeControl::ClassName.data()
		};
		Check(RegisterClassExW(&windowClass) != 0, "Test control class registration failed");
		TestForwardingAndLifetime();
		TestNotificationsAndClickHook();
		TestCallbackExceptionsAndInitialization();
		TestCalculatorActivation();
		report("All 4 control routing tests passed.");
		return 0;
	}
	catch (const std::exception& ex)
	{
		report(std::format("Control routing test failed: {}", ex.what()));
		return 1;
	}
}
