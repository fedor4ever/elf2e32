# encoding=utf-8
import os, subprocess

elf2e32=r"D:\codeblock\elf2e32\bin\Debug\elf2e32.exe"
tstdir=r"D:\codeblock\elf2e32\tests"
counter=0

caps=" --capability=All-TCB"
defin=r""" --definput="libcryptou.def" """
defout=r""" --defoutput="tmp\out.(%02d).def" """
elfin=r""" --elfinput="libcrypto.dll" """
e32bin=r""" --output="tmp\libcrypto-2.4.5.(%02d).dll" """
implibs=r""" --libpath="D:\Symbian\S60_5th_Edition_SDK_v1.0\epoc32\release\armv5\lib" """
linkas=r""" --linkas="libcrypto{000a0000}.dll" """
dsoout=r""" --dso="tmp\libcrypto{000a0000}.(%02d).dso" """
fpu=r" --fpu=softvfp"
iud1=r" --uid1=0x10000079"
uid2=r" --uid2=0x20004c45"
uid3=r" --uid3=0x00000000"
tgttype=r" --targettype=STDDLL"
tail=r" --dlldata --ignorenoncallable --debuggable --smpsafe --uncompressed"

# --capability=All-TCB --definput="tests\libcryptou.def" --defoutput="tests\out.def" --elfinput="tests\libcrypto.dll" --output="tests\libcrypto-2.4.5.tst.dll" --libpath="D:\Symbian\S60_5th_Edition_SDK_v1.0\epoc32\release\armv5\lib" --linkas="tests\libcrypto{000a0000}.dll" --dso="tests\libcrypto{000a0000}.dso" --fpu=softvfp --uid1=0x10000079 --uid2=0x20004c45 --uid3=0x00000000 --targettype=STDDLL --dlldata --ignorenoncallable --debuggable --smpsafe --uncompressed


print elf2e32
subprocess.check_call(elf2e32)

longtail=e32bin %counter+implibs+linkas+dsoout %counter+fpu+iud1+uid2+uid3+tgttype+tail


try:
   print "Test #%d" %counter
   print elf2e32+caps+defin+defout %counter+elfin+longtail
   subprocess.check_call(elf2e32+caps+defin+defout %counter+elfin+longtail)
except:
   print "Unexpectable test failure: Full options list!"
finally:
   print "\n"
   counter+=1
longtail=e32bin %counter+implibs+linkas+dsoout %counter+fpu+iud1+uid2+uid3+tgttype+tail

try:
   print "Test #%d" %counter
   print elf2e32+caps+defin.split("=")[0]+defout %counter+elfin+longtail
   subprocess.check_call(elf2e32+caps+defin.split("=")[0]+defout %counter+elfin+longtail)
except:
   print "Unexpectable test failure: %s!" %defin.split("=")[0]
finally:
   print "\n"
   counter+=1
longtail=e32bin %counter+implibs+linkas+dsoout %counter+fpu+iud1+uid2+uid3+tgttype+tail
   
try:
   print "Test #%d" %counter
   print elf2e32+caps+defin.split("=")[0]+defout.split("=")[0]+elfin+longtail
   subprocess.check_call(elf2e32+caps+defin.split("=")[0]+defout.split("=")[0]+elfin+longtail)
except:
   print "Unexpectable test failure: --definput, --defoutput!"
finally:
   print "\n"
   counter+=1
longtail=e32bin %counter+implibs+linkas+dsoout %counter+fpu+iud1+uid2+uid3+tgttype+tail

try:
   print "Test #%d" %counter
   print elf2e32+caps+defin+defout.split("=")[0]+elfin+longtail
   subprocess.check_call(elf2e32+caps+defin+defout.split("=")[0]+elfin+longtail)
except:
   print "Unexpectable test failure: --defoutput!"
finally:
   print "\n"
   counter+=1
longtail=e32bin %counter+implibs+linkas+dsoout %counter+fpu+iud1+uid2+uid3+tgttype+tail

try:
   print "Test #%d" %counter
   print elf2e32+caps+defin+defout %counter+elfin.split("=")[0]+longtail
   subprocess.check_call(elf2e32+caps+defin+defout %counter+elfin.split("=")[0]+longtail)
except:
   print "Expectable test failure: %s!" %elfin.split("=")[0]
finally:
   print "\n"
   counter+=1
longtail=e32bin %counter+implibs+linkas+dsoout %counter+fpu+iud1+uid2+uid3+tgttype+tail

try:
   print "Test #%d" %counter
   print elf2e32+caps+defin.split("=")[0]+defout.split("=")[0]+elfin.split("=")[0]+longtail
   subprocess.check_call(elf2e32+caps+defin.split("=")[0]+defout.split("=")[0]+elfin.split("=")[0]+longtail)
except:
   print "Expectable test failure: %s" %(defin.split("=")[0] + " " + defout.split("=")[0] + " " + elfin.split("=")[0])
finally:
   print "\n"
   counter+=1
longtail=e32bin %counter+implibs+linkas+dsoout %counter+fpu+iud1+uid2+uid3+tgttype+tail

try:
   print "Test #%d" %counter
   print elf2e32+""" --elfinput="libcrypto{000a0000}.dso" """+defout %counter
   subprocess.check_call(elf2e32+""" --elfinput="tmp\libcrypto{000a0000}.(03).dso" """+defout %counter)
except:
   print "Unexpectable test failure: %s" %(""" --elfinput="libcrypto{000a0000}.dso" """+defout)
finally:
   print "\n"
   counter+=1
longtail=e32bin %counter+implibs+linkas+dsoout %counter+fpu+iud1+uid2+uid3+tgttype+tail

try:
   print "Test #%d" %counter
   print elf2e32+defin+ """ --defoutput="tmp\def2def.def" """
   subprocess.check_call(elf2e32+defin+ """ --defoutput="tmp\def2def.def" """)
except:
   print "Unexpectable test failure: %s" %(elf2e32+defin+ """ --defoutput="tmp\def2def.def" """)
finally:
   print "\n"
   counter+=1

