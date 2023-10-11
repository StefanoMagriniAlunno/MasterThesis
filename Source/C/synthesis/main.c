/**
 * \file 		main.c
 * \brief 		Define the main function
 */



/**********************/
/*!< included headers */
/**********************/

#include "../config.h"
#include "sort.h"
#include "darr.h"
#include "select.h"
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


/***********************/
/*!< MACRO definitions */
/***********************/

#define input_file (argv[1]) /* input_file */
#define ERRSTR_LEN 256 /* max length of error string */
#define IMAG_FORMAT (".ppm") /* images format */
#define BIN_FORMAT (".bin") /* synthesis format */


/**********************/
/*!< types definition */
/**********************/

/**
 * \brief 		pool_t
 * \note		This structure is used to implement a pool of processes.
*/
typedef struct
{
	pthread_mutex_t 	mutex; 	        /*!< mutex used as a light */
	char** 	            directories; 	/*!< list of directories */
	int32_t 	        index, 	        /*!< next input */
		                count; 	        /*!< num of directories */
} pool_t;

/**
 * \brief		image_t
 * \note		This structure is used to implement an images.
*/
typedef struct
{
	uint8_t* 	bitboard; 	    /*!< is a vector with data of pixels */
	int32_t 	width, height; 	/*!< image shape */
} image_t;


/****************************/
/*!< function and variables */
/****************************/

pool_t 	            main_list; 	                            /*!< pool of processes */
pthread_t 	        threads[THREAD_COUNT]; 	                /*!< vector of threads */
pthread_mutex_t 	error_mutex; 	                        /*!< mutex used to coordinate error reporting.  */
uint8_t 	        flag; 	                                /*!< if flag is 0 the pool of processes stops */
char 	            source_directory[FILENAME_MAX]; 	    /*!< directory of the set folder */
char 	            destination_directory[FILENAME_MAX]; 	/*!< directory of the synthesis folder */
char 	            buffer[8]; 	                            /*!< buffer used to save the format images */

#if MODEL == 0
int 	std_cmp(const void*, const void*, void*);
int 	cmp(const void*, const void*, void*);
#endif /* MODEL == 0 */
int 	synth(char*);
void* 	activation(void*);
int 	main(int, char**);


/******************************/
/*!< function implementations */
/******************************/

#if MODEL == 0
/**
 * \brief 	    standard comparison between two floats
 * \note 	    std_cmp <= 0 iff the 'a' <= 'b'.
 * \param[in] 	a: reference to first float
 * \param[in] 	b: reference to second float
 * \param[in] 	context: unused
 * \return 		-1: if a < b
 *              0: if a == b
 *              1: if a > b
 */
int
std_cmp(const void* a, const void* b, void* context)
{
	if (*(float*)a < *(float*)b) {
		return -1;
	} else {
		return *(float*)a > *(float*)b;
	}
}

/**
 * \brief 	    comparison between two grams, used in sort function
 * \note 	    cmp <= 0 iff the 'a' <= 'b'.
 * \param[in] 	a: reference to index int32_t
 * \param[in] 	b: reference to index int32_t
 * \param[in] 	context: reference to image_t
 * \return 		-1: if a < b
 *              0: if a == b
 *              1: if a > b
 */
int
cmp(const void* a, const void* b, void* context)
{
	image_t image = *(image_t*)context;
	int32_t i = *(int32_t*)a, j = *(int32_t*)b;
	uint8_t *curr_i = image.bitboard + i, *curr_j = image.bitboard + j;

	/* check gram existence */
	{
		int32_t raw_i = i / image.width, col_i = i % image.width, raw_j = j / image.width, col_j = j % image.width;
		if (raw_i + BW_GRAM_SIZE > image.height || col_i + BW_GRAM_SIZE > image.width) {
			return (raw_j + BW_GRAM_SIZE > image.height || col_j + BW_GRAM_SIZE > image.width) ? 0 : 1;
		} else if (raw_j + BW_GRAM_SIZE > image.height || col_j + BW_GRAM_SIZE > image.width) {
			return -1;
		}
	}

	/* compare grams */
	for (volatile int32_t r = 0; r < BW_GRAM_SIZE; r++) {
		for (volatile int32_t c = 0; c < BW_GRAM_SIZE; c++) {
			if (*curr_i == 0) {
				if (*curr_j == 1) {
					return -1;
				}
			} else if (*curr_j == 0) {
				return 1;
			}
			curr_i++;
			curr_j++;
		}
		curr_i += image.width - BW_GRAM_SIZE;
		curr_j += image.width - BW_GRAM_SIZE;
	}
	return 0;
}

#endif  /* MODEL == 0 */

/**
 * \brief 	    perform a synthesis of the image
 * \note 	    read the image, compute the synthesis, save synthesis.
 *              In the event of an error, it writes to stderr the communicating thread and error details.
 * \param[in] 	directory: image file path respect its set.
 * \return 		0: any error.
 *              1: error encountered.
 */
int
synth(char* directory)
{
	image_t my_image;
	size_t num_of_pixels;

	/* read image */
	{
		FILE* fp;
		char source_dir[FILENAME_MAX] = {'\0'};

		strcpy(source_dir, source_directory);
		strcat(source_dir, "/");
		strcat(source_dir, directory);
		strcat(source_dir, IMAG_FORMAT);
		fp = fopen(source_dir, "rb");
		if (fp == NULL) {
			pthread_mutex_lock(&error_mutex);
			{
				fflush(stderr);
				fprintf(stderr, "\t> %lu: file not found: input %s\n", (unsigned long)pthread_self(), source_dir);
			}
			pthread_mutex_unlock(&error_mutex);
		}
		if (fscanf(fp, "P6\n%d %d\n255", &my_image.width, &my_image.height) != 2) {
			pthread_mutex_lock(&error_mutex);
			{
				fflush(stderr);
				fprintf(stderr, "\t> %lu: image format error\n", (unsigned long)pthread_self());
			}
			pthread_mutex_unlock(&error_mutex);
			return 1;
		}
		fgetc(fp);  // used to skip the character newline
		num_of_pixels = (size_t)my_image.width * (size_t)my_image.height;
		my_image.bitboard = calloc(3*num_of_pixels, sizeof (uint8_t));
		if (my_image.bitboard == NULL) {
			pthread_mutex_lock(&error_mutex);
			{
				fflush(stderr);
				fprintf(stderr, "\t> %lu: out of memory\n", (unsigned long)pthread_self());
			}
			pthread_mutex_unlock(&error_mutex);
			return 1;
		}
		if (fread(my_image.bitboard, sizeof (uint8_t), 3*num_of_pixels, fp) != 3*num_of_pixels) {
			pthread_mutex_lock(&error_mutex);
			{
				fflush(stderr);
				fprintf(stderr, "\t> %lu: pixels reading error\n", (unsigned long)pthread_self());
			}
			pthread_mutex_unlock(&error_mutex);
			return 1;
		}
		fclose(fp);
	}

#if MODEL == 0
	/* Optimisation: compression to bw bitboard */
	{
		float *bright = calloc(num_of_pixels, sizeof (float)), *cpy_bright = calloc(num_of_pixels, sizeof (float));
		float median_bright;

		if (bright == NULL) {
			pthread_mutex_lock(&error_mutex);
			{
				fflush(stderr);
				fprintf(stderr, "\t> %lu: out of memory\n", (unsigned long)pthread_self());
			}
			pthread_mutex_unlock(&error_mutex);
			return 1;
		}
		if (cpy_bright == NULL) {
			pthread_mutex_lock(&error_mutex);
			{
				fflush(stderr);
				fprintf(stderr, "\t> %lu: out of memory\n", (unsigned long)pthread_self());
			}
			pthread_mutex_unlock(&error_mutex);
			return 1;
		}
		for (size_t bit_index = 0; bit_index < num_of_pixels; ++bit_index) {
			uint8_t r = my_image.bitboard[3*bit_index],
				g = my_image.bitboard[3*bit_index + 1],
				b = my_image.bitboard[3*bit_index + 2],
				min = r < g ? (r < b ? r : b) : (g < b ? g : b),
				max = r >= g ? (r >= b ? r : b) : (g >= b ? g : b);
			bright[bit_index] = ((float)min/255 + (float)max/255)/2;
		}
		memcpy(cpy_bright, bright, num_of_pixels*sizeof (float));
		median_bright = *(float*)select(cpy_bright, num_of_pixels, sizeof (float), num_of_pixels/2, std_cmp, NULL);
		for (size_t bit_index = 0; bit_index < num_of_pixels; ++bit_index)
			my_image.bitboard[bit_index] = (uint8_t)(bright[bit_index] >= median_bright);
		free(cpy_bright);
		free(bright);
	}

	my_image.bitboard = realloc(my_image.bitboard, num_of_pixels * sizeof (uint8_t));

	/* perform analysis on my_image.bitboard*/
	{
		darr_t my_list = empty_vec;
		size_t* index_matrix;
		uint32_t* recurrence;
		float* recurrence_map;
		uint32_t size_list = 0;

		/* build matrix of indices */
		index_matrix = calloc(num_of_pixels, sizeof (size_t));
		if (index_matrix == NULL) {
			pthread_mutex_lock(&error_mutex);
			{
				fflush(stderr);
				fprintf(stderr, "\t> %lu: out of memory\n", (unsigned long)pthread_self());
			}
			pthread_mutex_unlock(&error_mutex);
			return 1;
		}
		for (size_t i = 0; i < num_of_pixels; ++i)
			index_matrix[i] = i;

		/* sort used to sort the matrix of indices */
		sort(index_matrix, num_of_pixels, sizeof (size_t), cmp, &my_image);

		/* make list of data */
		{
			size_t i = 0;
			int32_t max_num_of_grams = (my_image.width-1+BW_GRAM_SIZE)*(my_image.height-1+BW_GRAM_SIZE);
			recurrence = calloc(max_num_of_grams, sizeof (int32_t));
			if (recurrence == NULL) {
				pthread_mutex_lock(&error_mutex);
				{
					fflush(stderr);
					fprintf(stderr, "\t> %lu: out of memory\n", (unsigned long)pthread_self());
				}
				pthread_mutex_unlock(&error_mutex);
				return 1;
			}
			while (i < num_of_pixels) {
				uint8_t* curr;
				size_t j = i, index = index_matrix[i];
				int32_t counter = 1;

				curr = my_image.bitboard + index;

				/* check the index, if it is close the margin the algorithm ends */
				if (index / (size_t)my_image.width + BW_GRAM_SIZE > my_image.height ||
					index % (size_t)my_image.width + BW_GRAM_SIZE > my_image.width) {
					break;
				}

				/* the gram exists, so it is pushed on the list */
				{
					for (int32_t raw = 0; raw < BW_GRAM_SIZE; ++raw)
						if (darr_write(
								curr + raw*my_image.width,
								BW_GRAM_SIZE * sizeof (uint8_t),
								BW_GRAM_SIZE*BW_GRAM_SIZE * size_list + raw*BW_GRAM_SIZE,
								&my_list)) {
							pthread_mutex_lock(&error_mutex);
							{
								fflush(stderr);
								fprintf(stderr, "\t> %lu: write error on the dynamic array\n", (unsigned long)pthread_self());
							}
							pthread_mutex_unlock(&error_mutex);
							return 1;
						}
				}

				/* compare each gram with the current one until find a different one */
				{
					while (j + 1 < num_of_pixels) {
						++j;
						if (cmp(index_matrix+i, index_matrix+j, &my_image)) {
							recurrence[size_list] = counter;
							i = j; // pass at next iteration
							break;
						} else {
							++counter;
						}
					}
					if (j + 1 == num_of_pixels) { // no break
						recurrence[size_list] = counter;
						i = j + 1; // finish writing
					}
				}
				++size_list;
			}
		}

		/* make a matrix with float values */
		recurrence_map = calloc(num_of_pixels, sizeof (float));
		if (recurrence_map == NULL) {
			pthread_mutex_lock(&error_mutex);
			{
				fflush(stderr);
				fprintf(stderr, "\t> %lu: out of memory\n", (unsigned long)pthread_self());
			}
			pthread_mutex_unlock(&error_mutex);
			return 1;
		}
		{
			size_t* curr_index = index_matrix;

			for (int32_t i = 0; i < size_list; ++i) {
				int32_t curr_ric = recurrence[i];
				for (volatile int32_t j = 0; j < curr_ric; ++j)
					recurrence_map[*(curr_index++)] = 1./curr_ric;
			}
		}

		/* write on file grams and occurrence */
		{
			FILE* fp;
			char binary_dir[FILENAME_MAX] = {'\0'};

			strcpy(binary_dir, destination_directory);
			strcat(binary_dir, "/");
			strcat(binary_dir, directory);
			strcat(binary_dir, BIN_FORMAT);
			fp = fopen(binary_dir, "wb");
			if (fp == NULL) {
				pthread_mutex_lock(&error_mutex);
				{
					fflush(stderr);
					fprintf(stderr, "\t> %lu: file not found: input %s\n", (unsigned long)pthread_self(), binary_dir);
				}
				pthread_mutex_unlock(&error_mutex);
			}

			{
				int32_t s = BW_GRAM_SIZE;

				fwrite(&s, sizeof (int32_t), 1, fp);
			}

			fwrite(&size_list, sizeof (int32_t), 1, fp);

			fwrite(my_list.array, sizeof (uint8_t), size_list*BW_GRAM_SIZE*BW_GRAM_SIZE, fp);
			fwrite(recurrence, sizeof (int32_t), size_list, fp);

			fwrite(&my_image.width, sizeof (int32_t), 1, fp);
			fwrite(&my_image.height, sizeof (int32_t), 1, fp);
			fwrite(recurrence_map, sizeof (float), num_of_pixels, fp);

			fwrite(my_image.bitboard, sizeof (uint8_t), num_of_pixels, fp);
			fclose(fp);
		}

		free(recurrence_map);
		free(recurrence);
		free(index_matrix);
		darr_free(my_list);
	}
#endif  /* defined(BW_GRAM) */

	free(my_image.bitboard);

	return 0;
}

/**
 * \brief 	    activation function of the pool
 * \note 	    extract directory that will be pass at the synth function.
 *              In the event of an error, it writes to stderr the communicating thread and error details.
 * \param[in] 	addr: unused
 * \return 		'NULL'
 */
void*
activation(void* addr)
{
	while (flag) {
		int32_t index, output;

		/* pop next index */
		{
			pthread_mutex_lock(&main_list.mutex);
			{
				if (main_list.index < main_list.count) {
					index = main_list.index;

					#if PROGRESS == 1
						float prog = 100.*(float)(main_list.index+1)/main_list.count;
						fflush(stdout);
						printf("\033[A");
						printf("\tprogress: %.2f%%\n", prog);
					#endif /* PROGRESS == 1*/

					++main_list.index;
				} else {
					index = main_list.count;
				}
			}
			pthread_mutex_unlock(&main_list.mutex);
		}

		/* end of pool */
		if (index == main_list.count) {
			break;
		}

		/* synthesis */
		output = synth(main_list.directories[index]);

		/* error check */
		if(output) {
			pthread_mutex_lock(&error_mutex);
			{
				flag = false;
				fflush(stderr);
				fprintf(stderr, "\t> %lu: %s not synthesized\n", (unsigned long)pthread_self(), main_list.directories[index]);
			}
			pthread_mutex_unlock(&error_mutex);
			break;
		}
	}
	return NULL;
}


/*******************/
/*!< main function */
/*******************/

/**
 * \brief 	    main
 * \note 	    init the pool of processed and start it.
 * \param[in] 	argc: is 2
 * \param[in] 	argv[0]: current executable name
 *              argv[1]: input_file name
 * \return 		'EXIT_SUCCESS': any error
 *              'EXIT_FAILURE': error encountered
 */
int
main(int argc, char** argv)
{
	if(argc != 2) return EXIT_FAILURE;

	/* init main_list & flag */
	{
		FILE* fp = fopen(input_file, "r");

		fscanf(fp, "%s ", source_directory);  // nota: insert a space avoid reading of \n
		fscanf(fp, "%s ", destination_directory);
		fscanf(fp, "%d ", &main_list.count);
		main_list.directories = calloc(main_list.count, sizeof (char*));
		for (int32_t i = 0; i < main_list.count; ++i) {
			main_list.directories[i] = calloc(FILENAME_MAX, sizeof (char));
			fscanf(fp, "%[^.]%s ", main_list.directories[i], buffer);
		}
		pthread_mutex_init(&main_list.mutex, NULL);
		flag = true;
		fclose(fp);

		#if PROGRESS == 1
			printf("<Subprocess>\n");
			printf("\tpool: %ld processes, %ld input\n\n", THREAD_COUNT, main_list.count);
		#endif /* PROGRESS == 1 */
	}

	/* pool of processes */
	for (int32_t i = 0; i < THREAD_COUNT; ++i)
		pthread_create(&threads[i], NULL, activation, NULL);

	for (int32_t i = 0; i < THREAD_COUNT; ++i)
		pthread_join(threads[i], NULL);

	free(main_list.directories);
	pthread_mutex_destroy(&(main_list.mutex));

	return flag ? EXIT_SUCCESS : EXIT_FAILURE;
}
