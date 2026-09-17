cmake_minimum_required(VERSION 3.20)
project(Oli_Nat C)

set(CMAKE_C_STANDARD 11)

add_executable(Oli_Nat
        src/main.c
        src/scanner.c
        src/debug.c
        src/ASTcompiler.c
        src/Bytecompiler.c
        src/vm.c
        src/Expr.c
        src/memory.c
        src/typeChecker.c
        src/chunk.c
        src/Value.c
        src/object.c
        src/Hashmap.c
        src/common.c
        src/natives.c
)

target_include_directories(Oli_Nat PRIVATE
        include
        src
)

target_link_libraries(Oli_Nat m)

target_compile_options(Oli_Nat PRIVATE -fsanitize=address -g)
target_link_options(Oli_Nat PRIVATE -fsanitize=address)