# Microsoft Developer Studio Project File - Name="vtk_saf" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=vtk_saf - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vtk_saf.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vtk_saf.mak" CFG="vtk_saf - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vtk_saf - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "vtk_saf - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vtk_saf - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /w /W0 /GX /O2 /I "../../../../../../src/safapi/tools/vtk_reader" /I "../../../../../../src/safapi/tools/ensight_reader" /I "$(VTK_PATH)\include\vtk" /I "$(SAF_INSTALL_PATH)\include" /I "$(SAF_INSTALL_PATH)\include\private" /I "$(HDF5_INSTALL_PATH)\include" /I "$(ZLIB_INSTALL_PATH)\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_HDF5USEDLL_" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 vtkCommon.lib vtkGraphics.lib vtkIO.lib vtkFiltering.lib vtkRendering.lib vtkHybrid.lib safapi.lib vbt.lib dsl.lib hdf5.lib zlib.lib opengl32.lib glaux.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:none /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\paraview" /libpath:"$(VTK_PATH)\lib\vtk" /libpath:"$(SAF_INSTALL_PATH)\lib" /libpath:"$(HDF5_INSTALL_PATH)\lib" /libpath:""$(VTK_PATH)\lib\vtk"" /libpath:""$(HDF5_INSTALL_PATH)\lib"" /libpath:""$(ZLIB_INSTALL_PATH)\lib"" /libpath:""$(VTK_PATH)\lib\vtk"" /libpath:""$(SAF_INSTALL_PATH)\lib"" /libpath:""$(HDF5_INSTALL_PATH)\lib"" /libpath:""$(ZLIB_INSTALL_PATH)\lib""
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "vtk_saf - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /w /W0 /Gm /GX /ZI /Od /I "../../../../../../src/safapi/tools/vtk_reader" /I "../../../../../../src/safapi/tools/ensight_reader" /I "$(VTK_PATH)\include\vtk" /I "$(SAF_INSTALL_PATH)\include" /I "$(SAF_INSTALL_PATH)\include\private" /I "$(HDF5_INSTALL_PATH)\include" /I "$(ZLIB_INSTALL_PATH)\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 vtkCommon.lib vtkGraphics.lib vtkIO.lib vtkFiltering.lib vtkRendering.lib vtkHybrid.lib safapi.lib vbt.lib dsl.lib hdf5.lib zlib.lib opengl32.lib glaux.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:none /debug /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\paraview" /libpath:"$(VTK_PATH)\lib\vtk" /libpath:"$(SAF_INSTALL_PATH)\lib" /libpath:"$(HDF5_INSTALL_PATH)\lib" /libpath:""$(VTK_PATH)\lib\vtk"" /libpath:""$(HDF5_INSTALL_PATH)\lib"" /libpath:""$(ZLIB_INSTALL_PATH)\lib"" /libpath:""$(VTK_PATH)\lib\vtk"" /libpath:""$(SAF_INSTALL_PATH)\lib"" /libpath:""$(HDF5_INSTALL_PATH)\lib"" /libpath:""$(ZLIB_INSTALL_PATH)\lib""
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "vtk_saf - Win32 Release"
# Name "vtk_saf - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\..\..\src\safapi\tools\vtk_reader\main.cxx
# End Source File
# Begin Source File

SOURCE=..\paraview\str_mesh_reader.cxx
# End Source File
# Begin Source File

SOURCE=..\paraview\unstr_mesh_reader.cxx
# End Source File
# Begin Source File

SOURCE=..\paraview\variable_names.cxx
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\src\safapi\tools\vtk_reader\vtkSAFRectilinearReader.cxx
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\src\safapi\tools\vtk_reader\vtkSAFStructuredReader.cxx
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\src\safapi\tools\vtk_reader\vtkSAFUnstructuredReader.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\paraview\str_mesh_reader.h
# End Source File
# Begin Source File

SOURCE=..\paraview\unstr_mesh_reader.h
# End Source File
# Begin Source File

SOURCE=..\paraview\variable_names.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\src\safapi\tools\vtk_reader\vtkSAFRectilinearReader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\src\safapi\tools\vtk_reader\vtkSAFStructuredReader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\..\src\safapi\tools\vtk_reader\vtkSAFUnstructuredReader.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
