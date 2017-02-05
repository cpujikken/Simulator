#include <stdio.h>
#include "base.h"

int reg[32];
float freg[32];
char memory[MEM_SIZE];//64MB=67108864B
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
int mode_sipnext = 0;//次の番地の命令まで続けて実行するモード。SIP時に使う
int used[NUM_OF_OP];//各命令実行回数
int dyna = 0;//動的実行命令数
int branch[NUM_OF_OP];
int nbranch[NUM_OF_OP];
unsigned char mem_used[MEM_SIZE/4];
int start_print = -1;
int start_with_step = 0;//ステップ実行でstart_pointを指定した場合、start_pointになるまでこれを1にし、start_pointになったらそこからstep実行をする
int end_point = -1;

char default_file[100] = "example";// 2017/1/19 "binary"から変更
char *filename = default_file;
FILE *fp;
FILE *fp_out;
FILE *fp_sld = NULL;
