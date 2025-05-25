#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *CommandsList[6] = {"#BCS", "#ENT","#F12", "#DEL", "!FCP", "#TAB"};
int CommandsInAcsii[6] = {8, 13, 0, 127, 0xFF, 9}; // Backspace, Enter, NMP, Delete, OOBE cmd, Tab
int Result[3] = {0, 0, 0}; // [0] is the ascii code of the command, [1] is the position in the string, [2] is the Keycode mode.

// Return an array of int
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
        if(KeybaordInput[i] == '!') { // commands with Keycode enabled.
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

// Delete 4 bytes (4 chars) starting from the PrefixPlace
//
// (is I can see does function isn't memory safe, it might write zero into invaild or another variable memory location if used incorrectly.)
void RemoveCommandString(char *KeyboardInput, int PrefixPlace) {
    for(int i = 0;i < 4;i++) {
        KeyboardInput[PrefixPlace + i] = '\0';
    }
}


