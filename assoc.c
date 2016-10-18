#include <string.h>
#include <stdlib.h>
#include "assoc.h"

//連想配列を2分木で実装

struct assoc *create_assoc() {
  return NULL;
}

struct assoc *insert_assoc(char *s, int v,struct assoc *a) {
  if(a == NULL) {
    struct assoc *b = malloc(sizeof(struct assoc));
    char *k = malloc(32);
    strcpy(k,s);
    b->key = k;
    b->val = v;
    b->lchild = NULL;
    b->rchild = NULL;
    return b;
  }
  if(strcmp(s,a->key) < 0) {
    a->lchild = insert_assoc(s,v,a->lchild);
  } else if (strcmp(s,a->key) > 0) {
    a->rchild = insert_assoc(s,v,a->rchild);
  } else {
    a->val = v;
  }
  return a;
}
  /*
  struct assoc *b = malloc(sizeof(struct assoc));
  char *k = malloc(32);
  strcpy(k,s);
  b->key = k;
  b->val = v;
  b->next = a;
  return b;
}
  */


int find_assoc(char *s,struct assoc *a) {
  if(a==NULL) {
    return -1;
  }
  if(strcmp(s,a->key) < 0) {
    return find_assoc(s,a->lchild);
  } else if (strcmp(s,a->key) > 0) {
    return find_assoc(s,a->rchild);
  } else {
    return a->val;
  }
}
  /*  
  struct assoc *b = a;
  while(b->next != NULL) {
    if(strcmp(s,b->key) == 0) {
      return b->val;
    }
    b = b->next;
  }
  
  if(strcmp(s,b->key) == 0) {
    return b->val;
  }
  
  return -1;
}
  */
