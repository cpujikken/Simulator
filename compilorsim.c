#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assoc.h"

typedef union {
  int i;
  float f;
  char c[4];
} Mydata;

int reg[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
char memory[67108864];//64MB=67108864B
int pc = 1;
int lr = 0;
int link = 0;
struct assoc *a;

//for debug
int stop = 0;

const int INVALID_REGISTER = 17;
const int SYNTAX_ERROR = -1;

const int BYTE = 8;
const int HALF = 16;
const int WORD = 32;
const int WORDL = 15;
const int WORDR = 17;
//エラー処理
void pcause(int status, int line) {
  printf("error at line:%d",line);
  if(status == INVALID_REGISTER) {
    printf("invalid register");
  }
  else if(status == SYNTAX_ERROR) {
    printf("syntax error\n");
  }
  return;
}

//"r10"などの文字列からレジスタ番号を読みとる
int read_reg(char *s) {
  if(s[0] == 'r' || s[0] == 'R') {
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

//ジャンプ命令
int jump(char *s) {
  //sが数字の場合とラベルの場合に対応しなきゃ
  //sが数字の場合。
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
    md.i=0;
    md.c[3] = memory[m];
  }
  else if(size == WORD) {
    for(i=0;i<4;i++) {
      md.c[i] = memory[m+i];
    }
  }
  reg[rnum] = md.i;

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
  if(size == WORD) {
    for(i=0;i<4;i++) {
      memory[m+i] = md.c[i];
      
    }
  }

  printf(" => STORED %d TO MEMORY[%d]\n",md.i,m);
  return 0;
}

int execute(char *s) {
  int status;
  char opc[32] = "";
  char opr1[32] = "";
  char opr2[32] = "";
  char opr3[32] = "";
  sscanf(s,"%s %[^, \t] , %[^, \t] , %[^, \t] ",opc,opr1,opr2,opr3);

  int ra = read_reg(opr1);
  int rb = read_reg(opr2);
  int rc = read_reg(opr3);

  int nojump = 1;
  
  //printf("->OPECODE:%s OPERAND1:%s OPERAND2:%s OPERAND3:%s\n",opc,opr1,opr2,opr3);
  if(opc[0] == '\0' || opc[0] == '\n' || strcmp(opc,"NOP") == 0) {
  }
  else if(strcmp(opc,"add") == 0 || strcmp(opc,"ADD") == 0) {
    //ADD Rd,Rs,Rt : Rd=Rs+Rt
    reg[ra] = reg[rb] + reg[rc];
    printf(" => %d\n",reg[ra]);
  }
  else if(strcmp(opc,"addi") == 0 || strcmp(opc,"ADDI") == 0) {
    //ADDI Rd,Rs,imm : Rd=Rs+imm
    reg[ra] = reg[rb] + atoi(opr3);
    printf(" => %d\n",reg[ra]);
  }
  else if(strcmp(opc,"shiftl") == 0 || strcmp(opc,"SHIFTL") == 0) {
    reg[ra] = (reg[rb] << atoi(opr3));
  }
  else if(strcmp(opc,"shiftr") == 0 || strcmp(opc,"SHIFTR") == 0) {
    reg[ra] = (reg[rb] >> atoi(opr3));
  }
  else if(strcmp(opc,"b") * strcmp(opc,"B")==0) {
    pc += atoi(opr2);
    nojump = 0;
  }
  else if(strcmp(opc,"beq") * strcmp(opc,"BEQ") == 0) {
    if(reg[ra] == reg[rb]) {
      pc += atoi(opr2);
      nojump = 0;
    }
  }
  else if(strcmp(opc,"j") * strcmp(opc,"J") == 0) {
    jump(opr1);
    nojump = 0;
  }
  else if(strcmp(opc,"jeq") * strcmp(opc,"JEQ") == 0) {
    if(reg[ra] == reg[rb]) {
      jump(opr3);
      nojump = 0;
    }
  }
  else if(strcmp(opc,"jle") * strcmp(opc,"JLE") == 0) {
    if(reg[ra] <= reg[rb]) {
      jump(opr3);
      nojump = 0;
    }
  }
  else if(strcmp(opc,"lb") * strcmp(opc,"LB") == 0) {
    load(ra,opr2,BYTE);
  }
  else if(strcmp(opc,"lw") * strcmp(opc,"LW") == 0) {
    load(ra,opr2,WORD);
  }
  else if(strcmp(opc,"sb") * strcmp(opc,"SB") == 0) {
    store(ra,opr2,BYTE);
  }
  else if(strcmp(opc,"sw") * strcmp(opc,"SW") == 0) {
    store(ra,opr2,WORD);
  }
  else if(strcmp(opc,"jlink") * strcmp(opc,"JLINK") == 0) {
    reg[15] = pc + 1;//復帰したときにまた同じJLINK命令を踏まないよう、+1
    jump(opr1);
    nojump = 0;
    printf(" => r15 <- %d\n",reg[15]);
  }
  else if(strcmp(opc,"link") * strcmp(opc,"LINK") == 0) {
    pc = reg[15];
    nojump = 0;
  }
  else {//ラベルへの対応
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

void print_pc() {
  printf("IP = %d\n",pc);
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
  
  //execute
  while(stop == 0 && pc <= line) {
    printf("%d : %s",pc,code[pc]);
    status = execute(code[pc]);
    if(status != 0) {
      pcause(status,line);
      return -1;
    }
  }
  print_reg();
  print_pc();
  return 0;
}
