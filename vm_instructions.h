/*
  Copyright (c) 2013 Noel Raymond Cower.

  This file is part of Rusalka VM.

  Rusalka VM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Rusalka VM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Rusalka VM.  If not, see <http://www.gnu.org/licenses/>.
*/

// BEGIN INSTRUCTIONS
INSTRUCTION( ADD,             ADD,           0,         4,    output, input, input, litflag )
INSTRUCTION( SUB,             SUB,           1,         4,    output, input, input, litflag )
INSTRUCTION( DIV,             DIV,           2,         4,    output, input, input, litflag )
INSTRUCTION( IDIV,            IDIV,          3,         4,    output, input, input, litflag )
INSTRUCTION( MUL,             MUL,           4,         4,    output, input, input, litflag )
INSTRUCTION( POW,             POW,           5,         4,    output, input, input, litflag )
INSTRUCTION( MOD,             MOD,           6,         4,    output, input, input, litflag )
INSTRUCTION( IMOD,            IMOD,          7,         4,    output, input, input, litflag )
INSTRUCTION( NEG,             NEG,           8,         2,    output, input )
INSTRUCTION( NOT,             NOT,           9,         2,    output, input )
INSTRUCTION( OR,              OR,           10,         4,    output, input, input, litflag )
INSTRUCTION( AND,             AND,          11,         4,    output, input, input, litflag )
INSTRUCTION( XOR,             XOR,          12,         4,    output, input, input, litflag )
INSTRUCTION( ARITHSHIFT,      ASHIFT,       13,         4,    output, input, input, litflag )
INSTRUCTION( BITSHIFT,        BSHIFT,       14,         4,    output, input, input, litflag )
INSTRUCTION( FLOOR,           FLOOR,        15,         2,    output, input )
INSTRUCTION( CEIL,            CEIL,         16,         2,    output, input )
INSTRUCTION( ROUND,           ROUND,        17,         2,    output, input )
INSTRUCTION( RINT,            RINT,         18,         2,    output, input )
INSTRUCTION( CMP,             CMP,          19,         4,    output, input, input, litflag )
INSTRUCTION( JNZ,             JNZ,          20,         3,    input, input, litflag )
INSTRUCTION( JEZ,             JEZ,          21,         3,    input, input, litflag )
INSTRUCTION( JGEZ,            JGEZ,         22,         3,    input, input, litflag )
INSTRUCTION( JLEZ,            JLEZ,         23,         3,    input, input, litflag )
INSTRUCTION( JLTZ,            JLTZ,         24,         3,    input, input, litflag )
INSTRUCTION( JGTZ,            JGTZ,         25,         3,    input, input, litflag )
INSTRUCTION( JUMP,            JUMP,         26,         2,    input, litflag )
INSTRUCTION( PUSH,            PUSH,         27,         1,    input )
INSTRUCTION( POP,             POP,          28,         1,    input )
INSTRUCTION( LOAD,            LOAD,         29,         3,    output, input, litflag )
INSTRUCTION( CALL,            CALL,         30,         3,    input, input, litflag )
INSTRUCTION( RETURN,          RETURN,       31,         0     )
INSTRUCTION( REALLOC,         REALLOC,      32,         4,    output, input, input, litflag )
INSTRUCTION( FREE,            FREE,         33,         1,    input )
INSTRUCTION( MEMMOVE,         MEMMOVE,      34,         6,    input, input, input, input, input, litflag )
INSTRUCTION( LOGAND,          LOGAND,       35,         3,    output, input, input )
INSTRUCTION( LOGOR,           LOGOR,        36,         3,    output, input, input )
INSTRUCTION( TRAP,            TRAP,         37,         0     )
INSTRUCTION( SWAP,            SWAP,         38,         2,    output, output )
INSTRUCTION( MEMDUP,          MEMDUP,       39,         2,    output, input )
INSTRUCTION( MEMLEN,          MEMLEN,       40,         2,    output, input )
INSTRUCTION( PEEK,            PEEK,         41,         5,    output, input, input, input, litflat )  // r(dst), r(mem), lr(offset), lr(kind), litflag
INSTRUCTION( POKE,            POKE,         42,         5,    input, input, input, litflag ) // r(mem), lr(value), lr(offset), lr(kind), litflag
// END INSTRUCTIONS
