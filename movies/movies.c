/* Name: Derek Greene
*  OSU Email: greenede@oregonstate.edu
*  Course: CS 374 Operating Systems I
*  Assignment: Movies
*  Due Date: 2/2/2025
*  Description: This program reads a CSV file containing movie data and parses the data into a linked list.
*               The program then provides a menu for the user to select from three options: 1) show movies released in a specified year, 
*               2) show the highest rated movie for each year, and 3) show the title and year of release of all movies in a specific language.
*
*  References:
*  https://cplusplus.com/doc/tutorial/structures/
*  Code adapted from CS 374 starter code 'movies.c'. 
*/


#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <stdbool.h> 


// structs to store movie data and linked list nodes
typedef struct movie{
  char *title;
  int year;
  char *languages;
  float rating;
} movie;

typedef struct node {
  movie data;
  struct node *next;
} node;


/*
* Function to process the movie file line by line and parse the data into a linked list
* Parmeters: char* filePath, node **head
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
    printf("Processed file %s and parsed data for %d movies\n\n", filePath, lineCount - 1);
}


/*
* Function to print the movies released in a specified year
* Parameters: node *head, int year
* Returns: void
*/
  void moviesbyYear(node *head, int year){
  bool found = false;
  node *current = head;
  
  // iterate through linked list and print movies in specified year
  while (current != NULL){
    if (current->data.year == year){
      printf("%s\n", current->data.title);
      found = true;
    }
    current = current->next;
  }
  if (!found) { 
    printf("No data about movies released in the year %d\n\n", year);
  }
}


/*
* Function to print the highest rated movie for each year
* Parameters: node *head
* Returns: void
*/
void highestRated(node *head){
  for (int year = 1900; year <= 2025; year++){ 
    node *current = head;
    movie highestRated = {0};
    bool found = false;
    
    // iterate through linked list and find highest rated movie for each year
    while (current != NULL){
      if (current->data.year == year){
        if (current->data.rating > highestRated.rating){
          highestRated = current->data;
          found = true;
        }
      }
      current = current->next;
    }
    if (found){
      printf("%d %.1f %s\n", highestRated.year, highestRated.rating, highestRated.title);
    }
  }
}


/*
* Function to print the title and year of release of all movies in a specific languages
* Parameters: node *head, char *languages
* Returns: void
*/
void titleYearByLanguage(node *head, char *language){
  bool found = false; 
  node *current = head;
  
  // iterate through linked list and print movies in specified language
  while (current != NULL){
    if (strstr(current->data.languages, language)){
      printf("\n%d %s", current->data.year, current->data.title);
      found = true;
    }
    current = current->next;
  }
  if (!found){
    printf("\nNo data about movies released in %s", language);
  }
}
    

/*
* Function to display the menu and prompt the user for a choice
* Parameters: node *head
* Returns: void
*/
void menu(node *head){
  int choice = 0;
  while(choice != 4){
    printf("\n1. Show movies released in the specified year\n");
    printf("2. Show highest rated movie for each year\n");
    printf("3. Show the title and year of release of all movies in a specific language\n");
    printf("4. Exit from the program\n");
    printf("\nEnter a choice from 1 to 4: ");
    scanf("%d", &choice);
    switch(choice){
      case 1:
        printf("Enter the year for which you want to see movies: ");
        int movieYear;
        scanf("%d", &movieYear);
        moviesbyYear(head, movieYear);
        break;
      case 2:
        highestRated(head);     
        break;
      case 3:
        printf("Enter the language for which you want to see movies: ");
        char language[20];
        scanf("%s", language);
        titleYearByLanguage(head, language);
        break;
      case 4:
        printf("Exiting the program\n");
        exit(0);
      default:
        printf("You entered an incorrect choice. Try again\n");
        break;
    }
  }
}


/*
* Main function to process the movie file and display the menu
* Parameters: int argc, char **argv
* Returns: int
*/
int main ( int argc, char **argv ){
  // if no file is provided, print error message
  if (argc < 2){
    printf("You must provide the name of the file to process\n");
    printf("Example usage: ./movies movies.csv\n");

    return EXIT_FAILURE;
  }
  
  movie *movies = NULL;
  node *head = NULL;

  processMovieFile(argv[1], &head);
  menu(head);

  return EXIT_SUCCESS;
}
