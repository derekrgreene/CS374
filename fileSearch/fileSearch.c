/* Name: Derek Greene
*  OSU Email: greenede@oregonstate.edu
*  Course: CS 374 Operating Systems I
*  Assignment: FileSearch
*  Due Date: 2/9/2025
*  Description: This program allows users to process CSV files containing movie data in the current directory. 
*               Users are provided with option to select smallest file, largest file, or enter a file name.
*               Upon selecting a valid file, the program creates a directory "greende.movies.randnum", and crates text files 
*               inside of this directory named after each year that a movie exists for in the processed CSV file. If multiple 
*               movies exist for a year, they are all listed in the corresponding text file. Directory permissions are set to rwxr-x--- 
*               and file permissions are set to rw-r-----.The CSV file containing movie data is parsesed into a linked list using the 
*               function processMovieFile, which originates from my movies.c program. 
*
*  References:
*  https://cplusplus.com/reference/cstdio//
*
*  processMovieFile function apopted from movies.c.
*/


#include <stdio.h>
#include <time.h>
#include <stdlib.h> 
#include <string.h>
#include <stdbool.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>


// structs to store movie data and linked list
typedef struct movie{
  char* title;
  int year;
  char *languages;
  float rating;
} movie;

typedef struct node{
  movie data;
  struct node *next;
} node;


/*
* Function to process the movie file line by line and parse the data into a linked list
* Parameters: char* filePath, node **head
* Returns: void
*/
void processMovieFile(char* filePath, node **head){
  char *currLine = NULL;
  size_t len = 0;
  int lineCount = 0;
  
  FILE *movieFile = fopen(filePath, "r");

  while(getline(&currLine, &len, movieFile) != -1)
    {
      if(lineCount == 0){ // skip header line
        lineCount++;
        continue;
      }
      // parse data into movie struct
      node *newNode = malloc(sizeof(node));
      movie m;
      char *token = strtok(currLine, ",");
      m.title = strdup(token);
      token = strtok(NULL, ",");
      m.year = atoi(token);
      token = strtok(NULL, ",");
      m.languages = strdup(token);
      token = strtok(NULL, ",");
      m.rating = atof(token);
      
      // add movie to linked list
      newNode->data = m;
      newNode->next = *head;
      *head = newNode;
      lineCount++;
    }
    // free memory and close file
    free(currLine);
    fclose(movieFile);
}



/*
* Function to find largest file in the current directory
* Parameters: None
* Returns: char* largestName
*/
char* largestFile(){
  DIR* currDir = opendir(".");  // open current directory
  struct dirent *entry;
  struct stat dirStat;
  int largest = 0;
  static char largestName[256];

  while((entry = readdir(currDir)) != NULL){
    if(strncmp(entry->d_name, "movies_", 7) == 0){ // check if file name starts with movies_
      if(stat(entry->d_name, &dirStat) == 0){
        if(dirStat.st_size > largest){
          largest = dirStat.st_size;
          snprintf(largestName, sizeof(largestName), "%s", entry->d_name); // copy file name to largestName
        }
      }    
    }
  }
  closedir(currDir);
  return largestName;
}


/*
* Function to find the smallest file in the current directory
* Parameters: None
* Returns: char* smallestName
*/
char* smallestFile(){
  DIR* currDir = opendir(".");
  struct dirent *entry;
  struct stat dirStat;
  int smallest = 999999999;
  static char smallestName[256];

  while((entry = readdir(currDir)) != NULL){
    if(strncmp(entry->d_name, "movies_", 7) == 0){ // check if file name starts with movies_
      if(stat(entry->d_name, &dirStat) == 0){
        if(dirStat.st_size < smallest){
        smallest = dirStat.st_size;
        snprintf(smallestName, sizeof(smallestName), "%s", entry->d_name); // copy file name to smallestName  
        }
      }
    }
  }
  closedir(currDir);
  return smallestName;
}


/*
* Function to check if a file exists 
* Parameters: char* fileName 
* Returns: int
*/
int fileExist(char* fileName){
  struct stat buffer;
  return (stat(fileName, &buffer) == 0); // return 1 if file exists, 0 otherwise
}


/*
* Function to create text files for each year a movie exists in linked list 
* Parameters: node *head, char *dirName
* Returns: void
*/
void yearFiles(node *head, char *dirName){
  node *curr = head;
  FILE *yearFile = NULL;
  char yearFileName[256];

  while(curr != NULL){
    sprintf(yearFileName, "%s/%d.txt", dirName, curr->data.year); // create file name
    yearFile = fopen(yearFileName, "a");
    fprintf(yearFile, "%s\n", curr->data.title); // write movie title to file
    fclose(yearFile);
    curr = curr->next; // move to next node in linked list
  }
  
  node *temp = head;
  while (temp != NULL){
    sprintf(yearFileName, "%s/%d.txt", dirName, temp->data.year); // create file name
    chmod(yearFileName, 0640); // set file permissions to rw-r----
    temp = temp->next;
  }
}


/*
* Function to process selected file and create directory and text files 
* Parameters: char* fileName 
* Returns: void
*/
void processFile(char* fileName){
  srand(time(0)); // seed random number 
  int random = rand() % 100000; 
  char *onid = "greenede";
  char dirName[256];

  sprintf(dirName, "%s.movies.%d", onid, random); // create directory name
  mkdir(dirName, 0750); // create directory with permissions rwxr-x---
  printf("\nCreated directory with name %s\n\n", dirName);

  node *head = NULL;
  processMovieFile(fileName, &head); // parse movie data into linked list
  yearFiles(head, dirName); // create text files for each year a movie exists

}


/*
* Function to display menu and process user input
* Parameters: None
* Returns: void
*/
void menu(){
  int choice;
  int subchoice;
  char fileName[256];

  while(choice != 2){
    printf("1. Select file to process\n");
    printf("2. Exit the program\n");
    printf("\nEnter a choice 1 or 2: ");
    scanf("%d", &choice);

    switch(choice){
      case 1:
        while (1){
          printf("\nWhich file you want to process?\n");
          printf("Enter 1 to pick the largest file\n");
          printf("Enter 2 to pick the smallest file\n");
          printf("Enter 3 to specify the name of a file\n");
          printf("\nEnter a choice from 1 to 3: ");
          scanf("%d", &subchoice);

          switch(subchoice){
            case 1: {
              char* largestF = largestFile();
              printf("\nNow processing the chosen file named %s", largestF);
              processFile(largestF);
              break;
            }
            case 2: {
              char* smallestF = smallestFile();
              printf("\nNow processing the chosen file named %s", smallestF);
              processFile(smallestF);
              break;
            }
            case 3: {
              printf("Enter the complete file name: ");
              scanf("%s", fileName);

              if (fileExist(fileName)){
                printf("Now processing the chosen file named %s", fileName);
                processFile(fileName);
                break;
              } else {
                printf("\nThe file %s was not found. Try again.\n", fileName);
                continue;
              }
            }
            default:
              printf("You entered an incorrect choice. Try again.\n");
              continue;
          }
          break;  // exit submenu 
        }
        break;  // exit main menu
      case 2:
        exit(0); // exit program
      default:
        printf("You entered an incorrect choice. Try again.\n");
        break;
    }
  }
}


/* 
* Main function 
* Parameters: None
* Returns: int
*/
int main(){
  menu(); // display menu
  return 0;
}
