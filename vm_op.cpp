#include "vm_op.h"
#include "vm_unit.h"


opcode_t op_t::opcode() const
{
  return unit.instructions[ip].opcode;
}


value_t op_t::operator [] (int32_t index) const
{
  int32_t argv_base = unit.instructions[ip].arg_pointer;
  return unit.instruction_argv[argv_base + index];
}


uint16_t op_t::litflag() const
{
  return unit.instructions[ip].litflag;
}
