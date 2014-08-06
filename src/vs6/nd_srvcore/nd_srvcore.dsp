# Microsoft Developer Studio Project File - Name="nd_srvcore" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=nd_srvcore - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nd_srvcore.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nd_srvcore.mak" CFG="nd_srvcore - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nd_srvcore - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "nd_srvcore - Win32 Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "nd_srvcore - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nd_srvcore - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ND_SRVCORE_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /ZI /Od /I "../../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ND_SRVCORE_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../../bin/nd_srvcore.dll" /implib:"../../../lib/nd_srvcore.lib" /libpath:"../../../lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "nd_srvcore - Win32 Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Unicode"
# PROP Intermediate_Dir "Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ND_SRVCORE_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ND_SRVCORE_EXPORTS" /D "ND_UNICODE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../../../bin/nd_srvcore_uni.dll" /implib:"../../../lib/nd_srvcore_uni.lib" /libpath:"../../../lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "nd_srvcore - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ND_SRVCORE_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ND_SRVCORE_EXPORTS" /D "ND_DEBUG" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"..\..\..\bin\nd_srvcore_dbg.dll" /implib:"..\..\..\lib\nd_srvcore_dbg.lib" /pdbtype:sept /libpath:"../../../lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "nd_srvcore - Win32 Release"
# Name "nd_srvcore - Win32 Unicode"
# Name "nd_srvcore - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\nd_srvcore\src\client_map.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_common\dllmain.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_srvcore\src\linux_listen.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_srvcore\src\nd_session.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_srvcore\src\nd_thpoolio.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_srvcore\src\srv_listen.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_srvcore\src\srv_udt.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_srvcore\src\srvcore.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_srvcore\src\thread_srv.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_srvcore\src\user_srv.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\include\nd_srvcore\client_map.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_srvcore\nd_listensrv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_srvcore\nd_session.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_srvcore\nd_srvlib.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_srvcore\nd_threadsrv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_srvcore\nd_udtsrv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_srvcore\nd_userth.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "win_iocp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\nd_srvcore\win_iocp\nd_iocp.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_srvcore\win_iocp\nd_iocp.h
# End Source File
# Begin Source File

SOURCE=..\..\nd_srvcore\win_iocp\thread_iocp.c
# End Source File
# End Group
# End Target
# End Project
