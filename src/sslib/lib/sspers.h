/*
 * Copyright(C) 2004-2005 The Regents of the University of California.
 *     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
 *     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
 *     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
 *     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
 *     dissemination, commercialization  through formal licensing, or other  disposition under terms of
 *     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
 *     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
 *     University.
 * 
 * Copyright(C) 2004-2005 Sandia Corporation.
 *     Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive license for use of this work
 *     on behalf of the U.S. Government.  Export  of this program may require a license from the United
 *     States Government.
 * 
 * Disclaimer:
 *     This document was  prepared as an account of  work sponsored by an agency  of  the United States
 *     Government. Neither the United States  Government nor the United States Department of Energy nor 
 *     the  University  of  California  nor  Sandia  Corporation nor any  of their employees  makes any 
 *     warranty, expressed  or  implied, or  assumes   any  legal liability  or responsibility  for the 
 *     accuracy,  completeness,  or  usefulness  of  any  information, apparatus,  product,  or process 
 *     disclosed,  or  represents that its  use would   not infringe  privately owned rights. Reference 
 *     herein  to any  specific commercial  product,  process,  or  service by  trade  name, trademark, 
 *     manufacturer,  or  otherwise,  does  not   necessarily  constitute  or  imply  its  endorsement, 
 *     recommendation, or favoring by the  United States Government   or the University of  California.  
 *     The views and opinions of authors expressed herein do not necessarily state  or reflect those of
 *     the  United  States Government or  the   University of California   and shall  not be  used  for
 *     advertising or product endorsement purposes.
 * 
 * Authors:
 *     Robb P. Matzke              LLNL
 *     Eric A. Illescas            SNL
 * 
 * Acknowledgements:
 *     Mark C. Miller              LLNL - Design input
 * 
 */
#ifndef SS_HAVE_SSPERS_H
#define SS_HAVE_SSPERS_H

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 *
 * Description: A persistent object is anything that typically gets stored in an SSlib database. Examples are sets, fields,
 *              relations, templates thereof, etc.  For each class of persistent object SSlib creates a /link/ datatype, which
 *              is akin to a C pointer in that it's a lightweight piece of data (a small C struct with no dynamically
 *              allocated components). But it's also more than a C pointer because it can point to disk-resident objects, and
 *              the links can be stored in files.  Also like C pointers, the object to which a link points must be
 *              dereferenced in order to get to to the actual object, and SSlib provides macros for doing such. In fact, SSlib
 *              provides three macros for each persistent object class.
 *
 *              * SS_FIELD:   Takes one argument, /field/, of type ss_field_t and dereferences that link to obtain a pointer to
 *                            the C struct that implements the field object.  The returned value is of type ss_fieldobj_t. The
 *                            same pattern is followed for other persistent classes.
 *
 *              * SS_FIELD_M: Just like SS_FIELD except it also takes a member name whose value it returns via C's arrow
 *                            operator.  There really isn't much point in using this macro but it's supplied for completeness.
 *
 *              * SS_FIELD_P: Just like SS_FIELD_M except it returns the address of the member instead of the member value.
 *                            This macro is used most often to obtain a pointer to a link that's stored in some object because
 *                            objects store links but almost all SSlib functions take pointers to links.  It's only supplied
 *                            for completeness because one can do the same thing by using the "address of" (ampersand) C
 *                            operator in front of SS_FIELD_M.
 *
 *              Generally the SAF library will pass around links to objects and dereference the link whenever access to
 *              members of the actual object's struct is necessary. The primary reason for passing around links instead of
 *              object pointers is that it allows SSlib to relocate objects in memory in order to do certain memory management
 *              tasks and optimizations. However, it's not really a big performance penalty to repeatedly dereference links
 *              because the link caches the pointer in such a way that most dereferencing operations will incur only two
 *              memory comparisons. And precisely for that reason it is normal practice to pass non-const-qualified pointers
 *              to links when calling most functions: it allows SSlib library to update the link itself and propagate that
 *              change up through the call stack.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Internals
 *
 * Description: A persistent object is an object that exists in a SAF file as an row (element) of a table (1-dimensional
 *              HDF5 dataset).  File and memory representations of a persistent object are slightly different: the memory
 *              representation contains only native datatypes and contains transient information in addition to those things
 *              stored in the file.  For each persistent object datatype there is also a persistent object link datatype.
 *              For an object class /foo/ the persistent object link type is ss_/foo/_t and there are a variety of datatypes
 *              associated with the object:
 *
 *              * ss_/foo/obj_tm:  A C !struct defining the memory-resident transient information for the persistent object.
 *                                 Since each specific persistent object class is derived from ss_persobj_t, which in turn is
 *                                 derived from ss_obj_t, this struct must be the first member in memory of any persistent object.
 *
 *              * ss_/foo/obj_tf:  A C !struct defining the memory-resident persistent information (i.e., the information that
 *                                 also appears when the object is stored in the file.  This struct is used to machine generate
 *                                 various other datatypes and is not even used directly by any other SSlib code.
 *
 *              * ss_/foo/obj_tfm: The HDF5 datatype (hid_t) equivalent of ss_/foo/obj_tf, machine generated from ss_/foo/obj_tf.
 *
 *              * ss_/foo/obj_tff: The HDF5 datatype (hid_t) for a row of the table (element of the dataset) that stores
 *                                 objects of this type.  This is the default datatype for creating the table, but if the table
 *                                 already exists then its type is used instead. Comparison of this type with the actual
 *                                 table type is how SSlib implements forward/backward file compatibility.
 *
 *              * ss_/foo/obj_t:   The C !struct defining a memory resident instance of the /foo/ class of persistent object. The
 *                                 first member of this struct is named !m and is of type ss_/foo/obj_tm and the remaining members
 *                                 are identical to those of ss_/foo/obj_tf.  This type is machine generated from ss_/foo/obj_tf.
 *
 *              The names are chosen such that a client need only remember the ss_/foo/obj_t version and the rest can be
 *              generated with macros as needed. The ss_/foo/obj_tf types are defined with a special format and support a
 *              limited range of datatypes as members. The following serves as an example:
 *
 *                  typedef struct ss_unit_tf {
 *                      ss_string_t     url;            // URL describing the unit
 *                      ss_string_t     name;           // Official name of the unit
 *                      ss_string_t     abbr;           // Abbreviation for the unit
 *                      // Blank lines and comment-only lines are ignored.
 *                      double          scale;          // Unit scale
 *                      double          offset;         // Unit offset
 *                      double          logbase;        // Logorithm base
 *                      double          logcoef;        // Logorithm coefficient
 *                      ss_quantity_t   quantity;       // Link to a quantity object
 *                      int             power[NQ];      // Signed powers of base quantities
 *                  } ss_unit_tf;
 *
 *              These definitions can appear in any SSlib source file and are parsed by the !sstypegen perl script, which
 *              generates additional files.  The following datatypes are recognized:
 *
 *              * ss_string_t:      SSlib uses a special datatype to store variable-length strings in a persistent object
 *                                  in order to hide the implementation (so we can change it at will for performance
 *                                  reasons) and to make full use of HDF5's string datatype.  The HDF5 datatypes will be
 *                                  opaque types named SSlib:m:string and SSlib:f:string for the memory and file
 *                                  representations, respectively. See [Strings] for details.
 *
 *              * ss_/foo/_t:       A persistent object link to any specific class of object. Since these links contain members
 *                                  that contain runtime information SSlib uses HDF5's opaque datatypes to obtain control over the
 *                                  conversion process. The HDF5 memory opaque datatype is called SSlib:m:ss_/foo/_t while the file
 *                                  opaque datatype is called SSlib:f:ss_/foo/_t.
 *
 *              * !int:             Natively supported by HDF5.
 *
 *              * !unsigned:        Natively supported by HDF5 (a.k.a., !unsigned !int)
 *
 *              * !char:            Natively supported by HDF5 as an integer type, but see ss_string_t above.
 *
 *              * !unsigned !char:  Natively supported by HDF5 as an integer type, but see ss_string_t above.
 *
 *              * !short:           Natively supported by HDF5.
 *
 *              * !unsigned !short: Natively supported by HDF5.
 *
 *              * !long:            Natively supported by HDF5.
 *
 *              * !unsigned !long:  Natively supported by HDF5.
 *
 *              * !float:           Natively supported by HDF5.
 *
 *              * !double:          Natively supported by HDF5.
 *
 *              * hbool_t:          Natively supported by HDF5.
 *
 *              The comments may be any length and are optional. When present, they become part of the column_descriptions
 *              attribute for any dataset that serves as an SSlib table. This attribute is a compound datatype where each
 *              member name is one of the names of a documented member of the ss_/foo/_tf struct and its value is a string just
 *              long enough to hold the comment without the surrounding C beginning and end of comment tokens or white space,
 *              and with all internal white space squeezed down to single SPC characters.
 *
 *              As mentioned above, each persistent object class has a corresponding persistent object link class and the
 *              link is a lightweight C struct that works somewhat like a C pointer. Links come in states described by
 *              ss_pers_link_state_t.
 *
 *              Links are normally dereferenced with macros of the form SS_/FOO/(/link/), as in the following:
 *
 *                  ss_field_t f1 = ...;
 *                  printf("assoc ratio: %d\n", SS_FIELD(f1)->assoc_ratio);
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/* A persistent object */
typedef struct ss_persobj_t {
    ss_obj_t            obj;            /* ss_persobj_t is derived from ss_obj_t. This must be first! */
    size_t              mapidx;         /* maintained by ss_table functions. If the object has a permanent home in the table
                                         * (that home location is identical across all tasks) then a direct index appears
                                         * here. Otherwise the object's indirect index is here. When an object is promoted
                                         * from a temporary home (e.g., a newly created object) to a permanent home (first
                                         * time it is synchronized across tasks) the object will still have an indirect index
                                         * entry in the map portion of the table, but we'll store the direct index in this
                                         * field. */
    hbool_t             dirty;          /* Set if object is locally modified w.r.t. the copy in the file. New objects are born
                                         * dirty. Only task zero of each scope needs to maintain this information since that's
                                         * the only task that will be writing to the file. */
    hbool_t             synced;         /* If true on all tasks then the object is considered to be synchronized across the
                                         * tasks. Set when reading an object into memory and after a synchronization event;
                                         * cleared by object creation and any function that changes an object. However, for
                                         * any object that is "born synchronized" by passing the SS_ALLSAME flag to
                                         * ss_pers_new() we set this to SS_ALLSAME to indicate that `sync_serial' and
                                         * `sync_cksum' have not yet been initialized. */
    hbool_t             marked;         /* A bit to mark graph traversal algorithms such as equality and checksum ops */
    unsigned            sync_serial;    /* The ss_table_synchronize() function sets this member to a serial number that
                                         * identifies that particular ss_table_synchronize() call and serves to mark the
                                         * object as having been considered for synchronization.  This is used by the
                                         * synchronization algorithm to detect when multiple tasks have new data for the same
                                         * direct-index object. */
    ss_val_cksum_t      sync_cksum;     /* This is the expected checksum of the object after the synchronization has been
                                         * completed. It is used by the synchronization algorithm to detect when two or more
                                         * tasks have made conflicting changes to an object. It allows SSlib to check for many
                                         * cases when the client accidently failed to invoke ss_pers_modified() after
                                         * modifying an object. */
    size_t              saf_each;       /* This member is sort of special in that ss_val_compile() automatically includes it
                                         * in the valinfo list (see ss_pers_class_t) yet it's not part of the file or memory
                                         * datatype. SSlib normally combines identical "new" objects into a single permanent
                                         * object during synchronization. By setting this `saf_each' member to a unique value
                                         * SSlib will think that the new objects are different and therefore not combine them.
                                         * It would have been cleaner to just add this `saf_each' member to each ss_*obj_t
                                         * object description below, but then it would have shown up in the database files as
                                         * well and it's not really necessary once the object is saved in the file. */
} ss_persobj_t;

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Internals
 * Purpose:     Persistent object link states
 * Description: These are the possible states in which a persistent object link may exist.
 *
 *              * Filed:  This is the file storage format of a link and is always associated with some particular
 *                        HDF5 file where the link is stored.  Such a link has an index into that file's /File/ table to
 *                        describe the destination file where the pointed-to object is stored; an index into the destination
 *                        file's /Scope/ table; an object magic number; and an index into a persistent object table.  The
 *                        ss_pers_link_state_t enumeration type doesn't have a specific value for this state since the very fact
 *                        that an object link is in a file (or in memory) indicates that the link is in the file state (or not).
 *
 *              * Null:   A link in this state is considered to be a link which doesn't point to any object.  This state is zero
 *                        by definition so that all memory initialized to zero (e.g., statically, memset(), or with one of the
 *                        SS_NULL constants) are in the Null state.
 *
 *              * Closed: A link existing in memory that points to an object in an as yet unopened file or in a file that is
 *                        open but the link has never been dereferenced.  When objects are read from disk SSlib converts links
 *                        in the File state to links in this state.  Such links have an index into the GFile array to
 *                        indicate the destination file but are otherwise the same as a link in the Filed state.
 *
 *              * Memory: Any link that has been dereferenced is moved into the memory state, which causes a pointer to the
 *                        object to be cached in the link.  SSlib can examine this pointer to determine if it's actually the
 *                        object to which the link points, and if not then update the cached pointer with a new one.
 *
 *              * Reserved: This state value is reserved for future use and should never appear in memory.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
typedef enum ss_pers_link_state_t {
    SS_PERS_LINK_NULL=0,
    SS_PERS_LINK_CLOSED=1,
    SS_PERS_LINK_MEMORY=2,
    SS_PERS_LINK_RESERVED=3
} ss_pers_link_state_t;

/* Persistent object links (Closed and Memory states). This type should be as small as reasonably possible. */
typedef struct ss_pers_t {
    ss_obj_t            obj;                    /* Persistent object links are derived from ss_obj_t. This member must be
                                                 * first! */
    ss_pers_link_state_t state;                 /* Link state. Since this is only a two-bit value we could eaily store it
                                                 * in the high-order bits of the `gfileidx' member since it's extremely
                                                 * unlikely that `gfileidx' would use all 32 or 64 bits! */
    size_t              gfileidx;               /* For both closed and memory states this is an index into the GFile array. */
    size_t              open_serial;            /* This is a serial number assigned to the GFile entry at the time the link
                                                 * was created. The serial number here is compared to the one in the GFile
                                                 * entry to determine if the file has been closed and reopened. If it has then
                                                 * the `objptr' is certainly invalid, probably pointing into freed memory. */
    size_t              scopeidx;               /* For both closed and memory states this is an index into the scope table
                                                 * of the destination file in order to determine what scope holds the object
                                                 * in question. */
    size_t              objidx;                 /* For both the closed and memory states This is the index into the table where
                                                 * the object is stored. It can be a direct or indirect index. */
    ss_persobj_t        *objptr;                /* For links in the Memory state this is a pointer to the object candidate.
                                                 * The memory will always be valid (if the indicated file is open) but might
                                                 * be pointing to the wrong table slot if the object has been relocated. We
                                                 * can determine whether it's been moved by looking at the object's mapidx
                                                 * value and comparing it to the `objidx' value here. */
} ss_pers_t;

/*-------------------------------------------------------------------------------------------------------------------------------
 * The following includes only the persistent object link datatypes from the generated header file. We have to do that because
 * some table types have circular references (e.g., a set has a link to a collection and a collection has a link to a set).
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#include "sslinks.h"                    /* include object link datatypes from generated header file */

/* This function gets called after a table of objects is read into memory. */
typedef herr_t (*ss_pers_postread_t)(struct ss_table_t*, ss_scope_t *scope);

/* Class variables for each object class, many initialized by generated code. These are all stored in a global array and
 * indexed by the sequence number part of the various persistent object magic numbers. */
typedef struct ss_pers_class_t {
    const char          *name;          /* name of the persistent object class, e.g., "relrep" for `ss_relrep_t' */
    hid_t               tfm;            /* the ss_/foo/obj_tfm HDF5 datatype */
    hid_t               tff;            /* the ss_/foo/obj_tff HDF5 datatype */
    hid_t               tm;             /* the ss_/foo/_tm HDF5 datatype */
    hid_t               tf;             /* the ss_/foo/_tf HDF5 datatype */
    size_t              t_size;         /* sizeof(ss_/foo/obj_t) */
    ss_val_info_t       *valinfo;       /* run-time compiled type information (see ss_val_compile()) */
    size_t              valinfo_nused;  /* number of used entries in the `valinfo' array */
    MPI_Datatype        serialized;     /* datatype for part of object that can be communicated between tasks */
} ss_pers_class_t;

#define SS_PERS_NCLASSES        20      /* Arbitrary, but must be large enough. See SS_MAGIC_ss_*obj_t dfns in ssobj.h */
extern ss_pers_class_t ss_pers_class_g[SS_PERS_NCLASSES];

#define SS_PERS_CLASS(_sequence_)                                                                                               \
    (_sequence_<SS_PERS_NCLASSES && ss_pers_class_g[_sequence_].name ? ss_pers_class_g + _sequence_ : NULL)

/* The following macros are safer ways to access various fields of an ss_pers_t object. This allows us to optimize the
 * ss_pers_t struct by combining certain members like `state' and `gfileidx' without rewriting lots of code. */
/*DOCUMENTED*/
#define ss_pers_link_state(LNK)         ((LNK)?((const ss_pers_t*)(LNK))->state : SS_PERS_LINK_NULL)
#define ss_pers_link_gfileidx(LNK)      (((const ss_pers_t*)(LNK))->gfileidx)
#define ss_pers_link_openserial(LNK)    (((const ss_pers_t*)(LNK))->open_serial)
#define ss_pers_link_scopeidx(LNK)      (((const ss_pers_t*)(LNK))->scopeidx)
#define ss_pers_link_objidx(LNK)        (((const ss_pers_t*)(LNK))->objidx)
#define ss_pers_link_objptr(LNK)        (((const ss_pers_t*)(LNK))->objptr)

#define ss_pers_link_setstate(LNK,LTYPE) (((ss_pers_t*)(LNK))->state = (LTYPE))
#define ss_pers_link_setgfileidx(LNK,IDX)(((ss_pers_t*)(LNK))->gfileidx = (IDX))
#define ss_pers_link_setopenserial(LNK,N)(((ss_pers_t*)(LNK))->open_serial = (N))
#define ss_pers_link_setscopeidx(LNK,IDX)(((ss_pers_t*)(LNK))->scopeidx = (IDX));
#define ss_pers_link_setobjidx(LNK,IDX)  (((ss_pers_t*)(LNK))->objidx = (IDX))
#define ss_pers_link_setobjptr(LNK,PTR)  (((ss_pers_t*)(LNK))->objptr = (ss_persobj_t*)(PTR))

/* See sspers.c ss_if_init() for how these are defined. */
extern hid_t ss_pers_tm;                /* HDF5 datatype corresponding to ss_pers_t in memory. */
extern hid_t ss_pers_tf;                /* HDF5 datatype for an ss_pers_t in a file. */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Miscellaneous types
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/* For passing information through ss_table_scan for ss_pers_find() */
typedef struct ss_pers_find_t {
    ss_persobj_t        *key;           /* Key describing values for which we're searching */
    ss_persobj_t        *mask;          /* Description of what elements of `key' should be used when searching */
    ss_pers_t           *buffer;        /* Array of returned handles being built up */
    size_t              nused;          /* Number of entries in `buffer' that are currently in use */
    size_t              nalloc;         /* Number of entries allocated in `buffer' */
    size_t              nskip;          /* Treat first NSKIP matches as if they didn't really match. */
    size_t              limit;          /* Do not exceed this many matches */
    hbool_t             detect_overflow;/* If true then look for one additional match beyond the caller-specified maximum */
    hbool_t             overflowed;     /* Set to true when an overflow situation is detected */
    ss_scope_t          *scope;         /* The scope being searched */
} ss_pers_find_t;

#define SS_PERS_TEST ((void*)0x01)      /* Can be passed as the buffer for ss_pers_find() */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Maximum IndexSpec dimensionality
 * Description: This constant represents the maximum number of dimensions that can be described by an index specification.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_MAX_INDEXDIMS        8       /* Maximum dimensionality of an IndexSpec */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Number of base quantities
 * Description: All quantities can be defined in terms of seven basic quantities.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_MAX_BASEQS           7       /* Number of basic quantities */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     SIL roles
 * Description: These are the roles that can appear in a subset inclusion lattice. One additional constant SAF_SROLE_ANY,
 *              although not part of this enumeration due to it never appearing in a file, can be used as a wildcard for
 *              searching.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
typedef enum ss_silrole_t {             /* NOTE: this used to be called VBT_SilRole_t with names like VBT_SROLE_TIME */
    SAF_SROLE_TIME,                     /* sil is a time-base */
    SAF_SROLE_SPACE,                    /* sil is for space */
    SAF_SROLE_STATE,                    /* for state space */
    SAF_SROLE_PARAM,                    /* sil is generic param space */
    SAF_SROLE_CTYPE,                    /* sil is for a cell */
    SAF_SROLE_ATYPE,                    /* sil is for an algebraic type */
    SAF_SROLE_USERD,                    /* sil has a user defined role */
    SAF_SROLE_SUITE                     /* for a suite */
} ss_silrole_t;

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Wildcard SIL role
 * Description: This constant can be used as a wildcard when searching based on SIL role.
 * Also:        ss_silrole_t
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_SROLE_ANY   ((ss_silrole_t)(-1))

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    API
 * Chapter:     Datatypes
 * Purpose:     Cell types
 *
 * Description:	Primitive cell types for collections.
 *
 *              Primitive cells are the most basic entities contained in a set.  
 *
 *		In the description that follows, we must differentiate between /topological/ dimension and /spatial/ dimension.
 *              Topological dimension distinguishes whether a primitive is a volume (3-d), surface (2-d), curve (1-d), or
 *              point (0-d).  Spatial dimension is a feature of the coordinate field (defined by how many components in the
 *              field) of the points used to define the topology of the primitive cells.  A point (0-d topology) may exist in
 *              1-d space (1 spatial coordinate), 2-d space (2 spatial coordinates), or 3-d space (3 spatial coordinates).
 *              Similarly, a curve (1-d topology) may exist in 1-d, 2-d, or 3-d space, and a surface (2-d topology) may exist
 *              in 2-d or 3-d space.
 *
 *		Primitive cells can be fully characterized by the following features:
 *
 *		* Shape (e.g., hexagon, triangle, etc).  This inherently includes the topological dimension, but /not/ the spatial
 *		  dimension.  For example, a hexagon is always a 3-d topology, a triangle is always a 2-d topology.
 *
 *		* Number of nodes to define the cell topology.  This defines the "order" (linear, quadratic, etc.) of the
 *		  coordinate field of the cells.
 *
 *		* Node, edge, and face permutation vectors that describe a SAF client's node, edge, and face numbering relative 
 *		  to a canonical representation which is PATRAN's numbering scheme
 *
 *		* Location and orientation of a local (cell) coordinate system
 *
 *		All of these features will be user-definable once the SAF "type registry" is fully implemented.  Until then,
 *		only the shape (e.g., hex, tet, tri, etc.) can be specified by a client.  The other features must be assumed
 *		to be the following:
 *
 *		* Number of nodes is the minimum number to define the shape.  Therefore, only linear cells can be specified.
 *
 *		* Node, edge, and face permutation vectors are assumed to be identity.  Therefore, node, edge, and face
 *		  numbering must match the canonical representation
 *
 *		* The local coordinate system must match that specified in the canonical representation
 *
 *		Finally, please see [Constants] for some usage guidelines.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef enum ss_celltype_t {            /* NOTE: this used to be called VBT_CellType_t with names like VBT_CELL_TYPE_NONE */
    SAF_CELLTYPE_SET,                   /* Use this cell type for collections that do not contain primitive sets (cells). */
    SAF_CELLTYPE_POINT,                 /* A 0d point.  See $SAFDOC/registry/cell_types.html#POINT */
    SAF_CELLTYPE_LINE,                  /* A 1d line bounded by 2 points. See $SAFDOC/registry/cell_types.html#LINE */
    SAF_CELLTYPE_TRI,                   /* A 2d triangle bounded by 3 edges. See $SAFDOC/registry/cell_types.html#TRI */
    SAF_CELLTYPE_QUAD,                  /* A 2d quadrilateral bounded by 4 edges. See $SAFDOC/registry/cell_types.html#QUAD */
    SAF_CELLTYPE_TET,                   /* A 3d tetrahedron bounded by 4 faces. See $SAFDOC/registry/cell_types.html#TET */
    SAF_CELLTYPE_PYRAMID,               /* A 3d pyramid bounded by 5 faces. See $SAFDOC/registry/cell_types.html#PYRAMID */
    SAF_CELLTYPE_PRISM,                 /* A 3d prism or wedge bounded by 5 faces. See $SAFDOC/registry/cell_types.html#PRISM */
    SAF_CELLTYPE_HEX,                   /* A 3d hexahedron bounded by 6 faces. See $SAFDOC/registry/cell_types.html#HEX */
    SAF_CELLTYPE_MIXED,                 /* Heterogeneous cell collection. See $SAFDOC/registry/cell_types.html#MIXED */
    SAF_CELLTYPE_ARB,                   /* Arbitrary, defined by face list. See $SAFDOC/registry/cell_types.html#ARB */
    SAF_CELLTYPE_1BALL,                 /* A "ball" in R1 (line). See $SAFDOC/registry/cell_types.html#1BALL */
    SAF_CELLTYPE_2BALL,                 /* A "ball" in R2 (filled circle). See $SAFDOC/registry/cell_types.html#2BALL */
    SAF_CELLTYPE_3BALL,                 /* A "ball" in R3 (filled sphere). See $SAFDOC/registry/cell_types.html#3BALL */
    SAF_CELLTYPE_1SHELL,
    SAF_CELLTYPE_2SHELL
} ss_celltype_t;
#define SAF_CELLTYPE_NONE SAF_CELLTYPE_SET

typedef enum ss_relkind_t {             /* NOTE: this used to be called VBT_RelKind_t with names like VBT_RELKIND_SUBSET */
    SAF_RELKIND_SUBSET,
    SAF_RELKIND_SUPSET,
    SAF_RELKIND_BOUND,
    SAF_RELKIND_PERMUTE,
    SAF_RELKIND_NEIGHBOR,
    SAF_RELKIND_COPY,
    SAF_RELKIND_EQUAL
} ss_relkind_t;

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    API
 * Chapter:     Datatypes
 * Purpose:     Field component interleave modes
 *
 * Description:	When fields have multiple components, the components can be stored in the field's blob in different ways
 *		relative to each other. For example, in a 3D coordinate field, we will have 3 components for the x, y and
 *		z components of each coordinate. These can be stored as three different component fields or as a single
 *		composite field. If they are stored as a single composite field, they may be stored interleaved or 
 *		non-interleaved.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef enum ss_interleave_t {          /* NOTE: this used to be called VBT_Interleave_t with names like VBT_INTERLEAVE_NONE */
    SAF_INTERLEAVE_COMPONENT,           /* Field is stored as composite, e.g., xxx...yyy...zzz... */
    SAF_INTERLEAVE_VECTOR,              /* Field is stored as composite, e.g., xyz...xyz...xyz... */
    SAF_INTERLEAVE_INDEPENDENT,         /* Field is stored as separate components */
    SAF_INTERLEAVE_NONE                 /* Field is a component field. E.g., there are no components */
} ss_interleave_t;

typedef enum ss_indextype_t {           /* NOTE: this used to be called VBT_IndexType_t with names like VBT_INDEX_TYPE_C_ORDER */
    SAF_INDEXTYPE_C,
    SAF_INDEXTYPE_FORTRAN
} ss_indextype_t;

typedef enum ss_basequant_t {
   SAF_BASEQ_TIME=0,
   SAF_BASEQ_MASS=1,
   SAF_BASEQ_CURRENT=2,
   SAF_BASEQ_LENGTH=3,
   SAF_BASEQ_LIGHT=4,
   SAF_BASEQ_TEMP=5,
   SAF_BASEQ_AMOUNT=6
} ss_basequant_t;

/*-------------------------------------------------------------------------------------------------------------------------------
 * These are here only temporarily until I get them fixed.  --rpm 2003-07-31
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*--------------------------------------*/

/*-------------------------------------------------------------------------------------------------------------------------------
 * The following typedefs are for the various table definitions.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

typedef struct ss_indexspecobj_tm {     /* Memory-resident transient information */
    ss_persobj_t        pers;           /* Must be first! */
    hid_t               data_type;
} ss_indexspecobj_tm;

typedef struct ss_indexspecobj_tf {     /* Memory-resident persistent information */
    ss_string_t         name;           /* Indexing specification  descriptive name */
    int                 ndims;          /* Number of entries used in various arrays herein */
    int                 origins[SS_MAX_INDEXDIMS];/* Should probably be `hsize_t' instead of `int' */
    int                 sizes[SS_MAX_INDEXDIMS];/* Should probably be `hsize_t' instead of `int' */
    int                 order[SS_MAX_INDEXDIMS];
    ss_indextype_t      index_type;     /* C or Fortran order */
    hbool_t             is_explicit;    /* If set then indices have enumerated values */
    hbool_t             is_sorted;      /* Set if the indices are sorted */
    hbool_t             is_compact;     /* Set if there are no holes in the index values */
    ss_blob_t           blob;           /* The enumerated index values */
    ss_collection_t     coll;           /* The collection that owns this index spec */
} ss_indexspecobj_tf;

/*--------------------------------------*/
typedef struct ss_roleobj_tm {          /* Memory-resident transient information */
    ss_persobj_t        pers;           /* Must be first! */
} ss_roleobj_tm;

typedef struct ss_roleobj_tf {          /* Memory-resident persistent information */
    ss_string_t         name;           /* Role descriptive name */
    ss_string_t         url;            /* Optional full description */
} ss_roleobj_tf;

/*--------------------------------------*/
typedef struct ss_basisobj_tm {         /* Memory-resident transient information */
    ss_persobj_t        pers;           /* Must be first! */
} ss_basisobj_tm;

typedef struct ss_basisobj_tf {         /* Memory-resident persistent information */
    ss_string_t         name;           /* Basis descriptive name */
    ss_string_t         url;            /* Optional full description */
} ss_basisobj_tf;

/*--------------------------------------*/
typedef struct ss_algebraicobj_tm {     /* Memory-resident transient information */
    ss_persobj_t        pers;           /* Must be first! */
} ss_algebraicobj_tm;

typedef struct ss_algebraicobj_tf {     /* Memory-resident persistent information */
    ss_string_t         name;           /* Algebraic descriptive name */
    ss_string_t         url;            /* Optional full description */
    hbool_t             indirect;
} ss_algebraicobj_tf;

/*--------------------------------------*/
typedef struct ss_evaluationobj_tm {    /* Memory-resident transient information */
    ss_persobj_t        pers;           /* Must be first! */
} ss_evaluationobj_tm;

typedef struct ss_evaluationobj_tf {    /* Memory-resident persistent information */
    ss_string_t         name;           /* Evaluation descriptive name */
    ss_string_t         url;            /* Optional full description */
} ss_evaluationobj_tf;

/*--------------------------------------*/
typedef struct ss_relrepobj_tm {        /* Memory-resident transient information */
    ss_persobj_t        pers;           /* Must be first! */
} ss_relrepobj_tm;

typedef struct ss_relrepobj_tf {        /* Memory-resident persistent information */
    ss_string_t         name;           /* Relation representation descriptive name */
    ss_string_t         url;            /* Optional full description */
    int                 id;             /* One of the ID numbers from SAFrelrep.h */
} ss_relrepobj_tf;

/*--------------------------------------*/
typedef struct ss_quantityobj_tm {      /* Memory-resident transient information */
    ss_persobj_t        pers;           /* Must be first! */
} ss_quantityobj_tm;

typedef struct ss_quantityobj_tf {      /* Memory-resident persistent information */
    ss_string_t         name;           /* Quantity descriptive name */
    ss_string_t         url;            /* Optional full description */
    ss_string_t         abbr;           /* Optional abbreviation or alternate name */
    unsigned int        flags;          /* Various bit flags */
    int                 power[SS_MAX_BASEQS]; /* Powers for the seven basic quantities */
} ss_quantityobj_tf;

/*--------------------------------------*/
typedef struct ss_unitobj_tm {          /* Memory-resident transient information */
    ss_persobj_t        pers;           /* Must be first! */
} ss_unitobj_tm;

typedef struct ss_unitobj_tf {          /* Memory-resident persistent information */
    ss_string_t         name;           /* Unit of measure descriptive name */
    ss_string_t         url;            /* Optional full description */
    ss_string_t         abbr;           /* Optional abbreviation or alternate name */
    double              scale;          /* Scale with respect to base unit for this quantity */
    double              offset;         /* Offset with respect to base unit for this quantity */
    double              logbase;        /* Logarithm base */
    double              logcoef;        /* Logarithm coefficient */
    ss_quantity_t       quant;          /* Quantity which this unit measures */
} ss_unitobj_tf;

/*--------------------------------------*/
typedef struct ss_catobj_tm {           /* Memory-resident transient information */
    ss_persobj_t        pers;           /* Must be first! */
} ss_catobj_tm;

typedef struct ss_catobj_tf {           /* Memory-resident persistent information */
    ss_string_t         name;           /* Collection category descriptive name; "self" implies self category */
    ss_role_t           role;           /* Link to a role */
    int                 tdim;           /* Topological dimensionality */
} ss_catobj_tf;

/*--------------------------------------*/
typedef struct ss_collectionobj_tm {    /* Memory-resident transient information */
    ss_persobj_t        pers;           /* Must be first! */
} ss_collectionobj_tm;

typedef struct ss_collectionobj_tf {    /* Memory-resident persistent information */
    ss_set_t            containing_set; /* Set containing this collection */
    ss_cat_t            cat;            /* Collection category */
    ss_celltype_t       cell_type;
    int                 count;          /* Is this the number of elements in the `members' array? [rpm 2004-06-04] */
    ss_indexspec_t      *indexing;      /* The first is the default; remaining are optional alternate index specifications */
    hbool_t             is_decomp;
    ss_set_t            *members;       /* Variable length array of sets (used to be called the `members_blob_id' DSL metablob) */
} ss_collectionobj_tf;

/*--------------------------------------*/
typedef struct ss_setobj_tm {           /* Memory-resident transient information */
    ss_persobj_t        pers;           /* Must be first! */
} ss_setobj_tm;

typedef struct ss_setobj_tf {           /* Memory-resident persistent information */
    ss_string_t         name;           /* Set descriptive name */
    int                 user_id;
    int                 tdim;           /* Topological dimensionality */
    ss_silrole_t        srole;          /* An enumeration type */
    hbool_t             is_top;         /* Is this a top set? */
    hbool_t             is_extendible;  /* Is this set extendible? */
    ss_field_t          dflt_coordfld;  /* Default coordinate field */
    ss_set_t            bnd_set;
    ss_collection_t     *colls;         /* The collection associated with each category (the collection points to the category) */
} ss_setobj_tf;

/*--------------------------------------*/
typedef struct ss_relobj_tm {           /* Memory-resident transient information */
    ss_persobj_t        pers;           /* Must be first! */

    /* Buffer info set by saf_declare_subset_relation() or saf_declare_topo_relation() and cleared by
     * saf_write_subset_relation() and saf_write_topo_relation() */
    void                *abuf;
    size_t              abuf_size;      /* Number of elements in `abuf' */
    hid_t               abuf_type;      /* Datatype of each element in `abuf' */
    void                *bbuf;
    size_t              bbuf_size;      /* Number of elements in `bbuf' */
    hid_t               bbuf_type;      /* Datatype of each element in `bbuf' */
} ss_relobj_tm;

typedef struct ss_relobj_tf {           /* Memory-resident persistent information */
    ss_set_t            sub;            /* subset */
    ss_cat_t            sub_cat;        /* subset collection category */
    ss_cat_t            sub_decomp_cat;
    ss_set_t            sup;            /* superset */
    ss_cat_t            sup_cat;        /* superset collection category */
    ss_cat_t            sup_decomp_cat;
    ss_relkind_t        kind;           /* An enumeration type */
    ss_relrep_t         rep_type;
    ss_blob_t           d_blob;         /* Domain data (b_buf)*/
    ss_blob_t           r_blob;         /* Range data (a_buf)*/
    ss_rel_t            *indirect_rels; /* Indirect relations instead of d_blob and r_blob; causes r_blob to be ignored */
} ss_relobj_tf;

/*--------------------------------------*/
typedef struct ss_fieldtmplobj_tm {     /* Memory-resident transient information */
    ss_persobj_t        pers;           /* Must be first! */

    /* The following fields are set by saf_declare_field() and cleared by saf_write_field() */
    size_t              nbufs;          /* Number of buffers pointed to by bufs[] */
    size_t              buf_size;       /* Number of elements in each buffer pointed to by buf[i] */
    hid_t               buf_type;       /* Datatype of each element of each buf[i] buffer */
    void                **bufs;         /* Array of `nbufs' pointers to arrays of values of type `buf_type' */

} ss_fieldtmplobj_tm;

typedef struct ss_fieldtmplobj_tf {     /* Memory-resident persistent information */
    ss_string_t         name;           /* Field template descriptive name */
    ss_algebraic_t      algebraic;
    ss_basis_t          basis;
    ss_quantity_t       quantity;
    int                 num_comps;      /* Signed to hold `-1' in special circumstances */
    ss_field_t          *ftmpls;        /* Used to be called `ftmpl_ids_blob' */
} ss_fieldtmplobj_tf;

/*--------------------------------------*/
typedef struct ss_fieldobj_tm {         /* Memory-resident transient information */
    ss_persobj_t        pers;           /* Must be first! */
    int                 nbufs;          /* Provided by saf_declare_field() or saf_write_field() */
    int                 buf_size;       /* Provided by saf_declare_field() or saf_write_field() */
    void                **bufs;         /* Provided by saf_declare_field() or saf_write_field() */
    hid_t               data_type;      /* Provided by saf_declare_field() or saf_write_field() */
    ss_field_t          parent;         /* Cached parent of a composit field, or self */
} ss_fieldobj_tm;

typedef struct ss_fieldobj_tf {         /* Memory-resident persistent information */
    ss_string_t         name;           /* Field descriptive name */
    ss_fieldtmpl_t      ftmpl;
    ss_set_t            base_space;
    ss_unit_t           units;
    ss_interleave_t     comp_intlv;     /* An enumeration type */
    ss_indexspec_t      indexing;
    ss_cat_t            dof_assoc_cat;  /* Category for collection with which dofs are n:1 associated */
    ss_cat_t            storage_decomp_cat;/* Category for the collection on which an indirect field is stored */
    int                 assoc_ratio;
    ss_cat_t            eval_decomp_cat;
    ss_evaluation_t     evaluation;
    hbool_t             is_homogeneous;
    hbool_t             is_coord_field;
    ss_field_t          *comp_fields;   /* List of component fields if this is an indirect field (was comp_ids_blob_id) */
    ss_blob_t           comp_ids_blob;
    ss_blob_t           composite_parent_blob;
    int                 *comp_order;    /* Optional permutation vector for `comp_fields' (was comp_order_blob_id) */
    ss_blob_t           vbasis_blob;
    ss_blob_t           dof_blob;       /* The actual degrees of freedom */
    ss_field_t          *indirect_fields; /* Indirect fields instead of DOFs */
} ss_fieldobj_tf;

/*--------------------------------------*/
typedef struct ss_topsobj_tm {          /* Memory-resident transient information */
    ss_persobj_t        pers;           /* Must be first! */
} ss_topsobj_tm;

typedef struct ss_topsobj_tf {          /* Memory-resident persistent information */
    ss_pers_t           top;            /* points to either a set or a field */
    int                 set_or_field;   /* determines what `top' points to? !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
} ss_topsobj_tf;

/* Types generated from above declarations */
#include "ssperstab.h"

/* Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Destructor
 * Description: This is simply a convenience function for ss_pers_dest() so that the caller doesn't have to cast the argument
 *              and return value. The underlying object is not destroyed--only the link to that object.
 * Return:      Always returns null so it can be easily assigned to the object link being destroyed.
 * Example:     field = SS_OBJ_DEST(field); */
#define SS_PERS_DEST(_pers_) (ss_pers_dest((ss_pers_t*)_pers_), SS_STATUS_OK, NULL)

/* Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Constructor
 * Description: This is simply a convenience function for ss_pers_new() so that the caller doesn't have to cast the argument
 *              and return value. The _TYPE_ should be one of the persistent object link datatypes like ss_field_t, and not
 *              one of the persistent object types like ss_fieldobj_t.
 * Return:      Returns a pointer to a link to the new object on success; null on failure.
 * Example:     To create a new Field object:
 *                 field = SS_PERS_NEW(scope, ss_field_t, SS_ALLSAME); */
#define SS_PERS_NEW(_scope_,_type_,_flags_) ((_type_*)ss_pers_new(_scope_,SS_MAGIC(_type_),NULL,_flags_,NULL,NULL))

/* Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Copy constructor
 * Description: This is simply a convenience function for ss_pers_copy() so that the caller doesn't have to cast the arguments
 *              and return value.
 * Return:      Returns a pointer to a link to the new object on success; null on failure. */
#define SS_PERS_COPY(_old_,_scope_,_flags_) ((void*)ss_pers_copy((ss_pers_t*)(_old_), _scope_, _flags_, NULL, NULL))

/* Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Find objecs in a scope
 * Description: This is simply a convenence function for ss_pers_find() so that the caller doesn't have to cast the arguments
 *              to ss_pers_t pointers (they'll still be run-time type checked).
 * Return:      See ss_pers_find()
 * Parallel:    See ss_pers_find() */
#define SS_PERS_FIND(scope, key, mask, limit, nfound)                                                                          \
    (((nfound)=(limit)), (void*)ss_pers_find((scope), (ss_pers_t*)(key), (ss_persobj_t*)(mask), 0, &(nfound), NULL, NULL))

/* Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Determine link equality
 * Description: This macro returns true if two links point to the same object. Arguments are cast appropriately.
 * Return:      True (positive) if same object, false if different, negative on error.
 * Parallel:    Independent */
#define SS_PERS_EQ(link1, link2) ss_pers_eq((ss_pers_t*)link1, (ss_pers_t*)link2)

/* Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Determine object equality
 * Description: This macro returns true if two links point to equivalent objects. Arguments are cast appropriately.
 * Return:      True (positive) if objects are equal, false if not equal, negative on error.
 * Parallel:    Independent. */
#define SS_PERS_EQUAL(link1,link2) ss_pers_equal((ss_pers_t*)link1, (ss_pers_t*)link2, NULL)

/* Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Mark object as modified
 * Description: This macro is simply a wrapper around ss_pers_modified() so the caller doesn't have to cast arguments.
 * Return:      Returns non-negative on success; negative on failure.
 * Parallel:    Independent */
#define SS_PERS_MODIFIED(_pers_,_flags_) ss_pers_modified((ss_pers_t*)(_pers_), (_flags_))

/* Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Check if link is null
 * Description: A null persistent object link is indicated by the link being in the SS_PERS_LINK_NULL state.
 * Return:      Returns true if the specified link a null pointer or is in the null state; false otherwise.
 * Parallel:    Independent */
#define SS_PERS_ISNULL(_pers_) (SS_PERS_LINK_NULL==ss_pers_link_state(_pers_))

/* Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Make an object unique
 * Description: Makes an object unique by giving it a unique serial number. The number is unique across all the MPI tasks so
 *              that when N tasks create N identical new objects that only differ in serial number, SSlib will convert them to
 *              N identical permanent objects instead of merging them all into a single permanent object.
 * Return:      Returns non-negative on success; negative on failure.
 * Parallel:    Independent */
#define SS_PERS_UNIQUE(_pers_) ss_pers_unique((ss_pers_t*)(_pers_))

/*-------------------------------------------------------------------------------------------------------------------------------
 * Function prototypes for sspers.c
 *-------------------------------------------------------------------------------------------------------------------------------
 */
extern const ss_pers_t SS_PERS_NULL;

#ifdef __cplusplus
extern "C" {
#endif

herr_t ss_perstab_init(void); /*to initialize machine generated stuff*/

herr_t ss_pers_init(void);
ss_pers_t *ss_pers_new(ss_scope_t *scope, unsigned tableid, const ss_persobj_t *init, unsigned flags, ss_pers_t *buf,
                       ss_prop_t *props);
ss_pers_t *ss_pers_copy(ss_pers_t *pers, ss_scope_t *scope, unsigned flags, ss_pers_t *buf, ss_prop_t *props);
herr_t ss_pers_reset(ss_pers_t *pers, unsigned flags);
herr_t ss_pers_reset_(ss_persobj_t *persobj, unsigned flags);
herr_t ss_pers_dest(ss_pers_t *pers);
ss_persobj_t *ss_pers_deref(ss_pers_t *pers);
herr_t ss_pers_update(ss_pers_t *pers);
ss_pers_t *ss_pers_refer(ss_scope_t *scope, ss_persobj_t *persobj, ss_pers_t *pers);
ss_pers_t *ss_pers_refer_c(ss_scope_t *scope, unsigned tableidx, size_t itemidx, ss_pers_t *pers);
ss_pers_t *ss_pers_refer_topscope(size_t gfileidx, ss_persobj_t *scopeobj);
ss_scope_t *ss_pers_scope(ss_pers_t *pers, ss_scope_t *buf);
ss_scope_t *ss_pers_topscope(ss_pers_t *pers, ss_scope_t *buf);
ss_file_t *ss_pers_file(ss_pers_t *pers, ss_file_t *file);
htri_t ss_pers_eq(ss_pers_t *pers1, ss_pers_t *pers2);
htri_t ss_pers_equal(ss_pers_t *pers1, ss_pers_t *pers2, ss_prop_t *props);
htri_t ss_pers_iswritable(ss_pers_t *pers);
herr_t ss_pers_state(ss_pers_t *pers, ss_pers_link_state_t state);
int ss_pers_cmp(ss_pers_t *p1, ss_pers_t *p2, const ss_persobj_t *mask);
int ss_pers_cmp_(ss_persobj_t *p1, ss_persobj_t *p2, const ss_persobj_t *mask);
herr_t ss_pers_cksum(ss_persobj_t *pers, ss_val_cksum_t *cksum);
ss_pers_t *ss_pers_find(ss_scope_t *scope, ss_pers_t *key, ss_persobj_t *mask, size_t nskip, size_t *nfound, ss_pers_t *buffer,
                        ss_prop_t *props);
herr_t ss_pers_modified(ss_pers_t *pers, unsigned flags);
herr_t ss_pers_unique(ss_pers_t *pers);
herr_t ss_pers_debug(ss_pers_t *pers);
herr_t ss_pers_dump_(ss_persobj_t *persobj, ss_scope_t *scope, FILE *out, const char *prefix, const char *fmt, ...);
herr_t ss_pers_dump(ss_pers_t *pers, FILE *out, const char *prefix, const char *fmt, ...);
herr_t ss_pers_dumpv(ss_pers_t *pers, FILE *out, const char *prefix, const char *fmt, va_list ap);
herr_t ss_pers_convert_mf(hid_t src_type, hid_t dst_type, H5T_cdata_t *cdata, size_t nelmts, size_t buf_stride,
                          size_t bkg_stride, void *buf, void *bkg, hid_t dxpl);
herr_t ss_pers_convert_mf2(ss_pers_t *src, void *_dst, struct ss_table_t *filetab);
herr_t ss_pers_convert_fm(hid_t src_type, hid_t dst_type, H5T_cdata_t *cdata, size_t nelmts, size_t buf_stride,
                          size_t bkg_stride, void *buf, void *bkg, hid_t dxpl);
herr_t ss_pers_convert_fm2(void *_src, ss_pers_t *dst, struct ss_table_t *filetab);
herr_t ss_pers_sum_cb(void *buffer, size_t size, size_t nelmts, ss_val_cksum_t *cksum);
int ss_pers_cmp_cb(void *_buf, void *_key, unsigned UNUSED flags, size_t size, size_t nelmts);
char *ss_pers_encode_cb(void *buffer, char *serbuf, size_t *serbuf_nused, size_t *serbuf_nalloc,
                        size_t size, size_t nelmts);
size_t ss_pers_decode_cb(void *buffer, const char *serbuf, size_t size, size_t nelmts);

#ifdef __cplusplus
}
#endif

#endif /*!SS_HAVE_SSPERS_H*/

