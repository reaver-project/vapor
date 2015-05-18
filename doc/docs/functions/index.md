## Functions

Functions in Vapor are first-class citizens coming in two variants - non-overloadable function objects and overloadable
proper functions.

### Function objects

Function objects, or lambdas (when not capturing anything) or closures (when the capture list is not empty) allow to
create unnamed functions in almost any context, and since they are not overloadable, you can always unambiguously pass
them to generic functions.

    lambda-expression ::= "[" capture-list? "]" ("(" argument-list? ")")? block
    block ::= single-statement-block | ("{" statement* block-value? "}")
    block-value ::= "=>" expression
    single-statement-block ::= block-value

TODO: decide on details of `capture-list` and `argument-list`.

### Overloadable functions

Overloadable functions are "regular" functions. All functions with the same name in the selected scope together create
an *overload set*.

    function-definition ::= "function" identifier "(" argument-list? ")" function-block
    function-block ::= (single-statement-block ";") | ("{" statement* block-value? "}")
