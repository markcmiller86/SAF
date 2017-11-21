# Microsoft Developer Studio Project File - Name="safapi" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=safapi - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "safapi.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "safapi.mak" CFG="safapi - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "safapi - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "safapi - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "safapi - Win32 Release"

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
# ADD CPP /nologo /MD /w /W0 /GX /O2 /I "../../vbt" /I "../../" /I "$(HDF5_INSTALL_PATH)\include" /I "../../../../src/safapi/lib" /I "../../../../src/sslib/lib" /I "$(ZLIB_INCLUDE_DIR)" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "safapi - Win32 Debug"

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
# ADD CPP /nologo /MDd /w /W0 /Gm /GX /ZI /Od /I "../../" /I "$(HDF5_INSTALL_PATH)\include" /I "../../../../src/safapi/lib" /I "../../../../src/sslib/lib" /I "$(ZLIB_INCLUDE_DIR)" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "safapi - Win32 Release"
# Name "safapi - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\algebraic.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\altindx.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\base.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\basis.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\cat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\coll.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\db.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\dbprops.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\error.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\evaluation.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\field.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\file.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\fileprops.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\ftempl.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\genreg.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\handles.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\hash.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\info.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\init.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\libprops.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\quant.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\rel.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\relrep.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\role.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\set.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\state.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\stempl.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\suite.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\unit.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\utils.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\wrapper.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\algebraic.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\basis.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\db.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\dbprops.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\evaluation.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\fileprops.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\genreg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\handles.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\hash.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\info.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\init.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\libprops.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\quant.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\relrep.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\role.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\saf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\SAFalgebraic.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\SAFbasis.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\SAFdb.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\SAFdbprops.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\SAFevaluation.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\SAFfileprops.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\SAFhandles.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\SAFinfo.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\SAFinit.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\SAFlibprops.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\safP.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\SAFquant.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\SAFrelrep.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\SAFrole.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\SAFunit.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\unit.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\safapi\lib\wrapper.h
# End Source File
# End Group
# End Target
# End Project
