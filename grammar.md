# C♭

$$
\begin{align}
\text{Prog} &\to  \text{Stmt} \\
\text{Stmt} &\to
\begin{cases}
\text{int}\space\text{ident} = \text{Expr}
\end{cases}
\end{align}
$$

## English Explaination

1. A program is a some amount of statements.
2. A Statement is:

    - Variable initialization, int, #string, float = [literal, expression, variable]
    - Variable reassignment = [literal, expression, variable]
    - print([string])
    - exit([expr]) // bounds by the syscall itself.

3. An expression is:

    - [term]
    - [binary-expression]

4. A binary expression is:
    - [expr] [+*-/] [expr]

5. A Literal can be any of

    - [int, string, float] (to be implemented in order int -> string -> float)

Binary Expressions always evaluate to a single term, which can be considered a literal.
