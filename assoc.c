#include <string.h>
#include <stdlib.h>
#include "assoc.h"

struct assoc *create_assoc() {
  return NULL;
  /*
  struct assoc *a = malloc(sizeof(struct assoc));
  char dummy[2] = "0";
  a->key = dummy;
  a->val = 0;
  a->next = NULL;
  return a;
  */
}

struct assoc *insert_assoc(char *s, int v,struct assoc *a) {
  struct assoc *b = malloc(sizeof(struct assoc));
  char *k = malloc(32);
  strcpy(k,s);
  b->key = k;
  b->val = v;
  b->next = a;
  return b;
  /*
  struct assoc *b = malloc(sizeof(struct assoc));
  char k[32];
  strcpy(k,s);
  b->key = k;
  b->val = v;
  b->next = NULL;
  struct assoc *c = a;
  while(c->next != NULL) {
    c = c->next;
  }
  c->next = b;
  return a;
  */
}

int find_assoc(char *s,struct assoc *a) {
  if(a==NULL) {
    return -1;
  }
  
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
