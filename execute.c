#include <stdio.h>
#include <string.h>
#include <math.h>
#include "define.h"
#include "base.h"
#include "print.h"
#include "label.h"
#include "parse.h" //読み取る系はこっちに移転
#include "execute.h"

int sipflag = 0;

int setflag(int rnum) {
  if(reg[rnum] == 0) {
    flag[ZF] = 1;
  } else {
    flag[ZF] = 0;
  }
  if(print_debug) {
    printf(" => r%d = %d\n",rnum,reg[rnum]);
  }
  return 0;
}


/*
 * ロードストア
 * メモリにはビッグエンディアンで書かれてるが、レジスタに載せる際は
 * リトルエンディアンにしないと、シミュレータ的にはすごく不便
 */
//アクセスするメモリの値がおかしい場合は停止
int isnil(unsigned int addr) {
  if(addr >= MEM_SIZE) {
    printf("accessing nil memory(MEMORY[%d])\n",addr);
    stop = 1;
    return 1;
  }
  return 0;
}

int load(unsigned int rnum,unsigned int addr) {
  if(isnil(addr) != 0) {
    return 1;
  }
  Mydata md;
  unsigned int i;
  md.i = 0;
  for(i=0;i<4;i++) {
    md.c[3-i] = memory[addr+i]; //ビッグエンディアン->リトルエンディアン
  }
  reg[rnum] = md.i;
  if(print_debug)
    printf(" => LOADED %d FROM MEMORY[%d]\n",reg[rnum],addr);
  if(print_stat) {
    mem_used[addr/4]=1;
  }  
  return 0;
}

int fload(unsigned int rnum,int addr) {
  if(isnil(addr)) {
    return 1;
  }

  Mydata md;
  int i;

  md.f = 0;
  for(i=0;i<4;i++) {
    md.c[3-i] = memory[addr+i];
  }
  freg[rnum] = md.f;
  if(print_debug)
    printf(" => LOADED %f FROM MEMORY[%d]\n",freg[rnum],addr);
  if(print_stat) {
    mem_used[addr/4]=1;
  }
  return 0;
}

int store(unsigned int rnum,int addr) {
  if(isnil(addr) != 0) {
    return 1;
  }
  Mydata md;
  int i;
  
  md.i = reg[rnum];
  for(i=0;i<4;i++) {
    memory[addr+i] = md.c[3-i];
  }
  if(print_debug)
    printf(" => STORED %d TO MEMORY[%d]\n",md.i,addr);
  return 0;
}

int fstore(unsigned int rnum,unsigned int addr) {
  if(isnil(addr)) {
    return 1;
  }

  Mydata md;
  int i;
  md.f = freg[rnum];
  for(i=0;i<4;i++) {
    memory[addr+i] = md.c[3-i];//リトルエンディアン
  }
  if(print_debug)
    printf(" => STORED %f TO MEMORY[%d]\n",md.f,addr);
  return 0;
}

//ジャンプ デバッグ情報ON時、どこへジャンプしたか表示するUIにした
void jump(unsigned int j) {
  //0番地にジャンプすることはないはずなので、その時は停止する
  if(j <= 0) {
    printf("jumped to wrong address(%d)",j);
    printf(" at IP=%d\n",pc);
    stop = 1;
    return;
  }
  printf(" => jumping from %d(%s) to %d(%s)\n",pc,addr2label(pc),j,addr2label(j));

  pc = j;
  if(mode_jump) {
    mode_jump = 0;
    mode_step = 1;
  }
  /*
  if(print_debug) {
    printf(" => JUMPED TO %d(%s)\n",j,addr2label(j));
  }
  */
}
//ジャンプ非成立時の表示
void nojump() {
  if(print_debug)
    printf(" => NO JUMP\n");
}


//浮動小数点演算実行後、値がinfになったら止まる
void stop_ifinf(int rnum) {
  if(isinf(freg[rnum])) {
    stop = 1;
    printf("%%fr%d is infinity\n",rnum);
  }  
}

//コード一行を実行
int execute(unsigned int op ,Operation o, Ldst l) {
  int status;
  unsigned int ra = o.opr1;
  unsigned int rb = o.opr2;
  unsigned int rc = o.opr3;
  Mydata md,md2;
  int i;
  double doub;

  //命令使用回数をカウント
  used[o.opc]++;
  dyna++;

  //オペコードによる場合分け
  switch (o.opc) {
  case OP_NOP:
    break;
  case OP_ADD:
    reg[ra] = reg[rb] + reg[rc];
    dprintr(ra);
    //setflag(ra);
    break;
  case OP_SUB:
    reg[ra] = reg[rb] - reg[rc];
    dprintr(ra);
    //setflag(ra);
    break;
  case OP_ADDI:
    reg[ra] = reg[rb] + o.const16;
    dprintr(ra);
    break;
  case OP_HALF:
    reg[ra] = reg[rb] >> 1;
    dprintr(ra);
    break;
  case OP_FOUR:
    reg[ra] = reg[rb] << 2;
    dprintr(ra);
    break;
  case OP_J:
    if(sipflag) {
      //print_com(pc-4);
      sipflag = 0;
    }
    if(used[OP_SIP] > sip_count) {
      printf("function call over %d times\n",sip_count);
      stop = 1;
    } else {
      jump(o.off_addr26);
    }
    break;
  case OP_JZ:
    if(flag[ZF]) {
      jump(o.off_addr26);
      branch[OP_JZ]++;
    } else {
      nojump();
      nbranch[OP_JZ]++;
    }
    break;
  case OP_FJLT:
    if(float_flag[0] == FLT) {
      jump(o.off_addr26);
      branch[OP_FJLT]++;
    } else {
      nojump();
      nbranch[OP_FJLT]++;
    }
    break;
  case OP_FADD:
    freg[ra] = freg[rb] + freg[rc];
    dprintfr(ra);
    stop_ifinf(ra);
    break;
  case OP_FSUB:
    freg[ra] = freg[rb] - freg[rc];
    dprintfr(ra);
    stop_ifinf(ra);
    break;
  case OP_FMUL:
    freg[ra] = freg[rb] * freg[rc];
    dprintfr(ra);
    stop_ifinf(ra);
    break;
  case OP_FDIV:
    if(freg[rc] == 0) {
      printf("devision by %f\n",freg[rc]);
      stop = 1;
      } else {
      freg[ra] = freg[rb] / freg[rc];
      dprintfr(ra);
      stop_ifinf(ra);
      }
    break;
  case OP_FCMP:
    if(freg[ra] > freg[rb]) {
      float_flag[0] = FGT;
      if(print_debug)
	printf("%f > %f\n => FGT\n",freg[ra],freg[rb]);
    } else if(freg[ra] < freg[rb]) {
      float_flag[0] = FLT;
      if(print_debug)
	printf("%f < %f\n => FLT\n",freg[ra],freg[rb]);
    } else {
      float_flag[0] = FEQ;
      if(print_debug)
	printf("%f = %f\n => FEQ\n",freg[ra],freg[rb]);
    }
    break;
  case OP_FJEQ:
    if(float_flag[0] == FEQ) {
      jump(o.off_addr26);
      branch[OP_FJEQ]++;
    } else {
      nojump();
      nbranch[OP_FJEQ]++;
    }
    break;
  case OP_CMP:
    if(reg[ra] >= reg[rb]) {
      flag[ZF] = 0;//reset zero flag
      if(print_debug)
	{
	  printf(" %d >= %d => reset ZF\n",reg[ra],reg[rb]);
	}
    } else {
      flag[ZF] = 1;//set zero flag
      if(print_debug)
	printf("%d < %d => set ZF\n",reg[ra],reg[rb]);
    }
    break;
  case OP_LINK:
    i = reg[REG_SP] - 4;
    if(i < 0) {
      printf("accessing nil memory(%d) : at IP = %d\n",i,pc);
    }
    i = read_mem32(i);//stack pointer-4番地をロード
    if(print_debug)
      printf(" READ %d FROM MEMORY[%d]\n ",i,reg[REG_SP]-4);
    reg[REG_SP] = reg[REG_SP] - 4;//pop
    jump(i);
    call_stack--;
    break;
  case OP_JC:
    i = read_mem32(reg[REG_CL]);//stack pointer番地をロード
    if(sipflag) {
      //print_com(pc-4);
      sipflag = 0;
    }
    if(used[OP_SIP] > sip_count) {
      printf("function call over %d times\n",sip_count);
      stop = 1;
    } else if(print_debug) {
      printf(" READ %d FROM MEMORY[%d]\n ",i,reg[REG_SP]);
    }
    jump(i);
    break;
  case OP_MV:
    reg[ra] = reg[rb];
    dprintr(ra);
    break;
  case OP_NEG1:
    reg[ra] = -1 * reg[ra];
    dprintr(ra);
    break;
  case OP_FNEG1:
    freg[ra] = -1 * freg[ra];
    dprintfr(ra);
    break;
  case OP_NEG2:
    reg[ra] = -1 * reg[rb];
    dprintr(ra);
    break;
  case OP_FNEG2:
    freg[ra] = -1 * freg[rb];
    dprintfr(ra);
    break;
  case OP_INC:
    reg[ra] = reg[ra] + 1;
    dprintr(ra);
    break;
  case OP_DEC:
    reg[ra] = reg[ra] - 1;
    dprintr(ra);
    break;
  case OP_INC1:
    reg[ra] = reg[rb] + 1;
    dprintr(ra);
    break;
  case OP_DEC1:
    reg[ra] = reg[rb] - 1;
    dprintr(ra);
    break;
  case OP_MVI:
    reg[ra] = o.off21;
    dprintr(ra);
    break;
  case OP_LDR:
    load(ra,reg[rb]+o.const16);
    break;
  case OP_LDD:
    i = reg[l.rs] + l.size4 * reg[l.ro];
    if(print_debug) {
      printf(" => %%r%d + %d * %%r%d = %d",l.rs,l.size4,l.ro,i);
    }
    load(l.rd,i);
    break;
  case OP_LDA:
    load(l.rd,l.addr21);
    break;
  case OP_SDR:
    store(l.rd,reg[rb]+o.const16);
    break;
  case OP_SDD:
    i = reg[l.rs] + l.size4 * reg[l.ro];
    if(print_debug) {
      printf(" => %%r%d + %d * %%r%d = %d",l.rs,l.size4,l.ro,i);
    }
    store(l.rd,i);
    break;
  case OP_SDA:
    store(l.rd,l.addr21);
    break;
  case OP_FLDR:
    fload(l.rd,reg[rb]+o.const16);
    break;
  case OP_FLDD:
    i = reg[l.rs] + l.size4 * reg[l.ro];
    if(print_debug) {
      printf(" => %%r%d + %d * %%r%d = %d",l.rs,l.size4,l.ro,i);
    }
    fload(l.rd,i);
    break;
  case OP_FLDA:
    fload(l.rd,l.addr21);
    break;
  case OP_FSDR:
    fstore(l.rd,reg[rb]+o.const16);
    break;
  case OP_FSDD:
    i = reg[l.rs] + l.size4 * reg[l.ro];
    if(print_debug) {
      printf(" => %%r%d + %d * %%r%d = %d",l.rs,l.size4,l.ro,i);
    }
    fstore(l.rd,i);
    break;
  case OP_FSDA:
    fstore(l.rd,l.addr21);
    break;
  case OP_XOR:
    //旧仕様のバイナリとの互換のため残してあります
    reg[ra] = (reg[rb] ^ reg[rc]);
    setflag(ra);
    break;
  case OP_FMV:
    freg[ra] = freg[rb];
    if(print_debug)
      printf(" => fr%d = %f\n",ra,freg[ra]);
    break;
  case OP_SL:
    reg[ra] = (reg[rb] << reg[rc]);
    dprintr(ra);
    break;
  case OP_SR:
    reg[ra] = (reg[rb] >> reg[rc]);
    dprintr(ra);
    break;
  case OP_PRINT:
    print_to_file(ra);
    break;
  case OP_FABS:
    if(freg[rb] >= 0) {
      freg[ra] = freg[rb];
    } else {
      freg[ra] = -1 * freg[rb];
    }
    dprintfr(ra);
    break;
  case OP_MUL:
    reg[ra] = reg[rb] * reg[rc];
    dprintr(ra);
    break;
  case OP_DIV:
    if(reg[rc] == 0) {
      printf("division by 0\n");
      stop = 1;
    } else {
      reg[ra] = reg[rb] / reg[rc];
      dprintr(ra);
    }
    break;
  case OP_SIP:
    /*命令セットにはIP+8とあるが、
      命令実行前にすでにIPに4足してあるのでここは+4*/
    sipflag = 1;
    md.i = pc+4;
    for(i=0;i<4;i++) {
      memory[reg[REG_SP]-4+i] = md.c[3-i];//stack pointer-4番地にストア
    }
    if(print_debug)
      printf(" => STORED %d TO MEMORY[%d]\n",md.i,reg[REG_SP]-4);
    break;
  case OP_FIN:
    if(print_debug==0&&print_to_stdin==1) {
      putchar('\n');
    }
    printf(" => finished by operation FIN\n");
    
    stop = 1;
    break;
  case OP_CEQ:
    if(reg[ra] == reg[rb]) {
      flag[ZF] = 1;
      if(print_debug)
	{
	  printf(" %d == %d => set ZF\n",reg[ra],reg[rb]);
	}
	} else {
      flag[ZF] = 0;
      if(print_debug)
	printf("%d != %d => reset ZF\n",reg[ra],reg[rb]);
    }
    break;
  case OP_RC:
    md.i=0;//mdを初期化
    //sldファイルから文字列をバイナリとして1byte読み取る
    if(fp_sld == NULL) {
      if((fp_sld = fopen(name_sld,"rb")) == NULL) {
	printf("%s not found\n",name_sld);
	stop = 1;
      }
    }
    if(fp_sld != NULL) {
      if((i=fread(&(md.c[0]),sizeof(char),1,fp_sld)) <= 0) {
	reg[ra] = 255;
	if(print_debug) {
	  printf("%%r%d = 255(EOF)\n",ra);
	}
      } else {
	reg[ra] = md.i;
	if(md.c[0] == EOF) {
	  reg[ra] = 255;
	  if(print_debug) {
	    printf("%%r%d = 255(EOF)\n",ra);
	  }
	}
	
	if(print_debug) {
	  printf(" => %%r%d = ",ra);
	  putchar(md.c[0]);
	  printf(" | ");
	  print_bin_byte(md.c[0]);
	  putchar('\n');
	}
      }
    }
    break;
    
  default:
    printf("undefined operation\n");
    stop = 1;
  }

  //動的命令数が指定回数以上になったらデバッグ情報を表示し始める
  if(start_print > 0 && dyna >= start_print) {
    start_print = 0;
    print_debug = 1;
    mode_step = start_with_step;
  }

  //動的命令数が指定回数以上になったらシミュレーションを終える
  if(end_point > 0 && dyna >= end_point) {
    printf("arrived at end point\n");
    stop = 1;
  }

  return 0;
}
