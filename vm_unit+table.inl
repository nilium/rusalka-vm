#ifndef __VM_UNIT_TABLE_INL__
#define __VM_UNIT_TABLE_INL__


template <typename TABLETYPE>
struct table_chunk_t
{

  using name_t = std::array<char, 4>;
  using table_t = TABLETYPE;


  name_t    name;
  table_t   table;


  table_chunk_t() = default;

  table_chunk_t(const table_chunk_t &t)
  : name(t.name)
  , table(t.table)
  {
    // nop
  }

  table_chunk_t(table_chunk_t &&t)
  : name(t.name)
  , table(std::move(t.table))
  {
    t.zero_name();
  }


  table_chunk_t &
  operator = (const table_chunk_t &t) = default;


  table_chunk_t &
  operator = (table_chunk_t &&t)
  {
    std::copy(std::begin(t.name), std::end(t.name), std::begin(name));
    t.zero_name();
    return *this;
  }


  template <typename T>
  bool
  name_is(const T &name)
  {
    using namespace std;
    return equal(begin(this->name), end(this->name), begin(name));
  }


private:

  void
  zero_name()
  {
    std::for_each(
      std::begin(name),
      std::end(name),
      [](char &ch) { ch = '\0'; }
      );
  }

};


#endif /* end __VM_UNIT_TABLE_INL__ include guard */
