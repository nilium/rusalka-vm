#ifndef __VM_THREAD_H__
#define __VM_THREAD_H__


#include <vector>

#include "_types.h"
#include "vm_value.h"
#include "vm_function.h"


class vm_op;
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


  using stack_t = std::vector<vm_value>;

  vm_state &_process;
  int32_t _sequence = 0;
  int32_t _trap = 0;
  stack_t _stack;
  vm_value _registers[REGISTER_COUNT] {};


  template <class T, class... ARGS>
  int32_t load_registers(int32_t index, T &&first, ARGS&&... args);
  template <class T>
  int32_t load_registers(int32_t index, T &&first);
  int32_t load_registers(int32_t index) const
  {
    return index;
  }


  void exec(const vm_op &op);
  bool run(int32_t from_ip);
  bool run();

  int32_t fetch();

  vm_value ip() const { return _registers[R_IP]; }
  vm_value &ip() { return _registers[R_IP]; }

  vm_value ebp() const { return _registers[R_EBP]; }
  vm_value &ebp() { return _registers[R_EBP]; }

  vm_value esp() const { return _registers[R_ESP]; }
  vm_value &esp() { return _registers[R_ESP]; }

  vm_value rp() const { return _registers[R_RP]; }
  vm_value &rp() { return _registers[R_RP]; }

  vm_value reg(int32_t off) const;
  vm_value &reg(int32_t off);

  vm_value stack(int32_t off) const;
  vm_value &stack(int32_t off);

  void push(vm_value value);
  vm_value pop(bool copy_only = false);

  void exec_call(int32_t instr, int32_t argc);

  vm_thread(vm_state &state, size_t stack_size);

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
  vm_value call_function(const char *name, ARGS&&... args);

  template <class... ARGS>
  vm_value call_function(int32_t pointer, ARGS&&... args);

  vm_value call_function(const char *name);

  vm_value call_function_nt(int32_t pointer) { return call_function_nt(pointer, 0); }

  vm_value call_function_nt(const char *name, int32_t argc, const vm_value *argv);
  vm_value call_function_nt(int32_t pointer, int32_t argc, const vm_value *argv);
  vm_value call_function_nt(int32_t pointer, int32_t argc);

  vm_function_t function(const char *name);
  vm_function_t function(int32_t pointer);

  vm_value deref(vm_value input, uint16_t flag, uint32_t mask = ~0u) const;

  vm_state &process() { return _process; }
  vm_state const &process() const { return _process; }

  int thread_index() const;

  vm_value return_value() const { return rp(); }
};



template <class... ARGS>
vm_value vm_thread::call_function(const char *name, ARGS&&... args)
{
  const auto pointer = find_function_pointer(name);
  // if (!pointer.ok) throw std::runtime_error("no such function");
  return call_function(pointer.value, args...);
}



template <class... ARGS>
vm_value vm_thread::call_function(int32_t pointer, ARGS&&... args)
{
  const int32_t argc = load_registers(4, std::forward<ARGS>(args)...) - 4;
  return call_function_nt(pointer, argc);
}



template <class T, class... ARGS>
int32_t vm_thread::load_registers(int32_t index, T &&first, ARGS&&... args)
{
  push(make_value(std::forward<T>(first)));
  return load_registers(index + 1, std::forward<ARGS>(args)...);
}



template <class T>
int32_t vm_thread::load_registers(int32_t index, T &&first)
{
  push(make_value(std::forward<T>(first)));
  return index + 1;
}



template <class... ARGS>
vm_value vm_invoke_function(vm_thread &thread, int32_t pointer, ARGS &&... args)
{
  return thread.call_function(pointer, std::forward<ARGS>(args)...);
}


#endif /* end __VM_THREAD_H__ include guard */
