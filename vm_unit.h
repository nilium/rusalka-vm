#ifndef __VM_UNIT_H__
#define __VM_UNIT_H__

#include <cstdint>
#include <array>
#include <iterator>
#include <map>
#include <vector>
#include "vm_opcode.h"
#include "vm_op.h"
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
  friend class op_t;
  friend class vm_state_t;

  struct relocation_ptr_t
  {
    // Index of the instruction to relocate relative to the instructions
    // table.
    int32_t pointer;
    // Mask indicating which arguments are to be relocated.
    uint32_t args_mask;
  };

  struct instruction_ptr_t
  {
    opcode_t  opcode;
    int32_t   arg_pointer;
  };

  struct data_block_t
  {
    int32_t id;
    int32_t offset; // offset into _data
    int32_t size;   // size in bytes of the block
  };

  using relocation_table_t = std::vector<relocation_ptr_t>;
  using relocation_map_t = std::map<int32_t, int32_t>;
  // Externs may be relocated in two ways:
  // 1) The extern might just need to be adjusted because there are prior
  //  unresolved externs, in which case the second field is false (unresolved).
  // 2) The extern might've been resolved in the process of loading the extern
  //  table, so the second field is true (resolved).
  using extern_relocation_t = std::pair<int32_t /* new_address */, bool /* resolved */>;
  using extern_relocations_t = std::map<int32_t, extern_relocation_t>;

  using instruction_ptrs_t = std::vector<instruction_ptr_t>;
  using instruction_argv_t = std::vector<value_t>;
  using label_table_t = std::map<std::string, int32_t>;
  using data_id_ary_t = std::vector<int32_t>;

  int32_t last_import = 0;

  instruction_ptrs_t instructions;
  instruction_argv_t instruction_argv;

  label_table_t imports;
  label_table_t exports;
  label_table_t externs; // only contains unresolved externs
  relocation_table_t unresolved_relocations;

  std::vector<uint8_t> _data;
  std::vector<data_block_t> _data_blocks;
  relocation_table_t _data_relocations;

  void read_instruction(std::istream &input);
  void read_instructions(std::istream &input);

  void read_imports(std::istream &input, relocation_map_t &relocations);
  void read_exports(std::istream &input, int32_t base, relocation_map_t &relocations);

  void read_externs(std::istream &input, extern_relocations_t &relocations);

  void read_label_relocations(
    std::istream &input,
    int32_t instruction_base,
    relocation_map_t const &relocations
    );

  void read_extern_relocations(
    std::istream &input,
    int32_t instruction_base,
    extern_relocations_t const &relocations
    );

  void resolve_externs();

  void read_data_table(
    std::istream &input,
    int32_t data_base,
    relocation_map_t &relocations
    );

  void read_data_relocations(
    std::istream &input,
    int32_t instr_base,
    int32_t data_base,
    relocation_map_t &load_relocations
    );

  void relocate_static_data(data_id_ary_t const &pointers);

public:

  vm_unit_t();
  ~vm_unit_t();

  // Reads a unit and links it into this unit.
  void read(std::istream &input);

  bool is_valid() const
  {
    return externs.size() == 0 && unresolved_relocations.size() == 0;
  }

  void debug_write_instructions(std::ostream &out) const;

  void merge_unit(const vm_unit_t &unit);

  vm_instruction_t instruction(int32_t pointer) const;

  template <typename Func>
  void each_data(Func &&fn) const;

  template <typename Func>
  void each_data(Func &&fn);
};



template <typename Func>
void vm_unit_t::each_data(Func &&fn)
{
  int32_t index = 0;
  bool stop = false;
  for (data_block_t const &blk : _data_blocks) {
    fn(index++, blk.id, blk.size, (void const *)&_data[blk.offset], stop);
    if (stop) return;
  }
}



template <typename Func>
void vm_unit_t::each_data(Func &&fn) const
{
  int32_t index = 0;
  bool stop = false;
  for (data_block_t const &blk : _data_blocks) {
    fn(index++, blk.id, blk.size, (void const *)&_data[blk.offset], stop);
    if (stop) return;
  }
}


#endif /* end __VM_UNIT_H__ include guard */
