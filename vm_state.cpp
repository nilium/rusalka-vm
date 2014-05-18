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

#include <cfenv>
#include <cmath>
#include <iostream>
#include <iomanip>

#define VM_KEEP_DATA_MACROS
#include "vm_state.h"


#define VM_FCMP_EPSILON (1.0e-12)

#define VM_NONVOLATILE_REGISTERS 8


enum memop_typed_t : int32_t
{
  MEMOP_UINT8   = 0,          // Cast to uint8_t
  MEMOP_INT8    = 1,          // Cast to int8_t
  MEMOP_UINT16  = 2,          // Cast to uint16_t
  MEMOP_INT16   = 3,          // Cast to int16_t
  MEMOP_UINT32  = 4,          // Cast to uint32_t
  MEMOP_INT32   = 5,          // Cast to int32_t
  MEMOP_UINT64  = 6,          // Cast to uint64_t
  MEMOP_INT64   = 7,          // Cast to int64_t
  MEMOP_FLOAT32 = 8,          // Cast to float
  MEMOP_FLOAT64 = 9,          // Copied as double (no change)
};


static
bool
vm_fequals(double lhs, double rhs)
{
  return std::abs(lhs - rhs) < VM_FCMP_EPSILON;
}


/*
  TODO: handle proper stack unwinding on trap somehow. Currently, it can really
  screw with the VM if there are nested run() calls (e.g., an imported function
  calls back into the vm before returning).

  Could be done by throwing an object that the VM can catch and use to unwind.
  Maybe a counter that's decremented and re-thrown to handle each nesting.

  Not sure if it would be possible to then continue execution normally. I
  haven't decided yet if trap should be the equivalent of yielding to the host
  or if it should be for signalling that the VM cannot continue to run due to
  some error. In the latter case, it might be a good idea to completely reset
  the stack and all registers and then return the trapped value.

  Don't know -- for now, the TRAP instruction is iffy and kind of just grinds
  the VM to a halt and makes it hard to recover.
*/


#ifdef VM_PRESERVE_PRESERVE_ALL_REGISTERS_ON_CALL
// Preserve all registers but rp (as it's overwritten by the returned value anyway).
#define CALL_STACK_MASK (~0x8)
#ifdef VM_PRESERVE_CALL_ARGUMENT_REGISTERS
#undef VM_PRESERVE_CALL_ARGUMENT_REGISTERS
#endif
#else
// Only preserve registers 0 through 12. rp shouldn't be preserved.
#define CALL_STACK_MASK (0xFF7)
#endif


static constexpr uint32_t cndmask[2] = { 0x0u, ~0x0u };


static constexpr uint32_t arg_masks[32] = {
  0x00000000, 0x00000010, 0x00000030, 0x00000070,
  0x000000f0, 0x000001f0, 0x000003f0, 0x000007f0,
  0x00000ff0, 0x00001ff0, 0x00003ff0, 0x00007ff0,
  0x0000fff0, 0x0001fff0, 0x0003fff0, 0x0007fff0,
  0x000ffff0, 0x001ffff0, 0x003ffff0, 0x007ffff0,
  0x00fffff0, 0x01fffff0, 0x03fffff0, 0x07fffff0,
  0x0ffffff0, 0x1ffffff0, 0x3ffffff0, 0x7ffffff0,
  0xfffffff0, 0xfffffff0, 0xfffffff0, 0xfffffff0
};


template <typename T>
static
T
byte_bit_counts(int32_t idx) {
  static constexpr T bits_array[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3,
    3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4,
    3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4,
    4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5,
    3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, 2,
    2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5,
    4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5,
    5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5,
    5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
  };
  return bits_array[idx];
}


#if 0
inline static uint32_t count_bits_in_byte(uint8_t num8) {
  switch (num8) {
  default: return 0;
  case 1: case 2: case 4: case 8: case 16: case 32: case 64: case 128: return 1;
  case 3: case 5: case 6: case 9: case 10: case 12: case 17: case 18: case 20: case 24: case 33: case 34: case 36: case 40: case 48: case 65: case 66: case 68: case 72: case 80: case 96: case 129: case 130: case 132: case 136: case 144: case 160: case 192: return 2;
  case 7: case 11: case 13: case 14: case 19: case 21: case 22: case 25: case 26: case 28: case 35: case 37: case 38: case 41: case 42: case 44: case 49: case 50: case 52: case 56: case 67: case 69: case 70: case 73: case 74: case 76: case 81: case 82: case 84: case 88: case 97: case 98: case 100: case 104: case 112: case 131: case 133: case 134: case 137: case 138: case 140: case 145: case 146: case 148: case 152: case 161: case 162: case 164: case 168: case 176: case 193: case 194: case 196: case 200: case 208: case 224: return 3;
  case 15: case 23: case 27: case 29: case 30: case 39: case 43: case 45: case 46: case 51: case 53: case 54: case 57: case 58: case 60: case 71: case 75: case 77: case 78: case 83: case 85: case 86: case 89: case 90: case 92: case 99: case 101: case 102: case 105: case 106: case 108: case 113: case 114: case 116: case 120: case 135: case 139: case 141: case 142: case 147: case 149: case 150: case 153: case 154: case 156: case 163: case 165: case 166: case 169: case 170: case 172: case 177: case 178: case 180: case 184: case 195: case 197: case 198: case 201: case 202: case 204: case 209: case 210: case 212: case 216: case 225: case 226: case 228: case 232: case 240: return 4;
  case 31: case 47: case 55: case 59: case 61: case 62: case 79: case 87: case 91: case 93: case 94: case 103: case 107: case 109: case 110: case 115: case 117: case 118: case 121: case 122: case 124: case 143: case 151: case 155: case 157: case 158: case 167: case 171: case 173: case 174: case 179: case 181: case 182: case 185: case 186: case 188: case 199: case 203: case 205: case 206: case 211: case 213: case 214: case 217: case 218: case 220: case 227: case 229: case 230: case 233: case 234: case 236: case 241: case 242: case 244: case 248: return 5;
  case 63: case 95: case 111: case 119: case 123: case 125: case 126: case 159: case 175: case 183: case 187: case 189: case 190: case 207: case 215: case 219: case 221: case 222: case 231: case 235: case 237: case 238: case 243: case 245: case 246: case 249: case 250: case 252: return 6;
  case 127: case 191: case 223: case 239: case 247: case 251: case 253: case 254: return 7;
  case 255: return 8;
  }
}
#endif


template <typename T>
static
T
count_bits(uint32_t num) {
  #if 0
  return
    count_bits_in_byte(num & 0xFF) +
    count_bits_in_byte(num >> 8) +
    count_bits_in_byte(num >> 16) +
    count_bits_in_byte(num >> 24);
  #endif

  return
    byte_bit_counts<T>(num & 0xFF) +
    byte_bit_counts<T>((num >> 8)  & 0xFF) +
    byte_bit_counts<T>((num >> 16) & 0xFF) +
    byte_bit_counts<T>((num >> 24) & 0xFF);
}


static constexpr uint32_t arg_bits_for_count(uint32_t count) {
  return arg_masks[count & 0x1F];
}


static uint32_t arg_bits(uint32_t bits) {
  uint32_t r = 0;
  uint32_t args = 0;
  while (bits) {
    if (bits & 0x1) r = (r << 1) | 0x10;
    bits >>= 1;
    ++args;
  }
  // if (args > 28) throw std::runtime_error("too many arguments");
  return r;
}


vm_state_t::vm_state_t(size_t stackSize)
  : _stack()
  , _block_counter(1)
  , _sequence(0)
{
  for (int32_t index = 0; index < REGISTER_COUNT; ++index) {
    _registers[index] = 0;
  }

  _stack.reserve(stackSize);
}


vm_state_t::~vm_state_t() {
  for (auto kvpair : _blocks) {
    if (!(kvpair.second.flags & VM_MEM_SOURCE_DATA)) {
      std::free(kvpair.second.block);
    }
  }
}


vm_function_t<vm_state_t> vm_state_t::function(const char *name) {
  const auto pointer = find_function_pointer(name);
  // if (!pointer.first) throw std::runtime_error("no such function");
  return vm_function_t<vm_state_t>(*this, pointer.second);
}

vm_function_t<vm_state_t> vm_state_t::function(int pointer) {
  return vm_function_t<vm_state_t>(*this, pointer);
}


int32_t vm_state_t::fetch() {
  /*
  while (vm.ip() < ops.size() && vm.ip() >= 0) {
    op_t op = ops[vm.ip()++];
    */
  const int32_t next_instr = ip().i32() + 1;
  ip() = next_instr;
  _trap += (next_instr < 0) || (next_instr >= _source_size);
  return next_instr;
}


void vm_state_t::set_source(source_t &&source) {
  _source = std::move(source);
  _source_size = (uint32_t)_source.size();
  _callbacks.clear();
  _callbacks.resize(_source.imports_table().size(), NULL);
  for (auto kvpair : _source.data_table()) {
    _block_counter = std::max(kvpair.first, _block_counter + 1);
    memblock_t block = {
      kvpair.second->size,
      VM_MEM_SOURCE_DATA,
      kvpair.second->data
    };
    _blocks.emplace(kvpair.first, block);
  }
}


void vm_state_t::bind_callback(const char *name, vm_callback_t *function) {
  auto finding = _source.imported_function(name);
  if (finding.first) {
    const int32_t idx = -(finding.second + 1);
    _callbacks.at(idx) = function;
  }
}


bool vm_state_t::run(int32_t from_ip) {
  ip() = from_ip;
  return run();
}


bool vm_state_t::run() {
  _trap = 0;
  const int32_t term_sequence = _sequence++;
  int32_t opidx = fetch();
  for (; term_sequence < _sequence && !_trap; opidx = fetch()) {
    exec(_source.fetch_op(opidx));
  }
  return _trap == 0;
}


int32_t vm_state_t::unused_block_id() {
  auto end = _blocks.end();
  const auto current = _block_counter;
  while (_blocks.find(_block_counter) != end || _block_counter == 0) {
    ++_block_counter;
  }
  return _block_counter++;
}


int32_t vm_state_t::realloc_block(int32_t block_id, int32_t size) {
  void *src = nullptr;
  if (block_id != 0) {
    memblock_map_t::iterator iter = _blocks.find(block_id);

    if (iter == _blocks.cend()) {
      std::abort();
      return 0;
    }

    src = iter->second.block;
  } else {
    block_id = unused_block_id();
  }

  memblock_t block {
    size,
    VM_MEM_WRITABLE | VM_MEM_READABLE,
    std::realloc(src, static_cast<size_t>(size))
  };

  if (block.block == nullptr) {
    // TODO: replace std::abort calls with sane handling for realloc failure
    std::abort();
    return 0;
  }

  _blocks[block_id] = block;
  return block_id;
}


int32_t vm_state_t::duplicate_block(int32_t block_id) {
  memblock_map_t::const_iterator iter = _blocks.find(block_id);
  if (iter != _blocks.cend()) {
    const auto entry = iter->second;
    if (entry.flags & VM_MEM_READABLE) {
      int32_t new_block_id = realloc_block(VM_NULL_BLOCK, entry.size);
      void *new_block = get_block(new_block_id, VM_MEM_WRITABLE);
      std::memcpy(new_block, entry.block, entry.size);
      return new_block_id;
    }
  }
  return 0;
}


int32_t vm_state_t::block_size(int32_t block_id) const {
  memblock_map_t::const_iterator iter = _blocks.find(block_id);
  if (iter != _blocks.cend()) {
    return iter->second.size;
  }
  return 0;
}


void vm_state_t::free_block(int32_t block_id) {
  memblock_map_t::const_iterator iter = _blocks.find(block_id);
  if (iter != _blocks.cend()) {
    if (!(iter->second.flags & VM_MEM_SOURCE_DATA)) {
      std::free(iter->second.block);
      _blocks.erase(iter);
    } else {
      std::abort();
    }
  }
}


void *vm_state_t::get_block(int32_t block_id, uint32_t permissions) {
  auto block = _blocks.at(block_id);
  if (permissions != VM_MEM_NO_PERMISSIONS && !(block.flags & permissions)) {
    std::abort();
  }
  return block.block;
}


const void *vm_state_t::get_block(int32_t block_id, uint32_t permissions) const {
  auto block = _blocks.at(block_id);
  if (permissions != VM_MEM_NO_PERMISSIONS && !(block.flags & permissions)) {
    std::abort();
  }
  return block.block;
}


value_t vm_state_t::deref(value_t input, value_t flag, uint32_t mask) const {
  return (flag.ui32() & mask) ? input : reg(input.i32());
}


void vm_state_t::exec(const op_t &op) {
  int32_t mask;
  value_t value;
  int8_t *block;
  int8_t *block_in;

  #ifdef LOG_OP_INFO
  std::clog << op.opcode;
  int32_t argidx = 0;
  for (; argidx < g_opcode_argc[op.opcode]; ++argidx) std::clog << ' ' << op[argidx];
  std::clog << std::endl;
  #endif

  switch (op.opcode) {
  // For all math and bitwise instructions, litflag applies to both LHS and RHS
  // input. See vm_state_t::deref for how the test works.
  //
  // For arithmetic/bitwise instructions, the litflags are:
  // 0x1 - LHS is a literal value.
  // 0x2 - RHS is a literal value.
  //
  // For unary instructions, there are no litflags.

  // ADD OUT, LHS, RHS, LITFLAG
  // Addition (fp64).
  case ADD: {
    reg(op[0]).set(deref(op[1], op[3], 0x1).f64() + deref(op[2], op[3], 0x2).f64());
  } break;

  // SUB OUT, LHS, RHS, LITFLAG
  // Subtraction (fp64).
  case SUB: {
    reg(op[0]).set(deref(op[1], op[3], 0x1).f64() - deref(op[2], op[3], 0x2).f64());
  } break;

  // DIV OUT, LHS, RHS, LITFLAG
  // Floating point division.
  case DIV: {
    reg(op[0]).set(deref(op[1], op[3], 0x1).f64() / deref(op[2], op[3], 0x2).f64());
  } break;

  // IDIV OUT, LHS, RHS, LITFLAG
  // Integer division (64-bit signed -- rationale: 64-bit is used as the result
  // will never be out of range of a 64-bit float).
  case IDIV: {
    reg(op[0]).set(deref(op[1], op[3], 0x1).i64() / deref(op[2], op[3], 0x2).i64());
  } break;

  // MUL OUT, LHS, RHS, LITFLAG
  // Multiplication (fp64).
  case MUL: {
    reg(op[0]).set(deref(op[1], op[3], 0x1).f64() * deref(op[2], op[3], 0x2).f64());
  } break;

  // POW OUT, LHS, RHS, LITFLAG
  // Power (fp64).
  case POW: {
    reg(op[0]) = std::pow(deref(op[1], op[3], 0x1).f64(), deref(op[2], op[3], 0x2).f64());
  } break;

  // MOD OUT, LHS, RHS, LITFLAG
  // Floating point modulo.
  case MOD: {
    reg(op[0]) = std::fmod(
        deref(op[1], op[3], 0x1).f64(),
        deref(op[2], op[3], 0x2).f64()
      );
  } break;

  // IMOD OUT, LHS, RHS, LITFLAG
  // Signed integer modulo (32-bit).
  case IMOD: {
    reg(op[0]).set(deref(op[1], op[3], 0x1).i32() % deref(op[2], op[3], 0x2).i32());
  } break;

  // NEG OUT, IN
  // Negation (fp64).
  case NEG: {
    reg(op[0]).set(-reg(op[1]).f64());
  } break;

  // NOT OUT, IN
  // Bitwise not (unsigned 32-bit).
  case NOT: {
    reg(op[0]).set(~(reg(op[1]).ui32()));
  } break;

  // OR OUT, LHS, RHS, LITFLAG
  // Bitwise or (unsigned 32-bit).
  case OR: {
    reg(op[0]).set(deref(op[1], op[2], 0x1).ui32() | deref(op[2], op[3], 0x2).ui32());
  } break;

  // AND OUT, LHS, RHS, LITFLAG
  // Bitwise and (unsigned 32-bit).
  case AND: {
    reg(op[0]).set(deref(op[1], op[2], 0x1).ui32() & deref(op[2], op[3], 0x2).ui32());
  } break;

  // XOR OUT, LHS, RHS, LITFLAG
  // Bitwise xor (unsigned 32-bit).
  case XOR: {
    reg(op[0]).set(deref(op[1], op[2], 0x1).ui32() ^ deref(op[2], op[3], 0x2).ui32());
  } break;

  // ARITHSHIFT OUT, LHS, RHS, LITFLAG
  // Arithmetic shift. Signed 32-bit.
  // RHS > 0  -> Left shift.
  // RHS < 0  -> Right shift.
  // RHS == 0 -> Cast to signed 32-bit int.
  case ARITHSHIFT: {
    const int32_t input = deref(op[1], op[3], 0x1).i32();
    const int32_t shift = deref(op[2], op[3], 0x2).i32();
    if (shift > 0) reg(op[0]).set(input << shift);
    else if (shift < 0) reg(op[0]).set(input >> (-shift));
    else reg(op[0]).set(input);
  } break;

  // BITSHIFT OUT, LHS, RHS, LITFLAG
  // Bitwise shift. Signed 32-bit.
  // RHS > 0  -> Left shift.
  // RHS < 0  -> Right shift.
  // RHS == 0 -> Cast to unsigned 32-bit int.
  case BITSHIFT: {
    const uint32_t input = deref(op[1], op[3], 0x1).ui32();
    const int32_t shift = deref(op[2], op[3]).i32();
    if (shift > 0) reg(op[0]).set(input << shift);
    else if (shift < 0) reg(op[0]).set(input >> (-shift));
    else reg(op[0]).set(input);
  } break;

  // FLOOR OUT, IN
  // Nearest integral value <= IN.
  case FLOOR: {
    reg(op[0]).set(std::floor(reg(op[1]).f64()));
  } break;

  // CEIL OUT, IN
  // Nearest integral value >= IN.
  case CEIL: {
    reg(op[0]).set(std::ceil(reg(op[1]).f64()));
  } break;

  // ROUND OUT, IN
  // Nearest integral value using FE_TONEAREST.
  case ROUND: {
    std::fesetround(FE_TONEAREST);
    reg(op[0]).set(std::round(reg(op[1]).f64()));
  } break;

  // RINT OUT, IN
  // Nearest integral value using FE_TOWARDZERO.
  case RINT: {
    std::fesetround(FE_TOWARDZERO);
    reg(op[0]).set(std::round(reg(op[1]).f64()));
  } break;

  // CMP OUT, LHS, RHS, LITFLAG
  // Compares LHS and RHS as doubles.
  //
  // Litflags:
  // 0x1 - LHS is a literal value.
  // 0x2 - RHS is a literal value.
  //
  // Result stored in OUT depends on the result of comparing the two:
  // if LHS == RHS using vm_fequals => 0.0
  // if LHS > RHS, 1.0
  // Otherwise, LHS < RHS, -1.0
  //
  // The results of CMP are such that they are exactly the 64-bit floating
  // point values above, therefore making them ideal for use with the
  // conditional jumps below.
  case CMP: {
    const double lhs = deref(op[1], op[3], 0x1).f64();
    const double rhs = deref(op[2], op[3], 0x2).f64();
    if (vm_fequals(rhs, lhs)) reg(op[0]).set(0.0);
    else if (lhs > rhs) reg(op[0]).set(1.0);
    else reg(op[0]).set(-1.0);
  } break;

  // Jumps for comparisons. If LITFLAG is set, the POINTER for all jumps is
  // a literal value. All jump pointers are 32-bit signed integers.
  //
  // For all jumps and calls, negative pointers are addresses of host
  // functions and causes the VM to call into them, if bound.
  //
  // All comparisons are expected to be the result of a CMP instruction, though
  // this is not necessarily required. However, it is useful, as the CMP
  // instruction produces exact 64-bit numbers that simplify the checks, as
  // any slightly non-zero value will pass JNE and fail JEQ. JLT and JGT both
  // check for any number greater than or equal to the VM_FCMP_EPSILON (in the
  // case of JLT, this is reversed and it is anything less than
  // or equal to -VM_FCMP_EPSILON).

  // JNEZ COMPARE, POINTER, LITFLAG
  // Jump if the value at COMPARE is non-zero.
  case JNZ: {
    if (reg(op[0]).f64() != 0.0) {
      ip() = deref(op[1], op[2]);
    }
  } break;

  // JEZ COMPARE, POINTER, LITFLAG
  case JEZ: {
    if (reg(op[0]).f64() == 0.0) {
      ip() = deref(op[1], op[2]);
    }
  } break;

  // JGEZ COMPARE, POINTER, LITFLAG
  case JGEZ: {
    if (reg(op[0]).f64() >= 0.0) {
      ip() = deref(op[1], op[2]);
    }
  } break;

  // JLEZ COMPARE, POINTER, LITFLAG
  case JLEZ: {
    if (reg(op[0]).f64() <= 0.0) {
      ip() = deref(op[1], op[2]);
    }
  } break;

  // JLTZ COMPARE, POINTER, LITFLAG
  case JLTZ: {
    if (reg(op[0]).f64() <= -VM_FCMP_EPSILON) {
      ip() = deref(op[1], op[2]);
    }
  } break;

  // JGTZ COMPARE, POINTER, LITFLAG
  case JGTZ: {
    if (reg(op[0]).f64() >= VM_FCMP_EPSILON) {
      ip() = deref(op[1], op[2]);
    }
  } break;

  // JUMP POINTER, LITFLAG
  // Unconditional jump.
  case JUMP: {
    ip() = deref(op[1], op[2]);
  } break;

  // PUSH MASK, LITFLAG
  // Pushes as many values to the stack as are bits set in MASK. Each bit in the
  // mask corresponds to registers 0 through 31. If LITFLAG is set, the mask is
  // a literal 32-bit uint.
  case PUSH: {
    push(reg(op[0]));
  } break;

  // POP MASK, LITFLAG
  // Pops as many values off the stack as are bits set in MASK and assigns them
  // to the registers corresponding to the bits in MASK. If LITFLAG is set,
  // MASK is a literal 32-bit uint.
  case POP: {
    reg(op[0]) = pop();
  } break;

  // LOAD OUT, IN, LITFLAG
  // Copies the value of IN to OUT. If LITFLAG is set, the IN argument is a
  // literal value.
  case LOAD: {
    reg(op[0]) = deref(op[1], op[2]);
  } break;

  // CALL POINTER, ARGC, LITFLAG
  // Executes a call to the function at the given pointer. ARGC indicated the
  // number of arguments on the stack for the receiving function, and is
  // the
  // Litflags:
  // 0x1 - POINTER is a literal address.
  // 0x2 - ARGSMASK is a literal 32-bit uint.
  case CALL: {
    exec_call(deref(op[0], op[2], 0x1), deref(op[1], op[2], 0x2));
  } break;

  // RETURN
  case RETURN: {
    esp() = ebp();
    --_sequence;
  } break;

  // ALLOC OUT, SIZE, LITFLAG
  // Allocates a block of SIZE bytes and writes its ID to the OUT register.
  // If LITFLAG is set, SIZE is a literal.
  case REALLOC: {
    reg(op[0]) = realloc_block(deref(op[1], op[3], 0x1), deref(op[2], op[3], 0x2));
  } break;

  // FREE BLOCKID
  // Frees the block whose ID is held in the given register and zeroes the
  // register.
  case FREE: {
    free_block(reg(op[0]));
    reg(op[0]) = 0.0;
  } break;

  // PEEK OUT, R(BLOCKID), LR(OFFSET), LR(TYPE), LITFLAG
  // Peeks a value of type TYPE from the block at the given OFFSET and writes
  // the result to OUT.
  // Litflags:
  //  0x4 - offset
  //  0x8 - type
  case PEEK: {
    value_t &out = reg(op[0]);
    block = (int8_t *)get_block(reg(op[1]), VM_MEM_READABLE);
    // reg(op[0]).data = *(const uint64_t *)(block + deref(op[2], op[3]).i32());
    int32_t const offset = deref(op[2], op[4], 0x4);
    switch ((memop_typed_t)deref(op[3], op[4], 0x8).i32()) {
    case MEMOP_UINT8:   out = *(uint8_t const *)(block + offset);  break;
    case MEMOP_INT8:    out = *(int8_t const *)(block + offset);   break;
    case MEMOP_UINT16:  out = *(uint16_t const *)(block + offset); break;
    case MEMOP_INT16:   out = *(int16_t const *)(block + offset);  break;
    case MEMOP_UINT32:  out = *(uint32_t const *)(block + offset); break;
    case MEMOP_INT32:   out = *(int32_t const *)(block + offset);  break;
    // 64-bit integral types are only partially supported at the moment (may change later).
    case MEMOP_UINT64:  out = *(uint64_t const *)(block + offset); break;
    case MEMOP_INT64:   out = *(int64_t const *)(block + offset);  break;
    case MEMOP_FLOAT32: out = *(float const *)(block + offset);    break;
    case MEMOP_FLOAT64: out = *(double const *)(block + offset);   break;
    default: /* invalid type */ std::abort(); break;
    }
  } break;

  // POKE R(BLOCKID), LR(VALUE), LR(OFFSET), LR(TYPE), LITFLAG
  // Pokes the given VALUE (reg or lit) into the block at the given OFFSET. The
  // value is converted to the given TYPE.
  //
  // See memop_typed_t for valid TYPE values.
  //
  // Litflags:
  // 0x2 - VALUE
  // 0x4 - OFFSET
  // 0x8 - TYPE
  case POKE: {
    block = (int8_t *)get_block(reg(op[0]), VM_MEM_WRITABLE);
    value = deref(op[1], op[4], 0x2);
    int32_t const offset = deref(op[2], op[4], 0x4);
    switch ((memop_typed_t)deref(op[3], op[4], 0x8).i32()) {
    case MEMOP_UINT8:   *(uint8_t *)(block + offset)  = value.ui8();  break;
    case MEMOP_INT8:    *(int8_t *)(block + offset)   = value.i8();   break;
    case MEMOP_UINT16:  *(uint16_t *)(block + offset) = value.ui16(); break;
    case MEMOP_INT16:   *(int16_t *)(block + offset)  = value.i16();  break;
    case MEMOP_UINT32:  *(uint32_t *)(block + offset) = value.ui32(); break;
    case MEMOP_INT32:   *(int32_t *)(block + offset)  = value.i32();  break;
    // 64-bit integral types are only partially supported at the moment (may change later).
    case MEMOP_UINT64:  *(uint64_t *)(block + offset) = value.ui64(); break;
    case MEMOP_INT64:   *(int64_t *)(block + offset)  = value.i64();  break;
    case MEMOP_FLOAT32: *(float *)(block + offset)    = value.f32();  break;
    case MEMOP_FLOAT64: *(double *)(block + offset)   = value.value;  break;
    default: /* invalid type */ std::abort(); break;
    }
  } break;

  // MEMMOVE BLOCKOUT, OUTOFFSET, BLOCKIN, INOFFSET, SIZE, LITFLAG
  // Just calls memmove for the blocks at the out/in registers. Offsets and
  // size may optionally be literals if their argument flags are set in LITFLAG.
  // Litflags:
  // 0x02 - out offset
  // 0x08 - in offset
  // 0x10 - size
  case MEMMOVE: {
    const value_t flags = op[3];
    int32_t size = deref(op[4], flags, 0x10);
    int32_t dst_offset = deref(op[1], flags, 0x2);
    int32_t src_offset = deref(op[3], flags, 0x8);
    if (size > 0 && dst_offset >= 0 && src_offset >= 0) {
      // check dst block
      int32_t dst_block_id = reg(op[0]);
      int32_t dst_bsize = block_size(dst_block_id);
      block = (int8_t *)get_block(dst_block_id, VM_MEM_READ_WRITE);

      if (dst_offset >= dst_bsize || (dst_offset + size) > dst_bsize || !block) {
        std::abort();
      }

      block += dst_offset;

      // check src block
      int32_t src_block_id = reg(op[2]);
      int32_t src_bsize = block_size(src_block_id);
      block_in = (int8_t *)get_block(src_block_id, VM_MEM_READABLE);

      if (src_offset >= src_bsize || (src_offset + size) > src_bsize || !block_in) {
        std::abort();
      }

      block_in += src_offset;

      std::memmove(block, block_in, size);
    }
  } break;

  // MEMDUP OUT, BLOCKID
  // Allocates a new block of at least the same length as that pointed to by
  // the register BLOCKID, copies the original block's data to the new block,
  // and writes the new block id to OUT.
  case MEMDUP: {
    reg(op[0]).set(duplicate_block(reg(op[1])));
  } break;

  // MEMLEN OUT, BLOCKID
  // Writes the length in bytes of the memory block referred to by the contents
  // of the BLOCKID register to OUT.
  case MEMLEN: {
    reg(op[0]).set(block_size(reg(op[1])));
  } break;

  // LOGAND OUT, LHS, RHS
  // Performs a logical `and` (&&) with the contents LHS and RHS registers and
  // stores the result in OUT.
  case LOGAND: {
    reg(op[0]).set(reg(op[1]).f64() && reg(op[2]).f64());
  } break;

  // LOGOR OUT, LHS, RHS
  // Performs a logical `or` (||) with the contents LHS and RHS registers and
  // stores the result in OUT.
  case LOGOR: {
    reg(op[0]).set(reg(op[1]).f64() || reg(op[2]).f64());
  } break;

  // TRAP
  // Sets the trap flag and returns to the caller. Next run resets the flag.
  case TRAP: {
    _trap = 1;
  } break;

  // SWAP R0, R1
  // Swaps contents of registers at R0 and R1.
  case SWAP: {
    value = reg(op[0]);
    reg(op[0]) = reg(op[1]);
    reg(op[1]) = value;
  } break;

  case OP_COUNT: ;
    // throw std::runtime_error("invalid opcode");
  }

  #ifdef LOG_STATE_CHANGES
  dump_registers();
  dump_stack();
  #endif
}


std::pair<bool, int32_t> vm_state_t::find_function_pointer(const char *name) const {
  const std::string str_name((name));
  auto finding = _source.imported_function(name);
  if (finding.first) return finding;
  else return _source.exported_function(name);
}


value_t vm_state_t::call_function_nt(const char *name, int32_t argc, const value_t *argv) {
  const auto pointer = find_function_pointer(name);
  // if (!pointer.first) throw std::runtime_error("no such function");
  return call_function_nt(pointer.second, argc, argv);
}


value_t vm_state_t::call_function_nt(int32_t pointer, int32_t argc, const value_t *argv) {
  for (int32_t arg_index = 0; arg_index < argc; ++arg_index)
    reg(4 + arg_index) = argv[arg_index];
  return call_function_nt(pointer, argc);
}


value_t vm_state_t::call_function_nt(int32_t pointer, int32_t num_args) {
  const int32_t last_sequence = _sequence++;
  exec_call(pointer, arg_bits_for_count(num_args));
  _sequence = last_sequence;
  return rp();
}


value_t vm_state_t::stack(int32_t loc) const {
  if (loc <= 0) {
    std::abort();
  } else if (loc >= _stack.size()) {
    return value_t::make(0);
  }

  return _stack[loc];
}


value_t &vm_state_t::stack(int32_t loc) {
  loc += ebp().i32();

  if (loc <= 0) {
    std::abort();
  } else if (loc >= _stack.size()) {
    _stack.resize(loc, value_t::make(0));
  }

  return _stack[loc];
}


void vm_state_t::exec_call(int32_t pointer, int32_t argc) {
  #if VM_NONVOLATILE_REGISTERS > 0
  // preserve nonvolatile registers
  std::array<value_t, VM_NONVOLATILE_REGISTERS> nonvolatile_reg;
  auto first_preserved = std::begin(_registers) + R_FIRST_NONVOLATILE;
  auto last_preserved = first_preserved + VM_NONVOLATILE_REGISTERS;
  std::copy(first_preserved, last_preserved, std::begin(nonvolatile_reg));
  #endif


  value_t const preserved_ip = ip();
  value_t const preserved_ebp = ebp();
  ebp() = esp().i32() - argc;

  if (pointer < 0) {
    ++_sequence;

    vm_callback_t *callback = _callbacks[-(ip().i32() + 1)];

    if (argc <= 0) {
      rp() = callback(*this, 0, nullptr);
    } else {
      stack_t::const_iterator argv_end = _stack.cbegin() + ebp().i32();
      stack_t::const_iterator argv_start = argv_end - argc;
      stack_t argv(argv_start, argv_end);
      rp() = callback(*this, argc, &argv[0]);
    }

    --_sequence;
  } else {
    ip() = pointer;
    run();
  }

  #if VM_NONVOLATILE_REGISTERS > 0
  // restore nonvolatiles
  std::copy(std::begin(nonvolatile_reg), std::end(nonvolatile_reg), first_preserved);
  #endif

  ip() = preserved_ip;
  ebp() = preserved_ebp;
}


bool vm_state_t::check_block_bounds(int32_t block_id, int32_t offset, int32_t size) const {
  int32_t const bsize = block_size(block_id);

  return
    offset >= 0 &&
    size > 0 &&
    (offset + size) < bsize &&
    (offset + size) > offset;
}


void vm_state_t::push(value_t value) {
  stack(0) = value;
  esp() = esp().i32() + 1;
}


value_t vm_state_t::pop(bool copy_only) {
  value_t result = stack(esp().i32() - 1);
  if (!copy_only) {
    esp() = esp().i32() - 1;
  }
  return result;
}


void vm_state_t::dump_registers(size_t count) const {
  uint32_t index = 0;
  for (; index < count && index < REGISTER_COUNT; ++index) {
    const value_t &regval = _registers[index];
    std::clog << std::setw(2) << index << " -> " << regval << std::endl;
  }
}


void vm_state_t::dump_stack(size_t until) const {
  uint32_t index = 0;
  for (; index < until && index < _stack.size(); ++index) {
    const value_t &stackval = _stack[index];

    std::clog << "stack[" << index << "]  " << stackval << std::endl;
  }
}
