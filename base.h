#ifndef _BASE_H_
#define _BASE_H_

#define MEM_SIZE 2097152//メモリサイズ 2097152byte = 2MiB
#define NUM_OF_OP 57
#define NUM_OF_REG 32//レジスタの数は32
#define REG_LR 32//使わない
#define REG_SP 31//旧30
#define REG_HP 30
#define REG_CL 29//旧28 //register for closure
#define ZF 6 //zero flag 今の汎用レジスタフラグはZFのみなので実際にはフラグは1bit
#define FEQ 0 //eq flag
#define FGT 1
#define FLT 2
#define UNDEFINED 3
#define COMMENT_LENGTH 100 //コメントを読み込む際の、読み込むコメントの１行の長さ
#define COMMENT_CODESIZE_MAX 100000 //codesizeがこれ以上のときはコメントを読み込めない
#define MAX_FUN_DEPTH 100000 //関数呼び出しの深さがこれ以上になると、ラベルの表示に失敗 
#define PRINT_FUN_NUM 50 //関数呼び出しの入れ子構造をいくつさかのぼって表示するか

extern int reg[];
extern float freg[];
extern char memory[];
extern unsigned int pc;
extern unsigned int init_pc;
extern int lr;
extern int link;
extern int flag[];
extern int float_flag[];
extern char default_file[];
extern char *filename;
extern FILE *fp;
extern FILE *fp_out;
extern FILE *fp_sld;
extern unsigned int op;
extern unsigned int link_stack[];
extern unsigned int link_sp;

//デバッグ、統計情報用
extern unsigned int codesize;
extern int stop;//実行を中止(実行を完了したときもこれを呼ぶ)
extern int mode_jump;//「次のジャンプ命令まで一気に実行」モード
extern int mode_sipnext;
extern long used[];//各命令が使われた回数
extern unsigned long dyna;//動的実行命令数
extern int branch[];//条件分岐で分岐したか
extern int nbranch[];//否か
extern unsigned char mem_used[];//メモリのi番地を使用したか
extern int start_print;//動的命令数>=start_printになったらデバッグ情報表示開始
extern int start_with_step; 
extern int end_point;
extern FILE *fp_com;//コメント読み込み
extern char memory_com[][COMMENT_LENGTH];//コメント保存
extern int call_stack;//どれだけ深く関数呼び出ししているか
extern int *label_stack;//関数呼び出し時のラベルを記憶するスタック
extern void set_stack();
extern int sipflag;//SIPの直後かつjumpする直前のみ立てる
extern char error_mes[];
extern int reg_hp_max;
extern int reg_sp_max;
extern int hp_flag;
extern int sp_flag;
extern int init_hp;
extern int init_sp;

//byte単位読み込みとかで使う共用体
typedef union {
  unsigned int i;
  unsigned short sh[2];
  unsigned char c[4];
  int si;
  float f;
} Mydata;

//オペコード
#define OP_NOP		0
#define OP_ADD		1
#define OP_SUB		2
#define OP_ADDI		3
#define OP_HALF		4
#define OP_FOUR		5
#define OP_J		6
#define OP_JZ		7
#define OP_FJLT		8
#define OP_FADD		9
#define OP_FSUB		10
#define OP_FMUL		11
#define OP_FDIV		12
#define OP_FCMP		13
#define OP_FJEQ		14
#define OP_CMP		15
#define OP_JLINK	16
#define OP_LINK		17
#define OP_JC		19
#define OP_JLINKC	20
#define OP_MV		21
#define OP_NEG1		22
#define OP_FNEG1	23
#define OP_NEG2		24
#define OP_FNEG2	25
#define OP_INC		26
#define OP_DEC		27
#define OP_INC1		28
#define OP_DEC1		29
#define OP_MVI		30
#define OP_LDR		31
#define OP_LDD		32
#define OP_LDA		33
#define OP_SDR		34
#define OP_SDD		35
#define OP_SDA		36
#define OP_FLDR		37
#define OP_FLDD		38
#define OP_FLDA		39
#define OP_FSDR		40
#define OP_FSDD		41
#define OP_FSDA		42
#define OP_XOR		43
#define OP_FMV		44
#define OP_SL		45
#define OP_SR		46
#define OP_RF		47
#define OP_RI		48
#define OP_PRINT       	49
#define OP_FABS		50
#define OP_MUL		51
#define OP_DIV		52
#define OP_SIP	        53
#define OP_FIN		54
#define OP_CEQ          55
#define OP_RC		56

#endif
