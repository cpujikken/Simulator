#include <stdio.h>
#include "base.h"

int reg[32];
float freg[32];
char memory[MEM_SIZE];
unsigned int pc = 0;
unsigned int init_pc = 0;
int lr = 0;
int link = 0;
int flag[16];//フラッグや浮動小数点フラッグが16bitというのは再現してない
int float_flag[8]={3,3,3,3,3,3,3,3};//本来は2bit刻み 以下の状態のいずれか
unsigned int link_stack[100000];
unsigned int link_sp = 0;


//for debug
unsigned int codesize;
int stop = 0;//コードの実行を中止/終了
int mode_jump = 0;//次の命令までジャンプするモード
int mode_sipnext = -1;//次の番地の命令まで続けて実行するモード。SIP時に使う
long used[NUM_OF_OP];//各命令実行回数
unsigned long dyna = 0;//動的実行命令数
int branch[NUM_OF_OP];
int nbranch[NUM_OF_OP];
unsigned char mem_used[MEM_SIZE/4];
int start_print = -1;
int start_with_step = 0;//ステップ実行でstart_pointを指定した場合、start_pointになるまでこれを1にし、start_pointになったらそこからstep実行をする
int end_point = -1;
int call_stack = 0;
int *label_stack = NULL;//関数呼び出し時のラベルを記憶するスタック
int sipflag = 0;
char error_mes[100] = "normal end\n";
int reg_hp_max = 0;
int reg_sp_max = 0;
int hp_flag = 1;
int sp_flag = 1;
int init_hp=0;
int init_sp=0;

char default_file[100] = "example";// 2017/1/19 "binary"から変更
char *filename = default_file;
FILE *fp;
FILE *fp_out;
FILE *fp_sld = NULL;
FILE *fp_com = NULL;
char memory_com[COMMENT_CODESIZE_MAX][COMMENT_LENGTH];

