#ifndef ___TYPES_H__
#define ___TYPES_H__

class vm_thread;
class vm_state;
class op_t;
struct value_t;


template <typename T>
struct vm_find_result {
  bool ok;
  T value;
};

using vm_found_fn = vm_find_result<int32_t>;


/// VM callback -- given a thread, argument count, and array of argument values.
using vm_callback_t = value_t (vm_thread &vm, int32_t argc, value_t const *argv);


#endif /* end ___TYPES_H__ include guard */
