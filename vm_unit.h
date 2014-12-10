/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

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

/**
 * VERSIONS (i.e., changes in bytecode versions)
 *
 *   - 8:
 *     All values are 64-bit floats (doubles).
 *
 *   - 9:
 *     Adds typed values as 32-bit signed integers. Values should be read as
 *     a signed 32-bit type ID integer followed by a 8-byte value data.
 */


/** Defined unit chunk names. */
enum vm_chunk_id : int32_t
{
  // TODO: Stop using four-char integers.
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



/**
 * vm_unit is a unit of loaded Rusalka bytecode. It defines all instructions,
 * operands, relocations, and static data needed to execute arbitrary Rusalka
 * bytecode. It also handles loading said bytecode from a stream.
 *
 * A unit may be the collation of one or more actual bytecode units (as read
 * from a stream). Each subsequently-loaded unit has its data relocated as
 * needed.
 */
class vm_unit
{
  friend class vm_op;
  friend class vm_state;

  /**
   * A relocation marker. Defines which instruction needs relocation and which
   * operands (arguments) must be relocated.
   */
  struct relocation_ptr
  {
    // Index of the instruction to relocate relative to the instructions
    // table.
    int64_t pointer;
    // Mask indicating which arguments are to be relocated.
    uint64_t args_mask;
  };
  static relocation_ptr read_relocation_ptr(std::istream &);

  /**
   * An instruction in the unit. Contains the opcode of the instruction, which
   * operands are constant values, and a pointer to the first operand of the
   * instruction in the unit's value array.
   */
  struct instruction_ptr
  {
    vm_opcode opcode;
    uint64_t  litflag;
    int64_t   arg_pointer;
  };

  /**
   * A static data block in the unit. Has a predefined ID, an offset, and the
   * size of the data block.
   */
  struct data_block
  {
    int64_t id;
    int64_t offset; // offset into _data
    int64_t size;   // size in bytes of the block
  };

  /**
   * An extern relocation marker. The pointer is the thing to be relocated,
   * resolved is whether the relocation has been resolved.
   *
   * An extern is defined as a reference to any outside operand that is defined
   * in a unit other than the one currently being loaded into the vm_unit.
   */
  struct extern_relocation
  {
    vm_value pointer;
    bool resolved;
  };

  using value_reader_t       = vm_value (std::istream &);
  using relocation_table_t   = std::vector<relocation_ptr>;
  using relocation_map_t     = std::map<vm_value, vm_value>;
  // Externs may be relocated in two ways:
  // 1) The extern might just need to be adjusted because there are prior
  //  unresolved externs, in which case the second field is false (unresolved).
  // 2) The extern might've been resolved in the process of loading the extern
  //  table, so the second field is true (resolved).
  using extern_relocations_t = std::map<vm_value, extern_relocation>;

  using instruction_ptrs_t   = std::vector<instruction_ptr>;
  using instruction_argv_t   = std::vector<vm_value>;
  using label_table_t        = std::map<uint64_t, int64_t>;
  using data_id_ary_t        = std::vector<int64_t>;

  /** The last version code the unit was loaded with. Currently unused. */
  int32_t version;
  int64_t last_import = 0;

  /** The instructions contained by the unit. */
  instruction_ptrs_t instructions;
  /**
   * All operand constants used by the unit. This includes both constant
   * values, register numbers, instruction pointers, data references, and so
   * on -- anything that is an operand to an instruction.
   */
  instruction_argv_t instruction_argv;

  label_table_t imports;
  label_table_t exports;
  label_table_t externs; // only contains unresolved externs
  relocation_table_t unresolved_relocations;

  std::vector<uint8_t> _data;
  std::vector<data_block> _data_blocks;
  relocation_table_t _data_relocations;

  void read_instruction(std::istream &input);
  void read_instructions(std::istream &input);

  void read_imports(std::istream &input, relocation_map_t &relocations);
  void read_exports(
    std::istream &input,
    int64_t base,
    relocation_map_t &relocations
    );

  void read_externs(std::istream &input, extern_relocations_t &relocations);

  void read_label_relocations(
    std::istream &input,
    int64_t instruction_base,
    relocation_map_t const &relocations
    );

  void read_extern_relocations(
    std::istream &input,
    int64_t instruction_base,
    extern_relocations_t const &relocations
    );

  void resolve_externs();

  void read_data_table(
    std::istream &input,
    int64_t data_base,
    relocation_map_t &relocations
    );

  void read_data_relocations(
    std::istream &input,
    int64_t instr_base,
    int64_t data_base,
    relocation_map_t &load_relocations
    );

  bool relocate_static_data(data_id_ary_t const &new_ids);

  void apply_instruction_relocation(
    relocation_ptr rel,
    relocation_map_t const &relocations
    );
  void apply_relocation_table(
    relocation_table_t const &table,
    relocation_map_t const &relocations
    );

  value_reader_t *value_reader() const;

  static vm_value read_value_v8(std::istream &);
  static vm_value read_value_v9(std::istream &);

public:

  vm_unit();
  vm_unit(vm_unit const &m);
  vm_unit(vm_unit &&m);
  ~vm_unit() = default;

  vm_unit &operator = (vm_unit const &m);
  vm_unit &operator = (vm_unit &&m);

  // Reads a unit and links it into this unit.
  void read(std::istream &input);

  bool is_valid() const
  {
    return externs.size() == 0 && unresolved_relocations.size() == 0;
  }

  void debug_write_instructions(std::ostream &out) const;

  vm_op fetch_op(int64_t ip) const;

  template <typename Func>
  void each_data(Func &&fn) const;
};



template <typename Func>
void vm_unit::each_data(Func &&fn) const
{
  int index = 0;
  bool stop = false;
  for (data_block const &blk : _data_blocks) {
    fn(index++, blk.id, blk.size, (void const *)&_data[blk.offset], stop);
    if (stop) {
      return;
    }
  }
}
