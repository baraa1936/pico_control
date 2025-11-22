
// Return the ascii value of the command

// Get Mouse Position and put it into MousePosition From StringPosition by parsing it
void ReFormatingString(char* StringPosition, uint16_t* MousePosition); 


void RemoveCommandString(char *KeyboardInput, int PrefixPlace);

int* ReadCommands(char* KeybaordInput);

int ReadMainFunction(char* KeybaordInput); // return 0 or 1 for keyboard and mouse or -1 when error