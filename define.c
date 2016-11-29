#include "define.h"
/*
 * このファイルの変数の値を変えることで設定を変更できます。
 * 1で有効,0で無効です。
 */

//実行中の命令名など、デバッグ用の情報を表示する
//0にすると、PRINT命令は標準出力に(charで)出力するようになる
int print_debug = 0;

//PRINT命令の出力をintで表示(print_debug=0の時のみ有効)
int print_raw = 0;

//実行後、各命令実行回数などの統計情報を表示する
int print_stat = 0;
//ステップ実行する
int mode_step = 0;
//PRINTでの出力のバイトオーダをリトルエンディアンにする
//(1推奨,print_debug=0なら無関係)
int print_little = 1;
