# elf2e32

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/175e7535c4af4c9eae9fc4280bed88a3)](https://www.codacy.com/app/fedor4ever/elf2e32?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=fedor4ever/elf2e32&amp;utm_campaign=Badge_Grade)

[![Build status](https://ci.appveyor.com/api/projects/status/ppmv7ly8d505h9vt?svg=true)](https://ci.appveyor.com/project/fedor4ever/elf2e32)

### In memory of my mother

Based on vanilla elf2e32 v. 2.1.15

All versions elf2e32 have critical bugs, that version may have too.
Need help with tests. I have no idea how write them.
 
That tool is replacement for broken one in SDK.

## Build instruction

### Code block users can import and build.

### Visual studio 15+ users - add src directory and build.

### Other - need C++14 compiler.

## Enhancements:
 - (un-)repacking existing E32 image
> syntax example: --e32input="tests\libcrypto-2.4.5.SDK.dll" --output="tests\tmp\libcrypto-2.4.5.inflate.dll" --compressionmethod=inflate