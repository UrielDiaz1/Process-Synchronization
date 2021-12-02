#include <stdio.h>
#include <stdlib.h>
#include "buffer.h"
#include <pthread.h>
#include <semaphore.h>

buffer_item buffer[BUFFER_SIZE];
int NUM_PRODUCERS;
int NUM_CONSUMERS;
int NUM_TO_PRODUCE;
int count = 0;    // Keeps track of current items produced.
int element = 0;  // Keeps track of end of queue.
int seedp, seedc;

int remainingProducers; // Keeps track of active producer threads.
int remainingConsumers; // Keeps track of active consumer threads.

pthread_mutex_t mutex;
sem_t empty;
sem_t full;

void *producer(void *param);
void *consumer(void *param);
int insert_item(buffer_item item);
int remove_item(buffer_item *item);

/* Producer threads entry function. */
void *producer(void *param) {
   buffer_item item;
   
   // Retrieves thread's index from passed argument.
   int index = *((int *) param);
   free(param);

   while(count < NUM_TO_PRODUCE) {
      // Sleeps between 1 to 3000 milliseconds. 
      usleep(rand_r(&seedp) % (3000000 + 1 - 1000) + 1000);
      
      sem_wait(&empty);
      pthread_mutex_lock(&mutex);
      
      if(count < NUM_TO_PRODUCE && element <= BUFFER_SIZE) {
         
         // Produces item with random value.
         item = rand_r(&seedp) % 1000;
         
         if(insert_item(item)) {
            fprintf(stderr, "Producing out of bounds.\n");
            exit(1);
         }
         else {
            printf("Producer %d produced %d\n", index, item);
         }
      }
      
      pthread_mutex_unlock(&mutex);
      sem_post(&full);
   }
   
   // Deadlock prevention.
   remainingProducers--;
   int i;
   for(i = 0; i < remainingConsumers; i++) {
      sem_post(&full);
   }
}

/* Consumer threads entry function. */
void *consumer(void *param) {
   buffer_item item;
   
   // Retrieves thread's index from passed argument.
   int index = *((int *) param);
   free(param);
   
   while(1) {
      // Sleeps between 1 to 3000 milliseconds.
      usleep(rand_r(&seedc) % (3000000 + 1 - 1000) + 1000);
      
      sem_wait(&full);
      pthread_mutex_lock(&mutex);
      
      // Checks if there's an item in the queue to consume.
      if(buffer[0] > 0) {
         if(remove_item(&item)) {
            fprintf(stderr, "Consuming out of bounds.\n");
            exit(1);
         }
         else {
            printf("Consumer %d consumed %d\n", index, item);
         }
      }
      
      pthread_mutex_unlock(&mutex);
      sem_post(&empty);
      
      // Ensures that all items are consumed before exiting.
      if(count >= NUM_TO_PRODUCE && buffer[0] == 0) {
         break;
      }
   }
   
   // Deadlock prevention.
   remainingConsumers--;
   int i;
   for(i = 0; i < remainingProducers; i++) {
      sem_post(&empty);
   }
}

/* For producer threads use. */
int insert_item(buffer_item item) {
   if(element < 0 || element > BUFFER_SIZE) {
      return -1;
   }
   
   // Produces item.
   buffer[element] = item;
   element++;
   count++;
   
   return 0;
}

/* For consumer threads use. */
int remove_item(buffer_item *item) {
   if(element < 0 || element > BUFFER_SIZE) {
      return -1;
   }
   
   // Consumes item, and moves everything one spot ahead in
   // the queue.
   *item = buffer[0];
   int i;
   for(i = 0; i < BUFFER_SIZE; i++) {
      buffer[i] = buffer[i + 1];
   }
   buffer[BUFFER_SIZE] = 0;
   
   element--;
   return 0;
}

int main(int argc, char *argv[]) {
   int i;
   
   // Get command line arguments.
   if(argc != 4) {
      fprintf(stderr, "Usage: ./psynch [#producers] [#consumers] [#items]\n");
      exit(1);  
   }
   NUM_PRODUCERS  = atoi(argv[1]);
   NUM_CONSUMERS  = atoi(argv[2]);
   NUM_TO_PRODUCE = atoi(argv[3]);
   
   if(NUM_PRODUCERS <= 0 || NUM_CONSUMERS <= 0) {
      fprintf(stderr, "Number of threads must be greater than 0.\n");
      exit(1);
   }
   
   // Deadlock prevention setup.
   remainingProducers = NUM_PRODUCERS;
   remainingConsumers = NUM_CONSUMERS;
   
   
   // Initialize semaphores and buffer.
   for(i = 0; i < BUFFER_SIZE; i++) {
      buffer[i] = 0;
   }
   
   pthread_mutex_init(&mutex, NULL);
   sem_init(&full, 0, 0);
   sem_init(&empty, 0, BUFFER_SIZE);
   
   // Create producer threads.
   pthread_t producerThreads[NUM_PRODUCERS];
   
   for(i = 0; i < NUM_PRODUCERS; i++) {
      // Allocates memory for each thread's index to be passed as
      // an argument. This ensures each thread's index has its own
      // address.
      int *arg = malloc(sizeof(*arg));
      *arg = i + 1;
      
      if(pthread_create(&producerThreads[i], NULL, producer, arg) != 0) {
         perror("Failed to create producer thread");
      }
   }
   
   // Create consumer threads.
   pthread_t consumerThreads[NUM_CONSUMERS];

   for(i = 0; i < NUM_CONSUMERS; i++) {
      int *arg = malloc(sizeof(*arg));
      *arg = i + 1;
      
      if(pthread_create(&consumerThreads[i], NULL, consumer, arg) != 0) {
         perror("Failed to create consumer thread");
      }
   }
   
   // Wait for consumer threads to finish their tasks.
   for(i = 0; i < NUM_CONSUMERS; i++) {
      pthread_join(consumerThreads[i], NULL);
   }
   
   // Wait for producer threads to finish their tasks.
   for(i = 0; i < NUM_PRODUCERS; i++) {
      pthread_join(producerThreads[i], NULL);
   }
   
   sem_destroy(&empty);
   sem_destroy(&full);
   pthread_mutex_destroy(&mutex);
   exit(0);
}