# Oli-Nat

A statically-typed bytecode VM language built in C from scratch.

## Overview

Oli-Nat compiles source code through a full pipeline into bytecode executed by a custom stack-based virtual machine. Every stage is hand-written with no external dependencies.

```
source → scanner → Pratt parser → AST → type checker → bytecode compiler → VM
```

## Language Features

- Static typing with explicit type annotations
- Integer, float, double, boolean, and string primitives
- Static arrays with type inference
- Global and local variable declarations with compound assignment operators (`+=`, `-=`, `*=`, `/=`, `++`, `--`)
- Lexical scoping with block statements
- If/else, while, and for control flow
- First-class functions with closures and upvalue capture
- Classes with typed fields, default values, and methods
- Class instance creation and field access via dot notation
- String concatenation and O(1) string equality via interning
- Arithmetic and comparison operators with numeric type promotion
- Standard library via `#pullf` imports

### Syntax Example

```
#pullf io

class Player
{
    make int health = 100;
    make string name = "hero";

    make empty takeDamage(int amount)
    {
        health = health - amount;
    }
}

make Player p = Player();
p.health = p.health - 10;
println(p.health);

make int x = 10;
make string greeting = "hello";

if (x > 5)
{
    make int y = x + 1;
    println(y);
}

for (make int i = 0; i < 5; i++)
{
    println(i);
}
```

## Standard Library

Imported via `#pullf <library>`. Available libraries:

| Library     | Contents                                              |
|-------------|-------------------------------------------------------|
| `io`        | `print`, `println`, `intake`                          |
| `math`      | `sin`, `cos`, `tan`, `sqrt`, `pow`, `floor`, `ceil`, `abs`, `ln`, `log10`, `log2`, `expo` |
| `chronos`   | `clock`, `time`, `sleep`, `dateString`, `timeString`  |
| `fileIO`    | `readFile`, `writeFile`, `appendFile`, `fileExists`, `deleteFile` |
| `types`     | `intToStr`, `intToDouble`, `intToFloat`, `doubleToStr`, `doubleToInt`, `doubleToFloat`, `floatToStr`, `floatToInt`, `floatToDouble`, `strToInt`, `strToDouble`, `strToFloat`, `strToBool`, `boolToStr` |
| `Strings`   | `strLength`, `strContains`, `strSlice`, `strToUpper`, `strToLower`, `strReplace` |
| `utils`     | `length`, `assert`                                    |
| `stdlib`    | All of the above                                      |

## Architecture

### Pipeline Stages

**Scanner** (`scanner.h/c`) — lazy token-at-a-time scanning, O(1) memory. Keyword recognition via a trie-style switch on the first character.

**Pratt Parser** (`ASTcompiler.h/c`) — two-pass compiler. The first pass pre-registers all function and class declarations so forward references and mutual recursion work correctly. The second pass produces a full AST and emits bytecode. Each token type has an associated prefix and infix parse function with a precedence level.

**AST** (`Expr.h/c`) — discriminated union with anonymous union fields. Node types cover literals, binary/unary expressions, variables, assignments, function calls, array operations, logical operators, and field get/set expressions.

**Type Checker** (`typeChecker.h/c`) — runs during the second pass, before bytecode emission. Maintains a flat symbol table mapping variable names to declared types, scope depths, and function signatures. For class symbols, field metadata is stored as a heap-allocated array of `CheckerFieldInfo` structs populated during the first pass. Rejects type mismatches, undeclared variables, invalid field accesses, and bad function call signatures at compile time.

**Bytecode Compiler** (`Bytecompiler.h/c`) — walks the AST recursively and emits bytecode into a `Chunk`. Resolves variable references to stack slots (locals), upvalues (captured locals), or global name constants at compile time.

**VM** (`vm.h/c`) — stack-based interpreter with a call frame stack supporting nested function calls and closures. Dispatch loop over opcodes with type-promoted arithmetic and direct stack slot access for locals.

**Garbage Collector** (`memory.h/c`) — tri-color mark-and-sweep GC. Objects are linked in a VM-owned intrusive list. The GC traces roots from the value stack, call frames, open upvalues, and the globals table. Class objects mark their methods hashmap and field default values; instances mark their class and field value array.

### Key Data Structures

**Value** — tagged union:
```c
typedef struct {
    ValueType type;
    union { bool boolean_val; int integer_val; float float_val;
            double double_val; Obj* object_val; } as;
} Value;
```

**ObjString** — heap string with cached FNV-1a hash for O(1) interned equality.

**ObjFunction / ObjClosure** — functions carry their bytecode chunk, arity, parameter type info, and return type. Closures wrap a function with a captured upvalue array.

**ObjClass** — holds the class name, a fixed-size `FieldInfo` array with per-field type, name, and default value, a field count, and a hashmap of methods.

**ObjInstance** — holds a pointer to its class and a heap-allocated `Value` array for field storage, initialized from the class's default values at instantiation.

**Hashmap** — open addressing with linear probing and tombstone deletion. Used for the string intern table, global variables, and class method tables.

**Chunk** — bytecode buffer with a parallel constants array and line info.

### Memory Model

All heap objects are allocated through a central `reallocate` function that tracks total bytes and triggers garbage collection when a growth threshold is crossed. The GC threshold grows by a configurable factor after each collection. The gray stack used during marking is allocated separately with raw `realloc` to avoid re-entrancy issues.

## Design Decisions

**String interning** — all strings are deduplicated on creation via FNV-1a hashing. Identical strings share the same pointer, so equality is a single pointer comparison. This also makes field name lookup in `OP_GET_FIELD` and `OP_SET_FIELD` an O(1) pointer comparison rather than `memcmp`.

**Static typing eliminates runtime type guards** — the type checker rejects any program that would reach an invalid type combination at runtime. The `IS_` macros in the VM serve as dispatch tools, not safety checks.

**Locals are stack slots, not named variables** — declaring a local variable pushes its initializer value onto the stack. The compiler tracks which stack slot each name maps to. No `OP_DEFINE_LOCAL` opcode is needed.

**Two-pass compilation** — the first pass scans for function and class declarations and registers their signatures in the type checker's symbol table. The second pass can then type-check calls and field accesses against those signatures without requiring forward declarations in source code.

**Class fields use slot indices** — at compile time each field is assigned a slot index. At runtime `ObjInstance.fields` is a plain `Value` array indexed by slot. Field name lookup only happens at class definition time via `OP_CLASS_FIELD`; at runtime `OP_GET_FIELD` and `OP_SET_FIELD` use interned string pointer comparison to find the right slot.

**Flat symbol table** — the type checker uses a simple array of `Symbol` structs scanned backwards so inner scopes shadow outer ones. Symbols are popped in sync with the compiler's locals array when a scope ends.

**Patchable jumps** — `emitJump` writes a placeholder two-byte operand and returns its offset. `patchJump` backfills the real offset once the jump target is known.

## Opcodes

| Opcode                        | Description                                          |
|-------------------------------|------------------------------------------------------|
| `OP_CONSTANT / _LONG`         | Push constant onto stack                             |
| `OP_ADD / SUB / MUL / DIV`    | Arithmetic with numeric type promotion               |
| `OP_NEGATE / INVERSE`         | Unary minus and boolean not                          |
| `OP_EQUAL / NOT_EQUAL`        | Equality (pointer comparison for strings)            |
| `OP_GREATER / LESS / _EQUAL`  | Numeric comparisons                                  |
| `OP_DEFINE_GLOBAL`            | Pop value, store in globals hashmap                  |
| `OP_GET_GLOBAL / SET_GLOBAL`  | Hashmap lookup and update by interned name           |
| `OP_GET_LOCAL / SET_LOCAL`    | Direct stack slot access by index                    |
| `OP_GET_UPVALUE / SET_UPVALUE`| Access captured variables through closure            |
| `OP_CLOSE_UPVALUE`            | Move upvalue from stack to heap on scope exit        |
| `OP_JUMP`                     | Unconditional forward jump                           |
| `OP_JUMP_IF_FALSE`            | Conditional jump, leaves condition on stack          |
| `OP_LOOP`                     | Jump backwards to repeat a loop body                 |
| `OP_CALL`                     | Call a closure or native function                    |
| `OP_CLOSURE`                  | Wrap a function in a closure with upvalue bindings   |
| `OP_RETURN`                   | Return from function, restore call frame             |
| `OP_MISSING_RETURN`           | Runtime error for non-void functions without return  |
| `OP_CREATE_ARRAY`             | Collect N stack values into a static array object    |
| `OP_GET_ARRAY_INDEX`          | Index into an array                                  |
| `OP_SET_ARRAY_INDEX`          | Assign to an array element                           |
| `OP_CLASS`                    | Create a class object and push onto stack            |
| `OP_CLASS_FIELD`              | Attach a field with default value to a class         |
| `OP_CLASS_METHOD`             | Attach a method closure to a class                   |
| `OP_FIELD_DEFAULT`            | Push a zero default value for a given type           |
| `OP_GET_FIELD`                | Get a field or method from an instance               |
| `OP_SET_FIELD`                | Set a field on an instance with type checking        |
| `OP_POP`                      | Discard top of stack                                 |

## Building

```bash
mkdir cmake-build-debug
cd cmake-build-debug
cmake ..
make
./Oli_Nat source.oli
```

Enable debug tracing by defining `DEBUG_TRACE_EXECUTION` to print the stack state and disassembled instruction before each opcode. Define `DEBUG_LOG_GC` to trace garbage collection events.

## Roadmap

- Method calls with `this` binding
- Inheritance and method dispatch up the class hierarchy
- Constructors with parameters
- Possibly a graphics library