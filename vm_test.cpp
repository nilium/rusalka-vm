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

#include "vm_state.h"
#include "vm_unit.h"
#include <fstream>


value_t printfn(vm_state_t &vm, int32_t argc, const value_t *argv) {
  std::cout << "PRINT: ";
  for (; argc > 0; --argc, ++argv) {
    std::cout << *argv << ' ';
  }
  std::cout << std::endl;
  return value_t { 0 };
}

value_t printsfn(vm_state_t &vm, int32_t argc, const value_t *argv) {
  std::cout << "PRINT: ";
  for (; argc > 0; --argc, ++argv) {
    std::cout << (const char *)vm.get_block(argv->i32(), VM_MEM_READABLE) << ' ';
  }
  std::cout << std::endl;
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

  {
    std::cerr << "Opening stream." << std::endl;
    std::fstream stream ("test.asm.bc", std::ios_base::in);
    std::cerr << "Reading bytecode." << std::endl;
    unit.read(stream);
  }

  {
    std::cerr << "Opening stream." << std::endl;
    std::fstream stream ("test.asm.bc", std::ios_base::in);
    std::cerr << "Reading bytecode." << std::endl;
    unit.read(stream);
  }

  // vm_state_t vm(2048);
  // vm.set_source(source_t(std::ifstream("out.bc")));
  // vm.bind_callback("print", printfn);
  // vm.bind_callback("prints", printsfn);
  // float fv = vm.function("main")(2.010101);
  // std::clog << "Returned: " << fv << std::endl;

  // #ifdef LOG_FINAL_STATE
  // vm.dump_registers();
  // vm.dump_stack();
  // #endif

  return 0;
}
