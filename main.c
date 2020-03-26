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
    char* word;   // key
    int frequency;      // value
} wordFrequency, *wordFrequency_p;

volatile wordFrequency_p wordTable = NULL;
volatile int maxWordTable = 0;  //size of the array
int countWordTable = 0;     // number of value in the array

long taskSize;
char* fileName;
pthread_mutex_t lock;

void add_word(char* w);
/* adding word to hashtable. if the word already exists, value of the key(the word) will increment by 1.
 * So the hash table will keep tracking how many times the word appeared in the .txt file.
 */
void* threadWorking(void *);
void getTaskSize(char*, int);

int main(int argc, char *argv[])
{
    int numberOfThread;
    pthread_t* threadPT;
    int* index;

    fileName = "NOFILE"; // having a default value is a good habit
    numberOfThread = 1;    // default value

    struct timespec startTime;
    struct timespec endTime;
    clock_gettime(CLOCK_REALTIME, &startTime);

    if(argc > 2)
    {
        fileName = argv[1];
        numberOfThread = atoi(argv[2]);
        getTaskSize(fileName, numberOfThread);
    }
    else
        {
        printf("argument error: %s filename ThreadCountYouWant\n", argv[0]);
        return (-1);
        }

    index = malloc(sizeof(long) * numberOfThread);
    threadPT = malloc(sizeof(pthread_t) * numberOfThread);

    for(int i =0; i<numberOfThread; i++)
    {
        index[i] = i;
        pthread_create(&(threadPT[i]), NULL, threadWorking, &(index[i]));
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

    free(index);
    free(threadPT);

    return 0;
}

void getTaskSize(char* fileName, int numberOfThread)
{
    int fileDescripter;
    long fileLen;
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
    }
}

void *threadWorking(void *arg)
{
    int index_i = *(int *)arg;
    int fileDescripter;
    char *buf;
    char *word;
    char *saveptr;
    char * delim = "\"\'.""''?:;-,—*($%)! \t\n\x0A\r";

    // need extra 4 for wide null
    buf = malloc((sizeof(char) * taskSize) +4);

    if(buf ==NULL)
    // buffer error indicator
    {
        printf("buffer error inside threadWorking\n");
        return NULL;
    }
    // open the target file for read-only mode.
    fileDescripter = open(fileName, O_RDONLY);
    if(fileDescripter==0)
    {
        printf("read file error inside threadWorking\n");
        free(buf);
        return NULL;
    }

    // read amount of task from main and store into buf
    //because we open the file read-only, we don't need mutex for race condition.
    lseek(fileDescripter, index_i * taskSize, SEEK_SET);

    ////////////////////////*if ()*///////////////////
    long res = read(fileDescripter, buf, taskSize);
    debug_print("On thread %d: asked for %ld bytes, got %ld bytes\n", index_i,taskSize, res);

    close(fileDescripter);

    //put wide null for saftey
    buf[taskSize] = 0;
    buf[taskSize+1] = 0;

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
    if(wordTable == NULL)  // first input.
    {
        pthread_mutex_lock(&lock);
        wordTable = malloc(sizeof(wordFrequency) * SIZEBLOCK);
        maxWordTable = SIZEBLOCK;
        if (wordTable == NULL)
        {
            printf("first init malloc error in addWord\n");
            exit(-2);
        }
        // w is char pointer, and pointer could change it. we have to make new heap memory to store
        // word to always have valid value.
        debug_print("new word %s\n", w);
        wordTable[0].word = malloc(strlen(w) +2);
        strcpy(wordTable[0].word, w);  //copy the word into wordTable
        wordTable[0].frequency = 1;
        countWordTable = 1;
        pthread_mutex_unlock(&lock);
    }else   // wordtable already exists.
    {
        for(int i =0; i < countWordTable; i ++)        // find the word in the table
        {
            if (strcasecmp(wordTable[i].word, w) == 0)
            {
                pthread_mutex_lock(&lock);
                wordTable[i].frequency = wordTable[i].frequency +1;
                pthread_mutex_unlock(&lock);
                return;
            }
        }
        if(maxWordTable <= countWordTable)
            // if we don't have enough room for the table, we reallocate malloc.
        {
            pthread_mutex_lock(&lock);
            maxWordTable = maxWordTable + SIZEBLOCK;
            wordFrequency_p reallocRet = realloc(wordTable, sizeof(wordFrequency) * maxWordTable);
            if (reallocRet == NULL)
            {
                printf("realloc malloc error in addWord\n");
                exit(-2);
            }
            wordTable = reallocRet;
            pthread_mutex_unlock(&lock);
        }
        // add the new word
        pthread_mutex_lock(&lock);

        wordTable[countWordTable].word = malloc (strlen(w) +2);
        strcpy(wordTable[countWordTable].word, w);  //copy the word into wordTable
        wordTable[countWordTable].frequency = wordTable[countWordTable].frequency + 1;
        ++countWordTable;
        pthread_mutex_unlock(&lock);

        debug_print("new word2 %s\n", w);
    }

    return;
}

