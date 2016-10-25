#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assoc.h"
#include "opcode.h"

typedef union {
  unsigned int opc;
  unsigned int opr1;
  unsigned int opr2;
  unsigned int opr3;
  short int const16; //off16も同じ
  unsigned int bits5;
  unsigned int off_addr26;//+-はどうやって...？
  
} Operation;

//ロードストアに使う共用体 byte単位読み込みとかで必要
typedef union {
  unsigned int i;
  unsigned char c[4];
  float f;
} Mydata;

int reg[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,67108860,0};
float freg[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned char memory[67108864];//64MB=67108864B
int pc = 1;
int lr = 0;
int link = 0;
struct assoc *a;//ラベル収納用の連想配列
const int ZF = 6;
int flag[16];//フラッグや浮動小数点フラッグが16bitというのは再現してない
int float_flag[8]={3,3,3,3,3,3,3,3};//本来は2bit刻み 以下の状態のいずれか
const int FEQ = 0;
const int FGT = 1;
const int FLT = 2;
const int UNDEFINED = 3;

//for debug
int print_debug = 1;//各命令の実行結果を一々表示したくないときはここを0にする
//int step = 0;//step実行したいときはここを1にする(未実装)
int stop = 0;//コードの実行を中止/完了する

const int BYTE = 8;
const int HALF = 16;
const int WORD = 32;
const int WORDL = 15;
const int WORDR = 17;

//メモリから32bitをunsigned int型で読み込む
//学科pcはリトルエンディアンなので、後ろから読み込む
unsigned int read_mem32(int mem_addr) {
  Mydata myd;
  int i;
  for (i = 0;i < 4;i++) {
    myd.c[i] = memory[mem_addr + 4 - i];
  }
}

//命令を16進数表示
void print_hex(unsigned int op) {
  Mydata myd;
  myd.i = op;
  printf("%x%x %x%x\n",myd.c[0],myd.c[1],myd.c[2],myd.c[3]);
  return;
}

//命令をオペコードとに分解
Operation parse(unsigned int op) {
  Operation o;
  //オペコード=上位6bit
  o.opc = op >> 26;
  o.opr1 = (op >> 22) & 0b1111;
  o.opr2 = (op >> 18) & 0b1111;
  o.opr3 = (op >> 14) & 0b1111;
  o.const16 = (short)((op >> 2) & 0b1111111111111111);//?
  o.bits5 = (op >> 13) & 0b11111;
  o.off_addr26 = op & 0x3ffffff;
  return o;
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

//浮動小数点演算の結果を表示
int pfloat(int rnum) {
  if (print_debug)
    printf(" => fr%d = %f\n",rnum,freg[rnum]);
  return 0;
}

//ロード
int load(unsigned int rnum,unsigned int rs,int offset,int size) {
  int m =  reg[rs] + offset;
  Mydata md;
  int i;
  if(size == BYTE) {
    md.i = 0;
    md.c[0] = memory[m];
  }
  else if(size == WORD) {
    md.i = 0;
    for(i=0;i<4;i++) {
      md.c[4-i] = memory[m+i];
    }
  }
  else if(size == WORDL) {
    md.i = reg[rnum];
    md.c[2] = memory[m];
    md.c[3] = memory[m+1];
  }
  else if(size == WORDR) {
    md.i = reg[rnum];
    md.c[0] = memory[m+2];
    md.c[1] = memory[m+3];
  }
  reg[rnum] = md.i;
  if(print_debug)
    printf(" => LOADED %d FROM MEMORY[%d]\n",reg[rnum],m);
  return 0;
}

int fload(unsigned int rnum,unsigned int rs,int offset) {
  int m =  reg[rs] + offset;
  Mydata md;
  int i;
  md.f = 0;
  for(i=0;i<4;i++) {
    md.c[4-i] = memory[m+i];
  }
  freg[rnum] = md.f;
  if(print_debug)
    printf(" => LOADED %f FROM MEMORY[%d]\n",freg[rnum],m);
  return 0;
}

//ストア
int store(unsigned int rnum,unsigned int rs,int offset, int size) {
  int m = reg[rs] + offset;
  Mydata md;
  int i;
  
  md.i = reg[rnum];
  if(size == BYTE) {
    memory[m] = md.c[0];//リトルエンディアン
  }
  else if(size == WORD) {
    for(i=0;i<4;i++) {
      memory[m+i] = md.c[4-i];//リトルエンディアン
    }
  }
  else if(size == WORDL) {
    memory[m] = md.c[2];
    memory[m+1] = md.c[3];
  }
  else if(size == WORDR) {
    memory[m+2] = md.c[0];
    memory[m+3] = md.c[1];
  }
  if(print_debug)
    printf(" => STORED %d TO MEMORY[%d]\n",md.i,m);
  return 0;
}

int fstore(unsigned int rnum,unsigned int rs,int offset) {
  int m = reg[rs] + offset;
  Mydata md;
  int i;
  
  md.f = reg[rnum];
  for(i=0;i<4;i++) {
    memory[m+i] = md.c[4-i];//リトルエンディアン
  }
  if(print_debug)
    printf(" => STORED %d TO MEMORY[%d]\n",md.i,m);
  return 0;
}


//コード一行を実行
int execute(unsigned int op) {
  int status;
  Operation o = parse(op);
  unsigned int ra = o.opr1;
  unsigned int rb = o.opr2;
  unsigned int rc = o.opr3;
  
  
  //オペコードによる場合分け
  switch (o.opc) {
  case OP_NOP:
    break;
  case OP_ADD:
    //ADD Rd,Rs,Rt : Rd=Rs+Rt
    reg[ra] = reg[rb] + reg[rc];
    setflag(ra);
    break;
  case OP_ADDI:
    //ADDI Rd,Rs,imm : Rd=Rs+imm
    reg[ra] = reg[rb] + o.const16;
    setflag(ra);
    break;
  case OP_SHIFTL:
    reg[ra] = (reg[rb] << o.bits5);
    setflag(ra);
    break;
  case OP_SHIFTR:
    reg[ra] = (reg[rb] >> o.bits5);
    setflag(ra);
    break;
  case OP_AND:
    reg[ra] = reg[rb] & reg[rc];
    setflag(ra);
    break;
  case OP_NOT:
    reg[ra] = ~reg[rb];
    setflag(ra);
    break;
  case OP_OR:
    reg[ra] = reg[rb] | reg[rc];
    setflag(ra);
    break;
  case OP_XOR:
    reg[ra] = reg[rb] ^ reg[rc];
    setflag(ra);
    break;
  case OP_B:
    pc += o.off_addr26;
    break;
  case OP_BEQ:
    if(flag[ZF])
      pc += o.off_addr26;
    break;
  case OP_J:
    pc = o.off_addr26;
    break;
  case OP_JEQ:
    if(flag[ZF]) 
      pc = o.off_addr26;
    break;
  case OP_LB:
    load(ra,rb,o.const16,BYTE);
      break;
  case OP_LW:
    load(ra,rb,o.const16,WORD);
    break;
  case OP_LWL:
    load(ra,rb,o.const16,WORDL);
    break;
  case OP_LWR:
    load(ra,rb,o.const16,WORDR);
    break;
  case OP_SB:
    store(ra,rb,o.const16,BYTE);
    break;
  case OP_SW:
    store(ra,rb,o.const16,WORD);
    break;
  case OP_SWL:
    store(ra,rb,o.const16,WORDL);
    break;
  case OP_SWR:
    store(ra,rb,o.const16,WORDR);
    break;
  case OP_FADD:
    freg[ra] = freg[rb] + freg[rc];
    pfloat(ra);
    break;
  case OP_FSUB:
    freg[ra] = freg[rb] - freg[rc];
    pfloat(ra);
    break;
  case OP_FMUL:
    freg[ra] = freg[rb] * freg[rc];
    pfloat(ra);
    break;
  case OP_FDIV:
    freg[ra] = freg[rb] / freg[rc];
    pfloat(ra);
    break;
  case OP_FZERO:
    freg[ra] = 0;
    pfloat(ra);
    break;
  case OP_FABS:
    if(freg[rb] >= 0) {
      freg[ra] = freg[rb];
    } else {
      freg[ra] = -1 * freg[rb];
    }
    pfloat(ra);
    break;
  case OP_JLINK:
    reg[15] = pc;
    pc = o.off_addr26;
    if(print_debug)
      printf(" => LR(=r15) = %d\n",reg[15]);
    break;
  case OP_LINK:
    pc = reg[15];
    break;
  case OP_OUT:
    printf(" => finish\n");
    stop = 1;
    break;
  case OP_FLW:
    fload(ra,rb,o.const16);
    break;
  case OP_FSW:
    fstore(ra,rb,o.const16);
    break;

  default:
    printf("undefined operation\n");
    stop = 1;
  }
  return 0;
}

//表示用の関数群
//0でない値が入っているレジスタを表示
void print_reg() {
  int i;
  for (i = 0;i < 16;i++) {
    if(reg[i] != 0)
      printf("r%d = %d, ",i,reg[i]);
  }
  printf("\n");
  return;
}

void print_freg() {
  int i;
  for (i = 0;i < 16;i++) {
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



int main(int argc,char *argv[])
{
  FILE *fp;
  char s[256];
  char code[10000][100]; //10000行まで保存??
  int i;
  int line = 0;
  int status = 0;//error code
  //default input filename is "binary"
  char default_file[10] = "binary";
  char *filename = default_file;
  unsigned int op;

  if(argc > 1) {
    filename = argv[1];
  }
  
  a = create_assoc();
  
  //バイナリモードでファイルオープン
  if((fp = fopen(filename,"rb")) == NULL) {
    printf("file open error\n");
    return -1;
  }
  //load codes バイナリだとラベル等も考えずひたすら読み込むだけなんで楽だなあ
  for(i=0;fread(memory+i,sizeof(unsigned char),1,fp) > 0;i++) {
  }
  fclose(fp);
  
  //まずプログラム開始番地を読み、PCに代入
  pc = read_mem32(0);
  print_hex(pc);
  pc = 4;
  if(print_debug) {
    print_hex(pc);
    printf("initial pc = %d\n",pc);
  }
  
  //1行(32bit)ずつ実行
  while(stop == 1 ) {
    op = read_mem32(pc);
    if(print_debug)
      print_hex(op);
    pc += 4;//命令を読んだ瞬間に+される
    execute(op);
  }
  
  //最後にレジスタ等を表示
  print_reg();
  print_freg();
  print_pc();
  return 0;
}
