#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

typedef struct ELE {
    char *value;
    struct ELE *next;
    struct list_head list;
} list_ele_t;

typedef struct {
    list_ele_t *head; /* Linked list of elements */
    list_ele_t *tail;
    size_t size;
    struct list_head list;
} queue_t;


list_ele_t *get_middle(struct list_head *list)
{
    struct list_head *fast, *slow;
    fast = list->next;
    list_for_each (slow, list) {
        if (fast->next != list && fast->next->next != list)
            fast = fast->next->next;
        else
            break;
    }
    return list_entry(slow, list_ele_t, list);
}

void list_merge(struct list_head *lhs,
                struct list_head *rhs,
                struct list_head *head)
{
    struct list_head *temp;

    INIT_LIST_HEAD(head);
    if (list_empty(lhs)) {
        list_splice_tail(lhs, head);
        return;
    }
    if (list_empty(rhs)) {
        list_splice_tail(rhs, head);
        return;
    }

    while (!list_empty(lhs) && !list_empty(rhs)) {
        if (strcmp(list_entry(lhs->next, list_ele_t, list)->value,
                   list_entry(rhs->next, list_ele_t, list)->value) <= 0)
            temp = lhs->next;
        else
            temp = rhs->next;
        list_del(temp);
        list_add_tail(temp, head);
    }
    list_splice_tail(list_empty(lhs) ? rhs : lhs, head);
}

void list_merge_sort(queue_t *q)
{
    struct list_head sorted;
    queue_t left;

    if (list_is_singular(&q->list))
        return;

    INIT_LIST_HEAD(&left.list);
    list_cut_position(&left.list, &q->list, &get_middle(&q->list)->list);
    list_merge_sort(&left);
    list_merge_sort(q);
    list_merge(&left.list, &q->list, &sorted);
    INIT_LIST_HEAD(&q->list);
    list_splice_tail(&sorted, &q->list);
}

queue_t *q_new()
{
    queue_t *q = malloc(sizeof(queue_t));
    /* What if malloc returned NULL? */
    if (!q)
        return 0;

    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    INIT_LIST_HEAD(&q->list);
    return q;
}

void q_free(queue_t *q)
{
    list_ele_t *curr, *temp;

    if (!q)
        return;

    curr = q->head;
    while (curr) {
        temp = curr;
        curr = curr->next;
        free(temp->value);
        free(temp);
    }

    free(q);
}

bool q_insert_head(queue_t *q, char *s)
{
    list_ele_t *newh;
    char *new_value;

    /* What should you do if the q is NULL? */
    if (!q)
        return false;

    newh = malloc(sizeof(list_ele_t));
    if (!newh)
        return false;

    new_value = strdup(s);
    if (!new_value) {
        free(newh);
        return false;
    }

    newh->value = new_value;
    newh->next = q->head;
    q->head = newh;
    if (q->size == 0)
        q->tail = newh;
    q->size++;
    list_add_tail(&newh->list, &q->list);

    return true;
}

int main(void)
{
    char buf[256];
    FILE *fp = fopen("cities.txt", "r");
    queue_t *q = q_new();

    if (!fp) {
        perror("failed to open cities.txt");
        exit(EXIT_FAILURE);
    }

    while (fgets(buf, 256, fp)) {
        printf("%s", buf);
        q_insert_head(q, buf);
    }

    fclose(fp);
    q_free(q);

    return 0;
}
