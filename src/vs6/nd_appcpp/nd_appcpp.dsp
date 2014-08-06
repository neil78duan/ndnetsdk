# Microsoft Developer Studio Project File - Name="nd_appcpp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=nd_appcpp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nd_appcpp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nd_appcpp.mak" CFG="nd_appcpp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nd_appcpp - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "nd_appcpp - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nd_appcpp - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../../include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\lib\nd_appcpp.lib"

!ELSEIF  "$(CFG)" == "nd_appcpp - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "ND_DEBUG" /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\lib\nd_appcpp_dbg.lib"

!ENDIF 

# Begin Target

# Name "nd_appcpp - Win32 Release"
# Name "nd_appcpp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\nd_appcpp\src\nd_cmmgr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\nd_appcpp\src\nd_connector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\nd_appcpp\src\nd_instance.cpp
# End Source File
# Begin Source File

SOURCE=..\..\nd_appcpp\src\nd_listener.cpp
# End Source File
# Begin Source File

SOURCE=..\..\nd_appcpp\src\nd_msgpack.cpp
# End Source File
# Begin Source File

SOURCE=..\..\nd_appcpp\src\nd_object.cpp
# End Source File
# Begin Source File

SOURCE=..\..\nd_appcpp\src\nd_session.cpp
# End Source File
# Begin Source File

SOURCE=..\..\nd_appcpp\src\nd_stlalloc.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\include\nd_appcpp\nd_allocator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_appcpp\nd_cmmgr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_appcpp\nd_connector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_appcpp\nd_instance.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_appcpp\nd_listener.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_appcpp\nd_msgpacket.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_appcpp\nd_object.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\nd_appcpp\nd_session.h
# End Source File
# End Group
# End Target
# End Project
