/*
 *          Copyright Noel Cower 2013 - 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#include "vm_opcode.h"


namespace
{


enum class undefined : int { VALUE = 0 };
enum class input     : int { VALUE = 1 };
enum class output    : int { VALUE = 2 };
enum class regonly   : int { VALUE = 3 };
enum class litflag   : int { VALUE = 4 };


template <size_t ARGC, typename... ARGV>
struct opcode_arg_types
{
  using arg_types__ = std::tuple<ARGV...>;

  template <size_t N>
  struct arg_type
  {
    static_assert(N < ARGC, "Invalid arg_type index");
    using type = typename std::tuple_element<N, arg_types__>::type;
    static const type value = type::VALUE;
  };

  using last_arg = arg_type<ARGC - 1>;
  static size_t const arg_count = ARGC;
};



template <>
struct opcode_arg_types<0>
{
  template <size_t>
  struct arg_type
  {
    using type = undefined;
    static const undefined value = undefined::VALUE;
  };

  using last_arg = arg_type<0>;
  static size_t const arg_count = 0;
};


} // namespace <anon>



const int32_t g_opcode_argc[OP_COUNT] {
#define INSTRUCTION(OPCODE, ASM_NAME, CODE, NUM_ARGS, ARG_INFO... ) NUM_ARGS,
#include "vm_instructions.h"
#undef INSTRUCTION
};



bool opcode_has_litflag(vm_opcode op)
{
  switch (op) {
  #define INSTRUCTION(OPCODE, ASM_NAME, CODE, NUM_ARGS, ARG_INFO... ) \
  case OPCODE : return std::is_same<opcode_arg_types<NUM_ARGS, ##ARG_INFO>::last_arg::type, litflag>::value;
  #include "vm_instructions.h"
  #undef INSTRUCTION
  default: return false;
  }
}



std::ostream &operator << (std::ostream &out, vm_opcode v)
{
  switch (v) {
  #define INSTRUCTION(OPCODE, ASM_NAME, CODE, ARGS, ARG_INFO... ) \
    case OPCODE: return out << #OPCODE;
  #include "vm_instructions.h"
  #undef INSTRUCTION
  case OP_COUNT: {
    // throw std::runtime_error("invalid opcode");
    return out << "INVALID OPCODE";
  }
  }
}
