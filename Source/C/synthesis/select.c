/**
 * \file 		select.c
 * \brief 		Define select function
 */

/*
 * Copyright (c) 2023 Stefano MAGRINI ALUNNO
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of select.
 *
 * Author: 		Stefano MAGRINI ALUNNO <stefanomagrini99@gmail.com>
 */



/**********************/
/*!< included headers */
/**********************/

#include "select.h"


/***********************/
/*!< MACRO definitions */
/***********************/

/**
 * \brief 			swap data
 * \param[in]       a: first reference
 * \param[in]       b: second reference
 * \param[in] 		size: num of bytes to swap
 * \hideinitializer
 */
#define SWAP(a, b, size) 			\
do { 								\
	size_t __size = (size); 		\
	char *__a = (a), *__b = (b); 	\
	do { 							\
		char __tmp = *__a; 			\
		*__a++ = *__b; 				\
		*__b++ = __tmp; 			\
	} while (--__size > 0); 		\
}while (0)

/**
 * \brief 			copy data
 * \param[in]       dest: destination reference
 * \param[in]       src: source reference
 * \param[in] 		size: num of byte
 * \hideinitializer
 */
#define CPY(dest, src, size) 				\
do { 										\
	size_t __size = (size); 				\
	char *__dest = (dest), *__src = (src); 	\
	do { 									\
		*__dest++ = *__src++; 				\
	} while (--__size > 0); 				\
}while (0)


/*************************/
/*!< function prototypes */
/*************************/

void* 	select_pivot(void* const, size_t, size_t, int (*)(const void*, const void*, void*), void*);
void 	select_partition(void* const, void* const, size_t, size_t, int (*)(const void*, const void*, void*), void*, void** , void**);
void* 	prv__select(void* const, void* const, size_t, size_t, size_t, int (*)(const void*, const void*, void*), void*);


/******************************/
/*!< function implementations */
/******************************/

/**
 * \brief 	    compute the pivot
 * \note 	    return the median between the first, the last and the central pivot.
 * \param[in] 	vec: reference at the first component of the vector
 * \param[in] 	len: num of components
 * \param[in] 	el_size: num of bytes of a component
 * \param[in] 	cmp: comparison function
 * \param[in] 	args: reference at the context of the comparison
 * \return 		reference at the pivot.
 */
void*
select_pivot(void* const vec, size_t len, size_t el_size, int (*cmp)(const void*, const void*, void*), void* args)
{
	void* begin = vec;
	void* last = vec + (len-1)*el_size;
	void* middle = vec + (len/2)*el_size;

	if (cmp(begin, middle, args) > 0) {
		if (cmp(middle, last, args) > 0) {
			return middle;  // begin > middle > last
		} else if (cmp(begin, last, args) > 0) {
			return last;  // last >= begin > middle
		} else {
			return begin;
		}
	} else if (cmp(begin, last, args) > 0) {
		return begin;  // last < begin <= middle
	} else if (cmp(middle, last, args) > 0) {
		return last;  // middle > last >= begin
	} else {
		return middle;  // begin <= middle <= last
	}
}

/**
 * \brief 	    perform the partition of the quick_select.
 * \note 	    It's important that pivot is allocated outside of the vector.
 * \param[in] 	pivot: the pivot
 * \param[in] 	vec: reference at the first component of the vector
 * \param[in] 	len: num of components
 * \param[in] 	el_size: num of bytes of a component
 * \param[in] 	cmp: comparison function
 * \param[in] 	args: reference at the context of the comparison
 * \param[in] 	l_mid: reference at the pointer of the lower center
 * \param[in] 	u_mid: reference at the pointer of the upper center
 */
void
select_partition(void* const pivot, void* const vec, size_t len, size_t el_size, int (*cmp)(const void*, const void*, void*), void* args, void** l_mid, void** u_mid)
{
	void* lower = vec;
	void* upper = vec + (len-1)*el_size;
	void* curr = vec;

	while (curr <= upper)
	{
		register int ret = cmp(curr, pivot, args);
		if (ret > 0) // curr > pivot
		{
			SWAP(curr, upper, el_size);
			upper -= el_size;
		}
		else if (ret < 0)
		{
			SWAP(curr, lower, el_size);
			curr += el_size;
			lower += el_size;
		}
		else curr += el_size;
	}
	*l_mid = lower;  // end of the lower partition
	*u_mid = curr;  // begin of the upper partition
}

/**
 * \brief 	    perform the quick_select
 * \note 	    use a preallocated buffer.
 * \param[in] 	buf: buffer
 * \param[in] 	vec: reference at the first component of the vector
 * \param[in] 	len: num of components
 * \param[in] 	el_size: num of bytes of a component
 * \param[in] 	cmp: comparison function
 * \param[in] 	args: reference at the context of the comparison
 */
void*
prv__select(void* const buf, void* const vec, const size_t k, size_t len, size_t el_size, int (*cmp)(const void*, const void*, void*), void* args)
{
	if (len == 1) {
		return vec;
	} else {
		void *pivot, *l_mid, *u_mid;
		pivot = select_pivot(vec, len, el_size, cmp, args);
		CPY(buf, pivot, el_size);
		select_partition(buf, vec, len, el_size, cmp, args, &l_mid, &u_mid);

		{
			size_t vec_l_len = ((size_t)l_mid - (size_t)vec)/el_size;
			size_t vec_m_len = ((size_t)u_mid - (size_t)l_mid)/el_size;
			size_t vec_u_len = len - vec_m_len - vec_l_len;
			if (k <= vec_l_len) {
				return prv__select(buf, vec, k, vec_l_len, el_size, cmp, args);
			} else if (k - vec_l_len <= vec_m_len) {
				return l_mid;
			} else {
				return prv__select(buf, u_mid, k - vec_l_len - vec_m_len, vec_u_len, el_size, cmp, args);
			}
		}
	}
}

/**
 * \brief 	    call prv__select
 * \param[in] 	vec: reference at the first component of the vector
 * \param[in] 	len: num of components
 * \param[in] 	el_size: num of bytes of a component
 * \param[in] 	cmp: comparison function
 * \param[in] 	args: reference at the context of the comparison
 */
void* select(void* const vec, size_t len, size_t el_size, const size_t k, int (*cmp)(const void*, const void*, void*), void* args)
{
	char* buf = malloc(el_size);
	void* ret = prv__select(buf, vec, k, len, el_size, cmp, args);
	free(buf);
	return ret;
}
