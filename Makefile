	#######################################
	#  Makefile for the RedBase project   #
	#######################################


# Set this variable to 1 if you want to turn on display of debugging
# messages. Set it to 0 if you want to turn off display of display of
# debugging messages. If you do change the value of this variable, you have
# to do a "make clean" before doing a "make".
DEBUG = 1

# Uncomment line 1 and comment line 2 for producing an
# executable containing debug info (for use with gdb).
# -- OR --
# Comment line 1 and uncomment line 2 for producing an
# optimized executable.
X = -g    # line 1
#X = -O6  # line 2

#CFLAGS = $(X) -c -Wall -Werror -ansi -DDEBUG=$(DEBUG)
CFLAGS = $(X) -c -Wall  -ansi -DDEBUG=$(DEBUG)
CPP = g++

# ****** Parser ******
PF_OBJS = PF.o PF_PageHandle.o BF_Manager.o PF_FileHandle.o PF_Manager.o
SM_OBJS = SM_Tuple.o SM_Manager.o
RM_OBJS = RID.o RM_Record.o RM_PageHandle.o RM_FileHandle.o RM_Manager.o RM_FileScan.o
ALL_OBJS = $(PF_OBJS) $(RM_OBJS) $(SM_OBJS)
ALL_EXE = redbase dbcreate dbdestroy

all: $(ALL_EXE)
redbase:  $(ALL_OBJS) lex.yy.o y.tab.o
	$(CPP) -o redbase lex.yy.o y.tab.o $(ALL_OBJS) -lfl
dbcreate: $(ALL_OBJS) dbcreate.cpp
	$(CPP) -o dbcreate -g -Wall -Werror -ansi dbcreate.cpp $(ALL_OBJS)
dbdestroy: $(ALL_OBJS) dbdestroy.cpp
	$(CPP) -o dbdestroy -g -Wall -Werror -ansi dbdestroy.cpp $(ALL_OBJS)
y.tab.o: y.tab.cpp y.tab.h
	$(CPP) $(CFLAGS) y.tab.cpp
y.tab.cpp y.tab.h: redbase.y
	yacc -d redbase.y 
	mv y.tab.c y.tab.cpp
lex.yy.o: lex.yy.cpp y.tab.h
	$(CPP) $(CFLAGS) lex.yy.cpp
lex.yy.cpp: redbase.l
	flex -+ redbase.l
	mv lex.yy.cc lex.yy.cpp

# ****** SM ******
SM_Manager.o: SM_Manager.cpp SM_Manager.h SM_Tuple.o
	$(CPP) $(CFLAGS) SM_Manager.cpp
SM_Tuple.o: SM_Tuple.cpp SM_Tuple.h SM.h $(RM_OBJS)
	$(CPP) $(CFLAGS) SM_Tuple.cpp

# ****** RM ******
RM_FileScan.o: RM_FileScan.cpp RM_FileScan.h RM_FileHandle.o
	$(CPP) $(CFLAGS) RM_FileScan.cpp
RM_Manager.o: RM_Manager.cpp RM_Manager.h RM_FileHandle.o
	$(CPP) $(CFLAGS) RM_Manager.cpp
RM_FileHandle.o: RM_FileHandle.cpp RM_FileHandle.h RM_PageHandle.o RM_Record.o RID.o
	$(CPP) $(CFLAGS) RM_FileHandle.cpp
RM_PageHandle.o: RM_PageHandle.cpp RM_PageHandle.h 
	$(CPP) $(CFLAGS) RM_PageHandle.cpp
RM_Record.o: RM_Record.cpp RM_Record.h RID.o
	$(CPP) $(CFLAGS) RM_Record.cpp
RID.o: RID.h RID.cpp RM.h $(PF_OBJS)
	$(CPP) $(CFLAGS) RID.cpp

# ****** PF ******
PF_Manager.o: PF_Manager.cpp PF_Manager.h PF_FileHandle.o
	$(CPP) $(CFLAGS) PF_Manager.cpp 
PF_FileHandle.o: PF_FileHandle.cpp PF_FileHandle.h PF_PageHandle.o BF_Manager.o
	$(CPP) $(CFLAGS) PF_FileHandle.cpp 
BF_Manager.o: BF_Manager.cpp BF_Manager.h PF_PageHandle.o
	$(CPP) $(CFLAGS) BF_Manager.cpp 
PF_PageHandle.o: PF_PageHandle.cpp PF_PageHandle.h PF.o
	$(CPP) $(CFLAGS) PF_PageHandle.cpp 
PF.o: PF.h PF.cpp
	$(CPP) $(CFLAGS) PF.cpp


clean:
	rm -f $(ALL_OBJS) $(ALL_EXE) lex.yy.o y.tab.o y.tab.cpp y.tab.h lex.yy.cpp
