## Camellia

A small (work in progress) C compiler.

This compiler is intended to follow the C89 specification defined in ANSI X3.159-1989.

<br/>

### Build

Build source with:

```
make -f makefile.main
```

Then run:

```
build/main
```

<br/>
<br/>

For running the .asm file directly:

```
nasm -f elf64 out/out.asm; ld out/out.o -o out/main; ./out/main
```


<br/>

### Testing

Build and run tests with:
```
make -f makefile.test
```


