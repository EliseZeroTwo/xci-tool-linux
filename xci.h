#ifndef xci_h
#define xci_h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int uint32_t;
typedef unsigned long uint64_t;
typedef unsigned char uint8_t;
typedef uint8_t byte;

typedef struct {
    uint32_t magic;
    uint32_t num_files;
    uint32_t string_table_size;
    uint32_t reserved;
} hfs0_header_t;

typedef struct {
    uint64_t offset;
    uint64_t size;
    uint32_t string_table_offset;
    uint32_t hashed_size;
    uint64_t reserved;
    unsigned char hash[0x20];
} hfs0_file_entry_t;

typedef struct {
    FILE *file;
    uint64_t offset;
    uint64_t size;
    // hactool_ctx_t *tool_ctx;
    hfs0_header_t *header;
    char *name;
} hfs0_ctx_t;

typedef struct {
    uint8_t header_sig[0x100];
    uint32_t magic;
    uint32_t secure_offset;
    uint32_t _0x108;
    uint8_t _0x10C;
    uint8_t cart_type;
    uint8_t _0x10E;
    uint8_t _0x10F;
    uint64_t _0x110;
    uint64_t cart_size;
    unsigned char reversed_iv[0x10];
    uint64_t hfs0_offset;
    uint64_t hfs0_header_size;
    unsigned char hfs0_header_hash[0x20];
    unsigned char crypto_header_hash[0x20];
    uint32_t _0x180;
    uint32_t _0x184;
    uint32_t _0x188;
    uint32_t _0x18C;
    unsigned char encrypted_data[0x70];
} xci_header_t;

typedef struct {
    FILE *file; /* File for this NCA. */
    hfs0_ctx_t partition_ctx;
    hfs0_ctx_t normal_ctx;
    hfs0_ctx_t update_ctx;
    hfs0_ctx_t secure_ctx;
    hfs0_ctx_t logo_ctx;
    unsigned char iv[0x10];
    /* TODO: Header decryption. */
    /* unsigned char decrypted_header[0x70]; */
    xci_header_t header;
} xci_ctx_t;

int main();
void xciprocess(xci_ctx_t *ctx);
void xcisave(xci_ctx_t *ctx);
void hfs0process(hfs0_ctx_t *ctx);
void hfs0have(hfs0_ctx_t *xci);

static inline hfs0_file_entry_t *hfs0_get_file_entry(hfs0_header_t *hdr, uint32_t i) {
    if (i >= hdr->num_files) return NULL;
    return (hfs0_file_entry_t *)((char *)(hdr) + sizeof(*hdr) + i * sizeof(hfs0_file_entry_t));
}

static inline char *hfs0_get_string_table(hfs0_header_t *hdr) {
    return (char *)(hdr) + sizeof(*hdr) + hdr->num_files * sizeof(hfs0_file_entry_t);
}

static inline uint64_t hfs0_get_header_size(hfs0_header_t *hdr) {
    return sizeof(*hdr) + hdr->num_files * sizeof(hfs0_file_entry_t) + hdr->string_table_size;
}

static inline char *hfs0_get_file_name(hfs0_header_t *hdr, uint32_t i) {
    return hfs0_get_string_table(hdr) + hfs0_get_file_entry(hdr, i)->string_table_offset;
}

#endif
