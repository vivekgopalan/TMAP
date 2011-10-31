/* Copyright (C) 2010 Ion Torrent Systems, Inc. All Rights Reserved */
#ifndef TMAP_MAP_DRIVER_H
#define TMAP_MAP_DRIVER_H

#include <sys/types.h>
#include "../index/tmap_index.h"

#ifdef HAVE_LIBPTHREAD
#define TMAP_MAP_DRIVER_THREAD_BLOCK_SIZE 512
#endif

/*!
  This function will be invoked after reading in all the reference data
  to initialize any program options and print messages.
  @param  data    the program persistent data
  @param  refseq  the reference sequence
  @param  opt     the program options
  @return         0 upon success, non-zero otherwise
 */
typedef int32_t (*tmap_map_driver_func_init)(void **data, tmap_refseq_t *refseq, tmap_map_opt_t *opt);

/*!
  This function will be invoked before a thread begins process its sequences.
  @param  data  the thread persistent data
  @param  opt   the program options
  @return       0 upon success, non-zero otherwise
 */
typedef int32_t (*tmap_map_driver_func_thread_init)(void **data, tmap_map_opt_t *opt);

/*!
  This function will be invoked to map a sequence.
  @param  data    the thread persistent data
  @param  seq     the sequence to map
  @param  index   the reference index
  @param  bwt     the bwt structure
  @param  sa      the sa structure
  @param  stat    the driver statistics (for mapall only)
  @param  rand    the random number generator
  @param  opt     the program options
  @return         the mappings upon success, NULL otherwise
 */
typedef tmap_map_sams_t* (*tmap_map_driver_func_thread_map)(void **data, tmap_seq_t *seq, tmap_index_t *index, tmap_map_stats_t *stat, tmap_rand_t *rand, tmap_map_opt_t *opt);

/*!
  This function will be invoked to give a mapping quality to a set of mappings
  @param  sams     the mappings
  @param  seq_len  the sequence length
  @param  opt      the program options
  @return          0 upon success, non-zero otherwise
  */
typedef int32_t (*tmap_map_driver_func_mapq)(tmap_map_sams_t *sams, int32_t seq_len, tmap_map_opt_t *opt);

/*!
  This function will be invoked after a thread has process all of its sequences.
  @param  data  the thread persistent data
  @param  opt   the program options
  @return       0 upon success, non-zero otherwise
  */
typedef int32_t (*tmap_map_driver_func_thread_cleanup)(void **data, tmap_map_opt_t *opt);

/*!
  This function will be invoked after all threads and reads have been processed.
  @param  data  the program persistent data
  @return       0 upon success, non-zero otherwise
 */
typedef int32_t (*tmap_map_driver_func_cleanup)(void **data);

/*!
  TODO
  */
typedef struct {
    tmap_map_driver_func_init func_init; /*!< this function will be run once per program to initialize persistent data across the program */
    tmap_map_driver_func_thread_init func_thread_init; /*!< this function will be run once per thread to initialize persistent data across that thread */
    tmap_map_driver_func_thread_map func_thread_map; /*!< this function will be run once per thread per input sequence to map the sequence */
    tmap_map_driver_func_mapq func_mapq; /*!< this function will be run to calculate the mapping quality */
    tmap_map_driver_func_thread_cleanup func_thread_cleanup; /*!< this function will be run once per thread to cleanup/destroy any persistent data across that thread */
    tmap_map_driver_func_cleanup func_cleanup; /*!< this function will be run once per program to cleanup/destroy any persistent data across the program */
    tmap_map_opt_t *opt; /*!< the program options specific to this algorithm */
    void *data; /*!< persistent data across the program */
} tmap_map_driver_algorithm_t;

/*!
  The mapping driver object.
  */
typedef struct {
    tmap_map_driver_algorithm_t **algorithms; /*!< the algorithms to run */
    void **data; /*< the program persistent data for each algorithm */
    int32_t num_algorithms; /*!< the number of algorithms to run */
    tmap_map_opt_t *opt; /*!< the global mapping options */
} tmap_map_driver_t;

tmap_map_driver_t*
tmap_map_driver_init(tmap_map_opt_t *opt);

void
tmap_map_driver_add(tmap_map_driver_t *driver,
                    tmap_map_driver_func_init func_init,
                    tmap_map_driver_func_thread_init func_thread_init,
                    tmap_map_driver_func_thread_map func_thread_map,
                    tmap_map_driver_func_mapq func_mapq,
                    tmap_map_driver_func_thread_cleanup func_thread_cleanup,
                    tmap_map_driver_func_cleanup func_cleanup,
                    tmap_map_opt_t *opt);

void
tmap_map_driver_run(tmap_map_driver_t *driver);

void
tmap_map_driver_destroy(tmap_map_driver_t *driver);


// TODO: below

/*! 
  data to be passed to a thread                         
  */
typedef struct {                                            
    int32_t num_ends;  /*!< the number of mates (one for fragments) */
    tmap_seq_t ***seq_buffer;  /*!< the buffers of sequences */    
    int32_t seq_buffer_length;  /*!< the buffers length */
    tmap_map_record_t **records;  /*!< the alignments for each sequence */
    tmap_index_t *index;  /*!< pointer to the reference index */
    tmap_map_driver_t *driver;  /*!< the main driver object */
    tmap_map_stats_t *stat; /*!< the driver statistics */
    tmap_rand_t *rand;  /*!< the random number generator */
    int32_t tid;  /*!< the zero-based thread id */
} tmap_map_driver_thread_data_t;

/*!
  The core worker routine of mapall
  @param  num_ends             the number of ends
  @param  seq_buffer           the buffer of sequences
  @param  sams                 the sams to return
  @param  seq_buffer_length    the number of sequences in the buffer
  @param  index                the reference index
  @param  bwt                  the BWT indices (forward/reverse)
  @param  sa                   the SA (forward/reverse)
  @param  func_thread_init     the thread initialization function
  @param  func_thread_map      the thread map function
  @param  func_mapq            the mapping quality function
  @param  func_thread_cleanup  the thread cleanup function
  @param  stat                 the driver statistics
  @param  rand                 the random number generator
  @param  tid                  the thread ids
  @param  opt                  the program parameters 
 */
void
tmap_map_driver_core_worker(int32_t num_ends, tmap_seq_t **seq_buffer[2], tmap_map_sams_t **sams[2], int32_t seq_buffer_length,
                         tmap_index_t *index,
                         tmap_map_driver_func_thread_init func_thread_init, 
                         tmap_map_driver_func_thread_map func_thread_map, 
                         tmap_map_driver_func_mapq func_mapq,
                         tmap_map_driver_func_thread_cleanup func_thread_cleanup,
                         tmap_map_stats_t* stat,
                         tmap_rand_t *rand,
                         int32_t tid, tmap_map_opt_t *opt);

/*!
 A wrapper around the core function of mapall
 @param  arg  the worker arguments in the type: tmap_map_driver_thread_data_t
 @return      the worker arguments
 */
void *
tmap_map_driver_core_thread_worker(void *arg);

/*!
  the core routine for mapping data with or without threads
  @param  func_init            the mapping algorithm initialization function
  @param  func_thread_init     the thread initialization function
  @param  func_thread_map      the thread map function
  @param  func_mapq            the mapq function (optional, but the mapq must be set in another function)
  @param  func_thread_cleanup  the thread cleanup function
  @param  opt                  the program parameters 
  */
void
tmap_map_driver_core(tmap_map_driver_func_init func_init,
                  tmap_map_driver_func_thread_init func_thread_init, 
                  tmap_map_driver_func_thread_map func_thread_map, 
                  tmap_map_driver_func_mapq func_mapq,
                  tmap_map_driver_func_thread_cleanup func_thread_cleanup,
                  tmap_map_opt_t *opt);

#endif 
