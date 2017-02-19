#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "define.h"
#include "base.h"
#include "parse.h"
#include "label.h"
#include "print.h"

//読み込んだ命令(32bit)を16進数表示 リトルエンディアン用
//だったのですが、バイナリで表示することにしました。そのほうが見やすいし。
void print_hex(unsigned int op) {
  print_bin_little(op);
}

//メモリの内容を直接見たい時
void print_mem(int addr) {
  //printf("MEMORY[%d] = ",addr);
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
  printf("(big endian)\n");
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
  //標準出力に出力する場合 <-?
  if(print_debug==0 && print_to_stdin==1) {
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
  
  //リトルエンディアンとか関係なくなったので
  Mydata myd;
  myd.i = reg[rnum];
  fwrite((void *)(&myd.c),sizeof(char),1,fp_out);
  if(print_debug) {
    printf(" => print out %s\n",myd.c);
  }
}

//デバッグ情報表示モードのときのみ
void dprintr(unsigned int rnum) {
  if(print_debug)
    printf(" => %%r%d = %d\n",rnum,reg[rnum]);
  return;
}
void dprintfr(unsigned int rnum){
  if(print_debug)
    printf(" => %%fr%d = %f\n",rnum,freg[rnum]);
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
  i = reg_hp_max - init_hp;
  printf("heap domain : %d Byte (= %f MiB)\n",i,i/1024.0/1024.0);
  i = reg_sp_max - init_sp;
  printf("stack domain : %d Byte (= %f MiB)\n",i,i/1024.0/1024.0);
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

//統計情報をプリントアウト
void print_statistics() {
  int i=0;
  //命令使用回数をプリント
  for(i=0;i<NUM_OF_OP;i++) {
    if(used[i]>0) {
      print_opc(i);
      printf(":\t%ld\ttimes\n",used[i]);
    }
  }
  printf("dynamic instruction number :\t%ld\n",dyna);

  //系統ごとに分けて動的命令数をプリント
  long alu_op,fpu_op,jump_op,load_op,store_op,readwrite_op,other_op;
  alu_op =
    used[OP_ADD]
    + used[OP_SUB]
    + used[OP_ADDI]
    + used[OP_HALF]
    + used[OP_FOUR]
    + used[OP_CMP]
    + used[OP_MV]
    + used[OP_NEG1]
    + used[OP_NEG2]
    + used[OP_INC]
    + used[OP_DEC]
    + used[OP_INC1]
    + used[OP_DEC1]
    + used[OP_MVI]
    + used[OP_SL]
    + used[OP_SR]
    + used[OP_MUL]
    + used[OP_DIV];
  printf("alu related:\t%ld\t times\n",alu_op);
  fpu_op =
    used[OP_FADD]
    + used[OP_FSUB]
    + used[OP_FMUL]
    + used[OP_FDIV]
    + used[OP_FCMP]
    + used[OP_FNEG1]
    + used[OP_FNEG2]
    + used[OP_FMV]
    + used[OP_FABS];
  printf("fpu related:\t%ld\t times\n",fpu_op);
  load_op =
    used[OP_LINK]
    + used[OP_LDR]
    + used[OP_LDA]
    + used[OP_LDD]
    + used[OP_FLDR]
    + used[OP_FLDA]
    + used[OP_FLDD];
  printf("load related\t%ld\t times\n",load_op);
  store_op =
    used[OP_SDR]
    + used[OP_SDA]
    + used[OP_SDD]
    + used[OP_FSDR]
    + used[OP_FSDA]
    + used[OP_FSDD]
    + used[OP_SIP];
  printf("store related\t%ld\t times\n",store_op);
  jump_op =
    used[OP_J]
    + used[OP_JZ]
    + used[OP_FJLT]
    + used[OP_FJEQ]
    + used[OP_JC];
  printf("jump related:\t%ld\t times\n",jump_op);
  other_op =
    used[OP_NOP]
    + used[OP_PRINT]
    + used[OP_FIN]
    + used[OP_RC];
  printf("other operations(=NOP,PRINT,FIN,RC):\t%ld\t times\n",other_op);
  
  //条件分岐回数をプリント
  for(i=0;i<NUM_OF_OP;i++) {
    if(branch[i] > 0 || nbranch[i] > 0) {
      print_opc(i);
      printf(" -> branched : %d times, not branched : %d times\n",
	     branch[i],nbranch[i]);
    }
  }

  //メモリ使用量をカウント
  int use=0;
  printf("code size : %d Byte (= %f MiB)\n",codesize,codesize/1024.0/1024.0);
  for(i=0;i<MEM_SIZE/4;i++) {
    if(mem_used[i]) {
      use += 4;
    }
  }
  printf("memory usage : %d Byte (= %f MiB)\n",use,use/1024.0/1024.0);
}
void set_stack(unsigned int init) {
  if(sip_count < 0) {
    label_stack = calloc(MAX_FUN_DEPTH+1,sizeof(unsigned int));
  } else {    
    label_stack = calloc(sip_count+1,sizeof(unsigned int));
  }
    label_stack[0] = init;
}

//関数呼び出しがどうなってるか表示
void print_stack() {
  int i=0;
  printf("function call:\n");
  int j = used[OP_SIP] - used[OP_LINK];
  if(j>MAX_FUN_DEPTH) {
    j = MAX_FUN_DEPTH;
  }
  for(i=0;i<=j;i++) {
    printf("%s\n",addr2label(label_stack[i]));
  }
  if(used[OP_SIP] > used[OP_LINK]) {
    printf("関数呼び出しが深すぎて、最後まで表示できません。\nもっと表示したい場合はbase.hのMAX_FUN_DEPTHを変更してください。");
  }
}
//シミュレータのデバッグ用、レジスタやメモリが指すポインタを表示
void print_pointer(void *p){
  printf("%10p\n",p);
};

//命令名を表示
void print_opc(unsigned int opcode) {
  
  char o[25];
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
  case OP_MUL:
    strcpy(o,"MUL");
    break;
  case OP_DIV:
    strcpy(o,"DIV");
    break;
  case OP_SIP:
    strcpy(o,"SIP");
    break;
  case OP_FIN:
    strcpy(o,"FIN");
    break;
  case OP_CEQ:
    strcpy(o,"CEQ");
    break;
  case OP_RC:
    strcpy(o,"RC");
    break;
  default:
    printf("%d",opcode);
    strcpy(o,"(unknown opcode)");
    break;
  }
  printf("%s",o);
  return;
}

void print_com(unsigned int ip) {
  if(print_comment == 0){
    putchar('\n');
    return;
  }
  if(print_debug && fp_com != NULL) {
    int i = ip/4 - 1;//基本はip/4、initial_ipにコメントがない分-1した
    if(i >=0 && i < COMMENT_CODESIZE_MAX) {
      printf(" #%s",memory_com[i]);
    } else {
      putchar('\n');
    }
  }
}

void print_op(Operation o,Ldst l,unsigned int ip) {
  
  if(print_debug == 0) 
    return;
  
  
  unsigned int ra = o.opr1;
  unsigned int rb = o.opr2;
  unsigned int rc = o.opr3;
  
  //命令名を表示する
  //printf("operation: ");
  
  print_opc(o.opc);
  
  putchar('\t');
  
  //オペコードによる場合分け
  switch (o.opc) {
  case OP_NOP:
  case OP_LINK:
  case OP_FIN:
  case OP_JC:
  case OP_JLINKC:
  case OP_SIP:
    break;
  case OP_ADD:
  case OP_SUB:
  case OP_XOR:
  case OP_SL:
  case OP_SR:
  case OP_MUL:
  case OP_DIV:
    printf("%%r%d, %%r%d, %%r%d",ra,rb,rc);
    break;
  case OP_ADDI:
  case OP_LDR:
  case OP_SDR:
    printf("%%r%d, %%r%d, $%d",ra,rb,o.const16);
    break;
  case OP_HALF:
  case OP_FOUR:
  case OP_CMP:
  case OP_MV:
  case OP_NEG2:
  case OP_INC1:
  case OP_DEC1:
  case OP_CEQ:
    printf("%%r%d, %%r%d",ra,rb);
    break;
  case OP_J:
  case OP_JZ:
  case OP_FJLT:
  case OP_FJEQ:
  case OP_JLINK:
    printf("$%d",o.off_addr26);
    break;
  case OP_FADD:
  case OP_FSUB:
  case OP_FMUL:
  case OP_FDIV:
    printf("%%fr%d, %%fr%d, %%fr%d",ra,rb,rc);
    break;
  case OP_FCMP:
  case OP_FNEG2:
  case OP_FMV:
  case OP_FABS:
    printf("%%fr%d, %%fr%d",ra,rb);
    break;
  case OP_NEG1:
  case OP_INC:
  case OP_DEC:
  case OP_RI:
  case OP_PRINT:
  case OP_RC:
    printf("%%r%d",ra);
    break;
  case OP_FNEG1:
  case OP_RF:
    printf("%%fr%d",ra);
    break;
  case OP_MVI:
    printf("%%r%d, $%d",ra,o.off21);
    break;
  case OP_LDD:
  case OP_SDD:
    printf("%%r%d, %%r%d, $%d, %%r%d",l.rd,l.rs,l.size4,l.ro);
    break;
  case OP_LDA:
  case OP_SDA:
    printf("%%r%d, $%d",l.rd,l.addr21);
    break;
  case OP_FLDR:
  case OP_FSDR:
    printf("%%fr%d, %%r%d, $%d",ra,rb,o.const16);
    break;
  case OP_FLDD:
  case OP_FSDD:
    printf("%%fr%d, %%r%d, $%d, %%r%d",l.rd,l.rs,l.size4,l.ro);
    break;
  case OP_FLDA:
  case OP_FSDA:
    printf("%%fr%d, $%d",l.rd,l.addr21);
    break;
  }

  int i;
  if(print_debug) {
    printf(" \t#");
    if(label_info) {
      i = used[OP_SIP]-used[OP_LINK]-sipflag;
      if(i>=0 && (i<=sip_count || sip_count < 0) && i < MAX_FUN_DEPTH) {
	printf("%s,",addr2label(label_stack[i]));
      }
    }
    printf("IP=%d,din=%ld",pc,dyna);
  }
  //comment.sについていたコメントを表示
  print_com(pc);
  
  return;
}
