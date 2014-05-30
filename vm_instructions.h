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
INSTRUCTION( EQ,              EQ,           19,         4,    input, input, input, litflag )
INSTRUCTION( LE,              LE,           20,         4,    input, input, input, litflag )
INSTRUCTION( LT,              LT,           21,         4,    input, input, input, litflag )
INSTRUCTION( JUMP,            JUMP,         22,         2,    input, litflag )
INSTRUCTION( PUSH,            PUSH,         23,         1,    input )
INSTRUCTION( POP,             POP,          24,         1,    input )
INSTRUCTION( LOAD,            LOAD,         25,         3,    output, input, litflag )
INSTRUCTION( CALL,            CALL,         26,         3,    input, input, litflag )
INSTRUCTION( RETURN,          RETURN,       27,         0     )
INSTRUCTION( REALLOC,         REALLOC,      28,         4,    output, input, input, litflag )
INSTRUCTION( FREE,            FREE,         29,         1,    input )
INSTRUCTION( MEMMOVE,         MEMMOVE,      30,         6,    input, input, input, input, input, litflag )
INSTRUCTION( LOGAND,          LOGAND,       31,         3,    output, input, input )
INSTRUCTION( LOGOR,           LOGOR,        32,         3,    output, input, input )
INSTRUCTION( TRAP,            TRAP,         33,         0     )
INSTRUCTION( SWAP,            SWAP,         34,         2,    output, output )
INSTRUCTION( MEMDUP,          MEMDUP,       35,         2,    output, input )
INSTRUCTION( MEMLEN,          MEMLEN,       36,         2,    output, input )
INSTRUCTION( PEEK,            PEEK,         37,         5,    output, input, input, input, litflag )  // r(dst), r(mem), lr(offset), lr(kind), litflag
INSTRUCTION( POKE,            POKE,         38,         5,    regonly, input, input, input, litflag ) // r(mem), lr(value), lr(offset), lr(kind), litflag
// END INSTRUCTIONS
