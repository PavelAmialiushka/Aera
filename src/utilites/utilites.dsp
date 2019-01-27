# Microsoft Developer Studio Project File - Name="utilites" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=utilites - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "utilites.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "utilites.mak" CFG="utilites - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "utilites - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "utilites - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "utilites - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\bin"
# PROP Intermediate_Dir "..\..\bin\release\utilites"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /Ob2 /I "..\..\include" /I "..\..\src" /I "c:\qt\boost_1_33_1" /I "..\..\src\others\wtl" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /Zm600 /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /i "..\..\include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "utilites - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\bin"
# PROP Intermediate_Dir "..\..\bin\debug\utilites"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /I "..\..\include" /I "..\..\src" /I "c:\qt\boost_1_33_1" /I "..\..\src\others\wtl" /D "_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /Zm600 /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /i "..\..\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\utilitesd.lib"

!ENDIF 

# Begin Target

# Name "utilites - Win32 Release"
# Name "utilites - Win32 Debug"
# Begin Group "Source files"

# PROP Default_Filter "*.cpp;*.h"
# Begin Group "debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CrashFileNames.h
# End Source File
# Begin Source File

SOURCE=.\Exception.h
# End Source File
# Begin Source File

SOURCE=.\Exception0.cpp
# End Source File
# Begin Source File

SOURCE=.\ExceptionHandler.cpp

!IF  "$(CFG)" == "utilites - Win32 Release"

!ELSEIF  "$(CFG)" == "utilites - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ExceptionHandler.h
# End Source File
# Begin Source File

SOURCE=.\FontSize.cpp
# End Source File
# Begin Source File

SOURCE=.\FontSize.h
# End Source File
# Begin Source File

SOURCE=.\FunctionStackTrace.h
# End Source File
# Begin Source File

SOURCE=.\GetWinVer.cpp
# End Source File
# Begin Source File

SOURCE=.\GetWinVer.h
# End Source File
# Begin Source File

SOURCE=.\MiniVersion.cpp
# End Source File
# Begin Source File

SOURCE=.\MiniVersion.h
# End Source File
# Begin Source File

SOURCE=.\SourcePos.cpp
# End Source File
# Begin Source File

SOURCE=.\SourcePos.h
# End Source File
# End Group
# Begin Group "log"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\counter.cpp
# End Source File
# Begin Source File

SOURCE=.\counter.h
# End Source File
# Begin Source File

SOURCE=.\log.cpp
# End Source File
# Begin Source File

SOURCE=.\log.h
# End Source File
# End Group
# Begin Group "util"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\application.cpp
# End Source File
# Begin Source File

SOURCE=.\application.h
# End Source File
# Begin Source File

SOURCE=.\cmdline.cpp
# End Source File
# Begin Source File

SOURCE=.\CmdLine.h
# End Source File
# Begin Source File

SOURCE=.\date.h
# End Source File
# Begin Source File

SOURCE=.\document.h
# End Source File
# Begin Source File

SOURCE=.\filesystem.cpp
# End Source File
# Begin Source File

SOURCE=.\filesystem.h
# End Source File
# Begin Source File

SOURCE=.\hash_table.h
# End Source File
# Begin Source File

SOURCE=.\heap.h
# End Source File
# Begin Source File

SOURCE=.\Localizator.cpp
# End Source File
# Begin Source File

SOURCE=.\Localizator.h
# End Source File
# Begin Source File

SOURCE=.\security.cpp
# End Source File
# Begin Source File

SOURCE=.\security.h
# End Source File
# Begin Source File

SOURCE=.\Singleton.cpp

!IF  "$(CFG)" == "utilites - Win32 Release"

!ELSEIF  "$(CFG)" == "utilites - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Singleton.h
# End Source File
# Begin Source File

SOURCE=.\streams.cpp
# End Source File
# Begin Source File

SOURCE=.\streams.h
# End Source File
# Begin Source File

SOURCE=.\strings.h
# End Source File
# Begin Source File

SOURCE=.\tokenizer.h
# End Source File
# Begin Source File

SOURCE=.\TestFile.cpp
# End Source File
# Begin Source File

SOURCE=.\TestFile.h
# End Source File
# Begin Source File

SOURCE=.\unittest.cpp
# End Source File
# Begin Source File

SOURCE=.\unittest.h
# End Source File
# Begin Source File

SOURCE=.\utilites.h
# End Source File
# End Group
# Begin Group "serl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\serl\Archive.h
# End Source File
# Begin Source File

SOURCE=.\element.cpp
# End Source File
# Begin Source File

SOURCE=.\serl\formats.h
# End Source File
# Begin Source File

SOURCE=.\serl\rtti.h
# End Source File
# Begin Source File

SOURCE=.\serl_data.cpp
# End Source File
# Begin Source File

SOURCE=.\serl_fwd.h
# End Source File
# Begin Source File

SOURCE=.\serl_registry.cpp
# End Source File
# Begin Source File

SOURCE=.\serl_test.cpp
# End Source File
# Begin Source File

SOURCE=.\serl_text.cpp
# End Source File
# Begin Source File

SOURCE=.\serl\wrappers.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\CommonIncludes.h
# End Source File
# Begin Source File

SOURCE=.\help.cpp
# End Source File
# Begin Source File

SOURCE=.\help.h
# End Source File
# Begin Source File

SOURCE=..\utilitesIncludes.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Target
# End Project
