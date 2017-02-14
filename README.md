# Simulator  
機械語のシミュレータ  

ビルドの仕方:  
$ make bsim

使い方:  
$ ./bsim (source code file) (input file) (option:DIN where simulator starts printing)  
・(source code file)を書かない場合、exampleというファイルを読み込みます。  
・PRINTによる出力は(source code file)_outに出力されます。  
・各種設定を変更するには、define.cを開いて該当する変数の値を書き換え、makeしなおしてください。  
・入力sldファイル名(input file)で指定しますが、はdefine.cで設定することもできます。デフォルトではcontest.sldを読みます。
・./bsim example contest.sld 120 のように、入力ファイル名のあとに数字を入力すると、動的命令数がその数字以上になったらデバッグ情報を表示します。実行したファイルが予期せぬ形で終了した時、終了した命令の近くのみのデバッグ情報が知りたい場合に使えます。  

・ステップ実行も可能。define.cの変数mode_step=1にするか、コマンドラインで -s をつけてください。   
$ ./bsim -s (filename)  
命令を一つ実行する度に、入力を待つ。  
n : 次の命令を実行  
s : 前の命令がSIP命令だった場合、IPが次の番地になるまで一気に実行 それ以外のときはnと同じ動作  
j : 次のジャンプやリンクが起こるまで一気に実行  
pr : 現在の汎用レジスタのうち、非ゼロのものを全て表示  
pr_bin (num) : 汎用レジスタ(num)番をバイナリで表示  
pfr : 現在の浮動小数点レジスタのうち、非ゼロのものを全て表示。  
pfr_bin (num) : 浮動小数点レジスタ(num)番をバイナリで表示  
pip : 現在のIP(instruction pointer)を表示。  
pm_int (num) : メモリの(num)番地をint型として表示。  
pm_float (num) : メモリの(num)番地をfloat型として表示。  
pm_bin (num) : メモリの(num)番地をバイナリで表示。  
q : 終了  

また、副産物として簡易バイナリ作成機/表示機を作成したので、ついでに上げておきます。  
ビルドの仕方:(作成機/表示機)  
$make wrbin  
$make rdbin  
使い方:(作成機)  
$./wrbin (filename)  
バイナリを8bitずつ入力してください(2進数以外には対応していません)。入力が7文字以下の場合は左詰めになります(例えば"101"と入力すると、10100000になる)。一度に9文字以上書いても9文字目以降は無視されます。  
終了時は1文字目に0や1以外を書けばOK.  
使い方:(表示機)  
$./rdbin (filename)  
いずれも、ファイル名を書かない場合はbinaryというファイルを開きます。  
使用例:  
$ ./wrbin testbin  
10101010  
11110000  
11001100  
00110011  
11111110  
q  

$ ./rdbin testbin  
10101010 11110000 11001100 00110011  
11111110  
