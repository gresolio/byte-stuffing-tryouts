#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Consistent Overhead Byte Stuffing (COBS) and "Reduced" mod (COBS/R)
// https://github.com/cmcqueen/cobs-c
#include "cobs/cobs.h"
#include "cobs/cobsr.h"

// TCOBS - experimental byte stuffing algorithm combined with run-length encoding.
// The name is misleading, TCOBS is not "consistent overhead" like the original COBS.
// https://github.com/rokath/TCOBS
#include "tcobs/tcobs.h"

// Run Length Encoding (RLE) Discussion and Implementation https://michaeldipperstein.github.io/rle.html
// RLE Compression https://moddingwiki.shikadi.net/wiki/RLE_Compression
#include "rle/rle.h"

#define INPUT_FILE "tricePackagesGrepSortUniqPlainNoDescriptor.txt"

#define MAX_LINE_LENGTH 2048
#define MAX_DATA_LENGTH 1024

void print_arr(const char* prefix, const uint8_t* src, size_t len)
{
    printf("%s[%zu]: ", prefix, len);

    for (size_t i = 0; i < len; i++)
        printf("%02X ", src[i]);

    printf("\n");
}

void encode_via_COBS(const uint8_t* src, size_t len)
{
    size_t out_len;
    uint8_t output[MAX_DATA_LENGTH];

    cobs_encode_result result = cobs_encode(output, MAX_DATA_LENGTH, src, len);
    out_len = result.out_len;

    print_arr("COBS ", output, out_len);
}

void encode_via_COBSR(const uint8_t* src, size_t len)
{
    uint8_t output[MAX_DATA_LENGTH];

    cobsr_encode_result result = cobsr_encode(output, MAX_DATA_LENGTH, src, len);
    if (result.status != COBSR_ENCODE_OK) {
        printf("cobsr_encode error: %d\n", result.status);
        return;
    }

    print_arr("COBSR", output, result.out_len);
}

void encode_via_TCOBS(const uint8_t* src, size_t len)
{
    size_t out_len;
    uint8_t output[MAX_DATA_LENGTH];
    out_len = TCOBSEncode(output, src, len);

    print_arr("TCOBS", output, out_len);
}

void encode_via_RLE(const uint8_t* src, size_t len)
{
    uint8_t output[MAX_DATA_LENGTH];

    rle_result result = rle_encode(output, MAX_DATA_LENGTH, src, len);
    if (result.status != RLE_OK) {
        printf("rle_encode error: %d\n", result.status);
        return;
    }

    print_arr("RLE  ", output, result.out_len);
}

void run_tests(const uint8_t* src, size_t len)
{
    print_arr("INPUT", src, len);

    encode_via_COBS(src, len);
    encode_via_COBSR(src, len);
    encode_via_TCOBS(src, len);
    encode_via_RLE(src, len);

    printf("\n");
}

int read_hex_line(uint8_t* dst_ptr, size_t* dst_len, const char* src_line)
{
    uint32_t value;
    int32_t bytes_now;
    int32_t bytes_consumed = 0;

    *dst_len = 0;
    while (sscanf(src_line + bytes_consumed, "%2x%n", &value, &bytes_now) > 0) {
        bytes_consumed += bytes_now;

        if (value > 0xFF) {
            // Expecting only byte values
            return -1;
        }

        if (*dst_len >= MAX_DATA_LENGTH) {
            // Input line contains more values than MAX_DATA_LENGTH
            return -2;
        }

        dst_ptr[*dst_len] = (uint8_t)value;
        *dst_len = *dst_len + 1;
    }

    if (0 == *dst_len) {
        // Input line doesn't contain valid values to read
        return -3;
    }

    return 0;
}

int main()
{
    FILE* input_file;
    char line_buf[MAX_LINE_LENGTH];

    size_t data_len;
    uint8_t data_buf[MAX_DATA_LENGTH];

    input_file = fopen(INPUT_FILE, "r");
    if (input_file == NULL) {
        perror("fopen() failed");
        return EXIT_FAILURE;
    }

    while (fgets(line_buf, MAX_LINE_LENGTH, input_file)) {
        if (0 == read_hex_line(data_buf, &data_len, line_buf)) {
            run_tests(data_buf, data_len);
        }
    }

    fclose(input_file);
    return EXIT_SUCCESS;
}
