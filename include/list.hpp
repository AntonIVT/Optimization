/**
* @file
*/

#pragma once
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include "list.hpp"
    
//#define DEBUG_MODE

struct list_iterator
{
    long long it;
};

typedef enum list_error_en 
{
    LIST_OK = 0,
    LIST_ERROR = 1,
    LIST_OVERFLOW = 2,
    LIST_UNDERFLOW = 3,
    LIST_WRONG_INDEX = 4,
    LIST_WRONG_REALLOC = 5,
    LIST_CYCLE = 6
} list_error;

template <typename T>
struct Node
{
    T value;
    long long prev;
    long long next;
};

template <typename T>
struct My_list
{
    Node<T> *data;
    T default_el;

    size_t size;
    size_t capacity;

    long long free;              
    char boost_mode;

    long long head;

    list_error construct(size_t capacity, const T &new_default_el);

    list_error destruct();

    list_error boost();

    list_error shrink_to_fit();

    list_iterator push_back(const T &x);

    list_iterator push_front(const T &x);

    list_error pop_back();

    list_error pop_front();

    T& operator[] (long long logic_number);
    T& operator[] (list_iterator iter);

    /* Insert before element with such logic number, (actually insert to this place) */
    list_error insert(const T &x, long long logic_number);
    list_iterator insert(const T &x, list_iterator iter);

    list_error erase(long long logic_number);
    list_error erase(list_iterator iter);

    size_t get_size();

    bool is_boosted();

    list_iterator begin();
    list_iterator end();

    void iter_increase(list_iterator &iter);
    void iter_decrease(list_iterator &iter);

private:
    /*! 
    Verify function list
    \param lst Object List which need to verify
    \return Code of verify. LIST_OK if list OK, or another error if list bad
    */  
    list_error verify();

    /*! 
    Reallocation of list's data memory
    \param lst Object List which data will be reallocated
    \param new_cap New capacity for objest lst
    \return Code of verify. LIST_OK if list OK, or another code if there's error
    */  
    list_error resize(size_t new_cap);

    /*! 
    Function of linear search of iterator
    \param lst Object List in which need to find iterator
    \param logic_number Number in lst
    \return Iterator if it's OK, or -1 if there's error
    */  
    list_iterator find_array_number(long long logic_number);

    /*! 
    Insert new element before array_number element. if array_number is head of the list insert new element in the end
    \param lst List where need to insert element
    \param x Element of list which need to insert
    \param array_number Number in array (iterator actually) before which need to insert element
    \return Code of verify. LIST_OK if list OK, or another code if there's error
    */  
    list_error insert_internal(const T &x, long long array_number);

    /*!
    Erase element by array_number.
    \param lst List where need to erase element
    \param array_number Number in array(iterator actually) which need to erase
    \return Code of verify. LIST_OK if list OK, or another code if there's error
    */
    list_error erase_internal(long long array_number);

    /*!
    Checking validity of iterator
    \param lst List where need to check iterator
    \param iter Iterator in list which need to check
    \return 1 if iterator valid, or 0 if it's not
    */
    bool is_iterator_valid(list_iterator iter);
};

template <typename T>
list_error My_list<T>::verify()
{
    if (data == NULL)
    {
        printf("Ban because this-data == NULL\n");
        return LIST_ERROR;
    }
    if (size > capacity)
    {
        printf("Ban because of overflow\n");
        return LIST_OVERFLOW;
    }
    if (size < 0)
    {
        printf("Ban because of underflow\n");
        return LIST_UNDERFLOW;
    }
    
    #ifdef DEBUG_MODE
    if (head != -1)
    {
        long long curr_node = data[head].next;
        long long logic_number = 1;
        while (curr_node != head && logic_number < size)
        {
            curr_node = data[curr_node].next;
            logic_number++;
        }
        if (logic_number != size)
        {
            printf("There's cycle in upstream way\n");
            return LIST_CYCLE;
        }
        curr_node = data[head].prev;
        logic_number = 1;
        while (curr_node != head && logic_number < size)
        {
            curr_node = data[curr_node].prev;
            logic_number++;
        }
        if (logic_number != size)
        {
            printf("There's cycle in reverse way\n");
            return LIST_CYCLE;
        }
    }
    if (free != -1)
    {
        long long curr_node = free;
        long long logic_number = 0;
        while (curr_node != -1 && logic_number < capacity - size)
        {
            curr_node = data[curr_node].next;
            logic_number++;    
        }
        if (logic_number != capacity - size)
        {
            printf("There's cycle in free places\n");
            return LIST_CYCLE;
        }
    }
    #endif
    
    return LIST_OK;
}

template <typename T>
bool My_list<T>::is_iterator_valid(list_iterator iter)
{    
    //if (verify() != LIST_OK)
    //    return false;
    
    if (iter.it < 0 || iter.it >= capacity || data[iter.it].prev == -1)
    {   
        printf("Wrong iterator\n");
        return false;
    }
    else
        return true;
}

template <typename T>
list_error My_list<T>::resize(size_t new_cap)
{    
    //if (verify() != LIST_OK)
    //    return LIST_ERROR;
        
    if (new_cap < size || 
       (boost_mode == false && new_cap <= capacity) || 
       (boost_mode == true  && new_cap <  size))
        return LIST_ERROR; 
    
    Node<T> *new_data = (Node<T> *)realloc(data, new_cap * sizeof(Node<T>));
    
    if (new_data == NULL)
        return LIST_WRONG_REALLOC;
    
    data = new_data;
    
    if (new_cap == size)
    {
        capacity = new_cap;
        return LIST_OK;
    }
    
    int last_free = free;
    if (last_free == -1)
        free = capacity;
    else    
    {
        while(data[last_free].next != -1)
            last_free = data[last_free].next;
        
        data[last_free].next = capacity;
    }
    for (int i = capacity; i < new_cap - 1; i++)
    {
        Node<T> free_node = {default_el, -1, i + 1};
        data[i] = free_node;
    }
    Node<T> last_node = {default_el, -1, -1};
    data[new_cap - 1] = last_node;
        
    capacity = new_cap;
    return LIST_OK;
}

template <typename T>
list_iterator My_list<T>::find_array_number(long long logic_number)
{
    list_iterator ret = {-1};

    //if (verify() != LIST_OK)
    //    return ret;

    if (logic_number >= size || logic_number < 0)
        return ret;
    
    long long array_number = head; 
    
    for (long long i = 0; i < logic_number; i++)
        array_number = data[array_number].next;
    
    ret.it = array_number;
    return ret;
}


template <typename T>
list_error My_list<T>::insert_internal(const T &x, long long array_number)
{
    //if (verify() != LIST_OK)
    //    return LIST_ERROR;
    
    if (array_number < 0 || array_number >= capacity)       
        return LIST_WRONG_INDEX;

    if (data[array_number].prev == -1 && !(size == 0 && array_number == 0))
        return LIST_WRONG_INDEX;

    if (capacity == size)
    {
        if (resize(capacity * 2) != LIST_OK)
            return LIST_WRONG_REALLOC;
    }
    
    long long free_tmp = free;
    free = data[free_tmp].next;                    
    
    if (size == 0)
    {   
        head = free_tmp;
        Node<T> new_node = {x, head, head};
        data[head] = new_node;
        
        size++;
        return LIST_OK;
    }
    
    Node<T> new_node = {x, data[array_number].prev, array_number};
    
    data[free_tmp] = new_node;
    data[new_node.prev].next = free_tmp;
    data[new_node.next].prev = free_tmp;
        
    size++;
    
    return LIST_OK;
}

template <typename T>
list_error My_list<T>::erase_internal(long long array_number)
{    
    //if (verify() != LIST_OK)
    //    return LIST_ERROR;
    
    if (array_number < 0 || array_number >= capacity) 
        return LIST_ERROR;
    
    if (data[array_number].prev == -1)
        return LIST_ERROR;
    
    if (size == 1)
    {
        head = -1;
        
        Node<T> dead_node = {default_el, -1, free};
        data[array_number] = dead_node;
        free = array_number;
        
        size--;
        boost_mode = 0;
        
        return LIST_OK;
    }
    
    if (head == array_number)
        head = data[array_number].next;
    
    data[data[array_number].prev].next = data[array_number].next;
    data[data[array_number].next].prev = data[array_number].prev;
    
    Node<T> dead_node = {default_el, -1, free};
    data[array_number] = dead_node;
    free = array_number;
    
    size--;
    
    return LIST_OK;
}

template <typename T>
list_error My_list<T>::construct(size_t new_capacity, const T& new_default_el)
{    
    if (new_capacity <= 0)
        new_capacity = 1;
    
    data = (Node<T> *)calloc(new_capacity, sizeof(Node<T>));
    
    assert(data != NULL);
    
    free = 0;
    for (int i = 0; i < new_capacity - 1; i++)
    {
        Node<T> free_node = {default_el, -1, i + 1};
        data[i] = free_node;
    }
    Node<T> last_node = {default_el, -1 , -1};
    data[new_capacity - 1] = last_node;
    
    head = -1;
    capacity = new_capacity;
    size = 0;
    boost_mode = 0;
    default_el = new_default_el;
    
    //return verify();
    return LIST_OK;
}

template <typename T>
list_error My_list<T>::destruct()
{   
    //if (verify() != LIST_OK)
    //    return LIST_ERROR;
    
    std::free(data);
    capacity = 0;
    size =  0;
    head = -1;
    free = -1;
    boost_mode =  0;
    
    return LIST_OK;
}

template <typename T>
list_error My_list<T>::boost()
{    
    //if (verify() != LIST_OK)
    //    return LIST_ERROR;
    
    if (boost_mode == 1)
        return LIST_OK;
    
    if (size == 0)
    {
        boost_mode = 1;
        return LIST_OK;
    }
    
    Node<T> *new_data = (Node<T> *)calloc(size, sizeof(Node<T>));
    
    if (new_data == NULL)
        return LIST_WRONG_REALLOC;
    
    long long curr_node = head;
    for (int i = 0; i < size; i++)
    {
        new_data[i].value = data[curr_node].value;
        new_data[i].next = (i + 1) % size;
        new_data[i].prev = (i - 1 + size) % size;
        curr_node = data[curr_node].next;
    }
    
    std::free(data);
    data = new_data;
    capacity = size;
    free = -1;
    head = 0;
    boost_mode = 1;
        
    return LIST_OK;
}

template <typename T>
list_error My_list<T>::shrink_to_fit()
{    
    //if (verify() != LIST_OK)
    //    return LIST_ERROR;
    
    if (boost_mode == 1)
        return resize( size);
    else
        return LIST_ERROR;
}

template <typename T>
list_iterator My_list<T>::push_back(const T &x)
{       
    list_iterator ret = {-1};

    //if (verify() != LIST_OK)
    //    return ret;
        
    if (size > 0)
    {
        if (insert_internal(x, head) == LIST_OK)
            ret.it =  data[head].prev;
        
        return ret;
    }
    else
    {
        if (insert_internal(x, 0) == LIST_OK)
        {
            ret.it = head;
        }
        return ret;        
    }
}

template <typename T>
list_iterator My_list<T>::push_front(const T &x)
{
    list_iterator ret = {-1};

    //if (verify() != LIST_OK)
    //    return ret;
    
    list_error check = LIST_OK;

    if (boost_mode == 1) boost_mode = 0;
    
    if (size > 0)
        check = insert_internal(x, head);
    else
        check = insert_internal(x, 0);
    
    if (check == LIST_OK)
    {
        head = data[head].prev;
        ret.it = head;
        return ret;
    }
    else 
        return ret;
}

template <typename T>
list_error My_list<T>::pop_back()
{
    //if (verify() != LIST_OK)
    //    return LIST_ERROR;
    
    if (size == 0)
        return LIST_ERROR;
    
    return erase_internal(data[head].prev);    
}

template <typename T>
list_error My_list<T>::pop_front()
{
    //if (verify() != LIST_OK)
    //    return LIST_ERROR;
    
    if (size == 0)
        return LIST_ERROR;
    
    if (boost_mode == 1) boost_mode = 0;
    
    return erase_internal(head);  
}

template <typename T>
list_error My_list<T>::insert(const T &x, long long logic_number)
{
    // if (verify() != LIST_OK)
        // return LIST_ERROR;
        
    if (logic_number > size || logic_number < 0)
        return LIST_WRONG_INDEX;
        
    if (logic_number == size)
        push_back(x);
    
    boost_mode = 0;
    
    if (logic_number == 0)
        push_front(x);
    
    return insert_internal(x, find_array_number(logic_number));
}


template <typename T>
list_iterator My_list<T>::insert(const T &x, list_iterator iter)
{
    list_iterator ret = {-1};

    //if (verify() != LIST_OK)
    //    return ret;
    
    //if (!is_iterator_valid(iter))
    //    return ret;
    
    boost_mode = 0;
    
    if (iter.it == head)
        return push_back(x);
    
    insert_internal(x, iter);
    
    ret.it = data[iter.it].prev;
    return ret;
}  

template <typename T>
list_error My_list<T>::erase(long long logic_number)
{
    //if (verify() != LIST_OK)
    //    return LIST_ERROR;
    
    if (logic_number >= size || logic_number < 0)
        return LIST_WRONG_INDEX;
    
    if (logic_number == size - 1)
        return pop_back();
    
    boost_mode = 0;
    
    if (logic_number == 0)
        return pop_front();
    
    return erase_internal(find_array_number(logic_number));
}

template <typename T>
list_error My_list<T>::erase(list_iterator iter)
{
    //if (verify() != LIST_OK)
    //    return LIST_ERROR;
    
    //if (!is_iterator_valid(iter))
    //    return LIST_ERROR;

    if (iter.it == data[head].prev)
        return pop_back();
        
    boost_mode = 0;
    
    if (iter.it == head)
        return pop_front();
    
    return erase_internal(iter.it);
}


template <typename T>
size_t My_list<T>::get_size()
{
    //if (verify() != LIST_OK)
    //    return -1;
    
    return size;
}

template <typename T>
bool My_list<T>::is_boosted()
{
    //if (verify() != LIST_OK)
    //    return false;
    
    return (boost_mode == 1) ? true : false;
}

template <typename T>
T& My_list<T>::operator[] (long long logic_number)
{
    //if (verify() != LIST_OK)
    //    return default_el;
        
    if (!boost_mode)
        return data[find_array_number(logic_number).it].value;
    else
        return data[logic_number].value;    
}

template <typename T>
T& My_list<T>::operator[] (list_iterator iter)
{
    //if (verify() != LIST_OK)
    //    return default_el;
        
    //if (is_iterator_valid(iter))
    return data[iter.it].value;
    //else
    //    return default_el;
}

template <typename T>
list_iterator My_list<T>::begin()
{
    list_iterator ret = {head};

    //if (verify() != LIST_OK)
    //    return ret;
    
    //ret.it = head;

    return ret;
}

template <typename T>
list_iterator My_list<T>::end()
{
    list_iterator ret = {-1};

    //if (verify() != LIST_OK)
    //    return ret;
    
    ret.it = data[head].prev;
    return ret;
}

template <typename T>
void My_list<T>::iter_increase(list_iterator &iter)
{    
    //if (verify() != LIST_OK)
    //    return;
    
    //if (is_iterator_valid(iter))
        iter.it = data[iter.it].next;    
}

template <typename T>
void My_list<T>::iter_decrease(list_iterator &iter)
{    
    //if (verify() != LIST_OK)
    //    return;
        
    //if (is_iterator_valid(iter))
        iter.it = data[iter.it].prev;
}
