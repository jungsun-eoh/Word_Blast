


/**************************************************************
*
* Class: CSC-415-0# Spring 2020
* Name: Jungsun Eoh
* Student ID: 918590990
* Project: Assignment 2 – Word Blast
* File: main.c
*
* Description: a program that returns number of all the words
*              that are 6 or more characters long, using multiple threads.
*
**************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef DEBUG
#define DEBUG_TEST 1
#else
#define DEBUG_TEST 0
#endif
// easy to change spec in one place
#define MINCHARS 6
#define TOP 10
#define SIZEBLOCK 25000

#define debug_print(fmt, ...) \
            do { if(DEBUG_TEST) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

char * delim = "\"\'.""''?:;-,—*($%)! \t\n\x0A\r";
char* fileName = "NOFILE";
int numberOfThread = 1;

typedef struct wordFrequency {
    char* word;
    int frequency;
} wordFrequency ,*wordFrequency_p;

volatile wordFrequency_p wordTable = NULL;     //array of target word
volatile int wordListCount = 0;                 // number of words in the array
int maxWordListSize = 0;                        // size of the array
pthread_mutex_t lock;

void* threadWorking(void *);
void add_word(char* w);
long getTaskSize(void);

int main(int argc, char *argv[])
{
    int *index;
    pthread_t* threadPT;

    // initialize global var inside the main. so easy to clean
    if(pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init error\n");
        return 1;
    }

    if(argc > 2)
    {
        fileName = argv[1];
        numberOfThread = atoi(argv[2]);
    }
    else
    {
        printf("argument error: %s filename ThreadCountYouWant\n", argv[0]);
        return (-1);
    }



    struct timespec startTime;
    struct timespec endTime;
    clock_gettime(CLOCK_REALTIME, &startTime);

    // Allocate thread array info. // keep the malloc in main so it could be easy to free.
    index = malloc(sizeof(int) * numberOfThread);    // array of task information for each thread
    threadPT = malloc(sizeof(pthread_t) * numberOfThread);  // array of thread doing the task

    // getting a taskSize.


    for(int i =0; i<numberOfThread; i++)
    {
        //creadte all the thread and wait till the end from diffrent loop.
        index[i]= i;
        pthread_create( &(threadPT[i]), NULL, threadWorking, &(index[i]));
    }

    // wait for threads to finish
    for(int i =0; i<numberOfThread; i++) {
        pthread_join(threadPT[i], NULL);    // tell me when you done
    }

    free(index);
    free(threadPT);

    wordFrequency temp;
    temp.frequency = 0;
    temp.word = 0;


    int maxIndex = 0;

    for (int i = 0; i < TOP; i++ )
    {
        maxIndex = i;
        for (int j = i+1; j < wordListCount; j++) {
            if (wordTable[j].frequency > wordTable[maxIndex].frequency)
            {
                maxIndex = j;
            }
        }

        temp.frequency = wordTable[maxIndex].frequency;
        temp.word = wordTable[maxIndex].word;

        wordTable[maxIndex].frequency = wordTable[i].frequency;
        wordTable[maxIndex].word = wordTable[i].word;

        wordTable[i].frequency = temp.frequency;
        wordTable[i].word = temp.word;

        temp.frequency = 0;
        temp.word = 0;
    }


    printf("\n Word frequency count on %s with %d threads\n", fileName, numberOfThread);
    printf("printing top %d words %d char or more.\n", TOP, MINCHARS);

    for(int a = 0; a < TOP; a++)
    {
        printf (" %d: %s,  with a count of %d \n", a+1, wordTable[a].word, wordTable[a].frequency);
    }

    clock_gettime(CLOCK_REALTIME, &endTime);
    time_t sec = endTime.tv_sec - startTime.tv_sec;
    long n_sec = endTime.tv_nsec - startTime.tv_nsec;
    if (endTime.tv_nsec < startTime.tv_nsec)
    {
        --sec;
        n_sec = n_sec + 1000000000L;
    }
    printf("Total Time was %ld.%09ld seconds \n", sec, n_sec);

    for (int k = 0; k < wordListCount; k++)
    {
        free(wordTable[k].word);
        wordTable[k].word = NULL;
    }

    free(wordTable);
    wordTable = NULL;

    pthread_mutex_destroy(&lock);

}

long getTaskSize(void)
{
    int fileDescripter;
    long fileLen;

    fileDescripter = open(fileName, O_RDONLY);

    if (fileDescripter == 0)
    {
        printf("File open error\n");
        return (-1);
    }

    lseek(fileDescripter, 0, SEEK_END);
    fileLen = lseek(fileDescripter, 0, SEEK_CUR);
    lseek(fileDescripter, 0, SEEK_SET); // reset the file cursor to the top again
    close(fileDescripter);

    return fileLen;
}

void* threadWorking(void * arg)
{
    // thread created in here.
    // unlike fork, thread execute here. not next line
    char *buf;
    int fileDescripter;

    long taskSize, fileLen;
    long start, fin;
    long result;

    int index_i = *(int *) arg;

    fileLen = getTaskSize();
    taskSize = fileLen/numberOfThread;

    if(index_i == numberOfThread-1)
    {   taskSize = fileLen - (taskSize * index_i);  }
    else
    {    taskSize = taskSize; }

    buf = malloc(taskSize + 4); // extra space for wide null
    if(buf == NULL)// buffer error indicator
    {
        printf("buffer error inside threadWorking\n");
        return NULL;
    }
    // open the target file for read-only mode.
    fileDescripter = open(fileName, O_RDONLY);
    if(fileDescripter==0)
    {
        printf("read file error inside threadWorking\n");
        free(buf);  // every malloc should clean after uses.
        return NULL;
    }

    // read amount of task from main and stroe it to buf
    //because we open the file read-only, we don't need mutex for race condition.
    lseek(fileDescripter, index_i * taskSize, SEEK_SET);
    result = read(fileDescripter, buf, taskSize);
    // don't need mutex because if all thread is creating open file in diffrence place.
    // not stepping another

    debug_print("On %d, asked for %ld bytes, got %ld bytes\n", index_i +1, taskSize, result);
    close(fileDescripter);

    //put wide null for saftey
    buf[taskSize] = 0;
    buf[taskSize+1] = 0;

    //parsing
    char *word;
    char *saveptr;

    // chunk down the taskthread by word using strtok_r()
    word = strtok_r(buf, delim, &saveptr);

    while (word != NULL)
    {
        if (strlen(word) >= MINCHARS)
        {
            add_word(word);
            debug_print("%d word : %s\n", index_i, word);
        }
        // need one more, to catch subsequence word
        word = strtok_r(NULL, delim, &saveptr);
    }
    free(buf);
    return NULL;
}

void add_word(char* w) {
    //make array of word%count
    // is the word already in the list?
    for(int i =0; i < wordListCount; i ++)        // find the word in the table
    {
        if (strcasecmp(wordTable[i].word, w) == 0)
            // if word and w have no difference with.
            // if so, increment the count by 1
        {
            pthread_mutex_lock(&lock);
            wordTable[i].frequency = wordTable[i].frequency +1;
            pthread_mutex_unlock(&lock);
            return;
        }
    }
    // if not, add the word to the list and set its count to 1
    // case word not found

    //debug_print("Adding new word : %s\n", w);

    pthread_mutex_lock(&lock);

    // Do we room for it?
    if(wordListCount >= maxWordListSize)
        // if we don't have enough room for the table, we reallocate malloc.
    {
        maxWordListSize= maxWordListSize + SIZEBLOCK;

        if(wordTable == NULL)  // first input.
        {
            wordTable = malloc(sizeof(wordFrequency) * maxWordListSize);
            if (wordTable == NULL) {
                printf("first init malloc error in addWord\n");
                exit(-2);
            }
            // w is char pointer, and pointer could change it. we have to make new heap memory to store
            // word to always have valid value.
        }
        else
        {
            wordFrequency_p reallocRet = realloc (wordTable, sizeof(wordFrequency) * maxWordListSize);
            if (reallocRet == NULL)
            {
                printf("realloc malloc error in addWord\n");
                exit(-2);
            }
            wordTable = reallocRet;
        }
    }
    // add the word to the list

    wordTable[wordListCount].word = malloc(strlen(w) + 2);

    if (wordTable[wordListCount].word == NULL) {
        printf("word malloc error in addWord\n");
        exit(-2);
    }
    strcpy(wordTable[wordListCount].word, w);  //copy the word into wordTable
    wordTable[wordListCount].frequency = 1;
    ++wordListCount;

    pthread_mutex_unlock(&lock);

    return;
}
