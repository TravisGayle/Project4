# Project4  
*Authors:* Nick Palutsis & Travis Gayle  
*Date:* March 27, 2017  

The program can be built by typing the command `make` into the command line using the included Makefile. After, running the program is possible by either typing in `./site-tester` to use default values or `./site-tester Configuration.txt` where Configuration.txt is a text file in the following form:
```
PERIOD_FETCH=300
NUM_FETCH=2
NUM_PARSE=1
SEARCH_FILE=Search.txt
SITE_FILE=Sites.txt
```  
The files Sites.txt and Search.txt must also be included in the directory. These contain information about the list of sites and search terms to parse by respectively. They may be named differently than this, but the name must be specified in Configuration.txt, and `./site-tester Configuration.txt` must be the command called at run time.