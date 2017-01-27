#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base.h"
#include "label.h"

int label_info = 0;
FILE *fp_label;
Label *lab;
Label *curlab;
char unknown_labelname[20] = "unknown label";
void make_label_list() {
  char buf[200];
  lab = malloc(sizeof(Label));
  curlab = lab;//現在いじってるlabel*
  Label *nextlab;
  
  while(fgets(buf,200,fp_label) != NULL) {
    if(sscanf(buf,"%d %s",  &(curlab->addr), curlab->name) >= 2) {
      nextlab = malloc(sizeof(Label));
      curlab->next = nextlab;
      curlab = nextlab;
    }
  }
  
  /*
  while(fscanf(fp_label,"%d %s",&(curlab->addr),curlab->name) != EOF) {
    nextlab = malloc(sizeof(Label));
    curlab->next = nextlab;
    curlab = nextlab;
  }
  */
  curlab->addr=codesize;
  strcpy(curlab->name,"memory_domain");
  curlab->next = NULL;  
}

char *addr2label(unsigned int in_addr) {
  curlab = lab;
  if(lab == NULL) {
    return unknown_labelname;
  }
  while(curlab->next != NULL && curlab->next->addr <= in_addr) {
    curlab = curlab->next;
  }
  return curlab->name;
}
