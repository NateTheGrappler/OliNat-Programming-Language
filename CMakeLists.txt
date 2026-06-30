cmake_minimum_required(VERSION 4.2)
project(Oli_Nat C)

set(CMAKE_C_STANDARD 11)

add_executable(Oli_Nat main.c
        common.h
        scanner.c
        scanner.h
        debug.c
        debug.h
        ASTcompiler.c
        ASTcompiler.h
        Bytecompiler.c
        Bytecompiler.h
        vm.c
        vm.h
        Expr.c
        Expr.h
        memory.c
        memory.h
        typeChecker.c
        typeChecker.h
        chunk.c
        chunk.h
        Value.c
        Value.h
        object.c
        object.h
        Hashmap.h
        Hashmap.c
        common.c
        natives.h
        natives.c
        hashmap_types.h
)
target_link_libraries(Oli_Nat m)

target_compile_options(Oli_Nat PRIVATE -fsanitize=address -g)
target_link_options(Oli_Nat PRIVATE -fsanitize=address)
