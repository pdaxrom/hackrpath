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

static int load_file(char *name, char **mem, size_t *size)
{
    struct stat sb;
    FILE *inf;

    if (stat(name, &sb) == -1) {
	perror("stat");
	return -1;
    }

    if ((sb.st_mode & S_IFMT) != S_IFREG) {
	fprintf(stderr, "File %s is not regular file!\n", name);
	return -1;
    }

    *size = sb.st_size;
    fprintf(stderr, "File size: %d\n", (int)*size);

    *mem = malloc(*size);
    if (!*mem) {
	fprintf(stderr, "cannot allocate memory for file\n");
	return -1;
    }

    inf = fopen(name, "rb");
    if (!inf) {
	fprintf(stderr, "can not open file %s\n", name);
	free(*mem);
	return -1;
    }

    if (fread(*mem, 1, *size, inf) != *size) {
	fprintf(stderr, "error reading file\n");
	fclose(inf);
	free(*mem);
	return -1;
    }

    fclose(inf);
    return 0;
}

int main(int argc, char *argv[])
{
    char *buf;
    size_t bufsiz;

    if (!strcmp(argv[1], "--set-rpath")) {
	if (load_file(argv[3], &buf, &bufsiz) != -1) {
	    Elf32_add_runpath(&buf, &bufsiz, argv[2]);
	    int ret = write_file(argv[3], buf, bufsiz);
	    free(buf);
	    return ret;
	}
    } else {
	fprintf(stderr, "Unknown parameter %s\n", argv[1]);
    }

    return -1;
}
