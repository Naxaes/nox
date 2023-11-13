

# Grammar

```
<module>        ::= <statement> | <statement> <module>

<block>         ::= "{" <statement> [<statement>] "}"

<statement>     ::= <fn-decl> | <expression> | <assignment>

<fn-decl>       ::= "fun" <identifier> "(" [<params>] ")" [<type>] <block>
<params>        ::= <identifier> ":" <type> ["," <params>]

<assignment>    ::= <identifier> "=" <expression>

<expression>    ::= <identifier> | <literal> | <call> | <binary>

<call>          ::= <identifier> "(" [<args>] ")"
<args>          ::= <expression> ["," <args>]

<binary>        ::= <expression> <operator> <expression>
<operator>      ::= "+" | "-" | "*" | "/" | "%" | "==" | "!=" | "<" | ">" | "<=" | ">="

<literal>       ::= <number> | <string> | <boolean>

<type>          ::= <identifier> | <primitive>
<primitive>     ::= "int" | "float" | "string" | "bool"

<identifier>    ::= <letter> | <identifier> <letter> | <identifier> <digit>

<number>        ::= <digit> | <number> <digit>
<string>        ::= '"' <char> '"' | '"' <char> <string> '"'
<boolean>       ::= "true" | "false"

<letter>        ::= "a" | "b" | ... | "z" | "A" | "B" | ... | "Z"
<digit>         ::= "0" | "1" | ... | "9"
```

The parser takes a stream of tokens and produces an untyped grammar tree.














