# Microsoft Developer Studio Project File - Name="safapi_test_safdiff" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=safapi_test_safdiff - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "safapi_test_safdiff.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "safapi_test_safdiff.mak" CFG="safapi_test_safdiff - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "safapi_test_safdiff - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "safapi_test_safdiff - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "safapi_test_safdiff - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../../" /I "../../../../../src/safapi/test" /I "../../../../../src/safapi/tools/ensight_reader" /I "../../../../../src/safapi/lib" /I "../../../../../src/sslib/lib" /I "$(HDF5_INSTALL_PATH)\include" /I "$(ZLIB_INCLUDE_DIR)" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"Release/dummy.exe"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Testing...
PostBuild_Cmds=cd Release	..\..\safapi_test_larry1w\Release\larry1w -quiet do_writes -perturb 0 -file larry1w_perturb_0.saf	..\..\safapi_test_larry1w\Release\larry1w -quiet do_writes -perturb 1 -file larry1w_perturb_1.saf	..\..\safapi_test_larry1w\Release\larry1w -quiet do_writes -perturb 2 -file larry1w_perturb_2.saf	..\..\safapi_test_larry1w\Release\larry1w -quiet do_writes -perturb 3 -file larry1w_perturb_3.saf	..\..\safapi_test_larry1w\Release\larry1w -quiet do_writes -perturb 4 -file larry1w_perturb_4.saf	..\..\safapi_test_larry1w\Release\larry1w -quiet do_writes -perturb 5 -file larry1w_perturb_5.saf	..\..\safapi_test_larry1w\Release\larry1w -quiet do_writes -perturb 6 -file larry1w_perturb_6.saf	echo ON > blank.txt	copy larry1w_perturb_0.saf larry1w_perturb_0copy.saf	..\..\..\tools\safdiff\Release\safdiff -f blank.txt -v 0 larry1w_perturb_0.saf larry1w_perturb_0copy.saf	..\..\..\tools\safdiff\Release\safdiff -f blank.txt -v 0 -expectFileDifferences larry1w_perturb_0.saf larry1w_perturb_1.saf	..\..\..\tools\safdiff\Release\safdiff -f blank.txt -v 0 -expectFileDifferences larry1w_perturb_0.saf larry1w_perturb_2.saf	..\..\..\tools\safdiff\Release\safdiff -f blank.txt -v 0\
 -expectFileDifferences larry1w_perturb_0.saf     larry1w_perturb_3.saf	..\..\..\tools\safdiff\Release\safdiff -f blank.txt -v 0 -expectFileDifferences larry1w_perturb_4.saf larry1w_perturb_0.saf	..\..\..\tools\safdiff\Release\safdiff -f blank.txt -v 0 -expectFileDifferences larry1w_perturb_5.saf larry1w_perturb_0.saf	..\..\..\tools\safdiff\Release\safdiff -f blank.txt -v 0 -expectFileDifferences larry1w_perturb_6.saf larry1w_perturb_0.saf
# End Special Build Tool

!ELSEIF  "$(CFG)" == "safapi_test_safdiff - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../" /I "../../../../../src/safapi/test" /I "../../../../../src/safapi/tools/ensight_reader" /I "../../../../../src/safapi/lib" /I "../../../../../src/sslib/lib" /I "$(HDF5_INSTALL_PATH)\include" /I "$(ZLIB_INCLUDE_DIR)" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/dummy.exe" /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Testing...
PostBuild_Cmds=cd Debug	..\..\safapi_test_larry1w\Debug\larry1w -quiet do_writes -perturb 0 -file larry1w_perturb_0.saf	..\..\safapi_test_larry1w\Debug\larry1w -quiet do_writes -perturb 1 -file larry1w_perturb_1.saf	..\..\safapi_test_larry1w\Debug\larry1w -quiet do_writes -perturb 2 -file larry1w_perturb_2.saf	..\..\safapi_test_larry1w\Debug\larry1w -quiet do_writes -perturb 3 -file larry1w_perturb_3.saf	..\..\safapi_test_larry1w\Debug\larry1w -quiet do_writes -perturb 4 -file larry1w_perturb_4.saf	..\..\safapi_test_larry1w\Debug\larry1w -quiet do_writes -perturb 5 -file larry1w_perturb_5.saf	..\..\safapi_test_larry1w\Debug\larry1w -quiet do_writes -perturb 6 -file larry1w_perturb_6.saf	echo ON > blank.txt	copy larry1w_perturb_0.saf larry1w_perturb_0copy.saf	..\..\..\tools\safdiff\Debug\safdiff -f blank.txt -v 0 larry1w_perturb_0.saf larry1w_perturb_0copy.saf	..\..\..\tools\safdiff\Debug\safdiff -f blank.txt -v 0 -expectFileDifferences larry1w_perturb_0.saf larry1w_perturb_1.saf	..\..\..\tools\safdiff\Debug\safdiff -f blank.txt -v 0 -expectFileDifferences larry1w_perturb_0.saf larry1w_perturb_2.saf	..\..\..\tools\safdiff\Debug\safdiff -f blank.txt -v 0 -expectFileDifferences\
 larry1w_perturb_0.saf larry1w_perturb_3.saf    	..\..\..\tools\safdiff\Debug\safdiff -f blank.txt -v 0 -expectFileDifferences larry1w_perturb_4.saf larry1w_perturb_0.saf	..\..\..\tools\safdiff\Debug\safdiff -f blank.txt -v 0 -expectFileDifferences larry1w_perturb_5.saf larry1w_perturb_0.saf	..\..\..\tools\safdiff\Debug\safdiff -f blank.txt -v 0 -expectFileDifferences larry1w_perturb_6.saf larry1w_perturb_0.saf
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "safapi_test_safdiff - Win32 Release"
# Name "safapi_test_safdiff - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\dummy.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
