#ifndef RLE_H_
#define RLE_H_

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    RLE_OK               = 0x00,
    RLE_INVALID_ARGUMENT = 0x01,
    RLE_DST_WRITE_ERROR  = 0x02, // Destination is too small, stop to prevent a write overflow.
    RLE_SRC_DECODE_ERROR = 0x04, // Source contains invalid data that cannot be decoded correctly.
} rle_status;

typedef struct
{
    size_t     out_len;
    rle_status status;
} rle_result;

rle_result rle_encode(void* dst_ptr, size_t dst_len, const void* src_ptr, size_t src_len);

rle_result rle_decode(void* dst_ptr, size_t dst_len, const void* src_ptr, size_t src_len);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // RLE_H_
