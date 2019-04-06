
# elf2e32

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/175e7535c4af4c9eae9fc4280bed88a3)](https://www.codacy.com/app/fedor4ever/elf2e32?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=fedor4ever/elf2e32&amp;utm_campaign=Badge_Grade)

[![Build status](https://ci.appveyor.com/api/projects/status/ppmv7ly8d505h9vt?svg=true)](https://ci.appveyor.com/project/fedor4ever/elf2e32)

### In memory of my mother

Based on vanilla elf2e32 v. 2.1.15

All versions elf2e32 shipped with SDK have critical bugs. That tool is replacement for broken one in SDK.

That one based on source code taken from Symbian sources. That sources was heavily rewritten for modern portable C++14 and folder **source** lost 60 kilobytes of code or by 1/9. All known bugs fixed but these creatures may still hide somewere.

I will glad if my work helps you. It was nice if you test my app like me did. It's not hard - see readme in ***tests*** folder.

If my app crashes - I wish to ask you for little play with debugger before open new issue. If you have no luck - open new issue and add that:
 - options passed to elf2e32
 - elf file
 - def file(if used)
 - sources not needed at all

## You wish to add new enhansements.
If you wish add some new tricks for E32 Image target use new modules:
 - E32Producer for binary generation
 - E32Parser for get E32 Image layout properties
 - E32Validator for checking E32 Image consistency
 - e32common.h for use E32 Image common structures

## Enhancements:
 - (un-)repacking existing E32 image
> syntax example: --e32input="tests\libcrypto-2.4.5.SDK.dll" --output="tests\tmp\libcrypto-2.4.5.inflate.dll" --compressionmethod=inflate
 - list global variables if --dlldata not specified for any targets except STDDLL and STDEXE
 - can fix wrong or missed argument for UID1

## Known issues:
 - option "--namedlookup" produce slightly different binary in comparison with others

## Build instruction
### Code block users can import and build.
### Visual studio 15+ users - add src directory and build.
### Other - need C++14 compiler.
