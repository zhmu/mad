#include "debugmad.h"
#include "mad.h"
#include "types.h"

/* are we already included? */
#ifndef __OPCODES_INCLUDED__
/* no. set flag we are included, and define things */
#define __OPCODES_INCLUDED__

/* opcodes */
#define OPCODE_NOP            0x00
#define OPCODE_PUSH           0x01
#define OPCODE_PUSHA          0x02
#define OPCODE_TOSS           0x03
#define OPCODE_POPA           0x04
#define OPCODE_PUSHW          0x05

#define OPCODE_RET            0x0A
#define OPCODE_JMP            0x0B
#define OPCODE_AND            0x0C
#define OPCODE_OR             0x0D
#define OPCODE_XOR            0x0E
#define OPCODE_NOT            0x0F
#define OPCODE_ADD            0x10
#define OPCODE_SUB            0x11
#define OPCODE_MUL            0x12
#define OPCODE_DIV            0x13
#define OPCODE_INC            0x14
#define OPCODE_DEC            0x15

#define OPCODE_EQ             0x20
#define OPCODE_GT             0x21
#define OPCODE_LT             0x22
#define OPCODE_GE             0x23
#define OPCODE_LE             0x24

#define OPCODE_JS             0x30
#define OPCODE_JC             0x31

#define OPCODE_LDA            0x40
#define OPCODE_LAP            0x41
#define OPCODE_LDP            0x42

#define OPCODE_CLK            0x50
#define OPCODE_CLO            0x51

#endif
