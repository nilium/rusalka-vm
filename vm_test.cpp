/*
 *          Copyright Noel Cower 2013 - 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#include "vm_state.h"
#include "vm_unit.h"
#include <fstream>


value_t printfn(vm_state_t &vm, int32_t argc, const value_t *argv)
{
  std::cerr << "PRINTI: ";
  for (; argc > 0; --argc, ++argv) {
    std::cerr << argv->i32() << ' ';
  }
  std::cerr << std::endl;
  return value_t { 0 };
}

value_t printsfn(vm_state_t &vm, int32_t argc, const value_t *argv)
{
  std::cerr << "PRINTS: ";
  for (; argc > 0; --argc, ++argv) {
    const char *ptr = reinterpret_cast<const char *>(vm.get_block(argv->i32(), VM_MEM_READABLE));
    int32_t size = vm.block_size(*argv);
    if (!ptr || !size) {
      std::cerr << "<null> ";
      continue;
    }
    std::cerr << std::string(ptr, size) << ' ';
  }
  std::cerr << std::endl;
  return value_t { 0 };
}


template <typename T, typename FN>
auto with(T &&obj, FN fn) -> decltype(fn(obj))
{
  return fn(obj);
}


int main(int argc, char const *argv[])
{
  vm_unit_t unit;

  std::cerr << "Unit is valid: " << unit.is_valid() << std::endl;

  {
    std::cerr << "Opening stream (test.asm.bc x1)." << std::endl;
    std::fstream stream ("test.asm.bc", std::ios_base::in);
    std::cerr << "Reading bytecode." << std::endl;
    unit.read(stream);
  }

  unit.debug_write_instructions(std::cerr);

  std::cerr << "Unit is valid: " << unit.is_valid() << std::endl;

  vm_state_t vm;
  vm.set_unit(std::move(unit));
  vm.bind_callback("print", printfn);
  vm.bind_callback("prints", printsfn);
  double fv = vm.function("__main__")(-123.456);
  std::clog << "Returned: " << fv << std::endl;

  #ifdef LOG_FINAL_STATE
  vm.dump_registers();
  vm.dump_stack();
  #endif

  return 0;
}
