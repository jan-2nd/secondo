########################################################################
#
# SECONDO makefile for special Secondo parser
#
########################################################################
# 31.03.2013 Jürgen Ehnes

include ../../../../makefile.env

ifdef SECONDO_HOST_PLATFORM
SECONDO_PLATFORM=$(SECONDO_HOST_PLATFORM)
endif


DEP_FILES += ../SpecParser.tab.dep

ifdef SECONDO_HOST_CXX
CC := $(SECONDO_HOST_CXX)
endif


OBJECTS := NestedText.o lex.yy.o lex2.yy.o
GENFILES := lex.yy.cpp lex2.yy.cpp SpecParser.tab.cpp

all: $(GENFILES) $(OBJECTS) Parser$(EXEEXT) Parser2$(EXEEXT)

NestedText.o: NestedText.cpp

lex.yy.o: lex.yy.cpp

lex2.yy.o: lex2.yy.cpp


lex.yy.cpp lex2.yy.cpp: ../SpecLex.l SpecParser.tab.cpp NestedText.cpp
	$(LEX) -o $@ $<
	$(LEX) -d -o../lex2.yy.cpp $<

NestedText.cpp: ../NestedText.cpp
	$(CP) $< $@     

SpecParser.tab.cpp: ../SpecParser.y
	$(YACC) -d -o $@ $<

Parser$(EXEEXT): SpecParser.tab.o lex.yy.o NestedText.o
	$(CXX) -o Parser$(EXEEXT) $^ $(DEFAULTCCFLAGS) $(COMMON_LD_FLAGS)
	$(CP) $@ ../$@ 

Parser2$(EXEEXT): SpecParser.tab.o lex2.yy.o NestedText.o
	$(CXX) -o Parser2$(EXEEXT) $^ $(DEFAULTCCFLAGS) $(COMMON_LD_FLAGS) 
	$(CP) $@ ../$@

ifneq ($(MAKECMDGOALS),clean)
-include $(DEP_FILES)
endif

clean:
	$(RM) $(GENFILES) $(DEP_FILES) $(OBJECTS) SpecParser.tab.o ../Parser$(EXEEXT) ../Parser2$(EXEEXT) Parser$(EXEEXT) Parser2$(EXEEXT) NestedText.cpp

