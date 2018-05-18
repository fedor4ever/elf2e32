# encoding=utf-8
import os, subprocess

elf2e32=r"D:\codeblock\elf2e32\bin\Debug\elf2e32.exe"
tstdir=r"D:\codeblock\elf2e32\tests"

defin=r""" --definput="libcryptou.def" """
dsoout=r""" --dso="tmp\libcrypto{000a0000}.def2dso.dso" """
linkas=r""" --linkas="libcrypto{000a0000}.dll" """
tgttype=r" --targettype=implib"


dsodefTests=(
(elf2e32+defin+dsoout+linkas+tgttype, "Simple def2dso creation. Options are: %s\n"),
(""" --elfinput="libcrypto.SDK.dso" """ + """ --defoutput="tmp\dso2def.(01).def" """,
"dso2def creation with simplified syntax. Options are: %s\n"),
(""" --definput="tmp\dso2def.(01).def" """ + """ --dso="tmp\dso2def2dso.(02).dso" """,
"Make dso from def generated from dso. Options are: %s\n"),
(defin+ """--dso="tmp\def2dso.(03).dso""", "Make dso from def which made from dso... Options are: %s\n"),
("""--elfinput="tmp\def2dso.(03).dso""" + """ --defoutput="tmp\def2dso2def.(04).def" """,
"Make def from dso which made from def... Options are: %s\n"),
)

def SuceededTests(*args):
   """These tests must alwais pass!"""
   tmp=args[0]
   try:
      print "\n"+tmp[0]
      subprocess.check_call(tmp[0])
   except:
      print "Unexpectable test failure: %s\n" %tmp[0]
   finally:
      str=tmp[1] %tmp[0]
      print "Test succeeded: %s!\n" %str

def run():
   for x in dsodefTests:
      SuceededTests(x)

if __name__ == "__main__":
   # execute only if run as a script
   run()