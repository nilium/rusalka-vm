#ifndef __VM_THREAD_H__
#define __VM_THREAD_H__


#include <vector>

#include "_types.h"
#include "vm_value.h"
#include "vm_function.h"


class op_t;
class vm_state;


class vm_thread
{

  friend class vm_state;

  enum
  {

    R_NONVOLATILE_REGISTERS = 8,

    /**
      RESERVED REGISTERS:
      0 -> ip
      1 -> ebp
      2 -> esp
      3 -> return value
    **/
    R_IP = 0,
    R_EBP = 1,
    R_ESP = 2,
    R_RP = 3,

    R_FIRST_NONVOLATILE,
    R_LAST_NONVOLATILE = R_FIRST_NONVOLATILE + (R_NONVOLATILE_REGISTERS - 1),

    R_FIRST_VOLATILE,

    REGISTER_COUNT = 256,

    R_VOLATILE_REGISTERS = REGISTER_COUNT - R_FIRST_VOLATILE,
  };


  using stack_t = std::vector<value_t>;

  vm_state &_process;
  int32_t _sequence = 0;
  int32_t _trap = 0;
  stack_t _stack;
  value_t _registers[REGISTER_COUNT] {};


  template <class T, class... ARGS>
  int32_t load_registers(int32_t index, T &&first, ARGS&&... args)
  {
    push(make_value(std::forward<T>(first)));
    return load_registers(index + 1, std::forward<ARGS>(args)...);
  }

  template <class T>
  int32_t load_registers(int32_t index, T &&first)
  {
    push(make_value(std::forward<T>(first)));
    return index + 1;
  }

  int32_t load_registers(int32_t index) const
  {
    return index;
  }


  void exec(const op_t &op);
  bool run(int32_t from_ip);
  bool run();

  int32_t fetch();

  value_t ip() const { return _registers[R_IP]; }
  value_t &ip() { return _registers[R_IP]; }

  value_t ebp() const { return _registers[R_EBP]; }
  value_t &ebp() { return _registers[R_EBP]; }

  value_t esp() const { return _registers[R_ESP]; }
  value_t &esp() { return _registers[R_ESP]; }

  value_t rp() const { return _registers[R_RP]; }
  value_t &rp() { return _registers[R_RP]; }

  value_t reg(int32_t off) const;
  value_t &reg(int32_t off);

  value_t stack(int32_t off) const;
  value_t &stack(int32_t off);

  void push(value_t value);
  value_t pop(bool copy_only = false);

  void exec_call(int32_t instr, int32_t argc);

  vm_thread(vm_state &state, size_t stack_size = 8192);

  vm_thread(vm_thread const &) = default;
  vm_thread &operator = (vm_thread const &) = default;

  vm_thread(vm_thread &&) = default;
  vm_thread &operator = (vm_thread &&) = default;

public:

  ~vm_thread() = default;

  // Debugging functions
  void dump_registers(size_t count = REGISTER_COUNT) const;
  void dump_stack(size_t until = SIZE_MAX) const;


  vm_found_fn find_function_pointer(const char *name) const;


  template <class... ARGS>
  value_t call_function(const char *name, ARGS&&... args)
  {
    const auto pointer = find_function_pointer(name);
    // if (!pointer.ok) throw std::runtime_error("no such function");
    return call_function(pointer.value, args...);
  }

  template <class... ARGS>
  value_t call_function(int32_t pointer, ARGS&&... args)
  {
    const int32_t argc = load_registers(4, std::forward<ARGS>(args)...) - 4;
    return call_function_nt(pointer, argc);
  }

  value_t call_function(const char *name);

  value_t call_function_nt(int32_t pointer) { return call_function_nt(pointer, 0); }

  value_t call_function_nt(const char *name, int32_t argc, const value_t *argv);
  value_t call_function_nt(int32_t pointer, int32_t argc, const value_t *argv);
  value_t call_function_nt(int32_t pointer, int32_t argc);

  vm_function_t<vm_thread> function(const char *name);
  vm_function_t<vm_thread> function(int32_t pointer);

  value_t deref(value_t input, uint16_t flag, uint32_t mask = ~0u) const;

  vm_state &process() { return _process; }
  vm_state const &process() const { return _process; }
};


#endif /* end __VM_THREAD_H__ include guard */
