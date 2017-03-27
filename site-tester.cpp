/* ********************************************** *
 * Project 4: System for Verifying Web Placement  *
 * Authors: Nick Palutsis & Travis Gayle          *
 * Date: March 24, 2017                           *
 * ********************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>
#include <pthread.h>
#include "queueBuild.h"
#include <queue>
#include <string>
using namespace std;

#define STRMAX 100
#define QMAX 100

char urlQueue[QMAX][80];
char fetchQueue[QMAX][80];
char searchQueue[QMAX][80];
char parseQueue[QMAX][80];



struct MemoryStruct {
	char *memory;
	size_t size;
};

struct thread_args{
    pthread_t id;
    char (*fetchQueue)[80], (*searchQueue)[80];
    struct MemoryStruct *parseQueue;
    int numSites, frontSite, rearSite, frontSearch, rearSearch;
};

struct responseStruct{
	string url;
	char *response;
};

queue <string> URLQueue;
queue <responseStruct> bufferQueue;

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	mem->memory = (char *) realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL) {
		/* out of memory! */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

int insertFetchQueue(char fetchQueue[QMAX][80], int *rear, char data[80]);
int deleteFetchQueue(char fetchQueue[QMAX][80], int *front, int *rear, char data[80]);
int insertSearchQueue(char searchQueue[QMAX][80], int *rear, char data[80]);
int deleteSearchQueue(char searchQueue[QMAX][80], int *front, int *rear, char data[80]);
void* fetch( struct thread_args *fetchArgs);
void* parse( struct thread_args *parseArgs);

int main(int argc, char *argv[]) {

	// default value for param
	int PERIOD_FETCH = 180; 					//The time (in seconds) between fetches of the various sites
	int NUM_FETCH = 1; 							//Number of fetch threads
	int NUM_PARSE = 1; 							//Number of parsing threads
	char SEARCH_FILE[STRMAX] = "Search.txt"; 	//File containing the search strings
	char SITE_FILE[STRMAX] = "Sites.txt";  		//File containing the sites to query

	int numSites = 0;
	// int printHTML = 0;

	if (argc == 2) {
		// We assume argv[1] is a filename to open
		FILE *file = fopen( argv[1], "r" );

		/* fopen returns 0, the NULL pointer, on failure */
		if ( file == 0 ) {
			printf( "Could not open file\n" );
		} else {
			char c;
			char variable[STRMAX];
			char value[STRMAX];
			int equal = 0;
			int i = 0;
			// read one character at a time from file, stopping at EOF
			while  ( ( c = fgetc( file ) ) != EOF ) {
				if (c == '=') {
					equal = 1;
					i = 0;
				} else if (!equal) {
					variable[i++] = c;
					variable[i+1] = '\0';
				} else if (c != '\n') {
					value[i++] = c;
					value[i+1] = '\0';
				} else {
					if (strcmp(variable, "PERIOD_FETCH") == 0) {
						PERIOD_FETCH = atoi(value);
					} else if (strcmp(variable, "NUM_FETCH") == 0) {
						NUM_FETCH = atoi(value);
					} else if (strcmp(variable, "NUM_PARSE") == 0) {
						NUM_PARSE = atoi(value);
					} else if (strcmp(variable, "SEARCH_FILE") == 0) {
						strcpy(SEARCH_FILE, value);
					} else if (strcmp(variable, "SITE_FILE") == 0) {
						strcpy(SITE_FILE, value);
					}

					for (i = 0; i < sizeof(variable); i++)
						variable[i] = '\0';
					for (i = 0; i < sizeof(value); i++)
						value[i] = '\0';
					equal = 0;
					i = 0;
				}
				// printf( "%s\n", value );
			}
			fclose( file );
		}
	}

	printf("PERIOD_FETCH: %d\n", PERIOD_FETCH);
	printf("NUM_FETCH: %d\n", NUM_FETCH);
	printf("NUM_PARSE: %d\n", NUM_PARSE);
	printf("SEARCH_FILE: %s\n", SEARCH_FILE);
	printf("SITE_FILE: %s\n\n", SITE_FILE);

	// char fetchQueue[QMAX][80];

	// char data[80];
	int frontSite = -1;
	int rearSite = -1;

	FILE *file = fopen( SITE_FILE, "r" );

	/* fopen returns 0, the NULL pointer, on failure */
	if ( file == 0 ) {
		printf( "Could not open file\n" );
	} else {
		char c;
		char url[STRMAX];
		int i = 0;
		// read one character at a time from file, stopping at EOF
		while  ( ( c = fgetc( file ) ) != EOF ) {
			if (c != '\n' && c != ' ' && c != '\t') {
				url[i++] = c;
				url[i+1] = '\0';

			} else {
				URLQueue.push(url);
				// printf("%s\n", url);
				if(insertFetchQueue(fetchQueue, &rearSite, url) == -1) printf("Queue is full\n");
				for (i = 0; i < sizeof(url); i++)
					url[i] = '\0';
				i = 0;
				numSites++;

			}

		}
		fclose( file );
		if(insertFetchQueue(fetchQueue, &rearSite, url) == -1) printf("Queue is full\n");
	}

	// printf("\n\n\n\n\n\n\n");
	// int a, b;
	// for (a = 0; a < QMAX; a++){
	// 	for (b = 0; b < 80; b++){
	// 		printf("%c", siteQueue[a][b]);
	// 	}
	// 	printf("\n");
	// }


	int frontSearch = -1;
	int rearSearch = -1;

	file = fopen( SEARCH_FILE, "r" );

	/* fopen returns 0, the NULL pointer, on failure */
	if ( file == 0 ) {
		printf( "Could not open file\n" );
	} else {
		char c;
		char phrase[STRMAX];
		int i = 0;
		// read one character at a time from file, stopping at EOF
		while  ( ( c = fgetc( file ) ) != EOF ) {
			if (c != '\n') {
				phrase[i++] = c;
				phrase[i+1] = '\0';
			} else {
				if(insertSearchQueue(searchQueue, &rearSearch, phrase) == -1) printf("Queue is full\n");
				for (i = 0; i < sizeof(phrase); i++)
					phrase[i] = '\0';
				i = 0;
				printf("%s", phrase);
			}

		}
		fclose( file );
		if(insertSearchQueue(searchQueue, &rearSearch, phrase) == -1) printf("Queue is full\n");

		// for (i = frontSearch+1; i <= rearSearch; i++)
		// 	printf("%s\n", searchQueue[i]);

		// if(deleteSearchQueue(searchQueue, &frontSearch, &rearSearch, data) != -1) printf("\n Deleted String from Queue is : %s\n", data);

		// for (i = frontSearch+1; i <= rearSearch; i++)
		// 	printf("%s\n", searchQueue[i]);
	}

//locking, when pushing and pulling to the Queue
//sites should be operated on one at a time

	struct MemoryStruct parseQueue[numSites];

	struct thread_args *fetchArgs;
    fetchArgs = (struct thread_args *) calloc(NUM_FETCH, sizeof(struct thread_args));
    int i;

	for(i = 0; i < NUM_FETCH; ++i){
        fetchArgs[i].fetchQueue = fetchQueue;
        fetchArgs[i].searchQueue = searchQueue;
        fetchArgs[i].parseQueue = parseQueue;
        fetchArgs[i].numSites = numSites;
        fetchArgs[i].frontSite = frontSite;
        fetchArgs[i].rearSite = rearSite;
        fetchArgs[i].frontSearch = frontSearch;
        fetchArgs[i].rearSearch = rearSearch;
		pthread_t temp;
        fetchArgs[i].id = temp;


        // (void) pthread_create(&fetchArgs[i].id,NULL, &fetch, &fetchArgs[i]);
		pthread_create(&fetchArgs[i].id, NULL, fetch,NULL);
		//fetch(&fetchArgs[i]);
    }

	for (i = 0; i < NUM_FETCH; ++i ){
		// pthread_join function takes two parameters:
		// 1. the pthread_t variable used when pthread_create was called
		// 2. a pointer to the return value pointer

		(void) pthread_join(fetchArgs[i].id, NULL);
	}


	struct thread_args *parseArgs;
    parseArgs = calloc(NUM_PARSE, sizeof(struct thread_args));

	for(i = 0; i < NUM_PARSE; ++i){
        parseArgs[i].fetchQueue = fetchQueue;
        parseArgs[i].searchQueue = searchQueue;
        parseArgs[i].parseQueue = parseQueue;
        parseArgs[i].numSites = numSites;
        parseArgs[i].frontSite = frontSite;
        parseArgs[i].rearSite = rearSite;
        parseArgs[i].frontSearch = frontSearch;
        parseArgs[i].rearSearch = rearSearch;


		pthread_create(&parseArgs[i].id, NULL,  &parse,&parseArgs[i]);
        //parse(&parseArgs[i]);
        // (void) pthread_create(&parseArgs[i].id,NULL, &parse, &parseArgs[i]);


    }

	for (i = 0; i < NUM_PARSE; ++i ){
		// pthread_join function takes two parameters:
		// 1. the pthread_t variable used when pthread_create was called
		// 2. a pointer to the return value pointer

		(void) pthread_join(parseArgs[i].id, NULL);
	}

	/* we're done with libcurl, so clean it up */
	curl_global_cleanup();

	return 0;
}

//=============================================================================
//============================== FUNCTIONS ====================================
//=============================================================================

int insertFetchQueue(char fetchQueue[QMAX][80], int *rear, char data[80]) {
	if(*rear == QMAX -1)
		return(-1);
	else {
		*rear = *rear + 1;
		strcpy(fetchQueue[*rear], data);
		return(1);
	}
}

int deleteFetchQueue(char fetchQueue[QMAX][80], int *front, int *rear, char data[80]) {
	if(*front == *rear)
		return(-1);
	else {
		(*front)++;
		strcpy(data, fetchQueue[*front]);
		return(1);
	}
}

int insertSearchQueue(char searchQueue[QMAX][80], int *rear, char data[80]) {
	if(*rear == QMAX -1)
		return(-1);
	else {
		*rear = *rear + 1;
		strcpy(searchQueue[*rear], data);
		return(1);
	}
}

int deleteSearchQueue(char searchQueue[QMAX][80], int *front, int *rear, char data[80]) {
	if(*front == *rear)
		return(-1);
	else {
		(*front)++;
		strcpy(data, searchQueue[*front]);
		return(1);
	}
}

void* fetch(void * weenie) {
	CURL *curl_handle;
	CURLcode res;

	//int currentChunk = 0;

//	printf("%s\n", fetchArgs->fetchQueue[0]);

//	while (currentChunk <= fetchArgs->numSites){ //FRONT OF LOOP

		struct MemoryStruct chunk;

		chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
		chunk.size = 0;    /* no data at this point */

		curl_global_init(CURL_GLOBAL_ALL);

		//int counter;

		/* init the curl session */
		curl_handle = curl_easy_init();

		string nextURL = URLQueue.front();
		URLQueue.pop();
	    //printf("queue being fetched --> %s\t%d\n", fetchArgs->fetchQueue[currentChunk], currentChunk);
		/* specify URL to get */
		curl_easy_setopt(curl_handle, CURLOPT_URL, nextURL);

		/* send all data to this function  */
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

		/* we pass our 'chunk' struct to the callback function */
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

		/* some servers don't like requests that are made without a user-agent
		 field, so we provide one */
		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

		/* get it! */
		res = curl_easy_perform(curl_handle);

		/* check for errors */
		if(res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		} else {
			/*
			 * Now, our chunk.memory points to a memory block that is chunk.size
			 * bytes big and contains the remote file.
			 *
			 * Do something nice with it!
			 */
			responseStruct r {url ,chunk.memory}

			bufferQueue.push(r);

		}

		/* cleanup curl stuff */
		curl_easy_cleanup(curl_handle);

		// free(chunk.memory);
	    // deleteFetchQueue(fetchQueue, &frontSite, &rearSite, data);
	//}

	return 0;
}

void* parse(struct thread_args *parseArgs) {
	int currentChunk = 0;
	while (currentChunk <= parseArgs->numSites) {
		int i = parseArgs->frontSearch + 1;

		// if (printHTML) printf("\n%s\n\n", parseArgs->parseQueue[currentChunk].memory);

	    char *sentence;
	    FILE * fp;

	    fp = fopen ("file.csv", "a");
	    sentence = "Time,Phrase,Site,Count\n";
	    fprintf(fp, "%s", sentence);
		while (i <= parseArgs->rearSearch) {
			int count = 0;
			//const char *tmp = parseArgs->parseQueue[currentChunk].memory;
			responseStruct tmp = bufferQueue.front();
			bufferQueue.pop();


			while((tmp.response = strstr(tmp.response, parseArgs->searchQueue[i]))) {
			   count++;
			   tmp++;
			}

			printf("Found \"%s\" %d times.\n", parseArgs->searchQueue[i], count);

	        //PRODUCING TIME
	        time_t now;
	        time(&now);

	        struct tm* now_tm;
	        now_tm = localtime(&now);

	        char out[80];
	        strftime(out, 80, "%Y-%m-%d %H:%M:%S", now_tm);

	        fprintf(fp, "%s,%s,%s,%d\n", out, parseArgs->searchQueue[i], parseArgs->fetchQueue[parseArgs->frontSite+1], count);
	        i++;

	    }
	    fprintf(fp, "%s", "\n");
			fclose(fp);
		printf("%lu bytes retrieved\n\n", (long)parseArgs->parseQueue[currentChunk].size);
		free(parseArgs->parseQueue[currentChunk].memory);
		char data[80];
		deleteFetchQueue(parseArgs->fetchQueue, &parseArgs->frontSite, &parseArgs->rearSite, data);
		currentChunk++;
	}

	return 0;
}
