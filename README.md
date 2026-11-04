# Hello Calculator 32

## Introduction

The aim of this repo is to explore interacting with Win32 GUI objects using ideas and techniques from type-oriented and declarative programming. A rudimentary calculator is built using these techniques.

## Rationale

The Win32 API is a C API, therefore a traditional C++ object-oriented GUI programming style might follow a setup like this: a pure virtual class representing an abstract Win32 window, inherited by an abstract base class that provides some functionality, which is then inherited by two additional classes with one being for Win32 controls and the other for top-level windows. Finally, an additional class inherits from the top-level window class, which represents the actual application's top-level window. This window would then have class members that represent the child controls and windows, or perhaps aggregate them into a collection of base class pointers, with the child controls being created at runtime. In such a setup, if a message is received where the window needs to know which child generated it (e.g. `WM_COMMAND` on a button click), the application window would then have to inspect its collection of child controls (perhaps through a loop) and compare the IDs. If the application needs to manipulate the actual child concrete control class as a response to the message, the application would need to do a downcast, which is an error-prone operation. This approach makes heavy use of inheritance, `virtual` and runtime polymorphism -- all prototypical aspects of a classical object-oriented approach.

By contrast, a type-oriented approach sits somewhere between an object-oriented style and a functional style of programming. Instead of type-erasing the concrete control types via base pointers and polymorphism, a type-oriented approach would instead declare all the child controls upfront, using such things as parameterized types and static polymorphism to represent child controls as a set of strong types. To manage these controls, in contrast to a collection of base-class pointers, these controls would exist as their strong types in a collection which provides primitives for retrieving and manipulating them. By dealing directly with the concrete types, C++'s type system can be better brought to bear to verify the correctness of an application's logic. Indeed, with this approach, certain kinds of runtime errors are either eliminated entirely, or become compile-time errors instead. Moreover, static polymorphism avoids `virtual`, and this can aid compiler optimizations by making the intent of the code more transparent to the compiler. 

This approach does not wholly dispense with object-oriented techniques; inheritance and classes can still be used, but the mechanisms of achieving polymorphism and managing types and instances are very different.

## Implementation

Five C++ features form the backbone of this implementation.

1. **Parameterized types:** rather than declaring a generic `Button` class that will represent all buttons, we can create parameterized types that represent each button. These strong types provider better guarantees than a generic `Button` class, e.g. you can't assign parameterized type to another parametized type; they are distinct. We can extend these stronger guarantees with techniques like function overloads for application logic, as opposed to runtime inspection and type coercion of a type-erased collection.
2. **Tuples:** tuples provide us a powerful mechanism to create a heterogeneous collection. By wrapping them up in a utility type, we can create something even more useful.
3. **Deducing this/explicit object parameter:** a C++23 feature that makes CRTP-style techniques much easier and more convenient to implement, including static polymorphism.
4. **Generic lambas and functions**: generic lambdas remain one of the most useful additions to C++ and are essential in generic code.
5. **Concepts:** these are extremely useful for dealing with a set of distinct types in generic way. E.g., you may wish to run some logic only on buttons (which are parameterized types); a concept can constrain the acceptable types to a function or lambda, avoiding things like extensive `if constexpr` statements.

You'll notice a console window is displayed alongside the GUI window when the calculator runs; this is simply used in the `Debug` and `Instrumented` builds for logging and debugging purposes. It's suppressed in the `Release` build.

### Message routing and lifetime

Window and control objects are non-copyable and non-movable because native callbacks retain their addresses. Call `Create()` through the concrete type, not a base reference: explicit-object-parameter dispatch uses the caller's static type. All creation, message handling, and destruction must stay on the owning UI thread.

If a derived window or control owns state used by handlers, call `Destroy()` at the start of its destructor, before that state is torn down. `CalculatorMainWindow` demonstrates this. The base destructors disconnect callbacks before releasing any remaining HWND; `WM_NCDESTROY` also clears ownership when Windows destroys a child through its parent.

`OnMessage` may return either `Win32::LRESULT` or `UI::MessageResult`. Every result value, including zero, means handled. Return `std::nullopt` to request default processing instead of calling a default procedure yourself; the router forwards once to the appropriate procedure. Adding an overload to a derived class hides inherited overloads, so preserve them with a declaration such as `using Button::OnMessage;`. Keep the relevant explicit message list in sync with new handlers; overloads for unlisted messages are not dispatched.

Controls receive their own native messages through `SetWindowSubclass`. Notifications about a control (`WM_COMMAND`, `WM_NOTIFY`, and `WM_DRAWITEM`) are sent to its parent instead. `CalculatorMainWindow` reflects these to the concrete control identified by the source HWND. Reflected messages retain the original **parent** HWND and parameters; return `std::nullopt` for parent default processing, never call `DefSubclassProc` on that parent. An optional button `OnClick()` hook consumes `BN_CLICKED`; otherwise the calculator handles the click. Raw mouse-up is not treated as activation.

The original `WM_CREATE` occurs before the subclass is installed and cannot reach a control's typed handler. Use `Init()` for post-creation initialization. Exceptions escaping typed handlers are caught at native callback boundaries, reported to the debugger, and request application shutdown with exit code 1; they are not forwarded for a second processing attempt.

## Building

Visual Studio 2026 with MSVC 17.15 or later with the C++ desktop workload is required to build the solution. Three build configurations are provided: `Release`, `Debug` and `Instrumented`. The `Instrumented` configuration has address sanitizer (ASAN) enabled, which ironically enough found [a bad codegen bug in MSVC](https://developercommunity.visualstudio.com/t/Runtime-crash-with-constexpr-and-std::st/10939905) (I'm pretty certain I've also run into other MSVC codegen bugs while developing this sample, but I can't be bothered fully investigating them).

From a Visual Studio Developer PowerShell, build and run the native control routing regression tests with:

```powershell
msbuild src\hello-calculator-32.vcxproj /p:Configuration=Debug /p:Platform=x64 /p:RoutingTests=true
.\src\x64\Debug\RoutingTests\control-routing-tests.exe
```

The tests use hidden windows to check forwarding, parent notification reflection, handle cleanup, callback exceptions, and mouse/keyboard activation without entering the interactive calculator loop.

Use `Platform=Win32` and the corresponding `src\Win32` output path to check 32-bit callbacks, or `Configuration=Instrumented` with `Platform=x64` to run with AddressSanitizer.
