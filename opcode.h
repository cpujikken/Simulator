#ifndef OPCODE_H
#define OPCODE_H
#define OP_NOP		0
#define OP_ADD		1
#define OP_SUB		2
#define OP_ADDI		3
//#define OP_SHIFTL	4
//#define OP_SHIFTR	5
#define OP_HALF		4
#define OP_FOUR		5
#define OP_J		6
#define OP_JZ		7
//#define OP_JEQ		7
//#define OP_JLT		8
//#define OP_LD		9
//#define OP_SD		10
#define OP_FJLT		8
#define OP_FADD		9
#define OP_FSUB		10
#define OP_FMUL		11
#define OP_FDIV		12
#define OP_FCMP		13
#define OP_FJEQ		14
#define OP_CMP		15
//#define OP_FZERO	27
//#define OP_FABS		28
//#define OP_FNEG		29
//#define OP_BEQF		31
//#define OP_JEQF		32
#define OP_JLINK	16
#define OP_LINK		17
#define OP_PUSH		18
#define OP_POP		19
#define OP_OUT		20
#define OP_LDATA	21 //???


#define OP_SAVE		37
#define OP_RESTORE	38
#define OP_SAVEH	39
#define OP_RESTOREH	40
#define OP_FLW          42
#define OP_FSW          43

#define OP_B		29
#define OP_BEQ		30

#define OP_AND		39
#define OP_NOT		40
#define OP_OR		41
#define OP_XOR		42

#endif

