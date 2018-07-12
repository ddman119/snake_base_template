#include "queue.h"

//Initializes queue
struct queue* queueInit(void)
{
    struct queue *q = (struct queue *)malloc(sizeof(struct queue));
    if(q == NULL)
    {
        perror("Malloc Error");
        exit(EXIT_FAILURE);
    }

    q->isEmpty = 1;
    q->isFull = q->head = q->tail = 0;
    q->mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    if(q->mutex == NULL)
    {
        perror("Malloc error pthread_mutex_t");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_init(q->mutex, NULL);

    q->notFull = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    if(q->notFull == NULL)
    {
        perror("Malloc error pthread_cond_t");
        exit(EXIT_FAILURE);   
    }
    pthread_cond_init(q->notFull, NULL);

    q->notEmpty = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    if(q->notEmpty == NULL)
    {
        perror("Malloc error pthread_cond_t");
        exit(EXIT_FAILURE);
    }
    pthread_cond_init(q->notEmpty, NULL);

    return q;
}

//Frees a queue
void queueDestroy(struct queue *q)
{
    pthread_mutex_destroy(q->mutex);
    pthread_cond_destroy(q->notFull);
    pthread_cond_destroy(q->notEmpty);
    free(q->mutex);
    free(q->notFull);
    free(q->notEmpty);
    free(q);
}

//Push to end of queue
void queuePush(struct queue *q, char* msg)
{
    q->buffer[q->tail] = msg;
    q->tail++;
    if(q->tail == MAX_BUFFER_SIZE)
        q->tail = 0;
    if(q->tail == q->head)
        q->isFull = 1;
    q->isEmpty = 0;
}

//Pop front of queue
char* queuePop(struct queue *q)
{
    char* msg = q->buffer[q->head];
    q->head++;
    if(q->head == MAX_BUFFER_SIZE)
        q->head = 0;
    if(q->head == q->tail)
        q->isEmpty = 1;
    q->isFull = 0;

    return msg;
}
