/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#include "vm_unit.h"


#include "vm_unit+chunk_types.inl"
#include "vm_unit+io.inl"
#include "vm_unit+chunk_offsets.inl"
#include "vm_opcode.h"
#include "vm_exception.h"
#include <set>



// Iterates over each bit in a mask and passes the  of set bits to func.
template <typename T, typename Func>
void
each_in_mask(T mask, Func func)
{
  static_assert(std::is_unsigned<T>::value, "Mask must be unsigned");
  for (int32_t index = 0; mask; index += 1) {
    if (mask & 0x1u) {
      func(index);
    }
    mask >>= 1;
  }
}


vm_unit_t::vm_unit_t()
{
  /* nop */
}


vm_unit_t::vm_unit_t(vm_unit_t const &m)
: last_import(m.last_import)
, instructions(m.instructions)
, instruction_argv(m.instruction_argv)
, imports(m.imports)
, exports(m.exports)
, externs(m.externs)
, unresolved_relocations(m.unresolved_relocations)
, _data(m._data)
, _data_blocks(m._data_blocks)
, _data_relocations(m._data_relocations)
{
  /* nop */
}


vm_unit_t::vm_unit_t(vm_unit_t &&m)
: last_import(m.last_import)
, instructions(std::move(m.instructions))
, instruction_argv(std::move(m.instruction_argv))
, imports(std::move(m.imports))
, exports(std::move(m.exports))
, externs(std::move(m.externs))
, unresolved_relocations(std::move(m.unresolved_relocations))
, _data(std::move(m._data))
, _data_blocks(std::move(m._data_blocks))
, _data_relocations(std::move(m._data_relocations))
{
  m.last_import = 0;
}


vm_unit_t &vm_unit_t::operator = (vm_unit_t const &m)
{
  last_import            = m.last_import;
  instructions           = m.instructions;
  instruction_argv       = m.instruction_argv;
  imports                = m.imports;
  exports                = m.exports;
  externs                = m.externs;
  unresolved_relocations = m.unresolved_relocations;
  _data                  = m._data;
  _data_blocks           = m._data_blocks;
  _data_relocations      = m._data_relocations;

  return *this;
}


vm_unit_t &vm_unit_t::operator = (vm_unit_t &&m)
{
  last_import            = 0;
  last_import            = m.last_import;
  instructions           = std::move(m.instructions);
  instruction_argv       = std::move(m.instruction_argv);
  imports                = std::move(m.imports);
  exports                = std::move(m.exports);
  externs                = std::move(m.externs);
  unresolved_relocations = std::move(m.unresolved_relocations);
  _data                  = std::move(m._data);
  _data_blocks           = std::move(m._data_blocks);
  _data_relocations      = std::move(m._data_relocations);

  m.last_import = 0;

  return *this;
}


void vm_unit_t::read_instruction(std::istream &input)
{
  opcode_t const opcode = static_cast<opcode_t>(read_primitive<uint16_t>(input));
  uint16_t const litflag = read_primitive<uint16_t>(input);
  int32_t arg_base = static_cast<int32_t>(instruction_argv.size());

  instructions.emplace_back(
    instruction_ptr_t {
      opcode,
      litflag,
      arg_base // curse unsigneds to death
    });

  int32_t const argc = g_opcode_argc[opcode] - (opcode_has_litflag(opcode) ? 1 : 0);

  for (int32_t counter = 0; counter < argc; ++counter) {
    value_t arg = read_primitive<value_t>(input);
    instruction_argv.push_back(arg);
  }
}


void vm_unit_t::read_instructions(std::istream &input)
{
  read_table(input, CHUNK_INST, [&](int32_t index) {
    (void)index;
    read_instruction(input);
  });
}


void vm_unit_t::read_extern_relocations(
  std::istream &input,
  int32_t instruction_base,
  extern_relocations_t const &relocations
  )
{
  extern_relocations_t::const_iterator not_found = relocations.cend();

  read_table(input, CHUNK_EREL, [&](int32_t rel_index) {
    (void)rel_index;

    relocation_ptr_t rel = read_primitive<relocation_ptr_t>(input);
    rel.pointer += instruction_base;
    int32_t const arg_base = instructions[rel.pointer].arg_pointer;

    each_in_mask(rel.args_mask, [&](int32_t mask_index) {
      int32_t const arg_index = arg_base + mask_index;
      value_t &arg = instruction_argv[arg_index];

      int32_t orig_base = arg;
      int32_t new_base = arg;

      auto iter = relocations.find(arg);
      if (iter == not_found) {
        unresolved_relocations.emplace_back(rel);
        return;
      }

      new_base = arg = iter->second.pointer;
      if (!iter->second.resolved) {
        /* unresolved */
        unresolved_relocations.emplace_back(rel);
      }
    });
  });
}


void vm_unit_t::read_label_relocations(
  std::istream &input,
  int32_t instruction_base,
  relocation_map_t const &relocations
  )
{
  relocation_map_t::const_iterator not_found = relocations.cend();

  read_table(input, CHUNK_LREL, [&](int32_t rel_index) {
    relocation_ptr_t rel = read_primitive<relocation_ptr_t>(input);
    rel.pointer += instruction_base;
    int32_t const arg_base = instructions[rel.pointer].arg_pointer;

    each_in_mask(rel.args_mask, [&](int32_t index) {
      int32_t const arg_index = arg_base + index;
      value_t &arg = instruction_argv[arg_index];
      relocation_map_t::const_iterator iter = relocations.find(arg);

      int32_t orig_base = arg;
      int32_t new_base;

      if (iter != not_found && iter->first == arg) {
        new_base = iter->second;
        arg = iter->second;
      } else if (arg.i32() >= 0) {
        new_base = orig_base + instruction_base;
        arg = new_base;
      } else {
        return;
      }
    });
  });
}



void vm_unit_t::read_externs(
  std::istream &input,
  extern_relocations_t &relocations
  )
{
  read_table(input, CHUNK_EXTS, [&](int32_t index) {
    std::string name = read_lstring(input);

    auto export_iter = exports.find(name);
    if (export_iter != exports.end()) {
      relocations.emplace(
        value_t { index },
        extern_relocation_t { value_t { export_iter->second }, true }
        );
      return;
    }

    auto extern_iter = externs.find(name);
    if (extern_iter != externs.end()) {
      if (extern_iter->second != index) {
        relocations.emplace(
          value_t { index },
          extern_relocation_t { value_t { extern_iter->second }, false }
          );
      }
      return;
    }

    int32_t new_address = static_cast<int32_t>(externs.size());
    if (index != new_address) {
      relocations.emplace(
        value_t { index },
        extern_relocation_t { value_t { new_address }, false }
        );
    }

    externs.emplace(std::move(name), new_address);
  });
}


void vm_unit_t::read_imports(std::istream &input, relocation_map_t &relocations)
{
  read_table(input, CHUNK_IMPT, [&](int32_t index) {
    label_t label = read_label(input);

    auto iter = imports.find(label.name);
    if (iter == imports.end()) {
      int32_t const orig_address = label.address;
      label.address = --last_import;

      if (orig_address != label.address) {
        relocations.emplace(value_t { orig_address }, value_t { label.address });
      }
    } else if (iter->second == label.address) {
      return;
    } else {
      relocations.emplace(value_t { label.address }, value_t { iter->second });
    }

    imports.emplace(std::move(label.name), label.address);
  });
}


void vm_unit_t::read_exports(
  std::istream &input,
  int32_t base,
  relocation_map_t &relocations
  )
{
  read_table(input, CHUNK_EXPT, [&](int32_t index) {
    label_t label = read_label(input);
    label_table_t::const_iterator iter = exports.find(label.name);
    int32_t address = label.address;

    if (iter != exports.cend()) {
      if (base != 0) {
        address += base;
        relocations.emplace(value_t { label.address }, value_t { address });
      }
      return;
    } else if (base != 0) {
      address += base;
      relocations.emplace(value_t { label.address }, value_t { address });
    }

    exports.emplace(std::move(label.name), address);
  });
}


void vm_unit_t::resolve_externs()
{
  if (unresolved_relocations.size() == 0) {
    return;
  }

  relocation_map_t relocations;
  label_table_t next_externs;
  relocation_table_t next_relocations;

  for (auto const &ext : externs) {
    auto iter = exports.find(ext.first);
    if (iter == exports.end()) {
      next_externs.emplace(ext);
      return;
    }

    relocations.emplace(value_t { ext.second }, value_t { iter->second });
  }

  if (relocations.size() == 0) {
    return;
  }

  auto const not_found = relocations.cend();
  for (auto const &rel : unresolved_relocations) {
    int32_t const arg_base = instructions[rel.pointer].arg_pointer;
    uint32_t updated_mask = 0;

    each_in_mask(rel.args_mask, [&](int32_t mask_index) {
      int32_t const arg_index = arg_base + mask_index;
      value_t &arg = instruction_argv[arg_index];

      relocation_map_t::const_iterator iter = relocations.find(arg);

      if (iter == not_found) {
        updated_mask |= 0x1U << (mask_index - 1);
        return;
      }

      arg = iter->second;
    });

    if (updated_mask) {
      next_relocations.emplace_back(relocation_ptr_t { rel.pointer, updated_mask });
    }
  }

  externs = std::move(next_externs);
  unresolved_relocations = std::move(next_relocations);
}


void vm_unit_t::read_data_table(
  std::istream &input,
  int32_t data_base,
  relocation_map_t &relocations
  )
{
  read_table(input, CHUNK_DATA, [&](int32_t max_count) {
      _data_blocks.reserve(_data_blocks.size() + max_count);
    },
    [&](int32_t data_index) {
      // base is always 1 (0 reserved for null, basically)
      int32_t const block_id = 1 + data_base + data_index;
      int32_t const block_size = read_primitive<int32_t>(input);
      int32_t const offset = _data.size();

      _data.resize(static_cast<size_t>(offset + block_size));
      input.read((char *)&_data[offset], block_size);

      _data_blocks.emplace_back(data_block_t { block_id, offset, block_size });

      if (data_base > 0) {
        relocations.emplace(value_t { 1 + data_index }, value_t { block_id });
      }
    });
}


void vm_unit_t::read_data_relocations(
  std::istream &input,
  int32_t instr_base,
  int32_t data_base,
  relocation_map_t &load_relocations
  )
{
  relocation_map_t::const_iterator not_found = load_relocations.cend();
  read_table(input, CHUNK_DREL, [&](int32_t count) {
      _data_relocations.reserve(_data_relocations.size() + count);
    }, [&](int32_t index) {
      relocation_ptr_t rel = read_primitive<relocation_ptr_t>(input);
      rel.pointer += instr_base;

      int32_t const arg_base = instructions[rel.pointer].arg_pointer;

      each_in_mask(rel.args_mask, [&](int32_t arg_index) {
        value_t &arg = instruction_argv[arg_base + arg_index];
        relocation_map_t::const_iterator iter = load_relocations.find(arg);
        if (iter == not_found) {
          return;
        }
        int32_t prev = arg;
        arg = iter->second;
      });

      _data_relocations.emplace_back(rel);
    });
}


void vm_unit_t::read(std::istream &input)
{
  version_chunk_t const filehead {
    read_primitive<chunk_header_t>(input),  // header
    read_primitive<int32_t>(input)          // version
  };

  int32_t instruction_base = static_cast<int32_t>(instructions.size());
  int32_t instruction_argv_base = static_cast<int32_t>(instruction_argv.size());

  relocation_map_t label_relocations;

  if (filehead.version < 8) {
    // TODO: Use own exceptions for these things
    throw vm_unsupported_unit_version("Invalid bytecode version.");
  }

  chunk_offsets_t const offsets { input };

  if (offsets.seek_to_offset(input, CHUNK_INST)) {
    read_instructions(input);
  } else {
    throw vm_bad_unit("Unable to seek to instruction table.");
  }

  if (offsets.seek_to_offset(input, CHUNK_IMPT)) {
    read_imports(input, label_relocations);
  } else {
    throw vm_bad_unit("Unable to seek to imported labels table.");
  }

  if (offsets.seek_to_offset(input, CHUNK_EXPT)) {
    read_exports(input, instruction_base, label_relocations);
  } else {
    throw vm_bad_unit("Unable to seek to exported labels table.");
  }

  if (label_relocations.size() > 0) {
    if (offsets.seek_to_offset(input, CHUNK_LREL)) {
      read_label_relocations(input, instruction_base, label_relocations);
    } else {
      throw vm_bad_unit("Unable to seek to relocated labels table.");
    }
  }

  extern_relocations_t extern_relocations;

  if (offsets.seek_to_offset(input, CHUNK_EXTS)) {
    read_externs(input, extern_relocations);
  } else {
    throw vm_bad_unit("Unable to seek to extern labels table.");
  }

  if (offsets.seek_to_offset(input, CHUNK_EREL)) {
    read_extern_relocations(input, instruction_base, extern_relocations);
  } else {
    throw vm_bad_unit("Unable to seek to relocated labels table.");
  }

  relocation_map_t data_relocations;
  int32_t data_base = static_cast<int32_t>(_data_blocks.size());

  if (offsets.seek_to_offset(input, CHUNK_DATA)) {
    read_data_table(input, data_base, data_relocations);
  } else {
    throw vm_bad_unit("Unable to seek to data table.");
  }

  if (offsets.seek_to_offset(input, CHUNK_DREL)) {
    read_data_relocations(input, instruction_base, data_base, data_relocations);
  } else {
    throw vm_bad_unit("Unable to seek to data relocation table.");
  }

  resolve_externs();
}



void vm_unit_t::debug_write_instructions(std::ostream &out) const
{
  size_t inum = 0;
  for (auto const &ins : instructions) {
    bool has_litflag = opcode_has_litflag(ins.opcode);
    out << (inum++) << ": " << ins.opcode;
    if (has_litflag && ins.litflag) {
      out << "#" << std::hex << ins.litflag << std::dec;
    }
    int32_t argc = g_opcode_argc[ins.opcode] - (has_litflag ? 1 : 0);
    for (int32_t arg_idx = 0; arg_idx < argc; ++arg_idx) {
      out << " " << instruction_argv[ins.arg_pointer + arg_idx];
    }
    out << std::endl;
  }
}



bool vm_unit_t::relocate_static_data(data_id_ary_t const &new_ids)
{
  relocation_map_t relocations;

  for (int32_t index = 0; index < _data_blocks.size(); ++index) {
    data_block_t &block = _data_blocks[index];

    auto insertion = relocations.emplace(
      value_t { block.id },
      value_t { new_ids[index] }
      );

    block.id = new_ids[index];

    if (!insertion.second) {
      return false;
    }
  }

  apply_relocation_table(_data_relocations, relocations);

  return true;
}


void vm_unit_t::apply_instruction_relocation(
  relocation_ptr_t rel,
  relocation_map_t const &relocations
  )
{
  int32_t const arg_base = instructions[rel.pointer].arg_pointer;
  relocation_map_t::const_iterator not_found = relocations.cend();
  each_in_mask(rel.args_mask, [&](int32_t arg_index) {
    value_t &arg = instruction_argv[arg_base + arg_index];
    relocation_map_t::const_iterator mapped = relocations.find(arg);

    if (mapped == not_found) {
      return;
    }

    arg = mapped->second;
  });
}


void vm_unit_t::apply_relocation_table(
  relocation_table_t const &table,
  relocation_map_t const &relocations
  )
{
  for (auto rel : table) {
    apply_instruction_relocation(rel, relocations);
  }
}


op_t vm_unit_t::fetch_op(int32_t ip) const
{
  return op_t { *this, ip };
}

