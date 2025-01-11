/* Name: Derek Greene
 * OSU Email: greenede@oregonstate.edu
 * Course: CS374 - Operating Systems I
 * Assignment: Programming Assignment 1 Basic Formulas
 * Due Date: 1/19/2025
 * Description: Program to calculate total surface area and volume of serveral spherical segments gien the sizes of their radius and heights. 
 */


#include <stdio.h>
#include <math.h>
#include <stdbool.h>


int n = 0;
float r;
float a;
float b;
float h;
float ha;
float hb;
float tpsa;
float btsa;
float lsa;
float tsa;
float vol;
float avgsa;
float avgvol;
float sum1;
float sum2;
float pi = 3.14159265359;
bool invalid = false;


int getSegmentData();


int getSegmentData() {
/* Function to get and validate radius and heights, also calculates total and avg area/volume
 * Parameters: int n
 * Returns: None
 */
    while (n < 2 || n > 10) {
        printf("Enter the number of spherical segments to evaluate [2-10]: ");
        scanf("%d", &n);
    }

    for (int i = 1; i <= n; i++) {
        printf("Obtaining data for spherical sgement number %d\n", i);
        printf("What is the radius of the sphere (R)?: ");
        scanf("%f", &r);

        printf("What is the height of the top area of the spherical segment (ha)?: ");
        scanf("%f", &ha);

        printf("What is the height of the bottom area of the spherical segment (hb)?: ");
        scanf("%f", &hb);

        printf("Entered data: R = %.2f ha = %.2f hb = %.2f.\n", r, ha, hb);
        
        if (r < 0 || ha < 0 || hb < 0) {
            printf("Invalid Input: R = %.2f ha = %.2f hb = %.2f. Numbers must be positive real values.\n", r, ha, hb);
            invalid = true;
        }
        if (r < ha) {
            printf("Invalid Input: R = %.2f ha = %.2f. R must be greater than or equal to ha.\n", r, ha);
            invalid = true;
        }
        if (r < hb) {
            printf("Invalid Input: R = %.2f hb = %.2f. R must be greater than or equal to hb.\n", r, hb);
            invalid = true;
        }
        if (ha < hb) {
            printf("Invalid Input: ha = %.2f hb = %.2f. ha must be greater than or equal to hb.\n", ha, hb);
            invalid = true;
        }
        if (invalid && i > 0) {
            i--;
            invalid = false;

        } else {
            a = sqrt((r * r) - (ha * ha));
            b = sqrt((r * r) - (hb * hb));
            h = ha - hb;    
            tpsa = pi * b * b;
            btsa = pi * a * a;
            lsa = 2 * pi * r * h;
            tsa = tpsa + btsa + lsa;
            vol = 1.0/6.0 * pi * h * (3 * (a * a) + 3 * (b * b) + (h * h));
            sum1 += tsa;
            sum2 += vol;

            printf("Total Surface Area = %.2f Volume = %.2f.\n", tsa, vol);
            
        }
    }
    avgsa = sum1 / n;
    avgvol = sum2 / n;

    printf("Total average results:\n Average Surface Area = %.2f Average Volume = %.2f.", avgsa, avgvol);
}


int main(void) {
    getSegmentData();
    
}
