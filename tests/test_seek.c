/* Build: cc -std=c89 -Isrc src/microtar.c tests/test_seek.c -o test_seek */
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "microtar.h"

struct test_stream {
  unsigned pos;
  unsigned requested;
  unsigned calls;
  int result;
};

static int stream_seek(mtar_t *tar, unsigned pos) {
  struct test_stream *stream = tar->stream;
  assert(tar->pos == stream->pos);
  stream->requested = pos;
  stream->calls++;
  if (stream->result == MTAR_ESUCCESS) {
    stream->pos = pos;
  }
  return stream->result;
}

static void check_seek(unsigned pos, int result) {
  mtar_t tar;
  struct test_stream stream = {17, 0, 0, MTAR_ESUCCESS};
  unsigned expected = result == MTAR_ESUCCESS ? pos : stream.pos;
  memset(&tar, 0, sizeof(tar));
  tar.stream = &stream;
  tar.seek = stream_seek;
  tar.pos = stream.pos;
  stream.result = result;

  assert(mtar_seek(&tar, pos) == result);
  assert(stream.requested == pos);
  assert(stream.calls == 1);
  assert(stream.pos == expected);
  assert(tar.pos == expected);

  /* A successful retry must also start from the actual stream position. */
  stream.result = MTAR_ESUCCESS;
  assert(mtar_seek(&tar, pos) == MTAR_ESUCCESS);
  assert(tar.pos == pos);
  assert(stream.pos == pos);
  assert(stream.calls == 2);
}

int main(void) {
  unsigned positions[] = {0, 16, 512, UINT_MAX};
  int results[] = {MTAR_ESUCCESS, MTAR_ESEEKFAIL, MTAR_EFAILURE, -42};
  unsigned i, j;
  for (i = 0; i < sizeof(positions) / sizeof(positions[0]); i++) {
    for (j = 0; j < sizeof(results) / sizeof(results[0]); j++) {
      check_seek(positions[i], results[j]);
    }
  }
  puts("PASS: 16 seek cases and successful retries");
  return 0;
}
