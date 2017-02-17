#ifndef _PRINT_H_
#define _PRINT_H_
#include "parse.h"

extern void print_hex(unsigned int);
extern void print_mem(int);
extern void print_bin_byte(unsigned char);//1byteデータのバイナリ
extern void print_bin_little(unsigned int);//リトルエンディアンの32bitデータのバイナリ
extern void print_bin_big(unsigned int);//ビッグエンディアンの
extern void print_to_file(unsigned int);
extern void dprintr(unsigned int);
extern void dprintfr(unsigned int);
extern void print_reg();
extern void print_freg();
extern void set_stack(unsigned int);
extern void print_stack();
extern void print_pc();
extern void print_opc(unsigned int);
extern void print_op(Operation,Ldst);
extern void print_pointer(void *);
extern void print_com(unsigned int);
#endif
