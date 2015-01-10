## Modules

The primary way to structure Vapor code logically is to use modules. Modules map almost 1:1 to the physical structure
of source files, but not always.

### Importing a module

Modules are imported using an `import` statement or expression. The grammar for them is as follows:

    import-statement ::= import-expression ";"
    import-expression ::= "import" "."? id-expression

`id_expression` is a non-terminal that is often used within the language; its grammar is as follows:

    id_expression ::= identifier ("." identifier)*

For example, `foo`, `foo.bar` and `foo.bar.baz` are `id-expression`s.

`import-statement` can be used at any scope and follow the rules of that scope - so, when used at module scope, its
physical position in the file doesn't matter, but it does when they are used at any other scope. `import-expression`
follows all the rules of expression; it can be then assigned to a name, which can later be used to reference members
of the imported module. So, after

    auto other_module = import foo;

`other_module` refers to the scope of module `foo`.

`import-statement` brings all public symbols defined in imported module into current scope, and doesn't export them.
To explicitly export the import statement (so that the imported symbols are visible in the current scope from outside),
the `import-statement` should appear in the `exports` block (<<-- TODO. Need to figure out exact semantics, but the
idea is to make modules create and explicit block listing all symbols to be exported in a special block - the symbols
defined there would also be visible as "regular" members of the scope from the point of view of other members of the
module).

### Module names and their location

As mentioned previously, modules map mostly 1:1 to actual file system structure. When an import is requested, its
argument `id-expression` is transformed into a relative path by replacing all dots with host system's path separators
and by appending `.vprl`. Next, the following directories are searched for a matching file:

  1. If the `id-expression` is preceded by the (optional) dot, local directory (relative first to the source file
  being processed, then relative to working directory of compiler's invocation) is searched. If no match is found,
  the compiler reports and error and aborts further compilation.
  2. If the `id-expression` is not preceded by the dot, system-specific global directories are searched.
  3. If no match was found by 2., then directories passed to the compiler explicitly as "module directories" are
  searched.
  4. If no match was found by 3., then local directories are searched, in the order explained in 1.

### Defining modules

The grammar for module definition is as follows:

    module-definition ::= "module" id-expression "{" (declaration | import-statement | module-exports)* "}
    module-exports ::= "exports" "{" (declaration | import-statement)* "}

All symbols defined and imported outside an `exports` block are private to the module. All symbols defined and imported
inside an `exports` block are visible as members of the module from the outside.

The order of definitions and imports at module scope is not important. That is, the following code at module scope is
correct:

    function f() => some_type{};
    auto some_type = struct {};

### Multi-file modules

Sometimes (although it's usually discouraged) a single module contains many different types and functions, and their
code can take significant number of source code lines. To avoid that, you usually want to split such a module definition
into multiple files. This is where another form of the `import-statement` (but not import expression) come into play:

    import-statement ::= "import" string-literal ";"

This form of the `import-statement` includes file named by the `string-literal`, relatively to the location of the
currently processed file, in the module definition. That file is required to contain a `module-definition` of the module
where this form of `import-statement` was used; the two definitions are later merged. Such a file can also contain
other `module-definition`s; those are ignored (which makes it possible to include such a file in two different modules).

This form of the `import-statement` is only allowed at module scope (and not even at an `exports` scope).

### Automatically importing all submodules

This section is not strictly a part of the specification, more of a guideline. If you want to create a module that
automatically makes its submodules accessible, consider using the following pattern:

    module foo
    {
        exports
        {
            auto bar = import foo.bar;
            auto baz = import foo.baz;
        }
    }
