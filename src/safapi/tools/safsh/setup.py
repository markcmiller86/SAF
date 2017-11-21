from distutils.core import setup, Extension

setup(name="ss", version="1.0",
      ext_modules=[
        Extension(
           "ss",
           ["ss.c"],
           include_dirs= ["/usr/local/src/hdf5-1.7.45/hdf5/include",
                          "/home/gdsjaar/src/saf/include"
           ],
           library_dirs= ["/usr/local/src/hdf5-1.7.45/hdf5/lib",
                          "/home/gdsjaar/src/saf/lib"
           ],
           libraries = ["safapi","ss","hdf5","z","m"]
        )
      ]
)


