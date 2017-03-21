/* ********************************************** *
 * Project 4: System for Verifying Web Placement  *
 * Authors: Nick Palutsis & Travis Gayle          *
 * Date: March 24, 2017                           *
 * ********************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "queueBuild.h"

#define STRMAX 100
#define QMAX 100

struct MemoryStruct {
	char *memory;
	size_t size;
};

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

int insertSiteQueue(char siteQueue[QMAX][80], int *rear, char data[80]);
int deleteSiteQueue(char siteQueue[QMAX][80], int *front, int *rear, char data[80]);
int insertSearchQueue(char searchQueue[QMAX][80], int *rear, char data[80]);
int deleteSearchQueue(char searchQueue[QMAX][80], int *front, int *rear, char data[80]);

int main(int argc, char *argv[]) {

	// default value for param
	int PERIOD_FETCH = 180; 					//The time (in seconds) between fetches of the various sites
	int NUM_FETCH = 1; 							//Number of fetch threads
	int NUM_PARSE = 1; 							//Number of parsing threads
	char SEARCH_FILE[STRMAX] = "Search.txt"; 	//File containing the search strings
	char SITE_FILE[STRMAX] = "Sites.txt";  		//File containing the sites to query

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

	printf("\nPERIOD_FETCH: %d\n", PERIOD_FETCH);
	printf("NUM_FETCH: %d\n", NUM_FETCH);
	printf("NUM_PARSE: %d\n", NUM_PARSE);
	printf("SEARCH_FILE: %s\n", SEARCH_FILE);
	printf("SITE_FILE: %s\n\n", SITE_FILE);

/*    insert("nd.edu");
    insert("cnn.com");
    insert("pbs.org");
    if(isFull()){
        printf("Queue is full!\n");
    }
    char *site = removeData();

    printf("Element removed: %s\n",site);*/

	char siteQueue[QMAX][80], data[80];
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
				if(insertSiteQueue(siteQueue, &rearSite, url) == -1) printf("Queue is full\n");
				for (i = 0; i < sizeof(url); i++)
					url[i] = '\0';
				i = 0;
			}
		}
		fclose( file );
		if(insertSiteQueue(siteQueue, &rearSite, url) == -1) printf("Queue is full\n");

		for (i = frontSite+1; i <= rearSite; i++)
			printf("%s\n", siteQueue[i]);
		if(deleteSiteQueue(siteQueue, &frontSite, &rearSite, data) != -1) printf("\n Deleted String from Queue is : %s\n", data);

		for (i = frontSite+1; i <= rearSite; i++)
			printf("%s\n", siteQueue[i]);
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

		for (i = frontSearch+1; i <= rearSearch; i++)
			printf("%s\n", searchQueue[i]);
		
		if(deleteSearchQueue(searchQueue, &frontSearch, &rearSearch, data) != -1) printf("\n Deleted String from Queue is : %s\n", data);

		for (i = frontSearch+1; i <= rearSearch; i++)
			printf("%s\n", searchQueue[i]);
	}

	//queue that stores newly downloaded webpages to be processed by consumers

	//MAX NUMBER OF THREADS FOR DATA PROCESSING

	//max number of threads available for fetching

	// 1 thread each

	// Control C or SIGHUP for operation termination

	CURL *curl_handle;
	CURLcode res;

	struct MemoryStruct chunk;

	chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
	chunk.size = 0;    /* no data at this point */

	curl_global_init(CURL_GLOBAL_ALL);

	/* init the curl session */
	curl_handle = curl_easy_init();

	/* specify URL to get */
	curl_easy_setopt(curl_handle, CURLOPT_URL, "https://www.nd.edu");

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

		printf("%lu bytes retrieved\n", (long)chunk.size);
	}

	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);

	free(chunk.memory);

	/* we're done with libcurl, so clean it up */
	curl_global_cleanup();

	return 0;
}

//=============================================================================
//============================== FUNCTIONS ====================================
//=============================================================================

int insertSiteQueue(char siteQueue[QMAX][80], int *rear, char data[80]) {
	if(*rear == QMAX -1)
		return(-1);
	else {
		*rear = *rear + 1;
		strcpy(siteQueue[*rear], data);
		return(1);
	}
}

int deleteSiteQueue(char siteQueue[QMAX][80], int *front, int *rear, char data[80]) {
	if(*front == *rear)
		return(-1);
	else {
		(*front)++;
		strcpy(data, siteQueue[*front]);
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
