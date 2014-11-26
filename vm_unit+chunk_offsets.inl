/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#include "vm_unit+io.inl"
#include "vm_unit+chunk_types.inl"


/**
 * Chunk offsets, used to jump to and get the offset of specific areas of a
 * unit file.
 *
 * Can jump to any chunk offset provided there is a valid OFFS chunk to read
 * the offsets from.
 */
struct vm_chunk_offsets
{
  /**
   * A chunk offset. Contains the chunk name and its offset in the unit file.
   */
  struct chunk_offset
  {
    vm_chunk_id  id;
    int32_t     offset;
  };


  vm_table_header                header;
  std::vector<chunk_offset>   offsets;


  /**
   * Constructor. Given an input stream starting at a unit's OFFS chunk, it
   * reads the OFFS chunk from the stream and caches all offsets as needed.
   */
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


  /** Returns an offset for a defined chunk ID. */
  int32_t offset_for(vm_chunk_id id) const
  {
    for (chunk_offset const &chunk_off : offsets) {
      if (chunk_off.id == id) {
        return chunk_off.offset;
      }
    }
    return -1;
  }


  /**
   * Seeks to a defined chunk ID's offset in the stream, if known.
   *
   * @return True if the seek went through. May need to check eof bit on the
   * stream, however, as this only returns input.good() in case the seek is
   * successful.
   */
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
