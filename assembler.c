#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#define HASHSIZE 200  // Default hashtable size
#define STRINGSIZE 10 // Default operand string size

// HashTable struct for quick lookup
typedef struct HashTable
{
    char key[STRINGSIZE];
    char out[STRINGSIZE];
} HashTable;

int addressLength;   // Address length (as it relates to memory size => 10^addressLength)
FILE *inputFile;     // Input file
FILE *intermFile;    // Intermediate stage file (with UNREPLACED LABELS)
FILE *outputFile;    // Final output file
HashTable *switcher; // Opcode hashtable
HashTable *symbols;  // Symbol hashtable for variable names
HashTable *labels;   // Label hashtable for label addresses
int ERROR_COUNT;     // Error count if assembly fails

HashTable *initOpHashTable();             // To initiate opcode hashtable and malloc others
void insert(HashTable *, char *, char *); // Insert into any hashtable
int hash(char *);                         // Hash function for strings
void initData();                          // Initialize data labels and memory addresses
int getSymbol(char *);                    // Hash search in symbols
int getLabel(char *);                     // Hash search in labels
int getSwitcher(char *);                  // Hash search in opcodes
void initProgram();                       // Initialize the program part by saving labels and replacing symbols
void adjustLabels();                      // Swap labels with their program addresses
void makeBeautifulCode();                 // Make beautiful code with spaces between addresses
void initInput();                         // Initialize input part by removing spaces

int isAddress(char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (!(str[i] >= '0' && str[i] <= '9'))
            return 0;
    }
    return 1;
}

int main(void)
{
    inputFile = fopen("assemblycode.txt", "r");
    intermFile = fopen("intermFile.txt", "w");
    outputFile = fopen("output.txt", "w");
    switcher = initOpHashTable();
    ERROR_COUNT = 0;

    initData();
    initProgram();
    initInput();
    adjustLabels();

    if (ERROR_COUNT)
    {
        printf("Unsuccessful assembly due to %d errors!\n", ERROR_COUNT);
        return 0;
    }

    printf("Successful assembly!\n");
    makeBeautifulCode();

    return 0;
}

void initData()
{
    int lineNumber = 0; // Initialize line number for data memory addressing
    char opcode[STRINGSIZE], op[3][STRINGSIZE];
    char temp[STRINGSIZE]; // For int to string conversions
    int words;             // Number of words to be allocated

    // Scan first data part line
    fscanf(inputFile, "%s %s %s %s\n", opcode, op[0], op[1], op[2]);

    // Get address length (it helps with leading zeros)
    addressLength = strlen(op[2]); // Eg. if third operand is 123, then address length is 3

    // Do loop until stop opcode
    while (strcmp(opcode, "+9"))
    {   
        // Verify if opcode is Valid
        if (strcmp(opcode, "DEC"))
        {
            printf("ERROR: %s INVALID OPCODE AT LINE >> %s %s %s %s\n", opcode, opcode, op[0], op[1], op[2]);
            ERROR_COUNT++;
        }
        // Get number of words
        sscanf(op[1], "%d", &words);

        // Convert lineNumber to string with leading zeros, then add to symbols hashtable
        sprintf(temp, "%0*d", addressLength, lineNumber);
        insert(symbols, op[0], temp);

        // Scan value stored in variable and print initialization for all concerned words
        fscanf(inputFile, "%s %s %s %s\n", opcode, op[0], op[1], op[2]);
        for (int i = 0; i < words; i++)
        {
            fprintf(intermFile, "%s%s%s%s\n", opcode, op[0], op[1], op[2]);
            lineNumber++;
        }

        // Scan next declaration
        fscanf(inputFile, "%s %s %s %s\n", opcode, op[0], op[1], op[2]);
    }

    // Print the stop code to signify end of data part
    fprintf(intermFile, "%s%s%s%s\n", opcode, op[0], op[1], op[2]);
}

void initProgram()
{
    char opcode[STRINGSIZE], op[3][STRINGSIZE]; // To tokenize line of instruction
    char temp[STRINGSIZE];                      // For int to string conversions
    int n, junk1, junk2;                        // For hashtable lookup
    int lineNumber = 0;                         // For program memory addressing

    // Scan first program part line
    fscanf(inputFile, "%s %s %s %s\n", opcode, op[0], op[1], op[2]);

    // Do loop until stop opcode
    while (strcmp(opcode, "+9"))
    {

        // Check if opcode is valid
        n = getSwitcher(opcode);
        if (n >= 0) // If opcode is valid meaning the opcode was found in the switcher
            strcpy(opcode, switcher[n].out);
        else
        {
            // Opcode is invalid, print error prompt
            printf("ERROR: %s INVALID OPCODE AT LINE >> %s %s %s %s\n", opcode, opcode, op[0], op[1], op[2]);
            ERROR_COUNT++;
        }

        // If the opcode is a label declaration
        if (strcmp(opcode, "-7") == 0)
        {
            sprintf(temp, "%0*d", addressLength, lineNumber);
            insert(labels, op[0], temp);
            strcpy(op[0], temp);
        }
        else
        {
            // Check if operands 1 and 2 are symbols, if yes then swap with their addresses
            for (int i = 0; i < 2; i++)
            {
                n = getSymbol(op[i]);
                if (n >= 0)
                    strcpy(op[i], symbols[n].out);
                else if (isAddress(op[i]))
                { /* DO NOTHING IF IT IS AN ADDRESS ALREADY*/
                }
                else
                {
                    printf("SYMBOL ERROR: %s DOES NOT EXIST AT LINE >> %s %s %s %s\n", op[i], opcode, op[0], op[1], op[2]);
                    ERROR_COUNT++;
                }
            }

            // Check if operand 3 should be a label or a symbol
            sscanf(opcode, "%d", &n);
            if (!(n == 4 || n == -4 || n == 5 || n == -5 || n == +7))
            {
                // Then the third operand should be a symbol
                n = getSymbol(op[2]);
                // A wrong symbol error will be handled in the label adjustment phase
                if (n >= 0)
                    strcpy(op[2], symbols[n].out);
                else if (isAddress(op[2]))
                { /* DO NOTHING IF IT IS AN ADDRESS ALREADY*/
                }
                else
                {
                    printf("SYMBOL ERROR: %s DOES NOT EXIST AT LINE >> %s %s %s %s\n", op[2], opcode, op[0], op[1], op[2]);
                    ERROR_COUNT++;
                }
            } // Else, the third operand should be a label that will be handled later

            // Print the modified instruction to the intermediate file
            fprintf(intermFile, "%s %s %s %s\n", opcode, op[0], op[1], op[2]);
            lineNumber++;
        }

        // Scan next line
        fscanf(inputFile, "%s %s %s %s\n", opcode, op[0], op[1], op[2]);
    }

    // Print stop code
    fprintf(intermFile, "%s %s %s %s\n", opcode, op[0], op[1], op[2]);
}

void initInput()
{
    char opcode[STRINGSIZE], op[3][STRINGSIZE];

    // Scan first input line
    fscanf(inputFile, "%s %s %s %s\n", opcode, op[0], op[1], op[2]);

    // Write it without spaces to intermediate file
    while (strcmp(opcode, "+9"))
    {
        fprintf(intermFile, "%s%s%s%s\n", opcode, op[0], op[1], op[2]);
        fscanf(inputFile, "%s %s %s %s\n", opcode, op[0], op[1], op[2]);
    }
    fprintf(intermFile, "%s%s%s%s\n", opcode, op[0], op[1], op[2]);

    // Close both files
    fclose(inputFile);
    fclose(intermFile);
}

void adjustLabels()
{
    char opcode[STRINGSIZE], op[3][STRINGSIZE], temp[100]; // To tokenize line of instruction
    int n, junk;
    FILE *readInterm = fopen("intermFile.txt", "r");
    fscanf(readInterm, "%s\n", temp);

    // Read data part entirely and write it to final output
    while (temp[0] != '+' || temp[1] != '9')
    {
        fprintf(outputFile, "%s\n", temp);
        fscanf(readInterm, "%s\n", temp);
    }
    // Print terminating stop
    fprintf(outputFile, "%s\n", temp);

    fscanf(readInterm, "%s %s %s %s\n", opcode, op[0], op[1], op[2]);
    // Read code part entirely
    while (strcmp(opcode, "+9"))
    {
        sscanf(opcode, "%d", &n);
        if (n == 4 || n == -4 || n == 5 || n == -5 || n == +7)
        {
            n = getLabel(op[2]);
            if (n >= 0)
                strcpy(op[2], labels[n].out);
            else
            {
                printf("LABEL ERROR: %s DOES NOT EXIST AT LINE >> %s %s %s %s\n", op[2], opcode, op[0], op[1], op[2]);
                ERROR_COUNT++;
            }
        }
        fprintf(outputFile, "%s%s%s%s\n", opcode, op[0], op[1], op[2]);
        fscanf(readInterm, "%s %s %s %s\n", opcode, op[0], op[1], op[2]);
    }
    // Print the terminator and the stop instruction
    fprintf(outputFile, "%s%s%s%s\n", opcode, op[0], op[1], op[2]);
    fscanf(readInterm, "%s %s %s %s\n", opcode, op[0], op[1], op[2]);
    fprintf(outputFile, "%s%s%s%s\n", opcode, op[0], op[1], op[2]);

    // Read input part entirely and write it to final output
    fscanf(readInterm, "%s\n", temp);

    while (temp[0] != '+' || temp[1] != '9')
    {
        fprintf(outputFile, "%s\n", temp);
        fscanf(readInterm, "%s\n", temp);
    }
    // Print terminating stop
    fprintf(outputFile, "%s\n", temp);

    fclose(outputFile);
    fclose(readInterm);
    remove("intermFile.txt");
}

HashTable *initOpHashTable()
{
    char *switcher[][2] = {{"ASG", "+0"}, {"ADD", "+1"}, {"MUL", "+2"}, {"SQR", "+3"}, {"EQL", "+4"}, {"GTE", "+5"}, {"RDA", "+6"}, {"ITJ", "+7"}, {"INP", "+8"}, {"STP", "+9"}, {"SUB", "-1"}, {"DIV", "-2"}, {"SQT", "-3"}, {"NEQ", "-4"}, {"LSS", "-5"}, {"WTA", "-6"}, {"LBL", "-7"}, {"OUT", "-8"}};
    // FOR HASHTABLE INITIALIZATIONS
    HashTable *myHashTable = malloc(HASHSIZE * sizeof(HashTable));
    symbols = malloc(HASHSIZE * sizeof(HashTable));
    labels = malloc(HASHSIZE * sizeof(HashTable));
    for (int i = 0; i < HASHSIZE; i++)
    {
        symbols[i].key[0] = '\0';
        symbols[i].out[0] = '\0';
        labels[i].key[0] = '\0';
        labels[i].out[0] = '\0';
        myHashTable[i].key[0] = '\0';
        myHashTable[i].out[0] = '\0';
    }
    // END OF INITIALIZATIONS
    for (int i = 0; i < 18; i++)
    {
        insert(myHashTable, switcher[i][0], switcher[i][1]);
    }
    return myHashTable;
}

int getSymbol(char *key)
{
    int hashIndex = hash(key);
    while (symbols[hashIndex].key[0] != '\0')
    {
        if (strcmp(symbols[hashIndex].key, key) == 0)
            return hashIndex;
        ++hashIndex;
        hashIndex %= HASHSIZE;
    }
    return -1;
}

int getLabel(char *key)
{
    int hashIndex = hash(key);
    while (labels[hashIndex].key[0] != '\0')
    {
        if (strcmp(labels[hashIndex].key, key) == 0)
            return hashIndex;
        ++hashIndex;
        hashIndex %= HASHSIZE;
    }
    return -1;
}
int getSwitcher(char *key)
{
    int hashIndex = hash(key);
    while (switcher[hashIndex].key[0] != '\0')
    {
        if (strcmp(switcher[hashIndex].key, key) == 0)
            return hashIndex;
        ++hashIndex;
        hashIndex %= HASHSIZE;
    }
    return -1;
}

void insert(HashTable *myHashTable, char *key, char *out)
{
    int n = hash(key);
    int collisions = 0, tablesize = 20;
    if (myHashTable[n].key[0] == '\0')
    {
        strcpy(myHashTable[n].key, key);
        strcpy(myHashTable[n].out, out);
    }
    else
    {
        collisions++;
        int j;
        for (j = n; myHashTable[j].key[0] != '\0' && j < tablesize; j++)
            ;
        if (j == tablesize)
        {
            j = 0;
            for (; myHashTable[j].key[0] != '\0' && j < n; j++)
                ;
            if (j < n)
            {
                strcpy(myHashTable[j].key, key);
                strcpy(myHashTable[j].out, out);
            }
            else
                return;
        }
        else
        {
            strcpy(myHashTable[j].key, key);
            strcpy(myHashTable[j].out, out);
        }
    }
}

int hash(char *word)
{
    int key = 0;
    for (int i = 0; word[i] != '\0'; i++)
        key = (int)word[i] + 3 * key;
    return key % HASHSIZE;
}

void makeBeautifulCode()
{
    FILE *beautiful = fopen("beautifuloutput.txt", "w");
    FILE *final = fopen("output.txt", "r");
    char temp[100];
    int n;

    while (!feof(final))
    {
        // Take an entire line
        fscanf(final, "%s\n", temp);
        // Print the opcode first and a space
        fprintf(beautiful, "%c%c ", temp[0], temp[1]);
        n = strlen(temp);
        // Print a space every nth character depending on the address length
        for (int i = 2; i < n; i = i + addressLength)
        {
            for (int j = 0; j < addressLength; j++)
                fprintf(beautiful, "%c", temp[i + j]);
            fprintf(beautiful, " ");
        }
        fprintf(beautiful, "\n");
    }

    fclose(final);
    fclose(beautiful);
}