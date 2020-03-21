/**************************************************************
*
* Class: CSC-415-0# Spring 2020
* Name: Jungsun Eoh
* Student ID: 918590990
* Project: Assignment 2 – Word Blast
* File: main.c
*
* Description: a program that returns number of all the words
*              that are 6 or more characters long, using threads.
*
**************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include "uthash.h"     // Copyright (c) 2003-2018, Troy D. Hanson


struct wordFrequency {
    const char* word;   // key
    int frequency;      // value
    UT_hash_handle hh;
};


struct wordFrequency *wordTable = NULL;
// hash table to keep word and its frequency.

void *add_word(const char* w);
/* adding word to hashtable. if the word already exists, value of the key(the word) will increment by 1.
 * So the hash table will keep tracking how many times the word appered in the .txt file.
 */

//void threadCreating(int taskSize);

char* getLine(void);
int numberOfThread;

int main(int argc, char *argv[])
{
    struct timespec startTime;
    struct timespec endTime;
    clock_gettime(CLOCK_REALTIME, &startTime);

    FILE *fptr;
    char* fileName;
    int fileLen;
    int taskSize;

    fileName = getLine();
  // open the target file for read-only mode.
    fptr = fopen(fileName, "r");

    if(fptr == NULL) {  printf("File open error\n");  }
    else
    {
        taskSize = fileLen/numberOfThread;

        fseek(fptr, 0, SEEK_END);
        fileLen = ftell(fptr);
 //      printf("Size of file: %d bytes\n", taskSize);
 //       threadCreating(taskSize);












        // closing file
        fclose(fptr);
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

    return 0;
}

void *add_word(const char* w) {
    struct wordFrequency *wF = NULL;

    HASH_FIND_STR( wordTable, w, wF);

    if(wF == NULL)
    {
        wF = (struct wordFrequency *)malloc(sizeof *wF);
        wF->word = w;
        wF->frequency = 1;

        HASH_ADD_KEYPTR(hh, wordTable, wF->word, strlen(wF->word), wF);
    }
    else
    {
        wF = (struct wordFrequency *)malloc(sizeof( *wF));
        wF->word = w;
        wF->frequency = (wF->frequency) + 1;

        HASH_ADD_KEYPTR(hh, wordTable, wF->word, strlen(wF->word), wF);
    }
}

char* getLine(void)
// get user input line.
{
    int limit = 255 * sizeof(char);     // maximum file name length
    char *buf = (char*) malloc(limit);
    char *temp = (char*) malloc(limit);
    int fail =1;
    int len;

    do
    {
        strcat(temp, "../");

        printf("Input the file name: ");
        if (fgets(buf, limit, stdin) == NULL)       // case: fgets() fail
        {
            printf("malloc error");
            exit(EXIT_FAILURE);
        }
        else if (strstr(buf, ".txt"))
        {
            len = strlen(buf);
            if(buf[len-1] == '\n')
            {   buf[len-1] =0;  }
            strcat(temp, buf);
            fail = 0;

            printf("Input how many thread will use: ");
            scanf("%d", &numberOfThread);

            return temp;
        }
        else    // case: user put wrong type of file.
        {
            printf("Please put relevant file name. it should be @@@.txt \n\n");
        }
        free(temp);
        free(buf);
    } while(fail);
    return 0;
}
