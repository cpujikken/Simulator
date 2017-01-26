#ifndef _LABEL_H_
#define _LABEL_H_

typedef struct label{
  unsigned int addr;
  char name[50];
  struct label *next;  
} Label;
extern int label_info;//ラベルと番地の対応のファイルがあるか
extern FILE *fp_label;
extern Label *lab;
extern void make_label_list();
extern char *addr2label(unsigned int);

#endif
