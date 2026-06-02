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
)
