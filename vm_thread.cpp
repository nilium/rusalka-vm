/*
 *          Copyright Noel Cower 2013 - 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#include <algorithm>
#include <functional>
#include <cfenv>
#include <cmath>
#include <iostream>
#include <iomanip>

#include "vm_thread.h"
#include "vm_state.h"
#include "vm_op.h"
#include "vm_opcode.h"
#include "vm_exception.h"


#ifndef VM_MAX_JOIN_LOOPS
#define VM_MAX_JOIN_LOOPS 4
#endif


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


namespace {
enum memop_typed_t : int64_t
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



/**
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



/**
 * Constructs a new thread for the given process with the stack size provided.
 */
vm_thread::vm_thread(vm_state &process, size_t stack_size)
: _process(process)
{
  for (int index = 0; index < REGISTER_COUNT; ++index) {
    _registers[index] = 0.0;
  }

  _stack.resize(stack_size);
}



/**
 * Looks up a function by name and returns a vm_function object bound to the
 * receiving thread to call that function.
 */
vm_function vm_thread::function(const char *name)
{
  const auto pointer = find_function_pointer(name);
  // if (!pointer.ok) throw std::runtime_error("no such function");
  return vm_function { *this, pointer.value };
}



/**
 * Returns a vm_function object bound to the current thread for the given
 * instruction pointer. The instruction pointer is not checked for whether it
 * is actually the start of a function.
 */
vm_function vm_thread::function(int64_t pointer)
{
  return vm_function { *this, pointer };
}



/**
 * Fetches the instruction pointer to be executed next and advances the
 * instruction pointer (as such, the IP returned and the IP register values
 * are different).
 */
int64_t vm_thread::fetch()
{
  const int64_t next_instr = ip();
  ip() = next_instr + 1;
  if (next_instr < 0 || next_instr >= _process._source_size) {
    ++_trap;
  }
  return next_instr;
}



/**
 * Runs the VM thread from the given instruction pointer onward. The IP must be
 * a valid pointer into the unit and may not be bound callback (from_ip >= 0).
 *
 * Returns false if a TRAP was encountered, true otherwise.
 */
bool vm_thread::run(int64_t from_ip)
{
  ip() = from_ip;
  return run();
}



/**
 * Runs the VM thread from its current instruction pointer.
 *
 * Returns false if a TRAP was encountered, true otherwise.
 */
bool vm_thread::run()
{
  const int64_t term_sequence = _sequence++;
  while (!_trap && term_sequence < _sequence) {
    int64_t const opidx = fetch();
    exec(_process._unit.fetch_op(opidx));
  }
  bool const good = _trap == 0;
  _trap = 0;
  return good;
}



/**
 * Dereferences an input value as either a constant or register, depending on
 * the provided flags and mask.
 */
vm_value vm_thread::deref(vm_value input, uint64_t flag, uint64_t mask) const
{
  return (flag & mask) ? input : reg(input);
}



/**
 * Convenience function for performing a bitwise shift against a numeric value.
 */
template <typename T>
constexpr T vm_shift(T num, int64_t shift)
{
  return
    shift == 0
    ? num
    : ((shift > 0) ? (num << shift) : (num >> (-shift)));
}



/**
 * Executes the given vm_op against this vm_thread.
 */
void vm_thread::exec(const vm_op &op)
{
  vm_value value;
  uint16_t const litflag = op.litflag();

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
  // Negation.
  case NEG: {
    reg(op[0]) = -reg(op[1]);
  } break;

  // NOT OUT, IN
  // Bitwise not (unsigned).
  case NOT: {
    reg(op[0]) = ~(reg(op[1]));
  } break;

  // OR OUT, LHS, RHS, LITFLAG
  // Bitwise or (unsigned).
  case OR: {
    reg(op[0]) = deref(op[1], litflag, 0x2) | deref(op[2], litflag, 0x4);
  } break;

  // AND OUT, LHS, RHS, LITFLAG
  // Bitwise and (unsigned).
  case AND: {
    reg(op[0]) = deref(op[1], litflag, 0x2) & deref(op[2], litflag, 0x4);
  } break;

  // XOR OUT, LHS, RHS, LITFLAG
  // Bitwise xor (unsigned).
  case XOR: {
    reg(op[0]) = deref(op[1], litflag, 0x2) ^ deref(op[2], litflag, 0x4);
  } break;

  // ARITHSHIFT OUT, LHS (signed), RHS (unsigned), LITFLAG
  // Arithmetic shift. Signed.
  // RHS > 0  -> Left shift.
  // RHS < 0  -> Right shift.
  // RHS == 0 -> Cast to signed int.
  case ARITHSHIFT: {
    const int64_t input = deref(op[1], litflag, 0x2);
    const int64_t shift = deref(op[2], litflag, 0x4);
    reg(op[0]) = vm_shift(input, shift);
  } break;

  // BITSHIFT OUT, LHS (unsigned), RHS (signed), LITFLAG
  // Bitwise shift. Signed 32-bit.
  // RHS > 0  -> Left shift.
  // RHS < 0  -> Right shift.
  // RHS == 0 -> Cast to unsigned 32-bit int.
  case BITSHIFT: {
    const uint64_t input = deref(op[1], litflag, 0x2);
    const int64_t shift = deref(op[2], litflag, 0x4);
    reg(op[0]) = vm_shift(input, shift);
  } break;

  // FLOOR OUT, IN
  // Nearest integral value <= IN.
  case FLOOR: {
    vm_value const in = reg(op[1]);
    if (in.type != vm_value::FLOAT) {
      reg(op[0]) = in.as(vm_value::FLOAT);
    } else {
      reg(op[0]) = std::floor(in.f64());
    }
  } break;

  // CEIL OUT, IN
  // Nearest integral value >= IN.
  case CEIL: {
    vm_value const in = reg(op[1]);
    if (in.type != vm_value::FLOAT) {
      reg(op[0]) = in.as(vm_value::FLOAT);
    } else {
      reg(op[0]) = std::ceil(in.f64());
    }
  } break;

  // ROUND OUT, IN
  // Nearest integral value using FE_TONEAREST.
  case ROUND: {
    vm_value const in = reg(op[1]);
    if (in.type != vm_value::FLOAT) {
      reg(op[0]) = in.as(vm_value::FLOAT);
    } else {
      with_rounding(FE_TONEAREST, [&] {
        reg(op[0]) = std::nearbyint(in.f64());
      });
    }
  } break;

  // RINT OUT, IN
  // Nearest integral value using FE_TOWARDZERO.
  case RINT: {
    vm_value const in = reg(op[1]);
    if (in.type != vm_value::FLOAT) {
      reg(op[0]) = in.as(vm_value::FLOAT);
    } else {
      with_rounding(FE_TOWARDZERO, [&] {
        reg(op[0]) = std::nearbyint(in.f64());
      });
    }
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
    if ((deref(op[0], litflag, 0x1) == deref(op[1], litflag, 0x2)) != (op[2] != 0)) {
      ip() = ip() + 1;
    }
  } break;

  case LT: {
    if ((deref(op[0], litflag, 0x1) < deref(op[1], litflag, 0x2)) != (op[2] != 0)) {
      ip() = ip() + 1;
    }
  } break;

  case LE: {
    if ((deref(op[0], litflag, 0x1) <= deref(op[1], litflag, 0x2)) != (op[2] != 0)) {
      ip() = ip() + 1;
    }
  } break;

  // JUMP POINTER, LITFLAG
  // Unconditional jump.
  // Litflags:
  // 0x1 - POINTER is a literal address.
  case JUMP: {
    vm_value const new_ip = deref(op[0], litflag, 0x1).as(vm_value::SIGNED);
    if (new_ip.is_undefined() || new_ip.is_error()) {
      throw vm_invalid_instruction_pointer("Attempt to jump to non-integral instruction pointer");
    }
    ip() = new_ip;
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
    vm_value const new_ip = deref(op[0], litflag, 0x1).as(vm_value::SIGNED);
    vm_value const argc = deref(op[1], litflag, 0x2).as(vm_value::SIGNED);
    if (new_ip.is_undefined() || new_ip.is_error()) {
      throw vm_invalid_instruction_pointer("Attempt to call non-integral instruction pointer");
    } else if (argc.is_undefined() || argc.is_error()) {
      throw vm_invalid_argument_count("Attempt to call instruction pointer with non-integral argument count");
    }
    exec_call(new_ip, argc);
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
    int64_t const block_id = (litflag & 0x2) ? 0 : op[1];
    int64_t const size = deref(op[2], litflag, 0x4);
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
    int64_t const block_id = deref(op[1], litflag, 0x2);
    int64_t const offset = deref(op[2], litflag, 0x4);
    memop_typed_t const type = (memop_typed_t)deref(op[3], litflag, 0x8).i64();
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
    int64_t const block_id = reg(op[0]);
    value = deref(op[1], litflag, 0x2);
    int64_t const offset = deref(op[2], litflag, 0x4);
    memop_typed_t const type = (memop_typed_t)deref(op[3], litflag, 0x8).i64();
    int8_t *rw_block = reinterpret_cast<int8_t *>(_process.get_block(block_id, VM_MEM_WRITABLE));

    if (!rw_block) {
      throw vm_null_access_error("Attempt to write to null block");
    } else if (!_process.check_block_bounds(block_id, offset, MEMOP_SIZE[type])) {
      throw vm_memory_access_error("Attempt to write outside block bounds");
    }

    rw_block += offset;

    switch (type) {
    case MEMOP_UINT8:   *(uint8_t *)rw_block  = value;  break;
    case MEMOP_INT8:    *(int8_t *)rw_block   = value;   break;
    case MEMOP_UINT16:  *(uint16_t *)rw_block = value; break;
    case MEMOP_INT16:   *(int16_t *)rw_block  = value;  break;
    case MEMOP_UINT32:  *(uint32_t *)rw_block = value; break;
    case MEMOP_INT32:   *(int32_t *)rw_block  = value;  break;
    // 64-bit integral types are only partially supported at the moment (may change later).
    case MEMOP_UINT64:  *(uint64_t *)rw_block = value; break;
    case MEMOP_INT64:   *(int64_t *)rw_block  = value;  break;
    case MEMOP_FLOAT32: *(float *)rw_block    = value;  break;
    case MEMOP_FLOAT64: *(double *)rw_block   = value;  break;
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
    int64_t const dst_block_id = reg(op[0]);
    int64_t const dst_offset = deref(op[1], litflag, 0x2);
    int64_t const src_block_id = deref(reg(op[2]), litflag, 0x4);
    int64_t const src_offset = deref(op[3], litflag, 0x8);
    int64_t const size = deref(op[4], litflag, 0x10);

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
    int64_t const thread_index = reg(op[0]);
    vm_thread &thread = _process.thread_by_index(thread_index);
    int loops = VM_MAX_JOIN_LOOPS;
    while (loops > 0 && !thread.run()) {
      --loops;
    }
    reg(op[1]) = thread.return_value();
    _process.destroy_thread(thread_index);
  } break;

  case OP_COUNT: ;
    throw vm_bad_opcode("Invalid opcode");
  }
}



/**
 * Looks up a function's instruction pointer by name.
 */
vm_found_fn_t vm_thread::find_function_pointer(const char *name) const
{
  return _process.find_function_pointer(name);
}



/**
 * Executes a CALL with a function's instruction pointer by name and returns
 * the result. The function must exist. The function is called without
 * arguments.
 */
vm_value vm_thread::call_function(const char *name)
{
  const auto pointer = find_function_pointer(name);
  // if (!pointer.ok) throw std::runtime_error("no such function");

  return call_function_nt(pointer.value, 0);
}



/**
 * Calls a function by name with the given argument count and argument array
 * and returns the result.
 */
vm_value vm_thread::call_function_nt(const char *name, int64_t argc, const vm_value *argv)
{
  const auto pointer = find_function_pointer(name);
  // if (!pointer.ok) throw std::runtime_error("no such function");
  return call_function_nt(pointer.value, argc, argv);
}



/**
 * Calls a function by its instruction pointer and returns the result.
 */
vm_value vm_thread::call_function_nt(int64_t pointer, int64_t argc, const vm_value *argv)
{
  for (int64_t arg_index = 0; arg_index < argc; ++arg_index) {
    push(argv[arg_index]);
  }
  return call_function_nt(pointer, argc);
}



/**
 * Calls a function pointer by its instruction pointer and returns the result
 * with the given number of arguments to be popped from the stack. num_args
 * may not be less than zero.
 *
 * The given instruction pointer may be a bound callback.
 */
vm_value vm_thread::call_function_nt(int64_t pointer, int64_t num_args)
{
  exec_call(pointer, num_args);
  if (pointer >= 0) {
    while (!run()) {
      /* nop */
    }
  }
  return rp();
}



/**
 * Returns a copy of a value on the stack at the given location.
 *
 * Stack locations given are absolute, not relative to EBP or ESP.
 */
vm_value vm_thread::stack(int64_t loc) const
{
  if (loc < 0) {
    throw vm_stack_access_error("Attempt to access stack location < 0");
  } else if (static_cast<size_t>(loc) >= _stack.size()) {
    return vm_value { 0 };
  }

  return _stack[loc];
}



/**
 * Returns a write-able reference to a value on the stack at the given location.
 *
 * Stack locations given are absolute, not relative to EBP or ESP.
 */
vm_value &vm_thread::stack(int64_t loc)
{
  if (loc < 0) {
    throw vm_stack_access_error("Attempt to access stack location < 0");
  } else if (static_cast<size_t>(loc) >= _stack.size()) {
    _stack.resize(loc + 1);
  }

  return _stack[loc];
}



/**
 * Descends a stack frame, keeping the given number of arguments (argc) inside
 * the new stack frame.
 * @param argc [description]
 */
void vm_thread::down_frame(int64_t argc)
{
  call_frame frame {
    ip(),
    ebp(),
    esp() - argc,
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



/**
 * Ascends a stack frame, copying value_count values off the stack and pushing
 * them back onto the stack after leaving the current frame.
 */
void vm_thread::up_frame(int64_t value_count)
{
  if (value_count < 0) {
    throw vm_stack_access_error("Attempt to preserve < 0 stack objects.");
  } else if (_frames.size() == 0) {
    throw vm_stack_underflow("Attempt to ascend frame when no frames are recorded.");
  }

  vm_value const *const copied_end = &stack(esp());
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



/**
 * Drops the current stack frame entirely, making no modifications to thread
 * state other than dropping the frame.
 *
 * Currently unused.
 */
void vm_thread::drop_frame()
{
  if (_frames.size() == 0) {
    throw vm_stack_underflow("Attempt to drop frame when no frames are recorded.");
  }

  _sequence = _frames.back().sequence;
  _frames.pop_back();
}



/**
 * Executes a function call. All function calls descend a frame, popping argc
 * values off the stack to be used as arguments for the function call.
 *
 * Optimized calls that don't leave the VM may optionally pass arguments via
 * registers. Stack-passed arguments are required for bound callbacks.
 *
 * The result of a bound callback always overwrites the RP register, whereas
 * VM functions are not required to modify RP.
 */
void vm_thread::exec_call(int64_t pointer, int64_t argc)
{
  if (argc < 0) {
    throw vm_invalid_argument_count("Encountered argument count less than 0");
  } else if (argc > esp()) {
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
      for (int64_t argi = 0; argi < argc; ++argi) {
        argv.push_back(pop());
      }
      rp() = callback.invoke(*this, argc, &argv[0]);
    }

    up_frame(0);
  } else {
    ip() = pointer;
  }
}



/**
 * Pushes a value onto the stack and increments the ESP register.
 */
void vm_thread::push(vm_value value)
{
  stack(esp()) = value;
  esp() = esp() + 1;
}



/**
 * Pops a value off the stack, decrementing the ESP register. If copy_only is
 * true, neither the stack nor ESP are modified and only the topmost value on
 * the stack is returned.
 */
vm_value vm_thread::pop(bool copy_only)
{
  int64_t stack_top = esp().i64() - 1;
  if (stack_top < ebp()) {
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



/**
 * Debugging function. Logs the contents of the first `count` registers to
 * std::clog.
 */
void vm_thread::dump_registers(size_t count) const
{
  size_t index = 0;
  for (; index < count && index < REGISTER_COUNT; ++index) {
    const vm_value &regval = _registers[index];
    std::clog << std::setw(2) << index << " -> " << regval << std::endl;
  }
}



/**
 * Debugging function. Logs the contents of the stack from index 0 to `until`
 * to std::clog.
 */
void vm_thread::dump_stack(size_t until) const
{
  size_t index = 0;
  for (; index < until && index < _stack.size(); ++index) {
    const vm_value &stackval = _stack[index];

    std::clog << "stack[" << index << "]  " << stackval << std::endl;
  }
}



/**
 * Returns a copy of the given register at `off`. If `off` is negative, it's a
 * value from the top of the stack downward (ESP + off, so -1 is the top of the
 * stack).
 */
vm_value vm_thread::reg(int64_t off) const
{
  if (off >= 0) {
    if (off >= REGISTER_COUNT) {
      throw vm_bad_register("Invalid register offset.");
    }
    return _registers[off];
  } else {
    off = esp() + off;
    if (off < 0) {
      throw vm_bad_register("Invalid relative stack offset.");
    }
    return _stack[off];
  }
}



/**
 * Returns a write-able reference for the given register at `off`. If `off` is
 * negative, it's a value from the top of the stack downward.
 */
vm_value &vm_thread::reg(int64_t off)
{
  if (off >= 0) {
    if (off >= REGISTER_COUNT) {
      throw vm_bad_register("Invalid register offset.");
    }
    return _registers[off];
  } else {
    off = esp() + off;
    if (off < 0) {
      throw vm_bad_register("Invalid relative stack offset.");
    }
    return _stack[off];
  }
}



/**
 * Looks up this thread's index in the VM process. Thread indices are not
 * guaranteed to be constant, so this value should not be relied upon as
 * unchanging.
 *
 * @todo Add a unique identifier for thread lookup.
 */
int64_t vm_thread::thread_index() const
{
  auto const start = _process._threads.cbegin();
  auto const end = _process._threads.cend();
  auto const pointer = std::find_if(start, end, [this](vm_state::thread_pointer_t const &p) {
    return p.get() == this;
  });
  if (pointer == end) {
    return -1;
  }
  return static_cast<int64_t>(std::distance(start, pointer));
}
