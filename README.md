# Primitive compiler and X86 binary translator

## Getting Started

Only linux version.

### Installing

```
1) git clone https://github.com/exucutional/jcompil.git
2) make linux
```

## Running

There are three elf files:
1) comp.out
Creates jfiles/code.masm virtual machine code file
``` 
./comp.out <code file path>
```
2) cpurun.out
virtual machine code execution
```
./cpurun.out <code.masm file path>
```
3) jitrun.out
X86 translation and execution
```
./jitrun.out <code.masm file path>
```
## Code syntax
Common integer variable
```
simple x = 10|
```
Function (!!! Main function nessecary !!!)
```
simple func(simple arg1, ...):
	<code>
@
```
If block
```
? (<condition>):
	<code>
@|
```
Return value
```
out 10 + 10 * func(1)|
```

### Examples

Aсkermann function calculation programm
```
simple akk(simple m, simple n):
		? (m == 0):
				out n + 1|
		@|
		? (n == 0):
				out akk(m - 1, 1)
		@|
		out akk(m - 1, akk(m, n - 1))|
@

simple main():
		out akk(3, 11)|
@ 
```