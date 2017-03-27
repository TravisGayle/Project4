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

#define STRMAX 100
#define QMAX 100

struct MemoryStruct {
	char *memory;
	size_t size;
};

void display_message(int s){
      signal(SIGALRM, SIG_IGN);          /* ignore this signal       */
      printf("still threading...\n"); //The print message informs the user that the process is not complete yet
      signal(SIGALRM, display_message);     /* reinstall the handler    */
      alarm(1); // a SIGALRM signal is generated every second prompting the
      //print statment to occur.
}

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
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

int main(int argc, char *argv[]) {

	// default value for param
	int PERIOD_FETCH = 180; 					//The time (in seconds) between fetches of the various sites
	int NUM_FETCH = 1; 							//Number of fetch threads
	int NUM_PARSE = 1; 							//Number of parsing threads
	char SEARCH_FILE[STRMAX] = "Search.txt"; 	//File containing the search strings
	char SITE_FILE[STRMAX] = "Sites.txt";  		//File containing the sites to query

	int numSites = 0;
	int printHTML = 0;

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

	char fetchQueue[QMAX][80], data[80];
	int frontSite = -1;
	int rearSite = -1;

	FILE *file = fopen( SITE_FILE, "r" );

	signal(SIGALRM,display_message);

	alarm(1);

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

	char searchQueue[QMAX][80];
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

	//queue that stores newly downloaded webpages to be processed by consumers

	//MAX NUMBER OF THREADS FOR DATA PROCESSING

	//max number of threads available for fetching

	// 1 thread each

	// Control C or SIGHUP for operation termination

	CURL *curl_handle;
	CURLcode res;

	int currentChunk = 0;
	struct MemoryStruct parseQueue[numSites];

	int keepRunning = 1;

	//MUTEX LOCKING EXAMPLE
	int count;

	pthread_mutex_t count_mutex;
	pthread_mutex_lock(&count_mutex);
		count = count + 1;
	pthread_mutex_unlock(&count_mutex);

	//THREAD CREATION
	pthread_t pThread[NUM_FETCH];
	int rc
	long t;
	for (t = 0; t < NUM_FETCH; ++t) {
		printf("Creating thread %d\n", 1);
		rc = pthread_create(&pThread, NULL, fetchFunction, (void *)t);
		if (rc) {
		  printf("ERROR: return code from pthread_create() is %d\n", rc);
		  exit(-1);
		}
	}

	//THREAD JOINING
	for (t = 0; t < NUM_FETCH; ++t) {
		pthread_join(pThread[NUM_FETCH], NULL);
	}

    while (currentChunk <= numSites){ //FRONT OF LOOP

		struct MemoryStruct chunk;

		chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
		chunk.size = 0;    /* no data at this point */

		curl_global_init(CURL_GLOBAL_ALL);

		//int counter;

		/* init the curl session */
		curl_handle = curl_easy_init();

	    printf("queue being fetched --> %s\n", fetchQueue[currentChunk]);
		/* specify URL to get */
		curl_easy_setopt(curl_handle, CURLOPT_URL, fetchQueue[currentChunk]);

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
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
		} else {
			/*
			 * Now, our chunk.memory points to a memory block that is chunk.size
			 * bytes big and contains the remote file.
			 *
			 * Do something nice with it!
			 */
			parseQueue[currentChunk] = chunk;
			currentChunk++;
		}

		/* cleanup curl stuff */
		curl_easy_cleanup(curl_handle);

		// free(chunk.memory);
	    // deleteFetchQueue(fetchQueue, &frontSite, &rearSite, data);
	}

	currentChunk = 0;
	while (currentChunk <= numSites) {
		int i = frontSearch + 1;

		if (printHTML) printf("\n%s\n\n", parseQueue[currentChunk].memory);

        char *sentence;
        FILE * fp;

        fp = fopen ("file.csv", "a");
        sentence = "Time,Phrase,Site,Count\n";
        fprintf(fp, "%s", sentence);
		while (i <= rearSearch) {
			int count = 0;
			const char *tmp = parseQueue[currentChunk].memory;
			while((tmp = strstr(tmp, searchQueue[i]))) {
			   count++;
			   tmp++;
			}

			printf("Found \"%s\" %d times.\n", searchQueue[i], count);

            //PRODUCING TIME
            time_t now;
            time(&now);

            struct tm* now_tm;
            now_tm = localtime(&now);

            char out[80];
            strftime(out, 80, "%Y-%m-%d %H:%M:%S", now_tm);

            fprintf(fp, "%s,%s,%s,%d\n", out, searchQueue[i], fetchQueue[frontSite+1], count);
            i++;

        }
        fprintf(fp, "%s", "\n");
   		fclose(fp);
		printf("%lu bytes retrieved\n\n", (long)parseQueue[currentChunk].size);
		free(parseQueue[currentChunk].memory);
		deleteFetchQueue(fetchQueue, &frontSite, &rearSite, data);
		currentChunk++;
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
