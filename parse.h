#ifndef _PARSE_H_
#define _PARSE_H_

typedef struct {
  unsigned int opc;
  unsigned int opr1;
  unsigned int opr2;
  unsigned int opr3;
  short const16; //off16も同じ
  unsigned int bits5;
  unsigned int off_addr26;//符号無しで良くなった
  int off21; //SIGNED!!!
} Operation;

typedef struct {
  unsigned int rd;
  short off16;
  unsigned int rs;
  unsigned int ro;
  unsigned int size4;
  unsigned int addr21;
} Ldst;

extern unsigned int read_mem32(unsigned int);
extern Operation parse(unsigned int);
extern Ldst parse_ldst(unsigned int);

#endif
