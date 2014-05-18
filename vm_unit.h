#ifndef __VM_UNIT_H__
#define __VM_UNIT_H__

#include <cstdint>
#include <array>
#include <iterator>
#include <map>
#include <vector>
#include "vm_opcode.h"
#include "vm_value.h"


#define VM_MIN_UNIT_VERSION 8
#define VM_MAX_UNIT_VERSION 200


enum chunk_id_t : int32_t
{
  CHUNK_VERS = 'SREV',
  CHUNK_OFFS = 'SFFO',
  CHUNK_EREL = 'LERE',
  CHUNK_LREL = 'LERL',
  CHUNK_DREL = 'LERD',
  CHUNK_DATA = 'ATAD',
  CHUNK_IMPT = 'TPMI',
  CHUNK_EXPT = 'TPXE',
  CHUNK_EXTS = 'STXE',
  CHUNK_INST = 'TSNI',
};


struct vm_instruction_t
{
  const value_t *argv;
  opcode_t opcode;
  // Pointer to argv data for the opcode. Opcodes have a fixed number of
  // arguments, so it's imperative that you never read more arguments than
  // are provided by the instruction/for the opcode.

  // Convenience member operator for op_t compatibility.
  value_t operator[] (int index) const { return argv[index]; }

  operator opcode_t () const { return opcode; }
};


class vm_unit_t
{

  struct relocation_ptr_t
  {
    // Index of the instruction to relocate relative to the instructions
    // table.
    int32_t pointer;
    // Mask indicating which arguments are to be relocated.
    uint32_t args_mask;
  };

  using relocation_table_t = std::vector<relocation_ptr_t>;
  using label_relocations_t = std::map<int32_t, int32_t>;

  struct instruction_ptr_t
  {
    opcode_t  opcode;
    int32_t   arg_pointer;
  };

  using instruction_ptrs_t = std::vector<instruction_ptr_t>;
  using instruction_argv_t = std::vector<value_t>;

  using data_t = std::string;
  using label_table_t = std::map<std::string, int32_t>;
  using data_table_t = std::map<int32_t, data_t>;

  int32_t last_import = 0;

  instruction_ptrs_t instructions;
  instruction_argv_t instruction_argv;

  label_table_t imports;
  label_table_t exports;
  label_table_t externs;

  void read_instruction(std::istream &input);
  void read_instructions(std::istream &input);

  void read_imports(std::istream &input, label_relocations_t &relocations);
  void read_exports(std::istream &input, int32_t base, label_relocations_t &relocations);

  void read_externs(std::istream &input);

  void read_label_relocations(
    std::istream &input,
    int32_t instruction_base,
    label_relocations_t const &relocations
    );

  void resolve_externs();

  static void read_labels(std::istream &input, chunk_id_t id, label_table_t &table);

public:

  vm_unit_t();
  ~vm_unit_t();

  // Reads a unit and links it into this unit.
  void read(std::istream &input);

  void merge_unit(const vm_unit_t &unit);

  vm_instruction_t instruction(int32_t pointer) const;
};


#endif /* end __VM_UNIT_H__ include guard */
