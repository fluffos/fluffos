# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "MudOS.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Debug"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : $(OUTDIR)/MudOS.exe $(OUTDIR)/MudOS.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /FR /c
# ADD CPP /nologo /G4 /MT /W3 /GX /YX /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "WIN95" /FR /c
CPP_PROJ=/nologo /G4 /MT /W3 /GX /YX /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D\
 "WIN95" /FR$(INTDIR)/ /Fp$(OUTDIR)/"MudOS.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"MudOS.bsc" 
BSC32_SBRS= \
	$(INTDIR)/grammar_tab.sbr \
	$(INTDIR)/lex.sbr \
	$(INTDIR)/main.sbr \
	$(INTDIR)/rc.sbr \
	$(INTDIR)/interpret.sbr \
	$(INTDIR)/simulate.sbr \
	$(INTDIR)/file.sbr \
	$(INTDIR)/object.sbr \
	$(INTDIR)/backend.sbr \
	$(INTDIR)/array.sbr \
	$(INTDIR)/mapping.sbr \
	$(INTDIR)/ed.sbr \
	$(INTDIR)/comm.sbr \
	$(INTDIR)/regexp.sbr \
	$(INTDIR)/swap.sbr \
	$(INTDIR)/buffer.sbr \
	$(INTDIR)/crc32.sbr \
	$(INTDIR)/malloc.sbr \
	$(INTDIR)/class.sbr \
	$(INTDIR)/efuns_main.sbr \
	$(INTDIR)/call_out.sbr \
	$(INTDIR)/otable.sbr \
	$(INTDIR)/dumpstat.sbr \
	$(INTDIR)/stralloc.sbr \
	$(INTDIR)/port.sbr \
	$(INTDIR)/reclaim.sbr \
	$(INTDIR)/parse.sbr \
	$(INTDIR)/simul_efun.sbr \
	$(INTDIR)/sprintf.sbr \
	$(INTDIR)/program.sbr \
	$(INTDIR)/compiler.sbr \
	$(INTDIR)/avltree.sbr \
	$(INTDIR)/icode.sbr \
	$(INTDIR)/trees.sbr \
	$(INTDIR)/generate.sbr \
	$(INTDIR)/scratchpad.sbr \
	$(INTDIR)/socket_efuns.sbr \
	$(INTDIR)/socket_ctrl.sbr \
	$(INTDIR)/qsort.sbr \
	$(INTDIR)/eoperators.sbr \
	$(INTDIR)/socket_err.sbr \
	$(INTDIR)/md.sbr \
	$(INTDIR)/strstr.sbr \
	$(INTDIR)/disassembler.sbr \
	$(INTDIR)/binaries.sbr \
	$(INTDIR)/replace_program.sbr \
	$(INTDIR)/ccode.sbr \
	$(INTDIR)/cfuns.sbr \
	$(INTDIR)/compile_file.sbr \
	$(INTDIR)/contrib.sbr \
	$(INTDIR)/sockets.sbr \
	$(INTDIR)/develop.sbr \
	$(INTDIR)/math.sbr \
	$(INTDIR)/interface.sbr \
	$(INTDIR)/hash.sbr \
	$(INTDIR)/efuns_port.sbr \
	$(INTDIR)/mallocwrapper.sbr \
	$(INTDIR)/parser.sbr \
	$(INTDIR)/crypt.sbr

$(OUTDIR)/MudOS.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:console /MACHINE:I386
! # ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /NOLOGO /VERSION:22,32 /SUBSYSTEM:console /PDB:none /MACHINE:I386
! # SUBTRACT LINK32 /DEBUG
  LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
   advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
!  odbccp32.lib wsock32.lib /NOLOGO /VERSION:22,32 /SUBSYSTEM:console /PDB:none\
!  /MACHINE:I386 /OUT:$(OUTDIR)/"MudOS.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/grammar_tab.obj \
	$(INTDIR)/lex.obj \
	$(INTDIR)/main.obj \
	$(INTDIR)/rc.obj \
	$(INTDIR)/interpret.obj \
	$(INTDIR)/simulate.obj \
	$(INTDIR)/file.obj \
	$(INTDIR)/object.obj \
	$(INTDIR)/backend.obj \
	$(INTDIR)/array.obj \
	$(INTDIR)/mapping.obj \
	$(INTDIR)/ed.obj \
	$(INTDIR)/comm.obj \
	$(INTDIR)/regexp.obj \
	$(INTDIR)/swap.obj \
	$(INTDIR)/buffer.obj \
	$(INTDIR)/crc32.obj \
	$(INTDIR)/malloc.obj \
	$(INTDIR)/class.obj \
	$(INTDIR)/efuns_main.obj \
	$(INTDIR)/call_out.obj \
	$(INTDIR)/otable.obj \
	$(INTDIR)/dumpstat.obj \
	$(INTDIR)/stralloc.obj \
	$(INTDIR)/port.obj \
	$(INTDIR)/reclaim.obj \
	$(INTDIR)/parse.obj \
	$(INTDIR)/simul_efun.obj \
	$(INTDIR)/sprintf.obj \
	$(INTDIR)/program.obj \
	$(INTDIR)/compiler.obj \
	$(INTDIR)/avltree.obj \
	$(INTDIR)/icode.obj \
	$(INTDIR)/trees.obj \
	$(INTDIR)/generate.obj \
	$(INTDIR)/scratchpad.obj \
	$(INTDIR)/socket_efuns.obj \
	$(INTDIR)/socket_ctrl.obj \
	$(INTDIR)/qsort.obj \
	$(INTDIR)/eoperators.obj \
	$(INTDIR)/socket_err.obj \
	$(INTDIR)/md.obj \
	$(INTDIR)/strstr.obj \
	$(INTDIR)/disassembler.obj \
	$(INTDIR)/binaries.obj \
	$(INTDIR)/replace_program.obj \
	$(INTDIR)/ccode.obj \
	$(INTDIR)/cfuns.obj \
	$(INTDIR)/compile_file.obj \
	$(INTDIR)/contrib.obj \
	$(INTDIR)/sockets.obj \
	$(INTDIR)/develop.obj \
	$(INTDIR)/math.obj \
	$(INTDIR)/interface.obj \
	$(INTDIR)/hash.obj \
	$(INTDIR)/efuns_port.obj \
	$(INTDIR)/mallocwrapper.obj \
	$(INTDIR)/parser.obj \
	$(INTDIR)/crypt.obj

$(OUTDIR)/MudOS.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : $(OUTDIR)/MudOS.exe $(OUTDIR)/MudOS.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /FR /c
# ADD CPP /nologo /MT /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "WIN95" /D "DEBUG" /FR /c
# SUBTRACT CPP /Gy
CPP_PROJ=/nologo /MT /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /D "WIN95" /D "DEBUG" /FR$(INTDIR)/ /Fp$(OUTDIR)/"MudOS.pch" /Fo$(INTDIR)/\
   /Fd$(OUTDIR)/"MudOS.pdb" /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"MudOS.bsc" 
BSC32_SBRS= \
	$(INTDIR)/grammar_tab.sbr \
	$(INTDIR)/lex.sbr \
	$(INTDIR)/main.sbr \
	$(INTDIR)/rc.sbr \
	$(INTDIR)/interpret.sbr \
	$(INTDIR)/simulate.sbr \
	$(INTDIR)/file.sbr \
	$(INTDIR)/object.sbr \
	$(INTDIR)/backend.sbr \
	$(INTDIR)/array.sbr \
	$(INTDIR)/mapping.sbr \
	$(INTDIR)/ed.sbr \
	$(INTDIR)/comm.sbr \
	$(INTDIR)/regexp.sbr \
	$(INTDIR)/swap.sbr \
	$(INTDIR)/buffer.sbr \
	$(INTDIR)/crc32.sbr \
	$(INTDIR)/malloc.sbr \
	$(INTDIR)/class.sbr \
	$(INTDIR)/efuns_main.sbr \
	$(INTDIR)/call_out.sbr \
	$(INTDIR)/otable.sbr \
	$(INTDIR)/dumpstat.sbr \
	$(INTDIR)/stralloc.sbr \
	$(INTDIR)/port.sbr \
	$(INTDIR)/reclaim.sbr \
	$(INTDIR)/parse.sbr \
	$(INTDIR)/simul_efun.sbr \
	$(INTDIR)/sprintf.sbr \
	$(INTDIR)/program.sbr \
	$(INTDIR)/compiler.sbr \
	$(INTDIR)/avltree.sbr \
	$(INTDIR)/icode.sbr \
	$(INTDIR)/trees.sbr \
	$(INTDIR)/generate.sbr \
	$(INTDIR)/scratchpad.sbr \
	$(INTDIR)/socket_efuns.sbr \
	$(INTDIR)/socket_ctrl.sbr \
	$(INTDIR)/qsort.sbr \
	$(INTDIR)/eoperators.sbr \
	$(INTDIR)/socket_err.sbr \
	$(INTDIR)/md.sbr \
	$(INTDIR)/strstr.sbr \
	$(INTDIR)/disassembler.sbr \
	$(INTDIR)/binaries.sbr \
	$(INTDIR)/replace_program.sbr \
	$(INTDIR)/ccode.sbr \
	$(INTDIR)/cfuns.sbr \
	$(INTDIR)/compile_file.sbr \
	$(INTDIR)/contrib.sbr \
	$(INTDIR)/sockets.sbr \
	$(INTDIR)/develop.sbr \
	$(INTDIR)/math.sbr \
	$(INTDIR)/interface.sbr \
	$(INTDIR)/hash.sbr \
	$(INTDIR)/efuns_port.sbr \
	$(INTDIR)/mallocwrapper.sbr \
	$(INTDIR)/parser.sbr \
	$(INTDIR)/crypt.sbr

$(OUTDIR)/MudOS.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:console /DEBUG /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /NOLOGO /SUBSYSTEM:console /DEBUG /MACHINE:I386
# SUBTRACT LINK32 /PDB:none
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib wsock32.lib /NOLOGO /SUBSYSTEM:console /INCREMENTAL:yes\
 /PDB:$(OUTDIR)/"MudOS.pdb" /DEBUG /MACHINE:I386 /OUT:$(OUTDIR)/"MudOS.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/grammar_tab.obj \
	$(INTDIR)/lex.obj \
	$(INTDIR)/main.obj \
	$(INTDIR)/rc.obj \
	$(INTDIR)/interpret.obj \
	$(INTDIR)/simulate.obj \
	$(INTDIR)/file.obj \
	$(INTDIR)/object.obj \
	$(INTDIR)/backend.obj \
	$(INTDIR)/array.obj \
	$(INTDIR)/mapping.obj \
	$(INTDIR)/ed.obj \
	$(INTDIR)/comm.obj \
	$(INTDIR)/regexp.obj \
	$(INTDIR)/swap.obj \
	$(INTDIR)/buffer.obj \
	$(INTDIR)/crc32.obj \
	$(INTDIR)/malloc.obj \
	$(INTDIR)/class.obj \
	$(INTDIR)/efuns_main.obj \
	$(INTDIR)/call_out.obj \
	$(INTDIR)/otable.obj \
	$(INTDIR)/dumpstat.obj \
	$(INTDIR)/stralloc.obj \
	$(INTDIR)/port.obj \
	$(INTDIR)/reclaim.obj \
	$(INTDIR)/parse.obj \
	$(INTDIR)/simul_efun.obj \
	$(INTDIR)/sprintf.obj \
	$(INTDIR)/program.obj \
	$(INTDIR)/compiler.obj \
	$(INTDIR)/avltree.obj \
	$(INTDIR)/icode.obj \
	$(INTDIR)/trees.obj \
	$(INTDIR)/generate.obj \
	$(INTDIR)/scratchpad.obj \
	$(INTDIR)/socket_efuns.obj \
	$(INTDIR)/socket_ctrl.obj \
	$(INTDIR)/qsort.obj \
	$(INTDIR)/eoperators.obj \
	$(INTDIR)/socket_err.obj \
	$(INTDIR)/md.obj \
	$(INTDIR)/strstr.obj \
	$(INTDIR)/disassembler.obj \
	$(INTDIR)/binaries.obj \
	$(INTDIR)/replace_program.obj \
	$(INTDIR)/ccode.obj \
	$(INTDIR)/cfuns.obj \
	$(INTDIR)/compile_file.obj \
	$(INTDIR)/contrib.obj \
	$(INTDIR)/sockets.obj \
	$(INTDIR)/develop.obj \
	$(INTDIR)/math.obj \
	$(INTDIR)/interface.obj \
	$(INTDIR)/hash.obj \
	$(INTDIR)/efuns_port.obj \
	$(INTDIR)/mallocwrapper.obj \
	$(INTDIR)/parser.obj \
	$(INTDIR)/crypt.obj

$(OUTDIR)/MudOS.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=.\grammar_tab.c
DEP_GRAMM=\
	.\std.h\
	.\compiler.h\
	.\lex.h\
	.\scratchpad.h\
	.\lpc_incl.h\
	.\simul_efun.h\
	.\generate.h

$(INTDIR)/grammar_tab.obj :  $(SOURCE)  $(DEP_GRAMM) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lex.c

$(INTDIR)/lex.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\main.c
DEP_MAIN_=\
	.\std.h

$(INTDIR)/main.obj :  $(SOURCE)  $(DEP_MAIN_) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\rc.c

$(INTDIR)/rc.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\interpret.c
DEP_INTER=\
	.\std.h

$(INTDIR)/interpret.obj :  $(SOURCE)  $(DEP_INTER) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\simulate.c
DEP_SIMUL=\
	.\std.h

$(INTDIR)/simulate.obj :  $(SOURCE)  $(DEP_SIMUL) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\file.c

$(INTDIR)/file.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\object.c
DEP_OBJEC=\
	.\std.h

$(INTDIR)/object.obj :  $(SOURCE)  $(DEP_OBJEC) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\backend.c

$(INTDIR)/backend.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\array.c
DEP_ARRAY=\
	.\std.h

$(INTDIR)/array.obj :  $(SOURCE)  $(DEP_ARRAY) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mapping.c

$(INTDIR)/mapping.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ed.c

$(INTDIR)/ed.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\comm.c
DEP_COMM_=\
	.\std.h\
	.\network_incl.h\
	.\lpc_incl.h\
	.\applies.h\
	.\main.h\
	.\comm.h\
	.\socket_efuns.h\
	.\backend.h\
	.\socket_ctrl.h\
	.\eoperators.h\
	.\debug.h\
	.\ed.h\
	.\file.h

$(INTDIR)/comm.obj :  $(SOURCE)  $(DEP_COMM_) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\regexp.c

$(INTDIR)/regexp.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\swap.c
DEP_SWAP_=\
	.\std.h

$(INTDIR)/swap.obj :  $(SOURCE)  $(DEP_SWAP_) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\buffer.c

$(INTDIR)/buffer.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\crc32.c
DEP_CRC32=\
	.\std.h

$(INTDIR)/crc32.obj :  $(SOURCE)  $(DEP_CRC32) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\malloc.c

$(INTDIR)/malloc.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\class.c
DEP_CLASS=\
	.\std.h

$(INTDIR)/class.obj :  $(SOURCE)  $(DEP_CLASS) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\efuns_main.c

$(INTDIR)/efuns_main.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\call_out.c
DEP_CALL_=\
	.\std.h

$(INTDIR)/call_out.obj :  $(SOURCE)  $(DEP_CALL_) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\otable.c
DEP_OTABL=\
	.\std.h

$(INTDIR)/otable.obj :  $(SOURCE)  $(DEP_OTABL) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dumpstat.c
DEP_DUMPS=\
	.\std.h

$(INTDIR)/dumpstat.obj :  $(SOURCE)  $(DEP_DUMPS) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\stralloc.c
DEP_STRAL=\
	.\std.h

$(INTDIR)/stralloc.obj :  $(SOURCE)  $(DEP_STRAL) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\port.c
DEP_PORT_=\
	.\std.h

$(INTDIR)/port.obj :  $(SOURCE)  $(DEP_PORT_) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\reclaim.c

$(INTDIR)/reclaim.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\parse.c

$(INTDIR)/parse.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\simul_efun.c
DEP_SIMUL_=\
	.\std.h

$(INTDIR)/simul_efun.obj :  $(SOURCE)  $(DEP_SIMUL_) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sprintf.c

$(INTDIR)/sprintf.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\program.c
DEP_PROGR=\
	.\std.h

$(INTDIR)/program.obj :  $(SOURCE)  $(DEP_PROGR) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\compiler.c
DEP_COMPI=\
	.\std.h

$(INTDIR)/compiler.obj :  $(SOURCE)  $(DEP_COMPI) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\avltree.c

$(INTDIR)/avltree.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\icode.c

$(INTDIR)/icode.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\trees.c

$(INTDIR)/trees.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\generate.c
DEP_GENER=\
	.\std.h

$(INTDIR)/generate.obj :  $(SOURCE)  $(DEP_GENER) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\scratchpad.c
DEP_SCRAT=\
	.\std.h

$(INTDIR)/scratchpad.obj :  $(SOURCE)  $(DEP_SCRAT) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\socket_efuns.c

$(INTDIR)/socket_efuns.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\socket_ctrl.c
DEP_SOCKE=\
	.\std.h

$(INTDIR)/socket_ctrl.obj :  $(SOURCE)  $(DEP_SOCKE) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\qsort.c

$(INTDIR)/qsort.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\eoperators.c

$(INTDIR)/eoperators.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\socket_err.c

$(INTDIR)/socket_err.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\md.c
DEP_MD_C2a=\
	.\std.h

$(INTDIR)/md.obj :  $(SOURCE)  $(DEP_MD_C2a) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\strstr.c

$(INTDIR)/strstr.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\disassembler.c

$(INTDIR)/disassembler.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\binaries.c

$(INTDIR)/binaries.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\replace_program.c
DEP_REPLA=\
	.\std.h

$(INTDIR)/replace_program.obj :  $(SOURCE)  $(DEP_REPLA) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ccode.c

$(INTDIR)/ccode.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cfuns.c
DEP_CFUNS=\
	.\std.h

$(INTDIR)/cfuns.obj :  $(SOURCE)  $(DEP_CFUNS) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\compile_file.c
DEP_COMPIL=\
	.\std.h

$(INTDIR)/compile_file.obj :  $(SOURCE)  $(DEP_COMPIL) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\packages\contrib.c

$(INTDIR)/contrib.obj :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\packages\sockets.c

$(INTDIR)/sockets.obj :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\packages\develop.c

$(INTDIR)/develop.obj :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\packages\math.c

$(INTDIR)/math.obj :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mudlib\interface.c

$(INTDIR)/interface.obj :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\hash.c
DEP_HASH_=\
	.\std.h

$(INTDIR)/hash.obj :  $(SOURCE)  $(DEP_HASH_) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\efuns_port.c

$(INTDIR)/efuns_port.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mallocwrapper.c

$(INTDIR)/mallocwrapper.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\packages\parser.c

$(INTDIR)/parser.obj :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\amiga\crypt.c

$(INTDIR)/crypt.obj :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\local_options
# End Source File
# End Group
# End Project
