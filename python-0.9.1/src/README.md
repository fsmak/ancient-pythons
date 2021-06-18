This directory contains the source for the Python interpreter.

To build the interpreter, edit the Makefile, follow the instructions
there, and type "make python".

To use the interpreter, you must set the environment variable PYTHONPATH
to point to the directory containing the standard modules.  These are
distributed as a sister directory called 'lib' of this source directory.
Try importing the module 'testall' to see if everything works.

Good Luck!


# 重新自定义Makefile，主要是开启DEBUG模式
```bash
obj = pythonmain.o acceler.o classobject.o dictobject.o fileobject.o \
      funcobject.o import.o listnode.o methodobject.o node.o parsetok.o \
      regexpmodule.o stringobject.o timemodule.o tupleobject.o \
      bltinmodule.o compile.o errors.o floatobject.o graminit.o \
      intobject.o listobject.o modsupport.o object.o posixmodule.o \
      regexp.o structmember.o tokenizer.o typeobject.o ceval.o \
      config.o fgetsintr.o frameobject.o grammar1.o intrcheck.o \
      mathmodule.o moduleobject.o parser.o strdup.o sysmodule.o \
      traceback.o fmod.o
target = python
CC = gcc

$(target): $(obj)
        $(CC) -g $(obj) -o $(target) -lm

%.o: %.c
        $(CC) -c -g -D DEBUG $< -o $@

clean:
        rm -rf *.o python
```
