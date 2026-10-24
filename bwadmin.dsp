# Microsoft Developer Studio Project File - Name="bwadmin" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=bwadmin - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bwadmin.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bwadmin.mak" CFG="bwadmin - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bwadmin - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "bwadmin - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "bwadmin - Win32 Pentium Pro" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bwadmin - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Work\BW-Admin\Output\Release"
# PROP Intermediate_Dir "..\Work\BW-Admin\Intermediate\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BWADMIN_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G5 /MD /W3 /O2 /I ".\include" /I "..\crypt-string\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib /nologo /version:1.72 /dll /pdb:none /map /machine:I386 /out:"..\Work\BW-Admin\Output/Release/gamex86.dll"
# SUBTRACT LINK32 /debug /nodefaultlib

!ELSEIF  "$(CFG)" == "bwadmin - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Work\BW-Admin\Output\Debug"
# PROP Intermediate_Dir "..\Work\BW-Admin\Intermediate\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BWADMIN_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /ZI /Od /I ".\include" /I "..\crypt-string\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /version:1.72 /dll /map /debug /machine:I386 /out:"..\Work\BW-Admin\Output/Debug/gamex86.dll" /pdbtype:sept
# SUBTRACT LINK32 /incremental:no

!ELSEIF  "$(CFG)" == "bwadmin - Win32 Pentium Pro"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Pentium Pro"
# PROP BASE Intermediate_Dir "Pentium Pro"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Work\BW-Admin\Output\PPro"
# PROP Intermediate_Dir "..\Work\BW-Admin\Intermediate\PPro"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /O1 /Ob2 /I ".\include" /I "..\crypt-string\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib /nologo /dll /map /machine:I386 /out:"Output/Release/gamex86.dll"
# SUBTRACT BASE LINK32 /debug /nodefaultlib
# ADD LINK32 kernel32.lib user32.lib /nologo /version:1.72 /dll /pdb:none /map /machine:I386 /out:"..\Work\BW-Admin\Output\PPro\gamex86.dll"
# SUBTRACT LINK32 /debug /nodefaultlib /force

!ENDIF 

# Begin Target

# Name "bwadmin - Win32 Release"
# Name "bwadmin - Win32 Debug"
# Name "bwadmin - Win32 Pentium Pro"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=".\src\admin.c"
# End Source File
# Begin Source File

SOURCE=".\src\auth.c"
# End Source File
# Begin Source File

SOURCE=".\src\bans.c"
# End Source File
# Begin Source File

SOURCE=".\src\input.c"
# End Source File
# Begin Source File

SOURCE=".\src\logging.c"
# End Source File
# Begin Source File

SOURCE=".\src\version.c"
# End Source File
# Begin Source File

SOURCE=".\src\wget.c"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\admin.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\game.def
# End Source File
# End Group
# Begin Group "Misc"

# PROP Default_Filter "*.txt;*.html"
# Begin Source File

SOURCE=".\bw-admin.cfg"
# End Source File
# Begin Source File

SOURCE=".\bw-admin.html"
# End Source File
# Begin Source File

SOURCE=.\Changelog
# End Source File
# Begin Source File

SOURCE=.\License
# End Source File
# Begin Source File

SOURCE=.\Makefile
# End Source File
# Begin Source File

SOURCE=.\Todo
# End Source File
# End Group
# Begin Group "Crypt Strings"

# PROP Default_Filter "*.c; *.h"
# Begin Source File

SOURCE="..\crypt-string\src\codec.c"
# End Source File
# Begin Source File

SOURCE="..\crypt-string\include\codec.h"
# End Source File
# Begin Source File

SOURCE=.\src\strings.c
# End Source File
# Begin Source File

SOURCE=.\include\strings.h
# End Source File
# Begin Source File

SOURCE=.\src\strings.in.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Game Specific"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\src\id_stuff.c"
# End Source File
# Begin Source File

SOURCE=.\include\kp_game.h
# End Source File
# Begin Source File

SOURCE=.\include\kp_local.h
# End Source File
# Begin Source File

SOURCE=.\include\kp_shared.h
# End Source File
# Begin Source File

SOURCE=.\include\q2_game.h
# End Source File
# Begin Source File

SOURCE=.\include\q2_local.h
# End Source File
# Begin Source File

SOURCE=.\include\q2_shared.h
# End Source File
# Begin Source File

SOURCE=.\include\shared.h
# End Source File
# End Group
# End Target
# End Project
