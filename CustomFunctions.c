#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
 * recives a string and return the values between 
 */
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
  
 
 // Delete 4 bytes (4 chars) starting from the PrefixPlace
void RemoveCommandString(char *KeyboardInput, int PrefixPlace) {
    for(int i = 0;i < 4;i++) {
        KeyboardInput[PrefixPlace + i] = '\0';
    }
}

const char const *CommandsList[6] = {"#BCS", "#ENT","#F12", "#DEL", "!FCP"};
const int CommandsInAcsii[6] = {8, 13, 0, 127, 0xFF}; // Backspace, Enter, NMP, Delete
int Result[3] = {0, 0, 0};


int* ReadCommands(char* KeybaordInput) {
    char StringSaved[6]; 

    int SizeCommandsList = sizeof(CommandsList) / sizeof(CommandsList[0]);
    int FoundCommandPrefix = 0;
    for(int i = 0;KeybaordInput[i] != '\0';i++) {

        if((KeybaordInput[i] == '#')) { // if the Prefix has been found
            FoundCommandPrefix = 1;
            for(int k = 0;k < 5;k++) { // loop to save the command into StringSaved
                if(k == 4) {
                    StringSaved[k] = '\0';
                    continue;
                }
                StringSaved[k] = KeybaordInput[i + k];
            }

            for(int k = 0;k < SizeCommandsList;k++) { // loop to check if this command exist
                int Command = strncmp(StringSaved, CommandsList[k], 4);
                if(!Command) {
                    printf("F %d", CommandsInAcsii[k]);
                    Result[0] = CommandsInAcsii[k];
                    Result[1] = i;
                    Result[2] = 0;

                    return Result;
                }; // currently this function only support end commands. using it mid-point may result a bug
                if(k == SizeCommandsList - 1) {
                    return NULL;    
                }
            }

        } 
        if(KeybaordInput[i] == '!') {
            FoundCommandPrefix = 1;
            for(int k = 0;k < 5;k++) { // loop to save the command into StringSaved
                if(k == 4) {
                    StringSaved[k] = '\0';
                    continue;
                }
                StringSaved[k] = KeybaordInput[i + k];
            }

            for(int k = 0;k < SizeCommandsList;k++) { // loop to check if this command exist
                int Command = strncmp(StringSaved, CommandsList[k], 4);

                if(!Command) {
                    
                    Result[0] = 0;
                    Result[1] = i;
                    Result[2] = 1;
                    return Result;
                }; // currently this function only support end commands. using it mid-point may result a bug
                if(k == SizeCommandsList - 1) {
                    return NULL;    
                }
            }

        } 
        
    }
    return 0;
}

int ReadMainFunction(char* KeyboardInput) { // return Eaother 1 or 0, 1 is mouse , 0 is for keyboard
    char FunctionsName[2][5] = {"Kbd()", "Mus()"};
    char Input_Function[5];
    strncpy(KeyboardInput, Input_Function, 4);
    
    
    if(!strncmp(FunctionsName[0], Input_Function, 4)) { // Kbd "Keyboard Function"
        return 0;
    }

    if(!strncmp(FunctionsName[1], Input_Function, 4)) { // Mus "Mouse Function"
        return 1;
    }
    
    return -1;
}

void LeftShift(char* KeyboardInput) {
    char NewString[128] = "";
    for(int i = 0;KeyboardInput[i] != '\0';i++) {
        NewString[i] = KeyboardInput[i + 1];
    };
    KeyboardInput = NewString;
}