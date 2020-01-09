# hackrpath
Add runpath to ELF

Simple dirty replacement for [patchelf](https://github.com/NixOS/patchelf) utility, that works also with arm architecture.

This utility replacing sections .note.ABI-tag, .note.gnu.build-id, .gnu.hash with .dynsym and expanded .dynstr. Dynamic entry GNU_HASH replacing with RUNPATH.

Usage:

$ hackrpath --set-rpath \\$ORIGIN/lib myapp
  
$ hackrpath --print-rpath myapp

