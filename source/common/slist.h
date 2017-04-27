#ifndef SLIST_H_
#define SLIST_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct slist{
    void * data;  /**< the pointer to data */
    struct slist * next;  /**< the pointer to next element of list */
    struct slist *previous;
}slist;

#define FORCE_DATA_TYPE(type,item) ((type*)(item->data))

typedef void (*release_data_function)(void*);
typedef bool (*compare_function)(void*,void*);
typedef void (*trace_function)(int , void*);

/*!
 * \brief Append an item to single link list
 * \param[in] list input single link list
 * \param[in] data pointer to data of appended item.
 * \return #slist the list after append
 */
extern slist * slist_append(slist * list, void * data);

/*!
 * \brief Remove an item from single link list without release data of removed item.
 * \param[in] list input single link list
 * \param[in] data pointer to data of removed item.
 * \return #slist the list after remove
 */

extern slist * slist_remove_by_data(slist * list, void * data);

/*!
 * \brief Remove an item from single link list and release data of removed item.
 * \param[in] list input single link list
 * \param[in] data pointer to data of removed item.
 * \param[in] release_data 	callback function to release data of removed item
 * \return #slist the list after remove
 */

extern slist * slist_remove_by_data_x(slist * list, void * data, void (* release_data) (void * data));

/*!
 * \brief Remove an item from single link list by index of it in list, without release data
 * \param[in] list input single link list
 * \param[in] index the index of removed item in list.
 * \return #slist the list after remove
 */

extern slist * slist_remove_by_index(slist * list, int index);

/*!
 * \brief Remove an item from single link list by index of it in list, release data of removed item.
 * \param[in] list input single link list
 * \param[in] index the index of removed item in list.
 * \param[in] release_data callback function to release data of removed item.
 * \return #slist the list after remove
 */

extern slist * slist_remove_by_index_x(slist * list, int index, void (* release_data) (void * data));

/*!
 * \brief Remove all item of list, without release data.
 * \param[in] list input single link list
 * \return #slist the list after delete
 */

extern slist * slist_delete_all(slist * list);

/*!
 * \brief Remove all item of list, release data of all item.
 * \param[in] list input single link list
 * \param[in] release_data callback function to release data of removed items.
 * \return #slist the list after delete
 */

extern slist * slist_delete_all_x(slist * list, void (*release_data)(void *data));

/*!
 * \brief Append a single link list to another single link list.
 * \param[in] list input single link list will be deleted
 * \param[in] add_list input single link list that is used to append to #list
 * \return #slist the list after append
 */

extern slist * slist_concat(slist * list, slist * added_list);

/*!
 * \brief Insert an item to a single link list.
 * \param[in] list input single link list will be appended
 * \param[in] data pointer to data of inserted item.
 * \param[in] data_cond pointer to condition data.
 * \param[in] condition callback function to check if match condition.
 * \return #slist the list after insert
 */

extern slist * slist_insert(slist * list,void * data,void * data_cond, bool (*condition)(void * data1,void * data2));

/*!
 * \brief Sort a single link list with  condition.
 * \param[in] list input single link list will be appended
 * \param[in] compare_func callback function to compare data of two item.
 * \return #slist the list after sort
 */

extern slist * slist_sort(slist * list, bool (*compare_func)(void * data1, void * data2) );

/*!
 * \brief Find an item in a single link list with  condition.
 * \param[in] list input single link list will be find
 * \param[in] data input data condition.
 * \param[in] compare_func callback function to compare data of an item with condition data.
 * \return #slist the found item.
 */
extern slist * slist_find(slist * list, void * data, bool (*compare_func)(void * data1, void * data2));

/*!
 * \brief Trace all information of data of all item in single link list
 * \param[in] list input single link list will be found
 * \param[in] trace_func callback function to trace data.
 */
extern void	slist_trace(slist *list, void * data, void (* trace_func)(int index, void * data,void * itemData));


/*!
 * \brief Do some job with all items in single link list
 * \param[in] list input single link list will be found
 * \param[in] work_func callback function to trace data.
 */
extern void	slist_work(slist *list, void * data,void (* work_func)(int index,void * itemData,void * data));

/*!
 * \brief Get amount of items in single link list.
 * \param[in] list input single link list
 * \return amount items
 */
extern size_t  slist_size(slist * list);







//ninh add
extern slist*  slist_prepend(slist *list, void* data);
extern slist* slist_nth(slist  *list, int n);
extern void * slist_nth_data(slist *list, int  n);
extern int slist_index(slist *list, void *data);
extern slist*  slist_last(slist *list);
extern slist* slist_insert_index (slist *list, void *data, int position);



#ifdef __cplusplus
}//extern "C" {
#endif


#endif /* SLIST_H_ */
