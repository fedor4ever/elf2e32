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
    "Bitfield"
    t = binascii.hexlify(arg[47:43:-1])
    print "flagfield: 0x%s" %t
    if((int(t, 16)&0x00000008)== 0x00000008):
        print "build against EKA2!"
    elif((int(t, 16)&0x00000008)== 0x00000000):
        print "build against EKA1!"

flagfield(fname)

#E32 image cosist from 4 byte fields
#exceptions:
 # ModuleVersion(split on two 16bit Major+Minor subfields)
 # Time file creation(consists of two 4 byte fields - TimeLo +TimeHi )
 # Flags field - store many info
 # Format of bitmap describing holes in the export table. One bit lenth field
# starts at pos 0x9A(154)
 # Bitmap describing any holes in the export table. This is a variable
# length field starts at pos 0x9B(155)
e32fields=("UID1", "UID2", "UID3", "UID Checksum",
 "Image Signature", "HeaderCrc(V-format)\CPU identifier", "ModuleVersion(V-format)/Checksum", "CompressionType (0 for none)",
 "PETRAN/ELFTRAN version", "TimeLo", "TimeHi",  "Flags field",
 "CodeSize", "DataSize", "Exe's HeapSizeMin", "Exe's HeapSizeMax",
 "Exe's StackSize", "BssSize", "EntryPoint", "CodeBase(where code is linked for)",
 "DataBase (where data is linked for)", "DllRefTableCount", "ExportDirOffset(Offset into the file of the export address table)", "ExportDirCount",
 "TextSize", "CodeOffset", "DataOffset", "ImportOffset",
 "TUint iCodeRelocOffset: 0x", "TUint iDataRelocOffset: 0x", "TUint16 iCpuIdentifier&&iProcessPriority(0x2000 = ARM): 0x", "Uncompressed size of the file: 0x",
 "Security information (capabilities, secure ID, vendor ID): 0x", "Security information (capabilities, secure ID, vendor ID): 0x", "Security information (capabilities, secure ID, vendor ID): 0x", "Security information (capabilities, secure ID, vendor ID): 0x",
 "Offset from start of code section to exception descriptor: 0x", "Spare: 0x", "Size of bitmap description holes in the export table: 0x", "Format of bitmap describing holes in the export table: 0x",
 "Bitmap describing any holes in the export table. This is a variable length field: 0x")
i=0
print e32fields[i] + ": 0x" + binascii.hexlify(my_bytes[3::-1]) + "\tat pos: %i" %(i*4)
i+=1
while(i<40):
    print e32fields[i] + ": 0x" + binascii.hexlify(my_bytes[3+ i*4:0 + i*4 -1:-1]) + "\tat pos: %i" %(i*4)
    # print e32fields[i] + binascii.hexlify(my_bytes2[3+ i*4] + my_bytes2[2+ i*4] + my_bytes2[1+ i*4] + my_bytes2[0+ i*4])
    i+=1


