#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assoc.h"
#include "opcode.h"
#define MEM_SIZE 67108864 //64MB=67108864B
#define INIT_SP  67108860

typedef struct {
  unsigned int opc;
  unsigned int opr1;
  unsigned int opr2;
  unsigned int opr3;
  short int const16; //off16も同じ
  unsigned int bits5;
  unsigned int off_addr26;//+-はどうやって...？
  int off21; //SIGNED!!!
} Operation;

//ロードストアに使う共用体 byte単位読み込みとかで必要
typedef union {
  unsigned int i;
  unsigned short sh[2];
  unsigned char c[4];
  int si;
  float f;
} Mydata;

int reg[32];
float freg[32];
const int REG_LR = 31;//link register
const int REG_SP = 30;//stack pointer
unsigned char memory[MEM_SIZE];
int init_pc = 0;
int pc = 0;
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
int mode_step = 0;
int print_debug = 1;//各命令の実行結果を一々表示したくないときはここを0にする
//int step = 0;//step実行したいときはここを1にする(未実装)
int stop = 0;//コードの実行を中止/完了する


  
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

//メモリから32bitをunsigned int型で読み込む
//学科pcはリトルエンディアンなので、後ろから読み込む必要があります
unsigned int read_mem32(int mem_addr) {
  Mydata myd;
  int i;
  for (i = 0;i < 4;i++) {
    myd.c[i] = memory[mem_addr + 3 - i];
  }
  return myd.i;
}

//命令をオペコードとに分解
Operation parse(unsigned int op) {
  Operation o;
  //オペコード=上位6bit register:5bit for each
  o.opc = op >> 26;
  o.opr1 = (op >> 21) & 0b1111;
  o.opr2 = (op >> 16) & 0b1111;
  o.opr3 = (op >> 11) & 0b1111;
  o.const16 = (short)((op >> 2) & 0b1111111111111111);//?
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

/*
 * ロード
 * メモリにはビッグエンディアンで書かれてるが、レジスタに載せる際は
 * リトルエンディアンにしないと、シミュレータ的にはすごく不便
 */
int load(unsigned int rnum,unsigned int rs,int offset) {
  int addr =  reg[rs] + offset;//番地
  Mydata md;
  unsigned int i;
  md.i = 0;
  for(i=0;i<4;i++) {
    md.c[3-i] = memory[addr+i]; //ビッグエンディアン->リトルエンディアン
  }
  reg[rnum] = md.i;
  if(print_debug)
    printf(" => LOADED %d FROM MEMORY[%d]\n",reg[rnum],addr);
  return 0;
}

int fload(unsigned int rnum,unsigned int rs,int offset) {
  int addr =  reg[rs] + offset;
  Mydata md;
  int i;
  md.f = 0;
  for(i=0;i<4;i++) {
    md.c[3-i] = memory[addr+i];
  }
  freg[rnum] = md.f;
  if(print_debug)
    printf(" => LOADED %f FROM MEMORY[%d]\n",freg[rnum],addr);
  return 0;
}

//ストア
int store(unsigned int rnum,unsigned int rs,int offset) {
  int addr = reg[rs] + offset;
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

int fstore(unsigned int rnum,unsigned int rs,int offset) {
  int addr = reg[rs] + offset;
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

//push: store reg[x] in memory[previous SP] and SP-=4
void push(unsigned int rnum) {
  store(rnum,REG_SP,0);
  reg[REG_SP] -= 4;
  if(print_debug)
    printf(" => SP -= 4\n");
  return;
}

//push: load reg[x] from memory[previous SP] and SP+=4
void pop(unsigned int rnum) {
  load(rnum,REG_SP,0);
  reg[REG_SP] += 4;
  if(print_debug)
    printf(" => SP += 4\n");
  return;
}

//コード一行を実行
int execute(unsigned int op) {
  int status;
  Operation o = parse(op);
  unsigned int ra = o.opr1;
  unsigned int rb = o.opr2;
  unsigned int rc = o.opr3;
  //命令名を表示する？
  //printf("operation code:%d\n",o.opc);
  //オペコードによる場合分け
  switch (o.opc) {
  case OP_NOP:
    break;
  case OP_ADD:
    reg[ra] = reg[rb] + reg[rc];
    setflag(ra);
    break;
  case OP_SUB:
    reg[ra] = reg[rb] - reg[rc];
    setflag(ra);
    break;
  case OP_ADDI:
    reg[ra] = reg[rb] + o.const16;
    setflag(ra);
    break;
    /*
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
    */
  case OP_HALF:
    reg[ra] = reg[rb] >> 1;
    setflag(ra);
    break;
  case OP_FOUR:
    reg[ra] = reg[rb] << 2;
    setflag(ra);
    break;
  case OP_J:
    pc = o.off_addr26;
    break;
  case OP_JZ://old JEQ
    if(flag[ZF]) 
      pc = o.off_addr26;
    break;
  case OP_FJLT:
    if(float_flag[0] == FLT)
      pc = o.off_addr26;
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
  case OP_FCMP:
    if(freg[ra] > freg[rb]) {
      float_flag[0] = FGT;
    } else if(freg[ra] < freg[rb]) {
      float_flag[0] = FLT;
    } else {
      float_flag[0] = FEQ;
    }
  case OP_FJEQ:
    if(float_flag[0] == FEQ)
      pc = o.off_addr26;
    break;
  case OP_CMP:
    if(reg[ra] < reg[rb]) {
      flag[ZF] = 0;
    }
    /*
  case OP_LD:
    load(ra,rb,o.const16);
    break;
  case OP_SD:
    store(ra,rb,o.const16);
    break;
    */
    /*
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
    */
  case OP_JLINK:
    reg[REG_LR] = pc;
    pc = o.off_addr26;
    if(print_debug)
      printf(" => LR(=r%d) = %d\n",REG_LR,reg[REG_LR]);
    break;
  case OP_LINK:
    pc = reg[REG_LR];
    break;
  case OP_PUSH:
    push(ra);
    break;
  case OP_POP:
    pop(ra);
    break;
  case OP_OUT:
    printf(" => OUT\n");
    stop = 1;
    break;
  case OP_LDATA:
    /*
  case OP_FLW:
    fload(ra,rb,o.const16);
    break;
  case OP_FSW:
    fstore(ra,rb,o.const16);
    break;
    */
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



int main(int argc,char *argv[])
{
  FILE *fp;
  char s[20];
  int i;
  int line = 0;
  int codesize;
  int status = 0;//error code
  char default_file[10] = "binary";//default input filename is "binary"
  char *filename = default_file;
  unsigned int op;
  int num;//ステップ実行用
  int read;//ステップ実行用

  if(argc > 1) {
    if(argv[1][0] == '-' && argv[1][1] == 's') {
      mode_step = 1;
      if(argc > 2) {
	filename = argv[2];
      }
    } else {
      filename = argv[1];
    }
  }

  reg[REG_SP] = INIT_SP;//initial SP
  
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
  codesize = i;//何番地めまで読み込んだのか

  //まずプログラム開始番地を読み、PCに代入
  init_pc = read_mem32(0);

  pc = init_pc;
  if(print_debug) {
    printf("read ");
    print_mem_hex(0);
    printf("initial IP = %d\n",pc);
  }
  //1行(32bit)ずつ実行
  while(stop == 0) {
    op = read_mem32(pc);
    if(print_debug)
      printf("IP = %d | ",pc);
      print_mem_hex(pc);
    //ステップ実行の場合,"n","p"などを読む
    if(mode_step) {
      read = 1;
      while(read) {
	scanf("%s",s);
	if(strcmp(s,"n") == 0) {
	  read = 0;
	} else if(strcmp(s,"q") == 0) {
	  return 0;
	} else if(strcmp(s,"pr") == 0) {
	  print_reg();
	} else if(strcmp(s,"pfr") == 0) {
	  print_freg();
	} else if(strcmp(s,"pip") == 0) {
	  print_pc();
	} else if(strcmp(s,"pm_int") == 0) {
	  Mydata my;
	  scanf("%d",&num);
	  my.i = read_mem32(num);
	  printf("MEMORY[%d] = %d\n",num,my.si);
	} else if(strcmp(s,"pm_float") == 0) {
	  Mydata my;
	  scanf("%d",&num);
	  my.i = read_mem32(num);
	  printf("MEMORY[%d] = %f\n",num,my.f);
	} else {
	  printf("undefined command\n");
	}
      }
    }
    //pcがソースコードの長さ以上になったら
    if(pc >= codesize) {
      printf("IP exceeded source code size. Simulate finished.\n");
      stop = 1;
    }
    pc += 4;//命令を読んだ瞬間(just before executing it)に+される
    execute(op);
    
  }
  
  //最後にレジスタ等を表示
  print_reg();
  print_freg();
  print_pc();

  return 0;
}
