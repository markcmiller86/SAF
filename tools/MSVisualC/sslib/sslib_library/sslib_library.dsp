# Microsoft Developer Studio Project File - Name="sslib_library" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=sslib_library - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sslib_library.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sslib_library.mak" CFG="sslib_library - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sslib_library - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sslib_library - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sslib_library - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\..\src\sslib\lib" /I "..\.." /I "$(HDF5_INSTALL_PATH)\include" /I "$(ZLIB_INCLUDE_DIR)" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Release\sslib.lib"

!ELSEIF  "$(CFG)" == "sslib_library - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\..\src\sslib\lib" /I "..\.." /I "$(HDF5_INSTALL_PATH)\include" /I "$(ZLIB_INCLUDE_DIR)" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\sslib.lib"

!ENDIF 

# Begin Target

# Name "sslib_library - Win32 Release"
# Name "sslib_library - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssaio.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssarray.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssattr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssattrtab.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssblob.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssblobtab.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\sserr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssfile.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssfiletab.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssgfile.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\sshdf5.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\sslib.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssmpi.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssobj.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\sspers.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssperstab.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssprop.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssscope.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssscopetab.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssstring.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\sstable.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssval.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssaio.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssarray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssattr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssattrtab.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssblob.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssblobtab.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssenv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\sserr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssfile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssfiletab.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssgfile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\sshdf5.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\sslib.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssmpi.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssobj.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\sspers.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssperstab.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssprop.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssscope.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssscopetab.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssstring.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\sstable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\sslib\lib\ssval.h
# End Source File
# End Group
# End Target
# End Project
