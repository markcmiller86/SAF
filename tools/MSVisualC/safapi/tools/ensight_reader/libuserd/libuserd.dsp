# Microsoft Developer Studio Project File - Name="libuserd" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=libuserd - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libuserd.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libuserd.mak" CFG="libuserd - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libuserd - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "libuserd - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libuserd - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBUSERD_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /w /W0 /GX /O2 /I "../../../../" /I "../../../../../../src/safapi/tools/ensight_reader" /I "../../../../../../src/safapi/lib" /I "../../../../../../src/sslib/lib" /I "$(HDF5_INSTALL_PATH)\include" /I "$(ZLIB_INCLUDE_DIR)" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBUSERD_EXPORTS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 safapi.lib sslib.lib hdf5.lib $(ZLIB_LIBRARY) szlib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /nodefaultlib:"libc.lib" /out:"Release/libuserd-saf.dll" /libpath:"..\..\..\..\sslib\sslib_library\Release" /libpath:"..\..\..\lib\Release" /libpath:"$(HDF5_INSTALL_PATH)\lib" /libpath:"$(SZIP_INSTALL_DIR)\lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Release\*.dll "$(SAF_INSTALL_PATH)\lib"	copy Release\*.lib "$(SAF_INSTALL_PATH)\lib"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "libuserd - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBUSERD_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /w /W0 /Gm /GX /ZI /Od /I "../../../../" /I "../../../../../../src/safapi/tools/ensight_reader" /I "../../../../../../src/safapi/lib" /I "../../../../../../src/sslib/lib" /I "$(HDF5_INSTALL_PATH)\include" /I "$(ZLIB_INCLUDE_DIR)" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBUSERD_EXPORTS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 safapi.lib sslib.lib hdf5d.lib $(ZLIB_LIBRARY) szlib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /nodefaultlib:"libc.lib" /out:"Debug/libuserd-saf.dll" /pdbtype:sept /libpath:"..\..\..\..\sslib\sslib_library\Debug" /libpath:"..\..\..\lib\Debug" /libpath:"$(HDF5_INSTALL_PATH)\lib" /libpath:"$(SZIP_INSTALL_DIR)\lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Debug\*.dll "$(SAF_INSTALL_PATH)\lib"	copy Debug\*.lib "$(SAF_INSTALL_PATH)\lib"
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "libuserd - Win32 Release"
# Name "libuserd - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\..\..\src\safapi\tools\ensight_reader\libuserd_saf.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\src\safapi\tools\ensight_reader\str_mesh_reader.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\src\safapi\tools\ensight_reader\subset_transforms.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\src\safapi\tools\ensight_reader\unstr_mesh_reader.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\src\safapi\tools\ensight_reader\variable_names.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\..\..\src\safapi\tools\ensight_reader\global_extern.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\src\safapi\tools\ensight_reader\str_mesh_reader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\src\safapi\tools\ensight_reader\unstr_mesh_reader.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
