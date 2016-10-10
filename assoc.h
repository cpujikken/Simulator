#ifndef ASSOC_H
#define ASSOC_H

typedef struct assoc
{
  char *key;
  int val;
  struct assoc *next;
} Assoc;
struct assoc *create_assoc();
struct assoc *insert_assoc(char *,int val,struct assoc *);
int find_assoc(char *,struct assoc *);
#endif
