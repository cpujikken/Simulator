#include <stdio.h>
#include <string.h>
#include "define.h"
#include "base.h"
#include "print.h"

//読み込んだ命令(32bit)を16進数表示 リトルエンディアン用
//だったのですが、バイナリで表示することにしました。そのほうが見やすいし。
void print_hex(unsigned int op) {
  print_bin_little(op);
}

//メモリの内容を直接見たい時
void print_mem(int addr) {
  printf("MEMORY[%d] = ",addr);
  print_bin_big(addr);
}

//1バイトをバイナリ表示
void print_bin_byte(unsigned char x){
  int i;
  unsigned char y = 0b10000000;
  for(i=0;i<8;i++) {
    if(x&y) {
      putchar('1');
    } else {
      putchar('0');
    }
    y = y >> 1;
  }
}

//リトルエンディアンの32bitデータxをビッグエンディアンに直してバイナリ表示
void print_bin_little(unsigned int x){
  Mydata myd;
  myd.i = x;
  int i;
  for(i=3;i>=0;i--){
    print_bin_byte(myd.c[i]);
    putchar(' ');
  }
  putchar('\n');
}

//メモリの内容を直接みたいとき
void print_bin_big(unsigned int x){
  int i;
  for(i=0;i<=3;i++){
    print_bin_byte(memory[x+i]);
    putchar(' ');
  }
  putchar('\n');
}

void print_to_file(unsigned int rnum) {
  //標準出力に出力する場合
  if(print_debug==0) {
    if(print_raw){
      printf("%d\n",reg[rnum]);
    } else {
      Mydata myd;
      myd.i = reg[rnum];
      int i;
      for(i=3;i>=0;i--){
	putchar(myd.c[i]);
      }
    }

    return;
  }

  //リトルエンディアン->ビッグエンディアン??
  //とりあえずリトルエンディアンのまま出力
  if(print_little) {
    fwrite((void *)(reg + rnum),sizeof(int),1,fp_out);
    if(print_debug) {
      printf(" => print out (in little endian) ");
      print_bin_little(reg[rnum]);
    }
  } else {
    Mydata myd;
    myd.i = reg[rnum];
    int i;
    for(i=3;i>=0;i--){
      fwrite((void *)(myd.c + i),sizeof(char),1,fp_out);
    }
    if(print_debug) {
      printf(" => print out (in big endian) ");
      print_bin_little(reg[rnum]);
    }
  }
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
      printf("%%r%d = %d\n",i,reg[i]);
  }
  printf("\n");
  return;
}

void print_freg() {
  int i;
  for (i = 0;i < 32;i++) {
    if(freg[i] != 0)
      printf("%%fr%d = %f\n",i,freg[i]);
  }
  printf("\n");
  return;
}

void print_pc() {
  printf("IP = %d\n",pc);
  return;
}

//未使用
void print_link_stack();

//統計情報をプリントアウト
void print_statistics() {
  int i;
  //命令使用回数をプリント
  for(i=0;i<NUM_OF_OP;i++) {
    if(used[i]>0) {
      print_opc(i);
      printf(":%d times\n",used[i]);
    }
  }
  
  //条件分岐回数をプリント
  for(i=0;i<NUM_OF_OP;i++) {
    if(branch[i] > 0 || nbranch[i] > 0) {
      print_opc(i);
      printf(" -> branched:%d times, not branched:%d times\n",
	     branch[i],nbranch[i]);
    }
  }
}

//シミュレータのデバッグ用、レジスタやメモリが指すポインタを表示
void print_pointer(void *p){
  printf("%10p\n",p);
};

//命令名を表示
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
    break;
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
    break;
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
  case OP_SIP:
    strcpy(o,"SIP");
    break;
  case OP_FIN:
    strcpy(o,"FIN");
    break;
  default:
    strcpy(o,"???");
    break;
  }
  printf("%s",o);
  return;
}
