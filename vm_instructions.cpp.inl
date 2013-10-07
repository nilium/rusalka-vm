{
  VM_INSTR_DISPATCH_NOP: goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_ADD_F32: {
    reg(op.argv[0].i32).f32 = reg(op.argv[1].i32).f32 + reg(op.argv[2].i32).f32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_ADD_I32: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 + reg(op.argv[2].i32).i32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_ADD_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 + reg(op.argv[2].i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_ADD_F32_L: {
    reg(op.argv[0].i32).f32 = reg(op.argv[1].i32).f32 + op.argv[2].f32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_ADD_I32_L: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 + op.argv[2].i32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_ADD_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 + op.argv[2].ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_SUB_F32: {
    reg(op.argv[0].i32).f32 = reg(op.argv[1].i32).f32 - reg(op.argv[2].i32).f32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_SUB_I32: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 - reg(op.argv[2].i32).i32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_SUB_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 - reg(op.argv[2].i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_SUB_F32_L: {
    reg(op.argv[0].i32).f32 = reg(op.argv[1].i32).f32 - op.argv[2].f32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_SUB_I32_L: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 - op.argv[2].i32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_SUB_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 - op.argv[2].ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_DIV_F32: {
    reg(op.argv[0].i32).f32 = reg(op.argv[1].i32).f32 / reg(op.argv[2].i32).f32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_DIV_I32: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 / reg(op.argv[2].i32).i32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_DIV_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 / reg(op.argv[2].i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_DIV_F32_L: {
    reg(op.argv[0].i32).f32 = reg(op.argv[1].i32).f32 / op.argv[2].f32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_DIV_I32_L: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 / op.argv[2].i32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_DIV_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 / op.argv[2].ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_MUL_F32: {
    reg(op.argv[0].i32).f32 = reg(op.argv[1].i32).f32 * reg(op.argv[2].i32).f32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_MUL_I32: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 * reg(op.argv[2].i32).i32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_MUL_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 * reg(op.argv[2].i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_MUL_F32_L: {
    reg(op.argv[0].i32).f32 = reg(op.argv[1].i32).f32 * op.argv[2].f32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_MUL_I32_L: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 * op.argv[2].i32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_MUL_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 * op.argv[2].ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_MOD_F32: {
    reg(op.argv[0].i32).f32 = std::fmod(reg(op.argv[1].i32).f32, reg(op.argv[2].i32).f32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_MOD_I32: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 % reg(op.argv[2].i32).i32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_MOD_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 % reg(op.argv[2].i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_MOD_F32_L: {
    reg(op.argv[0].i32).f32 = std::fmod(reg(op.argv[1].i32).f32, op.argv[2].f32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_MOD_I32_L: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 % op.argv[2].i32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_MOD_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 % op.argv[2].ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_NEG_F32: {
    reg(op.argv[0].i32).f32 = -reg(op.argv[1].i32).f32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_NEG_I32: {
    reg(op.argv[0].i32).i32 = -reg(op.argv[1].i32).i32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_NOT_UI32: {
    reg(op.argv[0].i32).ui32 = ~(reg(op.argv[1].i32).ui32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_OR_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 | reg(op.argv[2].i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_XOR_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 ^ reg(op.argv[2].i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_AND_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 & reg(op.argv[2].i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_SHR_I32: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 >> reg(op.argv[2].i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_SHR_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 >> reg(op.argv[2].i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_SHL_I32: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 << reg(op.argv[2].i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_SHL_UI32: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 << reg(op.argv[2].i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_OR_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 | op.argv[2].ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_XOR_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 ^ op.argv[2].ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_AND_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 & op.argv[2].ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_SHR_I32_L: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 >> op.argv[2].ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_SHR_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 >> op.argv[2].ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_SHL_I32_L: {
    reg(op.argv[0].i32).i32 = reg(op.argv[1].i32).i32 << op.argv[2].ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_SHL_UI32_L: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 << op.argv[2].ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_ITOUI: {
    reg(op.argv[0].i32).ui32 = (uint32_t)reg(op.argv[1].i32).i32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_ITOF: {
    reg(op.argv[0].i32).f32 = static_cast<float>(reg(op.argv[1].i32).i32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_FTOUI: {
    reg(op.argv[0].i32).ui32 = static_cast<uint32_t>(reg(op.argv[1].i32).f32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_FTOI: {
    reg(op.argv[0].i32).i32 = static_cast<int32_t>(reg(op.argv[1].i32).f32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_UITOF: {
    reg(op.argv[0].i32).f32 = static_cast<float>(reg(op.argv[1].i32).ui32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_UITOI: {
    reg(op.argv[0].i32).i32 = static_cast<int32_t>(reg(op.argv[1].i32).ui32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_CMP_F32: {
    const float l = reg(op.argv[1].i32).f32;
    const float r = reg(op.argv[2].i32).f32;
    reg(op.argv[0].i32).i32 = (l > r) - (l < r);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_CMP_I32: {
    const int32_t l = reg(op.argv[1].i32).i32;
    const int32_t r = reg(op.argv[2].i32).i32;
    reg(op.argv[0].i32).i32 = (l > r) - (l < r);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_CMP_UI32: {
    const uint32_t l = reg(op.argv[1].i32).ui32;
    const uint32_t r = reg(op.argv[2].i32).ui32;
    reg(op.argv[0].i32).i32 = (l > r) - (l < r);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_CMP_F32_L: {
    const float l = reg(op.argv[1].i32).f32;
    const float r = op.argv[2].f32;
    reg(op.argv[0].i32).i32 = (l > r) - (l < r);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_CMP_I32_L: {
    const int32_t l = reg(op.argv[1].i32).i32;
    const int32_t r = op.argv[2].i32;
    reg(op.argv[0].i32).i32 = (l > r) - (l < r);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_CMP_UI32_L: {
    const uint32_t l = reg(op.argv[1].i32).ui32;
    const uint32_t r = op.argv[2].ui32;
    reg(op.argv[0].i32).i32 = (l > r) - (l < r);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_RCMP_F32: {
    const float l = reg(op.argv[1].i32).f32;
    const float r = reg(op.argv[2].i32).f32;
    reg(op.argv[0].i32).i32 = (l < r) - (l > r);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_RCMP_I32: {
    const int32_t l = reg(op.argv[1].i32).i32;
    const int32_t r = reg(op.argv[2].i32).i32;
    reg(op.argv[0].i32).i32 = (l < r) - (l > r);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_RCMP_UI32: {
    const uint32_t l = reg(op.argv[1].i32).ui32;
    const uint32_t r = reg(op.argv[2].i32).ui32;
    reg(op.argv[0].i32).i32 = (l < r) - (l > r);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_RCMP_F32_L: {
    const float l = reg(op.argv[1].i32).f32;
    const float r = op.argv[2].f32;
    reg(op.argv[0].i32).i32 = (l < r) - (l > r);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_RCMP_I32_L: {
    const int32_t l = reg(op.argv[1].i32).i32;
    const int32_t r = op.argv[2].i32;
    reg(op.argv[0].i32).i32 = (l < r) - (l > r);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_RCMP_UI32_L: {
    const uint32_t l = reg(op.argv[1].i32).ui32;
    const uint32_t r = op.argv[2].ui32;
    reg(op.argv[0].i32).i32 = (l < r) - (l > r);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_JNZ: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 != 0];
    ip() = (reg(op.argv[1].i32).i32 & mask) | (ip() & ~mask);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_JEZ: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 == 0];
    ip() = (reg(op.argv[1].i32).i32 & mask) | (ip() & ~mask);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_JGTE: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 >= 0];
    ip() = (reg(op.argv[1].i32).i32 & mask) | (ip() & ~mask);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_JLTE: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 <= 0];
    ip() = (reg(op.argv[1].i32).i32 & mask) | (ip() & ~mask);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_JLT: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 < 0];
    ip() = (reg(op.argv[1].i32).i32 & mask) | (ip() & ~mask);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_JGT: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 > 0];
    ip() = (reg(op.argv[1].i32).i32 & mask) | (ip() & ~mask);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_JUMP: {
    ip() = reg(op.argv[1].i32).i32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_JNZ_L: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 != 0];
    ip() = (op.argv[1].i32 & mask) | (ip() & ~mask);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_JEZ_L: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 == 0];
    ip() = (op.argv[1].i32 & mask) | (ip() & ~mask);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_JGTE_L: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 >= 0];
    ip() = (op.argv[1].i32 & mask) | (ip() & ~mask);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_JLTE_L: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 <= 0];
    ip() = (op.argv[1].i32 & mask) | (ip() & ~mask);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_JLT_L: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 < 0];
    ip() = (op.argv[1].i32 & mask) | (ip() & ~mask);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_JGT_L: {
    const int32_t mask = cndmask[reg(op.argv[0].i32).i32 > 0];
    ip() = (op.argv[1].i32 & mask) | (ip() & ~mask);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_JUMP_L: {
    ip() = op.argv[0].i32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_STORE: {
    stack(reg(op.argv[0].i32).i32).ui32 = reg(op.argv[1].i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_STORE_L: {
    stack(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_GET: {
    reg(op.argv[0].i32).ui32 = stack(op.argv[1].i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_GET_L: {
    reg(op.argv[0].i32).ui32 = stack(reg(op.argv[1].i32).i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_PUSH: {
    push(op.argv[0].ui32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_POP: {
    pop(op.argv[0].ui32, true);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_LOAD: {
    reg(op.argv[0].i32).ui32 = op.argv[1].ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_MOVE: {
    reg(op.argv[0].i32) = reg(op.argv[1].i32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_CALL: {
    exec_call(op.argv[0].i32, op.argv[1].ui32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_CALL_D: {
    exec_call(reg(op.argv[0].i32).i32, reg(op.argv[1].i32).ui32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_RETURN: {
    value_t result = reg(op.argv[0].i32);
    #ifdef VM_PRESERVE_CALL_ARGUMENT_REGISTERS
    pop(esp(), true);
    #endif
    pop(CALL_STACK_MASK, true);
    rp() = result;
    --_sequence;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_ALLOC: {
    reg(op.argv[0].i32).ui32 = alloc(reg(op.argv[1].i32).ui32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_ALLOC_L: {
    reg(op.argv[0].i32).ui32 = alloc(reg(op.argv[1].i32).ui32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_FREE: {
    free(reg(op.argv[0].i32).ui32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_PEEK8: {
    const uint8_t *block = (const uint8_t *)get_block(reg(op.argv[1].i32).ui32, VM_MEM_NO_PERMISSIONS);
    reg(op.argv[0].i32).ui32 = block[reg(op.argv[2].i32).ui32];
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_PEEK16: {
    const uint8_t *block = (const uint8_t *)get_block(reg(op.argv[1].i32).ui32, VM_MEM_NO_PERMISSIONS);
    reg(op.argv[0].i32).ui32 = *(const uint16_t *)(block + reg(op.argv[2].i32).ui32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_PEEK32: {
    const uint8_t *block = (const uint8_t *)get_block(reg(op.argv[1].i32).ui32, VM_MEM_NO_PERMISSIONS);
    reg(op.argv[0].i32).ui32 = *(const uint32_t *)(block + reg(op.argv[2].i32).ui32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_POKE8: {
    uint8_t *block = (uint8_t *)get_block(reg(op.argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE);
    block[reg(op.argv[1].i32).ui32] = (uint8_t)(reg(op.argv[2].i32).ui32 & 0xFF);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_POKE16: {
    uint8_t *block = (uint8_t *)get_block(reg(op.argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE);
    *(uint16_t *)(block + reg(op.argv[1].i32).ui32) = (uint16_t)reg(op.argv[2].i32).ui32 & 0xFFFF;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_POKE32: {
    uint8_t *block = (uint8_t *)get_block(reg(op.argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE);
    *(uint32_t *)(block + reg(op.argv[1].i32).ui32) = (uint32_t)reg(op.argv[2].i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_PEEK8_L: {
    const uint8_t *block = (const uint8_t *)get_block(reg(op.argv[1].i32).ui32, VM_MEM_NO_PERMISSIONS);
    reg(op.argv[0].i32).ui32 = block[op.argv[2].ui32];
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_PEEK16_L: {
    const uint8_t *block = (const uint8_t *)get_block(reg(op.argv[1].i32).ui32, VM_MEM_NO_PERMISSIONS);
    reg(op.argv[0].i32).ui32 = *(const uint16_t *)(block + op.argv[2].ui32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_PEEK32_L: {
    const uint8_t *block = (const uint8_t *)get_block(reg(op.argv[1].i32).ui32, VM_MEM_NO_PERMISSIONS);
    reg(op.argv[0].i32).ui32 = *(const uint32_t *)(block + op.argv[2].ui32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_POKE8_L: {
    uint8_t *block = (uint8_t *)get_block(reg(op.argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE);
    block[op.argv[1].ui32] = (uint8_t)(reg(op.argv[2].i32).ui32 & 0xFF);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_POKE16_L: {
    uint8_t *block = (uint8_t *)get_block(reg(op.argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE);
    *(uint16_t *)(block + op.argv[1].ui32) = (uint16_t)reg(op.argv[2].i32).ui32 & 0xFFFF;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_POKE32_L: {
    uint8_t *block = (uint8_t *)get_block(reg(op.argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE);
    *(uint32_t *)(block + op.argv[1].ui32) = (uint32_t)reg(op.argv[2].i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_MEMMOVE: {
    uint8_t *block_out = ((uint8_t *)get_block(reg(op.argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE)) + reg(op.argv[1].i32).ui32;
    uint8_t *block_in = ((uint8_t *)get_block(reg(op.argv[2].i32).ui32, VM_MEM_NO_PERMISSIONS)) + reg(op.argv[3].i32).ui32;
    memmove(block_out, block_in, reg(op.argv[4].i32).ui32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_MEMMOVE_L: {
    uint8_t *block_out = ((uint8_t *)get_block(reg(op.argv[0].i32).ui32, VM_MEM_WRITABLE | VM_MEM_READABLE)) + op.argv[1].ui32;
    uint8_t *block_in = ((uint8_t *)get_block(reg(op.argv[2].i32).ui32, VM_MEM_NO_PERMISSIONS)) + op.argv[3].ui32;
    memmove(block_out, block_in, op.argv[4].ui32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_MEMDUP: {
    reg(op.argv[0].i32).ui32 = duplicate_block(reg(op.argv[1].i32).ui32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_MEMLEN: {
    reg(op.argv[0].i32).ui32 = block_size(reg(op.argv[1].i32).ui32);
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_LOGAND: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 && reg(op.argv[2].i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_LOGOR: {
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32 || reg(op.argv[2].i32).ui32;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_TRAP: {
    _trap = 1;
  } goto __VM_INSTR_DISPATCH_FINISHED;
  VM_INSTR_DISPATCH_SWAP: {
    uint32_t temp = reg(op.argv[0].i32).ui32;
    reg(op.argv[0].i32).ui32 = reg(op.argv[1].i32).ui32;
    reg(op.argv[1].i32).ui32 = temp;
  }
}
__VM_INSTR_DISPATCH_FINISHED:;