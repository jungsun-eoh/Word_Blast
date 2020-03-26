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

#ifdef DEBUG
#define DEBUG_TEST 1
#else
#define DEBUG_TEST 0
#endif

// easy to change spec in one place
#define MINCHARS 6
#define TOP 10
#define SIZEBLOCK 2500

#define debug_print(fmt, ...) \
            do { if(DEBUG_TEST) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

typedef struct wordFrequency {
    char* word;
    int frequency;
} wordFrequency ,*wordFrequency_p;

typedef struct taskInfo {
    char * filename;
    int threadID;
    long start;
    long size;
} taskInfo, *taskInfo_p ;

volatile wordFrequency_p *wordTable = NULL;     //array of target word
volatile int wordListCount = 0;                 // number of value in the array
int maxWordListSize = 0;                        // size of the array
pthread_mutex_t lock;

void add_word(char* w);
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
// find the top list
    wordFrequency top10[TOP];
    for (int j = 0; j < TOP; j++) {
        top10[j].frequency = 0;
    }

    for(int j = 0;  j< wordListCount; j++)
    {
        if(wordTable[j]->frequency > top10[TOP-1].frequency)
        {
            //debug_print("TOP %d. Adding %s %d\n", TOP, wordTable[j].word, wordTable[j].frequency);

            top10[TOP-1].word = wordTable[j]->word;
            top10[TOP-1].frequency = wordTable[j]->frequency;

            for(int p = TOP-2; p >=0; p-- )
            {
                if (wordTable[j]->frequency > top10[p].frequency)
                {
                    top10[p+1].word = top10[p].word;
                    top10[p+1].frequency = top10[p].frequency;
                    top10[p].word = top10[j].word;
                    top10[p].frequency = top10[j].frequency;
                }
            }
        }
    }

    printf("\n Word frequency count on %s with %d threads\n", fileName, numberOfThread);
    printf("printing top %d words %d char or more.\n", TOP, MINCHARS);

    for(int a = 0; a < TOP; a++)
        printf ("Number %d is %s with a count of %d", a+1, top10[a].word, top10[a].frequency);








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
    char *word;
    char *saveptr;
    char * delim = "\"\'.""''?:;-,—*($%)! \t\n\x0A\r";
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

    // chunk down the taskthread by word using strtok_r()
    word = strtok_r(buf, delim, &saveptr);
    //debug_print("%d word :: %s\n", index_i, word);

    while (word != NULL)
    {
        long int charSize = strlen(word);

        if (charSize >= MINCHARS)
        {
            add_word(word);
            //debug_print("%d word :: %s\n", index_i, word);
        }
        word = strtok_r(NULL, delim, &saveptr);
    }



    free(buf);
    return NULL;
}

void add_word(char* w) {
    for(int i =0; i < wordListCount; i ++)        // find the word in the table
    {
        if (strcasecmp(wordTable[i]->word, w) == 0)
        {
            pthread_mutex_lock(&lock);
            wordTable[i]->frequency = wordTable[i]->frequency +1;
            pthread_mutex_unlock(&lock);
            return;
        }
    }
    pthread_mutex_lock(&lock);
    //debug_print("maxWordTable: %d\n countWordTable: %d\n", maxWordTable, countWordTable);

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
        } else
        {
            wordFrequency_p reallocRet = realloc (*wordTable, sizeof(wordFrequency) * maxWordListSize);
            if (reallocRet == NULL)
            {
                printf("realloc malloc error in addWord\n");
                exit(-2);
            }
            wordTable = &reallocRet;
        }
    }
    // add the word to the list

    wordTable[wordListCount]->word = malloc(strlen(w) + 2);
    if (wordTable[wordListCount]->word == NULL) {
        printf("word malloc error in addWord\n");
        exit(-2);
    }
    strcpy(wordTable[wordListCount]->word, w);  //copy the word into wordTable
    wordTable[wordListCount]->frequency = 1;
    ++wordListCount;

    pthread_mutex_unlock(&lock);

    return;
}
