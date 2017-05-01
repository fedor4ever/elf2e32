from __future__ import with_statement
#encoding=ansi
import binascii
from bitstring import BitArray, BitStream

fname = "D:/codeblock/elf2e32/tests/libcrypto-uncompressed-2.4.5.dll"
fname2 = "D:/codeblock/elf2e32/tests/libcrypto-2.4.5.dll"
with open(fname, 'rb') as my_file:
    my_bytes = my_file.read(160)

with open(fname2, 'rb') as my_file2:
    my_bytes2 = my_file2.read(160)


#Most of e32 image headers have 4 byte field
def UID1(arg):
    "ignore 0 position on string"
    print "UID1: 0x" + binascii.hexlify(arg[3::-1])

def UID2(arg):
    print "UID2: 0x" + binascii.hexlify(arg[3+4:0+3:-1])

def UID3(arg):
    print "UID3: 0x" + binascii.hexlify(arg[3+8:3+4:-1])

def spare(arg):
    print "spare: 0x" + binascii.hexlify(arg[151:147:-1])

def flagfield(arg):
    "Second and 3rd bytes are reserved"
    t = binascii.hexlify(arg[47] + arg[44])
    print "flagfield: 0x%s" %t
    a = BitArray("0x%s" %t)
    print "flagfield raw data: " + a.bin

UID1(my_bytes)
UID1(my_bytes2)
flagfield(my_bytes)

e32fields=("TUint32 iUid1: 0x", "TUint32 iUid2: 0x", "TUint32 iUid3: 0x", "TUint32 iUidChecksum: 0x",
 "TUint iSignature: 0x", "TUint32 iHeaderCrc: 0x", "TUint32 iModuleVersion: 0x", "TUint32 iCompressionType (0 for none): 0x",
 "TVersion iToolsVersion: 0x", "TUint32 iTimeLo: 0x", "TUint32 iTimeHi: 0x",  "Flags field: 0x",
 "TInt iCodeSize: 0x", "TInt iDataSize: 0x", "TInt iHeapSizeMin: 0x", "TInt iHeapSizeMax: 0x",
 "TInt iStackSize: 0x", "TInt iBssSize: 0x", "TUint iEntryPoint: 0x", "TUint iCodeBase: 0x",
 "TUint iDataBase: 0x", "TInt iDllRefTableCount: 0x", "TUint iExportDirOffset: 0x", "TInt iExportDirCount: 0x",
 "TInt iTextSize: 0x", "TUint iCodeOffset: 0x", "TUint iDataOffset: 0x", "TUint iImportOffset: 0x",
 "TUint iCodeRelocOffset: 0x", "TUint iDataRelocOffset: 0x", "TUint16 iCpuIdentifier&&iProcessPriority(0x2000 = ARM): 0x", "Uncompressed size of the file: 0x",
 "Security information (capabilities, secure ID, vendor ID): 0x", "Security information (capabilities, secure ID, vendor ID): 0x", "Security information (capabilities, secure ID, vendor ID): 0x", "Security information (capabilities, secure ID, vendor ID): 0x",
 "Offset from start of code section to exception descriptor: 0x", "Spare: 0x", "Size of bitmap description holes in the export table: 0x", "Format of bitmap describing holes in the export table: 0x",
 "Bitmap describing any holes in the export table. This is a variable length field: 0x")
i=0
print e32fields[i] + binascii.hexlify(my_bytes[3::-1]) + "\tat pos: %i" %(i*4)
i+=1
while(i<40):
    print e32fields[i] + binascii.hexlify(my_bytes[3+ i*4:0 + i*4 -1:-1]) + "\tat pos: %i" %(i*4)
    # print e32fields[i] + binascii.hexlify(my_bytes2[3+ i*4] + my_bytes2[2+ i*4] + my_bytes2[1+ i*4] + my_bytes2[0+ i*4])
    i+=1


