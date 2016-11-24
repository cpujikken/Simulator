#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void print_bin_byte(unsigned char x){
  int i;
  unsigned char y = 0b10000000;
  for(i=0;i<8;i++) {
    if(x&y) {
      putchar('1');
    } else {
      putchar('0');
    }
    y = y >> 1;
  }
}

int main(int argc,char *argv[])
{
  FILE *fp;
  char filename[100] = "binary";
  unsigned char buf[8];
  int i;
  if(argc > 1) {
    strcpy(filename,argv[1]);
  }
  //バイナリモードでファイルオープン
  if((fp = fopen(filename,"rb")) == NULL) {
    printf("file open error\n");
    return -1;
  }
  //load codes
  printf("0\t: ");
  for(i=0;fread(buf,sizeof(unsigned char),1,fp) > 0;i++) {
    print_bin_byte(buf[0]);
    if(i%4!=3) {
      putchar(' ');
    } else {
      putchar('\n');
      printf("%d\t: ",i+1);
    }
  }
  //  if(i%4!=0)
  putchar('\n');
  fclose(fp);
  return 0;
}
