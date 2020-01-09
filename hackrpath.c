#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <linux/limits.h>

static int debug = 0;

#include "elf.h"
#define USE_ELF64
#include "elf.h"
#undef USE_ELF64

static int write_file(char *name, char *mem, size_t size)
{
    int ret = -1;
    char outname[PATH_MAX];
    snprintf(outname, sizeof(outname), "%s.out", name);
    FILE *out = fopen(outname, "wb");
    if (!out) {
	fprintf(stderr, "cannot write file %s\n", outname);
	return -1;
    }

    if (fwrite(mem, 1, size, out) != size) {
	fprintf(stderr, "cannot write file %s\n", outname);
    } else {
	ret = 0;
    }

    fclose(out);

    return ret;
}

int main(int argc, char *argv[])
{
    struct stat sb;
    char *buf;
    size_t bufsiz;
    FILE *inf;

    if (stat(argv[1], &sb) == -1) {
	perror("stat");
	exit(EXIT_FAILURE);
    }

    if ((sb.st_mode & S_IFMT) != S_IFREG) {
	fprintf(stderr, "File %s is not regular file!\n", argv[1]);
	return -1;
    }

    bufsiz = sb.st_size;
    fprintf(stderr, "File size: %d\n", (int)bufsiz);

    buf = malloc(bufsiz);
    if (!buf) {
	fprintf(stderr, "cannot allocate memory for file\n");
	return -1;
    }

    inf = fopen(argv[1], "rb");
    if (!inf) {
	fprintf(stderr, "can not open file %s\n", argv[1]);
	free(buf);
	return -1;
    }

    if (fread(buf, 1, bufsiz, inf) != bufsiz) {
	fprintf(stderr, "error reading file\n");
	fclose(inf);
	free(buf);
	return -1;
    }

    fclose(inf);

//    elf32_parse(buf, bufsiz);

//    elf32_resize_section(&buf, &bufsiz, ".dynamic", 0x100);

//    fprintf(stderr, "New file size: %d\n", (int)bufsiz);

    Elf32_add_runpath(&buf, &bufsiz, "$ORIGIN/../lib");

//    elf32_parse(buf, bufsiz);

    write_file(argv[1], buf, bufsiz);

    free(buf);

    return 0;
}
