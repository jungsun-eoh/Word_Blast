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

#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <zconf.h>
#include "uthash.h"     // Copyright (c) 2003-2018, Troy D. Hanson

#ifdef DEBUG
#define DEBUG_TEST 1
#else
#define DEBUG_TEST 0
#endif

// easy to change spec in one place
#define MINCHARS 6
#define TOP 10

#define debug_print(fmt, ...) \
            do { if(DEBUG_TEST) fprintf(stderr, fmt, __VA_ARGS__); } while (0)



struct wordFrequency {
    const char* word;   // key
    int frequency;      // value
    UT_hash_handle hh;
};

typedef struct taskInfo {
    char * filename;
    int threadID;
    long start;
    long size;
} taskInfo, *taskInfo_p ;

struct wordFrequency *wordTable = NULL;     // hash table to keep word and its frequency.



void *add_word(const char* w);
/* adding word to hashtable. if the word already exists, value of the key(the word) will increment by 1.
 * So the hash table will keep tracking how many times the word appeared in the .txt file.
 */

void* threadWorking(void *);

long getTaskSize(char*, int);

int main(int argc, char *argv[])
{
    char* fileName;

    int numberOfThread;
    pthread_t* threadPT;
    taskInfo *infoArr;
    long taskSize;

    fileName = "NOFILE"; // having a default value is a good habit
    numberOfThread = 1;    // default value

    struct timespec startTime;
    struct timespec endTime;
    clock_gettime(CLOCK_REALTIME, &startTime);


    if(argc > 2)
    {
        fileName = argv[1];
        numberOfThread = atoi(argv[2]);

        taskSize = getTaskSize(fileName, numberOfThread);
    }
    else
        {
        printf("argument error: %s filename ThreadCountYouWant\n", argv[0]);
        return (-1);
        }

    infoArr = malloc(sizeof(taskInfo) * numberOfThread);
    threadPT = malloc(sizeof(pthread_t) * numberOfThread);

    for(int i =0; i<numberOfThread; i++)
    {
        infoArr[i].filename = fileName;
        infoArr[i].threadID = i+1;  // give thread ID for easy debug
        infoArr[i].start = taskSize * i;
        infoArr[i].size = taskSize;

        pthread_create(&(threadPT[i]), NULL, threadWorking, &infoArr[i]);
    }

    // wait for threads to finish
    for(int i =0; i<numberOfThread; i++) {
        pthread_join(threadPT[i], NULL);
    }

    printf("\n Word frequency count on %s with %d threads\n", fileName, numberOfThread);
    printf("printing top %d words %d char or more.\n", TOP, MINCHARS);








    clock_gettime(CLOCK_REALTIME, &endTime);
    time_t sec = endTime.tv_sec - startTime.tv_sec;
    long n_sec = endTime.tv_nsec - startTime.tv_nsec;
    if (endTime.tv_nsec < startTime.tv_nsec)
    {
        --sec;
        n_sec = n_sec + 1000000000L;
    }
    printf("Total Time was %ld.%09ld seconds \n", sec, n_sec);

    return 0;
}

long getTaskSize(char* fileName, int numberOfThread)
{
    int fileDescripter;

    long fileLen;
    long taskSize;

    // open the target file for read-only mode.
    fileDescripter = open(fileName, O_RDONLY);

    if (fileDescripter == 0) { printf("File open error\n"); }
    else
        {
        lseek(fileDescripter, 0, SEEK_END);
        fileLen = lseek(fileDescripter, 0, SEEK_CUR);
        lseek(fileDescripter, 0, SEEK_SET);
        close(fileDescripter);

        taskSize = fileLen / numberOfThread;
        //printf("Size of file: %ld bytes\n", taskSize);
        return taskSize;
    }
    printf("task size error\n");
    return (-2);
}

void *threadWorking(void *p)
{
    taskInfo_p info = (taskInfo_p) p;
    int fileDescripter;
    char *buf;

    // need extra 4 for wide null
    buf = malloc(info->size + 4);

    if(buf == NULL)
    // buffer error indicator
    {
        printf("buffer error inside threadWorking\n");
        return NULL;
    }
    // open the target file for read-only mode.
    fileDescripter = open(info->filename, O_RDONLY);
    if(fileDescripter==0)
    {
        printf("read file error inside threadWorking\n");
        free(buf);
        return NULL;
    }
    // read amount of task from main and stroe it to buf
    //because we open the file read-only, we don't need mutex for race condition.
    lseek(fileDescripter, info->start, SEEK_SET);
    long res = read(fileDescripter, buf, info->size);
    debug_print("On %d, asked for %ld bytes, got %ld bytes\n", info->threadID, info->size, res);

    //printf("On %d, asked for %ld bytes, got %ld bytes\n", info->threadID, info->size, res);

    close(fileDescripter);

    //put wide null for saftey
    buf[info->size] = 0;
    buf[info->size+1] = 0;

    free(buf);
    return NULL;
}

//void *add_word(const char* w) {
//    struct wordFrequency *wF = NULL;
//
//    HASH_FIND_STR( wordTable, w, wF);
//
//    if(wF == NULL)
//    {
//        wF = (struct wordFrequency *)malloc(sizeof *wF);
//        wF->word = w;
//        wF->frequency = 1;
//
//        HASH_ADD_KEYPTR(hh, wordTable, wF->word, strlen(wF->word), wF);
//    }
//    else
//    {
//        wF = (struct wordFrequency *)malloc(sizeof( *wF));
//        wF->word = w;
//        wF->frequency = (wF->frequency) + 1;
//
//        HASH_ADD_KEYPTR(hh, wordTable, wF->word, strlen(wF->word), wF);
//    }
//    return NULL;
//}

