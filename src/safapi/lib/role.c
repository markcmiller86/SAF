/*
 * Copyright(C) 1999-2005 The Regents of the University of California.
 *     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
 *     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
 *     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
 *     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
 *     dissemination, commercialization  through formal licensing, or other  disposition under terms of
 *     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
 *     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
 *     University.
 * 
 * Copyright(C) 1999-2005 Sandia Corporation.  
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
 * 
 * Active Developers:
 *     Peter K. Espen              SNL
 *     Eric A. Illescas            SNL
 *     Jake S. Jones               SNL
 *     Robb P. Matzke              LLNL
 *     Greg Sjaardema              SNL
 * 
 * Inactive Developers:
 *     William J. Arrighi          LLNL
 *     Ray T. Hitt                 SNL
 *     Mark C. Miller              LLNL
 *     Matthew O'Brien             LLNL
 *     James F. Reus               LLNL
 *     Larry A. Schoof             SNL
 * 
 * Acknowledgements:
 *     Marty L. Barnaby            SNL - Red parallel perf. study/tuning
 *     David M. Butler             LPS - Data model design/implementation Spec.
 *     Albert K. Cheng             NCSA - Parallel HDF5 support
 *     Nancy Collins               IBM - Alpha/Beta user
 *     Linnea M. Cook              LLNL - Management advocate
 *     Michael J. Folk             NCSA - Management advocate 
 *     Richard M. Hedges           LLNL - Blue-Pacific parallel perf. study/tuning 
 *     Wilbur R. Johnson           SNL - Early developer
 *     Quincey Koziol              NCSA - Serial HDF5 Support 
 *     Celeste M. Matarazzo        LLNL - Management advocate
 *     Tyce T. McLarty             LLNL - parallel perf. study/tuning
 *     Tom H. Robey                SNL - Early developer
 *     Reinhard W. Stotzer         SNL - Early developer
 *     Judy Sturtevant             SNL - Red parallel perf. study/tuning 
 *     Robert K. Yates             LLNL - Blue-Pacific parallel perf. study/tuning
 * 
 */
#include <safP.h>

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Collection Roles
 * Purpose:     Declare a new collection role
 *
 * Description: This function declares a new collection role with a unique identification number.
 *
 * Return:      A handle to the new role.
 *
 * Programmer:  Robb Matzke
 *              Thursday, April 19, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Role *
saf_declare_role(SAF_ParMode pmode,                     /* The parallel mode */
                 SAF_Db *db,                            /* The database in which to create the new role */
                 const char *name,                      /* Name of the role */
                 const char *url,                       /* An optional URL to the role documentation */
                 SAF_Role *role                         /* OUT: Optional role handle to initialize (and return) */
                 )
{
    SAF_ENTER(saf_declare_role, NULL);
    ss_scope_t          topscope;

    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(NULL);
    ss_file_topscope(db, &topscope);
    role = (ss_role_t*)ss_pers_new(&topscope, SS_MAGIC(ss_role_t), NULL, SAF_ALL==pmode?SS_ALLSAME:0U, (ss_pers_t*)role, NULL);

    if (SAF_EACH==pmode) SS_PERS_UNIQUE(role);
    ss_string_set(SS_ROLE_P(role,name), name);
    ss_string_set(SS_ROLE_P(role,url), url);

    SAF_LEAVE(role);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Collection Roles
 * Purpose:     Describe a role
 *
 * Description: Breaks ROLE into its parts and returns them through pointers.
 *
 * Return:      A non-negative value indicates success, and a negative value indicates failure. On return, the output
 *              arguments NAME, URL, and ID will be initialized.
 *
 * Programmer:  Robb Matzke
 *              Thursday, April 19, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_describe_role(SAF_ParMode pmode,
                  SAF_Role *role,               /* Role to describe */
                  char **name,                  /* If non-null, on return points to malloc'd role name if any */
                  char **url                    /* If non-null, on return points to malloc'd URL if any */
                  )
{
    SAF_ENTER(saf_describe_role, SAF_PRECONDITION_ERROR);

    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    SAF_REQUIRE(SS_ROLE(role), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("ROLE must be a valid role handle"));

    _saf_setupReturned_string(name, ss_string_ptr(SS_ROLE_P(role,name)));
    _saf_setupReturned_string(url, ss_string_ptr(SS_ROLE_P(role,url)));

    SAF_LEAVE(SAF_SUCCESS);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Collection Roles
 * Purpose:     Find roles
 *
 * Description: This function allows a client to search for roles in the database. The search may be limited by one or
 *		more criteria such as the name of the role, etc.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_find_roles(SAF_ParMode pmode,
               SAF_Db *db,                      /* Database in which to limit the search. */
               const char *name,                /* Optional name to which to limit the search. */
               char *url,                       /* Optional URL to which to limit the search. */
               int *num,       	                /* For this and the succeeding argument [see Returned Handles]. */
               SAF_Role **found            	/* For this and the preceding argument [see Returned Handles]. */
               )
{
    SAF_ENTER(saf_find_roles, SAF_PRECONDITION_ERROR);
    SAF_KEYMASK(SAF_Role, key, mask);
    size_t              nfound;
    ss_scope_t          scope;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    SAF_REQUIRE(ss_file_isopen(db, NULL)>0, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("DB must be a valid database"));
    SAF_REQUIRE(_saf_valid_memhints(num, (void**)found), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
               _saf_errmsg("NUM and FOUND must be compatible for return value allocation"));

    ss_file_topscope(db, &scope);
    if (name) SAF_SEARCH_S(SAF_Role, key, mask, name, name);
    if (url)  SAF_SEARCH_S(SAF_Role, key, mask, url, url);

    if (!found) {
        /* Count the matches */
        assert(num);
        nfound = SS_NOSIZE;
        ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound, SS_PERS_TEST, NULL);
        *num = nfound;
    } else if (!*found) {
        /* Find all matches; library allocates results */
        nfound = SS_NOSIZE;
        *found = (ss_role_t*)ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound, NULL, NULL);
        if (num) *num = nfound;
    } else {
        /* Find limited matches; client allocates result buffer */
        assert(num);
        nfound = *num;
        if (NULL==ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound, (ss_pers_t*)*found,
                               _SAF_GLOBALS.find_detect_overflow)) {
            SAF_ERROR(SAF_CONSTRAINT_ERROR, _saf_errmsg("found too many matching objects"));
        }
        *num = nfound;
    }

    SAF_LEAVE(SAF_SUCCESS);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Collection Roles
 * Purpose:     Find one collection role
 *
 * Description: This is a convenience version of saf_find_roles() that returns the first role it finds whose name matches that
 *              which is specified.
 *
 * Return:      A handle to a matching role on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Robb Matzke
 *              Thursday, April 19, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Role *
saf_find_one_role(SAF_Db *database,                     /* The database in which to search */
                  const char *name,                     /* The name for which to search */
                  SAF_Role *buf                         /* OUT: Optional role handle to initialize and return. */
                  )
{
    SAF_ENTER(saf_find_one_role, NULL);
    int                 n=1;

    saf_find_roles(SAF_EACH, database, name, NULL, &n, &buf);
    SAF_LEAVE(buf);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Collection Roles
 * Purpose:     Find collection role space_slice
 *
 * Description: Find and return collection role space_slice
 *
 * Return:      A handle to a matching role on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Peter Espen
 *              July 19, 2002
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Role *
saf_find_role_space_slice(void)
{
  SAF_ENTER(saf_find_role_space_slice, NULL);
  if(_SAF_GLOBALS.role_space_slice == NULL){
      _SAF_GLOBALS.role_space_slice = saf_find_one_role(_SAF_GLOBALS.reg.std_file, "space_slice", NULL);
  }
  SAF_LEAVE(_SAF_GLOBALS.role_space_slice);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Collection Roles
 * Purpose:     Find collection role param_slice
 *
 * Description: Find and return collection role param_slice
 *
 * Return:      A handle to a matching role on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Peter Espen
 *              July 19, 2002
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Role *
saf_find_role_param_slice(void)
{
  SAF_ENTER(saf_find_role_param_slice, NULL);
  if(_SAF_GLOBALS.role_param_slice == NULL){
      _SAF_GLOBALS.role_param_slice = saf_find_one_role(_SAF_GLOBALS.reg.std_file, "param_slice", NULL);
  }
  SAF_LEAVE(_SAF_GLOBALS.role_param_slice);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Collection Roles
 * Purpose:     Find collection role topology
 *
 * Description: Find and return collection role topology
 *
 * Return:      A handle to a matching role on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April 19, 2002
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Role *
saf_find_role_topology(void)
{
  SAF_ENTER(saf_find_role_topology, NULL);
  if(_SAF_GLOBALS.role_topology == NULL){
      _SAF_GLOBALS.role_topology = saf_find_one_role(_SAF_GLOBALS.reg.std_file, "topology", NULL);
  }
  SAF_LEAVE(_SAF_GLOBALS.role_topology);
}
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Collection Roles
 * Purpose:     Find collection role processor
 *
 * Description: Find and return collection role processor
 *
 * Return:      A handle to a matching role on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April 19, 2002
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Role *
saf_find_role_processor(void)
{
  SAF_ENTER(saf_find_role_processor, NULL);
  if(_SAF_GLOBALS.role_processor == NULL){
      _SAF_GLOBALS.role_processor = saf_find_one_role(_SAF_GLOBALS.reg.std_file, "processor", NULL);
  }
  SAF_LEAVE(_SAF_GLOBALS.role_processor);
}
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Collection Roles
 * Purpose:     Find collection role block
 *
 * Description: Find and return collection role block
 *
 * Return:      A handle to a matching role on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April 19, 2002
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Role *
saf_find_role_block(void)
{
  SAF_ENTER(saf_find_role_block, NULL);
  if(_SAF_GLOBALS.role_block == NULL){
      _SAF_GLOBALS.role_block = saf_find_one_role(_SAF_GLOBALS.reg.std_file, "block", NULL);
  }
  SAF_LEAVE(_SAF_GLOBALS.role_block);
}
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Collection Roles
 * Purpose:     Find collection role domain
 *
 * Description: Find and return collection role domain
 *
 * Return:      A handle to a matching role on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April 19, 2002
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Role *
saf_find_role_domain(void)
{
  SAF_ENTER(saf_find_role_domain, NULL);
  if(_SAF_GLOBALS.role_domain == NULL){
      _SAF_GLOBALS.role_domain = saf_find_one_role(_SAF_GLOBALS.reg.std_file, "domain", NULL);
  }
  SAF_LEAVE(_SAF_GLOBALS.role_domain);
}
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Collection Roles
 * Purpose:     Find collection role assembly
 *
 * Description: Find and return collection role assembly
 *
 * Return:      A handle to a matching role on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April 19, 2002
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Role *
saf_find_role_assembly(void)
{
  SAF_ENTER(saf_find_role_assembly, NULL);
  if(_SAF_GLOBALS.role_assembly == NULL){
      _SAF_GLOBALS.role_assembly = saf_find_one_role(_SAF_GLOBALS.reg.std_file, "assembly", NULL);
  }
  SAF_LEAVE(_SAF_GLOBALS.role_assembly);
}
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Collection Roles
 * Purpose:     Find collection role material
 *
 * Description: Find and return collection role material
 *
 * Return:      A handle to a matching role on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April 19, 2002
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Role *
saf_find_role_material(void)
{
  SAF_ENTER(saf_find_role_material, NULL);
  if(_SAF_GLOBALS.role_material == NULL){
      _SAF_GLOBALS.role_material = saf_find_one_role(_SAF_GLOBALS.reg.std_file, "material", NULL);
  }
  SAF_LEAVE(_SAF_GLOBALS.role_material);
}

