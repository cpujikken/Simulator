#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "define.h"
#include "base.h"
#include "print.h"
#include "label.h"
#include "execute.h"

double gettime() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return ((double)(tv.tv_sec) + (double)(tv.tv_usec) * 0.001 * 0.001);
}


int main(int argc,char *argv[])
{
  char s[20];
  unsigned int i;
  int line = 0;
  unsigned int op;
  int num;//ステップ実行用
  int read;//ステップ実行用
  double t1,t2;//実行時間測定用

  t1 = gettime();

  if(argc > 1) {
    //ステップ実行モード
    if(argv[1][0] == '-' && argv[1][1] == 's') {
      mode_step = 1;
      print_debug = 1;
      if(argc > 2) {
	filename = argv[2];
	if(argc > 3) {
	  start_print = atoi(argv[3]);
	  if(start_print==0) print_debug = 1;
	}
      }
    } else {
      filename = argv[1];
	if(argc > 2) {
	  start_print = atoi(argv[2]);
	  if(start_print==0) print_debug = 1;
	}

    }
  }
    
  //バイナリモードでファイルオープン
  if((fp = fopen(filename,"rb")) == NULL) {
    printf("file open error\n");
    return -1;
  }
  //HW用、レジスタやメモリの初期値をランダム化
  //初期値が0埋めされてなくても実行できるかのテスト
  if(init_randomize) {
    for(i=0;i<NUM_OF_REG;i++) {
      reg[i] = rand();
      freg[i] = (float)rand();
    }
    for(i=0;i<MEM_SIZE;i+=4){
      Mydata myd;
      myd.i = rand();
      memory[i] = myd.c[0];
      memory[i+1] = myd.c[1];
      memory[i+2] = myd.c[2];
      memory[i+3] = myd.c[3];
    }
  }
  
  //コードをメモリに載せる
  for(i=0;fread(memory+i,sizeof(unsigned char),1,fp) > 0;i++) {
  }
  fclose(fp);
  codesize = i;//何番地めまで読み込んだのか
  if(print_stat) {
    //メモリ使用領域を記録
    for(i=0;i<codesize/4;i++) {
      mem_used[i] = 1;
    }
  }
  
  //ラベルの情報を読み込む(コードサイズを見てから)
  char labelname[100] = "\0";
  strcat(labelname,filename);
  strcat(labelname,"_label");
  if((fp_label = fopen(labelname,"r")) == NULL) {
    printf("label file not found\n");
  } else {
    label_info = 1;
    make_label_list();
    fclose(fp_label);
  }
  //出力用ファイルをオープン
  char outfile[100] = "_out";
  strcat(filename,outfile);
  if((fp_out = fopen(filename,"wb")) == NULL) {
    printf("file open error\n");
    return -1;
  }

  //まずプログラム開始番地を読み、PCに代入
  init_pc = read_mem32(0);

  pc = init_pc;
  if(print_debug) {
    printf("read ");
    print_mem(0);
    printf(" => initial IP = %d\n",pc);
  }
  //1行(32bit)ずつ実行
  while(stop == 0) {
    op = read_mem32(pc);
    if(0) {
      printf("IP = %d \t\t| ",pc);
      print_mem(pc);
    }
    //ステップ実行の場合,"n","p"などを読む
    if(mode_step) {
      read = 1;
      while(read) {
	scanf("%s",s);
	if(strcmp(s,"n") == 0) {
	  read = 0;
	} else if(strcmp(s,"q") == 0 || strcmp(s,"quit") == 0) {
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
	} else if(strcmp(s,"pm_bin") == 0) {
	  scanf("%d",&num);
	  printf("MEMORY[%d] = ",num);
	  print_mem(num);
	}else if(strcmp(s,"pr_bin") == 0) {
	  scanf("%d",&num);
	  print_bin_little((unsigned int)reg[num]);
	}else if(strcmp(s,"pfr_bin") == 0) {
	  scanf("%d",&num);
	  print_bin_little((unsigned int)freg[num]);
	}
	//シミュレータのデバッグ用。ポインタ表示機能
	else if(strcmp(s,"pp_r") == 0) {
	  scanf("%d",&num);
	  printf("&%%r%d = ",num);
	  print_pointer((void *)(reg+num));
	}else if(strcmp(s,"pp_fr") == 0) {
	  scanf("%d",&num);
	  printf("&%%fr%d = ",num);
	  print_pointer((void *)(freg+num));
	}else if(strcmp(s,"pp_mem") == 0) {
	  scanf("%d",&num);
	  printf("&memory[%d] = ",num);
	  print_pointer((void *)(memory+num));
	}
	else {
	  printf("undefined command\n");
	}
      }
    }
    pc += 4;//命令を読んだ瞬間(just before executing it)に+される
    execute(op);//命令実行

    //pcがソースコードの長さ以上になったら
    if(pc > codesize) {
      printf("IP exceeded source code size. Simulation finished.\n");
      stop = 1;
    }
  }

  putchar('\n');

  t2 = gettime();

  //最後にレジスタや統計情報等を表示
  if(print_stat) {
    print_reg();
    print_freg();
    print_pc();
    putchar('\n');
    print_statistics();
  }
  fclose(fp_out);
  
  //実行時間の表示
  printf("elapsed time: %fs\n",t2-t1);

  return 0;
}
