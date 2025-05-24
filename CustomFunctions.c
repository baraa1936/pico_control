#include <stdio.h>
#include <stdlib.h>

void ReFormatingString(char* StringPosition, uint16_t* MousePosition) {
    int i = 0;
    char* StringSpliter = strtok(StringPosition, ","); // Split String into Tokens
    char* endptr;
    while(StringSpliter != NULL) {
      MousePosition[i] = strtol(StringSpliter, &endptr, 10); //  String to Integer
      i++;
      // printf("%s", StringSpliter);
      StringSpliter = strtok(NULL, ",");
    }
    // printf("\nX%d, Y%d\n", MousePosition[0], MousePosition[1]);
  } 
  
 
 