/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Introduction
 * Description:
 *      The intent of this document is to describe the following information about Phase I of the conversion of VBT and DSL to
 *      SSlib:
 *      
 *      * What must be done to SAF client source code
 *      * How to find and fix problems
 *
 *      The main things to know about are:
 *
 *      * Object typedefs like SAF_Field are no longer pointers under the covers. They're structs, which makes them a lot
 *        easier to declare and initialize.
 *
 *      * All functions and macros take pointers as arguments (like SAF_Field* instead of just SAF_Field) but they're
 *        never passed with the !const qualifier since SSlib sometimes caches some info in the struct.
 *
 *      * HDF5 datatypes (hid_t) are used instead of DSL datatypes (DSL_Type).  The most important thing to remember here
 *        is that HDF5 datatypes must be compared with H5Tequal() whereas DSL types could be compared with `==' and `!='.
 *        Unfortunately the C compiler will not generate warnings here since both types are based on integers.
 *
 *      * SAF_EACH mode is still a collective operation over the file communicator even though we plan to relax this
 *        in Phase II (it's already relaxed for most functions, but I don't make any guarantees that it really works yet).
 *
 *      * Supplemental files went away, and every file is a database. Certain functions that used to take a SAF_File argument to
 *        to indicate where to store raw data now take an additional SAF_Db argument, which when omitted defaults to the file
 *        that contains the field or relation being written.
 *
 *      * When debugging, printing an object link (like a SAF_Field) won't tell you much, but there are at least a handful
 *        of debugging features to make things easier.
 *
 *      * Phase I did not address performance, nor did it add many new features.  The general feeling I get so far is that
 *        some things are faster and some are slower.
 *-------------------------------------------------------------------------------------------------------------------------------
 */


/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     API Argument Changes
 * Description:
 *      This is a list of functions where arguments were added or removed, an argument type was changed significantly, or the
 *      function's return value was changed.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     Parallel mode arguments
 * Chapter:     API Argument Changes
 * Description:
 *      Some functions that previously assumed a SAF_ALL mode now take an argument to specify the mode. It should be safe to
 *      pass SAF_ALL as the first argument to these functions.
 *
 *        saf_declare_algebraic()
 *        saf_declare_basis()
 *        saf_declare_category()
 *        saf_declare_evaluation()
 *        saf_declare_quantity()
 *        saf_declare_relrep()
 *        saf_declare_role()
 *        saf_declare_unit()
 *        saf_describe_algebraic()
 *        saf_describe_basis()
 *        saf_describe_category()
 *        saf_describe_evaluation()
 *        saf_describe_quantity()
 *        saf_describe_relrep()
 *        saf_describe_role()
 *        saf_describe_unit()
 *        saf_divide_quantity()
 *        saf_find_categories()
 *        saf_find_coords()
 *        saf_get_cat_att()
 *        saf_log_unit()
 *        saf_multiply_quantity()
 *        saf_multiply_unit()
 *        saf_offset_unit()
 *        saf_put_cat_att()
 *        saf_quantify_unit()
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     Database arguments
 * Chapter:     API Argument Changes
 * Description:
 *      Some declare functions lacked an argument to specify which database should hold the new object, or the database
 *      argument was in a different position than other declare functions in the API.
 *
 *        saf_declare_algebraic()
 *        saf_declare_alternate_indexspec()
 *        saf_declare_basis()
 *        saf_declare_evaluation()
 *        saf_declare_field()
 *        saf_declare_field_tmpl()    // the database argument was moved to the second position
 *        saf_declare_quantity()
 *        saf_declare_relrep()
 *        saf_declare_role()
 *        saf_declare_state_group()   // the database arg was moved to the second position
 *        saf_declare_state_tmpl()    // the database arg was moved to the second position
 *        saf_declare_subset_relation()
 *        saf_declare_topo_relation()
 *        saf_declare_unit()
 *        saf_find_alternate_indexspecs()
 *        saf_find_categories()
 *        saf_find_fields()
 *        saf_find_subset_relations()
 *        saf_find_topo_relations()
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     Supplemental file arguments
 * Chapter:     API Argument Changes
 * Description:
 *      Some functions took a SAF_File argument to say which supplemental file should be used for writing raw data, or to
 *      return a file pointer where raw data had previously been written. The new API removes these arguments from the query
 *      functions (the database is evident from the raw data blob) or changes them to SAF_Db pointers. The SAF_MASTER() macro
 *      is no longer necessary.
 *
 *        saf_describe_field()
 *        saf_describe_subset_relation()
 *        saf_describe_topo_relation()
 *        saf_use_written_subset_relation()
 *        saf_write_alternate_indexspec()
 *        saf_write_field()
 *        saf_write_subset_relation()
 *        saf_write_topo_relation()
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     Return value allocation
 * Chapter:     API Argument Changes
 * Description:
 *      The SSlib API was designed to work well for object handles allocated on the stack or the heap. From SSlib's point of
 *      view this means it should allow the client to allocate memory for return values or SSlib should allocate memory. The
 *      way this is indicated is that many SSlib functions return a pointer (NULL on failure) to memory that is supplied by
 *      the client (usually the last argument to the function) or to memory allocated by SSlib (in the case when the client
 *      passes a NULL pointer).  The following functions were modified to take this extra optional argument, and in most SAF
 *      client code you will allocate space for object links on the stack and thus pass a pointer to those links in the last
 *      argument.
 *
 *        saf_find_one_algebraic()
 *        saf_find_one_basis()
 *        saf_find_one_evaluation()
 *        saf_find_one_quantity()
 *        saf_find_one_relrep()
 *        saf_find_one_role()
 *        saf_find_one_unit()
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     Field and relation targeting arguments
 * Chapter:     API Argument Changes
 * Description:
 *      Field and relation targeting information for saf_read_field(), saf_read_subset_relation() and saf_read_topo_relation()
 *      was once stored in the handle for the object being read. This meant that it was dangerous for any function to read one
 *      of these objects without first duplicating the object handle to zero out any target information that might have been
 *      present. SSlib no longer suffers from this problem in that it keeps target information separated from the handles to
 *      which that info might apply. However, this means that you might need to pass around this extra target information and
 *      the raw data read functions mentioned above all take an extra argument.
 *
 *        saf_get_count_and_type_for_field()
 *        saf_get_count_and_type_for_subset_relation()
 *        saf_get_count_and_type_for_topo_relation()
 *        saf_read_field()
 *        saf_read_subset_relation()
 *        saf_read_topo_relation()
 *        saf_target_field()
 *        saf_target_subset_relation()
 *        saf_target_topo_relation()
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     API Removed Entities
 * Description:
 *      Lots of private functions were removed from SAF with a few replaced by new functions.  Also, things having to do
 *      with supplemental files and the object registry special cases were removed.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     Supplemental files
 * Chapter:     API Removed Entities
 * Description:
 *      The new SAF doesn't draw any distinctions between what were referred to as a /database/ and a /supplemental/file/
 *      in old SAF. An SSlib file contains zero or more /scopes/ all of which are referened by a single /top/level/scope/
 *      normally called "SAF" and the SAF client can choose which tasks open (or create) each of those scopes. A /database/
 *      is a collection of interrelated files where the relationships are established by the SAF client, and SSlib was
 *      designed to behave well even when only part of a database is available.
 *
 *      Under the covers, a SAF_Db is really an ss_file_t, which is a datatype derived from ss_pers_t and which is therefore
 *      binary compatible with any other type of persistent object. That is, a variable of type SAF_Db is actually a link to
 *      an object in the file to which the variable is describing.
 *      
 *      SSlib doesn't make any distinction between a database and a supplemental file--every file is a database. Therefore
 *      the following functions have been removed from the API.
 *
 *        SAF_File()                    // The datatype has also been removed.
 *        SAF_MASTER()                  // Use the bare SAF_Db pointer.
 *        saf_createProps_file()        // Use database properties instead.
 *        saf_declare_file()
 *        saf_describe_file()
 *        saf_find_files()
 *        saf_flush_file()
 *        saf_get_file_att()
 *        saf_new_file_handle()
 *        saf_put_file_att()
 *        saf_renew_file_handle()
 *        saf_setProps_MaxOpenFiles()
 *        saf_setProps_SFileDir()
 *        saf_suggest_close_file()
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     Miscellaneous removals
 * Chapter:     API Removed Entities
 * Description:
 *      These functions were also removed from the API:
 *
 *        saf_getInfo_MemoryResident()  // File access info is no longer stored in the file
 *        saf_getInfo_PromiseMode()     // Promise mode no longer exists
 *        saf_getInfo_apiversion()      // Use saf_getInfo_libversion() instead
 *        saf_getInfo_isDSLfile()       // No longer useful
 *        saf_getInfo_parallel()        // File access info no longer stored in the file
 *        saf_getInfo_sfiledir()        // No supplemental fils anymore
 *        saf_grab_dsl()
 *        saf_ungrab_dsl()
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     Private functions
 * Chapter:     API Removed Entities
 * Description:
 *      The following private functions have been removed from the library:
 *
 *        _saf_barrier_by_hndl()
 *        _saf_c_order()
 *        _saf_equiv_handles()
 *        _saf_fortran_order()
 *        _saf_handle()
 *        _saf_master_file()
 *        _saf_master_file_by_hndl()
 *        _saf_null_field()
 *        _saf_null_field_by_hndl()
 *        _saf_null_ftmpl()
 *        _saf_null_ftmpl_by_hndl()
 *        _saf_null_rel()
 *        _saf_null_rel_by_hndl()
 *        _saf_null_set()
 *        _saf_null_set_by_hndl()
 *        _saf_null_stategrp()
 *        _saf_null_stategrp_by_hndl()
 *        _saf_null_stmpl()
 *        _saf_null_stmpl_by_hndl()
 *        _saf_null_suite()
 *        _saf_null_suite_by_hndl()
 *        _saf_rank_by_hndl()
 *        _saf_self_decomp()
 *        _saf_self_decomp_by_hndl()
 *        _saf_size_by_hndl()
 *        _saf_string()
 *        _saf_universe()
 *        _saf_valid_handle()
 *        _saf_valid_uncommitted_handle()
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     Object registry functions
 * Chapter:     API Removed Entities
 * Description:
 *      SSlib's way of handling objects supersedes the stuff defined in the old handle.c source file. Therefore the following
 *      functions have been removed from the API and the types of objects that used these functions can now be operated on
 *      using the same style functions as for other objects.
 *
 *        SAF_VALID_UNCOMMITTED()
 *        saf_className()
 *        saf_classOf_handle()
 *        saf_commit()
 *        saf_committed()
 *        saf_consistent()
 *        saf_construct_handle()
 *        saf_declare()
 *        saf_find()
 *        saf_getAttribute()            // use saf_get_attribute() instead
 *        saf_putAttribute()            // use saf_put_attribute() instead
 *        saf_release()
 *        saf_sequenceOf_handle()
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     Object handle allocation functions
 * Chapter:     API Removed Entities
 * Description:
 *      /Object/links/ serve as a unidirectional linking mechanism between one object an another or between a program variable
 *      and an object. They have the same datatypes as the old SAF's object handles, such as SAF_Set and SAF_Field and are
 *      defined as simple C structs. There is no longer any need to have special functions to allocate or initialize these
 *      object links (or handles). Rather, you just declare them in the BSS or data segments, declare them on the stack, or
 *      allocate them on the heap. Once they have space allocated you can initialize that space to all zero just to be safe,
 *      using memset() or by assigning one of the SS_NULL values.
 *
 *        static SAF_Field field[10];                        // allocated in the BSS segment
 *        static SAF_Field *field=calloc(10,sizeof(*field)); // allocated on the heap
 *        for (i=0;i<10;i++) field[i] = SS_FIELD_NULL;       // reset by assignment
 *        memset(field,0,sizeof field);                      // reset with memset
 *        for (i=0;i<10;i++) SS_PERS_ISNULL(field+i);        // should return true now for all i
 *
 *      *NOTE* There should be a SAF name for each of the SS_*_NULL constants since these appear in SAF clients.
 *
 *      Therefore the following functions are no longer useful and have been removed.
 *
 *        saf_new_cat_handle()
 *        saf_new_field_handle()
 *        saf_new_file_handle()
 *        saf_new_ftmpl_handle()
 *        saf_new_rel_handle()
 *        saf_new_set_handle()
 *        saf_new_state_group_handle()
 *        saf_new_stmpl_handle()
 *        saf_new_suite_handle()
 *        saf_renew_cat_handle()
 *        saf_renew_field_handle()
 *        saf_renew_file_handle()
 *        saf_renew_ftmpl_handle()
 *        saf_renew_rel_handle()
 *        saf_renew_set_handle()
 *        saf_renew_state_group_handle()
 *        saf_renew_stmpl_handle()
 *        saf_renew_suite_handle()
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Chapter:     SSlib Phase I
 * Section:     Property lists
 * Audience:    public
 * Description:
 *      The database and library property lists are now pointers.
 *
 *      Old code
 *
 *        SAF_DbProps dp = saf_createProps_database();
 *        saf_setProps_LibComm(&dp, comm);
 *        saf_release(dp);
 *
 *      New code
 *
 *        SAF_DbProps *dp = saf_createProps_database();
 *        saf_setProps_LibComm(dp, comm);
 *        saf_freeProps_lib(dp);
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Runtime Datatype Issues
 * Description:
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     Use of HDF5 datatypes
 * Chapter:     Runtime Datatype Issues
 * Description:
 *      DSL Datatypes have been replaced by HDF5 datatypes, but most of the constants like SAF_INT were preserved.  Although
 *      most changes in the API will result in a compiler error/warning the following does not: comparing datatypes must be done
 *      with H5Tequal() instead of C's !== operator.
 * 
 *        if (Atype==DSL_INT)                          //old code
 * 	  if (H5Tequal(Atype, H5T_NATIVE_INT))         //new code
 *
 *      Any function where NULL was passed as the DSL_Type argument should now have the hid_t constant H5I_INVALID_HID passed
 *      instead.
 *
 *      When converting code I usually do the following:
 *
 *      * Fix compiler errors/warnings in sequence until I encounter a syntax error for a DSL_Type declaration.
 *      * Search for the DSL_Type variables from that line on and replace comparisons with H5Tequal() calls.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     Object link datatypes
 * Chapter:     Runtime Datatype Issues
 * Description:
 *      SAF object datatypes (SAF_Set, SAF_Field, etc) are still valid and are defined in terms of SSlib object link types
 *      (ss_set_t, ss_field_t, etc). The old SAF functions were pretty inconsistent about whether they took a pointer to an
 *      object argument. The new code always takes pointers and thus one of the main source code modifications will be to
 *      add ampersands to many function calls.
 *
 *      The object link types are all derived from, and binary compatible with, the ss_pers_t datatype. Some SSlib functions
 *      take a pointer to ss_pers_t as an argument instead of taking some specific object link type, and in those situations
 *      you should simply use a cast. It's even safe to cast an array of one type to an array of the other. Most of the common
 *      situations where this occurs have corresponding upper-case versions of the SSlib function that are macros that do the
 *      casting.  Comparing two objects is a common situation where this occurs:
 *
 *        SAF_Set s1, s2;
 *        ...
 *        if (ss_pers_eq((ss_pers_t*)&s1, (ss_pers_t*)&s2))     //or
 *        if (SS_PERS_EQ(&s1,&s2))
 *
 *      But be careful when you use the macros or make the cast because the cast will prevent the compiler from warning when
 *      you accidently cast a SAF_Field** to a SAF_Field* and similar. Erik and I have both been bitten by this, and the way
 *      to spot it is by an odd runtime error from ss_pers_eq() or ss_pers_deref() complaining about "problem finding gfile",
 *      "mangled object link", or "object not found".
 * 
 *      For indirect fields and relations, the datatype has changed from DSL_HANDLE to SAF_HANDLE. You may
 *      also sometimes see ss_pers_tm, which the HDF5 datatype corresponding to the C ss_pers_t.  The "tm" means it's a
 *      runtime type for something in memory whereas "tf" means a runtime type for something as it exists when written to a
 *      file (e.g., ss_pers_tf).
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     Variable length arrays
 * Chapter:     Runtime Datatype Issues
 * Description:
 *      You probably won't see any variable length arrays up in the SAF API, but I list them here just in case you're
 *      traversing down the stack into SSlib during a debugging session.  The variable length arrays only appear inside other
 *      objects (since C already has malloc() et al for stuff in memory) and are accessed with the functions defined in the
 *      FILE:sslib/lib/ssarray.c file. See ss_array_get() and related functions for more details.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     String-valued attributes
 * Chapter:     Runtime Datatype Issues
 * Description:
 *      DSL blurred the line between an array of one-byte integers (commonly referred to as !char in C) and a C-style,
 *      NUL-terminated, ASCII character string. Sometimes if you created an attribute with one type you would get the other
 *      type back, and there were certain restrictions on arrays of strings that made them difficult to use.
 *
 *      HDF5 doesn't suffer from these particular problems, although it does require that the string occupy a known fixed
 *      amount of memory (the NUL can appear anywhere in that memory). Since strings typically occupy different size
 *      !char arrays, a unique HDF5 datatype has to be created for each size before you can do anything useful with that size.
 *
 *        char value[256] = "one string of up to 255 characters in a 256-byte array";
 *        hid_t s256 = H5Tcopy(H5T_C_S1);
 *        H5Tset_size(s256, 256);
 *
 *      The type s256 can now be used to write the string to an attribute:
 *
 *        saf_put_attribute(SAF_ALL, db, "attr1", s256, 1, value);
 *        H5Tclose(s256);
 *
 *      As a special shortcut, saf_put_attribute() will do the H5Tcopy(), H5Tset_size(), and H5Tclose() for you when you pass
 *      H5T_C_S1 as the datatype and a count (penultimate argument) of one.  The implied H5Tset_size() call will pass the
 *      exact length of the string value including the NUL character.
 *
 *      You could read the attribute back into a buffer with a different size if you desired, something that DSL seemed to
 *      have some problems doing with strings:
 *
 *        int count=1;
 *        char got[1024], *got_p=got;
 *        hid_t s1024 = H5Tcopy(H5T_C_S1);
 *        H5Tset_size(s1024, 1024);
 *        saf_get_attribute(SAF_ALL, db, "attr1", &s1024, &count, &got_p);
 *        assert(1==count);
 *        H5Tclose(s1024);
 *
 *      If you want to pass more than one string at a time to saf_put_attribute() or saf_get_attribute() then you should
 *      declare the !value and/or !got arrays as two dimensional arrays rather than the more common array of pointers:
 *
 *        // This is the wrong way
 *        char *value[3] = {"first", "second", "third"};
 *        char *got[3], *got_p=got;
 *
 *        // This is the right way
 *        char value[3][256] = {{"first"}, {"second"}, {"third"}};
 *        char got[3][256], *got_p=got;
 *
 *      And the saf_put_attribute() call could either specify a count of three s256 types or you could build an HDF5 array of
 *      s256 and pass a count of one.  Whatever you decide will affect what saf_get_attribute() returns.
 *
 *      *Variable*Length*Strings*in*Objects*
 *
 *      If you ever debug down in SSlib you may see another string datatype called ss_string_t. These are variable length
 *      strings that appear in objects (e.g., the name of a field) and have HDF5 types ss_string_tm and ss_string_tf for
 *      memory and file.  You would access these strings with the functions in FILE:sslib/lib/ssstring.c such as
 *      ss_string_ptr().
 *
 *      SSlib uses a customized ss_string_t type for variable length strings in objects because HDF5's VL string type is too
 *      costly in parallel and yet we wanted some way for all Fields in a table to have different length names and if many
 *      fields had the same name (as they might in a SAF_EACH call to saf_declare_field) that we only store the name once.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Miscellaneous API Changes
 * Description:
 *      These miscellaneous changes were made to the SAF API:
 *
 *      * saf_base_quantity(): This was actually an internal function and is no longer necessary. Use macros like SAF_QTIME
 *        instead.
 *
 *      * saf_equiv(): This function and the SAF_EQUIV() macro (which remains) used to check whether two object handles
 *        pointed to the same object. The ss_pers_eq() function and SS_PERS_EQ() macro do the same thing. There will also be
 *        an ss_pers_equal() function and SS_PERS_EQUAL() macro that look inside the objects to see if they're equal (right
 *        now these just fall back to comparing whether two object links point to the same object).
 *
 *      * saf_find_algebraics(): This was added for completeness. Also saf_find_bases(), saf_find_evaluations(),
 *        saf_find_quantities(), saf_find_relreps(), saf_find_roles(), saf_find_units()
 *
 *      * saf_grab_hdf5(): This now returns the HDF5 file handle by value instead of reference.
 *
 *      * saf_setProps_DoToc(): This no longer makes any sense and has been removed.
 *
 *      * saf_setProps_MemoryResident(): The memory increment and save-on-close arguments have been removed.
 *
 *      * saf_setProps_OSModes(): This has been dropped as has saf_setProps_PoorMansGPFS() and
 *        saf_setProps_PromiseMode().
 *
 *      * saf_universe(): Use the old SAF_UNIVERSE() macro instead.
 *        
 *      * saf_declare_*: The declare functions now all return a pointer on success or NULL
 *                       on failure. The previous behavior was that some returned integers and some returned pointers.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Debugging
 * Description:
 *      This chapter describes various debugging techniques and hints.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     Looking at SAF databases with h5ls
 * Chapter:     Debugging
 * Description:
 *      Using h5ls isn't the easiest way to view SAF files generated with SSlib because certain tricks had to be used in order
 *      to get the best parallel performance and to fully leverage HDF5's datatype conversion infrastructure.  A better tool
 *      is saf2html described in the next section, but sometimes h5ls is good enough.  Here's a tour through a typical file
 *      using h5ls.
 *
 *      If you just list the file you'll probably only see a group named "SAF". This is the top-level scope of the file and,
 *      at least for Phase-I, will be the only scope in the file. Every scope in SSlib corresponds to a group in an HDF5 file.
 *
 *      Inside the SAF group you'll see one dataset for each of SAF's object types. These are the object tables from VBT, but
 *      in order to have a largely independent API SSlib needs to create the datasets up front and therefore they're sometimes
 *      empty.  You'll notice a few new datasets too.
 *
 *        matzke@inspiron test $ h5ls test_saf.saf/SAF
 *        1.9.0-sslib              Group
 *        Blob-Storage             Group
 *        Strings                  Dataset {3149/Inf}
 *        algebraic                Dataset {0/Inf}
 *        attr                     Dataset {0/Inf}
 *        basis                    Dataset {0/Inf}
 *        blob                     Dataset {50/Inf}
 *        cat                      Dataset {9/Inf}
 *        collection               Dataset {41/Inf}
 *        evaluation               Dataset {0/Inf}
 *        field                    Dataset {27/Inf}
 *        fieldtmpl                Dataset {14/Inf}
 *        file                     Dataset {2/Inf}
 *        indexspec                Dataset {45/Inf}
 *        quantity                 Dataset {0/Inf}
 *        rel                      Dataset {42/Inf}
 *        relrep                   Dataset {0/Inf}
 *        role                     Dataset {1/Inf}
 *        scope                    Dataset {1/Inf}
 *        set                      Dataset {23/Inf}
 *        tops                     Dataset {0/Inf}
 *        unit                     Dataset {0/Inf}
 *
 *      The new Strings dataset is used as a heap for all variable length strings in that scope.  Each string consists
 *      of a length/value pair, which allows for the storage of arbitrary binary data.  The offsets into the string table that
 *      appear in the various objects are offsets to the beginning of the value, which is preceded by the four-byte,
 *      little-endian length (it's done this way because offset zero has special meaning).
 *
 *      The 1.9.0-sslib group contains version specific stuff like shared datatypes and you should never have to look inside
 *      it.
 *
 *      The Blob-Storage group is where raw data is stored unless the SAF client specifies some other location. The datasets
 *      in this group have numeric names that look pretty random at first glance. If you look closer (use h5ls -v) you'll
 *      notice that the name is the same as the dataset's object header file address (the Location reported by h5ls).  This is
 *      a result of the way that SSlib guarantees unique dataset names.  If you look at the "blob" table you'll see that the
 *      !dsetaddr values are these same integers, and that's how a blob points to a dataset.
 *
 *      Now lets look at some specific objects, the collection categories:
 *
 *        matzke@inspiron test $ h5ls -vdl test_saf.saf/SAF/cat 
 *        Opened "test_saf.saf" with sec2 driver.
 *        SAF/cat                  Dataset {9/Inf}
 *            Location:  1:58264
 *            Links:     1
 *            Modified:  2004-09-27 21:10:48 EDT
 *            Chunks:    {215} 4085 bytes
 *            Storage:   171 logical bytes, 4085 allocated bytes, 4.19% utilization
 *            Type:      shared-1:4304 struct {
 *                           "name"             +0    4-byte opaque type
 *                           (tag = "SSlib::string_tf")
 *                           "role"             +4    struct {
 *                               "file_idx"         +0    native unsigned short
 *                               "scope_idx"        +2    native unsigned int
 *                               "table"            +6    enum native unsigned char {
 *                                   scope            = 0
 *                                   field            = 1
 *                                   role             = 2
 *                                   basis            = 3
 *                                   algebraic        = 4
 *                                   evaluation       = 5
 *                                   relrep           = 6
 *                                   quantity         = 7
 *                                   unit             = 8
 *                                   cat              = 9
 *                                   collection       = 10
 *                                   set              = 11
 *                                   rel              = 12
 *                                   fieldtmpl        = 13
 *                                   tops             = 14
 *                                   blob             = 15
 *                                   indexspec        = 16
 *                                   file             = 17
 *                                   attr             = 18
 *                               }
 *                               "item_idx"         +7    native unsigned int
 *                           } 11 bytes
 *                           "tdim"             +15   native int
 *                       } 19 bytes
 *            Data:
 *                (0) {name=07:03:00:00, role={file_idx=1, scope_idx=0, table=role, item_idx=1}, tdim=0},
 *                (1) {name=11:03:00:00, role={file_idx=1, scope_idx=0, table=role, item_idx=1}, tdim=2},
 *                (2) {name=1b:03:00:00, role={file_idx=0, scope_idx=0, table=role, item_idx=0}, tdim=1},
 *                (3) {name=25:03:00:00, role={file_idx=1, scope_idx=0, table=role, item_idx=3}, tdim=2},
 *                (4) {name=30:03:00:00, role={file_idx=0, scope_idx=0, table=role, item_idx=0}, tdim=1},
 *                (5) {name=3e:03:00:00, role={file_idx=0, scope_idx=0, table=role, item_idx=0}, tdim=0},
 *                (6) {name=4c:03:00:00, role={file_idx=1, scope_idx=0, table=role, item_idx=7}, tdim=1},
 *                (7) {name=60:03:00:00, role={file_idx=1, scope_idx=0, table=role, item_idx=8}, tdim=1},
 *                (8) {name=74:03:00:00, role={file_idx=1, scope_idx=0, table=role, item_idx=1}, tdim=0}
 *
 *      The things to notice are:
 *
 *      * The tables are extendible each having a chunk size of around 4kB. That means that most of the little
 *        SAF test cases and examples will waste lots of space.
 *
 *      * The !name member of the datatype is an opaque type named SSlib::string_tf and the values printed for name are
 *        four-byte hexadecimals. These are the little endian offsets into the Strings dataset.
 *
 *      * The !role member, which is a link to a role object has a more complicated structure than the simple integer
 *        that was used in DSL.  That's because objects in SSlib need to be able to point into other scopes and even into
 *        other files.
 *
 *      * The total size of a collection as stored in a file is only 19 bytes compared with 80 bytes in old SAF.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     Looking at SAF databases with saf2html
 * Chapter:     Debugging
 * Description:
 *      The easiest way to look inside a SAF database is to run the saf2html tool (FILE:sslib/tools/saf2html) to generate
 *      an HTML file which can be viewed with your favorite browser.  The saf2html tool takes one argument which says what
 *      part of the database should be rendered as HTML. I usually invoke it as:
 *
 *        matzke@inspiron test $ saf2html test_saf.saf/SAF >test_saf.html
 *
 *      The reason you need to tack on the FILE:/SAF is because SSlib can (almost) support multiple databases per HDF5 file.
 *      Currently we only use one database per file and the name of that database defaults to FILE:SAF.  You can also look at
 *      a particular table or a particular object within a table with arguments such as FILE:test_saf.saf/SAF/field or
 *      FILE:test_saf.saf/SAF/field/4, but I usually find that having the entire file as HTML is most useful. By the way, asking
 *      to see a specific object will also output some additional information that you don't get otherwise.
 *
 *      What follows is a tour of the test_saf.saf file generated by FILE:safapi/test/larry1w
 *
 *      The first thing you'll see is a couple of new tables called !scope and !file.  The Scope table should have only one
 *      item in Phase I--it is the name of the top-level scope which is opened by all tasks automatically when the file is
 *      opened.  The File table is a list of all files referenced by this scope with the first item in that table being the
 *      name of this file.  The file named FILE:/SAF:std_types is the built-in object registry and doesn't actually correspond
 *      to any file in the filesystem.
 *
 *      If you want to see what SSlib is using for the built-in object registry then you can run any application with the
 *      SAF_REGISTRY_SAVE environment variable set to the name of a file and the registry will be generated in that file.
 *      Don't expect the program to run correctly in this mode of operation because there is at least one piece of
 *      functionality currently missing from SSlib (a correct implementation of ss_pers_equal()).
 *
 *      Object links in the output have at least two different formats. Consider a field template: a field template has links to
 *      a quantity and zero or more component field templates (among others). The quantity is probably one of the predefined
 *      quantities from the built-in object registry and will be displayed something like:
 *
 *        (ss_quantity_t*){1,0,4}
 *
 *      which should be read as a link to the quantity described by file 1, scope 0, table item 4. To see what file that would
 *      be you would look at the file table at index 1 and you would see the name FILE:/SAF:std_types  The
 *      middle number (the scope index) should always be zero for Phase I and refers to the top scope named "SAF".
 *
 *      If an object link points to an object in the same file (which also implies the same scope for Phase I) then an
 *      abbreviated form of output would be used. You'll probably see this in field templates that point to component field
 *      templates. Those component templates will be displayed like this:
 *
 *        (ss_fieldtmpl_t*)3
 *
 *      which means the field template at index 3 in the field template table of the same scope of the same file.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     Comparing new and old files
 * Chapter:     Debugging
 * Description:
 *      It's been very valuable to have old and new versions of each of SAF's test cases in order to spot errors in the new
 *      SAF library. All one has to do is run the same test in the two CVS branches and look at the data files (or even run
 *      them both side by side in debuggers).
 *
 *      Some things to note when comparing old and new:
 *
 *      * Anywhere you see VBT_Id_t or a table row number in old SAF you'll see an ss_pers_t object link in new SAF.
 *
 *      * Old and new SAF handle collections a little differently. The old version just had a fixed length array of
 *        collections in a set and indexed those collections by category table row numbers.  The new SAF has a variable
 *        length array of collections in the set and the lookup is done by scanning the array and comparing the !cat link of
 *        each collection with the specified category.
 *
 *      * The tables might not have the same numbers of objects because old SAF copied objects from the registry into the file
 *        whereas new SAF simply points into the registry.  Otherwise the number of objects should generally be the same.
 *
 *      * The objects might not be in the same order in the tables, especially if objects are declared with SAF_EACH mode.
 *        This happens because of the different ways that the old and new SAF promoted local objects to their global
 *        table locations.
 *
 *      * Old SAF used DSL's metablob interface to store small arrays by overlaying the metablob information on top of
 *        where the blob info would normally be stored. New SAF creates additional variable length array members in the object
 *        to hold this info.  For instance, a relation stores indirect relation links in an indirect_rels array instead of the
 *        d_blob.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     The SSLIB_DEBUG environment variable
 * Chapter:     Debugging
 * Description:
 *      The SSLIB_DEBUG environment variable controls lots of run-time debugging things in SSlib. See ss_debug() for complete
 *      information about how to use it.  Here are some tricks:
 *
 *      To start the DDD debugger for MPI task 5 you would do the following. This currently only works on systems with a
 *      FILE:/proc file system that contains enough information to start the debugger (e.g., Linux). Once the debugger starts
 *      up you'll have to !continue twice before the program actually starts to run.
 *
 *        matzke@inspiron test $ SSLIB_DEBUG='task=5;debug' mpiexec -n 8 ./larry1w do_writes
 *
 *      To start GDB on all tasks you could do the following. Again, you'll have to !continue twice to get running. It's a
 *      little confusing about which xterm corresponds to which MPI task, but if you !up one level there should be a !self
 *      local variable you can print to get the task rank in MPI_COMM_WORLD.
 *
 *        matzke@inspiron test $ SSLIB_DEBUG='debugger=xterm -e gdb;debug' mpiexec -n 8 a.out
 *
 *      When SSlib or HDF5 detects an error it will print a stack trace. If you set SSLIB_DEBUG to the word !error then the
 *      stack trace will contain error identification numbers. If you want the program to stop when it hits error number 123
 *      from MPI task 0 you would invoke the program as follows (if you do this without the mpiexec or mpirun you'll probably
 *      get only the DDD command window and the shell prompt and all you need to do is foreground the job again to get the
 *      rest of the debugger windows).
 *
 *        matzke@inspiron test $ SSLIB_DEBUG='task=0;error=123' mpiexec -np 8 a.out
 *
 *      When SAF detects an error it calls MPI_Abort() and the program dies before you have a chance to debug it. But if you
 *      invoke it like this then you'll get a debugger just before the call to MPI_Abort(). You'll also get a debugger for
 *      segmentation faults and certain other fatal signals.
 *
 *        matzke@inspiron test $ SSLIB_DEBUG=signal mpiexec -np 8 a.out
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     Stack traces in emacs
 * Chapter:     Debugging
 * Description:
 *      If you run the SAF test suite or other program as part of a compile command (e.g., M-x compile RET) then Emacs will
 *      treat the stack trace as if it were error messages and you can use the next-error command to have Emacs take you
 *      directly to the source code line at that level of the stack.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Section:     Looking inside objects in a debugger
 * Chapter:     Debugging
 * Description:
 *      If you print or display an object link inside a debugger you don't get much obviously useful information about the
 *      object itself.  If !rel is a pointer to a SAF_Rel then you might get something like this:
 *      
 * 	  (gdb) print *rel
 * 	  $1 = {pers = {obj = {magic = 1525686284},
 * 	       state = SS_PERS_LINK_MEMORY, gfileidx = 3,
 *                open_serial = 1, scopeidx = 0, objidx = 12,
 *                objptr = 0x8672fb0}}
 *
 *      Here's what you can find out about the object just from the object link:
 *
 *      * The !state will tell you whether the link is a null link, a link to an object that might be in memory, or a link
 *        to something that's in a file that was either never opened or has since been closed and is known to have been
 *        closed. The reason this is a little fuzzy is because a link is really a cache of some object information and
 *        determining whether the cache is up to date is somewhat difficult.  The easiest way is to just invoke
 *        ss_pers_update() on the object link before looking at it.  But usually if the state is SS_PERS_LINK_MEMORY then the
 *        !objptr is a pointer to the object, and furthermore if the !objidx is less than 0x10000000 then the !objptr address
 *        will be valid for as long as the object's file remains open. This is very useful in DDD where you can use that
 *        address to display the object and object's value will be kept updated in the display window regardless of whether
 *        the object link is in scope.
 *
 *      * If you print the magic number in hexadecimal and then look it up in FILE:sslib/lib/ssobj.h you can figure out what
 *        kind of object is being pointed to. In this case it's the value for the constant SS_MAGIC_ss_rel_t, which means the
 *        object is of type ss_rel_t, an alias for SAF_Rel.
 * 
 *          (gdb) p/x rel->pers.obj.magic
 * 	    $2 = 0x5af0200c
 *
 *      * The !objidx value is the index into the table where the object is stored. If the high-order bit is set then the
 *        object is new (i.e., just born independently and not yet synchronized), it has only a temporary home in the table,
 *        and the low-order bits determine the current index into the table. A new object will turn into a permanent object
 *        during the next table synchronization, and once it has a permanent home in a table it will *never* change.
 *
 *      * The !scopeidx is an index into the scope table and will always be zero for Phase I.
 *
 *      * The !gfileidx is an index into a global array that contains information about the file that owns the object. The
 *        most useful thing here is the file name.
 *
 *          (gdb) p sslib_g.gfile.ent[rel->pers.gfileidx]
 *
 *      You have a few choices about how to follow an object link to see inside the object itself.
 *
 *      * If the link is in the SS_PERS_LINK_MEMORY state you can cast the link's !objptr pointer to the appropriate
 *        datatype and print or display it.  Object types have the same names as object link types except for the letters
 *        "obj" inserted before the "_t" (so a SAF_Rel is an alais for ss_rel_t, a link which points to an ss_relobj_t).
 *
 *          (gdb) p *(ss_relobj_t*)(rel->pers.objptr)
 *
 *      * You can step into the next ss_pers_deref() call and print the !persobj variable after the ss_pers_link_objptr()
 *        call.  Again, you'll have to cast it to the right object type.  Most of the time SAF will use a macro to call
 *        ss_pers_deref(). The names of the macros are uppercase versions of the object types like SS_FIELD() and SS_SET().
 *
 *      * You can call ss_pers_debug() which prints out everything that SSlib knows about the object link and the object
 *        to which it points.  The output format is the same as what you would get from saf2html when you ask for a specific
 *        object and which is described below.
 *
 *      * Some objects are complicated enough that a simple-minded !print will be almost unreadable. DDD comes in really handy
 *        here because the !display button/command will show the object in tabular form. And if you use an address instead of
 *        a variable then the display stays active even after you leave that scope.
 *
 *      When Slib prints information about an object link and the object to which it points it uses a special format. Anything
 *      printed in hexadecimal is either a pointer or a bit field (bit fields are pretty obvious because they're mostly zero).
 *      Things in quotes are generally names or strings. A word followed by parentheses usually means that the parenthesized
 *      thing is supplying more detail about the word (e.g., hdf5(16777216) means an HDF5 object whose value is 16777216).
 *      Variable length arrays are values between square brackets. Links to objects have the format described earlier (see
 *      Looking at SAF databases with saf2html).
 *        
 * 
 *        (gdb) p ss_pers_debug(rel)
 * 	  $1 = 0
 * 	  SSlib-0: link:            0x086fe150 memory rel(12)
 * 	  SSlib-0: global file:     direct(3) 0x084e0150 serial(0x00000003)
 * 	  SSlib-0: file name:       "/home/matzke/[...]/safapi/test/test_saf.saf"
 * 	  SSlib-0: file status:     open(1) flags(RDONLY) hdf5(16777216)
 * 	  SSlib-0: scope:           direct(0) 0x0859b2a0 hdf5(50331648) "SAF"
 * 	  SSlib-0: object:          direct(12) 0x086733f8 synchronized !dirty
 * 	  SSlib-0: checksums:       sync0=0x00000000-00 current=0x214c0057-00
 * 	  SSlib-0:   sub            (ss_set_t*)0
 * 	  SSlib-0:   sub_cat        (ss_cat_t*)1
 * 	  SSlib-0:   sub_decomp_cat (ss_cat_t*)2
 * 	  SSlib-0:   sup            (ss_set_t*)0
 * 	  SSlib-0:   sup_cat        (ss_cat_t*)0
 * 	  SSlib-0:   sup_decomp_cat (ss_cat_t*)2
 * 	  SSlib-0:   kind           SAF_RELKIND_SUBSET
 * 	  SSlib-0:   rep_type       (ss_relrep_t*){1,0,2}
 * 	  SSlib-0:   d_blob         (ss_pers_t*)NULL
 * 	  SSlib-0:   r_blob         (ss_pers_t*)NULL
 * 	  SSlib-0:   indirect_rels  [(ss_rel_t*)9, (ss_rel_t*)10, (ss_rel_t*)11]
 * 
 *      Here's the explanation line by line:
 *
 *      * 3: The address of the object link is 0x086fe150. The object's state is SS_PERS_LINK_MEMORY. The object link points
 *           to a relation and the sequence part of relation magic number is decimal 12 (the "c" in 0x5af0200c and
 *           0x5af0300c).
 *           
 *      * 4: The file to which this object belongs is described by a direct index of 3 into the GFile array
 *           (sslib_g.!gfile.ent[3]). The ss_gfile_t address for this file is 0x084e0150, which currently has a serial number
 *           of 3. The serial number is used by ss_pers_update() to determine whether information cached in the object link is
 *           up to date.
 *
 *      * 5: The file name is generated from the current working directory plus the name used in the H5Fopen() call.  It might
 *           not be the same as the file name stored at index zero of the File table in the top scope of the file.
 *
 *      * 6: The file is currently opened once (multiple nested calls to ss_file_open() are allowed) for read-only access. The
 *           underlying HDF5 file handle has a value of 16777216, which is what you would search for in the HDF5 API tracing
 *           output.
 *
 *      * 7: The object belongs to scope number zero. The word "direct" here and elswhere means zero is a normal index into the
 *           table rather than an indirect index. Indirect indices are used for new objects (objects that were recently
 *           created independently and have not yet been synchronized) and have their high-order bit set.  The scope object
 *           of type ss_scopeobj_t is at address 0x0859b2a0 and contains some useful run-time information about the scope. The
 *           HDF5 group for the scope is 50331648 and will appear in HDF5's API tracing output. The scope name and HDF5 group
 *           name is "SAF".
 *
 *      * 8: The object is not new (i.e., the object is direct) at index 12 in the relation table of this scope and has
 *           address 0x086733f8. We know that address will be valid until the file is closed since the object is direct. The
 *           object has not been modified on this task since it was last synchronized, and it is not dirty (i.e., it has not
 *           been modified on this task since this task last wrote to or read from disk).
 *
 *      * 9: The last time the object was synchronized was at pass zero (thus sync0) and it had an all zero checksum at that
 *           time. This situation is actually a special case that means the object has never been explicitly synchronized, but
 *           since line 8 says it's synchronized then it must have either been recently read from disk or collectively created.
 *           The current checksum is 0x214c0057-00 where the 00 means that the object is fully resolved (i.e., it contains no
 *           links to new objects).
 *
 *      * 10-20: These lines show the actual contents of the object and only appear if the object is in memory. The names
 *           in the left column are the names of the table columns. By the way, this stuff is all generated automatically
 *           from the object definition in (probably) the FILE:sslib/lib/sspers.h file (but unlike VBT/DSL, SSlib doesn't have
 *           huge amounts of generated source code).
 *
 *      * 10-15,17-19: These lines are links to sets, categories, a relation representation type, and two null links. Their
 *           format was described in the saf2html output (see Looking at SAF databases with saf2html).
 *
 *      * 16: !kind is an enumeration type with the value SAF_RELKIND_SUBSET.
 *
 *      * 20: indirect_rels is a variable length array currently containing three links to relations.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Revision Control
 * Description:
 *      The new SSlib version of SAF is on the sslib-phase1 branch of CVS and all Phase I work should be done on that
 *      branch. You would check out with a command such as the following:
 *
 *        $ cvs co -P -r sslib-phase1 -d saf-sslib saf
 *
 *      I occassionally merge the main branch changes into the sslib-phase1 branch. If someone else does it then
 *      please reset the sslib-merged(?) tag to the current revisions in the main branch so the next time we merge we know
 *      which mainline revisions to start with.
 *
 *      The FILE:src/vbt and FILE:src/dsl directories will continue to exist in the sslib-phase1 branch for the time being
 *      just because they're good reference, but eventually they'll be removed.
 *
 *      Once Phase I work is completed we will move all the SSlib stuff back to the main branch and retire the
 *      sslib-phase1 branch.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Testing
 * Description:
 *      We haven't been doing tier 1 testing, but now that the new SAF is in a condition where such testing makes sense
 *      we should start performing a tier1 test whenever we check something in.  I've installed the following software on
 *      tesla for this purpose:
 *
 *      * MPICH: Version 1.2.5 for tier1. Version 2.0.971 is also known to work with HDF5 and SAF and in some respects
 *               is easier to use than 1.2.5, but we will stick with 1.2.5 as the official version.
 *
 *      * HDF5:  SSlib requires 1.7.37 or later because of API changes and bug fixes.
 *
 *      The software is installed under my home directory FILE:~rpmatzk in directories FILE:mpich and FILE:hdf5 using the
 *      build_saf script. If anyone installs additional versions or configurations with the same naming scheme could you
 *      please let me know so I can add symbolic links to your versions.
 *
 *      The easiest way to configure, build, and install SAF (and MPICH and HDF5 too for that matter) is to use the
 *      build_saf script in the top-level FILE:tools directory. If you invoke it with the --help switch you'll get a
 *      couple pages describing all the command-line switches.  Here's a couple examples, but the first thing you should
 *      do is make symbolic links to my FILE:hdf5 and FILE:mpich directories from your home directory.
 *
 *      To configure, build, and test the version of SAF you checked out to a directory named FILE:~/saf-sslib you could
 *      do the following:
 *
 *        $ ~/saf-sslib/tools/build_saf --compiler=mpich-1.2.5 --generator=64 --hdf5=1.7.37
 *
 *      The command-line switches can be abbreviated to their shortest unique prefix and I generally use !comp instead of
 *      !compiler and !gen instead of !generator.
 *
 *      If you want to skip running the test suite you can supply different targets for the make command.  If you want to
 *      just configure and not run make at all you can specify !none as the target:
 *
 *        $ ~/saf-sslib/tools/build_saf --comp=mpich-1.2.5 --gen=64 --hdf5=1.7.37 --targets=all  # no test suite
 *        $ ~/saf-sslib/tools/build_saf --comp=mpich-1.2.5 --gen=64 --hdf5=1.7.37 --targets=none # configure only
 *
 *      The script creates a FILE:build directory where all temporary files are stored (its name is printed near the
 *      beginning of the build_saf output). Once you've configured SAF you can !cd down into the build directory and run
 *      make there manually.
 *
 *      To configure SAF for a serial build you would just not specify a compiler, or you could specify the name of a
 *      serial compiler such as !cc or !gcc.
 *
 *      By default, build_saf configures SAF for debugging. You can configure SAF for a production (optimized) environment
 *      or for profiling with !gprof by using the --product switch and a value of !optim or !profile (the default is
 *      !debug):
 *
 *        $ ~/saf-sslib/tools/build_saf --comp=mpich-1.2.5 --gen=64 --hdf5=1.7.37 --prod=optim
 *
 *      You can use the latest, greatest build_saf found in the sslib-phase1 branch to configure the old SAF too if you
 *      like (in fact it's necessary if you're using MPICH2). You would do this by adding the name of the configure script
 *      to the end of the command line, and you can even add additional configure switches if you like:
 *
 *        $ ~/saf-sslib/tools/build_saf --comp=mpich-1.2.5 --gen=64 --hdf5=1.6.3 ~/saf-old/src/configure --disable shared
 *
 *      The same thing works for MPICH and HDF5:
 *
 *        $ ~/saf-sslib/tools/build_saf --gen=64 /usr/local/mpich/2.0.971/source/configure --device=shmem
 *        $ ~/saf-sslib/tools/build_saf --comp=mpich-2.0.971 --gen=64 /usr/local/hdf5/1.7.37/source/configure
 *
 *      You probably will want to install MPICH and HDF5 and you do so by adding the --install switch. If you want to
 *      check where it will be installed you can run build_saf with the --no-exec switch.  Even if you don't use the
 *      --install switch you can still choose to install later by just saying !make !install.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*
 * 
 * General Coding notes
 * ==================
 * * Use of SSLIB_SUPPORT_PENDING
 *   If it is unknown whether a certain section of code will ultimately
 *   be needed by the Phase-I integration then comment it out with
 *     #ifdef SSLIB_SUPPORT_PENDING
 *   That symbol is not defined (and never will be).  This will allow us to
 *     1. easily convert the code for SSlib support if we determine that
 *        it must be included (no need to go copy it out of the mainline)
 *     2. easily find all code commented out in this manner so it can be
 *        deleted when we're about to retire the sslib-phase1 branch.
 * 
 * * Don't reformat code unless you really need to (or you know you're the
 *   only one touching it since your last commit) because it makes
 *   merging more difficult. Some functions are so unreadable that you'll
 *   have to reformat them anyway.  Interactive, visual merge tools are
 *   your friends in those cases! (e.g., xxdiff or XEmacs emerge which is
 *   under the Tools->Merge menu).
 * 
 * * The SSlib reference manual is at http://www.wcrtc.net/~matzke/sslib
 * 
 * 
 * Specific changes made to the code
 * =================================
 * * SAF parallel modes
 *    Any function that takes a SAF_ParMode argument will have the
 *    following near the beginning, even before it checks for other
 *    preconditions:
 * 
 *     SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
 *                 _saf_errmsg("PMODE must be valid"));
 *     if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
 * 
 *    Generally speaking, all SAF API functions are now independent
 *    unless the SAF_ALL parallel mode is specified, in which case
 *    they're collective across the SAF_Db communicator.
 * 
 *    By the way, SAF_ONE mode now works for all object types, but it
 *    will be retired in Phase-II because the same thing can be done with
 *    a SAF_EACH call from just one task (because these functions are now
 *    independent). SAF_ONE simply returns failure for all but the one
 *    task.
 * 
 * * SSlib parallel modes
 *    Nearly all SSlib functions are independent, however many of them
 *    take an optional SS_ALLSAME bit flag to indicate that all tasks are
 *    participating with identical data even though no communication will
 *    occur. Please use this flag when appropriate since it will prevent
 *    communication from occurring later.  The SS_ALLSAME can generally
 *    be used whenever the SAF_ParMode is SAF_ALL.
 * 
 *        set = SS_PERS_NEW(scope, ss_set_t, SAF_ALL==pmode?SS_ALLSAME:0U)
 * 
 *    (The `0U' is because all SSlib bit flags are unsigned.)
 *       
 * * Database argument in SAF
 *    Many functions take a SAF_Db argument. Those arguments will now be
 *    SAF_Db* pointers (they're actually SSlib file pointers, ss_file_t*)
 * 
 *    Phase-II will change SAF_Db* arguments to scope arguments and the
 *    SAF API will be extended with functions to create/open/close
 *    scopes within a database.
 * 
 * * Database argument in SSlib
 *    Most SSlib functions want a scope (ss_scope_t*) instead of a SAF_Db*
 *    (ss_file_t*) argument. Phase-I assumes that a SAF_Db and a scope
 *    are one-to-one mapped, so in order to get the scope for a database
 *    do the following:
 * 
 *        ss_scope_t scope;
 *        if (NULL==ss_file_scope(database, &scope))
 *            SAF_ERROR(SAF_SSLIB_ERROR,
 * 	              _saf_errmsg("unable to obtain the scope for "
 * 		                  "the database."))
 * 
 * * Supplemental files
 *    SSlib doesn't make any distinction between a supplemental file and
 *    a master file: they are all a SAF_Db and thus the database
 *    interface can be used for everything.
 * 
 * * Datatypes
 *    DSL's datatype interface was replaced with HDF5's interface. The
 *    SAF datatypes like SAF_INT are being phased out in favor of the
 *    much more complete HDF5 types like H5T_NATIVE_INT.
 * 
 * * Special objects (like SAF_UNIVERSE)
 *    These objects are now defined in the built-in registry and since
 *    SSlib can point from one dataset into another there is no need to
 *    define them in each dataset. The net effect is that the database
 *    argument to these macros is unused.  To make this clear, any time
 *    you write a new SAF_UNIVERSE or othe macro please use NULL as the
 *    argument.  Phase-II will go back and remove all the arguments.
 * 
 * * Declarators return pointers
 *    All the saf_declare_whateve functions will return a pointer to the
 *    object. In existing calls this will just be the same pointer as the
 *    last argument, however in new calls the last argument can be NULL
 *    in which case a handle is allocated.  The handle can be freed with
 *    free() or preferably _saf_free().
 * 
 *       SAF_Set *saf_declare_set(SAF_ParMode pmode, SAF_Db *db, .....,
 *                                SAF_Set *newset);
 * 
 * * Persistent object arguments
 *    Anything that's a persistent object (sets, fields, roles, etc.)
 *    should now be a pointer if it's used as an argument.  No persistent
 *    object handles are ever passed around by value, however they can
 *    certainly be created on the stack. You can also initialize them if
 *    you want (the initializer is all zero)
 * 
 *      void foo(SAF_Set *set_arg)
 *      {
 *          SAF_Set set_here=SS_SET_NULL, *set_alloc=NULL;
 * 
 * 	 saf_declare_set(........., &set_here);
 * 	 set_alloc = saf_declare_set(........, NULL);
 * 
 * 	 saf_describe_set(...., set_arg, ...);
 * 	 saf_describe_set(...., &set_here, ...);
 * 	 saf_describe_set(...., set_alloc, ...);
 * 
 * 	 set_alloc = _saf_free(set_alloc);
 *      }
 * 	 
 * * Marking objects dirty
 *    Any time you modify a persistent object (and didn't just create it
 *    in the same function) it needs to be marked as dirty.  For
 *    instance, if you make a Unit point to a new Quantity you would do
 *    something like this:
 * 
 *        SAF_ParMode pmode = ...;
 *        SAF_Unit *unit = ....;
 *        SAF_Quantity *quant = ....;
 * 
 *        SAF_DIRTY(unit, pmode);          <-- mark unit as dirty first
 *        SAF_UNIT(unit)->quant = quant;   <-- then modify things
 * 
 *     If a pmode is not available then assume that each task has
 *     different data by passing SAF_EACH.
 * 
 *     No need to mark as dirty if the object was just created (all
 *     objects are born dirty).
 * 
 *     No need to mark as dirty if you're only modifying in-memory
 *     stuff. All these are indicated by the fact that they're defined in
 *     an `m' struct, like:
 * 
 *        SAF_RELATION(rel)->m.abuf = NULL;
 * 
 * * Strings in persistent objects
 *    Persistent objects now use variable length strings so you have to
 *    use the ss_string_* interface to access them.  The two most common
 *    operations will be to copy an object name into a user-supplied
 *    address or to modify an object name:
 * 
 *      void foo(SAF_Set *set, char **oldname, const char *newname) {
 *          _saf_setupReturned_string(oldname, ss_string_ptr(SS_SET_P(set,name)));
 * 	 SAF_DIRTY(set,SAF_EACH);
 * 	 ss_string_set(SS_SET_P(set,name), newname);
 *      }
 * 
 * * DSL Metablobs
 *    Metablobs were used to store small variable length arrays of
 *    pointers to other objects. Most of these can be replaced with
 *    actual variable length arrays in the objects themselves (see how a
 *    collection points to its member sets or a set points to its
 *    collections).  These variable length arrays can be manipulated with
 *    the ss_array_* interface.
 * 
 * * DSL Blobs
 *    Replace with ss_blob_* interface, but be careful of collectivity
 *    constraints because they're different than most other parts of
 *    SSlib due to HDF5 dataset constraints.
 * 
 *    (Eric, I'll plan to work on the field.c and/or rel.c files where
 *    most of this sort of thing occurs.)
 * 
 * * Synchronization
 *    We'll try the first cut with no inter-task synchronization of
 *    objects and see what happens. That means that if someone does a
 *    SAF_EACH "declare" to create N objects and then does a "find" only
 *    one of those objects will be found on each of the N tasks.
 * 
 *    At first we'll see if we can make it the application's
 *    responsibility to synchronize. If that turns out to be infeasible
 *    then we'll make the SAF API collective again and add a call to
 *    ss_scope_synchronize() in each of the API functions. Doing so will
 *    probably make the SSlib version of SAF slower than the DSL/VBT
 *    version of SAF since the whole point of synchronization was to
 *    aggregate communication.
 * 
 * 
 * Implementation Schedule
 * =======================
 * 
 * * src/safapi/tools
 *    Nothing works here yet. Lets get the `test' directory working first
 *    before we expend too much effort on the tools. In fact, I'll change
 *    the order in the Makefile, but for now you'll have to do a `make'
 *    in the lib directory and then a `make' in the test directory in
 *    order to avoid errors in the tools directory.
 * 
 * * We are currently generating a valid libsafapi library. As more files
 *   are converted to Phase-I please add them to the .c list in the
 *   safapi/lib/Makefile.in. Then regenerate the Makefiles by cd'ing up
 *   to the top of the build directory and doing a `./config.status'
 * 
 * * I'll work on the field.c and rel.c files since they'll use the blob
 *   stuff to store their problem-sized data and that stuff hasn't been
 *   well tested.
 * 
 * * We'll almost certainly have problems with circular references in
 *   persistent objects (a collection points to member sets and sets
 *   point to collections).  The problem is that SSlib can't synchronize
 *   an object that points to unsynchronized objects. I'll work on
 *   resolving that once we have a concrete situation to work from.
 * 
 * * Good places for Eric to work:
 *    - the remaining source files in src/safapi/lib for persistent
 *      objects. You'll see the commented out list in the Makefile.in in
 *      that directory.
 *    - the src/safapi/test directory. Generally I've been doing a file
 *      in the lib directory and then see how many more test cases I can
 *      get to work, going back and forth like that.
 * 
 * * Send e-mail if you're about to work extensively on a particular
 *   source file so we're not duplicating effort. */


/*-------------------------------------------------------------------------------------------------------------------------------
 * Chapter:     SSlib Phase I
 * Section:     Known Bugs
 * Audience:    public
 * Description:
 *      The saf_allgather_handles() function has tighter parallel semantics than before.
 At this
 *        time, a memory resident SAF file has no underlying HDF5 file (not even a "core" file) and therefore is not able to
 *        store raw data.  This may change in the future, but even the old memory resident functionatily had problems when
 *        tasks started doing partial I/O to share data. *-------------------------------------------------------------------------------------------------------------------------------
 */
