# <img src="../v-gears.png" alt="V-Gears" width="50"/> V-Gears C++ Style Guide
- *For the LUA Style Guide, see the [V-Gears LUA Style Guide](STYLE-LUA.md "LUA Style Guide")*

-----------------------
## TL;DR;

1. Readability over performance.
2. Be consistent whit existing code.
3. When in doubt, use the simplest solution.

-----------------------
## Why a Style Guide?

As Q-Gears started growing in size, it became more and more obviuos that it was the work of multiple developers, across several years. There were differences in style, namespace use, variable and function name...

While no one of those different styles was inherently superior to any other, it made the project inelegant in the best of cases, and consfusing and non-interoperable in the worst of them.

When it became V-Gears, every piece of code was, in different degrees, refactored, documented, and simplified. The same style was applied across the whole project, and it was a huge amount of work. 

To avoid having to go through all of that again, this guide was created.

This guide describes the rules for code styling for V-Gears, and, as far as possible, explains why a rule is chosen over other options.

-----------------------
## C++ Version

Currently, the code should target C++14.

Over time, the project and this guide will be upgraded to newer versions of C++. This will be done when and only when it's safe enough to upgrade without compatibility issues. Until then, any contributions must be ensured to compile and run with C++14.

Do not use non-standard extensions, such as GNU extensions. They are very specific to compilers and can jeopardize current or furure portability efforts.

-----------------------
## Source Files

Source files must have the extension `.cpp`.

## Header Files

In general, every `.cpp` file should have an associated `.h` header file. There are some exceptions to this rule:

* Files with only `main()` functions.
* Test files.

Correct use of header files can make a huge difference to the readability and size of the project, and can also save a lot of time during compilation.

The following rules will guide you through the various pitfalls of using header files.

### Self-contained Headers

All header files should be self-contained. Users and refactoring tools should not have to adhere to special conditions to include the header.

When a header declares inline functions or templates that clients of the header will instantiate, the inline functions and templates must also have definitions in the header, either directly or in files it includes.

### Include Guards

All header files should have an include guard. The prefered methoud to do is is with `#pragma once`.

This directive is mature enough to favor it over `#ifndef` guards, which are more complex, and can conflict with one another in a large enough project.

### Include Only What Is Needed

If a source or header file refers to a symbol defined elsewhere, the file should directly include a header file which properly intends to provide a declaration or definition of that symbol. It should not include header files for any other reason.

Do not rely on transitive inclusions. This can be hard to debug when one of the transitive files gets refactored.

### Forward Declarations

Avoid using forward declarations where possible.

A "forward declaration" is a declaration of an entity without an associated definition. They can save on compile time, but they can be tricky for several reasons:

* They can hide dependencies.
* It can be difficult for IDEs to find a symbol definition.
* It may break code if the declared symbol definition changes,  in a non-obvious way.

The only acceptable reason to use a forward declaration is to prevent an include loop i.e. two files including each other. Even then, an include loop i'ts usually a sign of deeper problems, and the affected code probably can and should be refactored to avoid the loop and the forward declaration alltpgether.

### Inline Functions

As a rule of thumb, don't inline functions. Let your compiler do it's thing, it will inline function as it sees fit.

In any case, define functions inline only when they are small (about 10 lines or fewer).

Overuse of inlining can actually make the program slower. Depending on a function's size, inlining it can cause the code size to increase or decrease. Inlining a very small accessor function will usually decrease code size while inlining a very large function can dramatically increase code size. On modern processors smaller code usually runs faster due to better use of the instruction cache.

Always avoid inlining functions with loops or switch statements.

### Names and Order of Includes

Include headers in the following order, without whitelines behind them:

1 - C++ standard library headers
2 - C system headers
3 - Other libraries' headers
4 - Related header
5 - Other V-Gears headers.

There is no particular reason as to why this order has been chosen, save for consistency with the rest of the project.

Don't use relative paths for the include directives.

-----------------------
## Scoping

### Namespaces

The use of namespaces is neither prefered nor discouraged.

If a collection of files, classes, functions belong to a well-defined and well-limited part of the application, feel free to include them in a namespace.

When using namespaces, try to follow these directives:

* **Avoid heavy namespace nesting**: As a rule, do not nest more than 3 namespaces.
* **Use short but descriptive names**: When possible, one word is enough. Acronyms are OK.
* **Do not use unnamed namespaces**.
* **Do not inline namespaces**.

### Internal Linkage

When definitions in a `.cpp` file do not need to be referenced outside that file, give them internal linkage by declaring them `static`. Do not use either of these constructs in `.h` files.

Internal linkage should be used whenever possible in `.cpp` for all code that does not need to be referenced elsewhere. Do not use internal linkage in `.h` (it should not bee needed).


### Nonmember and Static Member

Whenever possible, try to avoid static functions outside of classes. Try to create them as static methods in the class they are related to. It is allowed to create a class to hold several related static methods.

### Global Variables and Functions

Avoid using global variables functions as much as possible, use them only as a last resort. They are incredibly hard to keep track of.

### Local Variables

Place a function's variables in the narrowest scope possible, and initialize variables in the declaration.

-----------------------
## Classes

This section describes several rules to take into account when writing a class.

### Work in Constructors

Avoid initialization that can fail if an error can't be signaled (terminating the application can be a good signal, it the error is non-recoverable).
Avoid calling virtual functions from constructors. When neccesary, implement a separate initialization method and call it from the constructor.

### Implicit Conversions

Do not define implicit conversions. Use the `explicit` keyword for conversion operators and single-argument constructors.


### Structs vs. Classes

The `struct` and `class` keywords behave almost identically in C++. When trying to choose between them, use these considerations.

Structs should be used for (mostly) passive for data. Most or all fields must be public. If there are methods, they must be accessors or very simple methods. For example, it's OK to have an array or a vector as a member, and a method to fetch an element at a specific position.

For anything more complex, use classes. In classes, fields must be private and protected, and have the required accessor methods.

Use a `struct` only for passive objects that carry data; everything else is a `class`.

### Inheritance


All inheritance should be `public`. 

Limit the use of `protected` to those member functions that might need to be accessed from subclasses.

Explicitly annotate overrides of virtual functions or virtual destructors with exactly one of an `override` or (less frequently) `final` specifier. 

### Operator Overloading

Overload operators judiciously. Do not use user-defined literals.

Do not overload `&&`, `||`, `,` (comma), or unary `&`. Do not overload `operator""`, i.e., do not introduce user-defined literals. Do not use any such literals provided by others (including the standard library).

### Access Control

Make classes' data members `private`, unless they are constants. This simplifies reasoning about invariants, at the cost of some easy boilerplate in the form of accessors (usually `const`) if necessary.

### Declaration Order

Group similar declarations together, placing public parts earlier.

A class definition should usually start with a `public:` section, followed by `protected:`, then `private:`. Omit sections that would be empty.

Within each section, prefer grouping similar kinds of declarations together, and prefer the following order:

1.  Types and type aliases (`typedef`, `using`, `enum`, nested structs and classes, and `friend` types)
2.  Static constants
3.  Static methods.
4.  Constructors and assignment operators
5.  Destructor
6.  Assignment operators.
7.  Operator overloads.
8.  Factory functions.
9.  All other functions.
10.  Data members (static and non-static)

-----------------------
## Functions

### Inputs and Outputs

The output of a C++ function is naturally provided via a return value and sometimes via output parameters (or in/out parameters).

Prefer using return values over output parameters whenever possible: they improve readability, and often provide the same or better performance.

Prefer to return by value or, failing that, return by reference. Avoid returning a pointer unless it can be nullable.

Parameters are either inputs to the function, outputs from the function, or both. Non-optional input parameters should usually be values or `const` references, while non-optional output and input/output parameters should usually be references (which cannot be null).

When ordering function parameters, put all input-only parameters before any output parameters. In particular, do not add new parameters to the end of the function just because they are new; place new input-only parameters before the output parameters. This is not a hard-and-fast rule. Parameters that are both input and output muddy the waters, and, as always, consistency with related functions may require you to bend the rule.

### Write Short Functions

Prefer small and focused functions.

While there is no hard limit is placed on functions length, if a function exceeds about 80 lines, think about whether it can be broken up without harming the structure of the program.

Short functions are easier to debug, modify or refactor, and keep the code more readable and easier to maintain.

### Default Arguments

Default arguments are allowed on non-virtual functions when the default is guaranteed to always have the same value.

-----------------------
## Other C++ Features

### Friends

The use of `friend` classes and functionsis allowed, within reason, but it's best avoided whenever possible.

Friends extend, but do not break, the encapsulation boundary of a class. In some cases this is better than making a member public when you want to give only one other class access to it. However, most classes should interact with other classes solely through their public members.

### Exceptions

The use of C++ exceptions is allowed. They allow higher levels of an application to decide how to handle failures in deeply nested functions, without the obscuring and error-prone bookkeeping of error codes.

Some caution must be taken when using exceptions:

* When a `throw` statement is added to an existing function, all of its transitive callers must be examined. Either they must make at least the basic exception safety guarantee, or they must never catch the exception and be happy with the program terminating as a result. For instance, if `f()` calls `g()` calls `h()`, and `h` throws an exception that `f` catches, `g` has to be careful or it may not clean up properly.
* Functions may return in places you don't expect. This causes maintainability and debugging difficulties. All these cases must be described in the fuunction documentation.
* Specify `noexcept` when it is useful and correct.

### Run-Time Type Information (RTTI)

RTTI allows a programmer to query the C++ class of an object at run-time. This is done by use of `typeid` or `dynamic_cast`.

Avoid them whenever possible, except for tests.

### Casting

Use C++-style casts like `static_cast<float>(double_value)`, or brace initialization for conversion of arithmetic types like `int64_t y = int64_t{1} << 42`. Do not use cast formats like `(int)x` unless the cast is to `void`. You may use cast formats like \`T(x)\` only when \`T\` is a class type.

### Streams

Use streams where appropriate, and stick to "simple" usages. Overload `<<` for streaming only for types representing values, and write only the user-visible value, not any implementation details.

Overload `<<` as a streaming operator for your type only if your type represents a value, and `<<` writes out a human-readable string representation of that value. Avoid exposing implementation details in the output of `<<`; if you need to print object internals for debugging, use named functions instead (a method named `DebugString()` is the most common convention).

### Preincrement and Predecrement

Use the prefix form (`++ i`) of the increment and decrement operators unless you need postfix (`i ++`) semantics. This can result in a slightly better performance of the algorithm.


### Use of const

In APIs, use `const` whenever it makes sense. `constexpr` is a better choice for some uses of const.

Declared variables and parameters can be preceded by the keyword `const` to indicate the variables are not changed (e.g., `const int foo`). Class functions can have the `const` qualifier to indicate the function does not change the state of the class member variables (e.g., `class Foo { int Bar(char c) const; };`).

The const keyword must be before the type (`const int *foo` instead of `int const *foo`). Putting the `const` first is arguably more readable, since it follows English in putting the "adjective" (`const`) before the "noun" (`int`).

### Use of constexpr

Use `constexpr` to define true constants or to ensure constant initialization.

### 64-bit Portability

Code should be 64-bit and 32-bit friendly. Bear in mind problems of printing, comparisons, and structure alignment.

### Preprocessor Macros

Avoid defining macros, especially in headers; prefer inline functions, enums, and `const` variables.

Macros mean that the code you see is not the same as the code the compiler sees. 

### 0, NULL and nullptr

Use `nullptr` for pointers, and `'\0'` for chars (and not the `0` literal).

For pointers (address values), use `nullptr`, as this provides type-safety.

Use `'\0'` for the null character. Using the correct type makes the code more readable.

### sizeof

Prefer `sizeof(varname)` to `sizeof(type)`.


### Type Deduction (including auto)

Use type deduction only for local variables, where the scope is short and simple enough to be easily understood.

Abusing type deduction makes the code harder to read, and IDEs sometimes can't determine deducted types.

#### Function template argument deduction

Function template argument deduction is almost always OK. Type deduction is the expected default way of interacting with function templates, because it allows function templates to act like infinite sets of ordinary function overloads. Consequently, function templates are almost always designed so that template argument deduction is clear and safe, or doesn't compile.


### Class Template Argument Deduction

Use class template argument deduction only with templates that have explicitly opted into supporting it.

### Template Metaprogramming

Avoid complicated template programming.

The techniques used in template metaprogramming are often obscure to anyone but language experts. Code that uses templates in complicated ways is often unreadable, and is hard to debug or maintain.

### Boost

It's OK to use libraries from the Boost library collection.

-----------------------
## Naming

The most important consistency rules are those that govern naming. The style of a name immediately informs us what sort of thing the named entity is: a type, a variable, a function, a constant, a macro, etc., without requiring us to search for the declaration of that entity. The pattern-matching engine in our brains relies a great deal on these naming rules.

Naming rules are pretty arbitrary, and no one is inherently better than other, but for V-Gears, these are the choosen rules.


### File Names

Filenames start with a capital letter and have a capital letter for each new word, with no underscores:
* `MyUsefulClass.cpp`
* `ANiceLibrary.h`
* `Utilities.cpp`

For test files, use the same scheme, but with `test_` before the name:
*   `test_MyUsefulClass.cpp`

C++ source files should end in `.cpp` and header files should end in `.h`. Files that rely on being textually included at specific points should end in `.inc`, but try to avoid them.

### Type Names

Type names start with a capital letter and have a capital letter for each new word, with no underscores:
* `MyExcitingClass`
* `MyExcitingEnum`.

The names of all types — classes, structs, type aliases, enums, and type template parameters — have the same naming convention. Type names should start with a capital letter and have a capital letter for each new word. No underscores. 

### Variable Names

The names of variables (including function parameters) and data members are all lowercase, with underscores between words. Data members of classes (but not structs) additionally have trailing underscores:
* `a_local_variable`,
* `some_public_struct_data_member`,

Protected and private data members of classes and structs foollow the same convention, but they also end with a trailing underscore:
* `private_class_data_member_`.

### Constant Names

Variables declared constexpr or const, and whose value is fixed for the duration of the program, are named with uppercase, with the words separated by underscores:
```cpp
const int DAYS_IN_A_WEEK = 7;
const int APRIL = 4;
```

### Function Names

Function names start with a capital letter and have a capital letter for each new word, with no underscores. This is true for regular functions, class and struct methods (public or private), accessors...:

AddTableEntry()
MyClass::DeleteUrl()
OpenFileOrDie()


### Namespace Names

Namespaces start with a capital letter and have a capital letter for each new word, with no underscores. If the name is an acronym, all letters can be uppercase.
```cpp
namespace FieldTools = 7;
using namespace ACME;
```

### Enumerator Names

Enumerators (for both scoped and unscoped enums) should be named like constants, uppercase, with the words separated by underscores:

enum class URL_TABLE_ERROR {
  OK = 0,
  OUT_OF_MEMORY,
  MALFORMED_INPUT,
};

### Macro Names

If the use of macros can't be avoided, name them like constants: Uppercase, with the words separated by underscores.

## Comments

This section applies only to individual inline comments. For code documentation, see the [Code Documentation](#Code__Documentation) section.

Always write comments in tricky, non-obvious, interesting, or important parts of the code.

### Comment Style

For comments inside the code, use either the `//` or `/* */` syntax. They both can be used together, there is no need for consistency in this regard.

### Punctuation, Spelling, and Grammar

Pay attention to punctuation, spelling, and grammar; it is easier to read well-written comments than badly written ones.

Comments should be as readable as narrative text, with proper capitalization and punctuation. In many cases, complete sentences are more readable than sentence fragments. 

### TODO Comments

Use `TODO` comments for code that is temporary, a short-term solution, or good-enough but not perfect.

`TODO`s should include the string `TODO` in all caps, followed by an explanation of what the code should be doing, ant the why of the TODO. A todo comment can expand serveral lines

```cpp
// TODO: This always give fixed values.

// TODO: Why is this used twice?

// TODO: Move depth calculation to flevelBackgroundLoader maybe? and let
// Backgorund2D only handle 0 <= depth <= 1 or so?
```

If your `TODO` is of the form "At a future date do something" make sure that you either include a very specific date ("Fix by November 2005") or a very specific event ("Remove this code when all clients can handle XML responses.").

### Copyright and License

Every file should contain copiright and license boilerplate in the very first line, exactly like this:

```cpp
/*
 * Copyright (C) 2022 The V-Gears Team
 *
 * This file is part of V-Gears
 *
 * V-Gears is free software: you can redistribute it and/or modify it under
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.0 (GPLv3) of the License.
 *
 * V-Gears is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
```

-----------------------
## Code Documentation

Every piece of code must be documented. The project uses Doxygen as a documentation generator, but the coments themselves are in JavaDoc format.

Do not duplicate comments in both the `.h` and the `.cpp`. Duplicated comments diverge. When a function is declared in a `.h` file, document it there.


### Class, Struct, Enum, Enum Items, Constant and Class Member Comments

Every class, struct, enum, enum item constant and class member must be preceded by a codeblock. If a one-liner is enough to describe what it's for, then so be it. If further explanation is needed, leave an empty line and add an explanation. Examples:

```cpp
/**
 * Used to store coordinates.
 */
struct Coordinates{...}
```

```cpp
/**
 * This class does a tricky thing.
 *
 * It represents something, but should not be used used when some other thing.
 */
class TrickyClass{...}
```

```cpp
/**
 * Represents a field map model.
 *
 */
class Model{
    ...
    private:

        /**
         * The height of the model.
         */
        height_;
}
```

```cpp
    /**
     * The scale factor for line point coordinates.
     * 
     * When a LINE opcode is found in the game scripts, the line points X and Y coordinates
     * must be scaled down by this factor.
     */
    static float LINE_SCALE_FACTOR;
```

```cpp
    /**
     * How to scroll the background.
     */
    enum SCROLL_TYPE{

        /**
         * Don't scroll the background.
         */
        NONE,

        /**
         * Linearly scroll.
         *
         * May seem unnatural in scripted sequences, but it's OK for
         * character tracking.
         */
        LINEAR,
            
        ...
};
```

### Function Comments

Each function or class method (public, protected or private) declaration must be preceded by it's own Javadoc block. Like this:
```cpp
/**
 * Processes a conditional jump instruction.
 *
 * Checks if the condition is a function or a comparison, and adds the function to the stack.
 *
 * @param[in] function Function to process. Unused.
 * @param[out] stack Function stack. The instruction will be added here.
 * @param[in] engine Engine. Unused.
 * @param[in,out] code_gen Code generator to append lines.
 */
virtual void ProcessInst(Function& function, ValueStack& stack, Engine* engine, CodeGenerator* code_gen) override;
```

```cpp
/**
 * Checks the model visibility.
 *
 * @return True if the model is visible, false if it's invisible.
 */
virtual bool IsVisible() const;
```
If one line is not enough to describe what the function it's for, leave an empty line and add an explanation. Always document every parameter the function takes, indicating it's direction (`[in]`, `[out]` or `[in,out]`) and the return value, if any

-----------------------
## Formatting

Coding style and formatting are pretty arbitrary, but a project is much easier to follow if everyone uses the same style. 

### Line Length

Each line of text in your code should be at most 100 characters long. This is a reasonable compromise between the old standard of 80 characters and what moders screens offer.


### Non-ASCII Characters

Non-ASCII characters should be avoided, and must use UTF-8 formatting when used.


### Spaces vs. Tabs

Use only spaces, and indent 4 spaces at a time. Do not use tabs.

### Function Declarations, Definitions, and Calls

Return type on the same line as function name, parameters on the same line if they fit. Wrap parameter lists which do not fit to a line on their own, indented by 2 extra spaces.

Functions look like this:

```cpp
ReturnType ClassName::FunctionName(Type par_name1, Type par_name2) {
  DoSomething();
  ...
}
```

If there is too much text to fit on one line:

```cpp
ReturnType ClassName::ReallyLongFunctionName(
  Type par\_name1, Type par\_name2, Type par\_name3
){
  DoSomething();
  ...
}
```

Some points to note:

*   Choose good parameter names.
*   A parameter name may not be omitted, even if the parameter is not used in the function's definition.
*   If you cannot fit the return type and the function name on a single line, break between them.
*   If you break after the return type of a function declaration or definition, overindent with two spaces
*   The open parenthesis is always on the same line as the function name.
*   There is never a space between the function name and the open parenthesis.
*   There is never a space between the parentheses and the parameters.
*   The open curly brace is always in the same line astthe closing parameter parenthesis.
*   There should not be a space between the close parenthesis and the open curly brace.

### Conditionals

In an `if` statement, including its optional `else if` and `else` clauses, put one space between the `if` and the opening parenthesis, but no spaces between the parentheses and the condition or initializer or between the closing parenthesis and the curly brace.
```cpp
if(condition){...}              // Bad - space missing after IF.
if ( condition ){...}          // Bad - space between the parentheses and the condition.
if (condition) {...}               // Bad - space before brace.
if (condition){...}              // Good
```

Use curly braces for the controlled statements following `if`, `else if` and `else`. Break the line immediately after the opening brace, and immediately before the closing brace. A subsequent `else`, if any, appears on a new line after the preceding closing brace.

```cpp
if (condition){
    DoOneThing();
    DoAnotherThing();
}
else if (int a = f();){
    DoAThirdThing(a);
}
else{
    DoNothing();
}
```

If only one statement depends on a condition (`if`, `else if` or `else`) it's Ok to omit the braces. Also, if the condition and the statement fit on the same line, they can be written together:
```cpp
if (condition){
    DoOneThing();
    DoAnotherThing();
}
else if (a != 3 && GetResultOfSomeVeryVeryLongFunctionCall(with, lots, of, arguments) == false)
    DoAThirdThing(a);
else DoNothing();
```

### Loops and Switch Statements

Use the same rules that for conditionals.

### Boolean Expressions

When you have a boolean expression that is longer than the maximum line length (100 characters), separate the expresion to a new line, indented with 2 characters. If even this is not enough, separate  the conditions in lines, breaking them before the operator

```cpp
if (
  this_one_thing > this_other_thing && a_third_thing == a_fourth_thing && yet_another
){
  ...
}

if (
  this_one_thing > this_other_thing && a_third_thing == a_fourth_thing
  && yet_another && last_one
){
  ...
}
```

### Return Values

Do not needlessly surround the `return` expression with parentheses.


### Variable and Array Initialization

You may choose between `=`, `()`, and `{}`; the following are all correct:
```cpp
int x = 3;
int x(3);
int x{3};
std::string name = "Some Name";
std::string name("Some Name");
std::string name{"Some Name"};
```


### Preprocessor Directives

The hash mark that starts a preprocessor directive should always be at the beginning of the line.

Even when preprocessor directives are within the body of indented code, the directives should start at the beginning of the line.

```cpp
if (lopsided_score) {
#if DISASTER\_PENDING      // Correct -- Starts at beginning of line
    DropEverything();
# if NOTIFY               // OK but not required -- Spaces after #
    NotifyClient();
# endif
#endif
    BackToNormal();
}

```

### Class Format

Sections in `public`, `protected` and `private` order, each indented four spaces. Leave an empty line before the `public`, `protected` and `private` keywords, and before each function or member.
```cpp
class MyClass: public OtherClass {

   public:
   
        MyClass();

        explicit MyClass(int var);

        ~MyClass() {}

        void SomeFunction();

    private:

        bool SomeInternalFunction();

        int some_var_;

};
```


### Horizontal Whitespace

Use of horizontal whitespace depends on location. Never put trailing whitespace at the end of a line.

#### General
```cpp
int i = 0;  // One or two spaces before end-of-line comments.

void f(bool b){  // Open braces should never have a space before them.
  ...
}

int i = 0;  // Semicolons have no space before them.

int x[] = {0};  // Don't use spaces inside braces or brackets.

// Spaces after (but not before) the colon in inheritance and initializer lists.
class Foo: public Bar {
    public:
        // For inline function implementations, put spaces between the braces
        // and the implementation itself.
        Foo(int b) : Bar(), baz\_(b){}
        void Reset(){baz_ = 0;}
  ...
```
Adding trailing whitespace can cause extra work for others editing the same file, when they merge, as can removing existing trailing whitespace. So: Don't introduce trailing whitespace.


#### Operators
```cpp
// All operators always have spaces around them.
x = 0;
v = w * x + y / z;
v = w\*x + y/z;
v = w * (x + z);
x = -5;
++ x;
if (x && !y){}
```
#### Templates and Casts
```cpp
// No spaces inside the angle brackets (< and >), before
// <, or between >( in a cast
std::vector<std::string> x;
y = static\_cast<char\*>(x);

// Spaces between type and pointer are OK, but be consistent.
std::vector<char \*> x;
```
-----------------------

## Credits

This file is derived from [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html "Google C++ Style Guide"). While some rules are completly different, the document structure, several paragraphs and many examples are original or closely derived.

-----------------------
