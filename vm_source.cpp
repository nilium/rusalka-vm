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

#include <cassert>

#include "vm_source.h"


static const std::string SCRIPT_IMPORTS_TITLE { "IMPT" };
static const std::string SCRIPT_EXPORTS_TITLE { "EXPT" };
static const std::string SCRIPT_CODE_TITLE { "CODE" };
static const std::string SCRIPT_DATA_TITLE { "DATA" };


std::pair<bool, int32_t> source_t::exported_function(const char *name) const {
  label_table_t::const_iterator iter = _exports.find(name);
  if (iter != _exports.cend()) return std::make_pair(true, iter->second);
  else return std::make_pair(false, 0);
}


std::pair<bool, int32_t> source_t::imported_function(const char *name) const {
  label_table_t::const_iterator iter = _imports.find(name);
  if (iter != _imports.cend()) return std::make_pair(true, iter->second);
  else return std::make_pair(false, 0);
}


void source_t::read_label_table(std::istream &input, label_table_t &table) {
  int32_t label_count = 0;
  std::string name;

  input.read((char *)&label_count, sizeof(label_count));
  for (; label_count; --label_count) {
    int32_t label_location = 0;
    int32_t label_length = 0;

    input.read((char *)&label_location, sizeof(label_location));
    input.read((char *)&label_length, sizeof(label_length));

    name.resize(label_length);
    input.read(const_cast<char *>(name.data()), label_length);

    table.emplace(name, label_location);
  }
}


void source_t::read_data_table(std::istream &input, data_table_t &table) {
  int32_t data_count = 0;

  input.read((char *)&data_count, sizeof(data_count));
  for (; data_count; --data_count) {

    int32_t data_id = 0;
    int32_t data_size = 0;

    input.read((char *)&data_id, sizeof(data_id));
    input.read((char *)&data_size, sizeof(data_size));

    data_entry_t *entry = (data_entry_t *)malloc(sizeof(*entry) + data_size + 1);
    new(entry) data_entry_t;
    entry->size = data_size + 1;

    input.read((char *)entry->data, data_size);
    entry->data[data_size] = 0;

    table.emplace(data_id, entry);
  }
}

source_t::source_t(source_t &&other)
: _ops(std::move(other._ops))
, _imports(std::move(other._imports))
, _exports(std::move(other._exports))
, _data(std::move(other._data))
{
  // nop
}

source_t &source_t::operator = (source_t &&other) {
  _ops = std::move(other._ops);
  _exports = std::move(other._exports);
  _imports = std::move(other._imports);
  _data = std::move(other._data);
  return *this;
}

source_t::source_t(std::istream &&input) {
  // std::array<char, 256> chunk;
  size_t ops_offset = 0;
  int32_t chunk_size;
  char chunk_title[5];
  chunk_title[4] = '\0';

  while (input) {
    input.read(chunk_title, 4);
    input.read((char *)&chunk_size, sizeof(chunk_size));

    if (SCRIPT_IMPORTS_TITLE == chunk_title) {
      read_label_table(input, _imports);
    } else if (SCRIPT_EXPORTS_TITLE == chunk_title) {
      read_label_table(input, _exports);
    } else if (SCRIPT_DATA_TITLE == chunk_title) {
      read_data_table(input, _data);
    } else if (SCRIPT_CODE_TITLE == chunk_title) {
      _ops.reserve(chunk_size);
      int32_t num_ops = 0;
      input.read((char *)&num_ops, sizeof(num_ops));
      op_t op;

      while (num_ops) {
        input.read((char *)&op.opcode, sizeof(int32_t));
        assert(op.opcode >= ADD);
        assert(op.opcode < OP_COUNT);

        const size_t args_size = g_opcode_argc[op.opcode] * sizeof(value_t);
        input.read((char *)op.argv, args_size);

        _ops.push_back(op);

        --num_ops;
      }

      // while (input && chunk_size > 0) {
      //   input.read(chunk.data(), std::min((int32_t)chunk.size(), chunk_size));
      //   const auto count = input.gcount();
      //   _ops.resize(_ops.size() + count);
      //   memcpy(&_ops[ops_offset], chunk.data(), count);
      //   ops_offset += count;
      //   chunk_size -= count;
      // }
    } else {
      input.seekg(chunk_size, std::ios_base::seekdir::cur);
    }
  }

  // const int32_t *code = (int32_t *)_ops.data();
  // const int32_t *code_end = (int32_t *)(_ops.data() + (_ops.size() - sizeof(int32_t)));
  // while (code < code_end) {
  //   const op_t *op = (const op_t *)code;
  //   assert(op->opcode >= ADD_F32);
  //   assert(op->opcode < OP_COUNT);
  //   _table.push_back(op);
  //   code += g_opcode_argc[op->opcode] + 1;
  // }
}

source_t::~source_t() {
  for (const auto kvpair : _data) {
    kvpair.second->~data_entry_t();
    ::free(kvpair.second);
  }
}

const op_t &source_t::fetch_op(int32_t index) const {
  // return *_table[index];
  return _ops[index];
}

void source_t::append_op(const op_t &op, size_t argc) {
  // const size_t data_size = (sizeof(op) + sizeof(value_t) * argc);
  // const size_t index = _ops.size();
  // _ops.resize(index + data_size);
  // op_t *store = (op_t *)&_ops[index];
  // memcpy(store, &op, data_size);
  // _table.push_back(store);
  _ops.push_back(op);
}