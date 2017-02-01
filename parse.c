#include <stdio.h>
#include "base.h"
#include "parse.h"

//メモリから32bitをunsigned int型で読み込む
//学科pcはリトルエンディアンなので、後ろから読み込む必要がある
unsigned int read_mem32(unsigned int mem_addr) {
  Mydata myd;
  int i;
  if(mem_addr >= MEM_SIZE-3) {
    stop = 1;
    printf("accessing nil memory(%d)\n",mem_addr);
    return 0;
  }
  for (i = 0;i < 4;i++) {
    myd.c[i] = memory[mem_addr + 3 - i];
  }
  return myd.i;
}

//命令を成分ごとに分解
Operation parse(unsigned int op) {
  Operation o;
  //オペコード=上位6bit register:5bit for each
  o.opc = op >> 26;
  o.opr1 = (op >> 21) & 0b11111;
  o.opr2 = (op >> 16) & 0b11111;
  o.opr3 = (op >> 11) & 0b11111;
  o.const16 = (short)(op & 0xffff);
  o.bits5 = (op >> 13) & 0b11111;
  o.off_addr26 = op & 0x3ffffff;//lower 26bit
  unsigned int hojo = op & 0x1fffff;//lower21bit.Whether sign bit is 1 or 0?
  if(hojo <= 0xfffff) {//<=> if top bit == 0 
    o.off21 = hojo;
  } else {
    o.off21 = (hojo - 0x100000) * -1;
  }
  return o;
}

Ldst parse_ldst(unsigned int op) {
  Ldst l;
  l.rd = (op >> 21) & 0b11111;
  l.off16 = (op >> 5) & 0xffff;
  l.rs = (op >> 16) & 0b11111;
  l.ro = (op >> 7) & 0b11111;
  l.size4 = (op >> 12) & 0b1111;
  l.addr21 = op & 0x1fffff;
  return l;
}
