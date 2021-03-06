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
  //asm( "pushf\n\torl $0x40000,(%rsp)\n\tpopf");//バスエラー検出

  char s[20];
  unsigned int i;
  int line = 0;
  unsigned int op;
  Operation o;
  Ldst l;
  double t1,t2;//実行時間測定用
  int num;//ステップ実行用
  int read;//ステップ実行用
  int pred_ip;//最後の命令をprintするためのpc保存用変数
  
  t1 = gettime();
  if(argc > 1) {
    //ステップ実行モード
    if(argv[1][0] == '-' && argv[1][1] == 's') {
      mode_step = 1;
      print_debug = 1;
      if(argc > 2) {
	filename = argv[2];
	if(argc > 3) {
	  strcpy(name_sld,argv[3]);
	  if(argc > 4) {
	    start_print = atoi(argv[4]);
	    printf("start from dic=%d\n",start_print);
	    if(start_print == 0) {
	      print_debug = 1;
	      mode_step = 1;
	    } else {
	      start_with_step = 1;
	      mode_step = 0;
	      print_debug = 0;
	    }
	    if(argc > 5) end_point = atoi(argv[5]);
	  }
	}
      }
    } else {
      filename = argv[1];
      if(argc > 2) {
	strcpy(name_sld,argv[2]);
	if(argc > 3) {
	  start_print = atoi(argv[3]);
	  printf("start from dic=%d\n",start_print);
	  if(start_print>0) {
	    start_with_step = mode_step;
	    mode_step = 0;
	    print_debug = 0;
	  } else {
	    print_debug = 1;
	  }
	  if(argc > 4) {
	    end_point = atoi(argv[4]);
	  }
	}
      }
    }
  }

  //debug情報非表示でstep実行は止まってるのと見分けがつかない
  if(print_debug == 0 && mode_step == 1) {
    mode_step = 0;
  }
    
  //バイナリモードでファイルオープン
  if((fp = fopen(filename,"rb")) == NULL) {
    printf("binary file open error\n");
    return -1;
  }
  
  
  //コメント付きのファイルをオープン
  if(print_comment == 1) {
    fp_com = fopen(name_com,"r");
    if(fp_com == NULL)
      printf("%s does not exist\n",name_com);
  }

  //入力sldファイルをオープン
  /*

  */
  fp_sld = fopen(name_sld,"rb");
  if(fp_sld == NULL) {
    printf("%s not found\n",name_sld);
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
  
  if(fp_com != NULL) {
    for(i=0;i<codesize/4;i++) {
      fgets(memory_com[i],COMMENT_LENGTH,fp_com);
    }
  
  }
  
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

  set_stack(init_pc);//デバッグ用の、関数呼び出しの記録を取るスタックをセット

  pc = init_pc;
  if(print_debug) {
    printf("read ");
    print_mem(0);
    printf(" => initial IP = %d\n",pc);
  }

  //1行(32bit)ずつ実行
  while(stop == 0) {
    
    if(din_count > 0 && dyna % din_count == 0) {
      printf("dic = %ld\n",dyna);
    }
    
    

    //skipモードなら次の番地まで飛ばす
    if(mode_sipnext > 0 && mode_step==1) {
      mode_step = 0;
      print_debug = 0;
      printf("skip until IP=%d...\n",mode_sipnext);
    }
    //skipモードからの復帰
    if(mode_sipnext == pc) {
      mode_sipnext = 0;
      mode_step = 1;
      print_debug = 1;
    }

    //命令解読
    if(pc > codesize) {
      stop = 1;
    } else {
      op = read_mem32(pc);
      o = parse(op);
      l = parse_ldst(op);
      
      //命令を表示 _labelファイルがあるときはラベル名も表示
      print_op(o,l,pc);
      pred_ip = pc;

      //binary表示
      if(print_op_bin && print_debug) {
	printf("IP = %d \t| ",pc);
	print_mem(pc);
      }
      
      //ステップ実行の場合,"n","p"などを読む
      if(mode_step) {
	read = 1;
	while(read) {
	  scanf("%s",s);
	  if(strcmp(s,"n") == 0) {
	    read = 0;
	  } else if(strcmp(s,"s") == 0) {
	    read = 0;
	    //前の命令がSIPである必要がある
	    i = read_mem32(pc-4);
	    i = i >> 26;//i=前の命令のオペコード
	    if(i == OP_SIP) {
	      mode_sipnext = pc + 4;
	    }
	  } else if(strcmp(s,"j") == 0) {
	    read = 0;
	    mode_step = 0;
	    mode_jump = 1;
	  } else if(strcmp(s,"r") == 0) { //最後まで実行
	    read = 0;
	    print_debug = 0;
	    mode_step = 0;
	  } else if(strcmp(s,"dic") == 0) {
	    scanf("%d",&num);
	    
	  } else if(strcmp(s,"rp") == 0){
	    read = 0;
	    mode_step = 0;
	  } else if(strcmp(s,"q") == 0 || strcmp(s,"quit") == 0) {
	    read=0;
	    stop=1;
	  } else if(strcmp(s,"pr") == 0) {
	    print_reg();
	  } else if(strcmp(s,"pfr") == 0) {
	    print_freg();
	  } else if(strcmp(s,"pip") == 0) {
	    print_pc();
	  } else if(strcmp(s,"pdic") == 0) {
	    printf("%ld\n",dyna);
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
    }
    pc += 4;//命令を読んだ瞬間(just before executing it)に+される
    
    if(stop==0)
      execute(op,o,l);//命令実行
    
    //pcがソースコードの長さ以上になったら or 4の倍数でなかったら
    if(pc > codesize) {
      sprintf(error_mes,"IP exceeded source code size\n");
      stop = 1;
    } else if (pc % 4 != 0) {
      sprintf(error_mes,"IP is not multiple of 4\n");
      stop = 1;
    }
    //デバッグ用の機能
    //reg_spに最初に値が代入されたらそれがinit_sp ヒープも同様
    if(sp_flag && reg[REG_SP] != 0) {
      init_sp = reg[REG_SP];
      sp_flag = 0;
    }
    if(hp_flag && reg[REG_HP] != 0) {
      init_hp = reg[REG_HP];
      hp_flag = 0;
    }
    
    //reg_hp_max,reg_sp_maxを更新
    if(reg_hp_max < reg[REG_HP]) {
      reg_hp_max = reg[REG_HP];
    }
    if(reg_sp_max < reg[REG_SP]) {
      reg_sp_max = reg[REG_SP];
    }

    //デバッグ用の機能
    //ヒープポインタがスタック領域に侵入したらor
    //スタックポインタがヒープ領域に侵入したらストップ
    if(init_sp > 0 && init_hp > 0) {
      if(init_hp < init_sp) {
	if(reg_hp_max >= init_sp) {
	  sprintf(error_mes,"heap pointer reached stack domain\n");
	  stop = 1;
	}
      } else if(reg_sp_max >= init_hp) {
	sprintf(error_mes,"stack pointer reached heap domain\n");
	stop = 1;
      }      
    }
  }

  putchar('\n');

  t2 = gettime();

  if(fp_sld != NULL) {
    //fclose(fp_sld);
  }

  //最後にレジスタや統計情報等を表示
  if(print_stat) {
    print_stack();
    putchar('\n');
    printf("%s",error_mes);
    if(reg[REG_SP] > init_sp) {
      printf("stack is not empty (%%rsp = %d > %d = initial sp)\n",
	     reg[REG_SP],init_sp);
    }
    //最後に実行した命令をプリント
    if(pred_ip > 0 && pred_ip <= codesize) {
      print_debug = 1;
      op = read_mem32(pred_ip);
      o = parse(op);
      l = parse_ldst(op);
      printf("last operation:\n");
      print_op(o,l,pred_ip);
    }
    putchar('\n');
    print_reg();
    print_freg();
    print_statistics();
  }
  fclose(fp_out);
  
  //実行時間の表示
  printf("elapsed time: %fs\n",t2-t1);
  /*  */
  return 0;
}
