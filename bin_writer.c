#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc,char *argv[])
{
  FILE *fp;
  char filename[100] = "binary";
  char buf[32];
  char x;
  int i;
  if(argc > 1) {
    strcpy(filename,argv[1]);
  }
  //バイナリモードでファイルオープン
  if((fp = fopen(filename,"wb")) == NULL) {
    printf("file open error\n");
    return -1;
  }
  while(1) {
    scanf("%s",buf);
    if(buf[0] < '0' || buf[0] > '1') {
    fclose(fp);
    return 0;
    }
    x=0;
    for(i=0;i<8;i++) {
      x = x << 1;
      if(buf[i] == '1'){
	x = x|1;
      }
    }
    fwrite(&x,sizeof(unsigned char),1,fp);
  }
  fclose(fp);
  return 0;
}
