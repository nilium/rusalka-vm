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
#include "vm_exception.h"
#include "vm_state.h"


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
  MEMOP_MAX,
};


static int32_t const MEMOP_SIZE[MEMOP_MAX] {
  1, 1,
  2, 2,
  4, 4,
  8, 8,
  4, 8,
};


/*
  Sets the rounding mode and then calls func; the previous rounding mode is
  restored after the call completes.
*/
template <typename FN>
static void with_rounding(int const round_mode, FN &&func) noexcept
{
  int const previous = std::fegetround();
  std::fesetround(round_mode);
  func();
  std::fesetround(previous);
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


vm_state_t::vm_state_t(size_t stackSize)
: _stack()
, _block_counter(1)
, _sequence(0)
{
  for (int32_t index = 0; index < REGISTER_COUNT; ++index) {
    _registers[index] = 0.0;
  }

  _stack.resize(stackSize);
}


vm_state_t::~vm_state_t()
{
  release_all_memblocks();
}


vm_function_t<vm_state_t> vm_state_t::function(const char *name)
{
  const auto pointer = find_function_pointer(name);
  // if (!pointer.first) throw std::runtime_error("no such function");
  return vm_function_t<vm_state_t>(*this, pointer.second);
}

vm_function_t<vm_state_t> vm_state_t::function(int pointer)
{
  return vm_function_t<vm_state_t>(*this, pointer);
}


int32_t vm_state_t::fetch()
{
  const int32_t next_instr = ip().i32();
  ip() = next_instr + 1;
  if (next_instr < 0 || next_instr >= _source_size) {
    ++_trap;
  }
  return next_instr;
}


void vm_state_t::set_unit(vm_unit_t const &unit)
{
  _unit = unit;
  prepare_unit();
}


void vm_state_t::set_unit(vm_unit_t &&unit)
{
  _unit = std::forward<vm_unit_t &&>(unit);
  prepare_unit();
}


void vm_state_t::prepare_unit()
{
  release_all_memblocks();
  _block_counter = 1;
  _source_size = _unit.instructions.size();
  _callbacks.resize(_unit.imports.size());
  std::fill(_callbacks.begin(), _callbacks.end(), nullptr);
  vm_unit_t::data_id_ary_t new_ids;
  new_ids.resize(_unit._data_blocks.size(), 0);
  _unit.each_data([&](int32_t index, int32_t id, int32_t size, void const *ptr, bool &stop) {
    int32_t new_id = unused_block_id();
    memblock_t block { size, VM_MEM_SOURCE_DATA, const_cast<void *>(ptr) };
    _blocks.emplace(new_id, block);
    new_ids[index] = new_id;
  });
  _unit.relocate_static_data(new_ids);
}


void vm_state_t::bind_callback(const char *name, int length, vm_callback_t *function)
{
  std::string name_str(name, length);
  auto imported = _unit.imports.find(name_str);
  if (imported != _unit.imports.cend()) {
    const int32_t idx = -(imported->second + 1);
    _callbacks.at(idx) = function;
  }
}


bool vm_state_t::run(int32_t from_ip)
{
  ip() = from_ip;
  return run();
}


bool vm_state_t::run()
{
  _trap = 0;
  const int32_t term_sequence = _sequence++;
  while (!_trap && term_sequence < _sequence) {
    int32_t opidx = fetch();
    exec(_unit.fetch_op(opidx));
  }
  return _trap == 0;
}


void vm_state_t::release_all_memblocks()
{
  for (auto kvpair : _blocks) {
    if (!(kvpair.second.flags & VM_MEM_STATIC)) {
      std::free(kvpair.second.block);
    }
  }
  _blocks.clear();
}


int32_t vm_state_t::unused_block_id()
{
  auto end = _blocks.end();
  while (_blocks.find(_block_counter) != end || _block_counter == 0) {
    ++_block_counter;
  }
  return _block_counter++;
}


int32_t vm_state_t::realloc_block(int32_t block_id, int32_t size)
{
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


int32_t vm_state_t::duplicate_block(int32_t block_id)
{
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


int32_t vm_state_t::block_size(int32_t block_id) const
{
  if (block_id == 0) {
    return 0;
  }

  memblock_map_t::const_iterator iter = _blocks.find(block_id);
  if (iter != _blocks.cend()) {
    return iter->second.size;
  }
  return 0;
}


void vm_state_t::free_block(int32_t block_id)
{
  memblock_map_t::const_iterator iter = _blocks.find(block_id);
  if (iter != _blocks.cend()) {
    if (!(iter->second.flags & VM_MEM_STATIC)) {
      std::free(iter->second.block);
      _blocks.erase(iter);
    } else {
      std::abort();
    }
  }
}


void *vm_state_t::get_block(int32_t block_id, uint32_t permissions)
{
  if (block_id == 0) {
    return nullptr;
  }

  auto block = _blocks.at(block_id);
  if (permissions != VM_MEM_NO_PERMISSIONS && !(block.flags & permissions)) {
    std::abort();
  }
  return block.block;
}


const void *vm_state_t::get_block(int32_t block_id, uint32_t permissions) const
{
  auto block = _blocks.at(block_id);
  if (permissions != VM_MEM_NO_PERMISSIONS && !(block.flags & permissions)) {
    std::abort();
  }
  return block.block;
}


value_t vm_state_t::deref(value_t input, uint16_t flag, uint32_t mask) const
{
  return (flag & mask) ? input : reg(input.i32());
}


template <typename T>
constexpr T vm_shift(T num, int32_t shift)
{
  return
    shift == 0
    ? num
    : ((shift > 0) ? (num << shift) : (num >> (-shift)));
}


void vm_state_t::exec(const op_t &op)
{
  value_t value;
  uint16_t const litflag = op.litflag();

  #ifdef LOG_OP_INFO
  bool has_litflag = opcode_has_litflag(op.opcode());
  std::clog << std::setw(10) << (ip().i32() - 1) << ": " << std::setw(10) << op.opcode();
  int32_t argidx = 0;
  for (; argidx < g_opcode_argc[op.opcode()] - (has_litflag ? 1 : 0); ++argidx) {
    std::clog << "    ";
    if (has_litflag) {
      if ((litflag & (0x1 << argidx)) != 0) {
        std::clog << op[argidx].f64();
      } else {
        std::clog << "r" << op[argidx].i32() << "(" << deref(op[argidx], litflag, 0x1 << argidx).f64() << ')';
      }
    } else {
      std::clog << "r" << op[argidx].i32() << "(" << reg(op[argidx]).f64() << ')';
    }
  }
  std::clog << std::endl;
  #endif

  switch (op.opcode()) {
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
    reg(op[0]) = deref(op[1], litflag, 0x2).f64() + deref(op[2], litflag, 0x4).f64();
  } break;

  // SUB OUT, LHS, RHS, LITFLAG
  // Subtraction (fp64).
  case SUB: {
    reg(op[0]) = deref(op[1], litflag, 0x2).f64() - deref(op[2], litflag, 0x4).f64();
  } break;

  // DIV OUT, LHS, RHS, LITFLAG
  // Floating point division.
  case DIV: {
    reg(op[0]) = deref(op[1], litflag, 0x2).f64() / deref(op[2], litflag, 0x4).f64();
  } break;

  // IDIV OUT, LHS, RHS, LITFLAG
  // Integer division (64-bit signed -- rationale: 64-bit is used as the result
  // will never be out of range of a 64-bit float).
  case IDIV: {
    reg(op[0]) = deref(op[1], litflag, 0x2).i64() / deref(op[2], litflag, 0x4).i64();
  } break;

  // MUL OUT, LHS, RHS, LITFLAG
  // Multiplication (fp64).
  case MUL: {
    reg(op[0]) = deref(op[1], litflag, 0x2).f64() * deref(op[2], litflag, 0x4).f64();
  } break;

  // POW OUT, LHS, RHS, LITFLAG
  // Power (fp64).
  case POW: {
    reg(op[0]) = std::pow(deref(op[1], litflag, 0x2).f64(), deref(op[2], litflag, 0x4).f64());
  } break;

  // MOD OUT, LHS, RHS, LITFLAG
  // Floating point modulo.
  case MOD: {
    reg(op[0]) = std::fmod(
        deref(op[1], litflag, 0x2).f64(),
        deref(op[2], litflag, 0x4).f64()
      );
  } break;

  // IMOD OUT, LHS, RHS, LITFLAG
  // Signed integer modulo (32-bit).
  case IMOD: {
    reg(op[0]) = deref(op[1], litflag, 0x2).i64() % deref(op[2], litflag, 0x4).i64();
  } break;

  // NEG OUT, IN
  // Negation (fp64).
  case NEG: {
    reg(op[0]) = -reg(op[1]).f64();
  } break;

  // NOT OUT, IN
  // Bitwise not (unsigned 32-bit).
  case NOT: {
    reg(op[0]) = ~reg(op[1]).ui32();
  } break;

  // OR OUT, LHS, RHS, LITFLAG
  // Bitwise or (unsigned 32-bit).
  case OR: {
    reg(op[0]) = deref(op[1], litflag, 0x2).ui32() | deref(op[2], litflag, 0x4).ui32();
  } break;

  // AND OUT, LHS, RHS, LITFLAG
  // Bitwise and (unsigned 32-bit).
  case AND: {
    reg(op[0]) = deref(op[1], litflag, 0x2).ui32() & deref(op[2], litflag, 0x4).ui32();
  } break;

  // XOR OUT, LHS, RHS, LITFLAG
  // Bitwise xor (unsigned 32-bit).
  case XOR: {
    reg(op[0]) = deref(op[1], litflag, 0x2).ui32() ^ deref(op[2], litflag, 0x4).ui32();
  } break;

  // ARITHSHIFT OUT, LHS, RHS, LITFLAG
  // Arithmetic shift. Signed 32-bit.
  // RHS > 0  -> Left shift.
  // RHS < 0  -> Right shift.
  // RHS == 0 -> Cast to signed 32-bit int.
  case ARITHSHIFT: {
    const int32_t input = deref(op[1], litflag, 0x2);
    const int32_t shift = deref(op[2], litflag, 0x4);
    reg(op[0]) = vm_shift(input, shift);
  } break;

  // BITSHIFT OUT, LHS, RHS, LITFLAG
  // Bitwise shift. Signed 32-bit.
  // RHS > 0  -> Left shift.
  // RHS < 0  -> Right shift.
  // RHS == 0 -> Cast to unsigned 32-bit int.
  case BITSHIFT: {
    const uint32_t input = deref(op[1], litflag, 0x2);
    const int32_t shift = deref(op[2], litflag, 0x4);
    reg(op[0]) = vm_shift(input, shift);
  } break;

  // FLOOR OUT, IN
  // Nearest integral value <= IN.
  case FLOOR: {
    reg(op[0]) = std::floor(reg(op[1]).f64());
  } break;

  // CEIL OUT, IN
  // Nearest integral value >= IN.
  case CEIL: {
    reg(op[0]) = std::ceil(reg(op[1]).f64());
  } break;

  // ROUND OUT, IN
  // Nearest integral value using FE_TONEAREST.
  case ROUND: {
    with_rounding(FE_TONEAREST, [&] {
      reg(op[0]) = std::nearbyint(reg(op[1]).f64());
    });
  } break;

  // RINT OUT, IN
  // Nearest integral value using FE_TOWARDZERO.
  case RINT: {
    with_rounding(FE_TOWARDZERO, [&] {
      reg(op[0]) = std::nearbyint(reg(op[1]).f64());
    });
  } break;

  // EQ|LE|LT LHS, RHS, RESULT, LITFLAG
  // Compares LHS and RHS. If the comparison's result is RESULT, the next
  // instruction is executed, otherwise IP is incremented by 1 and the next
  // instruction is skipped.
  // - EQ tests for equality if RESULT is non-zero, otherwise inequality.
  // - LE tests if LHS is less than or equal to RHS if non-zero, otherwise greater
  //   than.
  // - LT tests if LHS is less than RHS if non-zero, otherwise greater than or
  //   equal to.
  // Litflags:
  // 0x1 - LHS is a literal
  // 0x2 - RHS is a literal
  case EQ: {
    if ((deref(op[0], litflag, 0x1) == deref(op[1], litflag, 0x2)) != (op[2].i32() != 0)) {
      ip() = ip().i32() + 1;
    }
  } break;

  case LT: {
    if ((deref(op[0], litflag, 0x1) < deref(op[1], litflag, 0x2)) != (op[2].i32() != 0)) {
      ip() = ip().i32() + 1;
    }
  } break;

  case LE: {
    if ((deref(op[0], litflag, 0x1) <= deref(op[1], litflag, 0x2)) != (op[2].i32() != 0)) {
      ip() = ip().i32() + 1;
    }
  } break;

  // JUMP POINTER, LITFLAG
  // Unconditional jump.
  // Litflags:
  // 0x1 - POINTER is a literal address.
  case JUMP: {
    ip() = deref(op[0], litflag, 0x1);
  } break;

  // PUSH REG
  // Pushes the value in REG onto the stack.
  case PUSH: {
    push(reg(op[0]));
  } break;

  // POP REG
  // Pops the last value on the stack and stores it in REG.
  case POP: {
    reg(op[0]) = pop(false);
  } break;

  // LOAD OUT, IN, LITFLAG
  // Copies the value of IN to OUT.
  // Litflags:
  // 0x2 - IN is a literal.
  case LOAD: {
    reg(op[0]) = deref(op[1], litflag, 0x2);
  } break;

  // CALL POINTER, ARGC, LITFLAG
  // Executes a call to the function at the given pointer. ARGC indicated the
  // number of arguments on the stack for the receiving function, and is
  // the
  // Litflags:
  // 0x1 - POINTER is a literal address.
  // 0x2 - ARGC is a literal integer.
  case CALL: {
    exec_call(deref(op[0], litflag, 0x1), deref(op[1], litflag, 0x2));
  } break;

  // RETURN
  case RETURN: {
    esp() = ebp();
    --_sequence;
  } break;

  // REALLOC OUT, IN, SIZE, LITFLAG
  // Reallocates a block of SIZE bytes and writes its ID to the OUT register.
  // Litflags:
  // 0x4 - Size
  case REALLOC: {
    reg(op[0]) = realloc_block(op[1], deref(op[2], litflag, 0x4));
  } break;

  // FREE BLOCKID
  // Frees the block whose ID is held in the given register and zeroes the
  // register.
  case FREE: {
    free_block(reg(op[0]));
    reg(op[0]) = 0.0;
  } break;

  // PEEK OUT, LR(BLOCKID), LR(OFFSET), LR(TYPE), LITFLAG
  // Peeks a value of type TYPE from the block at the given OFFSET and writes
  // the result to OUT.
  // Litflags:
  //  0x2 - blockid
  //  0x4 - offset
  //  0x8 - type
  case PEEK: {
    value_t &out = reg(op[0]);
    int32_t const block_id = deref(op[1], litflag, 0x2);
    int32_t const offset = deref(op[2], litflag, 0x4);
    memop_typed_t const type = (memop_typed_t)deref(op[3], litflag, 0x8).i32();
    int8_t const *ro_block = reinterpret_cast<int8_t const *>(get_block(block_id, VM_MEM_READABLE));

    if (!(ro_block && check_block_bounds(block_id, offset, MEMOP_SIZE[type]))) {
      std::abort();
    }

    ro_block += offset;

    switch (type) {
    case MEMOP_UINT8:   out = *(uint8_t const *)ro_block;  break;
    case MEMOP_INT8:    out = *(int8_t const *)ro_block;   break;
    case MEMOP_UINT16:  out = *(uint16_t const *)ro_block; break;
    case MEMOP_INT16:   out = *(int16_t const *)ro_block;  break;
    case MEMOP_UINT32:  out = *(uint32_t const *)ro_block; break;
    case MEMOP_INT32:   out = *(int32_t const *)ro_block;  break;
    // 64-bit integral types are only partially supported at the moment (may change later).
    case MEMOP_UINT64:  out = *(uint64_t const *)ro_block; break;
    case MEMOP_INT64:   out = *(int64_t const *)ro_block;  break;
    case MEMOP_FLOAT32: out = *(float const *)ro_block;    break;
    case MEMOP_FLOAT64: out = *(double const *)ro_block;   break;
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
    int32_t const block_id = reg(op[0]);
    value = deref(op[1], litflag, 0x2);
    int32_t const offset = deref(op[2], litflag, 0x4);
    memop_typed_t const type = static_cast<memop_typed_t>(deref(op[3], litflag, 0x8).i32());
    int8_t *rw_block = reinterpret_cast<int8_t *>(get_block(block_id, VM_MEM_WRITABLE));

    if (!(rw_block && check_block_bounds(block_id, offset, MEMOP_SIZE[type]))) {
      std::abort();
    }

    rw_block += offset;

    switch (type) {
    case MEMOP_UINT8:   *(uint8_t *)rw_block  = value.ui8();  break;
    case MEMOP_INT8:    *(int8_t *)rw_block   = value.i8();   break;
    case MEMOP_UINT16:  *(uint16_t *)rw_block = value.ui16(); break;
    case MEMOP_INT16:   *(int16_t *)rw_block  = value.i16();  break;
    case MEMOP_UINT32:  *(uint32_t *)rw_block = value.ui32(); break;
    case MEMOP_INT32:   *(int32_t *)rw_block  = value.i32();  break;
    // 64-bit integral types are only partially supported at the moment (may change later).
    case MEMOP_UINT64:  *(uint64_t *)rw_block = value.ui64(); break;
    case MEMOP_INT64:   *(int64_t *)rw_block  = value.i64();  break;
    case MEMOP_FLOAT32: *(float *)rw_block    = value.f32();  break;
    case MEMOP_FLOAT64: *(double *)rw_block   = value.value;  break;
    default: /* invalid type */ std::abort(); break;
    }
  } break;

  // MEMMOVE BLOCKOUT, OUTOFFSET, BLOCKIN, INOFFSET, SIZE, LITFLAG
  // Just calls memmove for the blocks at the out/in registers. Offsets and
  // size may optionally be literals if their argument flags are set in LITFLAG.
  // Litflags:
  // 0x02 - out offset
  // 0x04 - block in
  // 0x08 - in offset
  // 0x10 - size
  case MEMMOVE: {
    int32_t const dst_block_id = reg(op[0]);
    int32_t const dst_offset = deref(op[1], litflag, 0x2);
    int32_t const src_block_id = deref(reg(op[2]), litflag, 0x4);
    int32_t const src_offset = deref(op[3], litflag, 0x8);
    int32_t const size = deref(op[4], litflag, 0x10);

    if (size > 0 && dst_offset >= 0 && src_offset >= 0) {
      int8_t *block_out;
      int8_t const *block_in;

      // check dst block
      block_out = reinterpret_cast<int8_t *>(get_block(dst_block_id, VM_MEM_READ_WRITE));
      if (!(block_out && check_block_bounds(dst_block_id, dst_offset, size))) {
        std::abort();
      }
      block_out += dst_offset;

      block_in = reinterpret_cast<int8_t const *>(get_block(src_block_id, VM_MEM_READABLE));
      if (!(block_in && check_block_bounds(src_block_id, src_offset, size))) {
        std::abort();
      }
      block_in += src_offset;

      std::memmove(block_out, block_in, size);
    }
  } break;

  // MEMDUP OUT, BLOCKID, LITFLAG
  // Allocates a new block of at least the same length as that pointed to by
  // the register or literal BLOCKID, copies the original block's data to the
  // new block, and writes the new block id to OUT.
  // Litflags:
  // 0x2 - blockid
  case MEMDUP: {
    reg(op[0]) = duplicate_block(deref(op[1], litflag, 0x2));
  } break;

  // MEMLEN OUT, BLOCKID, LITFLAG
  // Writes the length in bytes of the memory block referred to by the contents
  // of the BLOCKID register or literal to OUT.
  // Litflags:
  // 0x2 - blockid
  case MEMLEN: {
    reg(op[0]) = block_size(deref(op[1], litflag, 0x2));
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
    throw vm_bad_opcode("Invalid opcode");
  }

  #ifdef LOG_STATE_CHANGES
  dump_registers();
  dump_stack();
  #endif
}


vm_fn_find_result_t vm_state_t::find_function_pointer(const char *name) const
{
  const std::string str_name((name));
  vm_unit_t::label_table_t::const_iterator iter = _unit.imports.find(name);
  if (iter == _unit.imports.cend() &&
      (iter = _unit.exports.find(name)) == _unit.exports.cend()) {
    return vm_fn_find_result_t { false, 0 };
  }
  return vm_fn_find_result_t { true, iter->second };
}


value_t vm_state_t::call_function_nt(const char *name, int32_t argc, const value_t *argv)
{
  const auto pointer = find_function_pointer(name);
  // if (!pointer.first) throw std::runtime_error("no such function");
  return call_function_nt(pointer.second, argc, argv);
}


value_t vm_state_t::call_function_nt(int32_t pointer, int32_t argc, const value_t *argv)
{
  for (int32_t arg_index = 0; arg_index < argc; ++arg_index) {
    push(argv[arg_index]);
  }
  return call_function_nt(pointer, argc);
}


value_t vm_state_t::call_function_nt(int32_t pointer, int32_t num_args)
{
  exec_call(pointer, num_args);
  return rp();
}


value_t vm_state_t::stack(int32_t loc) const
{
  if (loc < 0) {
    std::abort();
  } else if (static_cast<size_t>(loc) >= _stack.size()) {
    return value_t { 0 };
  }

  return _stack[loc];
}


value_t &vm_state_t::stack(int32_t loc)
{
  if (loc < 0) {
    std::abort();
  } else if (static_cast<size_t>(loc) >= _stack.size()) {
    _stack.resize(loc + 1);
  }

  return _stack[loc];
}


void vm_state_t::exec_call(int32_t pointer, int32_t argc)
{
  // preserve nonvolatile registers
  std::array<value_t, R_NONVOLATILE_REGISTERS> nonvolatile_reg;
  auto first_preserved = std::begin(_registers) + R_FIRST_NONVOLATILE;
  auto last_preserved = first_preserved + R_NONVOLATILE_REGISTERS;
  if (R_NONVOLATILE_REGISTERS > 0) {
    std::copy(first_preserved, last_preserved, std::begin(nonvolatile_reg));
  }

  value_t const preserved_ip = ip();
  value_t const preserved_ebp = ebp();
  ebp() = esp().i32() - argc;
  value_t const preserved_esp = ebp();

  if (pointer < 0) {
    vm_callback_t *callback = _callbacks[-(pointer + 1)];
    ++_sequence;

    if (argc <= 0) {
      rp() = callback(*this, 0, nullptr);
    } else {
      stack_t argv;
      argv.reserve(argc);
      for (int32_t argi = 0; argi < argc; ++argi) {
        argv.push_back(pop());
      }
      rp() = callback(*this, argc, &argv[0]);
    }

    --_sequence;
  } else {
    ip() = pointer;
    run();
  }

  // restore nonvolatiles
  if (R_NONVOLATILE_REGISTERS > 0) {
    std::copy(std::begin(nonvolatile_reg), std::end(nonvolatile_reg), first_preserved);
  }

  ip() = preserved_ip;
  ebp() = preserved_ebp;
  esp() = preserved_esp;
}


bool vm_state_t::check_block_bounds(int32_t block_id, int32_t offset, int32_t size) const
{
  int32_t const bsize = block_size(block_id);
  int32_t const end = offset + size;

  return
    offset >= 0 &&
    size >= 0 &&
    size <= bsize &&
    end <= bsize &&
    end >= offset;
}


void vm_state_t::push(value_t value)
{
  stack(esp().i32()) = value;
  esp() = esp().i32() + 1;
}


value_t vm_state_t::pop(bool copy_only)
{
  int32_t stack_top = esp().i32() - 1;
  value_t result = stack(stack_top);
  if (!copy_only) {
    esp() = stack_top;
  }
  return result;
}


void vm_state_t::dump_registers(size_t count) const
{
  uint32_t index = 0;
  for (; index < count && index < REGISTER_COUNT; ++index) {
    const value_t &regval = _registers[index];
    std::clog << std::setw(2) << index << " -> " << regval << std::endl;
  }
}


void vm_state_t::dump_stack(size_t until) const
{
  uint32_t index = 0;
  for (; index < until && index < _stack.size(); ++index) {
    const value_t &stackval = _stack[index];

    std::clog << "stack[" << index << "]  " << stackval << std::endl;
  }
}


value_t vm_state_t::reg(int32_t off) const
{
  if (off >= 0) {
    if (off >= REGISTER_COUNT) {
      throw vm_bad_register("Invalid register offset.");
    }
    return _registers[off];
  } else {
    off = esp().i32() + off;
    if (off < 0) {
      throw vm_bad_register("Invalid relative stack offset.");
    }
    return _stack[off];
  }
}


value_t &vm_state_t::reg(int32_t off)
{
  if (off >= 0) {
    if (off >= REGISTER_COUNT) {
      throw vm_bad_register("Invalid register offset.");
    }
    return _registers[off];
  } else {
    off = esp().i32() + off;
    if (off < 0) {
      throw vm_bad_register("Invalid relative stack offset.");
    }
    return _stack[off];
  }
}
