// C Language program to implement to the basic operations on String Queue

# include <stdio.h>
// # include <conio.h>
# include <string.h>
# define max 5

int insq(char queue[max][80], int *rear, char data[80])
{
	if(*rear == max -1)
		return(-1);
	else
	{
		*rear = *rear + 1;
		strcpy(queue[*rear], data);
		return(1);
	} // else
}; // insq

int delq(char queue[max][80], int *front, int *rear, char data[80])
{
	if(*front == *rear)
		return(-1);
	else
	{
		(*front)++;
		strcpy(data, queue[*front]);
		return(1);
	} // else
}; // delq

int main()
{
	char queue[max][80], data[80];
	int front, rear, reply, option;

	// clrscr();

	//... Initialise a Queue

	front = rear = -1;

	do
	{
		printf("\n C Language program to implement the basic operations on String Queue \n");
		printf("\n 1. Insert String in a Queue");
		printf("\n 2. Delete String from a Queue");
		printf("\n 3. Exit");
		printf("\n Select proper option ( 1 / 2 / 3 ) : ");
		scanf("%d", &option);
		switch(option)
		{
			case 1 : // insert
				printf("\n Enter the String to be insert in a Queue : ");
				// flushall();
				// fgets(data, 80, stdin);
				scanf("%s",&data);
				reply = insq(queue, &rear, data);
				if( reply == -1 )
					printf("\n Queue is Full \n");
				else
					printf("\n Entered String is Inserted in a Queue \n");
				break;
			case 2 : // delete
				reply = delq(queue, &front, &rear, data);
				if( reply == -1 )
					printf("\n Queue is Empty \n");
				else
					printf("\n Deleted String from Queue is : %s", data);
					printf("\n");
				break;
			case 3 : exit(0);
		} // switch
	}while(1);
	return 0;
} // main