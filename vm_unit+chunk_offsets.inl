#ifndef __VM_UNIT_CHUNK_OFFSETS_INL__
#define __VM_UNIT_CHUNK_OFFSETS_INL__


#include "vm_unit+io.inl"
#include "vm_unit+chunk_types.inl"


struct chunk_offsets_t
{
  struct chunk_offset_t
  {
    chunk_id_t  id;
    int32_t     offset;
  };


  table_header_t                header;
  std::vector<chunk_offset_t>   offsets;


  explicit
  chunk_offsets_t(std::istream &input)
  {
    read_table(input, CHUNK_OFFS, header, [&](int32_t index) {
      offsets.emplace_back(
        chunk_offset_t {
          read_primitive<chunk_id_t>(input),
          read_primitive<int32_t>(input)
        });
    });
  }


  int32_t
  offset_for(chunk_id_t id) const
  {
    for (int32_t index = 0; index < header.count; ++index) {
      if (offsets[index].id == id) {
        return offsets[index].offset;
      }
    }
    return -1;
  }


  // Returns true if the seek went through. May need to check eof bit on the
  // stream, however, as this only returns input.good() in case the seek is
  // successful.
  bool
  seek_to_offset(std::istream &input, chunk_id_t id, int32_t io_start = 0) const
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
