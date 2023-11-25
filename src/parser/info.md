

# Grammar

```
<module>        ::= <statement> | <statement> <module>

<block>         ::= "{" <statement> [<statement>] "}"

<statement>     ::= <block>             :: "{"
                  | <fn-decl>           :: "fun"
                  | <struct-decl>       :: "struct"
                  | <expression>        :: "a-z" | "A-Z" | "_" | "0-9" | "(" | '"' | "true" | "false"
                  | <assign>            :: "a-z" | "A-Z" | "_"
                  | <return>            :: "return"
                  | <break>             :: "break"
                  | <continue>          :: "continue"
                  | <import>            :: "import"
                    
<expression>    ::= <identifier>        :: "a-z" | "A-Z" | "_"
                  | <literal>           :: "0-9" | '"' | "true" | "false"
                  | <call>              :: "a-z" | "A-Z" | "_"
                  | <binary>            :: "a-z" | "A-Z" | "_"
                  | <if>                :: "if"
                  | <while>             :: "while"
                  | <for>               :: "for"


<fn-decl>       ::= "fun" <identifier> "(" [<params>] ")" [<type>] <block>
<params>        ::= <identifier> ":" <type> ["," <params>]

<assign>        ::= <identifier> "=" <expression>


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














