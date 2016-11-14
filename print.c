#include <stdio.h>
#include <string.h>
#include "base.h"
#include "print.h"

//読み込んだ命令(32bit)を16進数表示 リトルエンディアン用
void print_hex(unsigned int op) {
  Mydata myd;
  myd.i = op;
  printf("%x %x %x %x\n",myd.c[3],myd.c[2],myd.c[1],myd.c[0]);
  return;
}

//メモリの内容を直接見たい時用
void print_mem_hex(int addr) {
  printf("MEMORY[%d] = %x %x %x %x\n",addr,
	 memory[addr],memory[addr+1],memory[addr+2],memory[addr+3]);
  return;
}
void print_to_file(int rnum) {
  //リトルエンディアン->ビッグエンディアン??
  //とりあえずリトルエンディアンのまま出力
  fwrite((void *)(reg + rnum),sizeof(int),1,fp_out);
  return;
}

//デバッグ情報表示モードのときのみ
void dprintr(unsigned int rnum) {
  if(print_debug)
    printf(" => r%d = %d\n",rnum,reg[rnum]);
  return;
}
void dprintfr(unsigned int rnum){
  if(print_debug)
    printf(" => fr%d = %f\n",rnum,freg[rnum]);
  return;
}

//表示用の関数群
//0でない値が入っているレジスタを表示
void print_reg() {
  int i;
  for (i = 0;i < 32;i++) {
    if(reg[i] != 0)
      printf("r%d = %d, ",i,reg[i]);
  }
  printf("\n");
  return;
}

void print_freg() {
  int i;
  for (i = 0;i < 32;i++) {
    if(freg[i] != 0)
      printf("fr%d = %f, ",i,freg[i]);
  }
  printf("\n");
  return;
}

void print_pc() {
  printf("IP = %d\n",pc);
  return;
}

//統計情報をプリントアウト
void print_statistics() {
  int i;
  //命令使用回数をプリント
  for(i=0;i<NUM_OF_OP;i++) {
    if(used[i]>0) {
      print_opc(i);
      printf(":%d times,",used[i]);
    }
  }

  //条件分岐回数をプリント
  printf("\nbranched:%d times, not branched:%d times\n",branch,nbranch);
}

void print_opc(unsigned int opcode) {
  char o[10];
  switch (opcode) {
  case OP_NOP:
    strcpy(o,"NOP");
    break;
  case OP_ADD:
    strcpy(o,"ADD");
    break;
  case OP_SUB:
    strcpy(o,"SUB");
    break;
  case OP_ADDI:
    strcpy(o,"ADDI");
    break;
  case OP_HALF:
    strcpy(o,"HALF");	
    break;
  case OP_FOUR:
    strcpy(o,"FOUR");
    break;
  case OP_J:
    strcpy(o,"J");
    break;
  case OP_JZ:
    strcpy(o,"JZ");
    break;
  case OP_FJLT:
    strcpy(o,"FJLT");
    break;
  case OP_FADD:
    strcpy(o,"FADD");
    break;
  case OP_FSUB:
    strcpy(o,"FSUB");
    break;
  case OP_FMUL:
    strcpy(o,"FMUL");
    break;
  case OP_FDIV:
    strcpy(o,"FDIV");
    break;
  case OP_FCMP:
    strcpy(o,"FCMP");  
  case OP_FJEQ:
    strcpy(o,"FJEQ");
    break;
  case OP_CMP:
    strcpy(o,"CMP");
    break;
  case OP_JLINK:
    strcpy(o,"JLINK");
    break;
  case OP_LINK:
    strcpy(o,"LINK");
    break;
  case OP_OUT:
    strcpy(o,"OUT");
    break;
  case OP_JC:
    strcpy(o,"JC");
    break;
  case OP_JLINKC:
    strcpy(o,"JLINKC");
    break;
  case OP_MV:
    strcpy(o,"MV");
    break;
  case OP_NEG1:
    strcpy(o,"NEG1");
    break;
  case OP_FNEG1:
    strcpy(o,"FNEG1");
    break;
  case OP_NEG2:
    strcpy(o,"NEG2");
    break;
  case OP_FNEG2:
    strcpy(o,"FNEG2");
    break;
  case OP_INC:
    strcpy(o,"INC");
    break;
  case OP_DEC:
    strcpy(o,"DEC");
    break;
  case OP_INC1:
    strcpy(o,"INC1");
   break;
  case OP_DEC1:
    strcpy(o,"DEC1");
    break;
  case OP_MVI:
    strcpy(o,"MVI");
    break;
  case OP_LDR:
    strcpy(o,"LDR");
    break;
  case OP_LDD:
    strcpy(o,"LDD");
    break;
  case OP_LDA:
    strcpy(o,"LDA");
    break;
  case OP_SDR:
    strcpy(o,"SDR");
    break;
  case OP_SDD:
    strcpy(o,"SDD");
    break;
  case OP_SDA:
    strcpy(o,"SDA");
    break;
  case OP_FLDR:
    strcpy(o,"FLDR");
    break;
  case OP_FLDD:
    strcpy(o,"FLDD");
    break;
  case OP_FLDA:
    strcpy(o,"FLDA");
    break;
  case OP_FSDR:
    strcpy(o,"FSDR");
    break;
  case OP_FSDD:
    strcpy(o,"FSDD");
    break;
  case OP_FSDA:
    strcpy(o,"FSDA");
    break;
  case OP_XOR:
    strcpy(o,"XOR");
    break;
  case OP_FMV:
    strcpy(o,"FMV");
  case OP_SL:
    strcpy(o,"SL");
    break;
  case OP_SR:
    strcpy(o,"SR");
    break;
  case OP_RF:
    strcpy(o,"RF");
    break;
  case OP_RI:
    strcpy(o,"RI");
    break;
  case OP_PRINT:
    strcpy(o,"PRINT");
    break;
  case OP_FABS:
    strcpy(o,"FABS");
    break;
  default:
    strcpy(o,"???");
    break;
  }
  printf("%s",o);
  return;
}
