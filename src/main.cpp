import std;
import calculator;

auto wWinMain(Win32::HINSTANCE, Win32::HINSTANCE, Win32::PWSTR, int) -> int
try
{
	//Win32::InitCommonControls();
	auto icc = Win32::INITCOMMONCONTROLSEX{
		.dwSize = sizeof(INITCOMMONCONTROLSEX),
		.dwICC =
			Win32::InitCommonControlsFlag::Animate |
			Win32::InitCommonControlsFlag::NativeClass |
			Win32::InitCommonControlsFlag::Standard |
			Win32::InitCommonControlsFlag::Cool |
			Win32::InitCommonControlsFlag::Bar,
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
