export module calculator;
export import :win32;
export import :error;
export import :raii;
export import :ui;
export import :log;

import std;

export extern "C" auto wWinMain(
	Win32::HINSTANCE hInstance,
	Win32::HINSTANCE hPrevInstance,
	Win32::PWSTR pCmdLine,
	int nCmdShow
) -> int
{
	// register class
	UI::MainWindow app;
	app.Register().Create().Show().MainLoop();

	// create window

	// draw input windo

	// draw buttons

	// read input

	// calculate result

	// show result

	return 0;
}
