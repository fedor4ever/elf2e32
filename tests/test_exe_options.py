# encoding=utf-8
import os, subprocess

elf2e32=r"D:\codeblock\elf2e32\bin\Debug\elf2e32.exe"
tstdir=r"D:\codeblock\elf2e32\tests"
counter=0

caps=" --capability=All-TCB"
# defin=r""" --definput="libcryptou.def" """
# defout=r""" --defoutput="tmp\out.(%02d).def" """
elfin=r""" --elfinput="kf_Python_launcher.exe" """
e32bin=r""" --output="tmp\kf_Python_launcher.exe" """
implibs=r""" --libpath="D:\Symbian\S60_5th_Edition_SDK_v1.0\epoc32\release\armv5\lib" """
linkas=r""" --linkas="kf_Python_launcher{000a0000}.dll" """
# dsoout=r""" --dso="tmp\libcrypto{000a0000}.(%02d).dso" """
fpu=r" --fpu=softvfp"
iud1=r" --uid1=0x10000079"
uid2=r" --uid2=0x20004c45"
uid3=r" --uid3=0x00000000"
tgttype=r" --targettype=STDEXE"
tail=r" --dlldata --ignorenoncallable --debuggable --smpsafe --uncompressed"

longtail=e32bin+implibs+linkas+fpu+iud1+uid2+uid3+tgttype+tail

def run():
   """These tests must alwais pass!"""
   global counter
   try:
      print "Simple exe creation. Options are: %s\n" %(elf2e32+caps+elfin+longtail)
      subprocess.check_call(elf2e32+caps+elfin+longtail)
      print "EXE created!"
   except:
      print "Unexpectable test failure: elf->exe"
      print "With options: %s" %(elf2e32+caps+elfin+longtail)
   finally:
      print "\n"

if __name__ == "__main__":
   # execute only if run as a script
   run()