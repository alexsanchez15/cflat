<del>TODO: this

# C♭, a programming language.

Based on C language structure and a desire to understand how compiliers work at a better level. \
This project compilies to NASM x86_64, so it can only be ran on Linux systems. \
Written in C++.

## How to use

- pass in a .cfl extended file to the compiled executabe.
- This creates an ```output.asm``` file, to view the actual assembly generated.
- To run this, enter ```nasm -f elf64 output.asm -o program.o```
- Run the linker: ```ld program.o -o program```
- And run the program ```./program```

## What are the rules?

Syntatiacally, the language is very similar any C based language. \
The only supported types are ```int``` and ```string```, and strings are read only and immutable. \
Variables are defined as ```int x = 5```, or ```int x = <expression>```, like ```int x = 3 * 2 + 5```. This operation supports PEMDAS precedence. \
This includes binary comparision operations, like ```==, <, <=```. \
Variables can be set again once defined provided they are set to an appropriately typed expression.

## Built-in functions:
- ```exit(<int expression>)```
- ```print(<string expression)```
- ```to_str(<int expression>) -> string```
- ```if(<int expression>)```
- ```while(<int expression>)```

Because of the presence of conditional statements (if statements), repeated execution (while statements), and access to arbitrary memeory (variable use & reuse), this language is capable of arbitrary computation and is by defination Turing complete.

Example (from code in test.cfl from this repo): \
<img src="https://github.com/user-attachments/assets/ec81f927-eac3-42a4-9113-b49df11a2fd1" width="550">
, as a simple visual proof on concept.
