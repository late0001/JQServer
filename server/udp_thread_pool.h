/*
 * thread_pool.h
 *
 *  Created on: 2015Äê2ÔÂ4ÈÕ
 *      Author: j8
 */

#ifndef __UDP_THREAD_POOL_H__
#define __UDP_THREAD_POOL_H__

#include "thread_pool_global.h"

#define	  	THREAD_POLL_SIZE	 	15// the thread number of thread poll
#define		BUFFER_SIZE				1024

typedef void* (*FUNC)(void* arg, int index);


typedef struct _udp_job_parameter{
	int  fd;
	struct sockaddr_in client_addr;
	char recv_buffer[1024];
}udp_job_parameter;

/**
 * define a task node
 */
typedef struct _udp_job_t{
    FUNC             		function;		// function point
//    void*                 	arg;     		// function parameter
    udp_job_parameter arg;
    time_t job_create_time;
    struct _udp_job_t* 	prev;     		// point previous note
    struct _udp_job_t* 	next;     		// point next note
}udp_job_t;

/**
 * define a job queue
 */
typedef struct _udp_job_queue{
   udp_job_t*    head;            	//queue head point
   udp_job_t*    tail;             	//queue tail point
   int              jobN;               //task number
   sem_t*           queueSem;           //queue semaphore
}udp_job_queue;

/**
 * thread pool
 */
typedef struct _udp_thpool_t{
   pthread_t*      	threads;    // point to array of thread point 
   int             	threadsN;   // thread pool number
   udp_job_queue* jobqueue;  	// job queue
}udp_thpool_t;


typedef struct _udp_thpool_thread_parameter{
	udp_thpool_t 		*thpool;
	int 			thread_index;
}udp_thpool_thread_parameter;


udp_thpool_t*  udp_thpool_init(int thread_pool_numbers);
int udp_thpool_add_work(udp_thpool_t* tp_p, void *(*func)(void* arg, int index), 
/*void *arg_p*/int event_index, int socket_fd, struct sockaddr_in sock_addr,  char *recev_buffer);

void udp_thpool_destroy(udp_thpool_t* tp_p);
int get_jobqueue_number(udp_thpool_t* tp_p);
int delete_timeout_job(udp_thpool_t* tp_p, int time_out);

#endif /* THREAD_POOL_H_ */

