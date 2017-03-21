#include <stdio.h>
#include <stdlib.h>  /* For exit() function */
int main()
{
   char program[50];
   char *sentence;
   FILE *fptr;

for (int i = 0; i < 4; i++){
   sprintf(program,"%d.csv", i);

   fptr = fopen(program, "w");
   if(fptr == NULL)
   {
      printf("Error!");
      exit(1);
   }

   sentence = "Time,Phrase,Site,Count";

   fprintf(fptr,"%s", sentence);
   fclose(fptr);
}
   return 0;
}
