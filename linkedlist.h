
struct linkedlist;
typedef struct linkedlist* LList;


LList makeLL();

int sizeOfLL(LList l);
int isEmptyLL(LList l);

void* getFromLL(LList l, int idx);
int indexOfLL(LList l, void *val);

void appendToLL(LList l, void *val);
void addToLL(LList l, int idx, void *val);

void* remFromLL(LList l, int idx);


