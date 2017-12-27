# Microsoft Developer Studio Project File - Name="nd_common" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=nd_common - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nd_common.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nd_common.mak" CFG="nd_common - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nd_common - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "nd_common - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "nd_common - Win32 Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nd_common - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ND_COMMON_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /ZI /Od /I "../../../include/" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ND_COMMON_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /mktyplib203 /win32
# ADD MTL /nologo /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../../bin/nd_common.dll" /implib:"../../../lib/nd_common.lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "nd_common - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ND_COMMON_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include/" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ND_COMMON_EXPORTS" /D "ND_DEBUG" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"..\..\..\bin\nd_common_dbg.dll" /implib:"..\..\..\lib\nd_common_dbg.lib" /pdbtype:sept /libpath:"../../../lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "nd_common - Win32 Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Unicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Unicode"
# PROP Intermediate_Dir "Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /Gm /GX /ZI /O2 /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ND_COMMON_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /GX /Zi /Od /I "../../../include/" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ND_COMMON_EXPORTS" /D "ND_UNICODE" /FR /YX /FD /c
# ADD BASE MTL /nologo /mktyplib203 /win32
# ADD MTL /nologo /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /debug /machine:I386 /out:"../../../bin/nd_common_uni.dll" /implib:"../../../lib/nd_common.lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "nd_common - Win32 Release"
# Name "nd_common - Win32 Debug"
# Name "nd_common - Win32 Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\nd_common\src\bintree.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_common\src\common.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_common\dllmain.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_common\src\nd_handle.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_common\src\nd_mempool.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_common\src\nd_static_alloc.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_common\src\nd_str.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_common\src\nd_timer.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_common\src\nd_trace.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_common\src\nd_unix.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_common\src\nd_win.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_common\src\nd_xml.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_common\src\node_mgr.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_common\src\recbuf.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_common\src\source_log.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\include\nd_common\list.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_common\nd_alloc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_common\nd_atomic.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_common\nd_bintree.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_common\nd_comcfg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_common\nd_comdef.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_common\nd_common.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_common\nd_dbg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_common\nd_handle.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_common\nd_mempool.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_common\nd_node_mgr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_common\nd_os.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_common\nd_recbuf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_common\nd_static_alloc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_common\nd_str.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_common\nd_timer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_common\nd_unix.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_common\nd_win.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_common\nd_xml.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_common\ndchar.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
