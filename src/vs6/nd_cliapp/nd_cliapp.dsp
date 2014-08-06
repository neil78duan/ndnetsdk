# Microsoft Developer Studio Project File - Name="nd_cliapp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=nd_cliapp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nd_cliapp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nd_cliapp.mak" CFG="nd_cliapp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nd_cliapp - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "nd_cliapp - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "nd_cliapp - Win32 Unicode" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nd_cliapp - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /Od /I "../../../include" /I "../../../include/nd_cliapp" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\lib\nd_cliapp.lib"

!ELSEIF  "$(CFG)" == "nd_cliapp - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../../../include/nd_cliapp" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "ND_DEBUG" /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\lib\nd_cliapp_dbg.lib"

!ELSEIF  "$(CFG)" == "nd_cliapp - Win32 Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Unicode"
# PROP BASE Intermediate_Dir "Unicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Unicode"
# PROP Intermediate_Dir "Unicode"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../../include" /I "../../../include/nd_cliapp" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "ND_UNICODE" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\lib\nd_cliapp_uni.lib"

!ENDIF 

# Begin Target

# Name "nd_cliapp - Win32 Release"
# Name "nd_cliapp - Win32 Debug"
# Name "nd_cliapp - Win32 Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\nd_cliapp\src\connect_msg.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_cliapp\src\login.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_cliapp\src\nd_cliapp.c
# End Source File
# Begin Source File

SOURCE=..\..\nd_appcpp\src\nd_connector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\nd_appcpp\src\nd_msgpack.cpp
# End Source File
# Begin Source File

SOURCE=..\..\nd_appcpp\src\nd_object.cpp
# End Source File
# Begin Source File

SOURCE=..\..\nd_cliapp\src\readconfig.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\include\nd_cliapp\nd_cliapp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_appcpp\nd_connector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_appcpp\nd_msgpacket.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_appcpp\nd_object.h
# End Source File
# End Group
# End Target
# End Project
