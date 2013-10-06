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

#ifndef __VM_SOURCE_H__
#define __VM_SOURCE_H__

#include <map>
#include <utility>
#include <vector>

#include "vm_op.h"


class source_t {
public:
  struct data_entry_t {
    uint32_t size;
    uint8_t data[0];
  };

  using label_table_t = std::map<std::string, int32_t>;
  using data_table_t = std::map<uint32_t, data_entry_t *>;

private:
  // using op_storage_t = uint8_t;
  // using op_table_t = std::vector<const op_t *>;
  // using op_data_t = std::vector<op_storage_t>;
  using op_data_t = std::vector<op_t>;

  // op_table_t _table;
  op_data_t _ops;
  label_table_t _imports;
  label_table_t _exports;
  data_table_t _data;


  static void read_label_table(std::istream &input, label_table_t &table);
  static void read_data_table(std::istream &input, data_table_t &table);

  data_table_t &data_table() { return _data; }

  friend class vm_state_t;

public:
  source_t() = default;
  source_t(std::istream &&input);
  source_t(const source_t &other) = delete;
  source_t(source_t &&other);
  ~source_t();

  source_t &operator = (source_t &&other);

  const op_t &fetch_op(int32_t index) const;
  void append_op(const op_t &op, size_t argc);

  size_t size() const { return _ops.size(); }

  const label_table_t &imports_table() const { return _imports; }
  const label_table_t &exports_table() const { return _exports; }
  const data_table_t &data_table() const { return _data; }

  std::pair<bool, int32_t> exported_function(const char *name) const;
  std::pair<bool, int32_t> imported_function(const char *name) const;
};

#endif /* end __VM_SOURCE_H__ include guard */
