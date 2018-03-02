from __future__ import with_statement
#encoding=ansi

orig = "objdump-log.txt"
e32 = "elfdump.txt"

with open(orig) as my_file:
    my_bytes = my_file.readlines()

with open(e32) as my_file2:
    my_bytes2 = my_file2.readlines()

orig_text=[i for i in my_bytes if ".text" in i]
orig_text=[i[0:8:] for i in orig_text]
orig_data=[i for i in my_bytes if ".data" in i]
orig_data=[i[0:8:] for i in orig_data]
orig_text.sort()
orig_data.sort()

e32_text=[i for i in my_bytes2 if ".text" in i]
e32_text=[i[0:8:] for i in e32_text]
e32_data=[i for i in my_bytes2 if ".data" in i]
e32_data=[i[0:8:] for i in e32_data]
e32_text.sort()
e32_data.sort()

print "text relocs by objdump: %d and elf2e32: %d" %(len(orig_text), len(e32_text) )

print "\ndata relocs by objdump: %d and elf2e32: %d" %(len(orig_data), len(e32_data) )


pos = 0
def t(x, y):
    global pos
    if x!=y:
        print "%d) data from elfdump: %s and elf2e32: %s" %(pos, x, y)
    pos+=1

ttt = [x for x in e32_text if x in orig_data]
print ttt
print len(ttt)
# map(t, orig_text, e32_text)
print "\n"
# map(t, orig_data, e32_data)
