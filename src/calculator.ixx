#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.1.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")

export module calculator;
export import :win32;
export import :error;
export import :raii;
export import :string;
export import :log;
export import :misc;
export import :math;
export import :build;
export import :ui;
