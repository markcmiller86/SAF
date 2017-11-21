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
/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Unstructured Mesh File (create_unstr_mesh)
 * Description:
 *
 *
 * ok  IndexSpec origins (i.e. c vs fortran, vs arbitrary)
 * ok  SAF_FLOAT,SAF_DOUBLE,SAF_INT,SAF_LONG
 * ok  SAF_DECOMP_TRUE,SAF_DECOMP_FALSE
 * ok  SAF_EXTENDIBLE_TRUE,SAF_EXTENDIBLE_FALSE
 * ok  Field name variations for same variable, blank field names(?)
 * ok  coords w SAF_INTERLEAVE_VECTOR, SAF_INTERLEAVE_COMPONENT
 * ok  coords written on composite or component
 * vector variables
 * tensor variables
 * topo rel variations ?
 * vars that dont use the same ftempl?
 * 2d vector variables
 *
 *
 *
 *--------------------------------------------------------------------------------------------------- */


#include <saf.h>
#include <math.h>

#define MY_PRECISION float /*temporary*/
#define MY_SAF_PRECISION SAF_FLOAT /*temporary*/

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Unstructured Mesh File (create_unstr_mesh)
 * Purpose:     Global Variables
 *
 * Description: 
 *
 * These are global variables.
 *
 *--------------------------------------------------------------------------------------------------- */

static int g_rank=0;
static int g_nprocs=1;

static SAF_Db *g_db;                /* Handle to the SAF database. */
static SAF_Db *g_safFile;        /* Handle to the saf file. */

static SAF_Cat g_nodes,g_elems;
static SAF_Unit g_umeter,g_ukelvin,g_uampere,g_ukilogram;

SAF_FieldTmpl g_coordsComponentFtmpl,g_coordsFtmpl;
SAF_FieldTmpl g_globalNodalVarFtmpl,g_globalElemVarFtmpl;
SAF_FieldTmpl g_globalNodalComponentVectorVarFtmpl,g_globalNodalVectorVarFtmpl;
SAF_FieldTmpl g_globalElemVectorVarFtmpl;

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Unstructured Mesh File (create_unstr_mesh)
 * Purpose:     Prototypes
 *
 * Description: 
 *
 * These are prototypes.
 *
 *--------------------------------------------------------------------------------------------------- */
void create_perimeter_block( double a_xsize, double a_ysize );
const char *get_celltype_desc( SAF_CellType a_type );
void create_single_element_block( SAF_CellType a_type, double a_x, double a_y );
void create_single_quadratic_element_block( SAF_CellType a_type, double a_x, double a_y );
void create_row_of_single_element_indirect_blocks( double a_x, double a_y );
void create_mixed_element_block( double a_x, double a_y );
void create_elem_subset_vars_block( double a_x, double a_y );
void create_node_subset_vars_block( double a_x, double a_y );

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Temporary Replacement for snprintf on WIN32
 * Description:
 *
 * For some reason, the WIN32 multi-threaded library doesnt seem to contain snprintf. For now, using
 * this dumb snprintf. NOTE THIS TEMPORARY SNPRINTF DOESNT CHECK FOR MEMORY OVERFLOWS! THERE
 * COULD EASILY BE A MEMORY ERROR IF YOU ARE NOT CAREFUL!
 *--------------------------------------------------------------------------------------------------- */
#if defined WIN32 || defined JANUS
int snprintf(char *buf, size_t count, const char *fmt, ... )
{
  va_list args;
  va_start(args,fmt);
  vsprintf(buf, fmt, args);
                                                                                                                                                       
  if(strlen(buf)>=count)
  {
          printf("WIN32 snprintf error here!! %d %d\n",strlen(buf),count);
          exit(-1);
  }
  return(0);
}
#endif


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Unstructured Mesh File (create_unstr_mesh)
 * Purpose:     
 *
 * Description: 
 *
 *
 *--------------------------------------------------------------------------------------------------- */
void create_perimeter_block( double a_xsize, double a_ysize )
{
  SAF_Set l_set;
  SAF_Rel l_rel;
  SAF_Field l_field, l_componentField[3];


  saf_declare_set(SAF_ALL, g_db, "PERIMETER_SET", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &l_set);
  saf_declare_collection(SAF_ALL, &l_set, &g_nodes, SAF_CELLTYPE_POINT, 24, SAF_1DC(24), SAF_DECOMP_FALSE); 
  saf_declare_collection(SAF_ALL, &l_set, &g_elems, SAF_CELLTYPE_HEX, 4, SAF_1DC(4), SAF_DECOMP_TRUE);

  saf_declare_topo_relation(SAF_ALL, g_db, &l_set, &g_elems, &l_set, &g_nodes, SAF_SELF(g_db), &l_set,
			    SAF_UNSTRUCTURED, 0, NULL, 0, NULL, &l_rel);

  { 
    int abuf[1] = {8};
    int bbuf[4*8] = {0,1,2,3,4,5,6,7,  8,9,10,11,12,13,14,15,  4,7,16,17,8,11,18,19, 5,20,21,6,9,22,23,10 };

    saf_write_topo_relation(SAF_ALL, &l_rel, SAF_INT, abuf, SAF_INT, bbuf, g_safFile);
  }


  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "X", &l_set, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Y", &l_set, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Z", &l_set, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));

  saf_declare_field(SAF_ALL, g_db, &g_coordsFtmpl, "coords", &l_set, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_DOUBLE, l_componentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &l_field); 

  {
    double buf[3*24]= {
      0.0, -.1, .1,    1.0, -.1, .1,   1.0, -.1, 0,    0.0, -.1, 0,
      0.0, 0.0, .1,    1.0, 0.0, .1,   1.0, 0.0, 0,    0.0, 0.0, 0,
      0.0, 1.0, .1,    1.0, 1.0, .1,   1.0, 1.0, 0,    0.0, 1.0, 0,
      0.0, 1.1, .1,    1.0, 1.1, .1,   1.0, 1.1, 0,    0.0, 1.1, 0,
      -.1, 0.0, 0.0,   -.1, 0.0, .1,  -.1, 1.0, 0.0,   -.1, 1.0, .1,
      1.1, 0.0, .1,   1.1, 0.0, 0,  1.1, 1.0, .1,   1.1, 1.0, 0
    };
    void *pbuf = &buf[0];
    int i;
    for(i=0;i<3*24;i+=3) if(buf[i]>.5) buf[i] += a_xsize;
    for(i=1;i<3*24;i+=3) if(buf[i]>.5) buf[i] += a_ysize;
    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }

  saf_declare_coords(SAF_ALL, &l_field);
  saf_declare_coords(SAF_ALL, &(l_componentField[0]) );
  saf_declare_coords(SAF_ALL, &(l_componentField[1]) );
  saf_declare_coords(SAF_ALL, &(l_componentField[2]) );
   
  saf_declare_default_coords(SAF_ALL,&l_set,&l_field);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     
 * Purpose:     Get the Name of a SAF_CellType
 *
 * Description: 
 * 
 * Returns the name of a SAF_CellType, e.g. "SAF_CELLTYPE_HEX".
 *
 *--------------------------------------------------------------------------------------------------- */
const char *get_celltype_desc( SAF_CellType a_type )
{
  if( a_type == SAF_CELLTYPE_POINT ) return("SAF_CELLTYPE_POINT");
  if( a_type == SAF_CELLTYPE_HEX ) return("SAF_CELLTYPE_HEX");
  if( a_type == SAF_CELLTYPE_LINE ) return("SAF_CELLTYPE_LINE");
  if( a_type == SAF_CELLTYPE_TRI ) return("SAF_CELLTYPE_TRI");
  if( a_type == SAF_CELLTYPE_QUAD ) return("SAF_CELLTYPE_QUAD");
  if( a_type == SAF_CELLTYPE_TET ) return("SAF_CELLTYPE_TET");
  if( a_type == SAF_CELLTYPE_PYRAMID ) return("SAF_CELLTYPE_PYRAMID");
  if( a_type == SAF_CELLTYPE_PRISM ) return("SAF_CELLTYPE_PRISM");
  /*if( a_type == SAF_CELLTYPE_0BALL ) return("SAF_CELLTYPE_0BALL");*/
  if( a_type == SAF_CELLTYPE_1BALL ) return("SAF_CELLTYPE_1BALL");
  if( a_type == SAF_CELLTYPE_2BALL ) return("SAF_CELLTYPE_2BALL");
  if( a_type == SAF_CELLTYPE_3BALL ) return("SAF_CELLTYPE_3BALL");
  if( a_type == SAF_CELLTYPE_MIXED ) return("SAF_CELLTYPE_MIXED");
  if( a_type == SAF_CELLTYPE_ARB ) return("SAF_CELLTYPE_ARB");
  if( a_type == SAF_CELLTYPE_SET ) return("SAF_CELLTYPE_SET");
  if( a_type == SAF_CELLTYPE_ANY ) return("SAF_CELLTYPE_ANY");
  return("SAF_CELLTYPE_unknown");
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Unstructured Mesh File (create_unstr_mesh)
 * Purpose:     
 *
 * Description: 
 *
 *
 *--------------------------------------------------------------------------------------------------- */
void create_single_element_block( SAF_CellType a_type, double a_x, double a_y )
{
  SAF_Set l_set;
  SAF_Rel l_rel;
  SAF_Field l_field, l_componentField[3];
  char l_setName[256];
  int l_numElems=1;
  int l_numPoints=0;

  sprintf(l_setName,"%s_SET",get_celltype_desc(a_type));

  if( a_type == SAF_CELLTYPE_LINE ) l_numPoints=2;
  else if( a_type == SAF_CELLTYPE_TRI ) l_numPoints=3;
  else if( a_type == SAF_CELLTYPE_QUAD ) l_numPoints=4;
  else if( a_type == SAF_CELLTYPE_TET ) l_numPoints=4;
  else if( a_type == SAF_CELLTYPE_PYRAMID ) l_numPoints=5;
  else if( a_type == SAF_CELLTYPE_HEX ) l_numPoints=8;
  else if( a_type == SAF_CELLTYPE_PRISM ) l_numPoints=6;
  else
    {
      printf("\nerror: create_single_element_block, cant yet handle %s\n\n", 
	     get_celltype_desc(a_type) );
      exit(-1);
    }

  saf_declare_set(SAF_ALL, g_db, l_setName+strlen("SAF_CELLTYPE_"), /*get rid of the leading SAF_CELLTYPE_*/
		  2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &l_set);
  saf_declare_collection(SAF_ALL, &l_set, &g_nodes, SAF_CELLTYPE_POINT, l_numPoints, SAF_1DC(l_numPoints), SAF_DECOMP_FALSE); 

  saf_declare_collection(SAF_ALL, &l_set, &g_elems, a_type, l_numElems, SAF_1DC(l_numElems), SAF_DECOMP_TRUE);

  saf_declare_topo_relation(SAF_ALL, g_db, &l_set, &g_elems, &l_set, &g_nodes, SAF_SELF(g_db), &l_set,
			    SAF_UNSTRUCTURED, 0, NULL, 0, NULL, &l_rel);

  { 
    int bbuf[24] = {0,1,2,3,4,5,6,7,  8,9,10,11,12,13,14,15, 16,17,18,19,20,21,22,23 };/*larger than the largest element?*/

    saf_write_topo_relation(SAF_ALL, &l_rel, SAF_INT, &l_numPoints, SAF_INT, bbuf, g_safFile);
  }

  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "X", &l_set, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Y", &l_set, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Z", &l_set, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));

  saf_declare_field(SAF_ALL, g_db, &g_coordsFtmpl, "coords", &l_set, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_FLOAT, l_componentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &l_field); 

  {
    float buf[3*8]= {
      -.25, -.25, -.25,   .25, -.25, -.25,  .25, .25, -.25,   -.25, .25, -.25,
      -.25, -.25, .25,    .25, -.25, .25,   .25, .25, .25,    -.25, .25, .25
    };
    void *pbuf = &buf[0];
    int i;
    for(i=0;i<3*8;i+=3) buf[i] += a_x;
    for(i=1;i<3*8;i+=3) buf[i] += a_y;

    if( a_type == SAF_CELLTYPE_TET ) 
      {
	/*Copy the 4th node coords to the 3rd node, otherwise tet will look like a quad, 
	  because all points will be in a plane*/
	buf[3*3+0] = buf[4*3+0];
	buf[3*3+1] = buf[4*3+1];
	buf[3*3+2] = buf[4*3+2];
      }
    else if( a_type == SAF_CELLTYPE_PRISM ) 
      {	/* SAF_CELLTYPE_PRISM or vtkWedge are two triangles (0,1,2) and (3,4,5) connected by 3 quads.*/
	for(i=3;i<6;i++)
	  {
	    buf[i*3+0] = buf[(i+1)*3+0];
	    buf[i*3+1] = buf[(i+1)*3+1];
	    buf[i*3+2] = buf[(i+1)*3+2];
	  }
      }

    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }

  saf_declare_coords(SAF_ALL, &l_field);
  saf_declare_coords(SAF_ALL, &(l_componentField[0]) );
  saf_declare_coords(SAF_ALL, &(l_componentField[1]) );
  saf_declare_coords(SAF_ALL, &(l_componentField[2]) );
   
  saf_declare_default_coords(SAF_ALL,&l_set,&l_field);



 
  /*Write the global scalar nodal variable on the top set. This variable
    will be mapped to every part. It must use g_ukelvin units, to match all the others*/
  {
    SAF_Field l_field;

    double buf[8]= { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0 };
    void *pbuf = &buf[0];



    if( a_type == SAF_CELLTYPE_TET ) 
      {
	/*Copy the 4th node coords to the 3rd node, otherwise tet will look like a quad, 
	  because all points will be in a plane*/
	buf[3] = buf[4];
      }
    else if( a_type == SAF_CELLTYPE_PRISM ) 
      {	/* SAF_CELLTYPE_PRISM or vtkWedge are two triangles (0,1,2) and (3,4,5) connected by 3 quads.*/
	int i;
	for(i=3;i<6;i++) buf[i] = buf[i+1];
      }

    saf_declare_field(SAF_ALL, g_db, &g_globalNodalVarFtmpl, "global_nodal_var", &l_set, &g_ukelvin, SAF_SELF(g_db), 
		      SAF_NODAL(&g_nodes, &g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);

    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }


#if 1

 
  /*Write the global scalar elem variable on the top set. This variable
    will be mapped to every part. It must use g_uampere units, to match all the others*/
  {
    SAF_Field l_field;

    double buf[1];
    void *pbuf = &buf[0];
    buf[0]=a_x+1.0;

    saf_declare_field(SAF_ALL, g_db, &g_globalElemVarFtmpl, "global_elem_var", &l_set, &g_uampere, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }





  /*Write the global scalar elem vector variable on the top set. This variable
    will be mapped to every part. It must use g_umeter units, to match all the others*/
  {
    SAF_Field l_field;
    SAF_FieldTmpl l_temp[3];
    SAF_Field l_componentField[3];

    double buf[3];
    void *pbuf = &buf[0];
    buf[0]=  a_x;
    buf[1]=  8.0-a_x;
    buf[2]=  a_x;

    l_temp[0] = g_coordsComponentFtmpl;
    l_temp[1] = g_coordsComponentFtmpl;
    l_temp[2] = g_coordsComponentFtmpl;

    saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Xglobal_elem_vector_var", &l_set, &g_umeter, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
    saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Yglobal_elem_vector_var", &l_set, &g_umeter, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
    saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Zglobal_elem_vector_var", &l_set, &g_umeter, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));

    saf_declare_field(SAF_ALL, g_db, &g_globalElemVectorVarFtmpl, "global_elem_vector_var", &l_set, &g_umeter, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, l_componentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &l_field);

    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }





  /*Write the global scalar nodal vector variable on the top set. This variable
    will be mapped to every part. It must use g_ukilogram units, to match all the others*/
  {
    SAF_Field l_field;
    SAF_FieldTmpl l_temp[3];
    SAF_Field l_componentField[3];

    double buf[3*8]= { 0,7,0,  1,6,1,  2,5,2,  3,4,3,  4,3,4,  5,2,5,  6,1,6,  7,0,7 };
    void *pbuf = &buf[0];

    if( a_type == SAF_CELLTYPE_TET ) 
      {
	/*Copy the 4th node coords to the 3rd node, otherwise tet will look like a quad, 
	  because all points will be in a plane*/
	buf[3*3+0] = buf[4*3+0];
	buf[3*3+1] = buf[4*3+1];
	buf[3*3+2] = buf[4*3+2];
      }
    else if( a_type == SAF_CELLTYPE_PRISM ) 
      {	/* SAF_CELLTYPE_PRISM or vtkWedge are two triangles (0,1,2) and (3,4,5) connected by 3 quads.*/
	int i,j;
	for(i=3;i<6;i++) 
	  for(j=0;j<3;j++) buf[3*i+j] = buf[3*(i+1)+j];
      }

    l_temp[0] = g_globalNodalComponentVectorVarFtmpl;
    l_temp[1] = g_globalNodalComponentVectorVarFtmpl;
    l_temp[2] = g_globalNodalComponentVectorVarFtmpl;

    saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Xglobal_nodal_vector_var", &l_set, &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
    saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Yglobal_nodal_vector_var", &l_set, &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
    saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Zglobal_nodal_vector_var", &l_set, &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));

    saf_declare_field(SAF_ALL, g_db, &g_globalNodalVectorVarFtmpl, "global_nodal_vector_var", &l_set, &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_DOUBLE, l_componentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &l_field);

    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }

#endif 
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Unstructured Mesh File (create_unstr_mesh)
 * Purpose:     
 *
 * Description: 
 *
 *
 *--------------------------------------------------------------------------------------------------- */
void create_single_quadratic_element_block( SAF_CellType a_type, double a_x, double a_y )
{
  int i;
  SAF_Set l_set;
  SAF_Rel l_rel;
  SAF_Field l_field, l_componentField[3];
  char l_setName[256];
  int l_numElems=1;
  int l_numPoints=0;
  float l_nodalVar[20];


  sprintf(l_setName,"QUADRATIC_%s_SET",get_celltype_desc(a_type)+strlen("SAF_CELLTYPE_"));/*get rid of the leading SAF_CELLTYPE_*/

  if( a_type == SAF_CELLTYPE_LINE ) l_numPoints=3;
  else if( a_type == SAF_CELLTYPE_TRI ) l_numPoints=6;
  else if( a_type == SAF_CELLTYPE_QUAD ) l_numPoints=8;
  else if( a_type == SAF_CELLTYPE_TET ) l_numPoints=10;
  else if( a_type == SAF_CELLTYPE_PYRAMID ) l_numPoints=13;
  else if( a_type == SAF_CELLTYPE_HEX ) l_numPoints=20;
  else if( a_type == SAF_CELLTYPE_PRISM ) l_numPoints=15;
  else
    {
      printf("\nerror: create_single_quadratic_element_block, cant handle %s\n\n", 
	     get_celltype_desc(a_type) );
      exit(-1);
    }
  /*need to add quadratic to the name*/


  saf_declare_set(SAF_ALL, g_db, l_setName,
		  2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &l_set);
  saf_declare_collection(SAF_ALL, &l_set, &g_nodes, SAF_CELLTYPE_POINT, l_numPoints, SAF_1DC(l_numPoints), SAF_DECOMP_FALSE); 

  saf_declare_collection(SAF_ALL, &l_set, &g_elems, a_type, l_numElems, SAF_1DC(l_numElems), SAF_DECOMP_TRUE);

  saf_declare_topo_relation(SAF_ALL, g_db, &l_set, &g_elems, &l_set, &g_nodes, SAF_SELF(g_db), &l_set,
			    SAF_UNSTRUCTURED, 0, NULL, 0, NULL, &l_rel);

  { 
    int bbuf[20] = {0,1,2,3,4,5,6,7,  8,9,10,11, 12,13,14,15, 16,17,18,19 };
    saf_write_topo_relation(SAF_ALL, &l_rel, SAF_INT, &l_numPoints, SAF_INT, bbuf, g_safFile);
  }


  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "X", &l_set, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Y", &l_set, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Z", &l_set, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));

  saf_declare_field(SAF_ALL, g_db, &g_coordsFtmpl, "coords", &l_set, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_FLOAT, l_componentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &l_field); 


  {
    float buf[3*20];
    void *pbuf = &buf[0];

    for(i=0;i<3*20;i++) buf[i]=0;
    for(i=0;i<20;i++) l_nodalVar[i]=0;

    if( a_type == SAF_CELLTYPE_LINE ) 
      {
	buf[0*3+0]= -.25; buf[0*3+1]= -.25; buf[0*3+2]= -.25; 
	buf[1*3+0]=  .25; buf[1*3+1]= -.25; buf[1*3+2]= -.25; 
	buf[2*3+0]=    0; buf[2*3+1]= -.24; buf[2*3+2]= -.24; 
	l_nodalVar[0]=0;
	l_nodalVar[1]=1;
	l_nodalVar[2]=(l_nodalVar[0]+l_nodalVar[1])/2;
      }
    else if( a_type == SAF_CELLTYPE_TRI ) 
      {
	buf[0*3+0]= -.25; buf[0*3+1]= -.25; buf[0*3+2]= -.25; 
	buf[1*3+0]=  .25; buf[1*3+1]= -.25; buf[1*3+2]= -.25; 
	buf[2*3+0]=  .25; buf[2*3+1]=  .25; buf[2*3+2]= -.25; 

	buf[3*3+0]=    0; buf[3*3+1]= -.24; buf[3*3+2]= -.24; 
	buf[4*3+0]=  .24; buf[4*3+1]=    0; buf[4*3+2]= -.24; 
	buf[5*3+0]=    0; buf[5*3+1]=    0; buf[5*3+2]= -.24; 

	l_nodalVar[0]=0;
	l_nodalVar[1]=1;
	l_nodalVar[2]=2;
	l_nodalVar[3]=(l_nodalVar[0]+l_nodalVar[1])/2;
	l_nodalVar[4]=(l_nodalVar[2]+l_nodalVar[1])/2;
	l_nodalVar[5]=(l_nodalVar[2]+l_nodalVar[0])/2;
      }
    else if( a_type == SAF_CELLTYPE_QUAD ) 
      {
	buf[0*3+0]= -.25; buf[0*3+1]= -.25; buf[0*3+2]= -.25; 
	buf[1*3+0]=  .25; buf[1*3+1]= -.25; buf[1*3+2]= -.25; 
	buf[2*3+0]=  .25; buf[2*3+1]=  .25; buf[2*3+2]= -.25; 
	buf[3*3+0]= -.25; buf[3*3+1]=  .25; buf[3*3+2]= -.25; 

	buf[4*3+0]=    0; buf[4*3+1]= -.24; buf[4*3+2]= -.24; 
	buf[5*3+0]=  .24; buf[5*3+1]=    0; buf[5*3+2]= -.24; 
	buf[6*3+0]=    0; buf[6*3+1]=  .24; buf[6*3+2]= -.24; 
	buf[7*3+0]= -.24; buf[7*3+1]=    0; buf[7*3+2]= -.24; 

	l_nodalVar[0]=0;
	l_nodalVar[1]=1;
	l_nodalVar[2]=2;
	l_nodalVar[3]=3;
	l_nodalVar[4]=(l_nodalVar[0]+l_nodalVar[1])/2;
	l_nodalVar[5]=(l_nodalVar[1]+l_nodalVar[2])/2;
	l_nodalVar[6]=(l_nodalVar[2]+l_nodalVar[3])/2;
	l_nodalVar[7]=(l_nodalVar[3]+l_nodalVar[0])/2;
      }
    else if( a_type == SAF_CELLTYPE_TET ) 
      {
	buf[0*3+0]= -.25; buf[0*3+1]= -.25; buf[0*3+2]= -.25; 
	buf[1*3+0]=  .25; buf[1*3+1]= -.25; buf[1*3+2]= -.25; 
	buf[2*3+0]=  .25; buf[2*3+1]=  .25; buf[2*3+2]= -.25; 
	buf[3*3+0]= -.25; buf[3*3+1]= -.25; buf[3*3+2]=  .25; 

	buf[4*3+0]=  .01; buf[4*3+1]= -.24; buf[4*3+2]= -.24; 
	buf[5*3+0]=  .24; buf[5*3+1]=  .01; buf[5*3+2]= -.24; 
	buf[6*3+0]=  .01; buf[6*3+1]=  .01; buf[6*3+2]= -.24; 

	buf[7*3+0]= -.24; buf[7*3+1]= -.24; buf[7*3+2]=.01; 
	buf[8*3+0]=  .01; buf[8*3+1]= -.24; buf[8*3+2]=.01; 
	buf[9*3+0]=  .01; buf[9*3+1]=  .01; buf[9*3+2]=.01; 

	l_nodalVar[0]=0;
	l_nodalVar[1]=1;
	l_nodalVar[2]=2;
	l_nodalVar[3]=4;

	l_nodalVar[4]=(l_nodalVar[0]+l_nodalVar[1])/2;
	l_nodalVar[5]=(l_nodalVar[1]+l_nodalVar[2])/2;
	l_nodalVar[6]=(l_nodalVar[2]+l_nodalVar[0])/2;

	l_nodalVar[7]=(l_nodalVar[0]+l_nodalVar[3])/2;
	l_nodalVar[8]=(l_nodalVar[1]+l_nodalVar[3])/2;
	l_nodalVar[9]=(l_nodalVar[2]+l_nodalVar[3])/2;

      }
    else if( a_type == SAF_CELLTYPE_PYRAMID ) 
      {
	buf[0*3+0]= -.25; buf[0*3+1]= -.25; buf[0*3+2]= -.25; 
	buf[1*3+0]=  .25; buf[1*3+1]= -.25; buf[1*3+2]= -.25; 
	buf[2*3+0]=  .25; buf[2*3+1]=  .25; buf[2*3+2]= -.25; 
	buf[3*3+0]= -.25; buf[3*3+1]=  .25; buf[3*3+2]= -.25; 

	buf[4*3+0]= -.25; buf[4*3+1]= -.25; buf[4*3+2]=  .25; 


	buf[5*3+0]=    0; buf[5*3+1]= -.24; buf[5*3+2]= -.24; 
	buf[6*3+0]=  .24; buf[6*3+1]=    0; buf[6*3+2]= -.24; 
	buf[7*3+0]=    0; buf[7*3+1]=  .24; buf[7*3+2]= -.24; 
	buf[8*3+0]= -.24; buf[8*3+1]=    0; buf[8*3+2]= -.24; 

	buf[9 *3+0]= -.24; buf[9 *3+1]= -.24; buf[9 *3+2]= 0; 
	buf[10*3+0]=    0; buf[10*3+1]= -.24; buf[10*3+2]= 0; 
	buf[11*3+0]=    0; buf[11*3+1]=    0; buf[11*3+2]= 0; 
	buf[12*3+0]= -.24; buf[12*3+1]=    0; buf[12*3+2]= 0; 

	l_nodalVar[0]=0;
	l_nodalVar[1]=1;
	l_nodalVar[2]=2;
	l_nodalVar[3]=3;
	l_nodalVar[4]=4;

	l_nodalVar[5]=(l_nodalVar[0]+l_nodalVar[1])/2;
	l_nodalVar[6]=(l_nodalVar[1]+l_nodalVar[2])/2;
	l_nodalVar[7]=(l_nodalVar[2]+l_nodalVar[3])/2;
	l_nodalVar[8]=(l_nodalVar[3]+l_nodalVar[0])/2;

	l_nodalVar[9]=(l_nodalVar[0]+l_nodalVar[4])/2;
	l_nodalVar[10]=(l_nodalVar[1]+l_nodalVar[4])/2;
	l_nodalVar[11]=(l_nodalVar[2]+l_nodalVar[4])/2;
	l_nodalVar[12]=(l_nodalVar[3]+l_nodalVar[4])/2;


      }
    else if( a_type == SAF_CELLTYPE_HEX ) 
      {
	buf[0*3+0]= -.25; buf[0*3+1]= -.25; buf[0*3+2]= -.25; 
	buf[1*3+0]=  .25; buf[1*3+1]= -.25; buf[1*3+2]= -.25; 
	buf[2*3+0]=  .25; buf[2*3+1]=  .25; buf[2*3+2]= -.25; 
	buf[3*3+0]= -.25; buf[3*3+1]=  .25; buf[3*3+2]= -.25; 

	buf[4*3+0]= -.25; buf[4*3+1]= -.25; buf[4*3+2]= .25; 
	buf[5*3+0]=  .25; buf[5*3+1]= -.25; buf[5*3+2]= .25; 
	buf[6*3+0]=  .25; buf[6*3+1]=  .25; buf[6*3+2]= .25; 
	buf[7*3+0]= -.25; buf[7*3+1]=  .25; buf[7*3+2]= .25; 
	
	buf[8 *3+0]=    0; buf[8 *3+1]= -.24; buf[8 *3+2]= -.25; 
	buf[9 *3+0]=  .24; buf[9 *3+1]=    0; buf[9 *3+2]= -.25; 
	buf[10*3+0]=    0; buf[10*3+1]=  .24; buf[10*3+2]= -.25; 
	buf[11*3+0]= -.24; buf[11*3+1]=    0; buf[11*3+2]= -.25; 

	/*note: this looks right in paraview, but doesnt match the
	  exodus book. To match the book, switch the next two sections*/

	buf[12*3+0]=    0; buf[12*3+1]= -.24; buf[12*3+2]= .25; 
	buf[13*3+0]=  .24; buf[13*3+1]=    0; buf[13*3+2]= .25; 
	buf[14*3+0]=    0; buf[14*3+1]=  .24; buf[14*3+2]= .25; 
	buf[15*3+0]= -.24; buf[15*3+1]=    0; buf[15*3+2]= .25; 

	buf[16*3+0]= -.24; buf[16*3+1]= -.24; buf[16*3+2]= 0; 
	buf[17*3+0]=  .24; buf[17*3+1]= -.24; buf[17*3+2]= 0; 
	buf[18*3+0]=  .24; buf[18*3+1]=  .24; buf[18*3+2]= 0; 
	buf[19*3+0]= -.24; buf[19*3+1]=  .24; buf[19*3+2]= 0; 
       

     
	l_nodalVar[0]=0;
	l_nodalVar[1]=1;
	l_nodalVar[2]=2;
	l_nodalVar[3]=3;

	l_nodalVar[4]=4;
	l_nodalVar[5]=5;
	l_nodalVar[6]=6;
	l_nodalVar[7]=7;

	l_nodalVar[8]=(l_nodalVar[0]+l_nodalVar[1])/2;
	l_nodalVar[9]=(l_nodalVar[1]+l_nodalVar[2])/2;
	l_nodalVar[10]=(l_nodalVar[2]+l_nodalVar[3])/2;
	l_nodalVar[11]=(l_nodalVar[3]+l_nodalVar[0])/2;

	l_nodalVar[12]=(l_nodalVar[4]+l_nodalVar[5])/2;
	l_nodalVar[13]=(l_nodalVar[5]+l_nodalVar[6])/2;
	l_nodalVar[14]=(l_nodalVar[6]+l_nodalVar[7])/2;
	l_nodalVar[15]=(l_nodalVar[7]+l_nodalVar[4])/2;

	l_nodalVar[16]=(l_nodalVar[4]+l_nodalVar[0])/2;
	l_nodalVar[17]=(l_nodalVar[5]+l_nodalVar[1])/2;
	l_nodalVar[18]=(l_nodalVar[6]+l_nodalVar[2])/2;
	l_nodalVar[19]=(l_nodalVar[7]+l_nodalVar[3])/2;

      }
    else if( a_type == SAF_CELLTYPE_PRISM ) 
      {
	buf[0*3+0]= -.25; buf[0*3+1]= -.25; buf[0*3+2]= -.25; 
	buf[1*3+0]=  .25; buf[1*3+1]= -.25; buf[1*3+2]= -.25; 
	buf[2*3+0]=  .25; buf[2*3+1]=  .25; buf[2*3+2]= -.25; 

	buf[3*3+0]= -.25; buf[3*3+1]= -.25; buf[3*3+2]= .25; 
	buf[4*3+0]=  .25; buf[4*3+1]= -.25; buf[4*3+2]= .25; 
	buf[5*3+0]=  .25; buf[5*3+1]=  .25; buf[5*3+2]= .25; 

	buf[6 *3+0]=    0; buf[6 *3+1]= -.24; buf[6 *3+2]= -.25; 
	buf[7 *3+0]=  .24; buf[7 *3+1]=    0; buf[7 *3+2]= -.25; 
	buf[8*3+0]=    .01; buf[8*3+1]=  .01; buf[8*3+2]= -.24; 


	/*note: this looks right in paraview, but doesnt match the
	  exodus book. To match the book, switch the next two sections*/

	buf[9*3+0]=    0; buf[9*3+1]= -.24; buf[9*3+2]= .25; 
	buf[10*3+0]=  .24; buf[10*3+1]=    0; buf[10*3+2]= .25; 
	buf[11*3+0]=    -.01; buf[11*3+1]=  -.01; buf[11*3+2]= .24; 


	buf[12*3+0]= -.24; buf[12*3+1]= -.24; buf[12*3+2]= 0; 
	buf[13*3+0]=  .24; buf[13*3+1]= -.24; buf[13*3+2]= 0; 
	buf[14*3+0]=  .24; buf[14*3+1]=  .24; buf[14*3+2]= 0; 

       
	l_nodalVar[0]=0;
	l_nodalVar[1]=1;
	l_nodalVar[2]=2;

	l_nodalVar[3]=4;
	l_nodalVar[4]=5;
	l_nodalVar[5]=6;

	l_nodalVar[6]=(l_nodalVar[0]+l_nodalVar[1])/2;
	l_nodalVar[7]=(l_nodalVar[1]+l_nodalVar[2])/2;
	l_nodalVar[8]=(l_nodalVar[2]+l_nodalVar[0])/2;

	l_nodalVar[9]=(l_nodalVar[3]+l_nodalVar[4])/2;
	l_nodalVar[10]=(l_nodalVar[4]+l_nodalVar[5])/2;
	l_nodalVar[11]=(l_nodalVar[5]+l_nodalVar[3])/2;

	l_nodalVar[12]=(l_nodalVar[0]+l_nodalVar[3])/2;
	l_nodalVar[13]=(l_nodalVar[1]+l_nodalVar[4])/2;
	l_nodalVar[14]=(l_nodalVar[2]+l_nodalVar[5])/2;


      }



    for(i=0;i<3*20;i+=3) buf[i] += a_x;
    for(i=1;i<3*20;i+=3) buf[i] += a_y;

    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }


  saf_declare_coords(SAF_ALL, &l_field);
  saf_declare_coords(SAF_ALL, &(l_componentField[0]) );
  saf_declare_coords(SAF_ALL, &(l_componentField[1]) );
  saf_declare_coords(SAF_ALL, &(l_componentField[2]) );
   
  saf_declare_default_coords(SAF_ALL,&l_set,&l_field);


 
  /*Write the global scalar nodal variable on the top set. This variable
    will be mapped to every part. It must use g_ukelvin units, to match all the others*/
  {
    SAF_Field l_field;
    void *pbuf = &l_nodalVar[0];

    saf_declare_field(SAF_ALL, g_db, &g_globalNodalVarFtmpl, "global_nodal_var", &l_set, &g_ukelvin, SAF_SELF(g_db), 
		      SAF_NODAL(&g_nodes, &g_elems),
		      SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }



 
  /*Write the global scalar elem variable on the top set. This variable
    will be mapped to every part. It must use g_uampere units, to match all the others*/
  {
    SAF_Field l_field;

    double buf[1];
    void *pbuf = &buf[0];
    buf[0]=a_x+1.0;

    saf_declare_field(SAF_ALL, g_db, &g_globalElemVarFtmpl, "global_elem_var_with_extra_name", &l_set, &g_uampere, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }




  /*Write the global scalar elem vector variable on the top set. This variable
    will be mapped to every part. It must use g_umeter units, to match all the others*/
  {
    SAF_Field l_field;
    SAF_FieldTmpl l_temp[3];
    SAF_Field l_componentField[3];

    double buf[3];
    void *pbuf = &buf[0];
    buf[0]=  a_x;
    buf[1]=  8.0-a_x;
    buf[2]=  a_x;

    l_temp[0] = g_coordsComponentFtmpl;
    l_temp[1] = g_coordsComponentFtmpl;
    l_temp[2] = g_coordsComponentFtmpl;

    saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Xglobal_elem_vector_var", &l_set, &g_umeter, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
    saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Yglobal_elem_vector_var", &l_set, &g_umeter, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
    saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Zglobal_elem_vector_var", &l_set, &g_umeter, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));

    saf_declare_field(SAF_ALL, g_db, &g_globalElemVectorVarFtmpl, "global_elem_vector_var", &l_set, &g_umeter, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, l_componentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &l_field);

#if 1
    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
#else
    /*jake hooey XXX: if I do this, they will come thru as 3 different vars
      .....perhaps should check for a composite field in reader....???? XXXX
    */
    pbuf = &buf[0];
    saf_write_field(SAF_ALL, l_componentField[0], SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
    pbuf = &buf[1];
    saf_write_field(SAF_ALL, l_componentField[1], SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
    pbuf = &buf[2];
    saf_write_field(SAF_ALL, l_componentField[2], SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
#endif
  }












  /*Write the global scalar nodal vector variable on the top set. This variable
    will be mapped to every part. It must use g_ukilogram units, to match all the others*/
  {
    SAF_Field l_field;
    SAF_FieldTmpl l_temp[3];
    SAF_Field l_componentField[3];
    void *pbuf;
    float l_compositeNodalVar[3*20];

    for(i=0;i<l_numPoints;i++) l_compositeNodalVar[i]              =l_nodalVar[i];
    for(i=0;i<l_numPoints;i++) l_compositeNodalVar[i+l_numPoints]  =7-l_nodalVar[i];
    for(i=0;i<l_numPoints;i++) l_compositeNodalVar[i+2*l_numPoints]=l_nodalVar[i];

    pbuf = &l_compositeNodalVar;

    l_temp[0] = g_globalNodalComponentVectorVarFtmpl;
    l_temp[1] = g_globalNodalComponentVectorVarFtmpl;
    l_temp[2] = g_globalNodalComponentVectorVarFtmpl;

    saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Xglobal_nodal_vector_var", &l_set, &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
    saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Yglobal_nodal_vector_var", &l_set, &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
    saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Zglobal_nodal_vector_var", &l_set, &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));

    saf_declare_field(SAF_ALL, g_db, &g_globalNodalVectorVarFtmpl, "global_nodal_vector_var", &l_set, &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_FLOAT, l_componentField, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &l_field);

    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }






}




/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Unstructured Mesh File (create_unstr_mesh)
 * Purpose:     
 *
 * Description: 
 *
 *
 *--------------------------------------------------------------------------------------------------- */
void create_row_of_single_element_indirect_blocks( double a_x, double a_y )
{
  SAF_Set l_nodeSet;
  SAF_Field l_field, l_componentField[3];
  char l_setName[256];
  int l_numElems=1;
  int l_numPoints=0;
  int l_totalNumNodes,i;

  const int l_numTypes=7;
  SAF_CellType l_types[7] = {
    SAF_CELLTYPE_HEX,
    SAF_CELLTYPE_LINE,
    SAF_CELLTYPE_TRI,
    SAF_CELLTYPE_QUAD,
    SAF_CELLTYPE_TET,
    SAF_CELLTYPE_PYRAMID,
    SAF_CELLTYPE_PRISM };

  l_totalNumNodes=l_numTypes*8;

  saf_declare_set(SAF_ALL, g_db, "INDIR_NODE_SET", 0, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &l_nodeSet);
  saf_declare_collection(SAF_ALL,&l_nodeSet,&g_nodes,SAF_CELLTYPE_POINT,l_totalNumNodes,SAF_1DC(l_totalNumNodes),SAF_DECOMP_FALSE); 

  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "X", &l_nodeSet, &g_umeter, SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		    SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Y", &l_nodeSet, &g_umeter, SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		    SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Z", &l_nodeSet, &g_umeter, SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		    SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));

  saf_declare_field(SAF_ALL, g_db, &g_coordsFtmpl, "coords", &l_nodeSet, &g_umeter, SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		    SAF_DOUBLE, l_componentField, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &l_field); 


  {/*note this tests SAF_INTERLEAVE_COMPONENT*/
    double l_eightHexPts[3*8]= {
      -.25, -.25, -.25,   .25, -.25, -.25,  .25, .25, -.25,   -.25, .25, -.25,
      -.25, -.25, .25,    .25, -.25, .25,   .25, .25, .25,    -.25, .25, .25
    };
    int i,j;
    double *l_data;
    l_data = (double *)malloc(3*8*l_numTypes*sizeof(double));
    for(i=0;i<l_numTypes;i++)
      {
	for(j=0;j<8;j++)
	  {
	    l_data[                 i*8 + j]=l_eightHexPts[j*3+0]+a_x+i*1.0;
	    l_data[1*8*l_numTypes + i*8 + j]=l_eightHexPts[j*3+1]+a_y;
	    l_data[2*8*l_numTypes + i*8 + j]=l_eightHexPts[j*3+2];
	  }
      }
    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, (void **)(&l_data), g_safFile); 
    free(l_data);
  }

  saf_declare_coords(SAF_ALL, &l_field);
  saf_declare_coords(SAF_ALL, &(l_componentField[0]) );
  saf_declare_coords(SAF_ALL, &(l_componentField[1]) );
  saf_declare_coords(SAF_ALL, &(l_componentField[2]) );
   
  saf_declare_default_coords(SAF_ALL,&l_nodeSet,&l_field);


  /*Write the global scalar nodal variable on the set. This variable
    will be mapped to every part. It must use g_ukelvin units, to match all the others*/
  {
    SAF_Field l_fieldvar;
    double buf[8]= { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0 };
    double *l_data;
    void *pbuf;
    l_data = (double *)malloc(8*l_numTypes*sizeof(double));
    pbuf = &l_data[0];

    for(i=0;i<l_numTypes;i++)
      {
	int j;
	for(j=0;j<8;j++)
	  {
	    l_data[i*8+j]=buf[j];
	  }
      }

    saf_declare_field(SAF_ALL, g_db, &g_globalNodalVarFtmpl, "global_nodal_var", &l_nodeSet, &g_ukelvin, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_nodes),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_fieldvar);
    saf_write_field(SAF_ALL, &l_fieldvar, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
    free(l_data);
  }





  /*Write the global scalar nodal vector variable on the top set. This variable
    will be mapped to every part. It must use g_ukilogram units, to match all the others*/
  {
    SAF_Field l_field;
    SAF_FieldTmpl l_temp[3];
    SAF_Field l_componentField[3];
    float *l_data;
    void *pbuf;
    l_data = (float *)malloc(3*8*l_numTypes*sizeof(float));
    pbuf = &l_data[0];

    for(i=0;i<l_numTypes;i++)
      {
	int j;
	for(j=0;j<8;j++)
	  {
	    l_data[i*8*3+j*3+0]=(float)j;
	    l_data[i*8*3+j*3+1]=7.0-(float)j;
	    l_data[i*8*3+j*3+2]=(float)j;
	  }
      }

    l_temp[0] = g_globalNodalComponentVectorVarFtmpl;
    l_temp[1] = g_globalNodalComponentVectorVarFtmpl;
    l_temp[2] = g_globalNodalComponentVectorVarFtmpl;

    saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "diff_name_Xglobal_nodal_vector_var", &l_nodeSet, 
		      &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
    saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Yglobal_nodal_vector_var", &l_nodeSet, &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
    saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Zglobal_nodal_vector_var", &l_nodeSet, &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));

    saf_declare_field(SAF_ALL, g_db, &g_globalNodalVectorVarFtmpl, "global_nodal_vector_var", &l_nodeSet, &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_FLOAT, l_componentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &l_field);

    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
    free(l_data);
  }



  for(i=0;i<l_numTypes;i++)
    {
      SAF_Set l_set;
      SAF_Rel l_rel;

      sprintf(l_setName,"%s_INDIR_SET",get_celltype_desc(l_types[i]));
      saf_declare_set(SAF_ALL, g_db, l_setName+strlen("SAF_CELLTYPE_"), /*get rid of the leading SAF_CELLTYPE_*/
		      2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &l_set);


      if( l_types[i] == SAF_CELLTYPE_LINE ) l_numPoints=2;
      else if( l_types[i] == SAF_CELLTYPE_TRI ) l_numPoints=3;
      else if( l_types[i] == SAF_CELLTYPE_QUAD ) l_numPoints=4;
      else if( l_types[i] == SAF_CELLTYPE_TET ) l_numPoints=4;
      else if( l_types[i] == SAF_CELLTYPE_PYRAMID ) l_numPoints=5;
      else if( l_types[i] == SAF_CELLTYPE_HEX ) l_numPoints=8;
      else if( l_types[i] == SAF_CELLTYPE_PRISM ) l_numPoints=6;
      else
	{
	  printf("\nerror: create_single_element_indirect_block, cant yet handle %s\n\n", 
		 get_celltype_desc(l_types[i]) );
	  exit(-1);
	}


      saf_declare_collection(SAF_ALL, &l_set, &g_elems, l_types[i], l_numElems, SAF_1DC(l_numElems), SAF_DECOMP_TRUE);

      saf_declare_topo_relation(SAF_ALL, g_db, &l_set, &g_elems, &l_nodeSet, &g_nodes, SAF_SELF(g_db), &l_set,
				SAF_UNSTRUCTURED, 0, NULL, 0, NULL, &l_rel);

      { 
	int j;
	int bbuf[24] = {0,1,2,3,4,5,6,7,  8,9,10,11,12,13,14,15, 16,17,18,19,20,21,22,23 };/*larger than the largest element?*/
	for(j=0;j<l_numPoints;j++) bbuf[j] += i*8;



	if( l_types[i] == SAF_CELLTYPE_TET ) 
	  {
	    /*Copy the 4th node coords to the 3rd node, otherwise tet will look like a quad, 
	      because all points will be in a plane*/
	    bbuf[3] += 1;
	  }
	else if( l_types[i] == SAF_CELLTYPE_PRISM ) 
	  {	/* SAF_CELLTYPE_PRISM or vtkWedge are two triangles (0,1,2) and (3,4,5) connected by 3 quads.*/
	    bbuf[3] += 1;
	    bbuf[4] += 1;
	    bbuf[5] += 1;
	  }

	saf_write_topo_relation(SAF_ALL, &l_rel, SAF_INT, &l_numPoints, SAF_INT, bbuf, g_safFile);
      }

      
      /*Write the global scalar elem variable on the top set. This variable
	will be mapped to every part. It must use g_uampere units, to match all the others*/
      {
	SAF_Field l_fieldb;
	double buf[1];
	void *pbuf = &buf[0];
	buf[0]=a_x+i*1.0+1.0;

	saf_declare_field(SAF_ALL, g_db, &g_globalElemVarFtmpl, "global_elem_var", &l_set, &g_uampere, SAF_SELF(g_db), 
			  SAF_ZONAL(&g_elems),
			  SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_fieldb);
	saf_write_field(SAF_ALL, &l_fieldb, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
      }



      /*Write the global scalar elem vector variable on the top set. This variable
	will be mapped to every part. It must use g_umeter units, to match all the others*/
      {
	SAF_Field l_field;
	SAF_FieldTmpl l_temp[3];
	SAF_Field l_componentField[3];
	double buf[3];
	void *pbuf = &buf[0];
	buf[0]=  (i+1)*1.0;
	buf[1]=  (7-i)*1.0; 
	buf[2]=  (i+1)*1.0;
	l_temp[0] = g_coordsComponentFtmpl;
	l_temp[1] = g_coordsComponentFtmpl;
	l_temp[2] = g_coordsComponentFtmpl;
	saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Xglobal_elem_vector_var", &l_set, &g_umeter, SAF_SELF(g_db), 
			  SAF_ZONAL(&g_elems),
			  SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
	saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Yglobal_elem_vector_var", &l_set, &g_umeter, SAF_SELF(g_db), 
			  SAF_ZONAL(&g_elems),
			  SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
	saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Zglobal_elem_vector_var", &l_set, &g_umeter, SAF_SELF(g_db), 
			  SAF_ZONAL(&g_elems),
			  SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));
	saf_declare_field(SAF_ALL, g_db, &g_globalElemVectorVarFtmpl, "global_elem_vector_var", &l_set, &g_umeter, 
			  SAF_SELF(g_db), SAF_ZONAL(&g_elems),
			  SAF_DOUBLE, l_componentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &l_field);
	saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
      }




    }
}






/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Unstructured Mesh File (create_unstr_mesh)
 * Purpose:     
 *
 * Description: 
 *
 *
 *--------------------------------------------------------------------------------------------------- */
void create_mixed_element_block( double a_x, double a_y )
{
  SAF_Set l_set1,l_set1a,l_set1b,l_set1a1,l_set1a2;
  SAF_Field l_coordfield, l_componentField[3];
  SAF_Rel l_rel;

  int l_orig = -99; /*An arbitrary indexing origin*/


  saf_declare_set(SAF_ALL, g_db, "MIXED_QUAD_TRI_TREE_SET_1",   2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &l_set1);
  saf_declare_set(SAF_ALL, g_db, "MIXED_QUAD_TRI_TREE_SET_1a",  2, SAF_SPACE, SAF_EXTENDIBLE_TRUE, &l_set1a);
  saf_declare_set(SAF_ALL, g_db, "MIXED_QUAD_TRI_TREE_SET_1b",  2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &l_set1b);
  saf_declare_set(SAF_ALL, g_db, "MIXED_QUAD_TRI_TREE_SET_1a1", 2, SAF_SPACE, SAF_EXTENDIBLE_TRUE, &l_set1a1);
  saf_declare_set(SAF_ALL, g_db, "MIXED_QUAD_TRI_TREE_SET_1a2", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &l_set1a2);

  saf_declare_collection(SAF_ALL, &l_set1,   &g_nodes, SAF_CELLTYPE_POINT, 8, 
			 _saf_indexspec(1,8,l_orig,0), SAF_DECOMP_FALSE); /*note arbitrary indexing origin*/

  saf_declare_collection(SAF_ALL, &l_set1a,  &g_nodes, SAF_CELLTYPE_POINT, 8, SAF_1DC(8), SAF_DECOMP_FALSE); 
  saf_declare_collection(SAF_ALL, &l_set1b,  &g_nodes, SAF_CELLTYPE_POINT, 4, SAF_1DC(4), SAF_DECOMP_FALSE); 
  saf_declare_collection(SAF_ALL, &l_set1a1, &g_nodes, SAF_CELLTYPE_POINT, 8, SAF_1DC(8), SAF_DECOMP_FALSE); 
  saf_declare_collection(SAF_ALL, &l_set1a2, &g_nodes, SAF_CELLTYPE_POINT, 4, SAF_1DC(4), SAF_DECOMP_FALSE); 

  saf_declare_collection(SAF_ALL, &l_set1,   &g_elems, SAF_CELLTYPE_MIXED, 7, SAF_1DC(7), SAF_DECOMP_FALSE); 
  saf_declare_collection(SAF_ALL, &l_set1a,  &g_elems, SAF_CELLTYPE_MIXED, 5, SAF_1DC(5), SAF_DECOMP_FALSE); 
  saf_declare_collection(SAF_ALL, &l_set1b,  &g_elems, SAF_CELLTYPE_TRI,   2, SAF_1DC(2), SAF_DECOMP_FALSE); 
  saf_declare_collection(SAF_ALL, &l_set1a1, &g_elems, SAF_CELLTYPE_QUAD,  4, SAF_1DC(4), SAF_DECOMP_FALSE); 
  saf_declare_collection(SAF_ALL, &l_set1a2, &g_elems, SAF_CELLTYPE_QUAD,  1, SAF_1DC(1), SAF_DECOMP_FALSE); 


  /*do the elems subsets: so far nothing interesting here*/
  {
    int l_conn[] = { 6,1,2,3,0 };
    saf_declare_subset_relation(SAF_ALL, g_db, &l_set1, &l_set1a, SAF_COMMON(&g_elems), SAF_TUPLES, 0, NULL, 0, NULL, &l_rel);
    saf_write_subset_relation(SAF_ALL, &l_rel, SAF_INT, l_conn, 0, NULL, g_safFile);
  }
  {
    int l_conn[] = { 5,4 };
    saf_declare_subset_relation(SAF_ALL, g_db, &l_set1, &l_set1b, SAF_COMMON(&g_elems), SAF_TUPLES,0, NULL, 0, NULL, &l_rel);
    saf_write_subset_relation(SAF_ALL, &l_rel, SAF_INT, l_conn, 0, NULL, g_safFile);
  }
  {
    int l_conn[] = { 0,4,2,3 };
    saf_declare_subset_relation(SAF_ALL, g_db, &l_set1a, &l_set1a1, SAF_COMMON(&g_elems), SAF_TUPLES,0, NULL, 0, NULL, &l_rel);
    saf_write_subset_relation(SAF_ALL, &l_rel, SAF_INT, l_conn, 0, NULL, g_safFile);
  }
  {
    int l_conn[] = { 1 };
    saf_declare_subset_relation(SAF_ALL, g_db, &l_set1a, &l_set1a2, SAF_COMMON(&g_elems), SAF_TUPLES,0, NULL, 0, NULL, &l_rel);
    saf_write_subset_relation(SAF_ALL, &l_rel, SAF_INT, l_conn, 0, NULL, g_safFile);
  }






  /*set1a has a subset of set1's nodes, and set 1a1 has a topo rel connecting to set1a*/
  {
    int l_conn[] = { 7,6,5,4,3,2,1,0 };
    int i;
    for(i=0;i<8;i++) l_conn[i] += l_orig; /*note arbitrary indexing origin*/
    saf_declare_subset_relation(SAF_ALL, g_db, &l_set1, &l_set1a, SAF_COMMON(&g_nodes), SAF_TUPLES, 0, NULL, 0, NULL, &l_rel);
    saf_write_subset_relation(SAF_ALL, &l_rel, SAF_INT, l_conn, 0, NULL, g_safFile);
  }

  {
    int l_abuf[]={4};
    int l_bbuf[] = {0,1,2,3, 0,1,5,4, 1,2,6,5, 3,0,4,7};
    saf_declare_topo_relation(SAF_ALL, g_db, &l_set1a1, &g_elems, 
			      &l_set1a, /*note: had probs with this: set1 worked, but not set1a*/
			      &g_nodes, SAF_SELF(g_db), &l_set1a1,
			      SAF_UNSTRUCTURED, 0,NULL,0,NULL, &l_rel);
    saf_write_topo_relation(SAF_ALL, &l_rel, SAF_INT, l_abuf, SAF_INT, l_bbuf, g_safFile);
  }

  /*set1a2 has a subset of set1a's nodes, and a topo rel pointing to itself*/
  {
    int l_conn[] = { 4,5,6,7 };
    saf_declare_subset_relation(SAF_ALL, g_db, &l_set1a, &l_set1a2, SAF_COMMON(&g_nodes), SAF_TUPLES, 0, NULL, 0, NULL, &l_rel);
    saf_write_subset_relation(SAF_ALL, &l_rel, SAF_INT, l_conn, 0, NULL, g_safFile);
  }
  {
    int l_abuf[]={4};
    int l_bbuf[] = { 0,1,2,3 };
    saf_declare_topo_relation(SAF_ALL, g_db, &l_set1a2, &g_elems, 
			      &l_set1a2, 
			      &g_nodes, SAF_SELF(g_db), &l_set1a2,
			      SAF_UNSTRUCTURED, 0,NULL,0,NULL, &l_rel);
    saf_write_topo_relation(SAF_ALL, &l_rel, SAF_INT, l_abuf, SAF_INT, l_bbuf, g_safFile);
  }


  /*set1b is simple: no nodes, topo rel pointing to set 1*/
  {
    int l_abuf[]={3};
    int l_bbuf[] = {0,1,5,5,4,0};/*XXX*/ 
    int i;
    for(i=0;i<6;i++) l_bbuf[i] += l_orig; /*note arbitrary indexing origin*/
    saf_declare_topo_relation(SAF_ALL, g_db, &l_set1b, &g_elems, 
			      &l_set1, 
			      &g_nodes, SAF_SELF(g_db), &l_set1b,
			      SAF_UNSTRUCTURED, 0,NULL,0,NULL, &l_rel);
    saf_write_topo_relation(SAF_ALL, &l_rel, SAF_INT, l_abuf, SAF_INT, l_bbuf, g_safFile);
  }




  /*write the coordinate fields*/
  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "X", &l_set1, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Y", &l_set1, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Z", &l_set1, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));

  saf_declare_field(SAF_ALL, g_db, &g_coordsFtmpl, "coords", &l_set1, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_DOUBLE, l_componentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &l_coordfield); 


  {
    /*Note this tests writing vectors to the individual components*/
    double l_eightHexPtsX[8]= {
      -.25,  .25, .25, -.25, 
      -.25,  .25, .25, -.25
    };
    double l_eightHexPtsY[8]= {
      -.25, -.25, .25, .25, 
      -.25, -.25, .25, .25
    };
    double l_eightHexPtsZ[8]= {
      -.25, -.25, -.25, -.25, 
      .25,   .25,  .25,  .25
    };

    void *pbuf;
    int i;
    for(i=0;i<8;i++) l_eightHexPtsX[i] += a_x;
    for(i=0;i<8;i++) l_eightHexPtsY[i] += a_y;

    pbuf = &l_eightHexPtsX[0];
    saf_write_field(SAF_ALL, &(l_componentField[0]), SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
    pbuf = &l_eightHexPtsY[0];
    saf_write_field(SAF_ALL, &(l_componentField[1]), SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
    pbuf = &l_eightHexPtsZ[0];
    saf_write_field(SAF_ALL, &(l_componentField[2]), SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }


  saf_declare_coords(SAF_ALL, &l_coordfield);
  saf_declare_coords(SAF_ALL, &(l_componentField[0]) );
  saf_declare_coords(SAF_ALL, &(l_componentField[1]) );
  saf_declare_coords(SAF_ALL, &(l_componentField[2]) );
   
  saf_declare_default_coords(SAF_ALL,&l_set1,&l_coordfield);

 
  /*Write the global scalar nodal variable on the top set (l_set1). This variable
    will be mapped to every part. It must use g_ukelvin units, to match all the others*/
  {
    SAF_Field l_field;

    double buf[8]= { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0 };
    void *pbuf = &buf[0];

    saf_declare_field(SAF_ALL, g_db, &g_globalNodalVarFtmpl, "global_nodal_var_with_extra_name", &l_set1, &g_ukelvin, SAF_SELF(g_db), 
		      SAF_NODAL(&g_nodes, &g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }



  /*Write the global scalar nodal vector variable on the top set. This variable
    will be mapped to every part. It must use g_ukilogram units, to match all the others*/
  {
    SAF_Field l_field;
    SAF_FieldTmpl l_temp[3];
    SAF_Field l_componentField[3];
    int l_data[3*8] = { 0,1,2,3,4,5,6,7, 7,6,5,4,3,2,1,0, 0,1,2,3,4,5,6,7 };
    void *pbuf = &l_data[0];

    l_temp[0] = g_globalNodalComponentVectorVarFtmpl;
    l_temp[1] = g_globalNodalComponentVectorVarFtmpl;
    l_temp[2] = g_globalNodalComponentVectorVarFtmpl;

    saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Xglobal_nodal_vector_var", &l_set1, &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_INT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
    saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Yglobal_nodal_vector_var", &l_set1, &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_INT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
    saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Zglobal_nodal_vector_var", &l_set1, &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_INT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));

    saf_declare_field(SAF_ALL, g_db, &g_globalNodalVectorVarFtmpl, "global_nodal_vector_var", &l_set1, &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_INT, l_componentField, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &l_field);

    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }



  /*Write the global scalar elem variable on the top set. This variable
    will be mapped to every part. It must use g_uampere units, to match all the others*/
  {
    SAF_Field l_field;
    long buf[7]={ 1,2,3,4,5,6,7 };
    void *pbuf = &buf[0];

    /*note this tests using SAF_LONG for a variable*/
    saf_declare_field(SAF_ALL, g_db, &g_globalElemVarFtmpl, "global_elem_var", &l_set1, &g_uampere, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_elems),
		      SAF_LONG, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }


  /*Write the global scalar elem vector variable on the top set. This variable
    will be mapped to every part. It must use g_umeter units, to match all the others*/
  {
    SAF_Field l_field;
    SAF_FieldTmpl l_temp[3];
    SAF_Field l_componentField[3];
    double buf[7*3]={ 1,2,3,4,5,6,7, 7,6,5,4,3,2,1, 1,2,3,4,5,6,7 };
    void *pbuf = &buf[0];
    l_temp[0] = g_coordsComponentFtmpl;
    l_temp[1] = g_coordsComponentFtmpl;
    l_temp[2] = g_coordsComponentFtmpl;
    saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Xglobal_elem_vector_var", &l_set1, &g_umeter, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
    saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Yglobal_elem_vector_var", &l_set1, &g_umeter, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
    saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Zglobal_elem_vector_var", &l_set1, &g_umeter, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));
    saf_declare_field(SAF_ALL, g_db, &g_globalElemVectorVarFtmpl, "global_elem_vector_var", &l_set1, &g_umeter, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, l_componentField, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &l_field);
    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }



}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Unstructured Mesh File (create_unstr_mesh)
 * Purpose:     
 *
 * Description: 
 *
 *
 *--------------------------------------------------------------------------------------------------- */
void create_elem_subset_vars_block( double a_x, double a_y )
{
  SAF_Set l_set1,l_set1a,l_set1b,l_set1a1,l_set1a2;
  SAF_Field l_coordfield, l_componentField[3];
  SAF_Rel l_rel;


  saf_declare_set(SAF_ALL, g_db, "SUBSET_ELEM_VAR_SET_1",   2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &l_set1);
  saf_declare_set(SAF_ALL, g_db, "SUBSET_ELEM_VAR_SET_1a",  2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &l_set1a);
  saf_declare_set(SAF_ALL, g_db, "SUBSET_ELEM_VAR_SET_1b",  2, SAF_SPACE, SAF_EXTENDIBLE_TRUE, &l_set1b);
  saf_declare_set(SAF_ALL, g_db, "SUBSET_ELEM_VAR_SET_1a1", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &l_set1a1);
  saf_declare_set(SAF_ALL, g_db, "SUBSET_ELEM_VAR_SET_1a2", 2, SAF_SPACE, SAF_EXTENDIBLE_TRUE, &l_set1a2);

  saf_declare_collection(SAF_ALL, &l_set1,   &g_nodes, SAF_CELLTYPE_POINT, 8, SAF_1DC(8), SAF_DECOMP_FALSE); 
  saf_declare_collection(SAF_ALL, &l_set1,   &g_elems, SAF_CELLTYPE_TRI, 12, SAF_1DC(12), SAF_DECOMP_FALSE); 



  /*the only topo rel is on the top set (l_set1), pointing to the nodes on itself*/
  {
    int l_abuf[]={3};
    int l_bbuf[] = { 0,2,1, 0,3,2, 4,5,6, 4,6,7, 1,2,5, 5,2,6, 4,7,3, 3,0,4, 0,1,5, 0,5,4, 2,3,7, 7,6,2 };

    saf_declare_topo_relation(SAF_ALL, g_db, &l_set1, &g_elems, &l_set1, 
			      &g_nodes, SAF_SELF(g_db), &l_set1,
			      SAF_UNSTRUCTURED, 0,NULL,0,NULL, &l_rel);
    saf_write_topo_relation(SAF_ALL, &l_rel, SAF_INT, l_abuf, SAF_INT, l_bbuf, g_safFile);
  }


  saf_declare_collection(SAF_ALL, &l_set1a,  &g_elems, SAF_CELLTYPE_TRI, 8, SAF_1DC(8), SAF_DECOMP_FALSE); 
  saf_declare_collection(SAF_ALL, &l_set1b,  &g_elems, SAF_CELLTYPE_TRI, 4, SAF_1DC(4), SAF_DECOMP_FALSE); 


  /*note that this collection has fortran ordering, but since nothing ever points to this collection
    (i.e. it is never a superset and has no topo rels pointing to it), it doesnt matter*/
  saf_declare_collection(SAF_ALL, &l_set1a1, &g_elems, SAF_CELLTYPE_TRI, 4, SAF_1DF(4), SAF_DECOMP_FALSE); 


  saf_declare_collection(SAF_ALL, &l_set1a2, &g_elems, SAF_CELLTYPE_TRI, 4, SAF_1DC(4), SAF_DECOMP_FALSE); 


  /*do the elems subsets*/
  {
    int l_conn[] = { 4,3,2,1, 6,8,10,11 };
    saf_declare_subset_relation(SAF_ALL, g_db, &l_set1, &l_set1a, SAF_COMMON(&g_elems), SAF_TUPLES, 0, NULL, 0, NULL, &l_rel);
    saf_write_subset_relation(SAF_ALL, &l_rel, SAF_INT, l_conn, 0, NULL, g_safFile);
  }
  {
    int l_conn[] = { 0,5,7,9 };
    saf_declare_subset_relation(SAF_ALL, g_db, &l_set1, &l_set1b, SAF_COMMON(&g_elems), SAF_TUPLES,0, NULL, 0, NULL, &l_rel);
    saf_write_subset_relation(SAF_ALL, &l_rel, SAF_INT, l_conn, 0, NULL, g_safFile);
  }
  {
    int l_conn[] = { 0,2,4,6 }; 
    saf_declare_subset_relation(SAF_ALL, g_db, &l_set1a, &l_set1a1, SAF_COMMON(&g_elems), SAF_TUPLES,0, NULL, 0, NULL, &l_rel);
    saf_write_subset_relation(SAF_ALL, &l_rel, SAF_INT, l_conn, 0, NULL, g_safFile);
  }
  {
    int l_conn[] = { 1,3,5,7 };
    saf_declare_subset_relation(SAF_ALL, g_db, &l_set1a, &l_set1a2, SAF_COMMON(&g_elems), SAF_TUPLES,0, NULL, 0, NULL, &l_rel);
    saf_write_subset_relation(SAF_ALL, &l_rel, SAF_INT, l_conn, 0, NULL, g_safFile);
  }



  /*write the coordinate fields*/
  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "X", &l_set1, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Y", &l_set1, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Z", &l_set1, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));

  saf_declare_field(SAF_ALL, g_db, &g_coordsFtmpl, "coords", &l_set1, &g_umeter, SAF_SELF(g_db), SAF_NODAL(&g_nodes, &g_elems),
		    SAF_DOUBLE, l_componentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &l_coordfield); 

  {
    double buf[3*8]= {
      -.25, -.25, -.25,   .25, -.25, -.25,  .25, .25, -.25,   -.25, .25, -.25,
      -.25, -.25, .25,    .25, -.25, .25,   .25, .25, .25,    -.25, .25, .25
    };
    void *pbuf = &buf[0];
    int i;
    for(i=0;i<3*8;i+=3) buf[i] += a_x;
    for(i=1;i<3*8;i+=3) buf[i] += a_y;

    saf_write_field(SAF_ALL, &l_coordfield, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }

  saf_declare_coords(SAF_ALL, &l_coordfield);
  saf_declare_coords(SAF_ALL, &(l_componentField[0]) );
  saf_declare_coords(SAF_ALL, &(l_componentField[1]) );
  saf_declare_coords(SAF_ALL, &(l_componentField[2]) );
   
  saf_declare_default_coords(SAF_ALL,&l_set1,&l_coordfield);


  /*Write the global scalar nodal variable on the top set (l_set1). This variable
    will be mapped to every part. It must use g_ukelvin units, to match all the others*/
  {
    SAF_Field l_field;

    int buf[8]= { 0, 1, 2, 3, 4, 5, 6, 7 };
    void *pbuf = &buf[0];

    /*Note this tests using SAF_INT for a variable field*/
    saf_declare_field(SAF_ALL, g_db, &g_globalNodalVarFtmpl, "global_nodal_var", &l_set1, &g_ukelvin, SAF_SELF(g_db), 
		      SAF_NODAL(&g_nodes, &g_elems),
		      SAF_INT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }


  /*Write the global scalar nodal vector variable on the top set. This variable
    will be mapped to every part. It must use g_ukilogram units, to match all the others*/
  {
    SAF_Field l_field;
    SAF_FieldTmpl l_temp[3];
    SAF_Field l_componentField[3];
    long int l_data[3*8] = { 0,1,2,3,4,5,6,7, 7,6,5,4,3,2,1,0, 0,1,2,3,4,5,6,7 };
    void *pbuf = &l_data[0];

    l_temp[0] = g_globalNodalComponentVectorVarFtmpl;
    l_temp[1] = g_globalNodalComponentVectorVarFtmpl;
    l_temp[2] = g_globalNodalComponentVectorVarFtmpl;

    saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Xglobal_nodal_vector_var", &l_set1, &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_LONG, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
    saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Yglobal_nodal_vector_var", &l_set1, &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_LONG, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
    saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Zglobal_nodal_vector_var", &l_set1, &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_LONG, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));

    saf_declare_field(SAF_ALL, g_db, &g_globalNodalVectorVarFtmpl, "global_nodal_vector_var", &l_set1, &g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		      SAF_LONG, l_componentField, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &l_field);

    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }




  /*THIS IS THE HARD TEST*/
  /*Write the global scalar elem variable on each of the bottom sets. This variable
    will be mapped to every part. It must use g_uampere units, to match all the others*/

  /*NOTE: THE COLORS SEEM TO MATCH create_mixed_element_block, but should check again.....it doesnt seem right XXX*/
  {
    SAF_Field l_field;
    float buf[4]={ 3,7,4,1};
    void *pbuf = &buf[0];
    /*Note this is testing having a diff type for the same var connected by subset rels (this is float, rest are double)*/
    saf_declare_field(SAF_ALL, g_db, &g_globalElemVarFtmpl, "global_elem_var", &l_set1a1, &g_uampere, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_elems),
		      SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }
  {
    SAF_Field l_field;
    double buf[4]={ 7,2,6,1 };
    void *pbuf = &buf[0];
    saf_declare_field(SAF_ALL, g_db, &g_globalElemVarFtmpl, "global_elem_var", &l_set1a2, &g_uampere, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }
  {
    SAF_Field l_field;
    double buf[4]={ 2,3,4,5 };
    void *pbuf = &buf[0];
    saf_declare_field(SAF_ALL, g_db, &g_globalElemVarFtmpl, "global_elem_var", &l_set1b, &g_uampere, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }



  /*Write the global VECTOR elem var on all the bottom sets*/
  {
    SAF_Field l_field;
    SAF_FieldTmpl l_temp[3];
    SAF_Field l_componentField[3];
    void *pbuf;

    l_temp[0] = g_coordsComponentFtmpl;
    l_temp[1] = g_coordsComponentFtmpl;
    l_temp[2] = g_coordsComponentFtmpl;

    {
      SAF_Set l_theSet = l_set1a1;
      float buf[4*3]={ 3,8-3,3,   7,8-7,7,   4,8-4,4,    1,8-1,1 };
      pbuf = &buf[0];
      saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Xglobal_elem_vector_var", &l_theSet, &g_umeter, SAF_SELF(g_db), 
			SAF_ZONAL(&g_elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
      saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Yglobal_elem_vector_var", &l_theSet, &g_umeter, SAF_SELF(g_db), 
			SAF_ZONAL(&g_elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
      saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Zglobal_elem_vector_var", &l_theSet, &g_umeter, SAF_SELF(g_db), 
			SAF_ZONAL(&g_elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));
      saf_declare_field(SAF_ALL, g_db, &g_globalElemVectorVarFtmpl, "global_elem_vector_var", &l_theSet, &g_umeter, SAF_SELF(g_db), 
			SAF_ZONAL(&g_elems), SAF_FLOAT, l_componentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &l_field);
      saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
    }
    {
      SAF_Set l_theSet = l_set1a2;
      float buf[4*3]={ 7,8-7,7,   2,8-2,2,    6,8-6,6,   1,8-1,1 };
      pbuf = &buf[0];
      saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Xglobal_elem_vector_var", &l_theSet, &g_umeter, SAF_SELF(g_db), 
			SAF_ZONAL(&g_elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
      saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Yglobal_elem_vector_var", &l_theSet, &g_umeter, SAF_SELF(g_db), 
			SAF_ZONAL(&g_elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
      saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Zglobal_elem_vector_var", &l_theSet, &g_umeter, SAF_SELF(g_db), 
			SAF_ZONAL(&g_elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));
      saf_declare_field(SAF_ALL, g_db, &g_globalElemVectorVarFtmpl, "global_elem_vector_var", &l_theSet, &g_umeter, SAF_SELF(g_db), 
			SAF_ZONAL(&g_elems), SAF_FLOAT, l_componentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &l_field);
      saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
    }
    {
      SAF_Set l_theSet = l_set1b;
      float buf[4*3]={ 2,8-2,2,   3,8-3,3,   4,8-4,4,    5,8-5,5  };
      pbuf = &buf[0];
      saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Xglobal_elem_vector_var", &l_theSet, &g_umeter, SAF_SELF(g_db), 
			SAF_ZONAL(&g_elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
      saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Yglobal_elem_vector_var", &l_theSet, &g_umeter, SAF_SELF(g_db), 
			SAF_ZONAL(&g_elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
      saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Zglobal_elem_vector_var", &l_theSet, &g_umeter, SAF_SELF(g_db), 
			SAF_ZONAL(&g_elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));
      saf_declare_field(SAF_ALL, g_db, &g_globalElemVectorVarFtmpl, "global_elem_vector_var", &l_theSet, &g_umeter, SAF_SELF(g_db), 
			SAF_ZONAL(&g_elems), SAF_FLOAT, l_componentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &l_field);
      saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
    }


  }



}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Unstructured Mesh File (create_unstr_mesh)
 * Purpose:     
 *
 * Description: 
 *
 *
 *--------------------------------------------------------------------------------------------------- */
void create_node_subset_vars_block( double a_x, double a_y )
{
  SAF_Set l_set1,l_set1a,l_set1b,l_set1a1,l_set1a2;
  SAF_Field l_coordfield, l_componentField[3];
  SAF_Rel l_rel;

  saf_declare_set(SAF_ALL, g_db, "SUBSET_NODE_VAR_SET_1",   2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &l_set1);
  saf_declare_set(SAF_ALL, g_db, "SUBSET_NODE_VAR_SET_1a",  2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &l_set1a);
  saf_declare_set(SAF_ALL, g_db, "SUBSET_NODE_VAR_SET_1b",  2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &l_set1b);
  saf_declare_set(SAF_ALL, g_db, "SUBSET_NODE_VAR_SET_1a1", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &l_set1a1);
  saf_declare_set(SAF_ALL, g_db, "SUBSET_NODE_VAR_SET_1a2", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &l_set1a2);

  saf_declare_collection(SAF_ALL, &l_set1,   &g_nodes, SAF_CELLTYPE_POINT, 8, SAF_1DC(8), SAF_DECOMP_FALSE); 
  saf_declare_collection(SAF_ALL, &l_set1a,  &g_nodes, SAF_CELLTYPE_POINT, 8, SAF_1DF(8), SAF_DECOMP_FALSE);/*note fortran order*/
  saf_declare_collection(SAF_ALL, &l_set1b,  &g_nodes, SAF_CELLTYPE_POINT, 3, SAF_1DC(3), SAF_DECOMP_FALSE); 
  saf_declare_collection(SAF_ALL, &l_set1a1, &g_nodes, SAF_CELLTYPE_POINT, 4, SAF_1DC(4), SAF_DECOMP_FALSE); 
  saf_declare_collection(SAF_ALL, &l_set1a2, &g_nodes, SAF_CELLTYPE_POINT, 1, SAF_1DC(1), SAF_DECOMP_FALSE); 

  saf_declare_collection(SAF_ALL, &l_set1,   &g_elems, SAF_CELLTYPE_HEX, 1, SAF_1DC(1), SAF_DECOMP_FALSE); 


  { 
    int l_numPoints=8;
    int bbuf[8] = {0,1,2,3,4,5,6,7};
    saf_declare_topo_relation(SAF_ALL, g_db, &l_set1, &g_elems, &l_set1, &g_nodes, SAF_SELF(g_db), &l_set1,
			      SAF_UNSTRUCTURED, 0, NULL, 0, NULL, &l_rel);
    saf_write_topo_relation(SAF_ALL, &l_rel, SAF_INT, &l_numPoints, SAF_INT, bbuf, g_safFile);
  }

 
  /*do the node subsets*/
  {
    int l_conn[] = { 7,6,5,4,3,2,1,0 };
    saf_declare_subset_relation(SAF_ALL, g_db, &l_set1, &l_set1a, SAF_COMMON(&g_nodes), SAF_TUPLES, 0, NULL, 0, NULL, &l_rel);
    saf_write_subset_relation(SAF_ALL, &l_rel, SAF_INT, l_conn, 0, NULL, g_safFile);
  }
  {
    int l_conn[] = { 0,5,7 };
    saf_declare_subset_relation(SAF_ALL, g_db, &l_set1, &l_set1b, SAF_COMMON(&g_nodes), SAF_TUPLES,0, NULL, 0, NULL, &l_rel);
    saf_write_subset_relation(SAF_ALL, &l_rel, SAF_INT, l_conn, 0, NULL, g_safFile);
  }
  {
    int l_conn[] = { 1+1,3+1,4+1,5+1 };/*note fortran order*/
    saf_declare_subset_relation(SAF_ALL, g_db, &l_set1a, &l_set1a1, SAF_COMMON(&g_nodes), SAF_TUPLES,0, NULL, 0, NULL, &l_rel);
    saf_write_subset_relation(SAF_ALL, &l_rel, SAF_INT, l_conn, 0, NULL, g_safFile);
  }
  {
    int l_conn[] = { 6+1 };/*note fortran order*/
    saf_declare_subset_relation(SAF_ALL, g_db, &l_set1a, &l_set1a2, SAF_COMMON(&g_nodes), SAF_TUPLES,0, NULL, 0, NULL, &l_rel);
    saf_write_subset_relation(SAF_ALL, &l_rel, SAF_INT, l_conn, 0, NULL, g_safFile);
  }



  /*write the coordinate fields*/
  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "X", &l_set1, &g_umeter, SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		    SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Y", &l_set1, &g_umeter, SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		    SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
  saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Z", &l_set1, &g_umeter, SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		    SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));

  saf_declare_field(SAF_ALL, g_db, &g_coordsFtmpl, "coords", &l_set1, &g_umeter, SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
		    SAF_DOUBLE, l_componentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &l_coordfield); 

  {
    double buf[3*8]= {
      -.25, -.25, -.25,   .25, -.25, -.25,  .25, .25, -.25,   -.25, .25, -.25,
      -.25, -.25, .25,    .25, -.25, .25,   .25, .25, .25,    -.25, .25, .25
    };
    void *pbuf = &buf[0];
    int i;
    for(i=0;i<3*8;i+=3) buf[i] += a_x;
    for(i=1;i<3*8;i+=3) buf[i] += a_y;

    saf_write_field(SAF_ALL, &l_coordfield, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }

  saf_declare_coords(SAF_ALL, &l_coordfield);
  saf_declare_coords(SAF_ALL, &(l_componentField[0]) );
  saf_declare_coords(SAF_ALL, &(l_componentField[1]) );
  saf_declare_coords(SAF_ALL, &(l_componentField[2]) );
   
  saf_declare_default_coords(SAF_ALL,&l_set1,&l_coordfield);


  /*Write the global scalar elem variable on the top set. This variable
    will be mapped to every part. It must use g_uampere units, to match all the others*/
  {
    SAF_Field l_field;

    double buf[1];
    void *pbuf = &buf[0];
    buf[0]=a_x+1.0;

    saf_declare_field(SAF_ALL, g_db, &g_globalElemVarFtmpl, "global_elem_var", &l_set1, &g_uampere, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }



  /*THIS IS THE HARD TEST*/
  /*Write the global scalar node variable on each of the bottom sets. This variable
    will be mapped to every part. It must use g_ukelvin units, to match all the others*/
  {
    SAF_Field l_field;
    double buf[4]={ 6,4,3,2 };
    void *pbuf = &buf[0];
    saf_declare_field(SAF_ALL, g_db, &g_globalNodalVarFtmpl, "global_nodal_var", &l_set1a1, &g_ukelvin, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_nodes),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }
  {
    SAF_Field l_field;
    double buf[1]={ 1 };
    void *pbuf = &buf[0];
    saf_declare_field(SAF_ALL, g_db, &g_globalNodalVarFtmpl, "global_nodal_var", &l_set1a2, &g_ukelvin, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_nodes),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }
  {
    SAF_Field l_field;
    double buf[3]={ 0,5,7 };
    void *pbuf = &buf[0];
    saf_declare_field(SAF_ALL, g_db, &g_globalNodalVarFtmpl, "global_nodal_var", &l_set1b, &g_ukelvin, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_nodes),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }





  /*Write the global scalar nodal vector variable on each of the bottom sets. This variable
    will be mapped to every part. It must use g_ukilogram units, to match all the others*/
  {
    SAF_Field l_field;
    SAF_FieldTmpl l_temp[3];
    SAF_Field l_componentField[3];
    SAF_Set l_whichSet;
    l_temp[0] = g_globalNodalComponentVectorVarFtmpl;
    l_temp[1] = g_globalNodalComponentVectorVarFtmpl;
    l_temp[2] = g_globalNodalComponentVectorVarFtmpl;

    {
      double buf[3*4]={ 6,4,3,2,  1,3,4,5,  6,4,3,2 };
      void *pbuf = &buf[0];
      l_whichSet = l_set1a1;
      saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Xglobal_nodal_vector_var", &l_whichSet, &g_ukilogram, 
			SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
			SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
      saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Yglobal_nodal_vector_var", &l_whichSet, &g_ukilogram, 
			SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
			SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
      saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Zglobal_nodal_vector_var", &l_whichSet, &g_ukilogram, 
			SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
			SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));
      saf_declare_field(SAF_ALL, g_db, &g_globalNodalVectorVarFtmpl, "global_nodal_vector_var", &l_whichSet, &g_ukilogram, 
			SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
			SAF_DOUBLE, l_componentField, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &l_field);
      saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
    }
    {
      double buf[3*1]={ 1,6,1 };
      void *pbuf = &buf[0];
      l_whichSet = l_set1a2;
      saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Xglobal_nodal_vector_var", &l_whichSet, &g_ukilogram, 
			SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
			SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
      saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Yglobal_nodal_vector_var", &l_whichSet, &g_ukilogram, 
			SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
			SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
      saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Zglobal_nodal_vector_var", &l_whichSet, &g_ukilogram, 
			SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
			SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));
      saf_declare_field(SAF_ALL, g_db, &g_globalNodalVectorVarFtmpl, "global_nodal_vector_var", &l_whichSet, &g_ukilogram, 
			SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
			SAF_DOUBLE, l_componentField, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &l_field);
      saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
    }
    {
      double buf[3*3]={ 0,7,0,  5,2,5,   7,0,7 };
      void *pbuf = &buf[0];
      l_whichSet = l_set1b;
      saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Xglobal_nodal_vector_var", &l_whichSet, &g_ukilogram, 
			SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
			SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
      saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Yglobal_nodal_vector_var", &l_whichSet, &g_ukilogram, 
			SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
			SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
      saf_declare_field(SAF_ALL, g_db, &g_globalNodalComponentVectorVarFtmpl, "Zglobal_nodal_vector_var", &l_whichSet, &g_ukilogram, 
			SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
			SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));
      saf_declare_field(SAF_ALL, g_db, &g_globalNodalVectorVarFtmpl, "global_nodal_vector_var", &l_whichSet, &g_ukilogram, 
			SAF_SELF(g_db), SAF_ZONAL(&g_nodes),
			SAF_DOUBLE, l_componentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &l_field);
      saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
    }
  }










  /*Write the global scalar elem vector variable on the top set. This variable
    will be mapped to every part. It must use g_umeter units, to match all the others*/
  {
    SAF_Field l_field;
    SAF_FieldTmpl l_temp[3];
    SAF_Field l_componentField[3];

    double buf[3];
    void *pbuf = &buf[0];
    buf[0]=  a_x;
    buf[1]=  8.0-a_x;
    buf[2]=  a_x;

    l_temp[0] = g_coordsComponentFtmpl;
    l_temp[1] = g_coordsComponentFtmpl;
    l_temp[2] = g_coordsComponentFtmpl;

    saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Xglobal_elem_vector_var", &l_set1, &g_umeter, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
    saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Yglobal_elem_vector_var", &l_set1, &g_umeter, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
    saf_declare_field(SAF_ALL, g_db, &g_coordsComponentFtmpl, "Zglobal_elem_vector_var", &l_set1, &g_umeter, SAF_SELF(g_db), 
		      SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));

    saf_declare_field(SAF_ALL, g_db, &g_globalElemVectorVarFtmpl, "global_elem_vector_var", &l_set1, &g_umeter, 
		      SAF_SELF(g_db), SAF_ZONAL(&g_elems),
		      SAF_DOUBLE, l_componentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &l_field);

    saf_write_field(SAF_ALL, &l_field, SAF_WHOLE_FIELD, 1, 0, &pbuf, g_safFile); 
  }




}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Unstructured Mesh File (create_unstr_mesh)
 * Purpose:     main Function for create_unstr_mesh
 *
 * Description: 
 * This function parses the command line options and then creates a 
 * test database that exercises the various unstructured mesh routines defined here.
 *
 * Usage   
 *
 *   ./create_unstr_mesh [-h] [FILENAME]
 *
 * Command Line Options
 *
 *   -h, --help  print this help message and exit
 *
 *   FILENAME  create/overwrite this saf file, otherwise use file test_str_mesh.saf
 *
 *--------------------------------------------------------------------------------------------------- */
int main(int argc, char **argv)
{
#define MY_MAX_DBNAME_SIZE 1024

  char l_dbname[MY_MAX_DBNAME_SIZE];
  char l_dbnameDefault[]="test_unstr_mesh.saf";        /* Name of the SAF database file to be created. */
  SAF_DbProps *l_dbprops;      /* Handle to the SAF database properties. */


#ifdef HAVE_PARALLEL
  /* the MPI_init comes first because on some platforms MPICH's mpirun
   * doesn't pass the same argc, argv to all processors. However, the MPI
   * spec says nothing about what it does or might do to argc or argv. In
   * fact, there is no "const" in the function prototypes for either the
   * pointers or the things they're pointing too.  I would rather pass NULL
   * here and the spec says this is perfectly acceptable.  However, that too
   * has caused MPICH to core on certain platforms. */
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &g_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &g_nprocs);
#endif


  snprintf(l_dbname,MY_MAX_DBNAME_SIZE-1,"%s",l_dbnameDefault);

  /*PARSE COMMAND LINE*/
  {
    int l_whichArg;
    int l_gotFilename=0,l_printHelpAndExit=0;
    if(!g_rank)
      {
	for(l_whichArg=1; l_whichArg<argc; l_whichArg++)
	  {
	    if( !strcmp(argv[l_whichArg],"-h") ||  !strcmp(argv[l_whichArg],"-vh") ||
		!strcmp(argv[l_whichArg],"-hv") ||
		!strcmp(argv[l_whichArg],"--help") || argc>=6 )
	      {
		l_printHelpAndExit=1;
	      }
	    else if(!l_gotFilename)
	      {
		l_gotFilename=1;
		snprintf(l_dbname,MY_MAX_DBNAME_SIZE-1,argv[l_whichArg]);
	      }
	    else
	      {
		l_printHelpAndExit=1;
	      }
	    if(l_printHelpAndExit) break;
	  }
      }

#ifdef HAVE_PARALLEL    
    MPI_Bcast(&l_printHelpAndExit, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&l_gotFilename, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if(l_gotFilename)
      {
	MPI_Bcast(l_dbname, MY_MAX_DBNAME_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
      }
#endif



    if(l_printHelpAndExit)
      {
	printf("Usage: %s [-h] [-v] [-t NUM] [-n] [FILENAME]\n",argv[0]);
	printf("Create a test SAF unstructured mesh file.\n\n");
	printf("   -h, --help  print this help message and exit\n");
	printf("   FILENAME  create/overwrite this saf file, otherwise use file %s\n\n",l_dbnameDefault);
	printf("report bugs to saf-help@sourceforge.sandia.gov\n");

	return(-1);
      }
  }

 
  


  setbuf(stdout, NULL);
  setbuf(stderr, NULL);


#ifdef TEST_FILE_PATH
  chdir(TEST_FILE_PATH);
#endif

#ifdef HAVE_PARALLEL
  MPI_Barrier(MPI_COMM_WORLD);
#endif


  /* Initialize the library. */
  saf_init(SAF_DEFAULT_LIBPROPS);

  /* Create the database properties. */
  l_dbprops = saf_createProps_database();

  /* Set the clobber database property so any existing file
   * will be overwritten. */
  saf_setProps_Clobber(l_dbprops);

  /* Open the SAF database. Give it name l_dbname, properties l_dbprops and
   * set g_db to be a handle to this database. */
  g_db = saf_open_database(l_dbname,l_dbprops);


  /* Get the handle to the master file. */
  g_safFile = g_db;



  /****************************************************************************
   ****BEGIN THE UNSTRUCTURED MESH SPECIFIC PART OF MAIN************************
   ***************************************************************************/

  g_umeter = *(saf_find_one_unit(g_db, "meter", NULL));
  g_ukelvin = *(saf_find_one_unit(g_db, "kelvin", NULL));
  g_uampere = *(saf_find_one_unit(g_db, "ampere", NULL));
  g_ukilogram = *(saf_find_one_unit(g_db, "kilogram", NULL));

  saf_declare_category(SAF_ALL,g_db, "nodes", SAF_TOPOLOGY, 0, &g_nodes);
  saf_declare_category(SAF_ALL,g_db, "elems", SAF_TOPOLOGY, 2, &g_elems);




  {
    SAF_FieldTmpl l_temp[3];
    saf_declare_field_tmpl(SAF_ALL, g_db, "coords_component_ftmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1,
			   NULL, &g_coordsComponentFtmpl);
    l_temp[0] = g_coordsComponentFtmpl;
    l_temp[1] = g_coordsComponentFtmpl;
    l_temp[2] = g_coordsComponentFtmpl;
    saf_declare_field_tmpl(SAF_ALL, g_db, "coords_ftmpl", SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 3,
			   l_temp, &g_coordsFtmpl);


    /*Declare a field template for a global (on every block) nodal var.
      The units are g_ukelvin */
    saf_declare_field_tmpl(SAF_ALL, g_db, "global_node_var_tmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN,
			   SAF_QTEMP, 1, NULL, &g_globalNodalVarFtmpl);


    /*Declare a field template for a global (on every block) elem var.
      The units are g_uampere */
    saf_declare_field_tmpl(SAF_ALL, g_db, "global_elem_var_tmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN,
			   SAF_QCURRENT, 1, NULL, &g_globalElemVarFtmpl);
  


    /*Declare a field template for a global (on every block) vector elem var.
      The units are g_umeter */
    saf_declare_field_tmpl(SAF_ALL, g_db, "global_elem_vector_var_tmpl", SAF_ALGTYPE_VECTOR, SAF_CARTESIAN,
			   SAF_QLENGTH, 3, l_temp, &g_globalElemVectorVarFtmpl);
  



    /*Declare a field template for a global (on every block) nodal vector var.
      The units are g_ukilogram */
    saf_declare_field_tmpl(SAF_ALL, g_db, "nodal_vector_var_component_ftmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QMASS, 1,
			   NULL, &g_globalNodalComponentVectorVarFtmpl);
    l_temp[0] = g_globalNodalComponentVectorVarFtmpl;
    l_temp[1] = g_globalNodalComponentVectorVarFtmpl;
    l_temp[2] = g_globalNodalComponentVectorVarFtmpl;
    saf_declare_field_tmpl(SAF_ALL, g_db, "global_nodal_vector_var_tmpl", SAF_ALGTYPE_VECTOR, SAF_CARTESIAN,
			   SAF_QMASS, 3, l_temp, &g_globalNodalVectorVarFtmpl);



  }



#if 1
  create_perimeter_block(8.0, 3.0);
#endif 

  create_single_element_block(SAF_CELLTYPE_HEX,     1.0, 1.0);

#if 1
  create_single_element_block(SAF_CELLTYPE_LINE,    2.0, 1.0); 
  create_single_element_block(SAF_CELLTYPE_TRI,     3.0, 1.0);
  create_single_element_block(SAF_CELLTYPE_QUAD,    4.0, 1.0);
  create_single_element_block(SAF_CELLTYPE_TET,     5.0, 1.0);
  create_single_element_block(SAF_CELLTYPE_PYRAMID, 6.0, 1.0);
  create_single_element_block(SAF_CELLTYPE_PRISM,   7.0, 1.0); 


  create_row_of_single_element_indirect_blocks(1.0, 2.0);



  create_mixed_element_block(8.0, 1.0); 


  create_elem_subset_vars_block(8.0, 2.0); 

  create_node_subset_vars_block(8.0, 3.0); 

  create_single_quadratic_element_block(SAF_CELLTYPE_HEX,     1.0, 3.0);
  create_single_quadratic_element_block(SAF_CELLTYPE_LINE,    2.0, 3.0); 
  create_single_quadratic_element_block(SAF_CELLTYPE_TRI,     3.0, 3.0);
  create_single_quadratic_element_block(SAF_CELLTYPE_QUAD,    4.0, 3.0);
  create_single_quadratic_element_block(SAF_CELLTYPE_TET,     5.0, 3.0);
  create_single_quadratic_element_block(SAF_CELLTYPE_PYRAMID, 6.0, 3.0);
  create_single_quadratic_element_block(SAF_CELLTYPE_PRISM,   7.0, 3.0); 

#endif  

  /****************************************************************************
   ****END OF THE UNSTRUCTURED MESH SPECIFIC PART OF MAIN************************
   ***************************************************************************/






  /* Close the SAF database. */
  saf_close_database(g_db);


  /* Finalize access to the library. */
  saf_final();



#ifdef HAVE_PARALLEL
  MPI_Finalize();
#endif



  printf("proc %d of %d exiting with no errors\n",g_rank,g_nprocs);

  return(0);

}
