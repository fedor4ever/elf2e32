from __future__ import with_statement
#encoding=ansi
import binascii

orig = "D:/codeblock/elf2e32/tests/libcrypto-2.4.5.tst.dll"
tgt = "D:/codeblock/elf2e32/tests/libcrypto-2.4.5.tst_.dll"

with open(orig, 'rb') as my_file:
    my_bytes = my_file.read()

with open(tgt, 'rb') as my_file2:
    my_bytes2 = my_file2.read()

# my_bytes=my_bytes[::-1]
# my_bytes2=my_bytes2[::-1]

pos = 0
def t(x, y):
    global pos
    if x!=y:
        print "x != y at pos 0x%3x(%d)\t and x have val: 0x%s\t and y have val: 0x%s" %(pos, pos, binascii.hexlify(x), binascii.hexlify(y))
    pos+=1

map(t, my_bytes, my_bytes2 )

