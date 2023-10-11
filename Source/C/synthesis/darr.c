/**
 * \file 		darr.c
 * \brief 		define darr_alloc, darr_write, darr_free
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
 * This file is part of darr.
 *
 * Author: 		Stefano MAGRINI ALUNNO <stefanomagrini99@gmail.com>
 */



/**********************/
/*!< included headers */
/**********************/

#include "darr.h"
#include <stdio.h>


/***********************/
/*!< MACRO definitions */
/***********************/

/**
 * \brief 			copy data
 * \param[in]       dest: destination reference
 * \param[in]       src: source reference
 * \param[in] 		size: num of byte
 * \hideinitializer
 */
#define CPY(dest, src, size) 							\
do { 													\
	size_t __size = (size); 							\
	char *__dest = (dest), *__src = (src); 				\
	do { 												\
		*__dest++ = *__src++; 							\
	} while (--__size > 0); 							\
} while (0)


/***************/
/*!< variables */
/***************/

const darr_t empty_vec = {
	.array = NULL,
	.size = 0,
};


/******************************/
/*!< function implementations */
/******************************/

/**
 * \brief 	    allocation of a darr_t.
 * \param[in] 	len: initial size
 * \param[out] 	dv: allocated darr_t
 */
darr_t
darr_alloc(size_t len)
{
	if (len == 0) {
		return empty_vec;
	} else {
		register size_t size = ((len-(size_t)1) & (~(size_t)7)) + (size_t)8;
		darr_t dv = {
			.array = malloc(size*sizeof(char)),
			.size = size,
		};
		return dv;
	}
}

/**
 * \brief 	    add bytes to the darr_t
 * \param[in] 	data: data reference
 * \param[in] 	data_len: num of bytes
 * \param[in] 	index: destination index
 * \param[in] 	dv: darr_t
 */
int
darr_write(void* const data, size_t data_len, size_t index, darr_t * dv)
{
	size_t new_len = index + data_len;

	while (dv->size && new_len > dv->size) {
		dv->size <<= 2;
	}
	if (!(dv->size)) {
		if (dv->array) {
			return 1;
		}
		*dv = darr_alloc(new_len);
	}
	dv->array = realloc(dv->array, dv->size);
	CPY(dv->array + index, data, data_len);
	return 0;
}

/**
 * \brief 	    free dynamic array
 * \param[in] 	dv: dynamic array to free
 */
void
darr_free(darr_t dv)
{
	free(dv.array);
}
