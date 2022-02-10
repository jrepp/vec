#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "test_help.h"

typedef struct mem_header_t {
    struct mem_header_t   *next;
    size_t                size;
    size_t                sequence;
    size_t                reallocations;
    uint8_t               pattern[16];
} mem_header_t;

typedef struct mem_footer_t {
    uint8_t               pattern[16];
} mem_footer_t;

#define MEM_HEADER_BYTE 0xdf
#define MEM_FOOTER_BYTE 0xfd

static int force_fail_malloc = 0;
static int force_fail_realloc = 0;
static mem_header_t *regions = NULL;


static int assert_valid_pattern(uint8_t *pattern, size_t size, uint8_t byte) {
  for (size_t i = 0; i < size; ++i) {
    assert(pattern[i] == byte);
    if (pattern[i] != byte) {
      return -1;
    }
  }
  return 0;
}

static int assert_valid_header(mem_header_t *header) {
  for (const mem_header_t *check = regions; check; check = check->next) {
    if (check == header) {
      return 0;
    }
  }
  assert(!"header was not found in valid regions");
  return -1;
}

static int assert_allocation_size(mem_header_t *header, mem_footer_t *footer) {
  assert((ptrdiff_t)header->size == ((uint8_t*)footer - (uint8_t*)(header + 1)));
  if((ptrdiff_t)header->size == ((uint8_t*)footer - (uint8_t*)(header + 1))) {
    return 0;
  }
  return -1;
}

void set_fail_malloc(int enable) {
  force_fail_malloc = enable;
}

void set_fail_realloc(int enable) {
  force_fail_realloc = enable;
}

void *test_malloc(size_t bytes) {
  if (force_fail_malloc) {
    return NULL;
  }

  uint8_t *request = (uint8_t *)malloc(sizeof(mem_header_t) + sizeof(mem_footer_t) + bytes);
  assert(request != NULL);
  if (!request) {
    return NULL;
  }

  mem_header_t *header = (mem_header_t *)request;
  mem_footer_t *footer = (mem_footer_t *)(request + sizeof(mem_header_t) + bytes);

  header->next = regions;
  header->size = bytes;
  header->sequence = stats_->malloc_count++;
  header->reallocations = 0;

  regions = header;

  stats_->memory += bytes;
  if (stats_->memory > stats_->high_memory) {
    stats_->high_memory = stats_->memory;
  }

  memset(header->pattern, MEM_HEADER_BYTE, sizeof(header->pattern));
  memset(footer->pattern, MEM_FOOTER_BYTE, sizeof(footer->pattern));

  return (void *)(header + 1);
}

void *test_realloc(void *existing_request, size_t bytes) {
  if (force_fail_realloc) {
    return NULL;
  }

  // Convert realloc into malloc
  if (existing_request == NULL) {
    return test_malloc(bytes);
  }

  // get the current header and footer and validate their memory protections
  mem_header_t *header = ((mem_header_t *)existing_request) - 1;
  assert_valid_pattern(header->pattern, sizeof(header->pattern), MEM_HEADER_BYTE);
  mem_footer_t *footer = (mem_footer_t *)((uint8_t *)existing_request + header->size);
  assert_valid_pattern(footer->pattern, sizeof(footer->pattern), MEM_FOOTER_BYTE);
  assert_valid_header(header);
  assert_allocation_size(header, footer);

  // Remove reallocating pointer from regions list
  int active_region = 0;
  for (mem_header_t **f = &regions; *f != NULL; f = &(*f)->next) {
    if (*f == header) {
      *f = header->next;
      active_region = 1;
      break;
    }
  }
  assert(active_region == 1);
  if (!active_region) {
    return NULL;
  }

  // Reallocate into a new region
  const size_t header_seq = header->sequence;
  const size_t header_size = header->size;
  uint8_t *new_region = (uint8_t*)realloc((void *)header, bytes + sizeof(mem_header_t) + sizeof(mem_footer_t));
  assert(new_region != NULL);
  if (new_region == NULL) {
    return NULL;
  }
  stats_->realloc_count++;

  // Adjust memory stats
  int64_t diff = (int64_t)bytes - (int64_t)header_size;
  if (diff > 0) {
    stats_->memory += diff;
    if (stats_->memory > stats_->high_memory) {
      stats_->high_memory = stats_->memory;
    }
  } else {
    assert(labs(diff) <= (int64_t)stats_->memory);
    stats_->memory += diff;
  }

  // Fixup the header, it should be in the same location
  mem_header_t *new_header = (mem_header_t *)new_region;
  assert_valid_pattern(new_header->pattern, sizeof(new_header->pattern), MEM_HEADER_BYTE);
  assert(new_header->sequence == header_seq);
  new_header->reallocations++;
  new_header->size = bytes;

  // Reset the footer pattern at the new request boundary
  mem_footer_t *new_footer = (mem_footer_t *)(new_region + sizeof(mem_header_t) + bytes);
  memset(new_footer->pattern, MEM_FOOTER_BYTE, sizeof(new_footer->pattern));

  assert_allocation_size(new_header, new_footer);

  // Add back to regions list
  new_header->next = regions;
  regions = new_header;

  return (void *)(new_header + 1);
}

void test_free(void *request) {
  if (request == NULL) {
    return;
  }

  mem_header_t *header = ((mem_header_t *)request) - 1;
  assert_valid_pattern(header->pattern, sizeof(header->pattern), MEM_HEADER_BYTE);
  mem_footer_t *footer = (mem_footer_t *)((uint8_t *)request + header->size);
  assert_valid_pattern(footer->pattern, sizeof(footer->pattern), MEM_FOOTER_BYTE);

  stats_->free_count++;
  stats_->memory -= header->size;

  for (mem_header_t **f = &regions; *f != NULL; f = &(*f)->next) {
    if (*f == header) {
      *f = header->next;
      free(header);
      return;
    }
  }

  assert(!"header not found in valid regions during free");
}
