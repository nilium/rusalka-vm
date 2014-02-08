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
INSTRUCTION( ADD,             ADD,          1,          4,    output, input, input, litflag )
INSTRUCTION( SUB,             SUB,          2,          4,    output, input, input, litflag )
INSTRUCTION( DIV,             DIV,          3,          4,    output, input, input, litflag )
INSTRUCTION( IDIV,            IDIV,         4,          4,    output, input, input, litflag )
INSTRUCTION( MUL,             MUL,          5,          4,    output, input, input, litflag )
INSTRUCTION( MOD,             MOD,          6,          4,    output, input, input, litflag )
INSTRUCTION( IMOD,            IMOD,         7,          4,    output, input, input, litflag )
INSTRUCTION( NEG,             NEG,          8,          2,    output, input )
INSTRUCTION( NOT,             NOT,          9,          2,    output, input )
INSTRUCTION( OR,              OR,           10,         4,    output, input, input, litflag )
INSTRUCTION( AND,             AND,          11,         4,    output, input, input, litflag )
INSTRUCTION( XOR,             XOR,          12,         4,    output, input, input, litflag )
INSTRUCTION( ARITHSHIFT,      ASHIFT,       13,         4,    output, input, input, litflag )
INSTRUCTION( BITSHIFT,        BSHIFT,       14,         4,    output, input, input, litflag )
INSTRUCTION( FLOOR,           ITOU,         15,         2,    output, input )
INSTRUCTION( CEIL,            ITOF,         16,         2,    output, input )
INSTRUCTION( ROUND,           FTOU,         17,         2,    output, input )
INSTRUCTION( RINT,            FTOI,         18,         2,    output, input )
INSTRUCTION( CMP,             CMP,          19,         4,    output, input, input, litflag )
INSTRUCTION( JNE,             JNE,          20,         3,    input, input, litflag )
INSTRUCTION( JEQ,             JEQ,          21,         3,    input, input, litflag )
INSTRUCTION( JGTE,            JGTE,         22,         3,    input, input, litflag )
INSTRUCTION( JLTE,            JLTE,         23,         3,    input, input, litflag )
INSTRUCTION( JLT,             JLT,          24,         3,    input, input, litflag )
INSTRUCTION( JGT,             JGT,          25,         3,    input, input, litflag )
INSTRUCTION( JUMP,            JUMP,         26,         2,    input, litflag )
INSTRUCTION( STORE,           STORE,        27,         3,    input, input, litflag )
INSTRUCTION( GET,             GET,          28,         3,    output, input, litflag )
INSTRUCTION( PUSH,            PUSH,         29,         1,    input, litflag )
INSTRUCTION( POP,             POP,          30,         1,    input, litflag )
INSTRUCTION( LOAD,            LOAD,         31,         3,    output, input, litflag )
INSTRUCTION( CALL,            CALL,         32,         2,    input, input, litflag )
INSTRUCTION( RETURN,          RETURN,       33,         1,    input )
INSTRUCTION( ALLOC,           ALLOC,        34,         3,    output, input, litflag )
INSTRUCTION( FREE,            FREE,         35,         1,    input )
INSTRUCTION( PEEK8,           PEEK8,        36,         4,    output, input, input, litflag )
INSTRUCTION( PEEK16,          PEEK16,       37,         4,    output, input, input, litflag )
INSTRUCTION( PEEK32,          PEEK32,       38,         4,    output, input, input, litflag )
INSTRUCTION( POKE8,           POKE8,        39,         4,    input, input, input, litflag )
INSTRUCTION( POKE16,          POKE16,       40,         4,    input, input, input, litflag )
INSTRUCTION( POKE32,          POKE32,       41,         4,    input, input, input, litflag )
INSTRUCTION( MEMMOVE,         MEMMOVE,      42,         6,    input, input, input, input, input, litflag )
INSTRUCTION( LOGAND,          LOGAND,       43,         3,    output, input, input )
INSTRUCTION( LOGOR,           LOGOR,        44,         3,    output, input, input )
INSTRUCTION( TRAP,            TRAP,         45,         0     )
INSTRUCTION( SWAP,            SWAP,         46,         2,    output, output )
INSTRUCTION( MEMDUP,          MEMDUP,       47,         2,    output, input )
INSTRUCTION( MEMLEN,          MEMLEN,       48,         2,    output, input )
