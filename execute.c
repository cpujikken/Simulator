#include <stdio.h>
#include <string.h>
#include "define.h"
#include "base.h"
#include "print.h"
#include "execute.h"

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

void print_op(Operation o,Ldst l) {
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
    printf("%%fr%d, %%fr%d, $%d, %%r%d",l.rd,l.rs,l.size4,l.ro);
    break;
  case OP_FLDA:
  case OP_FSDA:
    printf("%%fr%d, $%d",l.rd,l.addr21);
    break;
  }
  
  putchar('\n');
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
  md.f = reg[rnum];
  for(i=0;i<4;i++) {
    memory[addr+i] = md.c[3-i];//リトルエンディアン
  }
  if(print_debug)
    printf(" => STORED %d TO MEMORY[%d]\n",md.i,addr);
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
  pc = j;
  if(print_debug)
    printf(" => JUMPED TO %d\n",j);
}
//ジャンプ非成立時の表示
void nojump() {
  if(print_debug)
    printf(" => NO JUMP\n");
}

//コード一行を実行
int execute(unsigned int op) {
  int status;
  Operation o = parse(op);
  unsigned int ra = o.opr1;
  unsigned int rb = o.opr2;
  unsigned int rc = o.opr3;
  Ldst l = parse_ldst(op);
  
  Mydata md;
  int i;
  double doub;
  //命令使用回数をカウント  
  used[o.opc]++;
  dyna++;

  //命令を表示
  print_op(o,l);

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
    jump(o.off_addr26);
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
    break;
  case OP_FSUB:
    freg[ra] = freg[rb] - freg[rc];
    dprintfr(ra);
    break;
  case OP_FMUL:
    freg[ra] = freg[rb] * freg[rc];
    dprintfr(ra);
    break;
  case OP_FDIV:
    if(freg[rc] == 0) {
      printf("devision by %f\n",freg[rc]);
      stop = 1;
      } else {
      freg[ra] = freg[rb] / freg[rc];
      dprintfr(ra);
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
    i = read_mem32(reg[REG_SP]-4);//stack pointer-4番地をロード
    if(print_debug)
      printf(" READ %d FROM MEMORY[%d]\n ",i,reg[REG_SP]-4);
    reg[REG_SP] = reg[REG_SP] - 4;//pop
    jump(i);
    break;
  case OP_JC:
    jump(read_mem32(reg[REG_CL]));
    break;
  case OP_MV:
    reg[ra]=reg[rb];
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
    load(l.rd,reg[l.rs] + l.ro * l.size4);
    break;
  case OP_LDA:
    load(l.rd,l.addr21);
    break;
  case OP_SDR:
    store(ra,reg[rb]+o.const16);
    break;
  case OP_SDD:
    store(l.rd,reg[l.rs] + l.ro * l.size4);
    break;
  case OP_SDA:
    store(l.rd,l.addr21);
    break;
  case OP_FLDR:
    fload(ra,reg[rb]+o.const16);
    break;
  case OP_FLDD:
    fload(l.rd,reg[l.rs] + l.ro * l.size4);
    break;
  case OP_FLDA:
    fload(l.rd,l.addr21);
    break;
  case OP_FSDR:
    fstore(ra,reg[rb]+o.const16);
    break;
  case OP_FSDD:
    fstore(l.rd,reg[l.rs] + l.ro * l.size4);
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
  case OP_RF:
    if(1) {
      printf("input to %%fr%d >",ra);
    }
    scanf("%f",&freg[ra]);
    dprintfr(ra);
    break;
  case OP_RI:
    if(1) {
      printf("input to %%r%d >",ra);
    }
    scanf("%lf",&doub);
    reg[ra] = doub;
    if(doub - reg[ra] != 0) {
      printf("error in RI : input was floating-point number(%f)\n",doub);
      stop = 1;
    } else {
      setflag(ra);
      dprintr(ra);
    }
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
    md.i = pc+4;
    for(i=0;i<4;i++) {
      memory[reg[REG_SP]-4+i] = md.c[3-i];//stack pointer-4番地にストア
    }
    if(print_debug)
      printf(" => STORED %d TO MEMORY[%d]\n",md.i,reg[REG_SP]);
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
    
    
  default:
    printf("undefined operation\n");
    stop = 1;
  }

  //動的命令数が指定回数以上になったらデバッグ情報を表示し始める
  if(start_print > 0 && print_debug == 0 && dyna >= start_print) {
    print_debug = 1;
  }
  return 0;
}
