#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

struct linkedlist;
typedef struct linkedlist* LList;


LList makeLL();
LList cloneLL(LList);

int sizeOfLL(LList);
int isEmptyLL(LList);

void* getFromLL(LList, int idx);
int indexOfLL(LList, void *val);

void appendToLL(LList, void *val);
void addToLL(LList, int idx, void *val);

void* remFromLL(LList l, int idx);

#endif
