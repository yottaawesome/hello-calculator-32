#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.1.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")

export module calculator;
export import :win32;
export import :error;
export import :raii;
export import :string;
export import :ui_toplevelwindow;
export import :ui_calculatormainwindow;
export import :ui_control;
export import :ui_common;
export import :ui_font;
export import :log;
export import :ui_capability;
export import :misc;

import std;

export extern "C" auto wWinMain(
	Win32::HINSTANCE hInstance, 
	Win32::HINSTANCE hPrevInstance,
	Win32::PWSTR pCmdLine, 
	int nCmdShow
) -> int 
try
{
	//Win32::InitCommonControls();
	Win32::INITCOMMONCONTROLSEX icc{
		icc.dwICC =
			Win32::InitCommonControlsFlag::Animate |
			Win32::InitCommonControlsFlag::NativeClass |
			Win32::InitCommonControlsFlag::Standard | 
			Win32::InitCommonControlsFlag::Cool |
			Win32::InitCommonControlsFlag::Bar,
		icc.dwSize = sizeof(icc)
	};
	Win32::InitCommonControlsEx(&icc);
	UI::CalculatorMainWindow{}.Create().Show().MainLoop();

	return 0;
}
catch (const std::exception& ex)
{
	Log::Info("An error occurred: {}", ex.what());
	Win32::MessageBoxA(nullptr, "Error", ex.what(), Win32::MessageBoxStyle::OK);
	return 1;
}
