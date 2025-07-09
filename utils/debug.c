#include "debug.h"

#define OPCODE(op) #op,
static const char* opcodes_to_string[] = {
  "DUMMY",
  #include "opcodes.inc"
};
#undef OPCODE

static size_t constant_instruction(Chunk* chunk, size_t offset, const char* name) {
  if (chunk->code[offset] == OP_CONSTANT) {
    uint8_t constant = chunk->code[offset + 1];
    printf("%-16s %hhu \"", name, constant);
    print_value(chunk->constants.values[constant]);
    printf("\"\n");
    return offset + 2;
  } else if (chunk->code[offset] == OP_CONSTANT_LONG) {
    uint8_t high = chunk->code[offset + 1];
    uint8_t mid = chunk->code[offset + 2];
    uint8_t low = chunk->code[offset + 3];
    uint32_t constant = (uint32_t) ((high << 16) | (mid << 8) | low);
    printf("%-26s %4u \"", name, constant);
    print_value(chunk->constants.values[constant]);
    printf("\"\n");
    return offset + 4;
  } else {
    fprintf(stderr, "Fatal: %s is not a constant opcode (%hhu, %hhu)", name, OP_CONSTANT, OP_CONSTANT_LONG);
    exit(1);
  }
}

static size_t simple_instruction(const char* name, size_t offset) {
  printf("%s\n", name);
  return offset + 1;
}

void disassemble_chunk(Chunk* chunk, const char* name) {
  printf("--- %s ---\n", name);
  for (size_t offset = 0; offset < chunk->count;) {
    offset = disassemble_instruction(chunk, offset);
  }
}

size_t disassemble_instruction(Chunk* chunk, size_t offset) {
  printf("%04lu", offset);

  size_t line = get_line(chunk, offset);
  if (line == get_line(chunk, offset - 1)) {
    printf("   | ");
  } else {
    printf("%4lu", line);
  }

  uint8_t instruction = chunk->code[offset];
  switch (instruction) {
    case OP_CONSTANT:
    case OP_CONSTANT_LONG:
      return constant_instruction(chunk, offset, opcodes_to_string[instruction]);
    case OP_NEGATE:
    case OP_ADD:
    case OP_SUBTRACT:
    case OP_MULTIPLY:
    case OP_DIVIDE:
    case OP_RETURN:
      return simple_instruction(opcodes_to_string[instruction], offset);
  }
}