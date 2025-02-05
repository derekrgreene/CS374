#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <stdbool.h> 

int menu(){
  int choice;
  int subcoice;
  while(choice != 4){
    printf("1. Select file to process\n");
    printf("2. Exit the program\n");
    printf("Enter a choice 1 or 2: ");
    scanf("%d", &choice);
    switch(choice){
      case 1:
        printf("Which file you want to process?\n");
        printf("Enter 1 to pick the largest file\");
        printf("Enter 2 to pick the smallest file\n");
        printf("Enter 3 to specify the name of a file\n");
        printf("Enter a choice from 1 to 3: ");
        scanf("%d", &subchoice);
          switch(subchoice){
            case 1:
              //code
              break;
            case 2:
              //code
              break;
            case 3:
              //code
              break;
            default:
              printf("You entered an incorrect choice. Try again.\n");
              break;
          }
        }
        break;
      case 2:
        exit(0);
      default:
        printf("You entered an incorrect choice. Try again.\n");
        break;
    }
  }
}

int main(){
  menu();
  return 0;
}
