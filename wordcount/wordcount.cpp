#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <ctype.h>
#include "list.h"

#define RECORD_LEN 144

// For each apprearance of one record,
// each word will have one workIndex;
typedef struct wordIndex {
  size_t index; // wordcount 
  list_t list;  // other index for this word? 
} wordIndex_t;

// For each word, we will maintain two list. 
// The first one is linking myself to other words (list).
// The second one is the list of those links.
typedef struct wordInfo {
  char * word;
  size_t count;
  size_t dup;
  list_t list;      // All words will be linked together 
  list_t indexlist; // Linked to specific index list.
} wordInfo_t;

// Here, we are trying to define a global list for all words. 
// Every time when we find a word, we must search through this list.
// Thus it is very slow but we do not care for now.
// We only want to get job done in the first phase.
// We may sort this word in the second phase. Or we may
// utilize the hash map if it is not too complicated:-)
list_t gWordsList; 

// Introduce two global variables so that we may use it 
// in the multithreaded phase
char * fileStart; // The start of a file that we are going to parse 
char * fileEnd; // The end of a file that we are going to parse


size_t totalWords = 0;
size_t totalDiffWords = 0;
size_t totalDuplicateWords = 0;

/* check whether a word is the same as the specified one?
 */
int isSameWord(char * srcStr, char * destStr, int length) {
  int isSameWord = 0;

  if(strlen(srcStr) == length && strncmp(srcStr, destStr, length) == 0) {
    isSameWord = 1;
  }
  
  return isSameWord;
} 


inline wordInfo_t * getWordFromList(list_t * entry) {
  wordInfo_t dummy;
  int offset;

  offset = (int)(((intptr_t)&dummy.list - (intptr_t)&dummy.word));
  return (wordInfo_t *)((intptr_t)entry - offset);
}

inline wordIndex_t * getWordIndexFromList(list_t * entry) {
  wordIndex_t dummy;
  int offset;

  offset = (int)(((intptr_t)&dummy.list - (intptr_t)&dummy.index));
  return (wordIndex_t *)((intptr_t)entry - offset);
}


/*
 Check whether a word is existing or not. 
 If this word is not existing, it return NULL.
 Otherwise, it will return corresponding wordInfo_t so that we can
 add the current word into this existing list.
 Then we can update its index and wordcount.
 */
wordInfo_t * isWordExisting(char * wordStart, int length) {
  list_t * current;
  wordInfo_t * wordFound = NULL;

  current = &gWordsList;

  // Traverse the whole list
  while(!isListTail(current, &gWordsList)) {
    // Check the next one
    current = nextEntry(current);
    
    wordInfo_t * thisWord = getWordFromList(current);

  //  fprintf(stderr, "current %p, thisWord is %p ***********%s***********\n", current, thisWord->word, thisWord->word);
    // Check whether the word is the same as current word?
    if(isSameWord(thisWord->word, wordStart, length)) {
      wordFound = thisWord;
      break;
    }    
  } 
  
  //fprintf(stderr, "\n\ncheck word end\n\n");
  return wordFound;
}

// Add an existing word to the global list
void addExistingWord(wordInfo_t * wordInfo, char * wordStart, int length, size_t index) {
  wordIndex_t * lastIndex;
  wordIndex_t * wordIndex;

  if(wordInfo == NULL) {
    fprintf(stderr, "Wrong: wordInfo not existing here.\n");
    return;
  }

  // Check whether the tail has the same index.
  list_t * list = tailList(&wordInfo->indexlist);
  if(lastIndex == NULL) {
    fprintf(stderr, "Wrong: lastIndex list is not existing.\n");
    return;
  }

  lastIndex = getWordIndexFromList(list);
  //fprintf(stderr, "lastIndex %p index %d at %p when existing\n", lastIndex, lastIndex->index, &lastIndex->index);
  if(lastIndex->index != index) {
    // Malloc a block of memory to hold it.
    wordIndex = (wordIndex_t *)malloc(sizeof(wordIndex_t));
    wordIndex->index = index;
    listInit(&wordIndex->list);

    listInsertTail(&wordIndex->list, &wordInfo->indexlist);
    wordInfo->count++;
    //listPrintItems(&gWordsList, 1);
    //fprintf(stderr, "NOt duplicate, lastIndex %d now index %d!!\n", lastIndex->index, index);
  }
  else {
    //fprintf(stderr, "duplicate!!\n");
    wordInfo->dup++;
    totalDuplicateWords++;
  }

  totalWords++;
}

// Add an new word to the global list
// wordStart is coming from the stack, so we must allocate a block 
// of memory to hold actual word here.
void addNewWord(char * wordStart, int length, size_t index) {
  wordInfo_t * wordInfo = NULL;
  char * word = NULL; 
  wordIndex_t * wordIndex;

  // Malloc a block of memory with size "wordInfo_t"
  wordInfo = (wordInfo_t *)malloc(sizeof(wordInfo_t));

  // We must allocate a new block of memory to hold actual word.
  // We can not point to wordStart since it is an local variable
  word = (char *)malloc(length+1);

  if(wordInfo == NULL || word == NULL) {
    fprintf(stderr, "Failed to allocate memory for wordInfo_t and word.\n");
    return;
  }

  // Initialize the word related information.
  memcpy(word, wordStart, length);
  word[length] = '\0';

  wordInfo->word = word;
  wordInfo->count = 1;
  wordInfo->dup = 0;
  listInit(&wordInfo->list);
  listInit(&wordInfo->indexlist);
  
  // Add this word into the global list.
  //fprintf(stderr, "Add new word: %s length %d. ADDING wordInfo %p\n", word, length,& wordInfo->list);
  listInsertTail(&wordInfo->list, &gWordsList);

  // Add corresponding index into the index list.
  wordIndex = (wordIndex_t *)malloc(sizeof(wordIndex_t));
  if(wordIndex  == NULL) {
    fprintf(stderr, "malloc failed for wordIndex\n");
    return;
  }

  wordIndex->index = index;
  //fprintf(stderr, "Adding wordIndex %p with index %ld\n", &wordIndex->list, index);
  listInit(&wordIndex->list);
  listInsertTail(&wordIndex->list, &wordInfo->indexlist);
  //listPrintItems(&gWordsList, 1);
  fprintf(stderr, "Adding wordIndex %p index %ld at %p\n", &wordIndex->list, wordIndex->index, &wordIndex->index);

  totalWords++;
  totalDiffWords++;
}

// Check whether we need to handle a word or not
int isGoodWord(char * start, char * end) {
  int isGood = 1;
   
  // Check whether this word is starting with "http"
  // If starting with "http", it is not a normal word,
  // We donot need to handle this word.
  if(strncmp(start, "http", strlen("http")) == 0) {
    isGood = 0;
  }
  else {
    char * cur = start;
    // Check whether it is a valid word or not
    // The last letter of a word doesn't matter.
    while(cur < end) {
      char curr_ltr = toupper(*cur);
      if((curr_ltr < 'A' || curr_ltr > 'Z') && (curr_ltr != '\n')) {
        isGood = 0;
        break;
      }
      cur++;
    }  
  }
  return isGood;
}

// Add a word to the list. Here, we do not differentiate
// the upper case and lower case. We will turn everything into
// upper case
void addWordToList(char * start, char * end) {
  char * cur = start;
  int  i = 0; 
  char word[1024]; 
  int length; 
  wordInfo_t * wordInfo; 
  int index;

  index = (start - fileStart)/RECORD_LEN;
  // We are copying the word to a temporary buffer
  while(cur < end) {
    word[i] = toupper(*cur); 
  //  printf("%c", word[i]);
    cur++;
    i++;
  }

  // We do not need to handle the last ":" in a word
  char curr_ltr = toupper(*cur);
  if(curr_ltr >= 'A' && curr_ltr <= 'Z') {
    word[i] = toupper(*cur); 
   // printf("%c", word[i]);
    i++;
  }
  
  word[i] = '\0';
  // Now length is i
  length = i;

 // fprintf(stderr, "Check new word: %s\n", word);
  wordInfo = isWordExisting(word, length); 
  if(wordInfo != NULL) {
    // Check whether this word is existing?
   // printf(" length %d, it is an existing word\n", i);
    // This word is already existing.
    addExistingWord(wordInfo, word, length, index);
  } 
  else {
   // printf(" length %d, it is a new word\n", i);
    // Add this new word to the global list.
    addNewWord(word, length, index);
  }
}

// Parse a file from the starting to the end.
void parseFile(void) {
  // Where we are paring now?
  char * current;
  char * wordStart;
  int    insideWord = 0;

  current = fileStart;
 
  while(current <= fileEnd) {
    if(insideWord == 1) {
      // If we are inside a word, we only care about
      // the end of this word.
      if(*current == ' ' || current==fileEnd) {
        if(isGoodWord(wordStart, current-1)) {
          addWordToList(wordStart, current-1);
        }

        insideWord = 0;
        wordStart = NULL;
      }
    }
    else {
      if(*current != ' ') {
        wordStart = current;
        insideWord = 1;
      //  printf("wordStart %c\n", *current);
      } 
    }
      
    current++;
  }


}

/*
 Main entry. This function may accept two different cases:
 First, we only specify inputfile, then the whole file must be analyzed.
*/
int main(int argc, char ** argv) {
  int fd;
  char * fname;
  struct stat finfo;
  size_t filesize;
  char * fdata;
 
  if(argc != 2 ) {
    fprintf(stderr, "Error, input should be like this: ./wordcount inputfile\n");
    exit(-1);
  }

  // Get the file name that we are going to parse.
  fname = argv[1];

  // Open the 
  fd = open(fname, O_RDONLY);
  if(fd < 0) {
    fprintf(stderr, "Open input file failed. \n");
    exit(-1);
  }

  // Get the file info (for file length)
  if(fstat(fd, &finfo) < 0) {
    fprintf(stderr, "fstat file failed. \n");
    exit(-1);
  }

  // mmap the file
  fdata = (char *)mmap(0, finfo.st_size + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if(fdata == MAP_FAILED) {
    fprintf(stderr, "mmap failed.\n");
    exit(-1);
  }

  // Now we can setup the start and end address now.
  fileStart = fdata;
  fileEnd = fdata + finfo.st_size;;

  // Initialize the global list.
  listInit(&gWordsList);

	while(1) {; }
  // Now we parse the file
  parseFile();

  printf("Totally, %ld words analyzed: %ld different words, %ld duplicates\n", totalWords, totalDiffWords, totalDuplicateWords);    
  return 0; 
}     
