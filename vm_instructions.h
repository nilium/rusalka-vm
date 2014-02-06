#ifndef VM_LICENSE
#define VM_LICENSE(ARGS...)
#endif
VM_LICENSE(
 "Copyright (c) 2013 Noel Raymond Cower.",
 "",
 "This file is part of Rusalka VM.",
 "",
 "Rusalka VM is free software: you can redistribute it and/or modify",
" it under the terms of the GNU General Public License as published by",
" the Free Software Foundation, either version 3 of the License, or",
" (at your option) any later version.",
 "",
 "Rusalka VM is distributed in the hope that it will be useful,",
" but WITHOUT ANY WARRANTY; without even the implied warranty of",
" MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the",
" GNU General Public License for more details.",
 "",
 "You should have received a copy of the GNU General Public License",
" along with Rusalka VM.  If not, see <http://www.gnu.org/licenses/>."
)

INSTRUCTION( NOP,             NOP,          0,          0     )
INSTRUCTION( ADD,             ADD,          1,          4,    reg(num, output), reg(num, input), reg(num, input), num )
INSTRUCTION( SUB,             SUB,          2,          4,    reg(num, output), reg(num, input), reg(num, input), num )
INSTRUCTION( DIV,             DIV,          3,          4,    reg(num, output), reg(num, input), reg(num, input), num )
INSTRUCTION( IDIV,            DIV,          4,          4,    reg(num, output), reg(num, input), reg(num, input), num )
INSTRUCTION( MUL,             MUL,          5,          4,    reg(num, output), reg(num, input), reg(num, input), num )
INSTRUCTION( MOD,             MOD,          6,          4,    reg(num, output), reg(num, input), reg(num, input), num )
INSTRUCTION( IMOD,            IMOD,         7,          4,    reg(num, output), reg(num, input), reg(num, input), num )
INSTRUCTION( NEG,             NEG,          8,          2,    reg(num, output), reg(num, input) )
INSTRUCTION( NOT,             NOT,          9,          2,    reg(num, output), reg(num, input) )
INSTRUCTION( OR,              OR,           10,         4,    reg(num, output), reg(num, input), reg(num, input), num )
INSTRUCTION( AND,             AND,          11,         4,    reg(num, output), reg(num, input), reg(num, input), num )
INSTRUCTION( XOR,             XOR,          12,         4,    reg(num, output), reg(num, input), reg(num, input), num )
INSTRUCTION( ARITHSHIFT,      ASHIFT,       13,         4,    reg(num, output), reg(num, input), reg(num, input), num )
INSTRUCTION( BITSHIFT,        BSHIFT,       14,         4,    reg(num, output), reg(num, input), reg(num, input), num )
INSTRUCTION( FLOOR,           ITOU,         15,         2,    reg(num, output), reg(num, input) )
INSTRUCTION( CEIL,            ITOF,         16,         2,    reg(num, output), reg(num, input) )
INSTRUCTION( ROUND,           FTOU,         17,         2,    reg(num, output), reg(num, input) )
INSTRUCTION( RINT,            FTOI,         18,         2,    reg(num, output), reg(num, input) )
INSTRUCTION( CMP,             CMP,          19,         4,    reg(num, output), reg(num, input), reg(num, input), num )
INSTRUCTION( JNE,             JNE,          20,         3,    reg(num, input), reg(num, input), num )
INSTRUCTION( JEQ,             JEQ,          21,         3,    reg(num, input), reg(num, input), num )
INSTRUCTION( JGTE,            JGTE,         22,         3,    reg(num, input), reg(num, input), num )
INSTRUCTION( JLTE,            JLTE,         23,         3,    reg(num, input), reg(num, input), num )
INSTRUCTION( JLT,             JLT,          24,         3,    reg(num, input), reg(num, input), num )
INSTRUCTION( JGT,             JGT,          25,         3,    reg(num, input), reg(num, input), num )
INSTRUCTION( JUMP,            JUMP,         26,         2,    reg(num, input), num )
INSTRUCTION( STORE,           STORE,        27,         3,    reg(num, input), reg(value, input), num )
INSTRUCTION( GET,             GET,          28,         3,    reg(value, output), reg(num, input), num )
INSTRUCTION( PUSH,            PUSH,         29,         1,    num, num )
INSTRUCTION( POP,             POP,          30,         1,    num, num )
INSTRUCTION( LOAD,            LOAD,         31,         2,    reg(value, output), value )
INSTRUCTION( CALL,            CALL,         32,         2,    num, num, num )
INSTRUCTION( RETURN,          RETURN,       33,         1,    reg(value, input) )
INSTRUCTION( ALLOC,           ALLOC,        34,         3,    reg(num, output), reg(num, input), num )
INSTRUCTION( FREE,            FREE,         35,         1,    reg(num, input) )
INSTRUCTION( PEEK8,           PEEK8,        36,         4,    reg(value, output), reg(num, input), reg(num, input), num )
INSTRUCTION( PEEK16,          PEEK16,       37,         4,    reg(value, output), reg(num, input), reg(num, input), num )
INSTRUCTION( PEEK32,          PEEK32,       38,         4,    reg(value, output), reg(num, input), reg(num, input), num )
INSTRUCTION( POKE8,           POKE8,        39,         4,    reg(num, input), reg(num, input), reg(value, input), num )
INSTRUCTION( POKE16,          POKE16,       40,         4,    reg(num, input), reg(num, input), reg(value, input), num )
INSTRUCTION( POKE32,          POKE32,       41,         4,    reg(num, input), reg(num, input), reg(value, input), num )
INSTRUCTION( MEMMOVE,         MEMMOVE,      42,         6,    reg(num, input), reg(num, input), reg(num, input), reg(num, input), reg(num, input), reg(num, input) )
INSTRUCTION( LOGAND,          LOGAND,       43,         3,    reg(value, output), reg(value, input), reg(value, input) )
INSTRUCTION( LOGOR,           LOGOR,        44,         3,    reg(value, output), reg(value, input), reg(value, input) )
INSTRUCTION( TRAP,            TRAP,         45,         0     )
INSTRUCTION( SWAP,            SWAP,         46,         2,    reg(value, inout), reg(value, inout) )
INSTRUCTION( MEMDUP,          MEMDUP,       47,         2,    reg(num, output), reg(num, input) )
INSTRUCTION( MEMLEN,          MEMLEN,       48,         2,    reg(num, output), reg(num, input) )
