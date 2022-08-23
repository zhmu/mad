# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=MAD - Win32 Release
!MESSAGE No configuration specified.  Defaulting to MAD - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "MAD - Win32 Release" && "$(CFG)" != "MAD - Win32 Debug" &&\
 "$(CFG)" != "MAD - Win32 Release with Debugging Info"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Mad.mak" CFG="MAD - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MAD - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "MAD - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "MAD - Win32 Release with Debugging Info" (based on\
 "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "MAD - Win32 Debug"
MTL=mktyplib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MAD - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\Mad.exe"

CLEAN : 
	-@erase "$(INTDIR)\Anm.obj"
	-@erase "$(INTDIR)\Archive.obj"
	-@erase "$(INTDIR)\Bar.obj"
	-@erase "$(INTDIR)\Ctrl_drv.obj"
	-@erase "$(INTDIR)\Debugmad.obj"
	-@erase "$(INTDIR)\Dlgman.obj"
	-@erase "$(INTDIR)\fontman.obj"
	-@erase "$(INTDIR)\Gadgets.obj"
	-@erase "$(INTDIR)\Game.obj"
	-@erase "$(INTDIR)\Gfx.obj"
	-@erase "$(INTDIR)\Gfx_drv.obj"
	-@erase "$(INTDIR)\Inv.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\Objman.obj"
	-@erase "$(INTDIR)\Parser.obj"
	-@erase "$(INTDIR)\Project.obj"
	-@erase "$(INTDIR)\Script.obj"
	-@erase "$(INTDIR)\textman.obj"
	-@erase "$(OUTDIR)\Mad.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O1 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WINDOWS" /D "RELEASE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O1 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "WINDOWS" /D "RELEASE" /Fp"$(INTDIR)/Mad.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Mad.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/Mad.pdb" /machine:I386 /out:"$(OUTDIR)/Mad.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Anm.obj" \
	"$(INTDIR)\Archive.obj" \
	"$(INTDIR)\Bar.obj" \
	"$(INTDIR)\Ctrl_drv.obj" \
	"$(INTDIR)\Debugmad.obj" \
	"$(INTDIR)\Dlgman.obj" \
	"$(INTDIR)\fontman.obj" \
	"$(INTDIR)\Gadgets.obj" \
	"$(INTDIR)\Game.obj" \
	"$(INTDIR)\Gfx.obj" \
	"$(INTDIR)\Gfx_drv.obj" \
	"$(INTDIR)\Inv.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\Objman.obj" \
	"$(INTDIR)\Parser.obj" \
	"$(INTDIR)\Project.obj" \
	"$(INTDIR)\Script.obj" \
	"$(INTDIR)\textman.obj"

"$(OUTDIR)\Mad.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\Mad.exe"

CLEAN : 
	-@erase "$(INTDIR)\Anm.obj"
	-@erase "$(INTDIR)\Archive.obj"
	-@erase "$(INTDIR)\Bar.obj"
	-@erase "$(INTDIR)\Ctrl_drv.obj"
	-@erase "$(INTDIR)\Debugmad.obj"
	-@erase "$(INTDIR)\Dlgman.obj"
	-@erase "$(INTDIR)\fontman.obj"
	-@erase "$(INTDIR)\Gadgets.obj"
	-@erase "$(INTDIR)\Game.obj"
	-@erase "$(INTDIR)\Gfx.obj"
	-@erase "$(INTDIR)\Gfx_drv.obj"
	-@erase "$(INTDIR)\Inv.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\Objman.obj"
	-@erase "$(INTDIR)\Parser.obj"
	-@erase "$(INTDIR)\Project.obj"
	-@erase "$(INTDIR)\Script.obj"
	-@erase "$(INTDIR)\textman.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\Mad.exe"
	-@erase "$(OUTDIR)\Mad.ilk"
	-@erase "$(OUTDIR)\Mad.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "WINDOWS" /D "DEBUG" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "WINDOWS" /D "DEBUG" /Fp"$(INTDIR)/Mad.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /D "DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /D "DEBUG" /win32 
# ADD BASE RSC /l 0x413 /d "_DEBUG"
# ADD RSC /l 0x413 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Mad.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/Mad.pdb" /debug /machine:I386 /out:"$(OUTDIR)/Mad.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Anm.obj" \
	"$(INTDIR)\Archive.obj" \
	"$(INTDIR)\Bar.obj" \
	"$(INTDIR)\Ctrl_drv.obj" \
	"$(INTDIR)\Debugmad.obj" \
	"$(INTDIR)\Dlgman.obj" \
	"$(INTDIR)\fontman.obj" \
	"$(INTDIR)\Gadgets.obj" \
	"$(INTDIR)\Game.obj" \
	"$(INTDIR)\Gfx.obj" \
	"$(INTDIR)\Gfx_drv.obj" \
	"$(INTDIR)\Inv.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\Objman.obj" \
	"$(INTDIR)\Parser.obj" \
	"$(INTDIR)\Project.obj" \
	"$(INTDIR)\Script.obj" \
	"$(INTDIR)\textman.obj"

"$(OUTDIR)\Mad.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "MAD___W0"
# PROP BASE Intermediate_Dir "MAD___W0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "MAD___W0"
# PROP Intermediate_Dir "MAD___W0"
# PROP Target_Dir ""
OUTDIR=.\MAD___W0
INTDIR=.\MAD___W0

ALL : "$(OUTDIR)\Mad.exe"

CLEAN : 
	-@erase "$(INTDIR)\Anm.obj"
	-@erase "$(INTDIR)\Archive.obj"
	-@erase "$(INTDIR)\Bar.obj"
	-@erase "$(INTDIR)\Ctrl_drv.obj"
	-@erase "$(INTDIR)\Debugmad.obj"
	-@erase "$(INTDIR)\Dlgman.obj"
	-@erase "$(INTDIR)\fontman.obj"
	-@erase "$(INTDIR)\Gadgets.obj"
	-@erase "$(INTDIR)\Game.obj"
	-@erase "$(INTDIR)\Gfx.obj"
	-@erase "$(INTDIR)\Gfx_drv.obj"
	-@erase "$(INTDIR)\Inv.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\Objman.obj"
	-@erase "$(INTDIR)\Parser.obj"
	-@erase "$(INTDIR)\Project.obj"
	-@erase "$(INTDIR)\Script.obj"
	-@erase "$(INTDIR)\textman.obj"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\Mad.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WINDOWS" /D "RELEASE" /YX /c
# ADD CPP /nologo /W3 /GX /Zi /Od /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WINDOWS" /D "RELEASE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /Zi /Od /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "WINDOWS" /D "RELEASE" /Fp"$(INTDIR)/Mad.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\MAD___W0/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Mad.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ddraw.lib dinput.lib dxguid.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/Mad.pdb" /machine:I386 /out:"$(OUTDIR)/Mad.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Anm.obj" \
	"$(INTDIR)\Archive.obj" \
	"$(INTDIR)\Bar.obj" \
	"$(INTDIR)\Ctrl_drv.obj" \
	"$(INTDIR)\Debugmad.obj" \
	"$(INTDIR)\Dlgman.obj" \
	"$(INTDIR)\fontman.obj" \
	"$(INTDIR)\Gadgets.obj" \
	"$(INTDIR)\Game.obj" \
	"$(INTDIR)\Gfx.obj" \
	"$(INTDIR)\Gfx_drv.obj" \
	"$(INTDIR)\Inv.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\Objman.obj" \
	"$(INTDIR)\Parser.obj" \
	"$(INTDIR)\Project.obj" \
	"$(INTDIR)\Script.obj" \
	"$(INTDIR)\textman.obj"

"$(OUTDIR)\Mad.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "MAD - Win32 Release"
# Name "MAD - Win32 Debug"
# Name "MAD - Win32 Release with Debugging Info"

!IF  "$(CFG)" == "MAD - Win32 Release"

!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"

!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\Projects\Mad\Source\Anm.cpp
DEP_CPP_ANM_C=\
	"..\anm.h"\
	"..\archive.h"\
	"..\mad.h"\
	"..\maderror.h"\
	

!IF  "$(CFG)" == "MAD - Win32 Release"


"$(INTDIR)\Anm.obj" : $(SOURCE) $(DEP_CPP_ANM_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"


"$(INTDIR)\Anm.obj" : $(SOURCE) $(DEP_CPP_ANM_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"


"$(INTDIR)\Anm.obj" : $(SOURCE) $(DEP_CPP_ANM_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\Mad\Source\Archive.cpp
DEP_CPP_ARCHI=\
	"..\archive.h"\
	"..\mad.h"\
	"..\maderror.h"\
	

!IF  "$(CFG)" == "MAD - Win32 Release"


"$(INTDIR)\Archive.obj" : $(SOURCE) $(DEP_CPP_ARCHI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"


"$(INTDIR)\Archive.obj" : $(SOURCE) $(DEP_CPP_ARCHI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"


"$(INTDIR)\Archive.obj" : $(SOURCE) $(DEP_CPP_ARCHI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\Mad\Source\Bar.cpp
DEP_CPP_BAR_C=\
	"..\anm.h"\
	"..\archive.h"\
	"..\bar.h"\
	"..\controls.h"\
	"..\debugmad.h"\
	"..\dlgman.h"\
	"..\gfx.h"\
	"..\gfx_drv.h"\
	"..\inv.h"\
	"..\mad.h"\
	"..\maderror.h"\
	"..\objman.h"\
	"..\project.h"\
	"..\script.h"\
	

!IF  "$(CFG)" == "MAD - Win32 Release"


"$(INTDIR)\Bar.obj" : $(SOURCE) $(DEP_CPP_BAR_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"


"$(INTDIR)\Bar.obj" : $(SOURCE) $(DEP_CPP_BAR_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"


"$(INTDIR)\Bar.obj" : $(SOURCE) $(DEP_CPP_BAR_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\Mad\Source\Debugmad.cpp
DEP_CPP_DEBUG=\
	"..\anm.h"\
	"..\archive.h"\
	"..\controls.h"\
	"..\debugmad.h"\
	"..\dlgman.h"\
	"..\fontman.h"\
	"..\gfx.h"\
	"..\gfx_drv.h"\
	"..\mad.h"\
	"..\maderror.h"\
	"..\objman.h"\
	"..\script.h"\
	

!IF  "$(CFG)" == "MAD - Win32 Release"


"$(INTDIR)\Debugmad.obj" : $(SOURCE) $(DEP_CPP_DEBUG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"


"$(INTDIR)\Debugmad.obj" : $(SOURCE) $(DEP_CPP_DEBUG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"


"$(INTDIR)\Debugmad.obj" : $(SOURCE) $(DEP_CPP_DEBUG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\Mad\Source\Gadgets.cpp
DEP_CPP_GADGE=\
	"..\gfx_drv.h"\
	"..\mad.h"\
	

!IF  "$(CFG)" == "MAD - Win32 Release"


"$(INTDIR)\Gadgets.obj" : $(SOURCE) $(DEP_CPP_GADGE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"


"$(INTDIR)\Gadgets.obj" : $(SOURCE) $(DEP_CPP_GADGE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"


"$(INTDIR)\Gadgets.obj" : $(SOURCE) $(DEP_CPP_GADGE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\Mad\Source\Gfx.cpp
DEP_CPP_GFX_C=\
	"..\anm.h"\
	"..\archive.h"\
	"..\bar.h"\
	"..\controls.h"\
	"..\debugmad.h"\
	"..\dlgman.h"\
	"..\fontman.h"\
	"..\gadgets.h"\
	"..\gfx.h"\
	"..\gfx_drv.h"\
	"..\mad.h"\
	"..\maderror.h"\
	"..\objman.h"\
	"..\project.h"\
	"..\script.h"\
	{$(INCLUDE)}"\sys\timeb.h"\
	

!IF  "$(CFG)" == "MAD - Win32 Release"


"$(INTDIR)\Gfx.obj" : $(SOURCE) $(DEP_CPP_GFX_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"


"$(INTDIR)\Gfx.obj" : $(SOURCE) $(DEP_CPP_GFX_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"


"$(INTDIR)\Gfx.obj" : $(SOURCE) $(DEP_CPP_GFX_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\Mad\Source\Main.cpp
DEP_CPP_MAIN_=\
	"..\anm.h"\
	"..\archive.h"\
	"..\bar.h"\
	"..\controls.h"\
	"..\debugmad.h"\
	"..\dlgman.h"\
	"..\fontman.h"\
	"..\gfx.h"\
	"..\gfx_drv.h"\
	"..\inv.h"\
	"..\mad.h"\
	"..\maderror.h"\
	"..\objman.h"\
	"..\parser.h"\
	"..\project.h"\
	"..\script.h"\
	"..\snd_drv.h"\
	"..\textman.h"\
	

!IF  "$(CFG)" == "MAD - Win32 Release"


"$(INTDIR)\Main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"


"$(INTDIR)\Main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"


"$(INTDIR)\Main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\Mad\Source\Project.cpp
DEP_CPP_PROJE=\
	"..\archive.h"\
	"..\mad.h"\
	"..\maderror.h"\
	"..\project.h"\
	

!IF  "$(CFG)" == "MAD - Win32 Release"


"$(INTDIR)\Project.obj" : $(SOURCE) $(DEP_CPP_PROJE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"


"$(INTDIR)\Project.obj" : $(SOURCE) $(DEP_CPP_PROJE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"


"$(INTDIR)\Project.obj" : $(SOURCE) $(DEP_CPP_PROJE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\Mad\Source\Script.cpp
DEP_CPP_SCRIP=\
	"..\anm.h"\
	"..\archive.h"\
	"..\bar.h"\
	"..\controls.h"\
	"..\debugmad.h"\
	"..\dlgman.h"\
	"..\fontman.h"\
	"..\game.h"\
	"..\gfx.h"\
	"..\gfx_drv.h"\
	"..\mad.h"\
	"..\maderror.h"\
	"..\objman.h"\
	"..\parser.h"\
	"..\script.h"\
	"..\textman.h"\
	

!IF  "$(CFG)" == "MAD - Win32 Release"


"$(INTDIR)\Script.obj" : $(SOURCE) $(DEP_CPP_SCRIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"


"$(INTDIR)\Script.obj" : $(SOURCE) $(DEP_CPP_SCRIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"


"$(INTDIR)\Script.obj" : $(SOURCE) $(DEP_CPP_SCRIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Ctrl_drv.cpp
DEP_CPP_CTRL_=\
	"..\controls.h"\
	"..\debugmad.h"\
	"..\dlgman.h"\
	"..\gfx_drv.h"\
	"..\mad.h"\
	"..\maderror.h"\
	"..\types.h"\
	

!IF  "$(CFG)" == "MAD - Win32 Release"


"$(INTDIR)\Ctrl_drv.obj" : $(SOURCE) $(DEP_CPP_CTRL_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"


"$(INTDIR)\Ctrl_drv.obj" : $(SOURCE) $(DEP_CPP_CTRL_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"


"$(INTDIR)\Ctrl_drv.obj" : $(SOURCE) $(DEP_CPP_CTRL_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfx_drv.cpp
DEP_CPP_GFX_D=\
	"..\gfx_drv.h"\
	"..\mad.h"\
	"..\maderror.h"\
	

!IF  "$(CFG)" == "MAD - Win32 Release"


"$(INTDIR)\Gfx_drv.obj" : $(SOURCE) $(DEP_CPP_GFX_D) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"


"$(INTDIR)\Gfx_drv.obj" : $(SOURCE) $(DEP_CPP_GFX_D) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"


"$(INTDIR)\Gfx_drv.obj" : $(SOURCE) $(DEP_CPP_GFX_D) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\Mad\Source\Objman.cpp
DEP_CPP_OBJMA=\
	"..\anm.h"\
	"..\archive.h"\
	"..\controls.h"\
	"..\debugmad.h"\
	"..\dlgman.h"\
	"..\fontman.h"\
	"..\gfx.h"\
	"..\gfx_drv.h"\
	"..\mad.h"\
	"..\objman.h"\
	"..\script.h"\
	

!IF  "$(CFG)" == "MAD - Win32 Release"


"$(INTDIR)\Objman.obj" : $(SOURCE) $(DEP_CPP_OBJMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"


"$(INTDIR)\Objman.obj" : $(SOURCE) $(DEP_CPP_OBJMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"


"$(INTDIR)\Objman.obj" : $(SOURCE) $(DEP_CPP_OBJMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\Mad\Source\fontman.cpp
DEP_CPP_FONTM=\
	"..\anm.h"\
	"..\archive.h"\
	"..\debugmad.h"\
	"..\dlgman.h"\
	"..\fontman.h"\
	"..\gadgets.h"\
	"..\gfx.h"\
	"..\gfx_drv.h"\
	"..\mad.h"\
	"..\maderror.h"\
	"..\objman.h"\
	"..\script.h"\
	

!IF  "$(CFG)" == "MAD - Win32 Release"


"$(INTDIR)\fontman.obj" : $(SOURCE) $(DEP_CPP_FONTM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"


"$(INTDIR)\fontman.obj" : $(SOURCE) $(DEP_CPP_FONTM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"


"$(INTDIR)\fontman.obj" : $(SOURCE) $(DEP_CPP_FONTM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\Mad\Source\Dlgman.cpp
DEP_CPP_DLGMA=\
	"..\anm.h"\
	"..\archive.h"\
	"..\controls.h"\
	"..\debugmad.h"\
	"..\dlgman.h"\
	"..\fontman.h"\
	"..\gadgets.h"\
	"..\gfx.h"\
	"..\gfx_drv.h"\
	"..\mad.h"\
	"..\maderror.h"\
	"..\objman.h"\
	"..\project.h"\
	"..\script.h"\
	

!IF  "$(CFG)" == "MAD - Win32 Release"


"$(INTDIR)\Dlgman.obj" : $(SOURCE) $(DEP_CPP_DLGMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"


"$(INTDIR)\Dlgman.obj" : $(SOURCE) $(DEP_CPP_DLGMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"


"$(INTDIR)\Dlgman.obj" : $(SOURCE) $(DEP_CPP_DLGMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\Mad\Source\Parser.cpp
DEP_CPP_PARSE=\
	"..\anm.h"\
	"..\archive.h"\
	"..\controls.h"\
	"..\debugmad.h"\
	"..\dlgman.h"\
	"..\fontman.h"\
	"..\game.h"\
	"..\gfx.h"\
	"..\gfx_drv.h"\
	"..\inv.h"\
	"..\mad.h"\
	"..\parser.h"\
	"..\project.h"\
	"..\script.h"\
	

!IF  "$(CFG)" == "MAD - Win32 Release"


"$(INTDIR)\Parser.obj" : $(SOURCE) $(DEP_CPP_PARSE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"


"$(INTDIR)\Parser.obj" : $(SOURCE) $(DEP_CPP_PARSE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"


"$(INTDIR)\Parser.obj" : $(SOURCE) $(DEP_CPP_PARSE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\Mad\Source\textman.cpp
DEP_CPP_TEXTM=\
	"..\archive.h"\
	"..\mad.h"\
	"..\maderror.h"\
	"..\textman.h"\
	

!IF  "$(CFG)" == "MAD - Win32 Release"


"$(INTDIR)\textman.obj" : $(SOURCE) $(DEP_CPP_TEXTM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"


"$(INTDIR)\textman.obj" : $(SOURCE) $(DEP_CPP_TEXTM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"


"$(INTDIR)\textman.obj" : $(SOURCE) $(DEP_CPP_TEXTM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\projects\mad\Source\Game.cpp
DEP_CPP_GAME_=\
	"..\anm.h"\
	"..\archive.h"\
	"..\controls.h"\
	"..\debugmad.h"\
	"..\dlgman.h"\
	"..\fontman.h"\
	"..\game.h"\
	"..\gfx.h"\
	"..\gfx_drv.h"\
	"..\mad.h"\
	"..\objman.h"\
	"..\project.h"\
	"..\script.h"\
	"..\textman.h"\
	

!IF  "$(CFG)" == "MAD - Win32 Release"


"$(INTDIR)\Game.obj" : $(SOURCE) $(DEP_CPP_GAME_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"


"$(INTDIR)\Game.obj" : $(SOURCE) $(DEP_CPP_GAME_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"


"$(INTDIR)\Game.obj" : $(SOURCE) $(DEP_CPP_GAME_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\Mad\Source\Inv.cpp
DEP_CPP_INV_C=\
	"..\archive.h"\
	"..\bar.h"\
	"..\debugmad.h"\
	"..\dlgman.h"\
	"..\fontman.h"\
	"..\inv.h"\
	"..\mad.h"\
	"..\project.h"\
	"..\script.h"\
	

!IF  "$(CFG)" == "MAD - Win32 Release"


"$(INTDIR)\Inv.obj" : $(SOURCE) $(DEP_CPP_INV_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Debug"


"$(INTDIR)\Inv.obj" : $(SOURCE) $(DEP_CPP_INV_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MAD - Win32 Release with Debugging Info"


"$(INTDIR)\Inv.obj" : $(SOURCE) $(DEP_CPP_INV_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
