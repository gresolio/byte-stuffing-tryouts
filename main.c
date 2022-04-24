#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Consistent Overhead Byte Stuffing - Reduced (COBS/R)
// https://github.com/cmcqueen/cobs-c
#include "cobs/cobsr.h"

// TCOBS - experimental COBS version combined with run-length encoding
// https://github.com/rokath/TCOBS
#include "tcobs/tcobs.h"

#define INPUT_FILE "tricePackagesGrepSortUniqPlainNoDescriptor.txt"
#define MAX_LINE_LENGTH 2048
#define MAX_DATA_LENGTH 1024

void PrintArr(const char* prefix, const uint8_t* data, uint32_t length)
{
    printf("%s[%d]: ", prefix, length);
    for (uint32_t i = 0; i < length; i++) {
        printf("%02x ", data[i]);
    }
    printf("\n");
}

void EncodeViaCOBSR(const uint8_t* data, uint32_t length)
{
    uint32_t outputLen;
    uint8_t output[MAX_DATA_LENGTH];

    cobsr_encode_result result = cobsr_encode(output, MAX_DATA_LENGTH, data, length);
    outputLen = result.out_len;

    PrintArr("COBSR", output, outputLen);
}

void EncodeViaTCOBS(const uint8_t* data, uint32_t length)
{
    uint32_t outputLen;
    uint8_t output[MAX_DATA_LENGTH];
    outputLen = TCOBSEncode(output, data, length);

    PrintArr("TCOBS", output, outputLen);
}

void RunTests(const uint8_t* data, uint32_t length)
{
    PrintArr("INPUT", data, length);

    EncodeViaCOBSR(data, length);
    EncodeViaTCOBS(data, length);

    printf("\n");
}

int ReadHexLine(uint8_t* data, uint32_t* length, const char* line)
{
    uint32_t value;
    int32_t bytesNow;
    int32_t bytesConsumed = 0;

    *length = 0;
    while (sscanf(line + bytesConsumed, "%2x%n", &value, &bytesNow) > 0) {
        bytesConsumed += bytesNow;

        if (value > 0xFF) {
            // Expecting only byte values
            return -1;
        }

        if (*length >= MAX_DATA_LENGTH) {
            // Input line contains more values than MAX_DATA_LENGTH
            return -2;
        }

        data[*length] = (uint8_t)value;
        *length = *length + 1;
    }

    if (0 == *length) {
        // Input line doesn't contain valid values to read
        return -3;
    }

    return 0;
}

int main()
{
    FILE* inputFile;
    char lineBuffer[MAX_LINE_LENGTH];

    uint32_t dataLen;
    uint8_t dataBuf[MAX_DATA_LENGTH];

    inputFile = fopen(INPUT_FILE, "r");
    if (inputFile == NULL) {
        perror("fopen() failed");
        return EXIT_FAILURE;
    }

    while(fgets(lineBuffer, MAX_LINE_LENGTH, inputFile)) {
        if (0 == ReadHexLine(dataBuf, &dataLen, lineBuffer)) {
            RunTests(dataBuf, dataLen);
        }
    }

    fclose(inputFile);
    return EXIT_SUCCESS;
}
