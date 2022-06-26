#include "rle.h"
#include <stdbool.h>

// -128 to -1 --> Repeat the next symbol 1 - n times
#define MIN_RUN 2
#define MAX_RUN 129

// 0 to 127 --> Copy the next n + 1 symbols verbatim
#define MAX_COPY 128

bool is_out_of_bounds(const size_t bytes_required, const uint8_t* src_ptr, const uint8_t* src_end_ptr)
{
    size_t bytes_available = src_end_ptr - src_ptr;
    if (bytes_available < bytes_required)
        return true;

    return false;
}

void write_run_segment(const size_t n, uint8_t** dst_write_ptr, const uint8_t** segment_start, const uint8_t** segment_end)
{
    *(*dst_write_ptr)++ = (uint8_t)(1 - n); // Write run header
    *(*dst_write_ptr)++ = **segment_start;  // Write run symbol
    *segment_start = *segment_end;
}

void write_copy_segment(const size_t n, uint8_t** dst_write_ptr, const uint8_t** segment_start, const uint8_t** segment_end)
{
    *(*dst_write_ptr)++ = n - 1; // Write copy header
    while (*segment_start < *segment_end) {
        *(*dst_write_ptr)++ = *(*segment_start)++; // Write data bytes
    }
    // Note: *segment_start is equal to *segment_end after the loop is executed
}

rle_result rle_encode(void* dst_ptr, size_t dst_len, const void* src_ptr, size_t src_len)
{
    rle_result result = { 0, RLE_OK };

    if (dst_ptr == NULL || src_ptr == NULL || dst_len == 0 || src_len == 0) {
        result.status = RLE_INVALID_ARGUMENT;
        return result;
    }

    const uint8_t* src_read_ptr = src_ptr;
    const uint8_t* src_end_ptr = src_read_ptr + src_len;

    uint8_t* dst_start_ptr = dst_ptr;
    uint8_t* dst_end_ptr = dst_start_ptr + dst_len;
    uint8_t* dst_write_ptr = dst_ptr;

    bool prev_run_flag = false;
    bool run_flag = false;

    const uint8_t* b_start_ptr = src_read_ptr; // Current segment start
    const uint8_t* b_end_ptr;                  // Current segment end

    size_t n = 0;      // Current segment size
    uint8_t count = 0; // To handle MAX_COPY/MAX_RUN situation

    // Iterate over the source bytes starting from 1 to allow [i - 1]
    for (size_t i = 1; i < src_len; i++) {
        count++;
        b_end_ptr = src_read_ptr + i;
        n = b_end_ptr - b_start_ptr;

        prev_run_flag = run_flag;
        run_flag = (src_read_ptr[i] == src_read_ptr[i - 1]);

        // 0 --> 1 transition: write a copy segment if any, start a new run
        if (!prev_run_flag && run_flag) {

            // Go one position back because of the run detection logic
            --b_end_ptr;
            n = b_end_ptr - b_start_ptr;

            if (n > 0) {
                // N + 1 bytes need to be written: header + N data bytes
                if (is_out_of_bounds(n + 1, dst_write_ptr, dst_end_ptr)) {
                    result.status |= RLE_DST_WRITE_ERROR;
                    return result;
                }
                write_copy_segment(n, &dst_write_ptr, &b_start_ptr, &b_end_ptr);
            }
        }

        // 1 --> 0 transition: write a run segment
        if (prev_run_flag && !run_flag) {
            // Two bytes need to be written: header + run symbol
            if (is_out_of_bounds(2, dst_write_ptr, dst_end_ptr)) {
                result.status |= RLE_DST_WRITE_ERROR;
                return result;
            }
            write_run_segment(n, &dst_write_ptr, &b_start_ptr, &b_end_ptr);
        }

        // The copy segment is too long, need to be broken up into smaller pieces
        if (!run_flag && count >= MAX_COPY) {
            count = 0;
            if (is_out_of_bounds(n + 1, dst_write_ptr, dst_end_ptr)) {
                result.status |= RLE_DST_WRITE_ERROR;
                return result;
            }
            write_copy_segment(n, &dst_write_ptr, &b_start_ptr, &b_end_ptr);
        }

        // The run segment is too long, need to be broken up into smaller pieces
        if (run_flag && count >= MAX_RUN) {
            count = 0;
            if (is_out_of_bounds(2, dst_write_ptr, dst_end_ptr)) {
                result.status |= RLE_DST_WRITE_ERROR;
                return result;
            }
            write_run_segment(n, &dst_write_ptr, &b_start_ptr, &b_end_ptr);
        }
    }

    // Process the last segment
    b_end_ptr = src_end_ptr;
    n = b_end_ptr - b_start_ptr;

    if (run_flag) {
        if (is_out_of_bounds(2, dst_write_ptr, dst_end_ptr)) {
            result.status |= RLE_DST_WRITE_ERROR;
            return result;
        }
        write_run_segment(n, &dst_write_ptr, &b_start_ptr, &b_end_ptr);
    } else {
        if (is_out_of_bounds(n + 1, dst_write_ptr, dst_end_ptr)) {
            result.status |= RLE_DST_WRITE_ERROR;
            return result;
        }
        write_copy_segment(n, &dst_write_ptr, &b_start_ptr, &b_end_ptr);
    }

    // Calculate the output length
    result.out_len = dst_write_ptr - dst_start_ptr;

    return result;
}

rle_result rle_decode(void* dst_ptr, size_t dst_len, const void* src_ptr, size_t src_len)
{
    rle_result result = { 0, RLE_OK };

    if (dst_ptr == NULL || src_ptr == NULL || dst_len == 0 || src_len == 0) {
        result.status = RLE_INVALID_ARGUMENT;
        return result;
    }

    const uint8_t* src_read_ptr = src_ptr;
    const uint8_t* src_end_ptr = src_read_ptr + src_len;

    uint8_t* dst_start_ptr = dst_ptr;
    uint8_t* dst_end_ptr = dst_start_ptr + dst_len;
    uint8_t* dst_write_ptr = dst_ptr;

    while (src_read_ptr < src_end_ptr) {
        int8_t header = *src_read_ptr++;
        if (header < 0) {
            // Run
            size_t n = 1 - header;

            if (is_out_of_bounds(n, dst_write_ptr, dst_end_ptr)) {
                result.status |= RLE_DST_WRITE_ERROR;
                return result;
            }

            // Check that one more byte can be read (run symbol)
            if (is_out_of_bounds(1, src_read_ptr, src_end_ptr)) {
                result.status |= RLE_SRC_DECODE_ERROR;
                return result;
            }

            uint8_t symbol = *src_read_ptr++;
            for (size_t i = 0; i < n; i++)
                *dst_write_ptr++ = symbol;
        } else {
            // Copy
            size_t n = header + 1;

            if (is_out_of_bounds(n, dst_write_ptr, dst_end_ptr)) {
                result.status |= RLE_DST_WRITE_ERROR;
                return result;
            }

            // Check that N more bytes can be read
            if (is_out_of_bounds(n, src_read_ptr, src_end_ptr)) {
                result.status |= RLE_SRC_DECODE_ERROR;
                return result;
            }

            for (size_t i = 0; i < n; i++)
                *dst_write_ptr++ = *src_read_ptr++;
        }
    }

    // Calculate the output length
    result.out_len = dst_write_ptr - dst_start_ptr;

    return result;
}
