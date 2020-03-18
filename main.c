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
#include <pthread.h>
#include "uthash.h"     // Copyright (c) 2003-2018, Troy D. Hanson


struct wordFrequency {
    const char* word;   // key
    int frequency;
    UT_hash_handle hh;
};

struct wordFrequency *wordTable = NULL;

void *add_word(const char* w);

int main(int argc, char *argv[])
{
    struct timespec startTime;
    struct timespec endTime;
    clock_gettime(CLOCK_REALTIME, &startTime);

    FILE *fptr;
    int fileLen;

  // open the target file for read-only mode.
    fptr = fopen("/Users/eoh/Documents/csc415/Word Blast/WarAndPeace.txt", "r");

    // case: file open error
    if(fptr == NULL) {  printf("File open error\n");  }

    fseek(fptr, 0, SEEK_END);
    fileLen = ftell(fptr);
//    printf("Size of file: %d bytes\n", fileLen);











    // closing file
    fclose(fptr);


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
