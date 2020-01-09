#if defined(USE_ELF64)
#define ELF_PREFIX(prf) Elf64_##prf
#define Elf_Ehdr	Elf64_Ehdr
#define Elf_Shdr	Elf64_Shdr
#define Elf_Dyn		Elf64_Dyn
#define Elf_Word	Elf64_Word
#define Elf_Sword	Elf64_Sword
#else
#define ELF_PREFIX(prf) Elf32_##prf
#define Elf_Ehdr	Elf32_Ehdr
#define Elf_Shdr	Elf32_Shdr
#define Elf_Dyn		Elf32_Dyn
#define Elf_Word	Elf32_Word
#define Elf_Sword	Elf32_Sword
#endif

static char *ELF_PREFIX(section_name)(char *mem, int idx)
{
    Elf_Ehdr *ehdr = (Elf_Ehdr *)mem;
    Elf_Shdr *shdr = (Elf_Shdr *) &mem[ehdr->e_shoff + ehdr->e_shentsize * ehdr->e_shstrndx];
    return &mem[shdr->sh_offset] + idx;
}

static Elf_Shdr *ELF_PREFIX(find_section)(char *mem, char *name)
{
    Elf_Ehdr *ehdr = (Elf_Ehdr *) mem;

    for (int i = 0; i < ehdr->e_shnum; i++) {
	Elf_Shdr *shdr = (Elf_Shdr *) &mem[ehdr->e_shoff + ehdr->e_shentsize * i];
	if (!strcmp(ELF_PREFIX(section_name)(mem, shdr->sh_name), name)) {
	    return shdr;
	}
    }

    return NULL;
}

static Elf_Dyn *ELF_PREFIX(find_dynamic_entry)(char *mem, Elf_Sword tag)
{
    Elf_Shdr *shdr = ELF_PREFIX(find_section)(mem, ".dynamic");
    if (!shdr) {
	return NULL;
    }
    Elf_Dyn *dyn = (Elf_Dyn *) &mem[shdr->sh_offset];
    while (dyn->d_tag != DT_NULL) {
	if (dyn->d_tag == tag) {
	    return dyn;
	}
	dyn++;
    }

    return NULL;
}

static int ELF_PREFIX(add_runpath)(char **ptr, size_t *size, char *path)
{
    char *mem = *ptr;
    Elf_Ehdr *ehdr = (Elf_Ehdr *)mem;

    if (ehdr->e_shentsize != sizeof(Elf_Shdr)) {
	fprintf(stderr, "Wrong size of section in header(%d, %d)!\n",
		ehdr->e_shentsize, sizeof(Elf_Shdr));
	return -1;
    }

    Elf_Shdr *shdr2 = ELF_PREFIX(find_section)(mem, ".note.ABI-tag");
    if (!shdr2) {
	fprintf(stderr, "Section .note.ABI-tag not found!\n");
	return -1;
    }

    Elf_Shdr *shdr_next = shdr2 + 1;
    if (strcmp(ELF_PREFIX(section_name)(mem, shdr_next->sh_name), ".note.gnu.build-id")) {
	fprintf(stderr, "Section .note.gnu.build-id not found!\n");
	return -1;
    }

    shdr_next++;
    if (strcmp(ELF_PREFIX(section_name)(mem, shdr_next->sh_name), ".gnu.hash")) {
	fprintf(stderr, "Section .gnu.hash not found!\n");
	return -1;
    }

    Elf_Shdr *shdr5 = ELF_PREFIX(find_section)(mem, ".dynsym");
    if (!shdr2) {
	fprintf(stderr, "Section .dynsym not found!\n");
	return -1;
    }

    Elf_Shdr *shdr6 = ELF_PREFIX(find_section)(mem, ".dynstr");
    if (!shdr2) {
	fprintf(stderr, "Section .dynstr not found!\n");
	return -1;
    }

    memmove(&mem[shdr2->sh_offset], &mem[shdr5->sh_offset], shdr5->sh_size);
    shdr5->sh_addr = shdr2->sh_addr;
    shdr5->sh_offset = shdr2->sh_offset;
    memset((char *)shdr2, 0, sizeof(Elf_Shdr) * 3);

    size_t endlen = (shdr6->sh_offset + shdr6->sh_size) - (shdr5->sh_offset + shdr5->sh_size + shdr6->sh_size);
    memmove(&mem[shdr5->sh_offset + shdr5->sh_size], &mem[shdr6->sh_offset], shdr6->sh_size);
    shdr6->sh_addr = shdr5->sh_addr + shdr5->sh_size;
    shdr6->sh_offset = shdr5->sh_offset + shdr5->sh_size;

    printf("get %d bytes for string table\n", endlen);

    memset(&mem[shdr6->sh_offset + shdr6->sh_size], 0, endlen);

    if (strlen(path) > endlen - 1) {
	fprintf(stderr, "path too long\n");
	return -1;
    }

    Elf_Word newpath = shdr6->sh_size;
    strcpy(&mem[shdr6->sh_offset + shdr6->sh_size], path);
    shdr6->sh_size += strlen(path) + 1;

    Elf_Dyn *dyn = ELF_PREFIX(find_dynamic_entry)(mem, DT_SYMTAB);
    if (dyn) {
	dyn->d_un.d_ptr = shdr5->sh_addr;
    }

    dyn = ELF_PREFIX(find_dynamic_entry)(mem, DT_STRTAB);
    if (dyn) {
	dyn->d_un.d_ptr = shdr6->sh_addr;
    }

    dyn = ELF_PREFIX(find_dynamic_entry)(mem, DT_STRSZ);
    if (dyn) {
	dyn->d_un.d_val = shdr6->sh_size;
    }

    dyn = ELF_PREFIX(find_dynamic_entry)(mem, DT_GNU_HASH);
    if (dyn) {
	dyn->d_tag = DT_RUNPATH;
	dyn->d_un.d_val = newpath;
    }

    return 0;
}

#undef ELF_PREFIX
#undef Elf_Ehdr
#undef Elf_Shdr
#undef Elf_Dyn
#undef Elf_Word
#undef Elf_Sword
