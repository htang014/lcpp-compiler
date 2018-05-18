# lcpp-compiler
A compiler from a toy "lcpp" language to LC3 assembly code.

## Installation
Ensure that you have the latest versions of git, GCC, flex, and bison.

In bash, perform the following:
```
$ git clone http://github.com/htang014/lcpp-compiler.git
$ cd lcpp-compiler
$ git checkout lc3c
$ make

```
## Usage
### Syntax
* All instructions should be typed inside a function.
* Your first function should be main.
* Instructions are terminated with newlines.

### Functions
To declare a function:
```
int function ( ){

        return 0
}
```
or
```
int function (int x, int y){

        return x
}
```
You must return a variable or an integer.
Your first function must be called main, and have no parameters:
```
int main ( ){

        return 0
}
```
Variables go out of scope once the function ends.

### Variable declaration
To declare a variable:
```
int var
```
Currently, only int variables are supported.
Furthermore, you may have up to 7 variables in any given function.

You may initialize a variable to an integer, otherwise it will initialize to 0.
```
int var = 15
```

### Variable assignment
To reassign a variable:
```
var = 30
```
Obviously this may only be done to a variable already declared.
You can also assign a variable to another variable, a function call, or an arithmetic expression (more on this later).
```
var = var2
```
```
var = function(x,y)
```
```
var = x + y
```

### Arithmetic
Currently, only addition is supported, and you may only have two terms.
The expression must be on the right, and the first term must be a variable.
The second term may be a variable or an integer
```
var = x + 10
```
```
var = x + y
```
```
var = x + -10
```

### If/Dowhile statements
If and dowhile operations are supported.
Note that variables declared inside do not go out of scope until the function ends.
The conditional must be in the form (VAR OPER VAR) or (VAR OPER INT).
For instance:
```
if (x > y) {
        x = y
        y = 10
}
```
```
dowhile (x <= 10) {
        x = x + 1
}
```
Currently <=, >=, <, >, and == comparators are supported.

### I/O
To output a string, type:
```
out("string")
```
To output an ascii character, assign its decimal value to an variable and type:
```
out(var)
```
To input a character from the keyboard, type:
```
in(var)
```
This will store the ascii value of the character you typed into the variable `var`

### Compiling
Type:
```
bin/lc3c [filename]
```
to compile to standard output, or
```
bin/lc3c [filename] > [outfile]
```
to redirect output

## Future checklist
* Add proper type-checking
* Add better arithmetic capability
* Improve I/O functionality
* Implement conditional connectors (&&, ||, !)
* !=
* else if, else, while, for
* Pointers
* Fix various edge case bugs
