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
- Global and local variable declarations and assignment
- Lexical scoping with block statements
- If/else control flow
- String concatenation and O(1) string equality via interning
- Arithmetic and comparison operators across all numeric types

### Syntax Example

```
make int x = 10;
make string greeting = "hello";

if (x > 5) {
    make int y = x + 1;
    y;
}
```

## Architecture

### Pipeline Stages

**Scanner** (`scanner.h/c`) — lazy token-at-a-time scanning, O(1) memory. Keyword recognition via a trie-style switch on the first character.

**Pratt Parser** (`ASTcompiler.h/c`) — produces an AST from source. Each token type has an associated prefix and infix parse function with a precedence level, enabling clean handling of unary/binary/grouping expressions.

**AST** (`Expr.h/c`) — discriminated union with anonymous union fields. Node types: `EXPR_LITERAL`, `EXPR_BINARY`, `EXPR_UNARY`, `EXPR_GROUPING`, `EXPR_VARIABLE`, `EXPR_ASSIGN`.

**Type Checker** (`typeChecker.h/c`) — runs after parsing, before bytecode emission. Maintains a flat array symbol table mapping variable names to declared types and scope depths. Rejects type mismatches, undeclared variables, and invalid operations at compile time.

**Bytecode Compiler** (`Bytecompiler.h/c`) — walks the AST recursively and emits bytecode into a `Chunk`. Resolves variable references to stack slots (locals) or global name constants at compile time.

**VM** (`vm.h/c`) — stack-based interpreter. Single tagged-value stack. Dispatch loop over opcodes with type-promoted arithmetic and direct stack slot access for locals.

### Key Data Structures

**Value** — tagged union:
```c
typedef struct {
    ValueType type;
    union { bool boolean; int int_val; float float_val;
            double double_val; Obj* object_val; } as;
} Value;
```

**ObjString** — heap string with cached FNV-1a hash:
```c
typedef struct ObjString {
    Obj obj;        // must be first — enables safe Obj* ↔ ObjString* casting
    int length;
    char* chars;
    uint32_t hash;
} ObjString;
```

**Hashmap** — open addressing with linear probing and tombstone deletion. Used for both the string intern table and the global variables table.

**Chunk** — bytecode buffer with a parallel constants array and line info.

## Design Decisions

**String interning** — all strings are deduplicated on creation via FNV-1a hashing. Identical strings are guaranteed to share the same pointer, so equality is a single pointer comparison rather than O(n) `strcmp`.

**Static typing eliminates runtime type guards** — the `IS_` macros serve as dispatch tools in the VM, not safety checks. The type checker has already rejected any program that would reach an invalid type combination at runtime.

**Locals are stack slots, not named variables** — declaring a local variable just pushes its initializer value onto the stack. The compiler tracks which stack slot each name maps to. No `OP_DEFINE_LOCAL` opcode is needed.

**Flat symbol table** — the type checker uses a simple array of `Symbol` structs scanned backwards (so inner scopes shadow outer ones). Symbols are popped in sync with the compiler's locals array when a scope ends.

**Patchable jumps** — `emitJump` writes a placeholder two-byte operand and returns its offset. `patchJump` backfills the real offset once the jump target is known. Used for if/else control flow.

**Single-stack VM** — simpler than multistack; the static type system means the tag overhead is negligible and no expensive runtime validation is needed.

**Forward declarations break circular includes** — `ValueType` lives in `common.h`, `Obj` is forward-declared in `value.h`, and `chunk.h` forward-declares `Vm` to avoid the `chunk.h` ↔ `vm.h` cycle.

## Opcodes

| Opcode                       | Description                                   |
|------------------------------|-----------------------------------------------|
| `OP_CONSTANT`                | Push constant onto stack                      |
| `OP_ADD / SUB / MUL / DIV`   | Arithmetic with type promotion                |
| `OP_NEGATE / INVERSE`        | Unary minus and boolean not                   |
| `OP_EQUAL / NOT_EQUAL`       | Equality (pointer comparison for strings)     |
| `OP_GREATER / LESS / _EQUAL` | Numeric comparisons                           |
| `OP_DEFINE_GLOBAL`           | Pop value, store in globals hashmap           |
| `OP_GET_GLOBAL / SET_GLOBAL` | Hashmap lookup/update by interned name        |
| `OP_GET_LOCAL / SET_LOCAL`   | Direct stack slot access by index             |
| `OP_JUMP`                    | Unconditional forward jump                    |
| `OP_JUMP_IF_FALSE`           | Conditional jump, leaves condition on stack   |
| `OP_LOOP`                    | Jumps the vm pointer backwards to repeat code |
| `OP_POP`                     | Discard top of stack                          |
| `OP_RETURN`                  | End execution                                 |

## Building

```bash
mkdir cmake-build-debug
cd cmake-build-debug
cmake ..
make
./Oli_Nat source.oli
```

Enable debug tracing by defining `DEBUG_TRACE_EXECUTION` to print the stack state and disassembled instruction before each opcode.

## Roadmap

- `while` and `for` loops
- Standard library
- Possibly a graphics library
- Functions and return values
- Empty/null semantics
- Classes and instances
- Garbage collector