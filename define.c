#include "define.h"
/*
 * このファイルの変数の値を変えることで設定を変更できます。
 * 1で有効,0で無効です。
 */

//実行中の命令名など、デバッグ用の情報を表示する
int print_debug = 1;

//命令をバイナリでも表示(主にアセンブラデバッグ用)
int print_op_bin = 0;

//PRINT命令の結果を(_outファイルでなく)標準出力に出力(print_debug=0の時のみ有効)
int print_to_stdin = 0;

//PRINT命令の出力をintで表示(print_debug=1かつprint_stdin=1の時のみ有効)
int print_raw = 1;

//実行後、各命令実行回数などの統計情報を表示する
int print_stat = 0;
//ステップ実行する
int mode_step = 1;
//レジスタやメモリの初期値をランダムにします。
//初期値が0でなくても正しく実行されるかのテストに使用。
/*
  この機能を使うとヒープ容量やスタック容量の表示が正しくなくなりますごめんなさい
  シミュレーション自体に支障はありません
*/
int init_randomize = 0;

//入力sldファイルの名称
char name_sld[100] = "contest.sld";
//アセンブリコードのコメントを表示するか
int print_comment = 1;
//アセンブリのコメントを書いたファイルの名称
char name_com[100] = "../compiler/comment.s";
//関数呼び出しがこの回数以上になったら実行を中止する -1で無効
int sip_count = -1;
//SIP->J[JC]時に、ジャンプ先をプリントする
int print_function_call = 0;
//dinがこれの倍数になるたびに標準出力、0以下だと表示しない
int din_count = -1;

/*
  library関数を、バイナリのプログラムを実行するかわりにシミュレータで計算する
  引数は%r1か%fr1,返り値は%r0か%fr0とする
  対応しているlibrary関数:
  float_of_int
  int_of_float
  print_int
  print_float
  read_int
  read_float
  print_newline
  print_space
  floor
  sqrt
  atan
  cos
  sin
  fiszero
  fispos
  fisneg
  fsqr
  fhalf
  fless
  */
int use_system_func_mode = 0;
