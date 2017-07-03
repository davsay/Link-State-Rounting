#Joshua Zarin
#Makefile for Programming Project 3
#CS457
#October 27, 2016

#Defining all files being used/created
EXEC 	= manager router target
TEXEC 	= Joshua_Zarin_P3
CXX		= g++
CFLAGS	= -std=c++11 -Wall -I. -o

FILES 	= manager.cpp router.cpp Makefile project3.h README.txt
OBJ 	= $(SRC:.cpp=.o)
TAR 	= $(TEXEC).tar 
TARS	= $(TAR)  $(HED) Makefile
CLN		= $(EXEC) $(OBJ) *.out

#standard make command
all: $(EXEC)

#only make awget
manager: manager.cpp
	$(CXX) $(CFLAGS) manager manager.cpp

#only make ss
router: router.cpp
	$(CXX) -lpthread $(CFLAGS) router router.cpp 

target: 
	tar cf Joshua_Zarin_P3.tar $(FILES) 

#clean the directory
.PHONY: clean
clean:
	rm -f $(CLN)

.PHONY: cleanclear
cleanclear:
	rm -f $(CLN) | clear

