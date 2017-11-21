Process for creating a distribution for saf in sierra:

1) Obtain a copy of saf. The assumpition here is that you have access to the
   saf repository.
        % cd ~
        % mkdir saf_tardir;  cd saf_tardir
        % setenv CVS_RSH "ssh"
        % setenv CVSROOT <user-name>@sourceforge.sandia.gov:/cvsroot/saf
        % cvs checkout -r sslib-phase1 saf    # sslib-phase1 is a branch-name
        % mv saf source
        % tar -cvzf saf-sslib.tar.z source
       
2) Go to the installation directory for the product and create a directory
   with a version number.
        % cd /sierra/Release/saf
        % mkdir 2.0.0
        % cd 2.0.0

3) Unzip & untar saf. You should not have to ever modify or run configure
   in the source directory.
        % tar -xvzf ~/saf_tardir/saf-sslib.tar.z

4) Create the files required by sierra's build system
        % cp /sierra/Release/saf/old.version/saf_sn.xml .
	% cp -r source/tools/build_sierra_saf build_sierra

5) Make sure you are in the correct directory prior to building.
        % pwd
        /sierra/Release/saf/2.0.0
        % ls
        build_sierra        saf_sn.xml      source

6) (optional)  Set an enviroment variable to store platform name
        % setenv LOCAL_PLATFORM your-platfrom-name

7) Build.... You have two options
   a) Build using sierra tool
        % build_tpl -s Sierra -o $LOCAL_PLATFORM -o opt -o dp
        % build_tpl -s Sierra -o $LOCAL_PLATFORM -o dbg -o dp
   b) Build using scripts in build_sierra
        ----  edit the scripts and change directory names in SRC_DIR and HDF_DIR
        % vi ./build_sierra/build.$LOCAL_PLATFORM.dbg # change SRC_DIR & HDF_DIR
        % ./build_sierra/build.$LOCAL_PLATFORM.dbg
        % vi ./build_sierra/build.$LOCAL_PLATFORM.opt # change SRC_DIR & HDF_DIR
        % ./build_sierra/build.$LOCAL_PLATFORM.opt
8) Test
        % cd cfg_${LOCAL_PLATFORM}_dbg   or cfg_${LOCAL_PLATFORM}_opt
        % gmake check

9) Update saf_sn.xml with proper include paths for platfrom.
