/* Build: cc -std=c89 -Isrc src/microtar.c tests/test_empty_archive.c -o test_empty
 * Run: ./test_empty /path/to/temporary/archive.tar (overwritten and removed)
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "microtar.h"

static void check_empty_archive(const char *path) {
  mtar_t tar;
  mtar_header_t h;
  assert(mtar_open(&tar, path, "w") == MTAR_ESUCCESS);
  assert(mtar_finalize(&tar) == MTAR_ESUCCESS);
  assert(mtar_close(&tar) == MTAR_ESUCCESS);

  assert(mtar_open(&tar, path, "r") == MTAR_ESUCCESS);
  assert(tar.pos == 0);
  assert(mtar_read_header(&tar, &h) == MTAR_ENULLRECORD);
  assert(mtar_close(&tar) == MTAR_ESUCCESS);
}

static void check_nonempty_archive(const char *path) {
  mtar_t tar;
  mtar_header_t h;
  char data[3];
  assert(mtar_open(&tar, path, "w") == MTAR_ESUCCESS);
  assert(mtar_write_file_header(&tar, "file.txt", 3) == MTAR_ESUCCESS);
  assert(mtar_write_data(&tar, "abc", 3) == MTAR_ESUCCESS);
  assert(mtar_finalize(&tar) == MTAR_ESUCCESS);
  assert(mtar_close(&tar) == MTAR_ESUCCESS);

  assert(mtar_open(&tar, path, "r") == MTAR_ESUCCESS);
  assert(mtar_read_header(&tar, &h) == MTAR_ESUCCESS);
  assert(strcmp(h.name, "file.txt") == 0);
  assert(h.size == sizeof(data));
  assert(mtar_read_data(&tar, data, sizeof(data)) == MTAR_ESUCCESS);
  assert(memcmp(data, "abc", sizeof(data)) == 0);
  assert(mtar_next(&tar) == MTAR_ESUCCESS);
  assert(mtar_read_header(&tar, &h) == MTAR_ENULLRECORD);
  assert(mtar_close(&tar) == MTAR_ESUCCESS);
}

static void check_bad_checksum(const char *path) {
  mtar_t tar;
  FILE *file = fopen(path, "r+b");
  assert(file != NULL);
  /* Change the preceding test's file name without updating its checksum. */
  assert(fputc('!', file) != EOF);
  assert(fclose(file) == 0);
  assert(mtar_open(&tar, path, "r") == MTAR_EBADCHKSUM);
}

static void check_empty_file(const char *path) {
  mtar_t tar;
  FILE *file = fopen(path, "wb");
  assert(file != NULL);
  assert(fclose(file) == 0);
  /* A zero-byte file has no tar end marker and remains a read error. */
  assert(mtar_open(&tar, path, "r") == MTAR_EREADFAIL);
}

int main(int argc, char **argv) {
  assert(argc == 2);
  check_empty_archive(argv[1]);
  check_nonempty_archive(argv[1]);
  check_bad_checksum(argv[1]);
  check_empty_file(argv[1]);
  assert(remove(argv[1]) == 0);
  puts("PASS: empty archive, file round trip, bad checksum and empty file");
  return 0;
}
