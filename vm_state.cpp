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

#include <cmath>
#include <iostream>
#include <iomanip>

#define VM_KEEP_DATA_MACROS
#include "vm_state.h"


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
// Only preserve ip, ebp, and esp. rp shouldn't be preserved.
#define CALL_STACK_MASK (0x7)
#endif


vm_state_t::instruction_fn_t *vm_state_t::_instruction_fns[OP_COUNT] = {
  #define VM_INSTRUCTION(OPCODE, ASM_NAME, CODE, ARGS, ARG_INFO... ) vm_state_t::EXEC_##OPCODE,
  #include "vm_instructions.h"
  #undef VM_INSTRUCTION
};


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


static constexpr uint32_t byte_bit_counts[256] = {
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


static constexpr uint32_t count_bits(uint32_t num) {
  #if 0
  return
    count_bits_in_byte(num & 0xFF) +
    count_bits_in_byte(num >> 8) +
    count_bits_in_byte(num >> 16) +
    count_bits_in_byte(num >> 24);
  #endif

  return
    byte_bit_counts[num & 0xFF] +
    byte_bit_counts[(num >> 8)  & 0xFF] +
    byte_bit_counts[(num >> 16) & 0xFF] +
    byte_bit_counts[(num >> 24) & 0xFF];
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
  for (int32_t index = 0; index < REGISTER_COUNT; ++index)
    _registers[index].ui32 = 0;

  _stack.reserve(stackSize);
}


vm_state_t::~vm_state_t() {
  for (auto kvpair : _blocks) {
    if (!(kvpair.second.flags & VM_MEM_SOURCE_DATA)) {
      ::free(kvpair.second.block);
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
  const int32_t result = ip()++;
  if (result < 0 || result >= _source.size()) ++_sequence;
  return result;
}


void vm_state_t::set_source(source_t &&source) {
  _source = std::move(source);
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
  const uint32_t term_sequence = _sequence++;
  int32_t opidx = fetch();
  for (; term_sequence < _sequence && !_trap; opidx = fetch()) {
    exec(_source.fetch_op(opidx));
  }
  return _trap == 0;
}


uint32_t vm_state_t::unused_block_id() {
  auto end = _blocks.end();
  const auto current = _block_counter;
  while (_blocks.find(_block_counter) != end) {
    ++_block_counter;

    // if (_block_counter == current) throw std::runtime_error("no available block id");
  }
  return _block_counter++;
}


uint32_t vm_state_t::alloc(uint32_t size) {
  const uint32_t block_id = unused_block_id();
  memblock_t block = {
    size,
    VM_MEM_WRITABLE | VM_MEM_READABLE,
    malloc(static_cast<size_t>(size))
  };
  _blocks.emplace(block_id, block);
  return block_id;
}


uint32_t vm_state_t::duplicate_block(uint32_t block_id) {
  memblock_map_t::const_iterator iter = _blocks.find(block_id);
  if (iter != _blocks.cend()) {
    const auto entry = iter->second;
    if (entry.flags & VM_MEM_READABLE) {
      uint32_t new_block_id = alloc(entry.size);
      void *new_block = get_block(new_block_id, VM_MEM_WRITABLE);
      memcpy(new_block, entry.block, entry.size);
      return new_block_id;
    }
  }
  return 0;
}


uint32_t vm_state_t::block_size(uint32_t block_id) const {
  memblock_map_t::const_iterator iter = _blocks.find(block_id);
  if (iter != _blocks.cend()) {
    return iter->second.size;
  }
  return 0;
}


void vm_state_t::free(uint32_t block_id) {
  memblock_map_t::const_iterator iter = _blocks.find(block_id);
  if (iter != _blocks.cend()) {
    if (!(iter->second.flags & VM_MEM_SOURCE_DATA)) {
      ::free(iter->second.block);
      _blocks.erase(iter);
    } else {
      std::abort();
    }
  }
}


void *vm_state_t::get_block(uint32_t block_id, uint32_t permissions) {
  auto block = _blocks.at(block_id);
  if (permissions != VM_MEM_NO_PERMISSIONS && !(block.flags & permissions)) {
    std::abort();
  }
  return block.block;
}


const void *vm_state_t::get_block(uint32_t block_id, uint32_t permissions) const {
  auto block = _blocks.at(block_id);
  if (permissions != VM_MEM_NO_PERMISSIONS && !(block.flags & permissions)) {
    std::abort();
  }
  return block.block;
}


void vm_state_t::exec(const op_t &op) {
  #ifdef LOG_OP_INFO
  std::clog << op.opcode;
  int32_t argidx = 0;
  for (; argidx < g_opcode_argc[op.opcode]; ++argidx) std::clog << ' ' << op.argv[argidx];
  std::clog << std::endl;
  #endif

#if 0
  switch (op.opcode) {
  case NOP: break;
  case ADD_F32: {
    reg(op.argv[0].i32).f32 = reg(op.argv[1].i32).f32 + reg(op.argv[2].i32).f32;
  } break;
  case ADD_I32: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 + reg(op.argv[2].i32).i32;
  } break;
  case ADD_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 + reg(op.argv[2].i32).ui32;
  } break;
  case ADD_F32_L: {
    reg(op.argv[0].i32).f32 = reg(op.argv[1].i32).f32 + op.argv[2].f32;
  } break;
  case ADD_I32_L: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 + op.argv[2].i32;
  } break;
  case ADD_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 + op.argv[2].ui32;
  } break;
  case SUB_F32: {
    reg(op.argv[0].i32).f32 = reg(op.argv[1].i32).f32 - reg(op.argv[2].i32).f32;
  } break;
  case SUB_I32: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 - reg(op.argv[2].i32).i32;
  } break;
  case SUB_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 - reg(op.argv[2].i32).ui32;
  } break;
  case SUB_F32_L: {
    reg(op.argv[0].i32).f32 = reg(op.argv[1].i32).f32 - op.argv[2].f32;
  } break;
  case SUB_I32_L: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 - op.argv[2].i32;
  } break;
  case SUB_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 - op.argv[2].ui32;
  } break;
  case DIV_F32: {
    reg(op.argv[0].i32).f32 = reg(op.argv[1].i32).f32 / reg(op.argv[2].i32).f32;
  } break;
  case DIV_I32: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 / reg(op.argv[2].i32).i32;
  } break;
  case DIV_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 / reg(op.argv[2].i32).ui32;
  } break;
  case DIV_F32_L: {
    reg(op.argv[0].i32).f32 = reg(op.argv[1].i32).f32 / op.argv[2].f32;
  } break;
  case DIV_I32_L: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 / op.argv[2].i32;
  } break;
  case DIV_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 / op.argv[2].ui32;
  } break;
  case MUL_F32: {
    reg(op.argv[0].i32).f32 = reg(op.argv[1].i32).f32 * reg(op.argv[2].i32).f32;
  } break;
  case MUL_I32: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 * reg(op.argv[2].i32).i32;
  } break;
  case MUL_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 * reg(op.argv[2].i32).ui32;
  } break;
  case MUL_F32_L: {
    reg(op.argv[0].i32).f32 = reg(op.argv[1].i32).f32 * op.argv[2].f32;
  } break;
  case MUL_I32_L: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 * op.argv[2].i32;
  } break;
  case MUL_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 * op.argv[2].ui32;
  } break;
  case MOD_F32: {
    reg(op.argv[0].i32).f32 = std::fmod(reg(op.argv[1].i32).f32, reg(op.argv[2].i32).f32);
  } break;
  case MOD_I32: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 % reg(op.argv[2].i32).i32;
  } break;
  case MOD_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 % reg(op.argv[2].i32).ui32;
  } break;
  case MOD_F32_L: {
    reg(op.argv[0].i32).f32 = std::fmod(reg(op.argv[1].i32).f32, op.argv[2].f32);
  } break;
  case MOD_I32_L: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 % op.argv[2].i32;
  } break;
  case MOD_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 % op.argv[2].ui32;
  } break;
  case NEG_F32: {
    reg(op.argv[0].i32).f32 = -reg(op.argv[1].i32).f32;
  } break;
  case NEG_I32: {
    reg(op.argv[0].i32).i32 = -reg(op.argv[1].i32).i32;
  } break;
  case NOT_UI32: {
    reg(op.argv[0].i32).ui32 = ~(reg(op.argv[1].i32).ui32);
  } break;
  case OR_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 | reg(op.argv[2].i32).ui32;
  } break;
  case XOR_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 ^ reg(op.argv[2].i32).ui32;
  } break;
  case AND_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 & reg(op.argv[2].i32).ui32;
  } break;
  case SHR_I32: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 >> reg(op.argv[2].i32).ui32;
  } break;
  case SHR_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 >> reg(op.argv[2].i32).ui32;
  } break;
  case SHL_I32: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 << reg(op.argv[2].i32).ui32;
  } break;
  case SHL_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 << reg(op.argv[2].i32).ui32;
  } break;
  case OR_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 | op.argv[2].ui32;
  } break;
  case XOR_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 ^ op.argv[2].ui32;
  } break;
  case AND_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 & op.argv[2].ui32;
  } break;
  case SHR_I32_L: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 >> op.argv[2].ui32;
  } break;
  case SHR_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 >> op.argv[2].ui32;
  } break;
  case SHL_I32_L: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 << op.argv[2].ui32;
  } break;
  case SHL_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 << op.argv[2].ui32;
  } break;
  case ITOUI: {
    reg(op.argv[0].i32).ui32 = (uint32_t)reg(op.argv[1].i32).i32;
  } break;
  case ITOF: {
    reg(op.argv[0].i32).f32 = static_cast<float>(reg(op.argv[1].i32).i32);
  } break;
  case FTOUI: {
    reg(op.argv[0].i32).ui32 = static_cast<uint32_t>(reg(op.argv[1].i32).f32);
  } break;
  case FTOI: {
    reg(op.argv[0].i32).i32 = static_cast<int32_t>(reg(op.argv[1].i32).f32);
  } break;
  case UITOF: {
    reg(op.argv[0].i32).f32 = static_cast<float>(reg(op.argv[1].i32).ui32);
  } break;
  case UITOI: {
    reg(op.argv[0].i32).i32 = static_cast<int32_t>(reg(op.argv[1].i32).ui32);
  } break;
  case CMP_F32: {
    const float l = reg(op.argv[1].i32).f32;
    const float r = reg(op.argv[2].i32).f32;
    reg(op.argv[0].i32).i32 = (l > r) - (l < r);
  } break;
  case CMP_I32: {
    const int32_t l = reg(op.argv[1].i32).i32;
    const int32_t r = reg(op.argv[2].i32).i32;
    reg(op.argv[0].i32).i32 = (l > r) - (l < r);
  } break;
  case CMP_UI32: {
    const uint32_t l = reg(op.argv[1].i32).ui32;
    const uint32_t r = reg(op.argv[2].i32).ui32;
    reg(op.argv[0].i32).i32 = (l > r) - (l < r);
  } break;
  case CMP_F32_L: {
    const float l = reg(op.argv[1].i32).f32;
    const float r = op.argv[2].f32;
    reg(op.argv[0].i32).i32 = (l > r) - (l < r);
  } break;
  case CMP_I32_L: {
    const int32_t l = reg(op.argv[1].i32).i32;
    const int32_t r = op.argv[2].i32;
    reg(op.argv[0].i32).i32 = (l > r) - (l < r);
  } break;
  case CMP_UI32_L: {
    const uint32_t l = reg(op.argv[1].i32).ui32;
    const uint32_t r = op.argv[2].ui32;
    reg(op.argv[0].i32).i32 = (l > r) - (l < r);
  } break;
  case RCMP_F32: {
    const float l = reg(op.argv[1].i32).f32;
    const float r = reg(op.argv[2].i32).f32;
    reg(op.argv[0].i32).i32 = (l < r) - (l > r);
  } break;
  case RCMP_I32: {
    const int32_t l = reg(op.argv[1].i32).i32;
    const int32_t r = reg(op.argv[2].i32).i32;
    reg(op.argv[0].i32).i32 = (l < r) - (l > r);
  } break;
  case RCMP_UI32: {
    const uint32_t l = reg(op.argv[1].i32).ui32;
    const uint32_t r = reg(op.argv[2].i32).ui32;
    reg(op.argv[0].i32).i32 = (l < r) - (l > r);
  } break;
  case RCMP_F32_L: {
    const float l = reg(op.argv[1].i32).f32;
    const float r = op.argv[2].f32;
    reg(op.argv[0].i32).i32 = (l < r) - (l > r);
  } break;
  case RCMP_I32_L: {
    const int32_t l = reg(op.argv[1].i32).i32;
    const int32_t r = op.argv[2].i32;
    reg(op.argv[0].i32).i32 = (l < r) - (l > r);
  } break;
  case RCMP_UI32_L: {
    const uint32_t l = reg(op.argv[1].i32).ui32;
    const uint32_t r = op.argv[2].ui32;
    reg(op.argv[0].i32).i32 = (l < r) - (l > r);
  } break;
  case JNZ: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 != 0];
    ip() = (reg(op.argv[1].i32).i32 & mask) | (ip() & ~mask);
  } break;
  case JEZ: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 == 0];
    ip() = (reg(op.argv[1].i32).i32 & mask) | (ip() & ~mask);
  } break;
  case JGTE: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 >= 0];
    ip() = (reg(op.argv[1].i32).i32 & mask) | (ip() & ~mask);
  } break;
  case JLTE: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 <= 0];
    ip() = (reg(op.argv[1].i32).i32 & mask) | (ip() & ~mask);
  } break;
  case JLT: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 < 0];
    ip() = (reg(op.argv[1].i32).i32 & mask) | (ip() & ~mask);
  } break;
  case JGT: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 > 0];
    ip() = (reg(op.argv[1].i32).i32 & mask) | (ip() & ~mask);
  } break;
  case JUMP: {
    ip() = reg(op.argv[1].i32).i32;
  } break;
  case JNZ_L: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 != 0];
    ip() = (op.argv[1].i32 & mask) | (ip() & ~mask);
  } break;
  case JEZ_L: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 == 0];
    ip() = (op.argv[1].i32 & mask) | (ip() & ~mask);
  } break;
  case JGTE_L: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 >= 0];
    ip() = (op.argv[1].i32 & mask) | (ip() & ~mask);
  } break;
  case JLTE_L: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 <= 0];
    ip() = (op.argv[1].i32 & mask) | (ip() & ~mask);
  } break;
  case JLT_L: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 < 0];
    ip() = (op.argv[1].i32 & mask) | (ip() & ~mask);
  } break;
  case JGT_L: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 > 0];
    ip() = (op.argv[1].i32 & mask) | (ip() & ~mask);
  } break;
  case JUMP_L: {
    ip() = op.argv[0].i32;
  } break;
  case STORE: {
    stack(reg(op.argv[0].i32).i32).ui32 = reg(op.argv[1].i32).ui32;
  } break;
  case STORE_L: {
    stack(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32;
  } break;
  case GET: {
    reg(op.argv[0].i32).ui32 = stack(op.argv[1].i32).ui32;
  } break;
  case GET_L: {
    reg(op.argv[0].i32).ui32 = stack(reg(op.argv[1].i32).i32).ui32;
  } break;
  case PUSH: {
    push(op.argv[0].ui32);
  } break;
  case POP: {
    pop(op.argv[0].ui32, true);
  } break;
  case LOAD: {
    reg(op.argv[0].i32).ui32 = op.argv[1].ui32;
  } break;
  case MOVE: {
    reg(op.argv[0].i32) = reg(op.argv[1].i32);
  } break;
  case CALL: {
    exec_call(op.argv[0].i32, op.argv[1].ui32);
  } break;
  case CALL_D: {
    exec_call(reg(op.argv[0].i32).i32, reg(op.argv[1].i32).ui32);
  } break;
  case RETURN: {
    value_t result = reg(op.argv[0].i32);
    #ifdef VM_PRESERVE_CALL_ARGUMENT_REGISTERS
    pop(esp(), true);
    #endif
    pop(CALL_STACK_MASK, true);
    rp() = result;
    --_sequence;
  } break;
  case ALLOC: {
    reg(op.argv[0].i32).ui32 = alloc(reg(op.argv[1].i32).ui32);
  } break;
  case ALLOC_L: {
    reg(op.argv[0].i32).ui32 = alloc(reg(op.argv[1].i32).ui32);
  } break;
  case FREE: {
    free(reg(op.argv[0].i32).ui32);
  } break;
  case PEEK8: {
    const uint8_t *block = (const uint8_t *)get_block(reg(op.argv[1].i32).ui32, VM_MEM_NO_PERMISSIONS);
    reg(op.argv[0].i32).ui32 = block[reg(op.argv[2].i32).ui32];
  } break;
  case PEEK16: {
    const uint8_t *block = (const uint8_t *)get_block(reg(op.argv[1].i32).ui32, VM_MEM_NO_PERMISSIONS);
    reg(op.argv[0].i32).ui32 = *(const uint16_t *)(block + reg(op.argv[2].i32).ui32);
  } break;
  case PEEK32: {
    const uint8_t *block = (const uint8_t *)get_block(reg(op.argv[1].i32).ui32, VM_MEM_NO_PERMISSIONS);
    reg(op.argv[0].i32).ui32 = *(const uint32_t *)(block + reg(op.argv[2].i32).ui32);
  } break;
  case POKE8: {
    uint8_t *block = (uint8_t *)get_block(reg(op.argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE);
    block[reg(op.argv[1].i32).ui32] = (uint8_t)(reg(op.argv[2].i32).ui32 & 0xFF);
  } break;
  case POKE16: {
    uint8_t *block = (uint8_t *)get_block(reg(op.argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE);
    *(uint16_t *)(block + reg(op.argv[1].i32).ui32) = (uint16_t)reg(op.argv[2].i32).ui32 & 0xFFFF;
  } break;
  case POKE32: {
    uint8_t *block = (uint8_t *)get_block(reg(op.argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE);
    *(uint32_t *)(block + reg(op.argv[1].i32).ui32) = (uint32_t)reg(op.argv[2].i32).ui32;
  } break;
  case PEEK8_L: {
    const uint8_t *block = (const uint8_t *)get_block(reg(op.argv[1].i32).ui32, VM_MEM_NO_PERMISSIONS);
    reg(op.argv[0].i32).ui32 = block[op.argv[2].ui32];
  } break;
  case PEEK16_L: {
    const uint8_t *block = (const uint8_t *)get_block(reg(op.argv[1].i32).ui32, VM_MEM_NO_PERMISSIONS);
    reg(op.argv[0].i32).ui32 = *(const uint16_t *)(block + op.argv[2].ui32);
  } break;
  case PEEK32_L: {
    const uint8_t *block = (const uint8_t *)get_block(reg(op.argv[1].i32).ui32, VM_MEM_NO_PERMISSIONS);
    reg(op.argv[0].i32).ui32 = *(const uint32_t *)(block + op.argv[2].ui32);
  } break;
  case POKE8_L: {
    uint8_t *block = (uint8_t *)get_block(reg(op.argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE);
    block[op.argv[1].ui32] = (uint8_t)(reg(op.argv[2].i32).ui32 & 0xFF);
  } break;
  case POKE16_L: {
    uint8_t *block = (uint8_t *)get_block(reg(op.argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE);
    *(uint16_t *)(block + op.argv[1].ui32) = (uint16_t)reg(op.argv[2].i32).ui32 & 0xFFFF;
  } break;
  case POKE32_L: {
    uint8_t *block = (uint8_t *)get_block(reg(op.argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE);
    *(uint32_t *)(block + op.argv[1].ui32) = (uint32_t)reg(op.argv[2].i32).ui32;
  } break;
  case MEMMOVE: {
    uint8_t *block_out = ((uint8_t *)get_block(reg(op.argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE)) + reg(op.argv[1].i32).ui32;
    uint8_t *block_in = ((uint8_t *)get_block(reg(op.argv[2].i32).ui32, VM_MEM_NO_PERMISSIONS)) + reg(op.argv[3].i32).ui32;
    memmove(block_out, block_in, reg(op.argv[4].i32).ui32);
  } break;
  case MEMMOVE_L: {
    uint8_t *block_out = ((uint8_t *)get_block(reg(op.argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE)) + op.argv[1].ui32;
    uint8_t *block_in = ((uint8_t *)get_block(reg(op.argv[2].i32).ui32, VM_MEM_NO_PERMISSIONS)) + op.argv[3].ui32;
    memmove(block_out, block_in, op.argv[4].ui32);
  } break;
  case MEMDUP: {
    reg(op.argv[0].i32).ui32 = duplicate_block(reg(op.argv[1].i32).ui32);
  } break;
  case MEMLEN: {
    reg(op.argv[0].i32).ui32 = block_size(reg(op.argv[1].i32).ui32);
  } break;
  case LOGAND: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 && reg(op.argv[2].i32).ui32;
  } break;
  case LOGOR: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 || reg(op.argv[2].i32).ui32;
  } break;
  case TRAP: {
    _trap = 1;
  } break;
  case SWAP: {
    uint32_t temp = reg(op.argv[0].i32).ui32;
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32;
    reg(op.argv[1].i32).ui32 = temp;
  } break;
  case OP_COUNT: {
    // throw std::runtime_error("invalid opcode");
  }
  }
#endif

  _instruction_fns[op.opcode](*this, op.argv);

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


value_t vm_state_t::call_function_nt(const char *name, uint32_t argc, const value_t *argv) {
  const auto pointer = find_function_pointer(name);
  // if (!pointer.first) throw std::runtime_error("no such function");
  return call_function_nt(pointer.second, argc, argv);
}


value_t vm_state_t::call_function_nt(int32_t pointer, uint32_t argc, const value_t *argv) {
  for (uint32_t arg_index = 0; arg_index < argc; ++arg_index)
    reg(4 + arg_index) = argv[arg_index];
  return call_function_nt(pointer, argc);
}


value_t vm_state_t::call_function_nt(int32_t pointer, uint32_t num_args) {
  const uint32_t last_sequence = _sequence++;
  exec_call(pointer, arg_bits_for_count(num_args));
  run(ip());
  _sequence = last_sequence;
  return rp();
}


void vm_state_t::exec_call(int32_t pointer, uint32_t args_mask) {
  ++_sequence;
  push(CALL_STACK_MASK);
  esp() = args_mask;
  push(esp());
  #ifdef VM_PRESERVE_CALL_ARGUMENT_REGISTERS
  pop(arg_bits(esp()), false);
  #else
  pop(arg_bits(esp()), true);
  #endif
  if ((ip() = pointer) < 0) {
    vm_callback_t *callback = _callbacks[-(ip() + 1)];
    // if (callback == NULL) throw std::runtime_error("unbound imported function");
    rp() = callback(*this, count_bits(args_mask), &reg(4));
    #ifdef VM_PRESERVE_CALL_ARGUMENT_REGISTERS
    pop(esp(), true);
    #endif
    pop(CALL_STACK_MASK, true);
    --_sequence;
  }
}


void vm_state_t::push(uint32_t bits) {
  if (bits) {
    std::bitset<REGISTER_COUNT> reg_bits(static_cast<unsigned long long>(bits));
    int32_t max_stack = (int32_t)reg_bits.count();
    _stack.resize(ebp() + max_stack);
    int32_t stack_index = 0;
    int32_t reg_index = 0;
    for (; reg_index < REGISTER_COUNT && stack_index < max_stack; ++reg_index)
      if (reg_bits[reg_index]) stack(stack_index++).ui32 = reg(reg_index).ui32;
    ebp() += stack_index;
  }
  // std::clog << "POST-PUSH("<<bits<<"): " << ebp() << std::endl;
}


void vm_state_t::pop(uint32_t bits, bool shrink) {
  if (bits) {
    auto off = ebp();
    // if (off == 0) throw std::underflow_error("stack underflow");
    std::bitset<REGISTER_COUNT> reg_bits(static_cast<unsigned long long>(bits));
    int32_t max_stack = (int32_t)reg_bits.count();
    int32_t stack_index = 0;
    int32_t reg_index = 0;
    ebp() -= max_stack;
    for (; reg_index < REGISTER_COUNT && stack_index < max_stack; ++reg_index) {
      if (reg_bits[reg_index]) {
        reg(reg_index).ui32 = stack(stack_index++).ui32;
      }
    }

    if (shrink) {
      _stack.resize(ebp());
    } else {
      ebp() = off;
    }
  }
  // std::clog << "POST-POP: (" << bits << ", " << shrink << ") " << ebp() << std::endl;
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

#include "vm_instructions.cpp.inl"
