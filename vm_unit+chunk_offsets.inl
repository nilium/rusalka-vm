/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __VM_UNIT_CHUNK_OFFSETS_INL__
#define __VM_UNIT_CHUNK_OFFSETS_INL__


#include "vm_unit+io.inl"
#include "vm_unit+chunk_types.inl"


struct vm_chunk_offsets
{
  struct chunk_offset
  {
    vm_chunk_id  id;
    int32_t     offset;
  };


  vm_table_header                header;
  std::vector<chunk_offset>   offsets;


  explicit vm_chunk_offsets(std::istream &input)
  {
    read_table(input, CHUNK_OFFS, header, [&](int32_t index) {
      offsets.emplace_back(
        chunk_offset {
          read_primitive<vm_chunk_id>(input),
          read_primitive<int32_t>(input)
        });
    });
  }


  int32_t offset_for(vm_chunk_id id) const
  {
    for (chunk_offset const &chunk_off : offsets) {
      if (chunk_off.id == id) {
        return chunk_off.offset;
      }
    }
    return -1;
  }


  // Returns true if the seek went through. May need to check eof bit on the
  // stream, however, as this only returns input.good() in case the seek is
  // successful.
  bool seek_to_offset(
    std::istream &input,
    vm_chunk_id id,
    int32_t io_start = 0
    ) const
  {
    int32_t const offset = io_start + offset_for(id);

    if (offset < io_start || offset < 0) {
      return false;
    }

    if (input.good()) {
      input.seekg(static_cast<std::istream::pos_type>(offset));
    }

    return input.good();
  }

};


#endif /* end __VM_UNIT_CHUNK_OFFSETS_INL__ include guard */
