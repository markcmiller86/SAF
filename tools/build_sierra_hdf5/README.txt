Process for creating a distribution for hdf5 in sierra:

1) Obtain a copy of hdf5
        % cd ~
	% ftp hdf.ncsa.uiuc.edu
        username: ftp
        passwd: eailles@sandia.gov   # email address
        ftp> cd /pub/outgoing/hdf5/snapshots
        ftp> dir
        ftp> get hdf5-1.8.0.tar.bz2
        ftp> exit

2) Go to the installation directory for the product and create a directory
   with a version number.
        % cd /sierra/Release/hdf5
        % mkdir 1.8.0
        % cd 1.8.0

3) Unzip & untar hdf5 and rename the untared directory to "source". You should
   not have to ever modify or run configure in the source directory.
        % bunzip2 < ~/hdf5-1.8.0.tar.bz2 |tar xf -
        % mv hdf5-1.8.0 source

4) Create the files required by sierra's build system
        % cp /sierra/Release/hdf5/old.version/hdf5_sn.xml .
	% cp -r ...../saf/tools/buid_sierra_hdf5 build_sierra

5) Make sure you are in the correct directory prior to building.
        % pwd
        /sierra/Release/hdf5/1.8.0
        % ls
        buid_sierra        hdf5_sn.xml      source

6) (optional)  Set an enviroment variable to store platform name
        % setenv LOCAL_PLATFORM your-platfrom-name

7) Build.... You have two options
   a) Build using sierra tool
        % build_tpl -s Sierra -o $LOCAL_PLATFORM -o opt -o dp
        % build_tpl -s Sierra -o $LOCAL_PLATFORM -o dbg -o dp
   b) Build using scripts in build_sierra
        % setenv SRC_DIR /sierra/Release/hdf5/1.8.0 
        % ./build_sierra/build.$LOCAL_PLATFORM.dbg

8) Test 
        % cd cfg_${LOCAL_PLATFORM}_dbg   or cfg_${LOCAL_PLATFORM}_opt
        % gmake check

9) Update hdf5_sn.xml with proper include paths for platform
