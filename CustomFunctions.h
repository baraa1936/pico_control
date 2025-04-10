#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *CommandsList[6] = {"#BCS", "#ENT","#F12", "#DEL"};
int CommandsInAcsii[6] = {8, 13, 0, 127}; // Backspace, Enter, NMP, Delete
int Result[2] = {0, 0};

// Return the ascii value of the command
int* ReadCommands(char* KeybaordInput) {
    char StringSaved[6]; 

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

            for(int k = 0;k < 5;k++) { // loop to check if this command exist
                int Command = strncmp(StringSaved, CommandsList[k], 4);

                if(!Command) {
                    Result[0] = CommandsInAcsii[k];
                    Result[1] = i;
                    return Result;
                }; // currently this function only support end commands. using it mid-point may result a bug
                if(k == 4) {
                    return NULL;    
                }
            }

        } 
        
    }
    return 0;
}

// Delete 4 bytes (4 chars) starting from the PrefixPlace
void RemoveCommandString(char *KeyboardInput, int PrefixPlace) {
    for(int i = 0;i < 4;i++) {
        KeyboardInput[PrefixPlace + i] = '\0';
    }
}

