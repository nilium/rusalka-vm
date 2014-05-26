#include "vm_unit.h"


#include "vm_unit+chunk_types.inl"
#include "vm_unit+io.inl"
#include "vm_unit+chunk_offsets.inl"
#include "vm_opcode.h"
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


vm_unit_t::~vm_unit_t()
{
  /* nop */
}


void vm_unit_t::read_instruction(std::istream &input)
{
  opcode_t const opcode = static_cast<opcode_t>(read_primitive<int32_t>(input));
  int32_t arg_base = static_cast<int32_t>(instruction_argv.size());

  std::cerr << instructions.size() << ": " << opcode << "(" << arg_base  << ")";

  instructions.emplace_back(
    instruction_ptr_t {
      opcode,
      arg_base // curse unsigneds to death
    });

  int32_t const argc = g_opcode_argc[opcode];

  for (int32_t counter = 0; counter < argc; ++counter) {
    value_t arg = read_primitive<value_t>(input);
    instruction_argv.push_back(arg);
    std::cerr << " " << arg;
  }
  std::cerr << std::endl;
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

    std::cerr << "Reading extern relocations for " << rel.pointer << "(" << arg_base << ")" << std::endl;

    each_in_mask(rel.args_mask, [&](int32_t mask_index) {
      int32_t const arg_index = arg_base + mask_index;
      value_t &arg = instruction_argv[arg_index];

      std::cerr << "Trying to relocate " << rel.pointer << ": " << instructions[rel.pointer].opcode << "[" << mask_index << "] -> " << arg << std::endl;

      int32_t orig_base = arg;
      int32_t new_base = arg;

      auto iter = relocations.find(arg);
      if (iter == not_found) {
        std::cerr << "Relocation of " << rel.pointer << ": " << instructions[rel.pointer].opcode << "[" << mask_index << "] for " << orig_base << " remains unresolved" << std::endl;
        unresolved_relocations.emplace_back(rel);
        return;
      }

      arg = new_base = iter->second.first;
      std::cerr << "Relocating " << rel.pointer << ": " << instructions[rel.pointer].opcode << "[" << mask_index << "] from " << orig_base << " to " << new_base;

      if (!iter->second.second) {
        /* unresolved */
        unresolved_relocations.emplace_back(rel);
        std::cerr << " (remains unresolved)";
      }

      std::cerr << std::endl;
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

      std::cerr << "Relocating " << rel.pointer << ": " << instructions[rel.pointer].opcode << "[" << index << "] from " << orig_base << " to " << new_base << std::endl;
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

    std::cerr << "Read extern " << index << " <" << name << ">";

    auto export_iter = exports.find(name);
    if (export_iter != exports.end()) {
      std::cerr << " (extern matches export, " << index << " -> " << export_iter->second << ")" << std::endl;
      relocations.emplace(
        value_t { index },
        extern_relocation_t { value_t { export_iter->second }, true }
        );
      return;
    }

    auto extern_iter = externs.find(name);
    if (extern_iter != externs.end()) {
      std::cerr << " (extern already exists; requires relocation: " << (extern_iter->second != index) << ")" << std::endl;
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
      std::cerr << " (address needs relocation)";
    }

    externs.emplace(std::move(name), new_address);
    std::cerr << std::endl;
  });
}


void vm_unit_t::read_imports(std::istream &input, relocation_map_t &relocations)
{
  read_table(input, CHUNK_IMPT, [&](int32_t index) {
    label_t label = read_label(input);
    std::cerr << "Read imported label " << label.name << " ";

    auto iter = imports.find(label.name);
    if (iter == imports.end()) {
      int32_t const orig_address = label.address;
      label.address = --last_import;

      if (orig_address != label.address) {
        relocations.emplace(value_t { orig_address }, value_t { label.address });
        std::cerr << "at " << label.address << " (new; requires relocation)" << std::endl;
      } else {
        std::cerr << "at " << label.address << " (new; does not require relocation)" << std::endl;
      }
    } else if (iter->second == label.address) {
      std::cerr << "at " << label.address << " (duplicate; does not require relocation)" << std::endl;
      return;
    } else {
      relocations.emplace(value_t { label.address }, value_t { iter->second });
      std::cerr << "at " << label.address << " (duplicate; requires relocation)" << std::endl;
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
    std::cerr << "Read exported label " << label.name << " at " << label.address;
    label_table_t::const_iterator iter = exports.find(label.name);
    int32_t address = label.address;
    if (iter != exports.cend()) {
      std::cerr << "(duplicate label -- label will not be accessible)" << std::endl;
      if (base != 0) {
        address += base;
        relocations.emplace(value_t { label.address }, value_t { address });
      }
      return;
    } else if (base != 0) {
      std::cerr << " (requires relocation)" << std::endl;
      address += base;
      relocations.emplace(value_t { label.address }, value_t { address });
    } else {
      std::cerr << " (does not require relocation)" << std::endl;
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

    std::cerr << "Found extern: " << ext.first << " at " << iter->second << std::endl;
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

      std::cerr << "Resolved extern reference, relocating " << rel.pointer << ": " << instructions[rel.pointer].opcode << "[" << mask_index << "] from " << arg.i32() << " to " << iter->second << std::endl;

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

      std::cerr << "Read data block " << data_index << ":" << block_id << "[" << block_size << "] <"
        << std::string((char const *)&_data[offset], (char const *)&_data[offset + block_size]) << ">"
        << std::endl;

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

        std::cerr << "Relocating data reference " << rel.pointer << ": " << instructions[rel.pointer].opcode << "[" << arg_index << "] from " << prev << " to " << iter->second << std::endl;
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
    std::cerr << "Bytecode version incorrect: expected 8, got " << filehead.version << std::endl;
    return;
  }

  std::cerr << "Instruction base for read: " << instruction_base << ':' << instruction_argv_base << std::endl;

  chunk_offsets_t const offsets { input };

  if (offsets.seek_to_offset(input, CHUNK_INST)) {
    read_instructions(input);
  } else {
    std::cerr << "Unable to seek to instruction table." << std::endl;
  }

  if (offsets.seek_to_offset(input, CHUNK_IMPT)) {
    read_imports(input, label_relocations);
  } else {
    std::cerr << "Unable to seek to imported labels table." << std::endl;
  }

  if (offsets.seek_to_offset(input, CHUNK_EXPT)) {
    read_exports(input, instruction_base, label_relocations);
  } else {
    std::cerr << "Unable to seek to exported labels table." << std::endl;
  }

  if (label_relocations.size() > 0) {
    if (offsets.seek_to_offset(input, CHUNK_LREL)) {
      read_label_relocations(input, instruction_base, label_relocations);
    } else {
      std::cerr << "Unable to seek to relocated labels table." << std::endl;
    }
  } else {
    std::cerr << "No local labels require relocation." << std::endl;
  }

  extern_relocations_t extern_relocations;

  if (offsets.seek_to_offset(input, CHUNK_EXTS)) {
    read_externs(input, extern_relocations);
  } else {
    std::cerr << "Unable to seek to extern labels table." << std::endl;
  }

  if (offsets.seek_to_offset(input, CHUNK_EREL)) {
    std::cerr << "Reading extern relocations table." << std::endl;
    read_extern_relocations(input, instruction_base, extern_relocations);
    std::cerr << "Done reading extern relocations table." << std::endl;
  } else {
    std::cerr << "Unable to seek to relocated labels table." << std::endl;
  }

  relocation_map_t data_relocations;
  int32_t data_base = static_cast<int32_t>(_data_blocks.size());

  if (offsets.seek_to_offset(input, CHUNK_DATA)) {
    read_data_table(input, data_base, data_relocations);
  } else {
    std::cerr << "Unable to seek to data table." << std::endl;
  }

  if (offsets.seek_to_offset(input, CHUNK_DREL)) {
    read_data_relocations(input, instruction_base, data_base, data_relocations);
  }

  resolve_externs();
}



void vm_unit_t::debug_write_instructions(std::ostream &out) const
{
  size_t inum = 0;
  for (auto const &ins : instructions) {
    out << (inum++) << ": " << ins.opcode;
    for (int32_t arg_idx = 0; arg_idx < g_opcode_argc[ins.opcode]; ++arg_idx) {
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

    std::cerr
      << "Relocating "
      << rel.pointer << ": " << instructions[rel.pointer].opcode
      << "[" << arg_index << "] from "
      << mapped->first << " to " << mapped->second << std::endl;

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

