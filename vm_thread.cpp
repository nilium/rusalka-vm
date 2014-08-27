/*
 *          Copyright Noel Cower 2013 - 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#include "vm_thread.h"
#include "vm_state.h"
#include "vm_op.h"
#include "vm_opcode.h"
#include "vm_exception.h"

#include <algorithm>
#include <functional>
#include <cfenv>
#include <cmath>
#include <iostream>
#include <iomanip>



namespace {
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
}


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




vm_thread::vm_thread(vm_state &process, size_t stack_size)
: _process(process)
{
  for (int32_t index = 0; index < REGISTER_COUNT; ++index) {
    _registers[index] = 0.0;
  }

  _stack.resize(stack_size);
}



vm_function vm_thread::function(const char *name)
{
  const auto pointer = find_function_pointer(name);
  // if (!pointer.ok) throw std::runtime_error("no such function");
  return vm_function { *this, pointer.value };
}

vm_function vm_thread::function(int pointer)
{
  return vm_function { *this, pointer };
}



int32_t vm_thread::fetch()
{
  const int32_t next_instr = ip().i32();
  ip() = next_instr + 1;
  if (next_instr < 0 || next_instr >= _process._source_size) {
    ++_trap;
  }
  return next_instr;
}



bool vm_thread::run(int32_t from_ip)
{
  ip() = from_ip;
  return run();
}



bool vm_thread::run()
{
  const int32_t term_sequence = _sequence++;
  while (!_trap && term_sequence < _sequence) {
    int32_t opidx = fetch();
    exec(_process._unit.fetch_op(opidx));
  }
  bool const good = _trap == 0;
  _trap = 0;
  return good;
}



vm_value vm_thread::deref(vm_value input, uint16_t flag, uint32_t mask) const
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



void vm_thread::exec(const vm_op &op)
{
  vm_value value;
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
  // input. See vm_thread::deref for how the test works.
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
  // Executes a call to the function at the given pointer. ARGC indicates the
  // number of arguments on the stack for the receiving function, and is
  // used to adjust esp/ebp accordingly.
  // Litflags:
  // 0x1 - POINTER is a literal address.
  // 0x2 - ARGC is a literal integer.
  case CALL: {
    exec_call(deref(op[0], litflag, 0x1), deref(op[1], litflag, 0x2));
  } break;

  // RETURN -- exits the current frame/sequence
  case RETURN: {
    up_frame(0);
  } break;

  // REALLOC INOUT, IN, SIZE, LITFLAG
  // Reallocates a block of SIZE bytes and writes its ID to the OUT register.
  // Litflags:
  // 0x2 - Block is zero
  // 0x4 - Size
  case REALLOC: {
    int32_t const block_id = (litflag & 0x2) ? 0 : op[1].i32();
    int32_t const size = deref(op[2], litflag, 0x4);
    reg(op[0]) = _process.realloc_block(block_id, size);
  } break;

  // FREE BLOCKID
  // Frees the block whose ID is held in the given register and zeroes the
  // register.
  case FREE: {
    vm_value &to_free = reg(op[0]);
    _process.free_block(to_free);
    to_free = 0.0;
  } break;


  // PEEK OUT, LR(BLOCKID), LR(OFFSET), LR(TYPE), LITFLAG
  // Peeks a value of type TYPE from the block at the given OFFSET and writes
  // the result to OUT.
  // Litflags:
  //  0x2 - blockid
  //  0x4 - offset
  //  0x8 - type
  case PEEK: {
    vm_value &out = reg(op[0]);
    int32_t const block_id = deref(op[1], litflag, 0x2);
    int32_t const offset = deref(op[2], litflag, 0x4);
    memop_typed_t const type = (memop_typed_t)deref(op[3], litflag, 0x8).i32();
    int8_t const *ro_block = reinterpret_cast<int8_t const *>(_process.get_block(block_id, VM_MEM_READABLE));

    if (!ro_block) {
      throw vm_null_access_error("Attempt to read from null block");
    } else if (!_process.check_block_bounds(block_id, offset, MEMOP_SIZE[type])) {
      throw vm_memory_access_error("Attempt to read outside block bounds");
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
    default: /* invalid type */
      throw vm_memory_access_error("Invalid type code for memory read");
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
    int8_t *rw_block = reinterpret_cast<int8_t *>(_process.get_block(block_id, VM_MEM_WRITABLE));

    if (!rw_block) {
      throw vm_null_access_error("Attempt to write to null block");
    } else if (!_process.check_block_bounds(block_id, offset, MEMOP_SIZE[type])) {
      throw vm_memory_access_error("Attempt to write outside block bounds");
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
    default: /* invalid type */
      throw vm_memory_access_error("Invalid type code for memory write");
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
      block_out = reinterpret_cast<int8_t *>(_process.get_block(dst_block_id, VM_MEM_READ_WRITE));
      if (!block_out) {
        throw vm_null_access_error("Attempt to use null block as memmove output");
      } else if (!(block_out && _process.check_block_bounds(dst_block_id, dst_offset, size))) {
        throw vm_memory_access_error("memmove operation is out of bounds for destination block");
      }
      block_out += dst_offset;

      block_in = reinterpret_cast<int8_t const *>(_process.get_block(src_block_id, VM_MEM_READABLE));
      if (!block_in) {
        throw vm_null_access_error("Attempt to use null block as memmove input");
      } else if (!(block_in && _process.check_block_bounds(src_block_id, src_offset, size))) {
        throw vm_memory_access_error("memmove operation is out of bounds for source block");
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
    reg(op[0]) = _process.duplicate_block(deref(op[1], litflag, 0x2));
  } break;

  // MEMLEN OUT, BLOCKID, LITFLAG
  // Writes the length in bytes of the memory block referred to by the contents
  // of the BLOCKID register or literal to OUT.
  // Litflags:
  // 0x2 - blockid
  case MEMLEN: {
    reg(op[0]) = _process.block_size(deref(op[1], litflag, 0x2));
  } break;

  // TRAP
  // Sets the trap flag and returns to the caller. Next run resets the flag.
  case TRAP: {
    _trap = 1;
  } break;

  // DEFER OUT
  // Copies the current thread to a new thread and returns an index to it. This
  // thread is not currently running and must be started using JOIN.
  //
  // The output register of the calling thread is set to the new thread's index
  // to be passed to JOIN while the new thread's output register is set to -1.
  // The output register should be checked to determine if the thread is the
  // caller or callee of the instruction.
  //
  // The resulting deferred thread's call stack is only valid for the function
  // it's currently in (i.e., if the call to defer happens in baz in a call
  // stack of foo -> bar -> baz, foo -> bar is lost). Upon return, the thread
  // exits.
  case DEFER: {
    reg(op[0]) = -1;
    vm_thread &thread = _process.fork_thread(*this);
    reg(op[0]) = thread.thread_index();
  } break;

  // JOIN OUT, THREAD
  // Runs any given thread index and assigns that thread's resulting RP to OUT.
  // Upon completion, THREAD is destroyed.
  case JOIN: {
    int32_t const thread_index = reg(op[0]);
    vm_thread &thread = _process.thread_by_index(thread_index);
    thread.run();
    reg(op[1]) = thread.return_value();
    _process.destroy_thread(thread_index);
  } break;

  case OP_COUNT: ;
    throw vm_bad_opcode("Invalid opcode");
  }

  #ifdef LOG_STATE_CHANGES
  dump_registers();
  dump_stack();
  #endif
}



vm_found_fn_t vm_thread::find_function_pointer(const char *name) const
{
  return _process.find_function_pointer(name);
}



vm_value vm_thread::call_function(const char *name)
{
  const auto pointer = find_function_pointer(name);
  // if (!pointer.ok) throw std::runtime_error("no such function");

  return call_function_nt(pointer.value, 0);
}



vm_value vm_thread::call_function_nt(const char *name, int32_t argc, const vm_value *argv)
{
  const auto pointer = find_function_pointer(name);
  // if (!pointer.ok) throw std::runtime_error("no such function");
  return call_function_nt(pointer.value, argc, argv);
}



vm_value vm_thread::call_function_nt(int32_t pointer, int32_t argc, const vm_value *argv)
{
  for (int32_t arg_index = 0; arg_index < argc; ++arg_index) {
    push(argv[arg_index]);
  }
  return call_function_nt(pointer, argc);
}



vm_value vm_thread::call_function_nt(int32_t pointer, int32_t num_args)
{
  exec_call(pointer, num_args);
  run();
  return rp();
}



vm_value vm_thread::stack(int32_t loc) const
{
  if (loc < 0) {
    throw vm_stack_access_error("Attempt to access stack location < 0");
  } else if (static_cast<size_t>(loc) >= _stack.size()) {
    return vm_value { 0 };
  }

  return _stack[loc];
}



vm_value &vm_thread::stack(int32_t loc)
{
  if (loc < 0) {
    throw vm_stack_access_error("Attempt to access stack location < 0");
  } else if (static_cast<size_t>(loc) >= _stack.size()) {
    _stack.resize(loc + 1);
  }

  return _stack[loc];
}




void vm_thread::down_frame(int32_t argc)
{
  call_frame frame {
    ip(),
    ebp(),
    esp().i32() - argc,
    _sequence,
  };

  ++_sequence;
  ebp() = frame.esp;

  if (R_NONVOLATILE_REGISTERS > 0) {
    auto const copy_register_begin = std::begin(_registers) + R_FIRST_NONVOLATILE;
    auto const copy_register_end = copy_register_begin + R_NONVOLATILE_REGISTERS;
    std::copy(copy_register_begin, copy_register_end, std::begin(frame.registers));
  }

  _frames.push_back(frame);
}



void vm_thread::up_frame(int32_t value_count)
{
  if (value_count < 0) {
    throw vm_stack_access_error("Attempt to preserve < 0 stack objects.");
  } else if (_frames.size() == 0) {
    throw vm_stack_underflow("Attempt to ascend frame when no frames are recorded.");
  }

  vm_value const *const copied_end = &stack(esp().i32());
  vm_value const *const copied_begin = copied_end - value_count;
  stack_t const copied_stack { copied_begin, copied_end };

  call_frame const &frame = _frames.back();

  ip() = frame.from_ip;
  ebp() = frame.ebp;
  esp() = frame.esp;

  for (vm_value const value : copied_stack) {
    push(value);
  }

  _sequence = frame.sequence;
  _frames.pop_back();
}



void vm_thread::drop_frame()
{
  if (_frames.size() == 0) {
    throw vm_stack_underflow("Attempt to drop frame when no frames are recorded.");
  }

  _sequence = _frames.back().sequence;
  _frames.pop_back();
}



void vm_thread::exec_call(int32_t pointer, int32_t argc)
{
  if (argc < 0) {
    throw vm_invalid_argument_count("Encountered argument count less than 0");
  } else if (argc > esp().i32()) {
    throw vm_invalid_argument_count("Encountered argument count greater than ESP");
  }

  down_frame(argc);

  if (pointer < 0) {
    auto callback = _process._callbacks[-(pointer + 1)];

    if (argc <= 0) {
      rp() = callback.invoke(*this, 0, nullptr);
    } else {
      stack_t argv;
      argv.reserve(argc);
      for (int32_t argi = 0; argi < argc; ++argi) {
        argv.push_back(pop());
      }
      rp() = callback.invoke(*this, argc, &argv[0]);
    }

    up_frame(0);
  } else {
    ip() = pointer;
  }
}



void vm_thread::push(vm_value value)
{
  stack(esp().i32()) = value;
  esp() = esp().i32() + 1;
}



vm_value vm_thread::pop(bool copy_only)
{
  int32_t stack_top = esp().i32() - 1;
  if (stack_top < ebp().i32()) {
    throw vm_stack_underflow("Attempt to pop from stack when ESP is EBP");
  } else if (stack_top < 0) {
    throw vm_stack_underflow("Attempt to pop from stack when stack is empty");
  }
  vm_value result = stack(stack_top);
  if (!copy_only) {
    esp() = stack_top;
  }
  return result;
}



void vm_thread::dump_registers(size_t count) const
{
  uint32_t index = 0;
  for (; index < count && index < REGISTER_COUNT; ++index) {
    const vm_value &regval = _registers[index];
    std::clog << std::setw(2) << index << " -> " << regval << std::endl;
  }
}



void vm_thread::dump_stack(size_t until) const
{
  uint32_t index = 0;
  for (; index < until && index < _stack.size(); ++index) {
    const vm_value &stackval = _stack[index];

    std::clog << "stack[" << index << "]  " << stackval << std::endl;
  }
}



vm_value vm_thread::reg(int32_t off) const
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



vm_value &vm_thread::reg(int32_t off)
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


int vm_thread::thread_index() const
{
  auto const start = _process._threads.cbegin();
  auto const end = _process._threads.cend();
  auto const pointer = std::find_if(start, end, [this](vm_state::thread_pointer_t const &p) {
    return p.get() == this;
  });
  if (pointer == end) {
    return -1;
  }
  return static_cast<int>(std::distance(start, pointer));
}
