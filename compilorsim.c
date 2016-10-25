#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assoc.h"

//ロードストアに使う共用体 byte単位読み込みとかで必要
typedef union {
  int i;
  float f;
  char c[4];
} Mydata;

int reg[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,67108860,0};
float freg[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
char memory[67108864];//64MB=67108864B
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

const int INVALID_REGISTER = 17;
const int SYNTAX_ERROR = -1;

const int BYTE = 8;
const int HALF = 16;
const int WORD = 32;
const int WORDL = 15;
const int WORDR = 17;

//"2" "r10" "fr5"などの文字列からレジスタ番号を読みとる
int read_reg(char *s) {
  while(s[0] == 'r' || s[0] == 'R' || s[0] == 'f' || s[0] == 'F') {
    s++;
  }
  int i = atoi(s);
  if (i == 0 && s[0] != '0') {
    return SYNTAX_ERROR;
  } else if (i < 0 || i > 15) {
    return INVALID_REGISTER;
  }
  return i;
}

//ベース相対 offset(Ra)の解読
int read_base_rel(char *s) {
  int i;
  char r[10];
  sscanf(s,"%d ( %[^)] )",&i,r);
  return i + reg[read_reg(r)];
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

int branch(char *s) {
  //sが数字の場合とラベルの場合に対応
  if(s[0] >= '0' && s[0] <= '9') {
    pc += atoi(s);
  } else {
    int i = find_assoc(s,a);
    if(i < 0) {
      printf("find label error : %s\n",s);
      stop = 1;
    }
    //番地を代入してるが、本来はi-pcを求めてからpc+=(i-pc)とすべき？
    if(stop == 0) pc = i;
  }
  return 0;
}


//ジャンプ命令
int jump(char *s) {
  //sが数字の場合とラベルの場合に対応(branchと同様)
  if(s[0] >= '0' && s[0] <= '9') {
    pc = atoi(s);
  } else {
    int i = find_assoc(s,a);
    if(i < 0) {
      printf("find label error : %s\n",s);
      stop = 1;
    }
    pc = i;
  }
  return 0;
}

//ロードストア 
int load(int rnum,char *s,int size) {
  int m = read_base_rel(s);
  Mydata md;
  int i;
  if(size == BYTE) {
    md.i = 0;
    md.c[3] = memory[m];
  }
  else if(size == WORD) {
    md.i = 0;
    for(i=0;i<4;i++) {
      md.c[i] = memory[m+i];
    }
  }
  else if(size == WORDL) {
    md.i = reg[rnum];
    md.c[0] = memory[m];
    md.c[1] = memory[m+1];
  }
  else if(size == WORDR) {
    md.i = reg[rnum];
    md.c[2] = memory[m+2];
    md.c[3] = memory[m+3];
  }
  reg[rnum] = md.i;
  if(print_debug)
    printf(" => LOADED %d FROM MEMORY[%d]\n",reg[rnum],m);
  return 0;
}

int store(int rnum,char *s,int size) {
  int m = read_base_rel(s);
  Mydata md;
  int i;

  md.i = reg[rnum];
  if(size == BYTE) {
    memory[m] = md.c[3];
  }
  else if(size == WORD) {
    for(i=0;i<4;i++) {
      memory[m+i] = md.c[i];      
    }
  }
  else if(size == WORDL) {
    memory[m] = md.c[0];
    memory[m+1] = md.c[1];
  }
  else if(size == WORDR) {
    memory[m+2] = md.c[2];
    memory[m+3] = md.c[3];
  }
  if(print_debug)
    printf(" => STORED %d TO MEMORY[%d]\n",md.i,m);
  return 0;
}

//コード一行を実行
int execute(char *s) {
  int status;
  char opc[32] = "";
  char opr1[32] = "";
  char opr2[32] = "";
  char opr3[32] = "";
  //コードをオペコード、オペランド１、オペランド２、オペランド３に分解
  sscanf(s,"%s %[^, \t\n] , %[^, \t\n] , %[^, \t\n] ",opc,opr1,opr2,opr3);

  int ra = read_reg(opr1);
  int rb = read_reg(opr2);
  int rc = read_reg(opr3);

  int nojump = 1;
  
  //printf("->OPECODE:%s OPERAND1:%s OPERAND2:%s OPERAND3:%s\n",opc,opr1,opr2,opr3);
  
  //オペコードによる場合分け
  if(opc[0] == '\0' || opc[0] == '\n' || strcmp(opc,"NOP") == 0) {
  }
  else if(strcmp(opc,"add") * strcmp(opc,"ADD") == 0) {
    //ADD Rd,Rs,Rt : Rd=Rs+Rt
    reg[ra] = reg[rb] + reg[rc];
    setflag(ra);
  }
  else if(strcmp(opc,"addi") * strcmp(opc,"ADDI") == 0) {
    //ADDI Rd,Rs,imm : Rd=Rs+imm
    reg[ra] = reg[rb] + atoi(opr3);
    setflag(ra);
  }
  else if(strcmp(opc,"shiftl") * strcmp(opc,"SHIFTL") == 0) {
    reg[ra] = (reg[rb] << atoi(opr3));
    setflag(ra);
  }
  else if(strcmp(opc,"shiftr") * strcmp(opc,"SHIFTR") == 0) {
    reg[ra] = (reg[rb] >> atoi(opr3));
    setflag(ra);
  }
  else if(strcmp(opc,"and") * strcmp(opc,"AND") == 0) {
    reg[ra] = reg[rb] & reg[rc];
    setflag(ra);
  }
  else if(strcmp(opc,"not") * strcmp(opc,"NOT") == 0) {
    reg[ra] = ~reg[rb];
    setflag(ra);
  }
  else if(strcmp(opc,"or") * strcmp(opc,"OR") == 0) {
    reg[ra] = reg[rb] | reg[rc];
    setflag(ra);
  }
  else if(strcmp(opc,"xor") * strcmp(opc,"XOR") == 0) {
    reg[ra] = reg[rb] ^ reg[rc];
    setflag(ra);
  }
  else if(strcmp(opc,"b") * strcmp(opc,"B")==0) {
    branch(opr1);
    nojump = 0;
  }
  else if(strcmp(opc,"beq") * strcmp(opc,"BEQ") == 0) {
    if(flag[ZF]) {
      branch(opr1);
      nojump = 0;
    }
  }
  else if(strcmp(opc,"j") * strcmp(opc,"J") == 0) {
    jump(opr1);
    nojump = 0;
  }
  else if(strcmp(opc,"jeq") * strcmp(opc,"JEQ") == 0) {
    if(flag[ZF]) {
      jump(opr1);
      nojump = 0;
    }
  }
  /*
  else if(strcmp(opc,"jle") * strcmp(opc,"JLE") == 0) {
    if(reg[ra] <= reg[rb]) {
      jump(opr3);
      nojump = 0;
    }
  }
  */
  else if(strcmp(opc,"lb") * strcmp(opc,"LB") == 0) {
    load(ra,opr2,BYTE);
  }
  else if(strcmp(opc,"lw") * strcmp(opc,"LW") == 0) {
    load(ra,opr2,WORD);
  }
  else if(strcmp(opc,"lwl") * strcmp(opc,"LWL") == 0) {
    load(ra,opr2,WORDL);
  }
  else if(strcmp(opc,"lwr") * strcmp(opc,"LWR") == 0) {
    load(ra,opr2,WORDR);
  }
  else if(strcmp(opc,"sb") * strcmp(opc,"SB") == 0) {
    store(ra,opr2,BYTE);
  }
  else if(strcmp(opc,"sw") * strcmp(opc,"SW") == 0) {
    store(ra,opr2,WORD);
  }
  else if(strcmp(opc,"swl") * strcmp(opc,"SWL") == 0) {
    store(ra,opr2,WORDL);
  }
  else if(strcmp(opc,"swr") * strcmp(opc,"SWR") == 0) {
    store(ra,opr2,WORDR);
  }
  else if(strcmp(opc,"fadd") * strcmp(opc,"FADD") == 0) {
    freg[ra] = freg[rb] + freg[rc];
    pfloat(ra);
  }
  else if(strcmp(opc,"fsub") * strcmp(opc,"FSUB") == 0) {
    freg[ra] = freg[rb] - freg[rc];
    pfloat(ra);
  }
  else if(strcmp(opc,"fmul") * strcmp(opc,"FMUL") == 0) {
    freg[ra] = freg[rb] * freg[rc];
    pfloat(ra);
  }
  else if(strcmp(opc,"fdiv") * strcmp(opc,"FDIV") == 0) {
    freg[ra] = freg[rb] / freg[rc];
    pfloat(ra);
  }
  else if(strcmp(opc,"fzero") * strcmp(opc,"FZERO") == 0) {
    freg[ra] = 0;
    pfloat(ra);
  }
  else if(strcmp(opc,"fabs") * strcmp(opc,"fabs") == 0) {
    if(freg[rb] >= 0) {
      freg[ra] = freg[rb];
    } else {
      freg[ra] = -1 * freg[rb];
    }
    pfloat(ra);
  }
  
  else if(strcmp(opc,"jlink") * strcmp(opc,"JLINK") == 0) {
    reg[15] = pc + 1;//復帰したときにまた同じJLINK命令を踏まないよう、+1
    jump(opr1);
    nojump = 0;
    if(print_debug)
      printf(" => LR(=r15) = %d\n",reg[15]);
  }
  else if(strcmp(opc,"link") * strcmp(opc,"LINK") == 0) {
    pc = reg[15];
    nojump = 0;
  }
  else if(strcmp(opc,"outA") * strcmp(opc,"OUT") == 0) {
    printf(" => finish\n");
    stop = 1;
  }
  //ラベルへの対応
  else {
    char c = '\0';
    char label[32] = "";
    sscanf(opc,"%[^:]%s",label,&c);
    if(c == ':') {
      nojump=1;
    } else {
      printf("undefined operation\n");
      stop = 1;
      nojump = 0;
    }
  }
  if(nojump) pc++;
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
  //default input filename is "assembly"
  char default_file[10] = "assembly";
  char *filename = default_file;
  if(argc > 1) {
    filename = argv[1];
  }

  a = create_assoc();

  if((fp = fopen(filename,"r")) == NULL) {
    printf("file open error\n");
    return -1;
  }
  //save codes and register labels
  while(fgets(s,256,fp) != NULL) {
    line++;
    i=0;
    while(s[i] != '\0') { //コメントの処理
      if(s[i] == '#') {
	s[i] = '\n';
	s[i+1] = '\0';
      }
      i++;
    }
    char c = '\0';
    char label[32] = "";
    sscanf(s,"%[^:]%s",label,&c);
    if(c==':') {
      a = insert_assoc(label,line,a);//ラベルを連想配列に登録
      printf("label:%s (line=%d)\n",label,line);
    }

    strcpy(code[line],s);
  }
  fclose(fp);
  
  //1行ずつ実行
  while(stop == 0 && pc <= line) {
    printf("%d : %s",pc,code[pc]);
    status = execute(code[pc]);
  }

  //最後にレジスタ等を表示
  print_reg();
  print_freg();
  print_pc();
  return 0;
}
