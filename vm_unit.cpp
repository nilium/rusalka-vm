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
  for (int32_t index = 0; mask; index += 4) {
    if (mask & 0xFu) {
      if (mask & 0x1u) { func(index); }
      if (mask & 0x2u) { func(index + 1); }
      if (mask & 0x4u) { func(index + 2); }
      if (mask & 0x8u) { func(index + 3); }
    }
    mask >>= 4;
  }
}


vm_unit_t::vm_unit_t()
{
  /* nop */
}


vm_unit_t::~vm_unit_t()
{
  /* nop */
}


void
vm_unit_t::read_instruction(std::istream &input)
{
  opcode_t const opcode = read_primitive<opcode_t>(input);

  instructions.emplace_back(
    instruction_ptr_t {
      opcode,
      static_cast<int32_t>(instruction_argv.size()) // curse unsigneds to death
    });

  int32_t const argc = g_opcode_argc[opcode];

  for (int32_t counter = 0; counter < argc; ++counter) {
    value_t arg = read_primitive<value_t>(input);
    instruction_argv.push_back(arg);
  }
}


void
vm_unit_t::read_instructions(std::istream &input)
{
  read_table(input, CHUNK_INST, [&](int32_t index) {
    read_instruction(input);
  });
}


void
vm_unit_t::read_label_relocations(
  std::istream &input,
  int32_t instruction_base,
  label_relocations_t const &relocations
  )
{
  label_relocations_t::const_iterator not_found = relocations.cend();

  read_table(input, CHUNK_LREL, [&](int32_t index) {
    relocation_ptr_t rel = read_primitive<relocation_ptr_t>(input);
    rel.pointer += instruction_base;

    each_in_mask(rel.args_mask, [&](int32_t index) {
      int32_t const arg_index = instructions[rel.pointer].arg_pointer + index;
      value_t &arg = instruction_argv[arg_index];
      label_relocations_t::const_iterator iter = relocations.find(arg.i32());

      int32_t orig_base = arg;
      int32_t new_base;

      if (iter != not_found && iter->first == arg.i32()) {
        new_base = iter->second;
        arg = iter->second;
      } else if (arg.i32() >= 0) {
        new_base = orig_base + instruction_base;
        arg = new_base;
      } else {
        return;
      }

      std::cout << "Relocating " << rel.pointer << ": " << instructions[rel.pointer].opcode << "[" << index << "] from " << orig_base << " to " << new_base << std::endl;
    });
  });
}



void
vm_unit_t::read_labels(
  std::istream &input,
  chunk_id_t id,
  label_table_t &table
  )
{
  read_table(input, id, [&](int32_t index) {
  });
}



void
vm_unit_t::read_externs(std::istream &input)
{
  read_table(input, CHUNK_EXTS, [&](int32_t index) {
  });
}


void
vm_unit_t::read_imports(std::istream &input, label_relocations_t &relocations)
{
  read_table(input, CHUNK_IMPT, [&](int32_t index) {
    label_t label = read_label(input);
    std::cout << "Read imported label " << label.name << " ";

    auto iter = imports.find(label.name);
    if (iter == imports.end()) {
      int32_t const orig_address = label.address;
      label.address = --last_import;

      if (orig_address != label.address) {
        relocations.emplace(orig_address, label.address);
        std::cout << "at " << label.address << " (new; requires relocation)" << std::endl;
      } else {
        std::cout << "at " << label.address << " (new; does not require relocation)" << std::endl;
      }
    } else if (iter->second == label.address) {
      std::cout << "at " << label.address << " (duplicate; does not require relocation)" << std::endl;
      return;
    } else {
      relocations.emplace(label.address, iter->second);
      std::cout << "at " << label.address << " (duplicate; requires relocation)" << std::endl;
    }


    imports.emplace(std::move(label.name), label.address);
  });
}


void
vm_unit_t::read_exports(std::istream &input, int32_t base, label_relocations_t &relocations)
{
  read_table(input, CHUNK_EXPT, [&](int32_t index) {
    label_t label = read_label(input);
    std::cout << "Read exported label " << label.name << " at " << label.address;
    label_table_t::const_iterator iter = exports.find(label.name);
    if (iter != exports.cend()) {
      std::cout << "(duplicate label -- label will not be accessible)" << std::endl;
      if (base != 0) {
        relocations.emplace(label.address, label.address + base);
      }
      return;
    } else if (base != 0) {
      std::cout << " (requires relocation)" << std::endl;
      relocations.emplace(label.address, label.address + base);
    } else {
      std::cout << " (does not require relocation)" << std::endl;
    }
    exports.emplace(std::move(label.name), label.address);
  });
}


void
vm_unit_t::read(std::istream &input)
{
  version_chunk_t const filehead {
    read_primitive<chunk_header_t>(input),  // header
    read_primitive<int32_t>(input)          // version
  };

  int32_t instruction_base = static_cast<int32_t>(instructions.size());
  int32_t instruction_argv_base = static_cast<int32_t>(instruction_argv.size());

  label_relocations_t label_relocations;

  if (filehead.version < 8) {
    std::cerr << "Bytecode version incorrect: expected 8, got " << filehead.version << std::endl;
    return;
  }

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


}

