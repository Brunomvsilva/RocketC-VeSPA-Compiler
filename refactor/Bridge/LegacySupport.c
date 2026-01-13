#include <stdio.h>


int lineCounter = 1;
void* postIncList = NULL; 
extern FILE* yyin;

int getLineNumber() {
    return lineCounter;
}

void incrementLineNumber() {
    lineCounter++;
}

FILE* getSourceFile() {
    return yyin;
}