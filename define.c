#include "define.h"
/*
 * このファイルの変数の値を変えることで設定を変更できます。
 * 1で有効,0で無効です。
 */

//実行中の命令名など、デバッグ用の情報を表示する
int print_debug = 0;

//PRINT命令の結果を(_outファイルでなく)標準出力に出力(print_debug=0の時のみ有効)
int print_to_stdin = 0;
//PRINT命令の出力をintで表示(print_debug=1かつprint_stdin=1の時のみ有効)
int print_raw = 1;

//実行後、各命令実行回数などの統計情報を表示する
int print_stat = 1;
//ステップ実行する
int mode_step = 0;
//PRINTでの出力のバイトオーダをリトルエンディアンにする
int print_little = 1;
//レジスタやメモリの初期値をランダムにします。
//初期値が0でなくても正しく実行されるかのテストに使用。
int init_randomize = 0;

//入力sldファイルの名称
char name_sld[20] = "contest.sld";
