#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "linkedlist.h"

struct linkedlist
{
    struct linkedlist_node *first;
    unsigned int length;
};

struct linkedlist_node
{
    int key;
    int value;
    struct linkedlist_node *next;
};
typedef struct linkedlist_node linkedlist_node_t;

linkedlist_t *ll_init()
{
    // create a new linked list

    // We have done this TODO for you as an example of how to use malloc().
    // You might want to read more about malloc() from Linux Manual page.
    // Usually free() should be used together with malloc(). For example,
    // the linkedlist you are currently implementing usually have free() in the
    // ll_free() function.

    // First, you use C's sizeof function to determine
    // the size of the linkedlist_t data type in bytes.
    // Then, you use malloc to set aside that amount of space in memory.
    // malloc returns a void pointer to the memory you just allocated, but
    // we can assign this pointer to the specific type we know we created
    linkedlist_t *list = malloc(sizeof(linkedlist_t));

    // set metadata for your new list and return the new list
    list->first = NULL;
    list->length = 0;
    return list;
}

void ll_free(linkedlist_t *list)
{
    // free a linked list from memory
    linkedlist_node_t *current = list->first;
    linkedlist_node_t *temp;
    while (current != NULL){
        temp = current->next;
        free(current);
        current = temp;
    }
    free(list);
}

void ll_add(linkedlist_t *list, int key, int value)
{
    // create a new node and add to the front of the linked list if a
    // node with the key does not already exist.
    // Otherwise, replace the existing value with the new value.
    bool exist = false;
    linkedlist_node_t *current = list->first;
    while (current != NULL){
        if (current->key == key){
            exist = true;
            current->value = value;
        }
        current = current->next;
    }
    if (!exist){
        linkedlist_node_t *node = malloc(sizeof(linkedlist_node_t));
        node->next = list->first;
        node->key = key;
        node->value = value;

        list->first = node;
        list->length++;
    }
}

int ll_get(linkedlist_t *list, int key)
{
    // go through each node in the linked list and return the value of
    // the node with a matching key.
    // If it does not exist, return 0.
    linkedlist_node_t *current = list->first;
    while (current != NULL){
        if (current->key == key){
            return current->value;
        }
        current = current->next;
    }
    return 0;
}

int ll_size(linkedlist_t *list)
{
    // return the number of nodes in this linked list
    return list->length;
}
