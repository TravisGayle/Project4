/* Project 4: System for Verifying Web Placement
 * Authors: Nick Palutsis & Travis Gayle
 * Date: March 24, 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

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

int main(int argc, char *argv[]) {

	// default value for param
	int PERIOD_FETCH = 180; //The time (in seconds) between fetches of the various sites
	int NUM_FETCH = 1; //Number of fetch threads
	int NUM_PARSE = 1; //Number of parsing threads
	char *SEARCH_FILE = "Search.txt"; //File containing the search strings
	char *SITE_FILE = "Sites.txt";  //File containing the sites to query

	if (argc == 2) {
		// const char *configFile = argv[1]; // one specified argument for config file using syntax "PARAM=XXXXXX"

		// We assume argv[1] is a filename to open
		FILE *file = fopen( argv[1], "r" );

		/* fopen returns 0, the NULL pointer, on failure */
		if ( file == 0 ) {
			printf( "Could not open file\n" );
		} else {
			const int STRMAX = 100;
			char c;
			char variable[STRMAX];
			char value[STRMAX];
			int equal = 0;
			int i = 0;
			/* read one character at a time from file, stopping at EOF, which
			   indicates the end of the file.  Note that the idiom of "assign
			   to a variable, check the value" used below works because
			   the assignment statement evaluates to the value assigned. */
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
					printf("Variable: %s\n", variable);
					if (strcmp(variable, "PERIOD_FETCH")) {
						PERIOD_FETCH = atoi(value);
						printf( "PERIOD_FETCH: %d\n", PERIOD_FETCH );
					} else if (strcmp(variable, "NUM_FETCH")) {
						NUM_FETCH = atoi(value);
						printf( "NUM_FETCH: %d\n", NUM_FETCH );
					} else if (strcmp(variable, "NUM_PARSE")) {
						NUM_PARSE = atoi(value);
						printf( "NUM_PARSE: %d\n", NUM_PARSE );
					} else if (strcmp(variable, "SEARCH_FILE")) {
						strcpy(SEARCH_FILE, value);
						printf( "SEARCH_FILE: %s\n", value );
					} else if (strcmp(variable, "SITE_FILE")) {
						strcpy(SITE_FILE, value);
						printf( "SITE_FILE: %s\n", value );
					}
					variable[0] = 0;
					value[0] = 0;
					equal = 0;
					// printf( "Variable: %s\n", variable );
					// printf( "Value: %s\n", value );
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
	printf("SITE_FILE: %s\n", SITE_FILE);


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
