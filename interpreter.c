#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

long int addressLength;  // Address length
long int *dataMemory;    // Data array
long int *programMemory; // Program array

long int getAddressLength(char *); // Find address length for integer operations
void initMemory();                 // Allocate space for data and program memory
void fillData(FILE *);             // Initialize the data array
void fillProgram(FILE *);          // Initialize the program array
void Display(int, int, char *);    // Display an portion of one of the arrays

int main()
{
    FILE *input_file; // Input file

    int IP = 0;
    long int instruction;
    int opcode, op1, op2, op3;

    // Initialize the address length
    addressLength = getAddressLength("output.txt");

    // Use address length to allocate memory space
    initMemory();

    // Open the numeric code
    input_file = fopen("output.txt", "r");

    fillData(input_file);    // Fill data array
    fillProgram(input_file); // Fill program array

    opcode = programMemory[IP] / (int)pow(10, (3 * addressLength));
    while (1)
    {
        // Compute the three operands using division and modulo
        op1 = labs(labs(programMemory[IP]) / (int)pow(10, (2 * addressLength)) - abs(opcode) * (int)pow(10, addressLength));
        op3 = labs(labs(programMemory[IP]) % (int)pow(10, (addressLength)));
        op2 = labs((labs(programMemory[IP]) % (int)pow(10, (2 * addressLength)) - op3) / (int)pow(10, addressLength));

        // Increment the IP
        IP += 1;

        // Execute an operation
        switch (opcode)
        {
        case +0: // Assignment operation
            dataMemory[op3] = dataMemory[op1];
            break;
        case +1: // Sum operation
            dataMemory[op3] = dataMemory[op1] + dataMemory[op2];
            break;
        case +2: // Multiplication operation
            dataMemory[op3] = dataMemory[op1] * dataMemory[op2];
            break;
        case +3: // Square operation
            dataMemory[op3] = dataMemory[op1] * dataMemory[op1];
            break;
        case +4: // Equals operation
            if (dataMemory[op1] == dataMemory[op2])
                IP = op3;
            break;
        case +5: // Greater than or equal operation
            if (dataMemory[op1] >= dataMemory[op2])
                IP = op3;
            break;
        case +6: // Read from array operation
            dataMemory[op3] = dataMemory[op1 + dataMemory[op2]];
            break;
        case +7: // Iterate/Jump operation
            dataMemory[op1] += 1;
            if (dataMemory[op1] < dataMemory[op2])
                IP = op3;
            break;
        case +8: // Input operation
            if (!feof(input_file))
                fscanf(input_file, "%ld\n", &dataMemory[op3]);
            break;
        case +9: // Termination operation
            fclose(input_file);
            return 0;
            break;
        case -1: // Substraction operation
            dataMemory[op3] = dataMemory[op1] - dataMemory[op2];
            break;
        case -2: // Division operation
            dataMemory[op3] = dataMemory[op1] / dataMemory[op2];
            break;
        case -3: // Square root operation
            dataMemory[op3] = (int)pow(dataMemory[op1], 0.5);
            break;
        case -4: // Not equal operation
            if (dataMemory[op1] != dataMemory[op2])
                IP = op3;
            break;
        case -5: // Less than operation
            if (dataMemory[op1] < dataMemory[op2])
                IP = op3;
            break;
        case -6: // Write to array operation
            dataMemory[op2 + dataMemory[op3]] = dataMemory[op1];
            break;
        case -8: // Output operation
            printf(">> %ld\n", dataMemory[op1]);
            break;
        }

        // Compute next opcode
        opcode = programMemory[IP] / (int)pow(10, (3 * addressLength));
    }
}

long int getAddressLength(char *file)
{
    // Open the input file, take first line
    FILE *input_file = fopen(file, "r");
    char temp[20];
    fscanf(input_file, "%s\n", temp);
    fclose(input_file);

    // Return the address length from the entire line length
    return (strlen(temp) - 2) / 3;
}

void initMemory()
{
    // Allocate space using address length
    long int size = pow(10, addressLength);
    dataMemory = malloc(sizeof(long int) * size);
    programMemory = malloc(sizeof(long int) * size);
    for (int i = 0; i < size; i++) {
        dataMemory[i] = 0 ;
        programMemory[i] = 0 ;
    }
}

void fillData(FILE *file)
{
    // Read through the input file until a +9 instruction
    long int current;
    int idx = 0;
    fscanf(file, "%ld\n", &current);
    while (current % 9 != 0 || current == 0)
    {
        dataMemory[idx] = current;
        fscanf(file, "%ld\n", &current);
        idx++;
    }
}

void fillProgram(FILE *file)
{
    // Read through the input file until a +9 instruction
    long int current;
    int idx = 0;
    fscanf(file, "%ld\n", &current);
    while (current % 9 != 0 || current == 0)
    {
        programMemory[idx] = current;
        fscanf(file, "%ld\n", &current);
        idx++;
    }
    programMemory[idx] = current;
    fscanf(file, "%ld\n", &current);
}

void Display(int startindex, int endindex, char *which)
{
    if (strcasecmp("data", which) == 0)
    {
        for (int i = startindex; i < endindex + 1; i++)
            printf("%ld ", dataMemory[i]);
    }
    else if (strcasecmp("program", which) == 0)
    {
        for (int i = startindex; i < endindex + 1; i++)
            printf("%09ld ", programMemory[i]);
    }
    else
        printf("INVALID!");
}