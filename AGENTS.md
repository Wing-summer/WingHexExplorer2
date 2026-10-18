# Coding Guidelines

- In project-owned C++ code, including WingPlugin, always enclose the bodies of `if`, `else`, `if constexpr`, `for`, range-based `for`, `while`, and `do ... while` in `{}` blocks, even for a single statement or an empty body.
- Every conditional branch in an `else if` chain must also use `{}` blocks. Indentation or line breaks must not replace braces.
- Keep cleanup of existing code within the scope of the current changes. Do not perform bulk rewrites of unrelated code or third-party dependencies to enforce this rule.
- Use the `.clang-format` configuration applicable to the file. Both the project root and WingPlugin configurations enable `InsertBraces`.
