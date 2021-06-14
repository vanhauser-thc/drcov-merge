#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_ENTRIES 128000

typedef struct _bb_entry_t {
    uint32_t start; /* offset of bb start from the image base */
    uint16_t size;
    uint16_t mod_id;
} bb_entry_t;

int main(int argc, char *argv[]) {
  bb_entry_t bb, bbs[MAX_ENTRIES];
  uint8_t   *header = NULL, *data, *ptr, text[64], unique = 0;
  uint32_t   header_len, entries = 0, idx = 2, off, s = sizeof(bb_entry_t), start, count, added, i;
  struct stat st;
  int        in, out;
  
  if (argc > 1 && strcmp(argv[1], "-u") == 0) {
    unique = 1;
    argv++;
    argc--;
  }
  
  if (argc < 4 || strncmp(argv[1], "-h", 2) == 0) {
    printf("Syntax: %s [-u] drcov.log drcov.1,log drcov.2.log drcov.3.log ...\n");
    printf("Merges all drcov logs to the first specified filename\n");
    printf("Option -u uniques the basic block information\n");
    exit(0);
  }

  if ((out = open(argv[1], O_WRONLY | O_CREAT | O_EXCL, 0644)) < 0) {
    perror(argv[1]);
    exit(-1);
  }

  while (idx < argc) {
  
    if ((in = open(argv[idx], O_RDONLY)) < 0 || fstat(in, &st) != 0 ||
        (data = (uint8_t*)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, in, 0)) == MAP_FAILED
       ) {
      perror(argv[idx]);
      goto next;
    }

    if ((ptr = strstr(data, "BB Table: ")) == NULL) {
      fprintf(stderr, "%s: no drcov header\n", argv[idx]);
      goto unmap; 
    }

    off = ptr - data;
    if (!header) {
      if ((header = malloc(off)) == NULL) {
        perror("malloc");
        exit(-1);
      }
      memcpy(header, data, off);
      write(out, header, off);
      header_len = off;
    } else {
      if (memcmp(header, data, header_len > off ? header_len : off) != 0) {
        fprintf(stderr, "%s: different drcov header\n", argv[idx]);
        goto unmap;
      }
    }

    while (*ptr != '\n' && ptr - data < st.st_size) ++ptr;
    if (*ptr != '\n')  {
      fprintf(stderr, "%s: no drcov header\n", argv[idx]);
      goto unmap; 
    } else {
      ++ptr;
    }

    printf("Processing %s (%lu bytes) ... ", argv[idx], st.st_size);
    
    start = ptr - data;    
    count = 0;
    added = 0;

    while (st.st_size - start >= s) {
      int found = 0;
      if (unique)
        for (i = 0; i < entries && !found; ++i)
          if (memcmp(&bbs[i], data + start, s) == 0) found = 1;
      if (!found) {
        if (entries >= MAX_ENTRIES) {
          fprintf(stderr, "MapFull!\n");
        } else {
          memcpy(&bbs[entries++], data + start, s);
          ++added;
        }
      }
      start += s;
      ++count;
    }

    printf("%u entries, %u new\n", count, added);

    unmap:  
    munmap(data, st.st_size);

    next:
    close(in);
    ++idx;
  }

  sprintf(text, "BB Table: %u bbs\n", entries);
  write(out, text, strlen(text));
  
  printf("Writing %u entries into %s\n", entries, argv[1]);

  for (i = 0; i < entries; ++i)
    write(out, &bbs[i], s);

  printf("Done.\n");

  close(out);
  return 0;  

}
