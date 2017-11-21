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

extern dbobject *_saf_open( char * , char * );
extern PyObject *get_topsets( PyObject * );
extern void sil( PyObject * );
extern void describe_set( PyObject * );
extern void my_saf_init( void );
extern PyObject *get_field_data( PyObject * );
extern PyObject *get_subset_relation_data( PyObject *, PyObject *);
/* int ss_saf_top_sets( void ); */


PyObject *ss_saf_open( PyObject *self, PyObject *args, PyObject *kwargs) {

	dbobject *db;
	char *filepath = "None";
	char *filename = "None";
	static char *argnames[] = {"filepath","filename",NULL};
	PyObject *private;

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

	return Py_BuildValue("{s:s,s:s,s:i,s,O,s:i,s:i,s:O}",\
		"type","SAF_Db",\
                "name",filename,\
		"_db_handle",db,\
		"_topsets",get_topsets(private),\
		"num_top_sets",db->num_top_sets,\
		"_topsets_handle",db->tops,\
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


PyObject * ss_saf_init( PyObject *self, PyObject *args) {

	my_saf_init();
	return Py_BuildValue("");


}

PyObject * ss_get_field_data( PyObject *self, PyObject *args ) {

  PyObject *data_array;
  PyObject *py_field;

  if( !PyArg_ParseTuple(args,"O",&py_field) ) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  data_array = get_field_data( py_field );
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
	{"_saf_open", ss_saf_open, METH_VARARGS | METH_KEYWORDS },
	{"sil", ss_sil, METH_VARARGS },
	{"saf_init", ss_saf_init, METH_VARARGS },
	{"describe_set", ss_describe_set, METH_VARARGS },
	{"top_sets",ss_topsets,METH_VARARGS },
	{"get_field_data",ss_get_field_data,METH_VARARGS },
	{"subset_relation_data",ss_subset_relation_data,METH_VARARGS },
	{NULL,NULL}
};

void initss() {
	Py_InitModule("ss",ssmethods);
}

