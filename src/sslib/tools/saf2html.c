/* Copyright(C) 2004-2005 The Regents of the University of California.
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
 */
#include <sslib.h>

static ss_file_t *open_file(const char *name, ss_scope_t *scope, ss_pers_t *pers);
static int dump_headers(ss_pers_t *obj);
static int dump_object(ss_pers_t *obj);
static htri_t table_scan(size_t UNUSED itemidx, ss_pers_t *pers, ss_persobj_t UNUSED *persobj, void UNUSED *udata);
static herr_t print_table(ss_scope_t *scope, unsigned tableidx);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     saf2html
 * Purpose:     Prints part of a SAF database as HTML
 *
 * Description: The command-line arguments are parsed and the specified persistent objects are output as HTML. Command line
 *              arguments have the following formats:
 *
 *              * File: the name of the HDF5 file containing the SAF database.
 *
 *              * Scope: the name of the scope within the database
 *
 *              * Type: The optional name of the object type such as `field', `set', `role', `scope', etc.  If omitted then
 *                      the following Index must also be omitted and all objects in the specified scope are printed.
 *
 *              * Index: An optional index (non-negative decimal number) of the object to be printed. If omitted then all
 *                       objects of the specified class are printed.
 *
 *              Examples, assuming a file named test/file.saf:
 *
 *                test/file.saf/SAF/field/0         -- the first field defined in the top-level scope
 *                test/file.saf/proc/0/set/1    -- set #1 in the `proc/0' scope
 *
 * Parallel:    Serial application
 *
 * Programmer:  Robb Matzke
 *              Monday, May 31, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
    ss_file_t           *file=NULL;
    ss_scope_t          scope=SS_SCOPE_NULL;
    ss_pers_t           obj=SS_PERS_NULL;
    ss_table_t          *table=NULL;
    unsigned            tableidx;

#ifdef HAVE_PARALLEL
    MPI_Init(&argc, &argv);
#endif

    ss_init(SS_COMM_WORLD);

    assert(argc>1);
    if (NULL==(file=open_file(argv[1], &scope, &obj))) exit(1);

    printf("<html><body>");

    /* This is mostly here just for temporary debugging. [rpm 2004-06-04] */
    if (!SS_PERS_ISNULL(&obj)) {
        printf("<code><pre>");
        ss_pers_dump(&obj, stdout, "", NULL);
        printf("</pre></code>");
    }
    

    if (!SS_PERS_ISNULL(&obj)) {
        printf("<table border=1>");
        dump_headers(&obj);
        dump_object(&obj);
        printf("</table>");
    } else if (SS_MAGIC_OF(&obj)) {
        printf("<table border=1>");
        dump_headers(&obj);
        table = ss_scope_table(&scope, SS_MAGIC_OF(&obj), NULL);
        ss_table_scan(table, &scope, (size_t)0, table_scan, NULL);
        printf("</table>");
    } else {
        print_table(&scope, SS_MAGIC(ss_scope_t));
        print_table(&scope, SS_MAGIC(ss_file_t));
        for (tableidx=0; tableidx<SS_PERS_NCLASSES; tableidx++) {
            if (tableidx==SS_MAGIC_SEQUENCE(SS_MAGIC(ss_scope_t))) continue;
            if (tableidx==SS_MAGIC_SEQUENCE(SS_MAGIC(ss_file_t))) continue;
            print_table(&scope, tableidx);
        }
    }
    
    printf("</body></html>\n");

    ss_finalize();
    H5close();
#ifdef HAVE_PARALLEL
    MPI_Finalize();
#endif

    exit(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     saf2html
 * Purpose:     Print an entire table
 *
 * Description: 
 *
 * Return:      
 *
 * Parallel:    
 *
 * Programmer:  Robb Matzke
 *              Monday, July 19, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static herr_t
print_table(ss_scope_t *scope, unsigned tableidx)
{
    ss_table_t          *table=NULL;
    ss_persobj_t        *objptr=NULL;
    ss_pers_t           obj=SS_PERS_NULL;
    ss_pers_class_t     *pc=SS_PERS_CLASS(SS_MAGIC_SEQUENCE(tableidx));

    if (!pc) return -1;
    if (NULL==(table=ss_scope_table(scope, tableidx, NULL))) return -1;
    H5E_BEGIN_TRY {
        objptr = ss_table_lookup(table, 0, SS_STRICT);
    } H5E_END_TRY;
    if (table && objptr) {
        ss_pers_refer(scope, objptr, &obj);
        printf("<h1>%s</h1>", pc->name);
        printf("<table border=1>");
        dump_headers(&obj);
        ss_table_scan(table, scope, (size_t)0, table_scan, NULL);
        printf("</table>");
    }
    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     saf2html
 * Purpose:     Open the file, scope, and/or object
 *
 * Description: Opens the specified object, its scope, and its file.
 *
 * Return:      On success, returns a link to the file which was opened and returns the object and its scope by reference.
 *              Returns the null pointer on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, May 31, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static ss_file_t *
open_file(const char *name,             /* Complete name of the object */
          ss_scope_t *scope,            /* OUT: The scope to which the object belongs */
          ss_pers_t *pers               /* OUT: The object that is opened */
          )
{
    const char          *index_s, *class_s, *slash;
    size_t              index, nfound;
    char                file_name[1024], scope_name[1024], class_name[64], *rest;
    ss_pers_class_t     *pc=NULL;
    int                 i;
    ss_file_t           *file=NULL, *tfile=NULL;
    ss_scope_t          topscope, key, tfiletop;
    ss_scopeobj_t       mask;
    unsigned            magic;
    
    /* The name should end with a slash followed by a positive number */
    if (NULL==(index_s=strrchr(name, '/'))) {
        fprintf(stderr, "malformed object name: no object index\n");
        return NULL;
    }
    if (isdigit(index_s[1])) {
        index = strtol(index_s+1, &rest, 0);
        if (*index_s && *rest) {
            fprintf(stderr, "malformed object name: mangled object index\n");
            return NULL;
        }
    } else {
        index = SS_NOSIZE;
        index_s = name + strlen(name); /*the NUL character*/
    }

    /* The component before the index should be an optional class name */
    if (name==index_s) {
        fprintf(stderr, "malformed object name: no class\n");
        return NULL;
    }
    for (class_s=index_s-1; class_s>=name; --class_s)
        if ('/'==*class_s) break;
    if ('/'!=*class_s) {
        fprintf(stderr, "malformed object name: no class\n");
        return NULL;
    }
    assert((size_t)(index_s-(class_s+1)) < sizeof class_name);
    memcpy(class_name, class_s+1, (size_t)(index_s-(class_s+1)));
    class_name[index_s-(class_s+1)] = '\0';
    for (i=0; i<SS_PERS_NCLASSES; i++) {
        pc = SS_PERS_CLASS(i);
        magic = SS_MAGIC_CONS(SS_MAGIC(ss_pers_t), i);
        if (pc && pc->name && !strcmp(pc->name, class_name)) break;
        pc = NULL;
    }
    if (!pc) {
        if (SS_NOSIZE!=index) {
            fprintf(stderr, "malformed object name: no class\n");
            return NULL;
        }
        class_name[0] = '\0';
        magic = 0;
        class_s = index_s;
    }

    /* The componenents before the class are the combined file name and scope name */
    for (slash=strchr(name, '/'); slash; slash=strchr(slash+1, '/')) {
        size_t file_chars=slash-name, scope_chars=class_s-(slash+1);
        assert(file_chars<sizeof file_name);
        memcpy(file_name, name, file_chars);
        file_name[file_chars] = '\0';
        assert(scope_chars<sizeof scope_name);
        memcpy(scope_name, slash+1, scope_chars);
        scope_name[scope_chars] = '\0';
        H5E_BEGIN_TRY {
            file = ss_file_open(NULL, file_name, H5F_ACC_RDONLY, NULL);
        } H5E_END_TRY;
        if (file) break;
    }
    if (!slash) {
        fprintf(stderr, "no file found\n");
        return NULL;
    }
    ss_file_topscope(file, &topscope);

    /* Open a temporary file to hold the scope key */
    if (NULL==(tfile=ss_file_create("/SAF:saf2html", H5F_ACC_TRANSIENT, NULL))) {
        fprintf(stderr, "cannot create temporary file");
        return NULL;
    }
    ss_file_topscope(tfile, &tfiletop);

    /* Create the scope key and mask */
    ss_pers_new(&tfiletop, SS_MAGIC(ss_scope_t), NULL, SS_ALLSAME, (ss_pers_t*)&key, NULL);
    ss_string_set(SS_SCOPE_P(&key,name), scope_name);
    memset(&mask, 0, sizeof mask);
    memset(&(mask.name), SS_VAL_CMP_DFLT, 1);
    
    /* Find the scope with the specified name */
    nfound=1;
    if (NULL==ss_pers_find(&topscope, (ss_pers_t*)&key, (ss_persobj_t*)&mask, 0, &nfound, (ss_pers_t*)scope, NULL) ||
        0==nfound) {
        fprintf(stderr, "no scope found named: %s\n", scope_name);
        return NULL;
    }

    /* Open the scope */
    if (strcmp(scope_name, "SAF") && ss_scope_open(scope, H5F_ACC_RDONLY, NULL)<0) {
        fprintf(stderr, "cannot open scope: %s\n", scope_name);
        return NULL;
    }

    /* Create a link to the object */
    if (SS_NOSIZE==index) {
        *pers = SS_PERS_NULL;
        pers->obj.magic = magic;
    } else {
        ss_pers_refer_c(scope, SS_MAGIC_SEQUENCE(magic), index, pers);
        if (!ss_pers_deref(pers)) {
            fprintf(stderr, "cannot obtain the object\n");
            return NULL;
        }
    }
    
    return file;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     saf2html
 * Purpose:     Print table headers
 *
 * Description: Output HTML table headers
 *
 * Return:      Non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, June  2, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
dump_headers(ss_pers_t *obj)
{
    ss_pers_class_t     *pc=SS_PERS_CLASS(SS_MAGIC_SEQUENCE(SS_MAGIC_OF(obj)));
    unsigned            membno, nmembs;
    char                *memb_name;
    
    assert(pc);
    assert(pc->tfm>0);

    printf("<th>Index</th>");
    nmembs = H5Tget_nmembers(pc->tfm);
    for (membno=0; membno<nmembs; membno++) {
        memb_name = H5Tget_member_name(pc->tfm, membno);
        printf("<th>%s</th>", memb_name);
        free(memb_name);
    }
    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     saf2html
 * Purpose:     Print information about one object
 *
 * Description: Prints information about an object in HTML format.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Issue:       We could use ss_pers_dump() instead of this function. However, we don't want the object link information
 *              printed which that function normally does.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, June  2, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
dump_object(ss_pers_t *obj)
{
    ss_pers_class_t     *pc=SS_PERS_CLASS(SS_MAGIC_SEQUENCE(SS_MAGIC_OF(obj)));
    unsigned            membno, nmembs;
    char                *memb_name;
    hid_t               memb_type;
    size_t              memb_off;
    
    assert(pc);
    assert(pc->tfm>0);

    printf("<tr>");
    printf("<td>%lu</td>", (unsigned long)ss_pers_link_objidx(obj));
    nmembs = H5Tget_nmembers(pc->tfm);
    for (membno=0; membno<nmembs; membno++) {
        memb_name = H5Tget_member_name(pc->tfm, membno);
        memb_type = H5Tget_member_type(pc->tfm, membno);
        memb_off = H5Tget_member_offset(pc->tfm, membno);
        ss_val_dump((char*)ss_pers_deref(obj)+memb_off, memb_type, obj, stdout, "td");
        free(memb_name);
        H5Tclose(memb_type);
    }
    printf("</tr>");

    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     saf2html
 * Purpose:     Prints information about one of many objects
 *
 * Description: 
 *
 * Return:      
 *
 * Parallel:    
 *
 * Programmer:  Robb Matzke
 *              Friday, June  4, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static htri_t
table_scan(size_t UNUSED itemidx, ss_pers_t *pers, ss_persobj_t UNUSED *persobj, void UNUSED *udata)
{
    dump_object(pers);
    return 0;
}
