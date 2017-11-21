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
#include "ss.h"

SAF_Cat *saf_self;

dbobject *_saf_open( char * , char * );
PyObject *get_topsets( PyObject * );
PyObject *get_suites( PyObject * );
void sil( PyObject * );
void describe_set( PyObject * );
void my_saf_init( void );
void _switch_db( DB_Info * );
PyObject *ss_switch_db( PyObject *, PyObject * );
PyObject *get_field_data( PyObject *, PyObject * );
PyObject *get_state_field_data( PyObject *, PyObject * );
PyObject *remap_and_read_field( PyObject *, PyObject * );
PyObject *get_subset_relation_data( PyObject *, PyObject *);
PyObject *ss_saf_open( PyObject *, PyObject *, PyObject *); 
PyObject * ss_sil( PyObject *, PyObject *);
PyObject * ss_describe_set( PyObject *, PyObject *);
PyObject * ss_topsets( PyObject *, PyObject *);
PyObject * ss_suites( PyObject *, PyObject *);
PyObject * ss_saf_init( PyObject *, PyObject *);
PyObject * ss_get_field_data( PyObject *, PyObject *);
PyObject * ss_subset_relation_data( PyObject *, PyObject *);
PyObject * is_there_topo_rel_data( PyObject *self, PyObject *py_allsets );
PyObject * get_collection_data( PyObject *self, PyObject *py_collection );
void initss(void);
void  get_topo_rel_data( PyObject *self, PyObject *py_allsets, char **sup_set_name,
			 char **sup_cat_name, char **sub_set_name, char **sub_cat_name,
			 PyObject **a_l_pyobject, PyObject **b_l_pyobject);


PyObject *ss_saf_open( PyObject *self, PyObject *args, PyObject *kwargs) {

	dbobject *db;
	char *filepath;
	char *filename;
	static const char *argnames[] = {"filepath","filename",NULL};
	PyObject *private;

        filepath = strdup("None");
        filename = strdup("None");

	if (!PyArg_ParseTupleAndKeywords(args,kwargs,"|ss",argnames,&filepath,&filename)) {
		return NULL;
	}
 
	db = _saf_open( filepath, filename );

	if( db == NULL )
	  return Py_BuildValue("");

	private = Py_BuildValue("{s:i,s:i}",\
				"_db_handle",db,\
				"_topset_handle",db->tops\
	);

	return Py_BuildValue("{s:s,s:s,s:i,s:O,s:i,s:i,s:O,s:i,s:i,s:O}",\
		"type","SAF_Db",\
                "name",filename,\
		"_db_handle",db,\
		"_topsets",get_topsets(private),\
		"num_top_sets",db->num_top_sets,\
		"_topsets_handle",db->tops,\
                "_suites",get_suites(private),\
                "_suites_handle",db->suites,\
                "num_suites",db->num_suites,\
		"_private",private\
	);

	
}

PyObject * ss_sil( PyObject *self, PyObject *args) {

	PyObject *db;

	if (!PyArg_ParseTuple(args,"O", &db  )) {
		return Py_BuildValue("");
	}

	sil( (PyObject *)db );
	return Py_BuildValue("");
}

PyObject * ss_describe_set( PyObject *self, PyObject *args) {

	PyObject *db;

	if (!PyArg_ParseTuple(args,"O", &db  )) {
		return Py_BuildValue("");
	}

	if( !PyDict_Check(db) ) {
	  db = PyObject_GetAttrString(db,"data");
	}
	describe_set( (PyObject *)db );
	return Py_BuildValue("");
}

PyObject * ss_topsets( PyObject *self, PyObject *args) {

  PyObject *db;
  PyObject *top_sets;

  if( !PyArg_ParseTuple(args,"O", &db) ) {
    return Py_BuildValue("");
  }

  top_sets = get_topsets( db );
  return( top_sets );

}

PyObject * ss_suites( PyObject *self, PyObject *args)
{

  PyObject *db;
  PyObject *suites;

  if( !PyArg_ParseTuple(args, "O", &db) ) {
    return Py_BuildValue("");
  }

  suites = get_suites( db );
  return( suites );
}


PyObject * ss_saf_init( PyObject *self, PyObject *args) {

	my_saf_init();
	return Py_BuildValue("");


}

PyObject * ss_get_collection_data( PyObject *self, PyObject *args ) {
/* 
   Author: Erik Illescas
   Retrieve topology data.
*/
                                                                                                                                     
  PyObject *data_array;
  PyObject *py_collection;
                                                                                                                                     
  if( !PyArg_ParseTuple(args,"O",&py_collection) ) {
    Py_INCREF(Py_None);
    return Py_None;
  }
                                                                                                                                     
  data_array = get_collection_data( self, py_collection );
  return Py_BuildValue("O",data_array);
                                                                                                                                     
                                                                                                                                     
}
PyObject * ss_is_there_topo_rel_data( PyObject *self, PyObject *args ) {
/* 
   Author: Erik Illescas
   is there topology data in the given object.
*/
                                                                                                                                     
  PyObject *data_array;
  PyObject *py_allsets;
                                                                                                                                     
  if( !PyArg_ParseTuple(args,"O",&py_allsets) ) {
    Py_INCREF(Py_None);
    return Py_None;
  }
                                                                                                                                     
  data_array = is_there_topo_rel_data( self, py_allsets );
  return Py_BuildValue("O",data_array);
                                                                                                                                     
                                                                                                                                     
}

PyObject *ss_get_topo_rel_data( PyObject *self, PyObject *args ) {
/*
   Author: Erik Illescas
   Retrieve collection data.
*/
                                                                                                                                     
  PyObject *b_data_array;
  PyObject *a_data_array;
  PyObject *db;

  char *sub_set_name;
  char *sub_cat_name;
  char *sup_set_name;
  char *sup_cat_name;
                                                                                                                                     
  if( !PyArg_ParseTuple(args,"O",&db) ) {
    Py_INCREF(Py_None);
    return Py_None;
  }
                                                                                                                                     
 get_topo_rel_data( self, db, &sup_set_name, &sup_cat_name, &sub_set_name, &sub_cat_name,
                          &a_data_array, &b_data_array ) ;
 return Py_BuildValue("{s:s,s:s,s:s,s:s,s:O,s:O}",\
                "sup_set_name",sup_set_name,\
                "sup_cat_name",sup_cat_name,\
                "sub_set_name",sub_set_name,\
                "sub_cat_name",sub_cat_name, \
                "abuf",a_data_array, \
                "bbuf",b_data_array \
                );
                                                                                                                                     
}


PyObject * ss_get_field_data( PyObject *self, PyObject *args ) {

  PyObject *data_array;
  PyObject *py_field;

  if( !PyArg_ParseTuple(args,"O",&py_field) ) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  data_array = get_field_data( self, py_field );
  return Py_BuildValue("O",data_array);


}


PyObject * ss_subset_relation_data( PyObject *self, PyObject *args ) {

  PyObject *data_array;
  PyObject *py_set;
  PyObject *py_coll;

  if( !PyArg_ParseTuple(args,"OO",&py_set,&py_coll) ) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  data_array = get_subset_relation_data( py_set, py_coll );

  return Py_BuildValue("O",data_array);

}



static PyMethodDef ssmethods[] = {
  {"_saf_open",              ss_saf_open,               METH_VARARGS | METH_KEYWORDS, NULL }, 
  {"sil",                    ss_sil,                    METH_VARARGS, NULL },
  {"saf_init",               ss_saf_init,               METH_VARARGS, NULL },
  {"describe_set",           ss_describe_set,           METH_VARARGS, NULL },
  {"top_sets",               ss_topsets,                METH_VARARGS, NULL },
  {"switch_db",              ss_switch_db,              METH_VARARGS, NULL },
  {"get_field_data",         ss_get_field_data,         METH_VARARGS, NULL },
  {"subset_relation_data",   ss_subset_relation_data,   METH_VARARGS, NULL },
  {"get_collection_data",    ss_get_collection_data,    METH_VARARGS, NULL },
  {"get_topo_rel_data",      ss_get_topo_rel_data,      METH_VARARGS, NULL },
  {"is_there_topo_rel_data", ss_is_there_topo_rel_data, METH_VARARGS, NULL },
  {NULL,NULL,0,NULL}
};

void initss() {
	Py_InitModule("ss",ssmethods);
}




SAF_Cat *p_cats;
SAF_Set *top_sets, *all_sets, *suite_sets, *suite_subsets, *tmp_sets, *tmp2_sets;
SAF_FieldTmpl *all_field_tmpls;
SAF_Field *all_fields;
SAF_CellType celltype;

static SAF_Db *db;
/* static  SAF_Db *return_db; */

SAF_DbProps *dbprops=NULL;


#define TRUE 1
#define FALSE 0
static int ss_saf_initialized = FALSE;

Set tops = NULL;
Set allsets = NULL;
Set suitesets = NULL;
Set suitesubsets = NULL;
FieldTmpls allfieldtmpls = NULL, tmp_fieldtmpl_info = NULL;
/* FieldTmpls *fieldtmpl_lookup = NULL; */

Fields allfields = NULL, tmp_field_info = NULL;
/* Fields *field_lookup = NULL; */


int num_top_sets, num_all_sets, num_tmp_sets, num_tmp2_sets, num_suite_sets, number_suite_subsets;
int num_field_tmpls, num_all_fields;


/* these are some of my local utility functions contained in this file */
int get_topo_range_set( int );
void find_collections( Set_Info *, int );
void find_fields_on_set( Set_Info *, int );
char * saf_set_name( SAF_Set * );
char * saf_cat_name( SAF_Cat * );
char * role_string( int );
void   rs( char * );
char * celltype_string( int );
char * objecttype_string( int );
/* char * algtype_string( int ); */
char * algtype_string( SAF_Algebraic * ); 
char * basis_string( SAF_Basis * );
char * evalfunc_string( SAF_Eval * );
void get_fieldtmpl_info( FieldTmpl_Info * );
void get_field_info( Field_Info * );
void get_topos( Set_Info *, int );
void find_topo_relations( Set_Info * );
Set_Info * get_subsets( Set_Info *, int );
Set_Info * oldoldget_subsets( Set_Info *, int );
void get_set_info( Set_Info * );
char * dsltype_string( hid_t dsl_type );
char * interleave_string( SAF_Interleave );
char * silrole_string(SAF_SilRole id);
SAF_Db * new_database( char *name );
void new_allsets( DB_Info *, Set_Info *, int);
void new_set( DB_Info *, Set_Info *, Set_Info *);
void new_subsets( DB_Info *, Set_Info *, int);
void new_sil( DB_Info *, Set_Info *);
void new_field( DB_Info *, Field_Info *, Field_Info * );
/* void new_fieldtmpls( DB_Info *, FieldTmpl_Info *, int ); */
void new_tmpl( DB_Info *, FieldTmpl_Info * , FieldTmpl_Info *);

void recurse(Set_Info *, SAF_Cat, int);
void level_print(int);

FieldTableOfValues_t *field_lookup=NULL;
int nfield_lookup=0;

FieldtmplTableOfValues_t *fieldtmpl_lookup=NULL;
int nfieldtmpl_lookup=0;

PyObject *py_sets( dbobject * , Set_Info *, int);
PyObject *py_collections( dbobject *, Set_Info * );
PyObject *py_subsets( dbobject *, Set_Info *, int );
PyObject *py_fields( dbobject *, Set_Info * );
PyObject *py_fields_from_list( Field_Info *, int );
int Ftabcompare(const void *, const void *) ;
int Ftmpltabcompare(const void *, const void *);
Fields Ftab_find(FieldTableOfValues_t *, SAF_Field *, int);
FieldTmpls Ftmpltab_find(FieldtmplTableOfValues_t *, SAF_FieldTmpl *, int);
/* dbobject PY_Db; */

int Ftabcompare(const void  *i, const void *j)
{
  int value;
  FieldTableOfValues_t *left;
  FieldTableOfValues_t *right;
  ss_fieldobj_t mask;
                                                                                                                                                                    
  left = (FieldTableOfValues_t *)i;
  right = (FieldTableOfValues_t *)j;
                                                                                                                                                                    
  /* ss_set_t s1, s2; */
  memset(&mask, SS_VAL_CMP_DFLT, sizeof mask);
                                                                                                                                                                    
  value = ss_pers_cmp((ss_pers_t*)&(left->key), (ss_pers_t*)&(right->key), (ss_persobj_t*)&mask);
                                                                                                                                                                    
  return(value);
}

int Ftmpltabcompare(const void  *i, const void *j)
{
  int value;
  FieldtmplTableOfValues_t *left;
  FieldtmplTableOfValues_t *right;
  ss_fieldtmplobj_t mask;
                                                                                                                                                                    
  left = (FieldtmplTableOfValues_t *)i;
  right = (FieldtmplTableOfValues_t *)j;
                                                                                                                                                                    
  /* ss_set_t s1, s2; */
  memset(&mask, SS_VAL_CMP_DFLT, sizeof mask);
                                                                                                                                                                    
  value = ss_pers_cmp((ss_pers_t*)&(left->key), (ss_pers_t*)&(right->key), (ss_persobj_t*)&mask);
                                                                                                                                                                    
  return(value);
}


Fields Ftab_find(FieldTableOfValues_t *table, SAF_Field *rlfield, int n)
{
  FieldTableOfValues_t *ret_field;
  ret_field = (FieldTableOfValues_t *)bsearch((void *)rlfield, (const void *)table, (size_t) n, sizeof (FieldTableOfValues_t),Ftabcompare);
  if (ret_field)
     return (ret_field->value);
  else
     return (NULL);
                                                                                                                                                                    
}

FieldTmpls Ftmpltab_find(FieldtmplTableOfValues_t *table, SAF_FieldTmpl *rlfield, int n)
{
  FieldtmplTableOfValues_t *ret_field;
  ret_field = (FieldtmplTableOfValues_t *)bsearch((void *)rlfield, (const void *)table, (size_t) n, sizeof (FieldtmplTableOfValues_t),Ftmpltabcompare);
  if (ret_field)
     return (ret_field->value);
  else
     return (NULL);
                                                                                                                                                                    
}


PyObject *ss_switch_db( PyObject *self, PyObject *args )
{

	PyObject *l_pyobject = NULL, *py_db;
	dbobject *the_db;
	DB_Info *the_db_info;

	if( !PyArg_ParseTuple(args,"O",&l_pyobject) ) {
	  Py_INCREF(Py_None);
	  return Py_None;
	}

    py_db = PyDict_GetItemString(l_pyobject,"_db_handle");
	the_db  = (dbobject *)PyInt_AsLong(py_db);

	the_db_info = the_db->dbhandle;

	_switch_db( the_db_info );

    Py_INCREF(Py_None);
    return Py_None;

}

void _switch_db( DB_Info *the_db_info )
{

	saf_self = the_db_info->saf_self;
	allsets = the_db_info->allsets;
	num_all_sets = the_db_info->num_all_sets;
	tops = the_db_info->tops;
	num_top_sets = the_db_info->num_top_sets;
	suitesets = the_db_info->suitesets;
	num_suite_sets = the_db_info->num_suite_sets;
	allfieldtmpls = the_db_info->allfieldtmpls;
	num_field_tmpls = the_db_info->num_field_tmpls;
	fieldtmpl_lookup = the_db_info->fieldtmpl_lookup;
	allfields = the_db_info->allfields;
	num_all_fields = the_db_info->num_all_fields;
	field_lookup = the_db_info->field_lookup;

}

/* int ss_main(int argc, char **argv) */
dbobject * _saf_open( char *filepath,  char *filename )
{
    int status = -1;
    int self = 0;
    int ntasks = 1;
    char *new_filename;
    char * js = "java";
    char ** jp = &js;

    int create_new_file = 0;


	/*SAF_Db *return_db;*/
   dbobject *PY_Db;

   DB_Info *return_db;
   DB_Info *db_info, *new_db_info;

	SAF_LibProps *libprops;

   char *set_name;
   int i, j, k;


   int tdim, num_colls;
   SAF_TopMode t_mode;
   SAF_ExtendMode e_mode;
   SAF_SilRole srole;

   /* SAF_TRType     trtype; */


   if( !ss_saf_initialized ) {

    
#ifdef HAVE_PARALLEL 

  		MPI_Init(&ntasks, &jp  );
  		MPI_Comm_rank(MPI_COMM_WORLD, &self);
		MPI_Comm_size(MPI_COMM_WORLD, &ntasks);
#endif
     


        libprops = saf_createProps_lib( );
     	saf_setProps_ErrorMode(libprops , SAF_ERRMODE_RETURN ) ; 
        saf_init(libprops);
	saf_freeProps_lib(libprops);
        ss_saf_initialized = TRUE;
	import_array();
   }

   chdir(filepath);

    /* open the database */
    dbprops = saf_createProps_database();

    libprops = saf_createProps_lib( );

    saf_setProps_ReadOnly(dbprops);

    db = saf_open_database(filename,dbprops);

    db_info = (DB_Info *)calloc((size_t)1, sizeof(DB_Info));
    db_info->the_db = db;
    


#if 0
    new_filename = (char *)calloc( (size_t)(strlen(filename)+5) , sizeof(char) );
    strcpy(new_filename, filename);
    strcat(new_filename, ".new");

    if( access( new_filename, F_OK ) ) {
      create_new_file = 1;
      newdb = new_database( new_filename );
      new_db_info = (DB_Info *)malloc(sizeof(DB_Info));
      new_db_info->the_db = *newdb;
      new_db_info->allsets = NULL;
    }
#endif

    if(db) {
      status = 0;
    }
    else {
      status = -1;
      printf("_saf_open: db not assigned \n");
      return NULL;
    }

	saf_self = (SAF_Cat *)SAF_SELF(db);

    if( status != 0 ) {
      printf("Database open error: %s:%d\n",filename,status);
      return NULL;
    }
      


    PY_Db = (dbobject *)calloc((size_t)1, sizeof(dbobject));

#ifdef SSLIB_SUPPORT_PENDING
   hbool_t multifile;
    /* determine if the database was generated in a multifile mode */
    saf_find_files(db, SAF_ANY_NAME, &i, NULL);
    if (i > 0)
       multifile = true;
    else
       multifile = false;
#endif /*SSLIB_SUPPORT_PENDING*/



    allsets = NULL;

    num_field_tmpls = 0;
    all_field_tmpls = NULL;
    saf_find_field_tmpls( SAF_ALL, db,  NULL, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_QUANTITY, 
			  &num_field_tmpls, &all_field_tmpls);

    if( num_field_tmpls > 0 ) {

      allfieldtmpls = (FieldTmpl_Info *)calloc((size_t) num_field_tmpls , sizeof(FieldTmpl_Info));

      for( i = 0 ; i < num_field_tmpls ; i++ ) {

	allfieldtmpls[i].fieldtmpl = all_field_tmpls + i;
	get_fieldtmpl_info( allfieldtmpls + i );
      }
    }

    db_info->num_field_tmpls = num_field_tmpls;
    db_info->allfieldtmpls = allfieldtmpls;


    num_all_fields = 0;
    all_fields = NULL;
    saf_find_fields(SAF_ALL, db, SAF_UNIVERSE(db), NULL, NULL, SAF_ALGTYPE_ANY, \
		    SAF_ANY_BASIS, SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, \
		    SAF_ANY_EFUNC, \
		    &num_all_fields, &all_fields);

    if( num_all_fields > 0 ) {
      allfields = (Field_Info *)calloc((size_t) num_all_fields , sizeof( Field_Info ));
      for( i = 0 ; i < num_all_fields; i++ ) {
	allfields[i].the_field = all_fields + i;
	get_field_info( allfields + i );
      }
    }
    
    db_info->num_all_fields = num_all_fields;
    db_info->allfields = allfields;
    db_info->saf_self = (SAF_Cat *)SAF_SELF(db);


    p_cats = NULL;
    all_sets = NULL;
    tmp_sets = NULL;
    tmp2_sets = NULL;
    top_sets = NULL;
    num_tmp_sets = -1;
    num_tmp2_sets = -1;
    num_top_sets = -1;


		saf_find_matching_sets(SAF_ALL, db, SAF_ANY_NAME, SAF_SPACE, SAF_ANY_TOPODIM, \
			SAF_EXTENDIBLE_TORF, SAF_TOP_TRUE, &num_top_sets, NULL);
		if( num_top_sets > 0 ) {
		  tmp_sets = (SAF_Set *)calloc((size_t)num_top_sets , sizeof(SAF_Set));
		  saf_find_matching_sets(SAF_ALL, db, SAF_ANY_NAME, SAF_SPACE, SAF_ANY_TOPODIM, \
					 SAF_EXTENDIBLE_TORF, SAF_TOP_TRUE, &num_top_sets, &tmp_sets);
		  top_sets = tmp_sets;
		}
		tmp_sets = NULL;
		saf_find_matching_sets(SAF_ALL, db, SAF_ANY_NAME, SAF_ANY_SILROLE, SAF_ANY_TOPODIM, \
			SAF_EXTENDIBLE_TORF, SAF_TOP_FALSE, &num_tmp_sets, NULL);
		if( num_tmp_sets > 0 ) {
		  tmp_sets = (SAF_Set *)calloc((size_t)num_tmp_sets , sizeof(SAF_Set));
		  saf_find_matching_sets(SAF_ALL, db, SAF_ANY_NAME, SAF_ANY_SILROLE, SAF_ANY_TOPODIM, \
					 SAF_EXTENDIBLE_TORF, SAF_TOP_FALSE, &num_tmp_sets, &tmp_sets);
		  
		}
		tmp2_sets = NULL;
		saf_find_matching_sets(SAF_ALL, db, SAF_ANY_NAME, SAF_SUITE, SAF_ANY_TOPODIM, \
			SAF_EXTENDIBLE_TORF, SAF_TOP_TORF, &num_tmp2_sets, NULL);
		if( num_tmp2_sets > 0 ) {
		  tmp2_sets = (SAF_Set *)calloc((size_t)num_tmp2_sets , sizeof(SAF_Set));
		  saf_find_matching_sets(SAF_ALL, db, SAF_ANY_NAME, SAF_SUITE, SAF_ANY_TOPODIM, \
					 SAF_EXTENDIBLE_TORF, SAF_TOP_TORF, &num_tmp2_sets, &tmp2_sets);
		  
		}


		num_all_sets = num_tmp_sets + num_top_sets + num_tmp2_sets;

		all_sets = (SAF_Set *)calloc((size_t) num_all_sets  , sizeof(SAF_Set));

		for( i = 0 ; i < num_top_sets ; i++ ) {
		  all_sets[i] = top_sets[i];
		}
		for( i = 0 ; i < num_tmp_sets; i++ ) {
		  all_sets[i + num_top_sets] = tmp_sets[i];
		}
		for( i = 0 ; i < num_tmp2_sets; i++ ) {
		  all_sets[i + num_top_sets + num_tmp_sets] = tmp2_sets[i];
		}

		
		free(top_sets);
		free(tmp_sets);
		free(tmp2_sets);
		top_sets = all_sets;
		






	allsets = (Set_Info *)calloc((size_t)num_all_sets , sizeof(Set_Info));

	db_info->allsets = allsets;
	db_info->num_all_sets = num_all_sets;

	/* do an error check for AllSets here */

	if( num_all_sets > 0 ) {
	  printf("exploring DataBase: %s ...",filename);
	}

	for(i = 0; i < num_all_sets; i++ ) {
		set_name=NULL;

                saf_describe_set(SAF_ALL, all_sets+i, &set_name, &tdim, &srole, &e_mode, &t_mode, &num_colls, NULL);

		/* printf("allsets %d is %s\n",i,set_name); */

		allsets[i].the_set = all_sets[i];
		allsets[i].num_sets = num_all_sets;
		allsets[i].set_name = set_name;
		allsets[i].silrole = srole;
		allsets[i].topo_relation = NULL;
		allsets[i].collections = NULL;
		allsets[i].fields = NULL;
		allsets[i].duplicate_set = NULL;
		allsets[i].num_fields = 0;

		find_collections( allsets, i );

		find_fields_on_set( allsets, i);


		for( j = 0 ; j < (allsets+i)->num_colls; j++ )
			get_topos( (allsets+i), j ); 

	}

    p_cats = NULL;
    suite_sets = NULL;
    suitesets = NULL;
    suite_subsets = NULL;
    number_suite_subsets = 0;
    num_suite_sets = -1;



		saf_find_matching_sets(SAF_ALL, db, SAF_ANY_NAME, SAF_SUITE, SAF_ANY_TOPODIM, \
			SAF_EXTENDIBLE_TORF, SAF_TOP_TORF, &num_suite_sets, NULL);
		saf_find_matching_sets(SAF_ALL, db, SAF_ANY_NAME, SAF_SUITE, SAF_ANY_TOPODIM, \
			SAF_EXTENDIBLE_TORF, SAF_TOP_TORF, &num_suite_sets, &suite_sets);

	db_info->num_suite_sets = num_suite_sets;




	if( num_suite_sets > 0 ) {

		suitesets = (Set_Info *)calloc((size_t)num_suite_sets , sizeof(Set_Info));

		for(i = 0; i < num_suite_sets; i++ ) {

			set_name=NULL;
            
			saf_describe_set(SAF_ALL, suite_sets+i, &set_name, &tdim, &srole, &e_mode, &t_mode, &num_colls, NULL);
			suitesets[i].the_set = suite_sets[i];
			suitesets[i].num_sets = num_suite_sets;
			suitesets[i].set_name = set_name;
			suitesets[i].silrole =  srole;
			suitesets[i].topo_relation = NULL;
			suitesets[i].collections = NULL;
			suitesets[i].fields = NULL;
			suitesets[i].num_fields = 0;

			find_collections( suitesets, i );
			find_fields_on_set( suitesets, i);
	
			for( j = 0 ; j < (suitesets+i)->num_colls; j++ ) {
 
			  if( !strcmp(suitesets[i].collections[j].cat_name,"space_slice_cat") ) {

			    saf_find_sets(SAF_ALL,SAF_FSETS_SUBS,&(suitesets[i].the_set),&((suitesets[i].collections[j]).the_cat),\
                                          &number_suite_subsets,NULL);
			    saf_find_sets(SAF_ALL,SAF_FSETS_SUBS,&(suitesets[i].the_set),&((suitesets[i].collections[j]).the_cat),\
                                          &number_suite_subsets,&suite_subsets);


			    suitesubsets = (Set_Info *)calloc((size_t)number_suite_subsets , sizeof(Set_Info));
			    for( k = 0 ; k < number_suite_subsets; k++ ) {
			      set_name = NULL;
			      saf_describe_set(SAF_ALL, suite_subsets+k, &set_name, &tdim, &srole, &e_mode, &t_mode, &num_colls, NULL);

			      suitesubsets[k].the_set = suite_subsets[k];
			      suitesubsets[k].num_sets = number_suite_subsets;
			      suitesubsets[k].set_name = set_name;
			      suitesubsets[k].silrole = srole;
			      suitesubsets[k].topo_relation = NULL;
			      suitesubsets[k].collections = NULL;
			      suitesubsets[k].fields = NULL;
			      suitesubsets[k].duplicate_set = NULL;
			      suitesubsets[k].num_fields = 0;
			      /*
			      find_collections( suitesubsets, k);
			      find_fields_on_set( suitesubsets, k);
			      */
                                
                            }
			  }
                        }

		}
	}
	else {
	  /* printf("DB: no suites\n"); */
	  suite_sets = NULL;
	  suitesets = NULL;
	  suite_subsets = NULL;
	  number_suite_subsets = 0;
	  num_suite_sets = -1;

	}

	db_info->suitesets = suitesets;
	db_info->suitesubsets = suitesubsets;
	db_info->number_suite_subsets = number_suite_subsets;


	tops = allsets;

	if( num_top_sets < 1 ) {

	  /* printf("old DB: num_top_sets < 1 \n"); */
	  if( number_suite_subsets > 0 ) {

	    num_top_sets = 1;

	    for( i = 0; i < number_suite_subsets ; i++ ) {
	      if ( suitesubsets[i].silrole == SAF_SPACE ){
		for( j = 0 ; j < num_all_sets ; i++ ) {
		  if(ss_pers_equal((ss_pers_t*)&(suitesubsets[i].the_set), (ss_pers_t*)&(allsets[j].the_set),NULL) ) {
		    tops = allsets + j;
		    break;
		  }
		}
	      }
	    }
	  }
	}
 
	if( num_top_sets <= 0 )  {
		printf("num_top_sets is %d\n",num_top_sets);
		return NULL;
	}


	for( i = 0 ; i < num_top_sets; i++ )
	  for( j = 0; j < (tops+i)->num_colls; j++ )
	    get_subsets( (tops+i), j );



	if( create_new_file ) {

	  new_sil( new_db_info , tops );
	  
	  tmp_fieldtmpl_info = (FieldTmpl_Info *)calloc((size_t) num_field_tmpls , sizeof(FieldTmpl_Info));
	  for( i = 0 ; i < num_field_tmpls ; i++ ) {
	    new_tmpl( new_db_info, allfieldtmpls + i, tmp_fieldtmpl_info + i );
	  }
	  new_db_info->allfieldtmpls = tmp_fieldtmpl_info;

	  tmp_field_info = (Field_Info *)calloc((size_t) num_all_fields , sizeof(Field_Info));

	  for( i = 0 ; i < num_all_fields; i++) {
	    new_field( new_db_info, allfields + i, tmp_field_info + i );
	  }
	}

	db_info->tops = tops;
	db_info->num_top_sets = num_top_sets;
	db_info->allsets = allsets;
	db_info->num_all_sets = num_all_sets;
	db_info->suitesets = suitesets;
	db_info->num_suite_sets = num_suite_sets;
	db_info->fieldtmpl_lookup = fieldtmpl_lookup;

        return_db = db_info;

  PY_Db->dbhandle = return_db;
  PY_Db->num_top_sets = num_top_sets;
  PY_Db->tops = tops;
  PY_Db->suites = suitesets;
  PY_Db->num_suites = num_suite_sets;


  printf("\n");

  if ( status == 0 )
        return PY_Db;


   return PY_Db;


}




void get_topos ( Set_Info *set_info, int coll_id )
{
	int i;
	int num_rels;
	SAF_Rel *rels;
	char *set_name, *cat_name;

	SAF_Set glueSet, partSet;
	SAF_Cat glueCat, partCat;
	/* SAF_TRType     trtype; */
	SAF_RelRep trtype;

	Collection_Info cat_info;

	cat_info = set_info->collections[coll_id];

	find_topo_relations( set_info );

	num_rels = set_info->num_topo_relations;


	if( num_rels > 0 ) {

		rels = set_info->topo_relation;

		for( i = 0; i < num_rels; i++ ) {

			saf_describe_topo_relation(SAF_ALL,rels+i,&partSet, &partCat, &glueSet, &glueCat, NULL, &trtype, NULL);

			if( SAF_EQUIV(&partCat, &(set_info->collections[coll_id].the_cat)) ) {
				cat_name = NULL;
				set_name = NULL;
				saf_describe_set(SAF_ALL,&glueSet, &set_name, NULL, NULL, NULL, NULL, NULL, NULL);
				saf_describe_category(SAF_ALL, &glueCat, &cat_name, NULL, NULL);
				set_info->collections[coll_id].topo_link = (void *)(rels + i);
				free(set_name);
				free(cat_name);
			}
		}
	}
}


void find_topo_relations( Set_Info *set_info)
{

	int num_rels;
	SAF_Rel *rels;


	if( set_info->topo_relation != NULL )
		return;


	rels = NULL;
	saf_find_topo_relations(SAF_ALL, db, &(set_info->the_set),NULL,&num_rels,NULL);
	saf_find_topo_relations(SAF_ALL, db, &(set_info->the_set),NULL,&num_rels,&rels);

	set_info->num_topo_relations = num_rels;

	if( num_rels > 0 )
		set_info->topo_relation = rels;
	else
		set_info->topo_relation = NULL;

}


int get_topo_range_set( int rel_id )
{
	int i;
	
	SAF_Rel *topo_rel;
	SAF_Set partSet, glueSet;
	SAF_Cat partCat, glueCat;
	/* SAF_TRType trtype; */
	SAF_RelRep trtype;

	topo_rel = (SAF_Rel *)rel_id;
	saf_describe_topo_relation(SAF_ALL,topo_rel,&partSet, &partCat, &glueSet, &glueCat, NULL, &trtype, NULL);

	for( i = 0; i < num_all_sets; i++ ) {
		if( SAF_EQUIV( &(allsets[i].the_set), &glueSet ) ) {
			return i;
		}
	}

	return 0;

}




void find_fields_on_set( Set_Info *the_set, int set_num)
{

	int i, j, num_fields = 0;
	SAF_Field *fields;
	Field_Info *field_info;
	SAF_Set saf_set;
	Set_Info *tmp_set;
	SAF_Cat *coeff_assoc;
	SAF_Cat *saf_self;
	SAF_Cat *tmp_cat = NULL;
	char *name;

	tmp_set = the_set + set_num;
	if( tmp_set->fields != NULL )
		return;
	
	if( tmp_set->fields != NULL )
		return;


	saf_set = tmp_set->the_set;
	fields = NULL;


	saf_find_fields(SAF_ALL, db, &saf_set, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_UNIT, \
			SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &num_fields, NULL);


	tmp_set->num_fields = num_fields;

	if( num_fields <= 0 ) {
		tmp_set->fields = NULL;
		return;
	}



	tmp_set->fields = (Field_Info *)(calloc((size_t)num_fields , sizeof(Field_Info)));

	saf_find_fields(SAF_ALL, db, &saf_set, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_UNIT, \
			SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &num_fields, &fields);

	saf_self = SAF_SELF(db);


	for( i = 0; i < num_fields; i++ ) {

		name = NULL;
		tmp_set->fields[i].the_field = fields + i;
		field_info = ((Field_Info *)(tmp_set->fields)) + i;


		get_field_info( field_info );


		for( j = 0 ; j < field_info->base_space->num_colls; j++ ) {
		  tmp_cat = &(field_info->base_space->collections[j].the_cat);
		  coeff_assoc = field_info->coeff_assoc;
		  if( SAF_EQUIV( tmp_cat, coeff_assoc ) ) {
		    field_info->my_set_coll_num = j;
		    break;
		  }
		}
	}
}

void get_field_info( Field_Info *field_info )
{

        int i;
        static size_t count = 0;
        static hid_t Ptype;
        SAF_Field *field=NULL;
        SAF_FieldTarget *ftarget;
        SAF_Set *base_space;
        SAF_FieldTmpl *field_template=NULL;
        FieldTmpl_Info *template_info=NULL;
        SAF_Cat *storage_decomp=NULL;
        SAF_Cat *coeff_assoc=NULL;
        SAF_Cat *eval_coll=NULL;
        char *name=NULL;
        hbool_t is_coord;
        int assoc_ratio;
        SAF_Eval *eval_func=NULL;
        int num_comps;
        SAF_Field *components=NULL;
        Field_Info *component_fields=NULL;
        SAF_Interleave interleave;
        int *comp_order=NULL;
                                                                                                                                       
        field = field_info->the_field;
        ftarget = field_info->ftarget;
                                                                                                                                       
        base_space = (SAF_Set *)calloc((size_t)1, sizeof(SAF_Set));
                                                                                                                                       
        template_info = (FieldTmpl_Info *)calloc((size_t)1, sizeof(FieldTmpl_Info));
        field_template = (SAF_FieldTmpl *)calloc((size_t)1, sizeof(SAF_FieldTmpl));
                                                                                                                                       
        name = NULL;
        comp_order = NULL;
                                                                                                                                       
        coeff_assoc = (SAF_Cat *)calloc((size_t)1, sizeof(SAF_Cat));
        storage_decomp = (SAF_Cat *)calloc((size_t)1, sizeof(SAF_Cat));
        eval_coll = (SAF_Cat *)calloc((size_t)1, sizeof(SAF_Cat));
                                                                                                                                       
                                                                                                                                       
                                                                                                                                       
        saf_describe_field(SAF_ALL, field, field_template, &name, base_space, NULL, &is_coord, storage_decomp, \
                           coeff_assoc, &assoc_ratio, eval_coll, eval_func, NULL, \
                           &num_comps, NULL, &interleave, NULL);
                                                                                                                                       
        saf_get_count_and_type_for_field( SAF_ALL, field, ftarget, &count, &Ptype);

        template_info->fieldtmpl = field_template;

        get_fieldtmpl_info( template_info );

        if( strcmp(template_info->algtype,"field") )  {

                        name = NULL;
                        comp_order = NULL;
                        saf_describe_field(SAF_ALL, field, field_template, &name, NULL, NULL, &is_coord, NULL, \
                                        NULL, &assoc_ratio, NULL, eval_func, NULL, \
                                        &num_comps, &components, &interleave, NULL);
                }
                                                                                                                                       
        /* a bug with Ptype in saf_describe_field for some state fields requires that we
           use this call to get Ptype instead
        */
        saf_get_count_and_type_for_field(SAF_ALL, field,ftarget, NULL, &Ptype);
                                                                                                                                       
        field_info->template_info = template_info;
        field_info->coeff_assoc = coeff_assoc;
        field_info->assoc_ratio = assoc_ratio;
        field_info->eval_coll = eval_coll;
        field_info->storage_decomp = storage_decomp;
        field_info->field_name = name;
        field_info->eval_func = eval_func;
        /*              field_info->eval_func = evalfunc_string(&eval_func); */
        field_info->is_coord = (unsigned char)is_coord;
        field_info->data_size = count;
	field_info->data_type = dsltype_string( Ptype );
        field_info->base_space_name = saf_set_name(base_space);
        field_info->field_data = NULL;
        field_info->base_space = NULL;
                                                                                                                                                                                                                                                                               
        if( allsets != NULL ) {
           for( i = 0 ; i < allsets->num_sets; i++ ) {
                 if( SAF_EQUIV(base_space, &(allsets[i].the_set)) ) {
                    field_info->base_space = allsets + i;
                    break;
                   }
           }

            if( field_info->base_space == NULL ) {
               printf("get_field_info: field_info->base_space is still NULL, %s:%s\n",
                                        field_info->field_name, field_info->base_space_name);
               }
            for( i = 0 ; i < num_all_fields ; i++ ) {
                if(SAF_EQUIV(field_info->the_field, allfields[i].the_field)) {
                   allfields[i].base_space = field_info->base_space;
                }
             }
          }
                                                                                                                                                                                                                                                                               
                field_info->interleave = interleave_string( interleave );

                                                                                                                                       
                if( components == NULL ) {
                        field_info->component_fields = NULL;
                        field_info->num_comps = 0;
                }
                else {
                        field_info->num_comps = num_comps;
                        component_fields = (Field_Info *)calloc((size_t)num_comps, sizeof(Field_Info));
                        field_info->component_fields = component_fields;
                        for( i = 0; i < num_comps; i++ ) {
                                component_fields[i].the_field = components + i;
                                get_field_info( component_fields + i );
                                                                                                                                       
                        }
                }

                if( SAF_EQUIV(storage_decomp,saf_self) ) {
                  if( storage_decomp != NULL )
                    free(storage_decomp);
                  if( !strcmp( saf_cat_name(field_info->coeff_assoc), "stategroups") ) {
                    saf_describe_state_group(SAF_ALL, field_info->the_field, NULL, NULL, NULL, NULL, NULL, NULL, &count);
                    field_info->data_size = count;
                  }
                                                                                                                                                                                                                                                                               
                  field_info->storage_decomp=NULL;
                }
                                                                                                                                                                                                                                                                               

                                                                                                                                       
}
                                                                                                                                       

void get_fieldtmpl_info(  FieldTmpl_Info *template_info )
{

	int i, num_comps=0;
	SAF_FieldTmpl *field_template;
	/* SAF_Set *base_space; */
	/* SAF_AlgType algtype; */
	SAF_Algebraic algtype;
	SAF_Basis basis;
	FieldTmpl_Info *component_tmpls=NULL;
	SAF_FieldTmpl *components=NULL;
	char *name;


		name = NULL;
		/* base_space = (SAF_Set *)malloc(sizeof(SAF_Set)); */

		field_template = template_info->fieldtmpl;



		if( field_template == NULL ) {
			printf("field_template is NULL\n");
			return;
		}




		/* saf_describe_field_tmpl(SAF_ALL,*field_template,&name,base_space,&algtype,&basis,NULL,&num_comps,NULL); */
		saf_describe_field_tmpl(SAF_ALL,field_template,&name,&algtype,&basis,NULL,&num_comps,NULL);

		if( num_comps > 0 ) {
			saf_describe_field_tmpl(SAF_ALL,field_template,NULL,NULL,NULL,NULL,&num_comps,&components);
		}



		if( components == NULL ) {
			template_info->component_tmpls = NULL;
		}
		else {


			component_tmpls = (FieldTmpl_Info *)calloc((size_t)num_comps,sizeof(FieldTmpl_Info));
			template_info->component_tmpls = component_tmpls;
			for( i = 0; i < num_comps; i++ ) {
				component_tmpls[i].fieldtmpl = components + i;

				get_fieldtmpl_info( component_tmpls + i );
				
			}
		}

		template_info->fieldtmpl_name = name;
		template_info->num_comps = num_comps;
		/*
		template_info->base_space_name = saf_set_name(base_space);
		template_info->base_space = base_space;
		*/
		/* template_info->algtype =  algtype_string((int)algtype); */
		template_info->algtype =  algtype_string(&algtype); 
		template_info->basis = basis_string(&basis);
}


void find_collections(  Set_Info *the_set, int set_num )
{

	int i, num_colls;
	SAF_Set saf_set;
	SAF_Cat *saf_cats;
	Set_Info *tmp_set;
	char *name;
        int count;
	SAF_Role role;
	int tdim;
	SAF_CellType celltype;

	tmp_set = the_set + set_num;

	if (tmp_set->collections != NULL)
		return;



	saf_cats = NULL;
	saf_set = tmp_set->the_set;

	saf_find_collections(SAF_ALL, &saf_set, SAF_ANY_ROLE, SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM, SAF_DECOMP_TORF, \
		&num_colls, NULL);

	tmp_set->num_colls = num_colls;

	if( num_colls == 0 ) {
		tmp_set->collections = NULL;
		return;
	}

	tmp_set->collections = (Collection_Info *)(calloc((size_t)num_colls , sizeof(Collection_Info)));

	saf_find_collections(SAF_ALL, &saf_set, SAF_ANY_ROLE, SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM, SAF_DECOMP_TORF, \
		&num_colls, &saf_cats);
	
	for( i = 0 ; i < num_colls; i++ ) {

		name = NULL;
		saf_describe_category(SAF_ALL,saf_cats+i, &name, &role, &tdim);
		saf_describe_collection(SAF_ALL,&saf_set,saf_cats+i,&celltype,&count,NULL,NULL,NULL);
		tmp_set->collections[i].the_cat = saf_cats[i];
		tmp_set->collections[i].cat_name = name;
                tmp_set->collections[i].count = count;
                tmp_set->collections[i].the_set = saf_set;
                /*
                */
		/* printf("find_collections: %d:%s\n",(int)(saf_cats[i].theRow),name); */
		
		tmp_set->collections[i].child_setlink = NULL;
		tmp_set->collections[i].subset_relation = NULL;
		tmp_set->collections[i].num_subset_relations = -1;
		tmp_set->collections[i].topo_link = NULL;
		tmp_set->collections[i].topo_domain_id = NULL;
		tmp_set->collections[i].parent_setlink = NULL;
		tmp_set->collections[i].parent_colllink = -1;
	}
	if (saf_cats) free(saf_cats);
}


Set_Info * get_subsets( Set_Info *set_info, int coll_id )
{
        int i, j, as;
        int number_of_subsets=0;
        int num_rels;
        SAF_Rel *SAF_rels;
        SAF_Set *SAF_subsets=NULL;
        char *set_name;
        char *cat_name;
        SAF_SilRole srole;
        SAF_ExtendMode e_mode;
        int tdim;
        int found_equiv_set = 0;
                                                                                                                                                 
        Set_Info *subsets;
        Collection_Info cat_info;
                                                                                                                                                 
        cat_info = set_info->collections[coll_id];
	printf(".");
                                                                                                                                                 
        saf_find_sets(SAF_ALL,SAF_FSETS_SUBS,&(set_info->the_set), &(cat_info.the_cat), &number_of_subsets, &SAF_subsets);
                                                                                                                                                 
        if( number_of_subsets == 0 ) {
                subsets = NULL;
                set_info->collections[coll_id].child_setlink=NULL;
                return 0;
        }
        else {
                subsets = (Set_Info *)(calloc((size_t) number_of_subsets, sizeof(Set_Info)));
        }
        if( subsets == NULL ) {
                return 0;
        }
                                                                                                                                                 
        for( i = 0; i < number_of_subsets; i++ ) {
                set_name = NULL;
                saf_describe_set(SAF_ALL, SAF_subsets+i, &set_name, &tdim, &srole, &e_mode, NULL, NULL, NULL);
                                                                                                                                                 
                subsets[i].the_set = SAF_subsets[i];
                subsets[i].set_name = saf_set_name(SAF_subsets + i);
/*
                subsets[i].max_topo_dim = tdim;
                subsets[i].extmode = extmode_string( e_mode );
                subsets[i].silrole = silrole_string( srole );
*/
                subsets[i].num_sets = number_of_subsets;
                subsets[i].topo_relation = NULL;
                subsets[i].fields = NULL;
                subsets[i].num_fields=0;
                subsets[i].collections = NULL;

                found_equiv_set = 0;
                for( as = 0; as < num_all_sets; as++ ) {
                        if( SAF_EQUIV(&(allsets[as].the_set), &(subsets[i].the_set)) ) {
                                subsets[i].fields = allsets[as].fields;
                                subsets[i].num_fields = allsets[as].num_fields;
                                found_equiv_set = 1;
                        }
                }
                if( found_equiv_set != 1 )
                  printf("get_subsets: Could not find equiv set for: %s\n", subsets[i].set_name);

                find_collections(subsets, i);
                if( subsets[i].collections != NULL ) {
                        for( j = 0 ; j < subsets[i].num_colls; j++ ) {
                                                                                                                                                 
			 	get_subsets( subsets+i, j );
                                SAF_rels = NULL ;
                                                                                                                                                 
                                saf_find_subset_relations(SAF_ALL, db, &(set_info->the_set), \
                                        &(subsets[i].the_set), &(subsets[i].collections[j].the_cat),\
                                        &(subsets[i].collections[j].the_cat),SAF_BOUNDARY_FALSE,SAF_BOUNDARY_FALSE,\
                                        &num_rels,NULL);
                                                                                                                                                 
                                if( num_rels > 0 ) {
                                        cat_name = NULL;
                                        saf_describe_category(SAF_ALL, &(subsets[i].collections[j].the_cat), &cat_name, NULL,NULL);
                                        saf_find_subset_relations(SAF_ALL, db, &(set_info->the_set), \
                                                &(subsets[i].the_set), &(subsets[i].collections[j].the_cat),\
                                                &(subsets[i].collections[j].the_cat),SAF_BOUNDARY_FALSE,SAF_BOUNDARY_FALSE,\
                                                &num_rels,&SAF_rels);
                                                                                                                                                 
                                        /* the subset gets the list of subset relations between it */
                                        /* and the superset. */
                                        subsets[i].collections[j].num_subset_relations = num_rels;
                                        subsets[i].collections[j].subset_relation = SAF_rels;
                                                                                                                                                 
                                }
                                else {
                                        subsets[i].collections[j].num_subset_relations = 0;
                                        subsets[i].collections[j].subset_relation = NULL;
                                }
                                                                                                                                                 
                                subsets[i].collections[j].parent_setlink = (void *)set_info;
                                subsets[i].collections[j].parent_colllink = coll_id;
                                (set_info->collections[coll_id]).child_setlink = (void *)subsets;
                                                                                                                                                 
                        }
                }
        }
	if (SAF_subsets) free(SAF_subsets);
        return subsets;

}
Set_Info * oldoldget_subsets( Set_Info *set_info, int coll_id )
{
	int i, j, as, found_match = -1;
	int number_of_subsets=0;
	int num_rels;
	SAF_Rel *SAF_rels;
	SAF_Set *SAF_subsets=NULL;

	SAF_BoundMode sbmode, cbmode;
	
	Set_Info *subsets;
	Collection_Info cat_info;

	cat_info = set_info->collections[coll_id];

	printf(".");

	if( set_info->collections[coll_id].child_setlink != NULL ) 
		return (set_info->collections[coll_id].child_setlink);

	saf_find_sets(SAF_ALL,SAF_FSETS_SUBS,&(set_info->the_set), &(cat_info.the_cat), &number_of_subsets, &SAF_subsets);

	if( number_of_subsets < 1 ) {
		subsets = NULL;
		set_info->collections[coll_id].child_setlink=NULL;
	}
	else {
		subsets = (Set_Info *)(calloc((size_t)number_of_subsets , sizeof(Set_Info)));
		
	}

	if( subsets == NULL ) {
		return NULL;
	}

	set_info->collections[coll_id].child_setlink = subsets;

	for( i = 0; i < number_of_subsets; i++ ) {


		subsets[i].the_set = SAF_subsets[i];
		subsets[i].set_name = saf_set_name(SAF_subsets + i);
		
		get_set_info(subsets + i);       /* diff; only here */
		subsets[i].num_sets = number_of_subsets;
		subsets[i].topo_relation = NULL;
		subsets[i].fields = NULL;
		subsets[i].duplicate_set = NULL;  /* diff; only here */
		subsets[i].num_fields=0;
		subsets[i].collections = NULL;

		found_match = -1;
		for( as = 0 ; as < num_all_sets; as++ ) {

			if( SAF_EQUIV( &(allsets[as].the_set), SAF_subsets+i) ) {
			        found_match = as;
				subsets[i].fields = allsets[as].fields;
				subsets[i].num_fields = allsets[as].num_fields;
				break;
			}
		}
		if( found_match <  0 ) {
		  find_fields_on_set(subsets, i);  /* diff; call not made; print statement */
		}


		find_collections(subsets, i);


		if( subsets[i].collections != NULL ) {
			for( j = 0 ; j < subsets[i].num_colls; j++ ) {


				get_subsets( subsets+i, j );

				SAF_rels = NULL ;
				num_rels = 0;


				if( SAF_EQUIV( &(set_info->collections[coll_id].the_cat), &(subsets[i].collections[j].the_cat) ) ) {


				saf_find_subset_relations(SAF_ALL,db,&(set_info->the_set), \
					&(subsets[i].the_set), &(set_info->collections[coll_id].the_cat) ,\
					&(subsets[i].collections[j].the_cat),SAF_BOUNDARY_FALSE,SAF_BOUNDARY_FALSE,\
					&num_rels,NULL);
				}
				else {
				saf_find_subset_relations(SAF_ALL,db,&(set_info->the_set), \
					&(subsets[i].the_set), &(set_info->collections[coll_id].the_cat) ,\
					&(subsets[i].collections[j].the_cat),SAF_BOUNDARY_FALSE, SAF_BOUNDARY_FALSE,\
					/* &(subsets[i].collections[j].the_cat),SAF_BOUNDARY_FALSE, SAF_BOUNDARY_TRUE,\   */
					&num_rels,NULL);


				}
				    

				if( num_rels > 0 ) {

				  if( SAF_EQUIV(  &(set_info->collections[coll_id].the_cat), &(subsets[i].collections[j].the_cat) ) ) {
					saf_find_subset_relations(SAF_ALL,db,&(set_info->the_set), \
						&(subsets[i].the_set),&(set_info->collections[coll_id].the_cat),\
						&(subsets[i].collections[j].the_cat),SAF_BOUNDARY_FALSE,SAF_BOUNDARY_FALSE,\
						&num_rels,&SAF_rels);
				  }
				  else {
				saf_find_subset_relations(SAF_ALL,db,&(set_info->the_set), \
					&(subsets[i].the_set), &(set_info->collections[coll_id].the_cat) ,\
					/* &(subsets[i].collections[j].the_cat),SAF_BOUNDARY_FALSE,SAF_BOUNDARY_TRUE,\ */
					&(subsets[i].collections[j].the_cat),SAF_BOUNDARY_FALSE,SAF_BOUNDARY_FALSE,\
					&num_rels,NULL);


				  }


					/* the subset gets the list of subset relations between it */
					/* and the superset. */

					subsets[i].collections[j].num_subset_relations = num_rels;
					subsets[i].collections[j].subset_relation = SAF_rels;

					/*
					saf_describe_subset_relation(SAF_ALL, SAF_rels[0], NULL, NULL, &sbmode, &cbmode, NULL, NULL, NULL, NULL, NULL);
					if( sbmode == SAF_BOUNDARY_TRUE )
					  printf("smode is SAF_BOUNDARY_TRUE\n");
					else if( sbmode == SAF_BOUNDARY_FALSE )
					  printf("smode is SAF_BOUNDARY_FALSE\n");
					else if( sbmode == SAF_BOUNDARY_TORF )
					  printf("smode is SAF_BOUNDARY_TORF\n");
					else
					  printf("smode is What?\n");
					*/

					subsets[i].collections[j].parent_setlink = set_info;
					subsets[i].collections[j].parent_colllink = coll_id;

					/*
					printf("%s:%s:%s:%s,:%d:%d\n",set_info->set_name, subsets[i].set_name,\
					       set_info->collections[coll_id].cat_name, subsets[i].collections[j].cat_name, \
					       coll_id, j);
					*/
				}	
				else {
					subsets[i].collections[j].num_subset_relations = -2;
					subsets[i].collections[j].subset_relation = NULL;
					subsets[i].collections[j].parent_colllink = -1;
					subsets[i].collections[j].parent_setlink = NULL;
				}
				
				/*
				subsets[i].collections[j].parent_setlink = (void *)set_info;
				subsets[i].collections[j].parent_colllink = coll_id;
				(set_info->collections[coll_id]).child_setlink = (void *)subsets;
				
				*/
			}
		}



	}
	return subsets;
}



char *saf_set_name( SAF_Set *SAF_subset) 
{


	char *set_name=NULL;
	int tdim;
	int num_colls;
	SAF_TopMode t_mode;
	SAF_ExtendMode e_mode;
	SAF_SilRole srole;

	saf_describe_set(SAF_ALL, SAF_subset, &set_name, &tdim, &srole, &e_mode, &t_mode, &num_colls, NULL);

	return set_name;

}

void get_set_info( Set_Info *set_info)
{
	int tdim;
	int num_colls;
	SAF_TopMode t_mode;
	SAF_ExtendMode e_mode;
	SAF_SilRole srole;


  saf_describe_set(SAF_ALL, &(set_info->the_set), NULL, &tdim, &srole, &e_mode, &t_mode, &num_colls, NULL);
  set_info->silrole = srole;

}

char *saf_cat_name( SAF_Cat *the_cat)
{

  char *cat_name = NULL;
  static char *self_name = NULL;
  if (self_name == NULL)
    self_name = strdup("SAF_SELF");

  if( ! SAF_EQUIV( the_cat, saf_self) ) {
  	saf_describe_category(SAF_ALL, the_cat, &cat_name, NULL, NULL);
  }
  else
	return( self_name );

  return cat_name;

}

void rs( char *s )
{

  int i,len;
  
  len = strlen(s);
  for( i = 0 ; i < len; i++ ) {
    if( s[i] == ' ' ) {
      s[i] = '_';
    }
  }
  
}



  char * basis_string( SAF_Basis *basis ) {

	char *name = NULL;
 
        if (SS_BASIS(basis))
	   saf_describe_basis(SAF_ALL, basis, &name, NULL);

	return name;

  }

  char * evalfunc_string( SAF_Eval *eval) {

	char *name = NULL;

        if (SS_EVALUATION(eval))
	   saf_describe_evaluation(SAF_ALL, eval, &name, NULL );

	return name;

  }


  char * algtype_string(SAF_Algebraic *algtype) {

	char *name = NULL;

        if (SS_ALGEBRAIC(algtype)) 
	    saf_describe_algebraic(SAF_ALL, algtype, &name, NULL,NULL);

	return name;

  }

  char * role_string(int id) {

        char string_c[40];
	switch(id) {
		case 0: strcpy(string_c,"SAF_TOPOLOGY");
		case 1: strcpy(string_c,"VBT_ROLE_BND");
		case 2: strcpy(string_c,"SAF_PROCESSOR");
		case 3: strcpy(string_c,"SAF_DOMAIN");
		case 4: strcpy(string_c,"SAF_BLOCK");
		case 5: strcpy(string_c,"SAF_ASSEMBLY");
		case 6: strcpy(string_c,"SAF_MATERIAL");
		case 7: strcpy(string_c,"VBT_ROLE_XPROD");
		case 8: strcpy(string_c,"SAF_USERD");
		default: strcpy(string_c,"Undefined SAF_ROLE");
	}
        return(strdup(string_c));

  }

  char * silrole_string(SAF_SilRole role_id) {

    char string_c[40];
    strcpy(string_c,"SAF_UNKNOWN_SILROLE");
    if( role_id == SAF_SPACE )
      strcpy(string_c,"SAF_SPACE");
    if( role_id == SAF_TIME )
      strcpy(string_c,"SAF_TIME");
    if( role_id == SAF_PARAM)
      strcpy(string_c,"SAF_PARAM");
    if( role_id == SAF_SUITE )
      strcpy(string_c,"SAF_SUITE");
    return(strdup(string_c));
    

      /*
	switch(id) {
		case 0: return("SAF_TIME");
		case 1: return("SAF_SPACE");
		case 2: return("VBT_SROLE_STATE");
		case 3: return("SAF_PARAM");
		case 4: return("VBT_SROLE_CTYPE");
		case 5: return("VBT_SROLE_ATYPE");
		case 6: return("VBT_SROLE_USERD");
		case 7: return("SAF_ANY_SILROLE");
		default: return("Undefined SAF_SILROLE");
	}
      */

  }

  char * celltype_string(int id) {

	char string_c[40];
	switch(id) {
		case 0: strcpy(string_c,"SAF_SET");
		case 1: strcpy(string_c,"SAF_POINT");
		case 2: strcpy(string_c,"SAF_LINE");
		case 3: strcpy(string_c,"SAF_TRI");
		case 4: strcpy(string_c,"SAF_QUAD");
		case 5: strcpy(string_c,"SAF_TET");
		case 6: strcpy(string_c,"SAF_PYRAMID");
		case 7: strcpy(string_c,"SAF_PRISM");
		case 8: strcpy(string_c,"SAF_HEX");
		case 9: strcpy(string_c,"SAF_MIXED");
		case 10: strcpy(string_c,"SAF_ARB");
		case 11: strcpy(string_c,"SAF_1BALL");
		case 12: strcpy(string_c,"SAF_2BALL");
		case 13: strcpy(string_c,"SAF_3BALL");
		case 14: strcpy(string_c,"SAF_1SHELL");
		case 15: strcpy(string_c,"SAF_2SHELL");
		case 16: strcpy(string_c,"SAF_ANY_CELL_TYPE");
		default: strcpy(string_c,"Undefined CELLTYPE");
	}
    return(strdup(string_c));
   }

  char * objecttype_string(int id) {

	char string_c[40];
	switch(id) {
		case 1: strcpy(string_c,"_SAF_OBJTYPE_DB");
		case 2: strcpy(string_c,"_SAF_OBJTYPE_CAT");
		case 3: strcpy(string_c,"_SAF_OBJTYPE_FIELD");
		case 4: strcpy(string_c,"_SAF_OBJTYPE_FILE");
		case 5: strcpy(string_c,"_SAF_OBJTYPE_FTMPL");
		case 6: strcpy(string_c,"_SAF_OBJTYPE_REL");
		case 7: strcpy(string_c,"_SAF_OBJTYPE_SEQ");
		case 8: strcpy(string_c,"_SAF_OBJTYPE_SET");
		case 9: strcpy(string_c,"_SAF_OBJTYPE_STATE");
		case 10: strcpy(string_c,"_SAF_OBJTYPE_STMPL");
		case 11: strcpy(string_c,"_SAF_OBJTYPE_QUANTITY");
		case 12: strcpy(string_c,"_SAF_OBJTYPE_UNIT");
		case 13: strcpy(string_c,"_SAF_OBJTYPE_BASE");
		default: strcpy(string_c,"Undefined SAF_OBJTYPE");
	}
    return(strdup(string_c));
   }


void my_saf_init( void )  
{
        SAF_LibProps *libprops;
    int self = 0;
    int ntasks = 1;
    char * js = "java";
    char ** jp = &js;

#ifdef HAVE_PARALLEL
 
  MPI_Init(&ntasks, &jp  );
  MPI_Comm_rank(MPI_COMM_WORLD, &self);
  MPI_Comm_size(MPI_COMM_WORLD, &ntasks);
    
#endif
        libprops = saf_createProps_lib( );
     	saf_setProps_ErrorMode(libprops , SAF_ERRMODE_RETURN ) ; 

	saf_init(libprops);
	ss_saf_initialized = TRUE;

	import_array();
}


void describe_set( PyObject *pyob )
{

	int i;
	Set_Info *set_info;
	SAF_Set  *saf_set;
	PyObject *l_pyobject;

	if( pyob == NULL )
		return;

	l_pyobject = PyDict_GetItemString(pyob,"Set_handle");
	set_info = (Set_Info *)PyInt_AsLong(l_pyobject);

	/*
	set_info = (Set_Info *)pyob;
	*/

	saf_set = &(set_info->the_set);

	printf("%s:",set_info->set_name);
	printf("\n\tcollections:");
	for( i = 0; i < set_info->num_colls; i++ ) {
		printf("\n\t\t%s",set_info->collections[i].cat_name);
	}
	printf("\n");

}

void  sil( PyObject *pyob )
{

	SAF_Db *local_db;
	PyObject *l_pyobject;
	dbobject *localdbobj;
	int i,j;
	Set_Info *tops;

	
	if ( pyob == NULL  )
		return;

	/* localdbobj = (dbobject *)pyob; */
	l_pyobject = PyDict_GetItemString(pyob,"_db_handle");
	localdbobj = (dbobject *)PyInt_AsLong(l_pyobject);


	if( localdbobj == NULL )
		return;

	
	local_db = localdbobj->dbhandle->the_db;
	tops = localdbobj->tops;

	for( i = 0; i < tops->num_sets; i++ ) {
		for( j = 0 ; j < tops[i].num_colls; j++ ) {
			printf("\n%s\n",tops[i].collections[j].cat_name);
			recurse( tops+i, tops[i].collections[j].the_cat, 1);
		}

	}


}

void recurse( Set_Info *l_top , SAF_Cat the_cat, int level)
{


	int j,k;
	Set_Info tmp_set;
	Set_Info *subset;
	Collection_Info tmp_coll;


	if( l_top == NULL )
		return;


		tmp_set = l_top[0];
		level_print(level);
		printf("%s\n\n",tmp_set.set_name);

		for( j = 0 ; j < tmp_set.num_colls ; j++ ) {
			tmp_coll = tmp_set.collections[j];
			if( SAF_EQUIV(&(tmp_set.collections[j].the_cat), &the_cat) ) {
				
				subset = tmp_coll.child_setlink;
				if( subset != NULL ) {
					for( k = 0 ; k < subset->num_sets; k++ ) {
						recurse(subset+k,the_cat,level+1);
					}
				}
			}

		} 




}

void level_print( int level ) {

	int i;


	for( i = 0 ; i < level; i++ ) 
		printf("     ");



}


PyObject * py_subsets( dbobject *localdbobj, Set_Info *set, int coll_id )
{

  Set_Info *subsets;
  PyObject *the_py_subsets;



  subsets = (set->collections[coll_id].child_setlink);
  

  if( subsets == NULL ) {
    Py_INCREF(Py_None);
    return Py_None;
  }



  the_py_subsets = py_sets( localdbobj, subsets, subsets->num_sets );

  return the_py_subsets;


}

PyObject * py_fields( dbobject *localdbobj, Set_Info *set)
{

  PyObject *field_list;



  if( set == NULL ) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  if( set->fields == NULL ) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  field_list = py_fields_from_list( set->fields, set->num_fields );

  return field_list;
}

PyObject *py_fields_from_list( Field_Info * fields, int num_fields)
{

  PyObject *field_list, *comp_list, *comp_handle_list;
  Field_Info *tmp_field_info;
  PyObject *tmp_dict_obj;
  Field_Info *component_fields;
  char *coeff_assoc_name;
  int num_comps;

  int i,j;


  if( num_fields == 0 ) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  field_list = PyList_New(0);


  for( i = 0 ; i < num_fields ; i++ ) {
    
    tmp_field_info = fields + i;
    tmp_dict_obj = PyDict_New();

    rs(tmp_field_info->field_name);

    if( (tmp_field_info->base_space->silrole == SAF_SUITE) && (strcmp(saf_cat_name(tmp_field_info->coeff_assoc), "stategroups")) ) {
      continue;
    }

    PyDict_SetItem(tmp_dict_obj,PyString_FromString("name"),PyString_FromString(tmp_field_info->field_name));
    PyDict_SetItem(tmp_dict_obj,PyString_FromString("_handle"),PyInt_FromLong((long)(tmp_field_info)));
    num_comps = tmp_field_info->num_comps;
    PyDict_SetItem(tmp_dict_obj,PyString_FromString("num_comps"),PyInt_FromLong((long)(tmp_field_info->num_comps)));
    coeff_assoc_name = saf_cat_name(tmp_field_info->coeff_assoc);
    PyDict_SetItem(tmp_dict_obj,PyString_FromString("coeff_assoc"),PyString_FromString(coeff_assoc_name));
    PyDict_SetItem(tmp_dict_obj,PyString_FromString("data_size"),PyInt_FromLong((long)(tmp_field_info->data_size)));
    PyDict_SetItem(tmp_dict_obj,PyString_FromString("data_type"),PyString_FromString(tmp_field_info->data_type ));
    PyDict_SetItem(tmp_dict_obj,PyString_FromString("interleave"),PyString_FromString(tmp_field_info->interleave));
    PyDict_SetItem(tmp_dict_obj,PyString_FromString("base_space"),PyString_FromString(tmp_field_info->base_space_name));

    if (num_comps > 0 ) {


      component_fields = (tmp_field_info->component_fields);
      
      comp_list = py_fields_from_list( component_fields, num_comps );

      comp_handle_list = PyList_New(0);
      for (j = 0 ; j < num_comps; j++ ) {
	
	PyList_Append(comp_handle_list,PyInt_FromLong((long)(component_fields + j)));
      }
      Py_INCREF(comp_handle_list);
      PyDict_SetItem(tmp_dict_obj,PyString_FromString("_comp_handles"),comp_handle_list);
      PyDict_SetItem(tmp_dict_obj,PyString_FromString("_comp_fields"),comp_list);

    }
    PyDict_SetItem(tmp_dict_obj,PyString_FromString("type"),PyString_FromString("SAF_Field"));

    PyList_Append(field_list,tmp_dict_obj);
  }

  return field_list;


} 


PyObject * py_collections( dbobject *localdbobj, Set_Info *set )
{

  /* Set_Info *set; */
  PyObject *coll_list; 
  Collection_Info *tmp_coll_info;
  PyObject *tmp_dict_obj;
  PyObject *subsets;

  int i;

  /* set = PyDict_GetItemString(pyob,"_handle"); */

  if (  set == NULL ) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  if( set->collections == NULL ) {
    Py_INCREF(Py_None);
    return Py_None;
  }
			    

  coll_list = PyList_New(0);
  for( i = 0 ; i < set->num_colls ; i++ ) {

    tmp_coll_info = set->collections + i;
    tmp_dict_obj = PyDict_New();

    rs(tmp_coll_info->cat_name);

    PyDict_SetItem(tmp_dict_obj,PyString_FromString("name"),PyString_FromString(tmp_coll_info->cat_name));
    PyDict_SetItem(tmp_dict_obj,PyString_FromString("_handle"),PyInt_FromLong((long)(tmp_coll_info)));
    PyDict_SetItem(tmp_dict_obj,PyString_FromString("_coll_num"),PyInt_FromLong((long)(i)));
    
    PyDict_SetItem(tmp_dict_obj,PyString_FromString("type"),PyString_FromString("SAF_Cat"));
    PyDict_SetItem(tmp_dict_obj,PyString_FromString("count"),PyInt_FromLong((long)(tmp_coll_info->count)));

    subsets = py_subsets( localdbobj, set, i  );
    PyDict_SetItem(tmp_dict_obj,PyString_FromString("subsets"),subsets);

    PyList_Append(coll_list, tmp_dict_obj);

  }

  return coll_list;
  
}


PyObject * get_topsets( PyObject *pyob )
{

  PyObject *l_pyobject;
  PyObject *py_topsets;
  dbobject *localdbobj;

  l_pyobject = PyDict_GetItemString(pyob,"_db_handle");
  localdbobj = (dbobject *)PyInt_AsLong(l_pyobject);




  py_topsets = py_sets( localdbobj, localdbobj->tops, num_top_sets );


  return py_topsets;
  
  
}

PyObject * get_suites( PyObject *pyob )
{
  PyObject *l_pyobject;
  PyObject *py_suites;
  dbobject *localdbobj;

  l_pyobject = PyDict_GetItemString(pyob,"_db_handle");
  localdbobj = (dbobject *)PyInt_AsLong(l_pyobject);

  if( localdbobj->num_suites <= 0 ) {

	  Py_INCREF(Py_None);
	  return Py_None;
  }

  py_suites = py_sets( localdbobj, localdbobj->suites, localdbobj->suites->num_sets );

  return py_suites;

}

PyObject * py_sets( dbobject *localdbobj , Set_Info *l_set_info, int number_of_sets ) 
{

	int i;
	Set_Info *sets;

	Set_Info *tmp_set_info;
        PyObject *set_list = NULL;
	PyObject *tmp_dict_obj;
	PyObject *coll_list;
	PyObject *field_list;
	SAF_Db *local_db;
	SAF_SilRole srole;


	if( localdbobj == NULL ) {
	  printf("ss.c: topsets: localdbobj is NULL\n");
	  Py_INCREF(Py_None);
	  return Py_None;
	}


	local_db = localdbobj->dbhandle->the_db;
	/* tops = localdbobj->tops; */
	sets = l_set_info;


	if(l_set_info == NULL ) {
	  Py_INCREF(Py_None);
	  return Py_None;
	}

	/*
	if( tops_list != NULL ) {
	  Py_INCREF(tops_list);
	  return tops_list;
	}

	printf("ss.c: topsets()\n");
	*/


	set_list = PyList_New(0);
	for( i = 0 ; i < number_of_sets ; i++ ) {
	  tmp_set_info = sets + i;

	  tmp_dict_obj = PyDict_New();

	  rs(tmp_set_info->set_name);

	  tmp_set_info->row_id  = 86;  /* some pos num; no longer used */

	  PyDict_SetItem(tmp_dict_obj,PyString_FromString("name"),PyString_FromString(tmp_set_info->set_name));
	  PyDict_SetItem(tmp_dict_obj,PyString_FromString("_handle"),PyInt_FromLong((long)(sets+i)));
          PyDict_SetItem(tmp_dict_obj,PyString_FromString("_row_id"),PyInt_FromLong((long)(tmp_set_info->row_id)));
	  
	  srole = tmp_set_info->silrole;
	  PyDict_SetItem(tmp_dict_obj,PyString_FromString("silrole"), PyString_FromString( silrole_string(srole) ) );


	  if( srole == SAF_SPACE )
	    PyDict_SetItem(tmp_dict_obj,PyString_FromString("type"),PyString_FromString("SAF_Set"));
	  else if ( srole == SAF_TIME )
	    PyDict_SetItem(tmp_dict_obj,PyString_FromString("type"),PyString_FromString("SAF_Time"));
	  else if ( srole == SAF_PARAM )
	    PyDict_SetItem(tmp_dict_obj,PyString_FromString("type"),PyString_FromString("SAF_Param"));
	  else
	    PyDict_SetItem(tmp_dict_obj,PyString_FromString("type"),PyString_FromString("SAF_Unknown"));

	  /*
	  PyDict_SetItem(tmp_dict_obj,PyString_FromString("type"),PyString_FromString("SAF_Set"));
	  */

	  coll_list = py_collections( localdbobj, sets + i );
	  field_list = py_fields( localdbobj, sets + i );
	  PyDict_SetItem(tmp_dict_obj,PyString_FromString("collections"),coll_list);
	  PyDict_SetItem(tmp_dict_obj,PyString_FromString("fields"),field_list);
	  /* PyDict_SetItem(tmp_dict_obj,PyString_FromString("db"),pyob); */ 

	  PyList_Append(set_list, tmp_dict_obj);
	}


	return set_list;
}
PyObject * py_states( dbobject *localdbobj , Set_Info *l_set_info) 
{

	int i;
	Set_Info *sets;

	Set_Info *tmp_set_info;
        PyObject *set_list = NULL;
	PyObject *tmp_dict_obj;
	PyObject *coll_list;
	PyObject *field_list;
	SAF_Db *local_db;



	if( localdbobj == NULL ) {
	  printf("ss.c: topsets: localdbobj is NULL\n");
	  Py_INCREF(Py_None);
	  return Py_None;
	}

	local_db = localdbobj->dbhandle->the_db;
	/* tops = localdbobj->tops; */
	sets = l_set_info;

	if(l_set_info == NULL ) {
	  Py_INCREF(Py_None);
	  return Py_None;
	}


	/*
	if( tops_list != NULL ) {
	  Py_INCREF(tops_list);
	  return tops_list;
	}

	printf("ss.c: topsets()\n");
	*/


	set_list = PyList_New(0);
	for( i = 0 ; i < sets->num_sets ; i++ ) {
	  tmp_set_info = sets + i;
	  tmp_dict_obj = PyDict_New();

	  rs(tmp_set_info->set_name);

	  PyDict_SetItem(tmp_dict_obj,PyString_FromString("name"),PyString_FromString(tmp_set_info->set_name));
	  PyDict_SetItem(tmp_dict_obj,PyString_FromString("_handle"),PyInt_FromLong((long)(sets+i)));
	  PyDict_SetItem(tmp_dict_obj,PyString_FromString("type"),PyString_FromString("SAF_Set"));
	  coll_list = py_collections( localdbobj, sets + i );
	  field_list = py_fields( localdbobj, sets + i );
	  PyDict_SetItem(tmp_dict_obj,PyString_FromString("collections"),coll_list);
	  PyDict_SetItem(tmp_dict_obj,PyString_FromString("fields"),field_list);
	  /* PyDict_SetItem(tmp_dict_obj,PyString_FromString("db"),pyob); */ 

	  PyList_Append(set_list, tmp_dict_obj);
	}

	return set_list;
}

PyObject * get_subset_relation_data( PyObject *py_set , PyObject *py_coll)
{

  PyObject *l_pyobject;
  int coll_num;
  Set_Info *set_info;
  SAF_Rel *rel;
  static size_t abuf_sz = 0;
  static size_t bbuf_sz = 0;
  hid_t abuf_type, bbuf_type;
  void *rbuf = NULL;
  int count = 2;
  


  l_pyobject = PyDict_GetItemString(py_set, "_handle");
  set_info = (Set_Info *)PyInt_AsLong(l_pyobject);
  l_pyobject = PyDict_GetItemString(py_coll,"_coll_num");
  coll_num = (int)(PyInt_AsLong(l_pyobject));


  rel = set_info->collections[coll_num].subset_relation;

  abuf_sz = bbuf_sz = 0;

  if( rel == NULL ) {
    
    Py_INCREF(Py_None);
    return Py_None;
  }



  saf_get_count_and_type_for_subset_relation(SAF_ALL,rel,NULL,&abuf_sz,&abuf_type,&bbuf_sz,&bbuf_type); 


  saf_read_subset_relation(SAF_ALL,rel, NULL, (void **)&rbuf, NULL);


  count = abuf_sz;

  l_pyobject = PyArray_FromDimsAndData( 1, &count, PyArray_INT,(char *)rbuf);

  /* Py_INCREF(l_pyobject); */

  return l_pyobject;

}

PyObject * remap_and_read_field( PyObject *self, PyObject *py_field )
{
  PyObject *l_pyobject = NULL;
  Field_Info *field_info = NULL, *new_field_info = NULL;
  SAF_Field *new_field = NULL;
  SAF_FieldTarget ftarg;
  size_t    dofCount;
  hid_t     dofType;


  l_pyobject = PyDict_GetItemString(py_field,"_handle");
  field_info  = (Field_Info *)PyInt_AsLong(l_pyobject);

  new_field = (SAF_Field *)calloc((size_t)1,sizeof(SAF_Field));
  *new_field = *(field_info->the_field);
  
  /* Remap and set datatype to nothing */
  saf_target_field(&ftarg, NULL, SAF_SELF(db),
                          SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, H5I_INVALID_HID,
                          SAF_INTERLEAVE_COMPONENT, NULL);
  dofCount = 0;
  dofType  = H5I_INVALID_HID;
  saf_get_count_and_type_for_field(SAF_ALL, new_field, &ftarg, &dofCount, &dofType);


  new_field_info = (Field_Info *)calloc((size_t)1, sizeof(Field_Info) );
  new_field_info->the_field = new_field;

  get_field_info(new_field_info);

  return py_fields_from_list( new_field_info, 1);

}


PyObject * get_state_field_data( PyObject *self, PyObject *py_field )
{

  SAF_Field *saf_field = NULL;
  SAF_Field *tmp_field = NULL;
  SAF_Field *the_fields = NULL;
  Field_Info *field_info = NULL;
  char *field_name = NULL;
  SAF_Field *fbuf = NULL;
  Field_Info *new_field_infos = NULL;
  int num_states = 0;
  int coll_num, num_fields_on_state;
  hid_t Ptype;
  SAF_Set base_space;
  PyObject *l_pyobject = NULL;
  PyObject *py_fields_on_state = NULL;
  PyObject *py_states = NULL;
  PyObject *py_coords = NULL;
  PyObject *py_state_data = NULL;

  SAF_Field *stategrp_contents, *stategrp_state;
  SAF_Field *coord,*coords,*param_coord;
  void *coords_data = NULL;
  int index[1];
  int i, j, k, l, the_count = 0;


  l_pyobject = PyDict_GetItemString(py_field,"_handle");
  field_info  = (Field_Info *)PyInt_AsLong(l_pyobject);

  saf_field = field_info->the_field;

  stategrp_contents = NULL;
  index[0] = 0;
  saf_read_field (SAF_ALL, saf_field, NULL, 1, SAF_TUPLES, index, (void **)(&stategrp_contents));
  coord = &(stategrp_contents[0]);
  stategrp_state = &(stategrp_contents[1]);

    /* now read the coord field and get the mesh coord and the param coord (dump times) */
  coords = NULL;
  saf_read_field (SAF_ALL, coord, NULL, 1, SAF_TUPLES, index, (void **)(&coords));
  /* mesh_coord = &(coords[1]); */
  param_coord = &(coords[0]);
  saf_read_field(SAF_ALL, param_coord, NULL, SAF_WHOLE_FIELD, &coords_data);

  saf_get_count_and_type_for_field( SAF_ALL, param_coord, NULL, &the_count, &Ptype);


  if (!Ptype) 
    py_coords = Py_None;
  else if( H5Tequal(Ptype, SAF_DOUBLE) ) {
    py_coords = PyArray_FromDimsAndData( 1, &the_count,PyArray_DOUBLE,(char *)coords_data);
  }
  else if( H5Tequal(Ptype, SAF_FLOAT) ){
    py_coords = PyArray_FromDimsAndData( 1, &the_count,PyArray_FLOAT,(char *)coords_data);
  }
  else if( H5Tequal(Ptype, SAF_INT) ) {
    py_coords = PyArray_FromDimsAndData( 1, &the_count, PyArray_INT, (char *)coords_data);
  }
  else
    py_coords = Py_None;

  Py_INCREF(py_coords);




  coll_num = field_info->my_set_coll_num;

  saf_describe_state_group( SAF_ALL, saf_field, NULL, NULL, NULL, NULL, NULL, NULL, &num_states);

  if( num_states <= 0 ) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  if( strcmp( field_info->base_space->collections[coll_num].cat_name, "stategroups") ) {
    /* this is not a stategroup field */
    Py_INCREF(Py_None);
    return Py_None;
  }

  py_states = PyList_New(0);

  for( i = 0 ; i < num_states ; i++ ) {

    fbuf = NULL;
    saf_read_state( SAF_ALL, saf_field, i, NULL, NULL, NULL, &fbuf);

    the_fields = (SAF_Field *)fbuf;
    num_fields_on_state = field_info->template_info->component_tmpls[1].num_comps;
    new_field_infos = (Field_Info *)calloc((size_t) num_fields_on_state , sizeof(Field_Info) );


    for( j = 0 ; j < num_fields_on_state ; j++ ) {

      field_name = NULL;
      saf_describe_field(SAF_ALL, the_fields + j, NULL, &field_name, &base_space, NULL, NULL, NULL, \
			 NULL, NULL, NULL, NULL, NULL, \
			 NULL, NULL, NULL, NULL);


      for( k = 0 ; k < allsets->num_sets ; k++ ) {

	if( SAF_EQUIV( &base_space, &(allsets[k].the_set) )  ) {
	  for( l = 0 ; l < allsets[k].num_fields ; l++ ) {
	    tmp_field = allsets[k].fields[l].the_field;
	    if( SAF_EQUIV( the_fields+j, tmp_field )  ) {
	      new_field_infos[j] = allsets[k].fields[l];
	      break;
	    }
	  }

	  break;
	}
      }
    }

    py_fields_on_state = py_fields_from_list( new_field_infos, num_fields_on_state );

    PyList_Append( py_states, py_fields_on_state );

  }


  py_state_data = PyDict_New();
    
  PyDict_SetItem(py_state_data,PyString_FromString("_timesteps"),py_coords);
  PyDict_SetItem(py_state_data,PyString_FromString("_states"), py_states);
    
  /* return py_states; */
  return py_state_data;

}
void  get_topo_rel_data( PyObject *self, PyObject *py_allsets,
                                    char **sup_set_name,
                                    char **sup_cat_name,
                                    char **sub_set_name,
                                    char **sub_cat_name,
                                    PyObject **a_l_pyobject,
                                    PyObject **b_l_pyobject
                             )
                                   
{
        int count = 0;
        PyObject *l_pyobject = NULL;
        void *rbuf = NULL;
        Set_Info *allsets_info = NULL;
        SAF_Set saf_set;
        SAF_Cat saf_cat;
        SAF_CellType celltype;
        SAF_IndexSpec ispec;
        SAF_DecompMode decomp;
        SAF_Rel *rel;

        static void *abuf = NULL;
        static void *bbuf = NULL;
        static size_t abuf_sz = 0;
        static size_t bbuf_sz = 0;
        hid_t abuf_type, bbuf_type;

        SAF_Set sup_set, sub_set;
        SAF_Cat sup_cat, sub_cat;
                                                                                                                                     
        l_pyobject = PyDict_GetItemString(py_allsets,"_handle");
        allsets_info  = (Set_Info *)PyInt_AsLong(l_pyobject);
                                                                                                                                     
        find_topo_relations( allsets_info );
        rel = allsets_info->topo_relation;

                                                                                                                                     
        count = allsets_info->num_topo_relations;
                                                                                                                                     
        abuf_sz = 0;
        *a_l_pyobject = NULL;
        *b_l_pyobject = NULL;
                                                                                                                                     
        if( rel != NULL ) {
             saf_describe_subset_relation(SAF_ALL, rel,  &sup_set, &sub_set, &sup_cat, &sub_cat, NULL, NULL, NULL, NULL);
                                                                                                                                     
             *sup_set_name = NULL;
             *sup_cat_name = NULL;
             *sub_set_name = NULL;
             *sub_cat_name = NULL;
             saf_describe_set(SAF_ALL,&sup_set, sup_set_name, NULL, NULL, NULL, NULL, NULL, NULL);
             saf_describe_category(SAF_ALL, &sup_cat, sup_cat_name, NULL, NULL);
             saf_describe_set(SAF_ALL,&sub_set, sub_set_name, NULL, NULL, NULL, NULL, NULL, NULL);
             saf_describe_category(SAF_ALL, &sub_cat, sub_cat_name, NULL, NULL);

             abuf = bbuf = NULL;
             abuf_sz = bbuf_sz = 0;
             saf_get_count_and_type_for_topo_relation(SAF_ALL,rel,NULL,NULL,&abuf_sz,&abuf_type,&bbuf_sz,&bbuf_type);
             saf_read_topo_relation(SAF_ALL, rel, NULL, &abuf, &bbuf);
             count = abuf_sz;
             *a_l_pyobject = PyArray_FromDimsAndData( 1, &count, PyArray_INT,(char *)abuf);
             count = bbuf_sz;
             *b_l_pyobject = PyArray_FromDimsAndData( 1, &count, PyArray_INT,(char *)bbuf);
        }
}

PyObject * is_there_topo_rel_data( PyObject *self, PyObject *py_allsets )
{
        int count = 1;
        PyObject *l_pyobject = NULL;
        Set_Info *allsets_info = NULL;
        SAF_Rel *rel;
                                                                                                                                     

        l_pyobject = PyDict_GetItemString(py_allsets,"_handle");
        allsets_info  = (Set_Info *)PyInt_AsLong(l_pyobject);

        find_topo_relations( allsets_info );
        rel = allsets_info->topo_relation;
                                                                                                                                     
                                                                                                                                     
        /*
        int num_rels;
        rel = NULL;
        saf_find_topo_relations(SAF_ALL, db, &(allsets_info->the_set),NULL,&num_rels,NULL);
        saf_find_topo_relations(SAF_ALL, db, &(allsets_info->the_set),NULL,&num_rels,&rel);
        */


        if( rel == NULL ) {
                                                                                                                                     
            Py_INCREF(Py_None);
            return Py_None;
         }
        count = 1;
        l_pyobject = PyArray_FromDimsAndData( 1, &count, PyArray_INT,(char *)rel);
                                                                                                                                     
        /* Py_INCREF(l_pyobject); */
       return l_pyobject;
}


PyObject * get_collection_data( PyObject *self, PyObject *py_collection )
{
        int count = 0;
        PyObject *l_pyobject = NULL;
        void *rbuf = NULL;
	Collection_Info *collection_info = NULL;
        SAF_Set saf_set;
        SAF_Cat saf_cat;
	SAF_CellType celltype;
        SAF_IndexSpec ispec;
        SAF_DecompMode decomp;
        SAF_Rel *rel;
        static size_t abuf_sz = 0;
        static size_t bbuf_sz = 0;
        hid_t abuf_type, bbuf_type;

	l_pyobject = PyDict_GetItemString(py_collection,"_handle");
	collection_info  = (Collection_Info *)PyInt_AsLong(l_pyobject);
        saf_set = collection_info->the_set;
        saf_cat = collection_info->the_cat;
        saf_describe_collection(SAF_ALL,&saf_set,&saf_cat,&celltype,&count,&ispec,&decomp,NULL);

#if 0
        printf("I'm in get_collection_data  in ss.c collection_info.name: %s \n",collection_info->cat_name);
        printf("I'm in get_collection_data  in ss.c collection_info.count: %d \n",collection_info->count);
        printf("I'm in get_collection_data  in ss.c collection_info.the_cat: %d \n",collection_info->the_cat);
        printf("I'm in get_collection_data  in ss.c collection_info.the_set: %d \n",collection_info->the_set);
        printf("I'm in get_collection_data  in ss.c collection_info.subset_relation: %d \n",collection_info->subset_relation);
        printf("I'm in get_collection_data  in ss.c collection_info.numsubset_relation: %d \n",collection_info->num_subset_relations);
        printf("I'm in get_collection_data  in ss.c count: %d \n",count);
        printf("I'm in get_collection_data  in ss.c celltype: %d \n",celltype);
        printf("I'm in get_collection_data  in ss.c ispec.ndims: %d \n",ispec.ndims);
        for(i=0;i<ispec.ndims;i++)
           printf("ispec.order[%d]: %d  ispec.origin[%d] %d\n",i,ispec.order[i],i,ispec.origins[i]);
#endif

        rel = collection_info->subset_relation;
                                                                                                                                     
        abuf_sz = bbuf_sz = 0;
                                                                                                                                     
        if( rel == NULL ) {
                                                                                                                                     
            Py_INCREF(Py_None);
            return Py_None;
         }
                                                                                                                                     
        saf_get_count_and_type_for_subset_relation(SAF_ALL,rel,NULL,&abuf_sz,&abuf_type,&bbuf_sz,&bbuf_type);
                                                                                                                                     
                                                                                                                                     
        saf_read_subset_relation(SAF_ALL,rel, NULL, (void **)&rbuf, NULL);
                                                                                                                                     
                                                                                                                                     
        count = abuf_sz;
                                                                                                                                     
        l_pyobject = PyArray_FromDimsAndData( 1, &count, PyArray_INT,(char *)rbuf);
                                                                                                                                     
        /* Py_INCREF(l_pyobject); */
       return l_pyobject;
                                                                                                                                     
}

      

PyObject * get_field_data( PyObject *self, PyObject *py_field )
{
	static size_t count = 0;
	int the_count = 0;
	void  *pbuf = NULL;
	SAF_Field *saf_field = NULL;
	SAF_Field the_field;
	Field_Info *field_info = NULL;
	static hid_t Ptype;
	PyObject *l_pyobject = NULL;
	PyObject *ret = NULL;
	int num_comps, comp_count;
        int dimensions[2];
        /* int *strides    = NULL; */

	l_pyobject = PyDict_GetItemString(py_field,"_handle");
	field_info  = (Field_Info *)PyInt_AsLong(l_pyobject);
	
	the_field = *(field_info->the_field);

	num_comps = field_info->num_comps;

	saf_field = field_info->the_field;
	saf_get_count_and_type_for_field( SAF_ALL, saf_field, NULL, &count, &Ptype);

	if( count == 0 ) {

		Py_INCREF(Py_None);
		return Py_None;
	}

        if (!Ptype) 
		return Py_None;
        else
          { if( H5Tequal(Ptype, SAF_HANDLE) ) {
	       if( field_info->storage_decomp == NULL ) {
	        /* state field */
	        return get_state_field_data( self, py_field );
	      }
	      else  
	        return remap_and_read_field( self, py_field );
          }
	}


        if( num_comps > 1 ) {
	  
	  /* strides    = (int *)(malloc( num_comps * sizeof(int) )); */

	  comp_count = count / num_comps;

	  if( !strcmp(field_info->interleave,"VECTOR") ) {
	    dimensions[0] = comp_count;
	    dimensions[1] = num_comps;
	  }
	  else {
	    dimensions[0] = num_comps;
	    dimensions[1] = comp_count;
	  }
	  
	}

	saf_read_field(SAF_ALL, saf_field, NULL, SAF_WHOLE_FIELD, &pbuf);
	the_count = count;

	if( H5Tequal(Ptype, SAF_DOUBLE) ) {
	  /* value = (float)(fbuf[0]);*/
	  if( num_comps > 1 )
	        ret = PyArray_FromDimsAndData( 2, dimensions, PyArray_DOUBLE,(char *)pbuf);
	  else
		ret = PyArray_FromDimsAndData( 1, &the_count,PyArray_DOUBLE,(char *)pbuf);
	}
	else if( H5Tequal(Ptype, SAF_FLOAT) ){
	  /* value =  (float)( ((float *)fbuf)[0] ); */
	  if( num_comps > 1 )
	    ret = PyArray_FromDimsAndData( 2, dimensions, PyArray_FLOAT, (char *)pbuf);
	  else
	    ret = PyArray_FromDimsAndData( 1, &the_count,PyArray_FLOAT,(char *)pbuf);
	}
	else if( H5Tequal(Ptype, SAF_INT) ) {
	  if( num_comps > 1 )
	    ret = PyArray_FromDimsAndData( 2, dimensions, PyArray_INT, (char *)pbuf);
	  else
	    ret = PyArray_FromDimsAndData( 1, &the_count, PyArray_INT, (char *)pbuf);
	}

	Py_INCREF(ret);
	return ret;
	
}

  char * dsltype_string( hid_t type ) {

                char string[40];
                strcpy(string, "Unknown");
                if (type > 0)
                  {
                   if(H5Tequal(type, SAF_CHAR) ) strcpy(string,"SAF_CHAR");
                   if(H5Tequal(type, SAF_FLOAT)) strcpy(string,"SAF_FLOAT");
                   if(H5Tequal(type, SAF_HANDLE)) strcpy(string,"SAF_HANDLE");
                   if(H5Tequal(type, SAF_INT)) strcpy(string,"SAF_INT");
                   if(H5Tequal(type, SAF_LONG)) strcpy(string,"SAF_LONG");
                   if(H5Tequal(type, SAF_DOUBLE)) strcpy(string,"SAF_DOUBLE");
                   if(H5T_STRING==H5Tget_class(type)) strcpy(string,"SAF_STRING");
                  }
                return(strdup(string));

  }


char * interleave_string( SAF_Interleave interleave )
{
  char string[40];
  strcpy(string, "NONE");
  if( interleave == SAF_INTERLEAVE_VECTOR )     strcpy(string,"VECTOR");
  if( interleave == SAF_INTERLEAVE_COMPONENT)   strcpy(string,"COMPONENT");
  if( interleave == SAF_INTERLEAVE_INDEPENDENT) strcpy(string,"INDENPENDENT");
  return(strdup(string));
}


SAF_Db * new_database( char *name )
{
  SAF_DbProps *dbprops;
  SAF_Db *newdb;
  
      dbprops = saf_createProps_database();
      saf_setProps_Clobber(dbprops);
      newdb = saf_open_database(name,dbprops);

      return newdb;

}


void new_sil( DB_Info *new_db_info, Set_Info *top_info)
{


  int i, j, k, l;
  SAF_Db *the_db;
  SAF_Rel *rels;
  Set_Info *subsets, *a_set_info;
  int num_rels, num_found_rels;

  Set_Info *new_sup_set = NULL, *new_sub_set = NULL;
  SAF_Cat *new_sup_cat = NULL, *new_sub_cat = NULL;
  SAF_Rel *new_rel = NULL;
  SAF_BoundMode  sbmode, cbmode;
  SAF_RelRep     srtype;
  hid_t       a_buf_type, b_buf_type;
  size_t         a_size, b_size;



  the_db = new_db_info->the_db;


  if( new_db_info->allsets == NULL ) {
    new_db_info->allsets = (Set_Info *)calloc((size_t) (allsets->num_sets) , sizeof(Set_Info));
    new_db_info->num_all_sets = allsets->num_sets;
    new_db_info->allsets->num_sets = allsets->num_sets;

    for( i = 0 ; i < allsets->num_sets; i++ ) {
    
      new_allsets( new_db_info, allsets, i ); 

    } 
  }

  if( top_info->duplicate_set == NULL ) {
    a_set_info = (Set_Info *)calloc((size_t)1, sizeof(Set_Info));
    new_set( new_db_info, top_info, a_set_info);
  }
  else
    a_set_info = (top_info->duplicate_set);

  new_subsets( new_db_info, top_info, 0 );

  for( i = 0 ; i < top_info->num_colls; i++ ) {

    subsets = (top_info->collections[i].child_setlink);

    if( subsets == NULL ) {
      continue;
    }

    num_rels = 0;

    for( j = 0 ; j < subsets->num_sets; j++ ) {

      for ( k = 0 ; k < subsets[j].num_colls; k++ ) {

	if( subsets[j].collections[k].parent_setlink == top_info ) {
	  if( subsets[j].collections[k].parent_colllink == i ) { 

	    /*
	    printf("\tcoll_match: %s:%s:%s:%s,%d:%d\n", top_info->set_name, subsets[j].set_name, \
		   top_info->collections[i].cat_name, subsets[j].collections[k].cat_name, \
		   subsets[j].collections[k].parent_colllink, k);
	    */
	    

	    num_rels = subsets[j].collections[k].num_subset_relations;
	    rels     = subsets[j].collections[k].subset_relation;   
	    if( num_rels > 0 ) {
	      for( l = 0 ; l < num_rels ; l++ ) {

		new_sup_set = (top_info->duplicate_set);
		new_sub_set = (subsets[j].duplicate_set);

		if( new_sup_set == NULL ) {
		  printf("new_sup_set is NULL\n");
		  return;
		}

		if( new_sub_set == NULL ) {
		  printf("new_sub_set is NULL\n");
		  return;
		}

		new_sup_cat = &(new_sup_set->collections[i].the_cat);
		new_sub_cat = &(new_sub_set->collections[k].the_cat);


		saf_describe_subset_relation(SAF_ALL, rels+l, NULL, NULL, NULL, NULL, &sbmode, &cbmode, &srtype, NULL);
		saf_get_count_and_type_for_subset_relation( SAF_ALL, rels+l,NULL, &a_size, &a_buf_type, &b_size, &b_buf_type );
		

	    num_found_rels = 0;

		saf_find_subset_relations(SAF_ALL,the_db,&(new_sup_set->the_set), \
			&(new_sub_set->the_set), new_sup_cat ,\
			new_sub_cat,SAF_BOUNDARY_FALSE,cbmode,\
			&num_found_rels,NULL);

		if( num_found_rels < num_rels ) {

		  /*
		  printf("\ndeclaring new subset relation: %s:%s:%s:%s\n", new_sup_set->set_name, new_sub_set->set_name, \
		       new_sup_set->collections[i].cat_name, new_sub_set->collections[k].cat_name);
		  */

			new_rel = (SAF_Rel *)calloc((size_t)1,sizeof(SAF_Rel));
			  saf_declare_subset_relation( SAF_ALL,the_db, &(new_sup_set->the_set), &(new_sub_set->the_set), \
					     new_sup_cat, new_sub_cat, sbmode, cbmode, &srtype, \
					     a_buf_type, NULL, a_buf_type, NULL, new_rel );

		}

	      }
	    }
	  }
	}
      }
      
      new_sil( new_db_info, subsets + j );

    }
  }
}




void new_subsets( DB_Info *new_db_info, Set_Info *head_set_info, int set_num )
{

  int i, j;
  Set_Info *set_info, *new_set_info, *new_subs, *old_subs;
  SAF_Set *old_set;

  set_info = head_set_info + set_num;
  new_set_info = (set_info->duplicate_set);

  if( new_set_info == NULL ) {
    new_set_info = (Set_Info *)calloc((size_t)1, sizeof(Set_Info ));
    new_set( new_db_info, set_info, new_set_info );
  }

  for( i = 0; i < set_info->num_colls; i++ ) {

    old_subs = (set_info->collections[i].child_setlink);

	
    if( old_subs != NULL ) {

      new_subs = (Set_Info *)calloc((size_t) (old_subs->num_sets) , sizeof(Set_Info) );
      
      for( j = 0 ; j < old_subs->num_sets ; j++ ) {


	old_set = &((old_subs+j)->the_set);


	
	new_set( new_db_info, old_subs + j, new_subs + j );
      } 
    }

  }
}


void new_allsets( DB_Info *new_db_info, Set_Info *head_set_info, int set_num )
{

  
  new_set( new_db_info, head_set_info + set_num, (new_db_info->allsets) + set_num );


}


void new_set( DB_Info *new_db_info, Set_Info *set_info, Set_Info *new_set_info)
{

  int i, existing_set = 0, found_l=0 ;

	char *set_name=NULL;
	char *cat_name=NULL;
	SAF_Db *the_db;
	int tdim;
	int num_colls=0;
	SAF_TopMode t_mode;
	SAF_ExtendMode e_mode;
	SAF_SilRole srole;
	SAF_Cat *new_cat;
	SAF_Cat *found_cats=NULL;
	SAF_Set *a_new_set;
	SAF_Set *the_set;

	SAF_Role cat_role;
	int cat_tdim;
	int count;

	SAF_IndexSpec ispec;
	SAF_DecompMode isDecomp;
	SAF_CellType cellType;


	if( new_set_info == NULL || set_info == NULL ) {
	  printf("new_set: passed NULL set_info\n");
	  return;
	}


	the_db = new_db_info->the_db;

	/*
	if( set_info->duplicate_set != NULL ) {
	  printf("new_set: duplicate_set is already set, returning\n");
	  return;
	}
	*/

	the_set = &(set_info->the_set);

	set_name = NULL;
	saf_describe_set(SAF_ALL, the_set, &set_name, &tdim, &srole, &e_mode, &t_mode, &num_colls, NULL);

	found_l = 0;

	for( i = 0 ; i < allsets->num_sets ; i++ ) {

	  if(ss_pers_equal((ss_pers_t*)&(set_info->the_set),(ss_pers_t*)&(allsets[i].the_set),NULL)) {

	    if( allsets[i].duplicate_set != NULL ) {
	      found_l = 1; /* some non-neg value */
	      new_set_info->the_set =(allsets[i].duplicate_set)->the_set;
	      a_new_set = &(new_set_info->the_set);
	      break;
	    }
	  }
	}

	if( !found_l ) {
	  a_new_set = (SAF_Set *)calloc((size_t)1, sizeof(SAF_Set) );
	  saf_declare_set( SAF_ALL, the_db, set_name, tdim, srole, e_mode, a_new_set );

	}

	set_name = NULL;
	saf_describe_set(SAF_ALL, a_new_set, &set_name, &tdim, &srole, &e_mode, &t_mode, &num_colls, NULL);
	

	set_info->duplicate_set = new_set_info;

	new_set_info->the_set = *a_new_set;
	new_set_info->num_sets = set_info->num_sets;
	new_set_info->set_name = set_name;
	new_set_info->silrole = srole;
	new_set_info->topo_relation = NULL;
	new_set_info->fields = NULL;
	new_set_info->duplicate_set = NULL;
	new_set_info->num_fields = 0;


	new_set_info->collections = (Collection_Info *)calloc((size_t) (set_info->num_colls) , sizeof(Collection_Info) );
	new_set_info->num_colls = set_info->num_colls;

	for( i = 0 ; i < set_info->num_colls; i++ ) {
	  cat_name = NULL;
	  count = 0;
	  saf_describe_category(SAF_ALL, &(set_info->collections[i].the_cat), &cat_name, &cat_role, &cat_tdim);
	  saf_find_categories(SAF_ALL,the_db,SAF_UNIVERSE(the_db),cat_name,SAF_ANY_ROLE,SAF_ANY_TOPODIM,&count,NULL);
	  if( count < 1 ) {
            new_cat = (SAF_Cat *)calloc((size_t)1, sizeof(SAF_Cat));
	    saf_declare_category(SAF_ALL, the_db, cat_name, &cat_role, cat_tdim, new_cat );
	    found_cats = new_cat;
	  }
	  else {
	    found_cats = NULL;
	    saf_find_categories(SAF_ALL,the_db,SAF_UNIVERSE(the_db),cat_name,SAF_ANY_ROLE,SAF_ANY_TOPODIM,&count, &found_cats);
	  }

	  if( existing_set == 0 ) {
	    count = 0;
	    saf_describe_collection(SAF_ALL, the_set, &(set_info->collections[i].the_cat), &cellType, &count, &ispec, &isDecomp, NULL);
	    saf_declare_collection( SAF_ALL, a_new_set, found_cats+0, cellType, count, ispec, isDecomp);
	  }
 
	  new_set_info->collections[i].the_cat = found_cats[0];
	  new_set_info->collections[i].cat_name = cat_name;
	  new_set_info->collections[i].count = count;
	  new_set_info->collections[i].child_setlink = NULL;
	  new_set_info->collections[i].subset_relation = NULL;
	  new_set_info->collections[i].num_subset_relations = -1;
	  new_set_info->collections[i].topo_link = NULL;
	  new_set_info->collections[i].topo_domain_id = NULL;
	  new_set_info->collections[i].parent_setlink = NULL;
	  new_set_info->collections[i].parent_colllink = -1;

	}

	/* find_collections( new_set_info, 0 ); */
}


void new_field( DB_Info *new_db_info, Field_Info *old_f, Field_Info *new_f )
{

  int i, num_comps = 0, cat_count = 0;
  char *name = NULL, *cat_name = NULL, *unit_name = NULL;
  int *comp_order = NULL;
  SAF_Db *new_db;
  SAF_Cat *new_saf_self;
  SAF_Set base_space;
  Field_Info *new_comps = NULL;
  SAF_Field *the_new_field;
  SAF_Field *components = NULL;
  Field_Info *old_component_infos = NULL;
  FieldTmpl_Info new_template_info;
  hbool_t is_coord;
  SAF_Cat homog_decomp, coeff_assoc, eval_coll;
  SAF_Cat *new_homog_decomp, *new_coeff_assoc, *new_eval_coll;
  SAF_Unit unit;
  int assoc_ratio;
  SAF_Eval eval_func;
  hid_t data_type;
  SAF_Interleave comp_intlv;
  Field_Info *buffer;
  
  /* eai row_id = (int)(((SAF_BaseHandle *)(old_f->the_field))->theRow); */
  buffer = Ftab_find(field_lookup, old_f->the_field,nfield_lookup);

  if( buffer ) {
    /* eai *new_f = *(field_lookup[ row_id ]); */
    *new_f = *(buffer);
    return;
  }


  /* printf("new_field: %s:%s\n", old_f->field_name, old_f->base_space_name); */

  if( old_f->base_space->silrole == SAF_SUITE ) {
    return;
  }

  new_db = new_db_info->the_db;

  num_comps = old_f->num_comps;
  old_component_infos = old_f->component_fields;

  if( (num_comps > 0) && (old_component_infos != NULL) ) {

    /* printf("new_field: in components creation: %d\n", num_comps); */

    new_comps = (Field_Info *)calloc((size_t) num_comps , sizeof(Field_Info));
    
    for( i = 0 ; i < num_comps ; i++ ) {
      new_field( new_db_info, old_component_infos + i, new_comps + i );
    }

    new_f->component_fields = new_comps;
    new_f->num_comps = num_comps;
  }
  else {
    /* printf("new_field: setting components to NULL\n"); */
    new_f->component_fields = NULL;
    new_f->num_comps = 0;
    num_comps = 0;
  }
    
 
  name = NULL;
  num_comps = 0;
  saf_describe_field( SAF_ALL, old_f->the_field, NULL, &name,  &base_space, &unit, &is_coord, \
		      &homog_decomp, &coeff_assoc, &assoc_ratio, &eval_coll, &eval_func, &data_type,
		      &num_comps, NULL, &comp_intlv, NULL);

  new_saf_self = SAF_SELF(new_db);
  
  if( SAF_EQUIV(&homog_decomp, saf_self) )
    new_homog_decomp = new_saf_self;
  else {
    cat_name = NULL;
    saf_describe_category(SAF_ALL, &homog_decomp, &cat_name, NULL, NULL);
    cat_count = 0;
    saf_find_categories(SAF_ALL,new_db,SAF_UNIVERSE(new_db),cat_name,SAF_ANY_ROLE,SAF_ANY_TOPODIM,&cat_count,NULL);
    new_homog_decomp = NULL;
    saf_find_categories(SAF_ALL,new_db,SAF_UNIVERSE(new_db),cat_name,SAF_ANY_ROLE,SAF_ANY_TOPODIM,&cat_count, &new_homog_decomp);
  }

  if( SAF_EQUIV( &coeff_assoc, saf_self) )
    new_coeff_assoc = new_saf_self;
  else {
    cat_name = NULL;
    saf_describe_category(SAF_ALL, &coeff_assoc, &cat_name, NULL, NULL);
    cat_count = 0;
    saf_find_categories(SAF_ALL,new_db,SAF_UNIVERSE(new_db),cat_name,SAF_ANY_ROLE,SAF_ANY_TOPODIM,&cat_count,NULL);
    new_coeff_assoc = NULL;
    saf_find_categories(SAF_ALL,new_db,SAF_UNIVERSE(new_db),cat_name,SAF_ANY_ROLE,SAF_ANY_TOPODIM,&cat_count, &new_coeff_assoc);
  }

  if( SAF_EQUIV(&eval_coll, saf_self) )
    new_eval_coll = new_saf_self;
  else {
    cat_name = NULL;
    saf_describe_category(SAF_ALL, &eval_coll, &cat_name, NULL, NULL);
    cat_count = 0;
    saf_find_categories(SAF_ALL,new_db,SAF_UNIVERSE(new_db),cat_name,SAF_ANY_ROLE,SAF_ANY_TOPODIM,&cat_count,NULL);
    new_eval_coll = NULL;
    saf_find_categories(SAF_ALL,new_db,SAF_UNIVERSE(new_db),cat_name,SAF_ANY_ROLE,SAF_ANY_TOPODIM,&cat_count, &new_eval_coll);
  }

  if( (new_eval_coll == NULL) || (new_coeff_assoc == NULL) || (new_homog_decomp == NULL) )
    printf("NULL category!  Oops\n");

  /* printf("got all the categories assigned\n"); */

  if( num_comps > 0 ) {
    components = NULL;
    comp_order = NULL;
  saf_describe_field(SAF_ALL, old_f->the_field, NULL, NULL,  NULL, NULL, NULL, \
		      NULL, NULL, NULL, NULL, NULL, NULL, \
		      &num_comps, &components, &comp_intlv, &comp_order);

  }

  new_tmpl( new_db_info, old_f->template_info, &new_template_info);

  /* printf("new_field: got the new_tmpl\n"); */

  if( old_f->base_space == NULL ) {
      printf("the base_space is NULL!!\n");
      return;
  }

  the_new_field = (SAF_Field *)calloc((size_t)1, sizeof(SAF_Field ));

  if( (num_comps > 0) && (components != NULL) ) {

    for( i = 0 ; i < num_comps ; i++ ) {
      components[i] = *(new_f->component_fields[i].the_field);
    }

    unit_name = NULL;
    saf_describe_unit(SAF_ALL, &unit, &unit_name, NULL, NULL, NULL, NULL,NULL, NULL, NULL );
    if( strlen(unit_name) == 0 ) {
      saf_declare_field( SAF_ALL, new_db, new_template_info.fieldtmpl, name, &(old_f->base_space->duplicate_set->the_set), \
			 NULL, new_homog_decomp, new_coeff_assoc, assoc_ratio, new_eval_coll, &eval_func, \
			 data_type, components, comp_intlv, comp_order,  NULL /* bufs */,  the_new_field);
    }
    else {
      /* printf("unit_name is %s:%d\n",unit_name, strlen(unit_name)); */
      saf_declare_field( SAF_ALL, new_db, new_template_info.fieldtmpl, name, &(old_f->base_space->duplicate_set->the_set), \
			 &unit, new_homog_decomp, new_coeff_assoc, assoc_ratio, new_eval_coll, &eval_func, \
			 data_type, components, comp_intlv, comp_order,  NULL /* bufs */,  the_new_field);

    }

  }

  else {


    unit_name = NULL;
    saf_describe_unit(SAF_ALL, &unit, &unit_name, NULL, NULL, NULL, NULL,NULL, NULL, NULL );
    if( strlen(unit_name) == 0 ) {
      /* printf("unit_name is empty!\n"); */ 
      saf_declare_field( SAF_ALL, new_db, new_template_info.fieldtmpl, old_f->field_name, &(old_f->base_space->duplicate_set->the_set), 
			 NULL, new_homog_decomp, new_coeff_assoc, assoc_ratio, new_eval_coll, &eval_func, \
			 data_type, NULL, comp_intlv, NULL,  NULL,  the_new_field);

    }
    else {
      /* printf("unit_name is %s:%d\n",unit_name, strlen(unit_name)); */
      saf_declare_field( SAF_ALL, new_db, new_template_info.fieldtmpl, old_f->field_name, &(old_f->base_space->duplicate_set->the_set), 
			 &unit, new_homog_decomp, new_coeff_assoc, assoc_ratio, new_eval_coll, &eval_func, 
			 data_type, NULL, comp_intlv, NULL,  NULL,  the_new_field);

    }

  }
  
  new_f->the_field = the_new_field;
    
  /*
  get_field_info( new_f );
  */

  field_lookup = (FieldTableOfValues_t *)realloc(field_lookup, (size_t) ((nfield_lookup+1)*sizeof(FieldTableOfValues_t)));
  field_lookup[nfield_lookup].key = *(old_f->the_field);
  field_lookup[nfield_lookup].value = new_f;
  nfield_lookup++;
  
}

void new_tmpl( DB_Info *new_db_info, FieldTmpl_Info *old_t, FieldTmpl_Info *new_t    )
{

  int i, num_comps = 0;
  char *name = NULL;
  SAF_Db *new_db;
  FieldTmpl_Info *new_comps = NULL;
  SAF_Algebraic alg_type;
  SAF_Basis basis;
  SAF_Quantity quantity;
  SAF_FieldTmpl *new_template;
  SAF_FieldTmpl *components = NULL;
  FieldTmpl_Info *old_component_infos = NULL;
  
  FieldTmpl_Info *buffer;
  
  /* eai row_id = (int)(((SAF_BaseHandle *)(old_t->fieldtmpl))->theRow);  */
  buffer = Ftmpltab_find(fieldtmpl_lookup, old_t->fieldtmpl,nfieldtmpl_lookup);

  if( buffer ) {
    /* eai *new_t = *(fieldtmpl_lookup[ row_id ]); */
    *new_t = *(buffer);
    return;
  }
  new_db = new_db_info->the_db;

  num_comps = old_t->num_comps;
  old_component_infos = old_t->component_tmpls;

  if( (num_comps > 0) && (old_component_infos != NULL) ) {

    new_comps = (FieldTmpl_Info *)calloc((size_t) num_comps , sizeof(FieldTmpl_Info));
    
    for( i = 0 ; i < num_comps ; i++ ) {
      new_tmpl( new_db_info, (old_t->component_tmpls) + i, new_comps + i );
    }

    new_t->component_tmpls = new_comps;
    new_t->num_comps = num_comps;
  }
  else {
    new_t->component_tmpls = NULL;
    new_t->num_comps = 0;
  }
    
 
  name = NULL;
  num_comps = 0;
  saf_describe_field_tmpl( SAF_ALL, old_t->fieldtmpl, &name,  &alg_type, &basis, \
			   &quantity, &num_comps, NULL );

  if( num_comps > 0 ) {
    components = NULL;
    saf_describe_field_tmpl( SAF_ALL, old_t->fieldtmpl, NULL, NULL, NULL, NULL, \
			     &num_comps, &components );  
  }

  new_template = (SAF_FieldTmpl *)calloc((size_t)1, sizeof(SAF_FieldTmpl ));

  if( (num_comps > 0) && (components != NULL) ) {

    for( i = 0 ; i < num_comps ; i++ ) {
      components[i] = *(new_t->component_tmpls[i].fieldtmpl);
    }

    saf_declare_field_tmpl( SAF_ALL, new_db, name, &alg_type, &basis, &quantity, num_comps,  components, new_template);
    
  }
  else  {

    saf_declare_field_tmpl( SAF_ALL, new_db, name, &alg_type, &basis, &quantity, num_comps, NULL, new_template );

  }
  
  new_t->fieldtmpl = new_template;
    
  get_fieldtmpl_info( new_t );

  /* eai fieldtmpl_lookup[ row_id _] = new_t; */

  fieldtmpl_lookup = (FieldtmplTableOfValues_t *)realloc(fieldtmpl_lookup, (size_t) ((nfieldtmpl_lookup+1)*sizeof(FieldtmplTableOfValues_t)));
  fieldtmpl_lookup[nfieldtmpl_lookup].key = *(old_t->fieldtmpl);
  fieldtmpl_lookup[nfieldtmpl_lookup].value = new_t;
  nfieldtmpl_lookup++;


}


