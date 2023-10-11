/**
 * \file            darr.h
 * \brief           Implementation of a dynamic array
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
 * Author:          Stefano MAGRINI ALUNNO <stefanomagrini99@gmail.com>
 */



#ifndef DYN_ARRAY_H
#define DYN_ARRAY_H


/**********************/
/*!< included headers */
/**********************/

#include <stdlib.h>


/***********************/
/*!< types definitions */
/***********************/

/**
 * \brief 		darr_t
 * \note		Implementation of a dynamic array.
*/
typedef struct
{
	void* array; 	/*!< reference to data */
	size_t size; 	/*!< num of allocated bytes */
} darr_t;


/****************************/
/*!< function and variables */
/****************************/

extern const darr_t empty_vec; 	/*!< empty darr_t */

darr_t 	darr_alloc(size_t);
int 	darr_write(void* const, size_t, size_t, darr_t*);
void 	darr_free(darr_t);

#endif /* guard */
