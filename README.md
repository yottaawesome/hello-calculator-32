# Hello Calculator 32

## Introduction

The aim of this repo is to explore interacting with Win32 GUI objects using some ideas from type-oriented programming. A rudimentary calculator is built using these techniques.

## Rationale

The Win32 API is a C API, therefore a traditional C++ object-oriented GUI programming style might follow a setup like this: a pure virtual class representing an abstract Win32 window, inherited by an abstract base class that provides some functionality, which is then inherited by two additional classes with one being for Win32 controls and the other for top-level windows. Finally, an additional class inherits from the top-level window class, which represents the actual application's top-level window. This window would then have class members that represent the child controls and windows, or perhaps aggregate them into a collection of base class pointers, with the child controls being created at runtime. In such a setup, if a message is received where the window needs to know which child generated it (e.g. `WM_COMMAND` on a button click), the application window would then have to inspect its collection of child controls (perhaps through a loop) and compare the IDs. If the application needs to manipulate the actual child concrete control class as a response to the message, the application would need to do a downcast, which is an error-prone operation. This approach makes heavy use of inheritance, `virtual` and runtime polymorphism -- all prototypical aspects of a classical object-oriented approach.

By contrast, a type-oriented approach sits somewhere between an object-oriented style and a functional style of programming. Instead of type-erasing the concrete control types via base pointers and polymorphism, a type-oriented approach would instead declare all the child controls upfront, using such things as parameterized types and static polymorphism to represent child controls as a set of strong types. To manage these controls, in contrast to a collection of base-class pointers, these controls would exist as their strong types in a collection which provides primitives for retrieving and manipulating them. By dealing directly with the concrete types, C++'s type system can be better brought to bear to verify the correctness of an application's logic. Indeed, with this approach, certain kinds of runtime errors are either eliminated entirely, or become compile-time errors instead. This approach does not wholly dispense with object-oriented techniques; inheritance and classes can still be used, but the mechanisms of achieving polymorphism and managing types and instances are very different.

## Implementation

Five C++ features form the backbone of this implementation.

1. **Parameterized types:** rather than declaring a generic `Button` class that will represent all buttons, we can create parameterized types that represent each button. These strong types provider better guarantees than a generic `Button` class, e.g. you can't assign parameterized type to another parametized type; they are distinct. We can extend these stronger guarantees with techniques like function overloads for application logic, as opposed to runtime inspection and type coercion of a type-erased collection.
2. **Tuples:** tuples provide us a powerful mechanism to create a heterogeneous collection. By wrapping them up in a utility type, we can create something even more useful.
3. **Deducing this/explicit object parameter:** a C++23 feature that makes CRTP-style techniques much easier and more convenient to implement, including static polymorphism.
4. **Generic lambas and functions**: generic lambdas remain one of the most useful additions to C++ and are essential in generic code.
5. **Concepts:** these are extremely useful for dealing with a set of distinct types in generic way. E.g., you may wish to run some logic only on buttons (which are parameterized types); a concept can constrain the acceptable types to a function or lambda, avoiding things like extensive `if constexpr` statements.

You'll notice a console window is displayed alongside the GUI window when the calculator runs; this is simply used for logging and debugging purposes.

## Building

Visual Studio 2022 with MSVC 17.14 or later with the C++ desktop workload is required to build the solution, as this version is the earliest that supports all the requisite C++ features. Three build configurations are provided: `Release`, `Debug` and `Instrumented`. The `Instrumented` configuration has address sanitizer (ASAN) enabled, which ironically enough found [a bad codegen bug in MSVC](https://developercommunity.visualstudio.com/t/Runtime-crash-with-constexpr-and-std::st/10939905) (I'm pretty certain I've also run into other MSVC codegen bugs while developing this sample, but I can't be bothered fully investigating them).
