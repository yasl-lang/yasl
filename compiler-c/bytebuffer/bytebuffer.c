#include "bytebuffer.h"

ByteBuffer *bb_new(int64_t size) {
    ByteBuffer *bb = malloc(sizeof(ByteBuffer));
    bb->size = size;
    bb->bytes = malloc(sizeof(char) * bb->size);
    bb->count = 0;
    return bb;
}

void bb_del(ByteBuffer *bb) {
    free(bb->bytes);
    free(bb);
}

void bb_add_byte(ByteBuffer  *bb, char byte) {
    if (bb->size <= bb->count) bb->bytes = realloc(bb->bytes, bb->count*2);
    bb->bytes[bb->count++] = byte;
};


void bb_append(ByteBuffer *bb, char *bytes, int64_t bytes_len) {
    if (bb->size < bb->count + bytes_len) bb->bytes = realloc(bb->bytes, (bb->count+bytes_len)*2);
    memcpy(bb->bytes + bb->count, bytes, bytes_len);
    //printf("copied: %s\n", bb->bytes + bb->count);
    bb->count += bytes_len;
};

void bb_floatbytes8(ByteBuffer *bb, double value) {
    if (bb->size < bb->count + sizeof(double)) bb->bytes = realloc(bb->bytes, (bb->count+sizeof(double))*2);
    memcpy(bb->bytes + bb->count, &value, sizeof(double));
    bb->count += sizeof(double);
}

void bb_intbytes8(ByteBuffer *bb, int64_t value) {
    if (bb->size < bb->count + sizeof(int64_t)) bb->bytes = realloc(bb->bytes, (bb->count+sizeof(int64_t))*2);
    memcpy(bb->bytes + bb->count, &value, sizeof(int64_t));
    bb->count += sizeof(int64_t);
}


void bb_rewrite_intbytes8(ByteBuffer *bb, int64_t index, int64_t value) {
    if (bb->size < index + sizeof(int64_t)) {
        puts("Bad rewrite_intbytes8: outside range");
        exit(EXIT_FAILURE);
    }
    memcpy(bb->bytes + index, &value, sizeof(int64_t));
}