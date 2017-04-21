
struct linkedlist;
typedef struct linkedlist* LList;


LinkedList makeLL();

int sizeOfLL(LList l);
int isEmptyLL(LList l);

void* getFromLL(LList l, int idx);

void appendToLL(LList l, void *val);
void addToLL(LList l, int idx, void *val);

void* remFromLL(LList l, int idx);


