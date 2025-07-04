import std;
import win32;

constexpr std::wstring_view ClassName = L"Calculator-Gui";

class MainWindow
{
public:

private:
};

auto RegisterWindowClass() -> Win32::ATOM
{
	Win32::WNDCLASSEXW windowClass{
		.cbSize = sizeof(windowClass),
		.lpfnWndProc = nullptr,
		.hInstance = Win32::GetModuleHandleW(nullptr),
		.lpszClassName = ClassName.data(),
	};

	Win32::RegisterClassExW(&windowClass);

	return {};
}

auto wWinMain(
	Win32::HINSTANCE hInstance, 
	Win32::HINSTANCE hPrevInstance, 
	Win32::PWSTR pCmdLine, 
	int nCmdShow
) -> int
{
	// register class

	// create window

	// draw input windo

	// draw buttons

	// read input

	// calculate result

	// show result

	return 0;
}

