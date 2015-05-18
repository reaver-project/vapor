## Lexical structure of the language

This specification requires that the source code accepted by conforming Vapor compilers be, by default, encoded using
UTF-8. The grammar itself is specified over Unicode codepoints, so if an implementation wishes so, it can also accept
other encodings, but UTF-8 support by default is required.

The grammar is specified in a non-really-conforming variant of EBNF, where nonterminals are defined using "::="; it
should be obvious enough from its usage. Also the grammar description provided here is often simplified. This is the
consequence of the fact that the parser of the Vapor reference implementation is a hand-written recursive descent
parser.

### Basic lexical conventions

    digit ::= 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
    identifier_begin ::= any character in a-zA-Z_
    identifier_char ::= identifier_begin | digit
    whitespace ::= " " | "\t" | "\n" | "\r"

where "\t", "\n" and "\r" are U+0009, U+000A and U+000D.

### Comments

Vapor supports two kinds of comments - block comments and line comments.

    block_comment ::= "/*" (any character except "*")* "*/"
    line_comment ::= "//" (any character except "\n")* "\n"

### Keywords

The following is the list of all keywords or otherwise reserved identifiers in current version of Vapor:

    auto
    exports
    function
    import
    module
    return
    with

### Identifiers

All nonterminals in form of

    identifier ::= identifier_begin identifier_char*

that are not listed on the keyword list are identifiers.

### Literals

TODO

### Symbols

Symbols are too many to list them here right now. A complete list might be added here at one time, but at any time a
symbol is used below in the grammar, it's referenced by its terminal. Each symbol produces a separate type of tokens
during the lexing stage.
