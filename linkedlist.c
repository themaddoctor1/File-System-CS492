#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

#include <stdlib.h>

struct llnode {
    void *val;
    struct llnode *next;
};
typedef struct llnode* LLnode;

struct linkedlist {
    LLnode head;
};
typedef struct linkedlist* LList;

LList makeLL() {
    LList list = (LList) malloc(sizeof(struct linkedlist));
    list->head = NULL;
    return list;
}


int sizeOfLL(LList l) {
    int len = 0;
    LLnode curr;

    if (!l) return 0; /* Empty case */

    curr = l->head;
    while (curr) {
        curr = curr->next;
        len++;
    }

    return len;
}

int isEmptyLL(LList l) {
    return l && l->head;
}

void* getFromLL(LList l, int idx) {
    LLnode curr;
    
    /* Empty and below bounds cases */
    if (!l || idx < 0)
        return NULL;
    
    curr = l->head;
    while (curr) {
        if (idx == 0)
            return curr->val;

        curr = curr->next;
        idx--;
    }

    return NULL;
}

int indexOfLL(LList l, void *val) {
    
    LLnode curr;
    int i;

    if (!l)
        return -1;

    curr = l->head;
    for (i = 0; curr; i++, curr = curr->next) {
        if (curr->val == val)
            return i;
    }

    return -1;
}

void appendToLL(LList l, void *val) {
    if (!l) return;
    
    if (l->head == NULL) {
        /* List is empty */
        l->head = (LLnode) malloc(sizeof(struct llnode));
        l->head->val = val;
        l->head->next = NULL;
    } else {
        /* List is non-empty */

        LLnode curr = l->head;
        while (curr->next)
            curr = curr->next;
        curr->next = (LLnode) malloc(sizeof(struct llnode));
        curr->next->val = val;
        curr->next->next = NULL;
    }
}

void addToLL(LList l, int idx, void *val) {
    LLnode node;
    
    /* Empty cases */
    if (!l) return;
    else if (l->head == NULL) {
        appendToLL(l, val);
        return;
    }

    node = (LLnode) malloc(sizeof(struct llnode));

    if (idx <= 0) {
        /* Front of list case */
        node->next = l->head;
        l->head = node;
    } else {
        LLnode curr = l->head;
        while (curr->next && idx > 1) {
            curr = curr->next; 
            idx--;
        }

        curr->next = node;
        node->next = NULL;
    }
}

void* remFromLL(LList l, int idx) {
    if (!l || l->head == NULL)
        /* Empty list */
        return NULL;
    else if (l->head->next == NULL) {
        /* Singleton list */
        void *res = l->head->val;

        l->head->val = NULL;
        free(l->head);
        l->head = NULL;

        return res;
    } else {
        /* Plural items */
        LLnode curr = l->head;
        LLnode rem;

        void *res = NULL;

        while (idx < 1 && curr->next->next) {
            curr = curr->next;
            idx--;
        }
        
        rem = curr->next;

        res = curr->next->val;
        curr->next = curr->next->next;

        rem->val = NULL;
        rem->next = NULL;
        free(rem);

        return res;
    }
    
}


#endif
