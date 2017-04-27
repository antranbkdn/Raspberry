//hihi
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "slist.h"
/*!
 * \brief Append an item to single link list
 * \param[in] list input single link list
 * \param[in] data pointer to data of appended item.
 * \return #slist the list after append
 */
extern slist * slist_append(slist * list, void * data) {
    slist* new_list = (slist*) malloc (sizeof (slist));
    slist* out_list = list;
    new_list->data = data;
    new_list->next = NULL;
    new_list->previous = NULL;

    if (list == NULL) {
        list = new_list;
        return list;
    }
    else {
        while (list->next != NULL) {
            list = list->next;
        }
        list->next = new_list;
        new_list->previous = list;
    }
    return out_list;
}


/*!
 * \brief Remove an item from single link list without release data of removed item.
 * \param[in] list input single link list
 * \param[in] data pointer to data of removed item.
 * \return #slist the list after remove
 */

extern slist * slist_remove_by_data(slist * list, void * data) {
    slist* walk = list;
    slist* out_list = list;

    while (walk != NULL) {
        if (walk->data == data) {                /* find data */
            if (walk == list) {                  /* first item */
                out_list = walk->next;
                out_list->previous = NULL;
                free (walk);
                return out_list;
            }
            else {								/* not first item */
                walk->previous->next = walk->next;
                free (walk);
                return out_list;
            }
        }
        walk = walk->next;
    }
    return out_list;
}

/*!
 * \brief Remove an item from single link list and release data of removed item.
 * \param[in] list input single link list
 * \param[in] data pointer to data of removed item.
 * \param[in] release_data 	callback function to release data of removed item
 * \return #slist the list after remove
 */

extern slist * slist_remove_by_data_x(slist * list, void * data, void (* release_data) (void * data)) {
    slist* walk = list;
    slist* out_list = list;

    while (walk != NULL) {
        if (walk->data == data) {
            if (walk == list) {
                out_list = walk->next;
                if (out_list) {
                    out_list->previous = NULL;
                }
                release_data ((void*)walk->data);
                free (walk);
                return out_list;
            }
            else {
                walk->previous->next = walk->next;
                release_data ((void*)walk->data);
                free (walk);
                return out_list;
            }
        }
        walk = walk->next;
    }
    return out_list;
}

/*!
 * \brief Remove an item from single link list by index of it in list, without release data
 * \param[in] list input single link list
 * \param[in] index the index of removed item in list.
 * \return #slist the list after remove
 */

extern slist * slist_remove_by_index(slist * list, int index) {
    slist* walk = list;
    slist* out_list = list;

    if (index == 0) {				/* first item */
        out_list = walk->next;
        out_list->previous = NULL;
        free (walk);
        return out_list;
    }
    while (index--) {
        walk = walk->next;
        if (walk == NULL) {			/* over tail */
                return out_list;	/* index invalid */
        }
    }
    /* after that, walk go to index */
    walk->previous->next = walk->next;
    free (walk);
    return out_list;
}
/*!
 * \brief Remove an item from single link list by index of it in list, release data of removed item.
 * \param[in] list input single link list
 * \param[in] index the index of removed item in list.
 * \param[in] release_data callback function to release data of removed item.
 * \return #slist the list after remove
 */

extern slist * slist_remove_by_index_x(slist * list, int index, void (* release_data) (void * data)) {
    slist* walk = list;
    slist* out_list = list;

    if (index == 0) {
        out_list = walk->next;
        out_list->previous = NULL;
        if (release_data) {
            release_data ((void*)walk->data);
        }
        free (walk);
        return out_list;
    }
    while (index--) {
        walk = walk->next;
        if (walk == NULL) { /* over */
                return out_list;
        }
    }
    /* after that walk go to index */
    walk->previous->next = walk->next;
    release_data ((void*)walk->data);
    free (walk);
    return out_list;
}

/*!
 * \brief Remove all item of list, without release data.
 * \param[in] list input single link list
 * \return #slist the list after delete
 */

extern slist * slist_delete_all(slist * list) {
    slist* walk = list;
    while (walk) {
        walk = list;
        list = list->next;
        free (walk);
        walk = list;
    }
    return NULL;
}


/*!
 * \brief Remove all item of list, release data of all item.
 * \param[in] list input single link list
 * \param[in] release_data callback function to release data of removed items.
 * \return #slist the list after delete
 */

extern slist * slist_delete_all_x(slist * list, void (*release_data)(void *data)) {
    slist* walk = list;
    while (walk) {
        walk = list;
        list = list->next;
        if (release_data) {
            release_data ((void*)walk->data);
        }
        free (walk);
        walk = list;
    }
    return NULL;
}

/*!
 * \brief Append a single link list to another single link list.
 * \param[in] list input single link list will be deleted
 * \param[in] add_list input single link list that is used to append to #list
 * \return #slist the list after append
 */

extern slist * slist_concat(slist * list, slist * added_list) {
    slist* out_list = list;
    slist* walk = list;

    if (walk == NULL) {				/* first list empty */
        return added_list;
    }

    while (walk->next != NULL) {	/* first list not empty */
        walk = walk->next;
    }
    /* after that, at tail of list */

    walk->next = added_list;
    added_list->previous = walk;
    return out_list;
}

/*!
 * \brief Insert an item to a single link list.
 * \param[in] list input single link list will be appended
 * \param[in] data pointer to data of inserted item.
 * \param[in] data_cond pointer to condition data.
 * \param[in] condition callback function to check if match condition.
 * \return #slist the list after insert
 */
extern slist * slist_insert(slist * list,void * data,void * data_cond, bool (*condition)(void * data1,void * data2)) {
    slist* insert_item = (slist*) malloc (sizeof (slist*));

    slist* walk = list;
    slist* out_list = list;

    while (walk->next) {
        if (condition && (data_cond != NULL)) {
            if (condition ((void*) walk->data, (void*) data_cond)) {
                insert_item->data = data;
                insert_item->next = walk->next;
                walk->next = insert_item;
                insert_item->previous = walk;
                insert_item->next->previous = insert_item;
                return out_list;
            }
        }
        walk = walk->next;
    }
    return out_list;
}

/*!
 * \brief Sort a single link list with  condition.
 * \param[in] list input single link list will be sort
 * \param[in] compare_func callback function to compare data of two item.
 * \return #slist the list after sort
 */

extern slist * slist_sort(slist * list, bool (*compare_func)(void * data1, void * data2) ) {
    int size_list = 0;
    int i,j;
    slist* walk = list;
    slist* walk2 = NULL;
    slist* out_list = NULL;

    if (walk == NULL)
        return list;
    while (walk->next) {
        size_list++;
        walk = walk->next;
    }
    size_list++;
    walk = list;

    for (i = 0; i < size_list; ++i) {
        walk2 = walk->next;
        for (j = i+1; j < size_list; j++) {
            if (compare_func) {
                if (compare_func ((void*) walk->data, (void*) walk2->data)) {
                    void* tmp = walk->data;
                    walk->data = walk2->data;
                    walk2->data = tmp;
                }
            }
            walk2 = walk2->next;
        }
        if (i == 0) out_list = walk;
        walk = walk->next;
    }
    return out_list;
}

/*!
 * \brief Find an item in a single link list with  condition.
 * \param[in] list input single link list will be find
 * \param[in] data input data condition.
 * \param[in] compare_func callback function to compare data of an item with condition data.
 * \return #slist the found item.
 */
extern slist * slist_find(slist * list, void * data, bool (*compare_func)(void * data1, void * data2)) {
    slist* walk = list;
    size_t index = 0;
    while (walk) {
        if (compare_func) {
            if (compare_func ((void*) walk->data, data)) {
                return walk;
            }
        }
        walk =  walk->next;
        index++;
    }
    return NULL;
}


/*!
 * \brief Trace all information of data of all item in single link list
 * \param[in] list input single link list will be found
 * \param[in] trace_func callback function to trace data.
 */
extern void	slist_trace(slist *list, void * data, void (* trace_func)(int index, void * data,void * itemData)) {
    slist* walk = list;
    int i = 0;
    while (walk) {
        if (trace_func) {
            trace_func (i, data, (void*) walk->data);
        }
        i++;
        walk = walk->next;
    }
}

extern size_t  slist_size(slist * list) {
    size_t size_list = 0;
    slist* walk = list;

    if (walk == NULL)
        return 0;
    while (walk->next) {
        size_list++;
        walk = walk->next;
    }
    return size_list + 1;
}
