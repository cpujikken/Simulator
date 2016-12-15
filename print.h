#ifndef _PRINT_H_
#define _PRINT_H_

void print_hex(unsigned int);
void print_mem(int);
void print_bin_byte(unsigned char);//1byteデータのバイナリ
void print_bin_little(unsigned int);//リトルエンディアンの32bitデータのバイナリ
void print_bin_big(unsigned int);//ビッグエンディアンの
void print_to_file(unsigned int);
void dprintr(unsigned int);
void dprintfr(unsigned int);
void print_reg();
void print_freg();
void print_pc();
void print_opc(unsigned int);
void print_pointer(void *);

#endif
