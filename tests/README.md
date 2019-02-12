Those who wish to test the program perform the following steps:
 - build a program using the utilities from the SDK and mine. Then compare the size of the files, if different - please report the error.

 - Verify these files using [vbindiff](https://www.cjmweb.net/vbindiff/). These ranges may differs:

***0x0058 - 0x0059***(E32ImageHeader::iExportDirOffset) - Should be zero for EXE without exports
***0x0015 - 0x0018***(E32ImageHeader::iHeaderCrc)
***0x0021 - 0x0024***(ToolVersion E32ImageHeader::iVersion)
***0x0025 - 0x002A***(E32ImageHeader::iTimeLo and E32ImageHeader::iTimeHi)
***0x002B - 0x0020***(E32ImageHeader::iFlags)
If there are differences in another ranges, please report the error.
