/**
 * \file            config.h
 * \brief           software configuration file
 */

#define PROGRESS 1  /* See the progress of C programs */

#define MODEL 0  /* Set the model */

#if MODEL == 0  /* BW standard model */
    #define BW_GRAM_SIZE 6
#elif MODEL == 1  /* Multilayer model */
	#define N_LAYERS 2
	#define BW_GRAM_SIZE 8
#elif MODEL == 2  /* Opinion model */
    #define INTERACTION_AREA 5
    #define CONFIDENCE 0.5
    #define GRAM_SIZE 4
#endif

#define THREAD_COUNT 6  /* Set num of threads*/
