void vm_state_t::EXEC_NOP(vm_state_t &vm, const value_t argv[]) {
  // nop
}


void vm_state_t::EXEC_ADD_F32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).f32 = vm.reg(argv[1].i32).f32 + vm.reg(argv[2].i32).f32;
}


void vm_state_t::EXEC_ADD_I32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).i32 = vm.reg(argv[1].i32).i32 + vm.reg(argv[2].i32).i32;
}


void vm_state_t::EXEC_ADD_UI32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 + vm.reg(argv[2].i32).ui32;
}


void vm_state_t::EXEC_ADD_F32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).f32 = vm.reg(argv[1].i32).f32 + argv[2].f32;
}


void vm_state_t::EXEC_ADD_I32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).i32 = vm.reg(argv[1].i32).i32 + argv[2].i32;
}


void vm_state_t::EXEC_ADD_UI32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 + argv[2].ui32;
}


void vm_state_t::EXEC_SUB_F32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).f32 = vm.reg(argv[1].i32).f32 - vm.reg(argv[2].i32).f32;
}


void vm_state_t::EXEC_SUB_I32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).i32 = vm.reg(argv[1].i32).i32 - vm.reg(argv[2].i32).i32;
}


void vm_state_t::EXEC_SUB_UI32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 - vm.reg(argv[2].i32).ui32;
}


void vm_state_t::EXEC_SUB_F32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).f32 = vm.reg(argv[1].i32).f32 - argv[2].f32;
}


void vm_state_t::EXEC_SUB_I32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).i32 = vm.reg(argv[1].i32).i32 - argv[2].i32;
}


void vm_state_t::EXEC_SUB_UI32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 - argv[2].ui32;
}


void vm_state_t::EXEC_DIV_F32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).f32 = vm.reg(argv[1].i32).f32 / vm.reg(argv[2].i32).f32;
}


void vm_state_t::EXEC_DIV_I32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).i32 = vm.reg(argv[1].i32).i32 / vm.reg(argv[2].i32).i32;
}


void vm_state_t::EXEC_DIV_UI32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 / vm.reg(argv[2].i32).ui32;
}


void vm_state_t::EXEC_DIV_F32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).f32 = vm.reg(argv[1].i32).f32 / argv[2].f32;
}


void vm_state_t::EXEC_DIV_I32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).i32 = vm.reg(argv[1].i32).i32 / argv[2].i32;
}


void vm_state_t::EXEC_DIV_UI32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 / argv[2].ui32;
}


void vm_state_t::EXEC_MUL_F32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).f32 = vm.reg(argv[1].i32).f32 * vm.reg(argv[2].i32).f32;
}


void vm_state_t::EXEC_MUL_I32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).i32 = vm.reg(argv[1].i32).i32 * vm.reg(argv[2].i32).i32;
}


void vm_state_t::EXEC_MUL_UI32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 * vm.reg(argv[2].i32).ui32;
}


void vm_state_t::EXEC_MUL_F32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).f32 = vm.reg(argv[1].i32).f32 * argv[2].f32;
}


void vm_state_t::EXEC_MUL_I32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).i32 = vm.reg(argv[1].i32).i32 * argv[2].i32;
}


void vm_state_t::EXEC_MUL_UI32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 * argv[2].ui32;
}


void vm_state_t::EXEC_MOD_F32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).f32 = std::fmod(vm.reg(argv[1].i32).f32, vm.reg(argv[2].i32).f32);
}


void vm_state_t::EXEC_MOD_I32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).i32 = vm.reg(argv[1].i32).i32 % vm.reg(argv[2].i32).i32;
}


void vm_state_t::EXEC_MOD_UI32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 % vm.reg(argv[2].i32).ui32;
}


void vm_state_t::EXEC_MOD_F32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).f32 = std::fmod(vm.reg(argv[1].i32).f32, argv[2].f32);
}


void vm_state_t::EXEC_MOD_I32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).i32 = vm.reg(argv[1].i32).i32 % argv[2].i32;
}


void vm_state_t::EXEC_MOD_UI32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 % argv[2].ui32;
}


void vm_state_t::EXEC_NEG_F32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).f32 = -vm.reg(argv[1].i32).f32;
}


void vm_state_t::EXEC_NEG_I32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).i32 = -vm.reg(argv[1].i32).i32;
}


void vm_state_t::EXEC_NOT_UI32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = ~(vm.reg(argv[1].i32).ui32);
}


void vm_state_t::EXEC_OR_UI32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 | vm.reg(argv[2].i32).ui32;
}


void vm_state_t::EXEC_XOR_UI32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 ^ vm.reg(argv[2].i32).ui32;
}


void vm_state_t::EXEC_AND_UI32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 & vm.reg(argv[2].i32).ui32;
}


void vm_state_t::EXEC_SHR_I32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).i32 = vm.reg(argv[1].i32).i32 >> vm.reg(argv[2].i32).ui32;
}


void vm_state_t::EXEC_SHR_UI32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 >> vm.reg(argv[2].i32).ui32;
}


void vm_state_t::EXEC_SHL_I32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).i32 = vm.reg(argv[1].i32).i32 << vm.reg(argv[2].i32).ui32;
}


void vm_state_t::EXEC_SHL_UI32(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 << vm.reg(argv[2].i32).ui32;
}


void vm_state_t::EXEC_OR_UI32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 | argv[2].ui32;
}


void vm_state_t::EXEC_XOR_UI32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 ^ argv[2].ui32;
}


void vm_state_t::EXEC_AND_UI32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 & argv[2].ui32;
}


void vm_state_t::EXEC_SHR_I32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).i32 = vm.reg(argv[1].i32).i32 >> argv[2].ui32;
}


void vm_state_t::EXEC_SHR_UI32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 >> argv[2].ui32;
}


void vm_state_t::EXEC_SHL_I32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).i32 = vm.reg(argv[1].i32).i32 << argv[2].ui32;
}


void vm_state_t::EXEC_SHL_UI32_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 << argv[2].ui32;
}


void vm_state_t::EXEC_ITOUI(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = (uint32_t)vm.reg(argv[1].i32).i32;
}


void vm_state_t::EXEC_ITOF(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).f32 = static_cast<float>(vm.reg(argv[1].i32).i32);
}


void vm_state_t::EXEC_FTOUI(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = static_cast<uint32_t>(vm.reg(argv[1].i32).f32);
}


void vm_state_t::EXEC_FTOI(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).i32 = static_cast<int32_t>(vm.reg(argv[1].i32).f32);
}


void vm_state_t::EXEC_UITOF(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).f32 = static_cast<float>(vm.reg(argv[1].i32).ui32);
}


void vm_state_t::EXEC_UITOI(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).i32 = static_cast<int32_t>(vm.reg(argv[1].i32).ui32);
}


void vm_state_t::EXEC_CMP_F32(vm_state_t &vm, const value_t argv[]) {
  const float l = vm.reg(argv[1].i32).f32;
  const float r = vm.reg(argv[2].i32).f32;
  vm.reg(argv[0].i32).i32 = (l > r) - (l < r);
}


void vm_state_t::EXEC_CMP_I32(vm_state_t &vm, const value_t argv[]) {
  const int32_t l = vm.reg(argv[1].i32).i32;
  const int32_t r = vm.reg(argv[2].i32).i32;
  vm.reg(argv[0].i32).i32 = (l > r) - (l < r);
}


void vm_state_t::EXEC_CMP_UI32(vm_state_t &vm, const value_t argv[]) {
  const uint32_t l = vm.reg(argv[1].i32).ui32;
  const uint32_t r = vm.reg(argv[2].i32).ui32;
  vm.reg(argv[0].i32).i32 = (l > r) - (l < r);
}


void vm_state_t::EXEC_CMP_F32_L(vm_state_t &vm, const value_t argv[]) {
  const float l = vm.reg(argv[1].i32).f32;
  const float r = argv[2].f32;
  vm.reg(argv[0].i32).i32 = (l > r) - (l < r);
}


void vm_state_t::EXEC_CMP_I32_L(vm_state_t &vm, const value_t argv[]) {
  const int32_t l = vm.reg(argv[1].i32).i32;
  const int32_t r = argv[2].i32;
  vm.reg(argv[0].i32).i32 = (l > r) - (l < r);
}


void vm_state_t::EXEC_CMP_UI32_L(vm_state_t &vm, const value_t argv[]) {
  const uint32_t l = vm.reg(argv[1].i32).ui32;
  const uint32_t r = argv[2].ui32;
  vm.reg(argv[0].i32).i32 = (l > r) - (l < r);
}


void vm_state_t::EXEC_RCMP_F32(vm_state_t &vm, const value_t argv[]) {
  const float l = vm.reg(argv[1].i32).f32;
  const float r = vm.reg(argv[2].i32).f32;
  vm.reg(argv[0].i32).i32 = (l < r) - (l > r);
}


void vm_state_t::EXEC_RCMP_I32(vm_state_t &vm, const value_t argv[]) {
  const int32_t l = vm.reg(argv[1].i32).i32;
  const int32_t r = vm.reg(argv[2].i32).i32;
  vm.reg(argv[0].i32).i32 = (l < r) - (l > r);
}


void vm_state_t::EXEC_RCMP_UI32(vm_state_t &vm, const value_t argv[]) {
  const uint32_t l = vm.reg(argv[1].i32).ui32;
  const uint32_t r = vm.reg(argv[2].i32).ui32;
  vm.reg(argv[0].i32).i32 = (l < r) - (l > r);
}


void vm_state_t::EXEC_RCMP_F32_L(vm_state_t &vm, const value_t argv[]) {
  const float l = vm.reg(argv[1].i32).f32;
  const float r = argv[2].f32;
  vm.reg(argv[0].i32).i32 = (l < r) - (l > r);
}


void vm_state_t::EXEC_RCMP_I32_L(vm_state_t &vm, const value_t argv[]) {
  const int32_t l = vm.reg(argv[1].i32).i32;
  const int32_t r = argv[2].i32;
  vm.reg(argv[0].i32).i32 = (l < r) - (l > r);
}


void vm_state_t::EXEC_RCMP_UI32_L(vm_state_t &vm, const value_t argv[]) {
  const uint32_t l = vm.reg(argv[1].i32).ui32;
  const uint32_t r = argv[2].ui32;
  vm.reg(argv[0].i32).i32 = (l < r) - (l > r);
}


void vm_state_t::EXEC_JNZ(vm_state_t &vm, const value_t argv[]) {
  const int32_t mask = cndmask[vm.reg(argv[0].i32).i32 != 0];
  vm.ip() = (vm.reg(argv[1].i32).i32 & mask) | (vm.ip() & ~mask);
}


void vm_state_t::EXEC_JEZ(vm_state_t &vm, const value_t argv[]) {
  const int32_t mask = cndmask[vm.reg(argv[0].i32).i32 == 0];
  vm.ip() = (vm.reg(argv[1].i32).i32 & mask) | (vm.ip() & ~mask);
}


void vm_state_t::EXEC_JGTE(vm_state_t &vm, const value_t argv[]) {
  const int32_t mask = cndmask[vm.reg(argv[0].i32).i32 >= 0];
  vm.ip() = (vm.reg(argv[1].i32).i32 & mask) | (vm.ip() & ~mask);
}


void vm_state_t::EXEC_JLTE(vm_state_t &vm, const value_t argv[]) {
  const int32_t mask = cndmask[vm.reg(argv[0].i32).i32 <= 0];
  vm.ip() = (vm.reg(argv[1].i32).i32 & mask) | (vm.ip() & ~mask);
}


void vm_state_t::EXEC_JLT(vm_state_t &vm, const value_t argv[]) {
  const int32_t mask = cndmask[vm.reg(argv[0].i32).i32 < 0];
  vm.ip() = (vm.reg(argv[1].i32).i32 & mask) | (vm.ip() & ~mask);
}


void vm_state_t::EXEC_JGT(vm_state_t &vm, const value_t argv[]) {
  const int32_t mask = cndmask[vm.reg(argv[0].i32).i32 > 0];
  vm.ip() = (vm.reg(argv[1].i32).i32 & mask) | (vm.ip() & ~mask);
}


void vm_state_t::EXEC_JUMP(vm_state_t &vm, const value_t argv[]) {
  vm.ip() = vm.reg(argv[1].i32).i32;
}


void vm_state_t::EXEC_JNZ_L(vm_state_t &vm, const value_t argv[]) {
  const int32_t mask = cndmask[vm.reg(argv[0].i32).i32 != 0];
  vm.ip() = (argv[1].i32 & mask) | (vm.ip() & ~mask);
}


void vm_state_t::EXEC_JEZ_L(vm_state_t &vm, const value_t argv[]) {
  const int32_t mask = cndmask[vm.reg(argv[0].i32).i32 == 0];
  vm.ip() = (argv[1].i32 & mask) | (vm.ip() & ~mask);
}


void vm_state_t::EXEC_JGTE_L(vm_state_t &vm, const value_t argv[]) {
  const int32_t mask = cndmask[vm.reg(argv[0].i32).i32 >= 0];
  vm.ip() = (argv[1].i32 & mask) | (vm.ip() & ~mask);
}


void vm_state_t::EXEC_JLTE_L(vm_state_t &vm, const value_t argv[]) {
  const int32_t mask = cndmask[vm.reg(argv[0].i32).i32 <= 0];
  vm.ip() = (argv[1].i32 & mask) | (vm.ip() & ~mask);
}


void vm_state_t::EXEC_JLT_L(vm_state_t &vm, const value_t argv[]) {
  const int32_t mask = cndmask[vm.reg(argv[0].i32).i32 < 0];
  vm.ip() = (argv[1].i32 & mask) | (vm.ip() & ~mask);
}


void vm_state_t::EXEC_JGT_L(vm_state_t &vm, const value_t argv[]) {
  const int32_t mask = cndmask[vm.reg(argv[0].i32).i32 > 0];
  vm.ip() = (argv[1].i32 & mask) | (vm.ip() & ~mask);
}


void vm_state_t::EXEC_JUMP_L(vm_state_t &vm, const value_t argv[]) {
  vm.ip() = argv[0].i32;
}


void vm_state_t::EXEC_STORE(vm_state_t &vm, const value_t argv[]) {
  vm.stack(vm.reg(argv[0].i32).i32).ui32 = vm.reg(argv[1].i32).ui32;
}


void vm_state_t::EXEC_STORE_L(vm_state_t &vm, const value_t argv[]) {
  vm.stack(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32;
}


void vm_state_t::EXEC_GET(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.stack(argv[1].i32).ui32;
}


void vm_state_t::EXEC_GET_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.stack(vm.reg(argv[1].i32).i32).ui32;
}


void vm_state_t::EXEC_PUSH(vm_state_t &vm, const value_t argv[]) {
  vm.push(argv[0].ui32);
}


void vm_state_t::EXEC_POP(vm_state_t &vm, const value_t argv[]) {
  vm.pop(argv[0].ui32, true);
}


void vm_state_t::EXEC_LOAD(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = argv[1].ui32;
}


void vm_state_t::EXEC_MOVE(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32) = vm.reg(argv[1].i32);
}


void vm_state_t::EXEC_CALL(vm_state_t &vm, const value_t argv[]) {
  vm.exec_call(argv[0].i32, argv[1].ui32);
}


void vm_state_t::EXEC_CALL_D(vm_state_t &vm, const value_t argv[]) {
  vm.exec_call(vm.reg(argv[0].i32).i32, vm.reg(argv[1].i32).ui32);
}


void vm_state_t::EXEC_RETURN(vm_state_t &vm, const value_t argv[]) {
  value_t result = vm.reg(argv[0].i32);
  #ifdef VM_PRESERVE_CALL_ARGUMENT_REGISTERS
  vm.pop(vm.esp(), true);
  #endif
  vm.pop(CALL_STACK_MASK, true);
  vm.rp() = result;
  --vm._sequence;
}


void vm_state_t::EXEC_ALLOC(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.alloc(vm.reg(argv[1].i32).ui32);
}


void vm_state_t::EXEC_ALLOC_L(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.alloc(vm.reg(argv[1].i32).ui32);
}


void vm_state_t::EXEC_FREE(vm_state_t &vm, const value_t argv[]) {
  vm.free(vm.reg(argv[0].i32).ui32);
}


void vm_state_t::EXEC_PEEK8(vm_state_t &vm, const value_t argv[]) {
  const uint8_t *block = (const uint8_t *)vm.get_block(vm.reg(argv[1].i32).ui32, VM_MEM_NO_PERMISSIONS);
  vm.reg(argv[0].i32).ui32 = block[vm.reg(argv[2].i32).ui32];
}


void vm_state_t::EXEC_PEEK16(vm_state_t &vm, const value_t argv[]) {
  const uint8_t *block = (const uint8_t *)vm.get_block(vm.reg(argv[1].i32).ui32, VM_MEM_NO_PERMISSIONS);
  vm.reg(argv[0].i32).ui32 = *(const uint16_t *)(block + vm.reg(argv[2].i32).ui32);
}


void vm_state_t::EXEC_PEEK32(vm_state_t &vm, const value_t argv[]) {
  const uint8_t *block = (const uint8_t *)vm.get_block(vm.reg(argv[1].i32).ui32, VM_MEM_NO_PERMISSIONS);
  vm.reg(argv[0].i32).ui32 = *(const uint32_t *)(block + vm.reg(argv[2].i32).ui32);
}


void vm_state_t::EXEC_POKE8(vm_state_t &vm, const value_t argv[]) {
  uint8_t *block = (uint8_t *)vm.get_block(vm.reg(argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE);
  block[vm.reg(argv[1].i32).ui32] = (uint8_t)(vm.reg(argv[2].i32).ui32 & 0xFF);
}


void vm_state_t::EXEC_POKE16(vm_state_t &vm, const value_t argv[]) {
  uint8_t *block = (uint8_t *)vm.get_block(vm.reg(argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE);
  *(uint16_t *)(block + vm.reg(argv[1].i32).ui32) = (uint16_t)vm.reg(argv[2].i32).ui32 & 0xFFFF;
}


void vm_state_t::EXEC_POKE32(vm_state_t &vm, const value_t argv[]) {
  uint8_t *block = (uint8_t *)vm.get_block(vm.reg(argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE);
  *(uint32_t *)(block + vm.reg(argv[1].i32).ui32) = (uint32_t)vm.reg(argv[2].i32).ui32;
}


void vm_state_t::EXEC_PEEK8_L(vm_state_t &vm, const value_t argv[]) {
  const uint8_t *block = (const uint8_t *)vm.get_block(vm.reg(argv[1].i32).ui32, VM_MEM_NO_PERMISSIONS);
  vm.reg(argv[0].i32).ui32 = block[argv[2].ui32];
}


void vm_state_t::EXEC_PEEK16_L(vm_state_t &vm, const value_t argv[]) {
  const uint8_t *block = (const uint8_t *)vm.get_block(vm.reg(argv[1].i32).ui32, VM_MEM_NO_PERMISSIONS);
  vm.reg(argv[0].i32).ui32 = *(const uint16_t *)(block + argv[2].ui32);
}


void vm_state_t::EXEC_PEEK32_L(vm_state_t &vm, const value_t argv[]) {
  const uint8_t *block = (const uint8_t *)vm.get_block(vm.reg(argv[1].i32).ui32, VM_MEM_NO_PERMISSIONS);
  vm.reg(argv[0].i32).ui32 = *(const uint32_t *)(block + argv[2].ui32);
}


void vm_state_t::EXEC_POKE8_L(vm_state_t &vm, const value_t argv[]) {
  uint8_t *block = (uint8_t *)vm.get_block(vm.reg(argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE);
  block[argv[1].ui32] = (uint8_t)(vm.reg(argv[2].i32).ui32 & 0xFF);
}


void vm_state_t::EXEC_POKE16_L(vm_state_t &vm, const value_t argv[]) {
  uint8_t *block = (uint8_t *)vm.get_block(vm.reg(argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE);
  *(uint16_t *)(block + argv[1].ui32) = (uint16_t)vm.reg(argv[2].i32).ui32 & 0xFFFF;
}


void vm_state_t::EXEC_POKE32_L(vm_state_t &vm, const value_t argv[]) {
  uint8_t *block = (uint8_t *)vm.get_block(vm.reg(argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE);
  *(uint32_t *)(block + argv[1].ui32) = (uint32_t)vm.reg(argv[2].i32).ui32;
}


void vm_state_t::EXEC_MEMMOVE(vm_state_t &vm, const value_t argv[]) {
  uint8_t *block_out = ((uint8_t *)vm.get_block(vm.reg(argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE)) + vm.reg(argv[1].i32).ui32;
  uint8_t *block_in = ((uint8_t *)vm.get_block(vm.reg(argv[2].i32).ui32, VM_MEM_NO_PERMISSIONS)) + vm.reg(argv[3].i32).ui32;
  memmove(block_out, block_in, vm.reg(argv[4].i32).ui32);
}


void vm_state_t::EXEC_MEMMOVE_L(vm_state_t &vm, const value_t argv[]) {
  uint8_t *block_out = ((uint8_t *)vm.get_block(vm.reg(argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE)) + argv[1].ui32;
  uint8_t *block_in = ((uint8_t *)vm.get_block(vm.reg(argv[2].i32).ui32, VM_MEM_NO_PERMISSIONS)) + argv[3].ui32;
  memmove(block_out, block_in, argv[4].ui32);
}


void vm_state_t::EXEC_MEMDUP(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.duplicate_block(vm.reg(argv[1].i32).ui32);
}


void vm_state_t::EXEC_MEMLEN(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.block_size(vm.reg(argv[1].i32).ui32);
}


void vm_state_t::EXEC_LOGAND(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 && vm.reg(argv[2].i32).ui32;
}


void vm_state_t::EXEC_LOGOR(vm_state_t &vm, const value_t argv[]) {
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32 || vm.reg(argv[2].i32).ui32;
}


void vm_state_t::EXEC_TRAP(vm_state_t &vm, const value_t argv[]) {
  vm._trap = 1;
}


void vm_state_t::EXEC_SWAP(vm_state_t &vm, const value_t argv[]) {
  uint32_t temp = vm.reg(argv[0].i32).ui32;
  vm.reg(argv[0].i32).ui32 = vm.reg(argv[1].i32).ui32;
  vm.reg(argv[1].i32).ui32 = temp;
}
