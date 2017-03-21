#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX 6

char intArray[MAX];
int front = 0;
int rear = -1;
int itemCount = 0;

int peek() {
   return intArray[front];
}

bool isEmpty() {
   return itemCount == 0;
}

bool isFull() {
   return itemCount == MAX;
}

int size() {
   return itemCount;
}

void insert(char *data) {

   if(!isFull()) {

      if(rear == MAX-1) {
         rear = -1;
      }

      intArray[++rear] = *data;
      itemCount++;
   }
}

char *removeData() {
   char *data = &intArray[front++];

   if(front == MAX) {
      front = 0;
   }

   itemCount--;
   return data;
}
