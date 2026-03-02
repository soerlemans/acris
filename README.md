Acris 𓆏
=======
![License](https://img.shields.io/github/license/soerlemans/acris)
![Tests](https://img.shields.io/github/actions/workflow/status/soerlemans/acris/.github/workflows/unit-tests.yml)

Home of the Acris programming language.
Named after the genus Acris which is the name of cricket frogs.

Acris syntax is somewhat inspired by Golang, Rust, Zig, Odin and C3 (I know its a lot).
In Acris expressions cannot be statements, expressions can only be used in context of a statement where they are required to produce a result.
This significantly helps with semicolon or end of statement inference:
```rust
let var = 10 + 10 // We know that the statement ends here, as there is no continuation operator and the next line will likely start with a keyword or similar.
```

Personally I am also not a big fan of writing ternary expressions like so:
```rust
let var = if(...) {
  // ...
} else {
  // ...
}
```

The main goal of the programming language is providing seamless interop with multiple programming languages.
Besides being able to select which backend you want to use for code generation:
 + LLVM (Partial implementation).
 + C++ (Generated from the AST).
 + C (not yet implemented).

It is also possible to select for which language you want your functions to be exported to:
 + Python (Works for C++ backend uses `pybind11`)
 + Lua (not yet implemented)
 + JavaScript (not yet implemented)
 + Common LISP (not yet implemented)

Here is a simple sample program.
```go
module main

// Standard library includes (#< is used for unhygienic macros):
#<include_once <core/core.ac> // <> are reserved for standard library.

#<ifdef NO_LIBC
// Will be excluded if not linking to libc.
#<else
// Will be included if linking to libc.
#<endif

// Comments like in C/C++.

/*
 * Multiline comments as in C/C++.
 */

func main() -> int {
  defer {
    println("Defer!")
  }

  println("Hello World!")

  return 0
}
```

Which can be compiled using:
```shell
acris --backend cpp --interop python samples/hello.ac
```
Which will create an importable Python DLL.

## Warning
Note that the project is not yet stable at all.
There are a list of language features implemented and its best to read through the `samples/`, for what is implemented.
Or to get an idea of the syntax.

## Dependencies
In order to compile the project you will need to following dependencies:

- C++ compiler with support for C++23.
- [Invoke](https://www.pyinvoke.org/) (Used to invoke CMake and scripts)
- [Cmake](https://cmake.org/)  (Main buildsysstem)
- [CLI11](https://github.com/CLIUtils/CLI11) (CLI option parsing library)
- [rang](https://github.com/agauniyal/rang/tree/master) (cross platform terminal coloring library)
- [tabulate](https://github.com/p-ranav/tabulate) (Text table library)
- [Boost](https://www.boost.org/) (Utility libraries for C++)
- [LLVM](https://llvm.org) (LLVM is used for code generation and optimization)
- [cereal](https://uscilab.github.io/cereal/) (Serialization library used for the AST)
- [libassert](https://github.com/jeremy-rifkin/libassert) (Modern assertion library for compile and runtime assertion checking)
- [pybind11](https://github.com/pybind/pybind11) (Used to generate C++ to Python bindings.)

### Build system
In order to compile the project you will need a couple of dependencies.
First we use invoke to invoke different python scripts, to orchestrate building the project.
In order to install invoke it is best to use pipx.
Which is installed using:
```shell
apt install -y pipx
```

In order to make use of invoke you should install it through `pipx`.
```shell
pipx install invoke
```

The you can invoke the appropriate setup script for your OS by running:
```shell
inv setup
```

If your OS is not supported it is best to checkout `tools/setup` and figure out what dependencies you require.

### Build
After installing the necessary dependencies you can build the compiler by running:
```
inv build --parallel
```

### Tests
To compile and run the tests compile the following:
```
inv build --mode=test-build --parallel
./test-build/acris-tests
```

## Project documentation

 - `assets/`: Non code related assets like images.
 - `cmake/`: Cmake sources that are needed to build the project.
 - `src/`: Sources of the Acris project.
 - `tests/`: Unit tests of the Acris project.
 - `samples/`: Acris source samples to learn and play around with.
 - `tools/`: Collection of tools and scripts, that aid development.
