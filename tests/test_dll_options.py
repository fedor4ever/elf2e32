# encoding=utf-8
import os, subprocess

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
tail=r" --dlldata --ignorenoncallable --uncompressed"

elf2e32=r"D:\codeblock\elf2e32\bin\Debug\elf2e32.exe"
elf2e32+=r" --debuggable --smpsafe"

# elf2e32=r"elf2e32.exe"

# --capability=All-TCB --definput="tests\libcryptou.def" --defoutput="tests\out.def" --elfinput="tests\libcrypto.dll" --output="tests\libcrypto-2.4.5.tst.dll" --libpath="D:\Symbian\S60_5th_Edition_SDK_v1.0\epoc32\release\armv5\lib" --linkas="libcrypto{000a0000}.dll" --dso="tests\libcrypto{000a0000}.dso" --fpu=softvfp --uid1=0x10000079 --uid2=0x20004c45 --uid3=0x00000000 --targettype=STDDLL --dlldata --ignorenoncallable --debuggable --smpsafe --uncompressed

counter=0
longtail=e32bin+implibs+linkas+dsoout+fpu+iud1+uid2+uid3+tgttype+tail

args1=(

("Test #%d: binary creation with outdated def file",
elf2e32+caps+defout+elfin+longtail+r" --unfrozen"+r""" --definput="libcryptou_openssl.def" """,
elf2e32+caps+defout+elfin+longtail+r" --unfrozen"+r""" --definput="libcryptou_openssl.def" """,
),
("Test #%d: Full options list",
elf2e32+caps+defin+defout+elfin+longtail,
"Full options list!",
),
("Test #%d: defin without args",
elf2e32+caps+defin.split("=")[0]+defout+elfin+longtail,
defin.split("=")[0],
),
("Test #%d: defin and defout without args",
elf2e32+caps+defin.split("=")[0]+defout.split("=")[0]+elfin+longtail,
defin.split("=")[0]+defout.split("=")[0],
),
("Test #%d: create all except defoutput",
elf2e32+caps+defin+defout.split("=")[0]+elfin+longtail,
defout.split("=")[0],
),
("Test #%d: dso2def conversion",
elf2e32+""" --elfinput="libcrypto{000a0000}.dso" """+defout,
elf2e32+""" --elfinput="libcrypto{000a0000}.dso" """+defout,
),
("Test #%d: def2def conversion",
elf2e32+defin+ """ --defoutput="tmp\def2def.def" """,
elf2e32+defin+ """ --defoutput="tmp\def2def.def" """,
),
("Test #%d: simple binary creation",
elf2e32+elfin+""" --output="tmp\elf2baree32.dll" """+implibs+tgttype+linkas+tail,
elf2e32+elfin+""" --output="tmp\elf2baree32.dll" """+implibs+tgttype+linkas+tail,
),
("Test #%d: ful binary creation for ECOM plugin",
elf2e32+implibs+""" --capability=ProtServ --defoutput=tmp\AlternateReaderRecog{000a0000}.def --elfinput="AlternateReaderRecog.dll" --output="tmp\AlternateReaderRecogE32.dll"  --linkas=AlternateReaderRecog{000a0000}[101ff1ec].dll --dso=tmp\AlternateReaderRecog{000a0000}.dso --fpu=softvfp --uid1=0x10000079 --uid2=0x10009d8d --uid3=0x101ff1ec --targettype=PLUGIN --sid=0x101ff1ec --version=10.0 --ignorenoncallable --debuggable --smpsafe --sysdef=_Z24ImplementationGroupProxyRi,1; """,

elf2e32+implibs+""" --capability=ProtServ --defoutput=tmp\AlternateReaderRecog{000a0000}.def --elfinput="AlternateReaderRecog.dll" --output="tmp\AlternateReaderRecogE32.dll"  --linkas=AlternateReaderRecog{000a0000}[101ff1ec].dll --dso=tmp\AlternateReaderRecog{000a0000}.dso --fpu=softvfp --uid1=0x10000079 --uid2=0x10009d8d --uid3=0x101ff1ec --targettype=PLUGIN --sid=0x101ff1ec --version=10.0 --ignorenoncallable --debuggable --smpsafe --sysdef=_Z24ImplementationGroupProxyRi,1; """,
) )

def SuceededTests(*args):
   """These tests must alwais pass!"""
   global counter
   tmp=args[0]
   try:
      longtail=e32bin %counter+implibs+linkas+dsoout %counter+fpu+iud1+uid2+uid3+tgttype+tail
      print tmp[0] %counter
      tmp1=tmp[1].replace("%02d", str(counter))
      print tmp1
      print "\n"
      subprocess.check_call(tmp1)
   except:
      print "Unexpectable test failure:\n %s" %tmp[2]
   finally:
      print "\n"
      counter+=1

args=(
("Test #%d: elfin without args",
elf2e32+caps+defin+defout+elfin.split("=")[0]+longtail,
elfin.split("=")[0],
),
("Test #%d: e32image creation without args in params defin defout elfin",
elf2e32+caps+defin.split("=")[0]+defout.split("=")[0]+elfin.split("=")[0]+longtail,
elf2e32+caps+defin.split("=")[0]+defout.split("=")[0]+elfin.split("=")[0]+longtail,
),
("Test #%d: elf2e32 conversion without other options",
elf2e32+elfin+""" --output="tmp\elf2baree32.dll" """,
elf2e32+elfin+""" --output="tmp\elf2baree32.dll" """,
) )

# try:
   # print  %counter
   # print 
   # subprocess.check_call()
# except:
   # print "Unexpectable test failure: %s" %()
# finally:
   # print "\n"
   # counter+=1

def FailureTests(*arg):
   """These tests must alwais fail!"""
   global counter
   tmp=arg[0]
   try:
      longtail=e32bin %counter+implibs+linkas+dsoout %counter+fpu+iud1+uid2+uid3+tgttype+tail
      print tmp[0] %counter
      tmp1=tmp[1].replace("%02d", str(counter))
      print tmp1
      subprocess.check_call(tmp1)
   except:
      print "Expectable test failure: %s" %tmp[2]
   finally:
      print "\n"
      counter+=1

def run():
   print "Tests running"
   for x in args1:
      SuceededTests(x)
   for y in args:
      FailureTests(y)

if __name__ == "__main__":
    # execute only if run as a script
   run()
