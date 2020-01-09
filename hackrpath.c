/*
 * Simple patchelf replacement for arm architecture.
 * Copyright © 2020 sashz <sashz@pdaXrom.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <linux/limits.h>

#include "elf.h"
#define USE_ELF64
#include "elf.h"
#undef USE_ELF64

static int Elf_get_type(char *mem, size_t size)
{
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)mem;

    if (!memcmp(ehdr->e_ident, ELFMAG, SELFMAG)) {
	switch(ehdr->e_ident[EI_CLASS]) {
	case 1:	return 32;
	case 2: return 64;
	default: break;
	}
    }

    return -1;
}

static int write_file(char *name, char *mem, size_t size)
{
    int ret = -1;
    FILE *out = fopen(name, "wb");
    if (!out) {
	fprintf(stderr, "cannot write file %s\n", name);
	return -1;
    }

    if (fwrite(mem, 1, size, out) != size) {
	fprintf(stderr, "cannot write file %s\n", name);
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
	    int ret = -1;

	    switch (Elf_get_type(buf, bufsiz)) {
	    case 32: ret = Elf32_add_runpath(&buf, &bufsiz, argv[2]); break;
	    case 64: ret = Elf64_add_runpath(&buf, &bufsiz, argv[2]); break;
	    default: break;
	    }

	    if (ret != -1) {
		ret = write_file(argv[3], buf, bufsiz);
	    }

	    free(buf);
	    return ret;
	}
    } else if (!strcmp(argv[1], "--print-rpath")) {
	if (load_file(argv[2], &buf, &bufsiz) != -1) {
	    int ret = -1;

	    switch (Elf_get_type(buf, bufsiz)) {
	    case 32: ret = Elf32_print_runpath(buf, bufsiz); break;
	    case 64: ret = Elf64_print_runpath(buf, bufsiz); break;
	    default: break;
	    }

	    return ret;
	}
    } else {
	fprintf(stderr, "Unknown parameter %s\n", argv[1]);
    }

    return -1;
}
