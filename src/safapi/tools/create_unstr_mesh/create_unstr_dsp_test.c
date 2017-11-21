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
 * Chapter:     Create an Example Unstructured Mesh File for Testing DSP Functions (create_unstr_dsp_test)
 * Description:
 *
 *
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

static SAF_Db g_db;                /* Handle to the SAF database. */
static SAF_File g_safFile;        /* Handle to the saf file. */

static SAF_Cat g_nodes,g_elems;
static SAF_Unit g_umeter,g_ukelvin,g_uampere,g_ukilogram;

SAF_FieldTmpl g_coordsComponentFtmpl,g_coordsFtmpl;
SAF_FieldTmpl g_globalNodalVarFtmpl,g_globalElemVarFtmpl;
SAF_FieldTmpl g_globalNodalComponentVectorVarFtmpl,g_globalNodalVectorVarFtmpl;
SAF_FieldTmpl g_globalElemVectorVarFtmpl;





/*
** g_fields,g_fieldTemplates,g_timestepsPerAddedField contain the template,field,timestep
** triplets. After all fields are written, then these three arrays are used
** to put each field in the appropriate timestep-suite. 
*/
  int g_maxNumFields=0;
  int g_currentNumFields=0;
  int g_numTimes = -1;
  SAF_Field *g_fields=0;
  SAF_FieldTmpl *g_fieldTemplates=0;
  int *g_timestepsPerAddedField=0;
  int g_numSuitesWritten=0;

SAF_Set g_suiteset;/*a single set above all 'topsets'*/
SAF_Suite g_suite;/*a single suite*/


  /*need to keep track of blocks and unstr nodessets added for new states&suites*/
  int g_numBlocksAdded=0;
  int g_blocksAndUnstrNodesetsAddedListSize=0;
  SAF_Set *g_blocksAndUnstrNodesetsAdded=0;




void write_timestep_data( MY_PRECISION *a_timeValues );


void add_field_to_timestep_data( SAF_Field *a_field, SAF_FieldTmpl *a_template, int a_timestep );

void sort_timestep_data( int a_num, SAF_Field *a_fields, 
			 SAF_FieldTmpl *a_fieldTemplates, int *a_timestepsPerAddedField );

SAF_Set create_TOP_SET(void);

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Assign a Timestep to a Field
 *
 * Description: 
 *
 * Adds a field and template to the global list that will later be used to
 * create the states and suites.
 *
 *--------------------------------------------------------------------------------------------------- */
void add_field_to_timestep_data( SAF_Field *a_field, SAF_FieldTmpl *a_template, int a_timestep )
{
 if(!g_maxNumFields)
   { /*nothing allocated yet*/
     g_maxNumFields = 5000;
     g_fields = (SAF_Field *)malloc(g_maxNumFields*sizeof(SAF_Field));
     if(!g_fields) { printf("malloc failed for g_fields\n");  exit(-1); }
     g_fieldTemplates = (SAF_FieldTmpl *)malloc(g_maxNumFields*sizeof(SAF_FieldTmpl));
     if(!g_fieldTemplates) { printf("malloc failed for g_fieldTemplates\n");  exit(-1); }
     g_timestepsPerAddedField = (int *)malloc(g_maxNumFields*sizeof(int));
     if(!g_timestepsPerAddedField) { printf("malloc failed for g_timestepsPerAddedField\n");  exit(-1); }
   }
 else if( g_currentNumFields >= g_maxNumFields )
   { /*need to resize the arrays*/
     /*XXX do this eventually, for now just exit*/
     printf("\n\nerror...NEED TO RESIZE g_fields....havent written it yet....easy fix\n");
     exit(-1);
   }

 g_fields[g_currentNumFields] = *a_field;
 g_fieldTemplates[g_currentNumFields] = *a_template;
 g_timestepsPerAddedField[g_currentNumFields] = a_timestep;
 g_currentNumFields++;

 if( a_timestep >= g_numTimes ) g_numTimes = a_timestep+1;

}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Sort the Global Timestep Data
 *
 * Description: 
 *
 * Sort the global template, field, timestep triplets by timestep.
 *
 *--------------------------------------------------------------------------------------------------- */
void sort_timestep_data( int a_num, SAF_Field *a_fields, 
			 SAF_FieldTmpl *a_fieldTemplates, int *a_timestepsPerAddedField )
{
  int l_didSomething,i;
  if(!g_rank) printf("sort_timestep_data a_num=%d\n",a_num);
  do {
    l_didSomething=0;
    for(i=0;i<a_num-1;i++)
      {
	if( a_timestepsPerAddedField[i] > a_timestepsPerAddedField[i+1] )
	  {
	    int l_tmpTime;
	    SAF_Field l_tmpField;
	    SAF_FieldTmpl l_tmpTmpl;
	    l_didSomething=1;
	    l_tmpField = a_fields[i];
	    l_tmpTmpl = a_fieldTemplates[i];
	    l_tmpTime = a_timestepsPerAddedField[i];
	    a_fields[i] = a_fields[i+1];
	    a_fieldTemplates[i] = a_fieldTemplates[i+1];
	    a_timestepsPerAddedField[i] = a_timestepsPerAddedField[i+1];
	    a_fields[i+1] = l_tmpField;
	    a_fieldTemplates[i+1] = l_tmpTmpl;
	    a_timestepsPerAddedField[i+1] = l_tmpTime;
	  }
      }
  } while( l_didSomething );



#if 0
  if(g_numTimes)
    {
      int *l_numFieldsPerTime=0;

      l_numFieldsPerTime = (int *)malloc( g_numTimes*sizeof(int) );
      for(i=0;i<g_numTimes;i++) l_numFieldsPerTime[i]=0;
      for(i=0;i<a_num;i++) l_numFieldsPerTime[ a_timestepsPerAddedField[i] ]++;

      for(i=0;i<g_numTimes;i++) printf("l_numFieldsPerTime[%d]=%d\n",i,l_numFieldsPerTime[i]);
    }

  for(i=0;i<a_num;i++)
    {/*verbose*/
      char *l_setname=0,*l_fieldname=0;
      SAF_Set l_set;
      saf_describe_field(SAF_ALL, a_fields[i], NULL, &l_fieldname,&l_set, NULL, NULL,
			 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      saf_describe_set(SAF_ALL, l_set, &l_setname, NULL, NULL, NULL, NULL, NULL, NULL );
      printf("time %d: \t %s \t\t\t %s\n",g_timestepsPerAddedField[i],l_fieldname,l_setname);
      if(l_fieldname) free(l_fieldname);
      if(l_setname) free(l_setname);
    }

  exit(-1);
#endif

}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write Timestep Data to Database
 *
 * Description: 
 * This function creates the states and suite for the database from the 
 * global list of sets, fields, templates and timesteps. After creating the suite,
 * the global list is emptied.
 *
 * Although it is desirable to have only one suite in a database, it is allowed to 
 * call this function numerous times (creating a new suite each time with whatever
 * blocks and variables are in the global list). 
 *
 *--------------------------------------------------------------------------------------------------- */
void write_timestep_data( MY_PRECISION *a_timeValues )
{
  int j;
  int l_whichStateGroup;
  int l_startIndex=0;
  char l_str[128];
  SAF_Field *l_allProcFields=0;
  SAF_FieldTmpl *l_allProcFieldTemplates=0;
  int l_allProcNumFields=0;
  int *l_allProcTimestepPerField=0;
  int l_lastNumStates=0;

  if(!g_rank) printf("\n--------------STARTING STATES&SUITES--------------\n\n");

  g_suiteset = create_TOP_SET();

 
#ifdef HAVE_PARALLEL

  if(!g_rank) printf("\nStarting gathering handles: num fields goes from %d to %d\n\n",
			g_currentNumFields,g_currentNumFields*g_nprocs);

  l_allProcNumFields=g_currentNumFields*g_nprocs;  
  l_allProcFields=(SAF_Field *)malloc(l_allProcNumFields*sizeof(SAF_Field));
  l_allProcFieldTemplates=(SAF_FieldTmpl *)malloc(l_allProcNumFields*sizeof(SAF_FieldTmpl));
  l_allProcTimestepPerField=(int *)malloc(l_allProcNumFields*sizeof(int));

  for(j=0;j<g_currentNumFields;j++)
    {
      int i,l_numReturned=0;
      SAF_Field *l_fptr = 0;
      SAF_FieldTmpl *l_tptr = 0;

      saf_allgather_handles(g_db, (void *)(&g_fields[j]), &l_numReturned, 
			    (void **)&l_fptr );

      /*dont need to exchange templates, because they are created with SAF_ALL*/
#define DONT_EXCHANGE_TEMPLATES

#ifndef DONT_EXCHANGE_TEMPLATES
      saf_allgather_handles(g_db, (void *)(&g_fieldTemplates[j]), &l_numReturned, 
			    (void **)&l_tptr );
#endif

      
      for(i=0;i<g_nprocs;i++) 
	{
	  /*note: assuming here that each proc is writing at the same timestep*/
	  l_allProcTimestepPerField[j*g_nprocs+i] = g_timestepsPerAddedField[j];
	  l_allProcFields[j*g_nprocs+i] = l_fptr[i];
#ifndef DONT_EXCHANGE_TEMPLATES
	  l_allProcFieldTemplates[j*g_nprocs+i] = l_tptr[i];
#else
	  l_allProcFieldTemplates[j*g_nprocs+i] = g_fieldTemplates[j];
#endif
	}
    }

  if(!g_rank) printf("\nFinished gathering handles.\n\n");

#else
  l_allProcNumFields=g_currentNumFields;
  l_allProcFields=g_fields;
  l_allProcFieldTemplates=g_fieldTemplates;
  l_allProcTimestepPerField = g_timestepsPerAddedField;
#endif


  sort_timestep_data( l_allProcNumFields, l_allProcFields, l_allProcFieldTemplates,
		      l_allProcTimestepPerField );

  if(!g_rank ) printf("write_timestep_data l_allProcNumFields=%d  g_numTimes=%d\n",
			 l_allProcNumFields,g_numTimes);


  /*create the one and only suite*/
  saf_declare_suite(SAF_ALL, g_db, "SUITE", g_suiteset, NULL, &g_suite);


  /*In the worst case (a different template set for each timestep) there will be
    as many state groups as timesteps*/
  for( l_whichStateGroup=0; l_whichStateGroup<g_numTimes; l_whichStateGroup++ )
    {
      SAF_StateGrp l_stateGroup;
      SAF_StateTmpl l_stateTemplate;
      SAF_FieldTmpl *l_fieldTemplates=0;
      int l_numFieldsPerTime=1;
      int l_numTimesInThisGroup=1;
      int l_done=0;
      SAF_Unit l_units;
       
      l_units = saf_find_one_unit(g_db, "second");
	

      /*count the number of fields that will be in this state group*/
      for(j=l_startIndex+1; j<l_allProcNumFields; j++ )
	{
	  if( l_allProcTimestepPerField[j]==l_allProcTimestepPerField[l_startIndex] )
	    {
	      l_numFieldsPerTime++;
	    }
	  else break;
	}

      /*count the number of different times in this group. (i.e. how many times, for
	this block, the series of field templates is the same for a certain timestep
	as for this first timestep)*/
      l_done=0;
      while(!l_done)
	{
	  for(j=l_startIndex; j<l_startIndex+l_numFieldsPerTime; j++ )
	    {
	      int l_index = j+l_numFieldsPerTime*l_numTimesInThisGroup;
	      if( l_index >= l_allProcNumFields ||
		  !SAF_EQUIV(l_allProcFieldTemplates[j], l_allProcFieldTemplates[l_index]) )
		{
		  l_done=1;
		  break;
		}
	    }
	  if( !l_done )
	    {
	      l_numTimesInThisGroup++;
	    }
	}

      if(!g_rank) printf("     for state group %d: found %d fields per state, and %d states\n",
			    l_whichStateGroup,l_numFieldsPerTime,l_numTimesInThisGroup );


      if(l_lastNumStates > l_numTimesInThisGroup)
	{
	  if(!g_rank) printf("\n\nwarning: 12-10-02 THERE WILL BE A READING BUG BECAUSE # STATES HAS DECREASED SINCE LAST GROUP\n\n\n");
	}
      l_lastNumStates = l_numTimesInThisGroup;

		
      /*create the list of field templates for this state group*/
      l_fieldTemplates = (SAF_FieldTmpl *)malloc(l_numFieldsPerTime*sizeof(SAF_FieldTmpl));
      for(j=0; j<l_numFieldsPerTime; j++ )
	{
	  l_fieldTemplates[j] = l_allProcFieldTemplates[l_startIndex+j];
	}

      snprintf(l_str,127,"state_tmpl_grp%d",l_whichStateGroup);
      saf_declare_state_tmpl(SAF_ALL, l_str, g_db, l_numFieldsPerTime, l_fieldTemplates, &l_stateTemplate);
	  

      snprintf(l_str,127,"state_grp%d",l_whichStateGroup);
      saf_declare_state_group(SAF_ALL, l_str, g_suite, g_db, g_suiteset, l_stateTemplate, SAF_QTIME, 
			      l_units, MY_SAF_PRECISION, &l_stateGroup);


      /* write the states */
      for(j=0; j<l_numTimesInThisGroup; j++ )
	{
	  int l_startIndexForThisGroup = l_startIndex + j*l_numFieldsPerTime;
	  int l_timestepForThisGroup = l_allProcTimestepPerField[l_startIndexForThisGroup];
	  MY_PRECISION l_timeValueForThisGroup = a_timeValues[l_timestepForThisGroup];

	  if(!g_rank) printf("           grp %d, writing state %d of %d, timestep %d, time %f\n",
				l_whichStateGroup, j,l_numTimesInThisGroup,
				l_timestepForThisGroup, l_timeValueForThisGroup );
 
	  saf_write_state(SAF_ALL,l_stateGroup,j,g_suiteset,MY_SAF_PRECISION,
			  &l_timeValueForThisGroup, l_allProcFields+l_startIndexForThisGroup );

	}

      if(l_fieldTemplates) free(l_fieldTemplates);
      l_startIndex += l_numFieldsPerTime*l_numTimesInThisGroup;

      if( l_startIndex>=l_allProcNumFields )
	{
	  if(!g_rank) printf("     ****finished making state groups for this suite\n\n");
	  break;
	}
    }
 


#ifdef HAVE_PARALLEL
  free( l_allProcFields );
  free( l_allProcFieldTemplates );
  free( l_allProcTimestepPerField );
#endif

}







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


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Unstructured Mesh File (create_unstr_mesh)
 * Purpose:     
 *
 * Description: 
 *
 *
 *--------------------------------------------------------------------------------------------------- */
void create_test_element_block( void  );
void create_test_element_block(  )
{
  SAF_Set l_set;
  SAF_Rel l_rel;
  SAF_Field l_field, l_componentField[3];
  char l_setName[]="SINGLE_HEX_SET";
  int l_numElems=1;
  int l_numPoints=8;
  int l_type=SAF_CELLTYPE_HEX;
  double l_x=1.0;
  double l_y=1.0;
  int i,j;
  int l_numTimes=200;

  saf_declare_set(SAF_ALL, g_db, l_setName+strlen("SAF_CELLTYPE_"), /*get rid of the leading SAF_CELLTYPE_*/
		  2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &l_set);
  saf_declare_collection(SAF_ALL, l_set, g_nodes, SAF_CELLTYPE_POINT, l_numPoints, SAF_1DC(l_numPoints), SAF_DECOMP_FALSE); 

  saf_declare_collection(SAF_ALL, l_set, g_elems, l_type, l_numElems, SAF_1DC(l_numElems), SAF_DECOMP_TRUE);

  saf_declare_topo_relation(SAF_ALL, l_set, g_elems, l_set, g_nodes, SAF_SELF(g_db), l_set,
			    SAF_UNSTRUCTURED, NULL, NULL, NULL, NULL, &l_rel);

  { 
    int bbuf[24] = {0,1,2,3,4,5,6,7,  8,9,10,11,12,13,14,15, 16,17,18,19,20,21,22,23 };/*larger than the largest element?*/

    saf_write_topo_relation(SAF_ALL, l_rel, DSL_INT, &l_numPoints, DSL_INT, bbuf, g_safFile);
  }

  saf_declare_field(SAF_ALL, g_coordsComponentFtmpl, "X", l_set, g_umeter, SAF_SELF(g_db), SAF_NODAL(g_nodes, g_elems),
		    DSL_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
  saf_declare_field(SAF_ALL, g_coordsComponentFtmpl, "Y", l_set, g_umeter, SAF_SELF(g_db), SAF_NODAL(g_nodes, g_elems),
		    DSL_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
  saf_declare_field(SAF_ALL, g_coordsComponentFtmpl, "Z", l_set, g_umeter, SAF_SELF(g_db), SAF_NODAL(g_nodes, g_elems),
		    DSL_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));

  saf_declare_field(SAF_ALL, g_coordsFtmpl, "coords", l_set, g_umeter, SAF_SELF(g_db), SAF_NODAL(g_nodes, g_elems),
		    SAF_FLOAT, l_componentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &l_field); 

  {
    float buf[3*8]= {
      -.25, -.25, -.25,   .25, -.25, -.25,  .25, .25, -.25,   -.25, .25, -.25,
      -.25, -.25, .25,    .25, -.25, .25,   .25, .25, .25,    -.25, .25, .25
    };
    void *pbuf = &buf[0];
    for(i=0;i<3*8;i+=3) buf[i] += l_x;
    for(i=1;i<3*8;i+=3) buf[i] += l_y;

    saf_write_field(SAF_ALL, l_field, SAF_WHOLE_FIELD, 1, NULL, &pbuf, g_safFile); 
  }

  saf_declare_coords(SAF_ALL, l_field);
  saf_declare_coords(SAF_ALL, l_componentField[0] );
  saf_declare_coords(SAF_ALL, l_componentField[1] );
  saf_declare_coords(SAF_ALL, l_componentField[2] );
   
  saf_declare_default_coords(SAF_ALL,l_set,l_field);


#if 0 
  /*Write the global scalar nodal variable on the top set. This variable
    will be mapped to every part. It must use g_ukelvin units, to match all the others*/
  {
    SAF_Field l_field;

    double buf[8]= { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0 };
    void *pbuf = &buf[0];


    saf_declare_field(SAF_ALL, g_globalNodalVarFtmpl, "global_nodal_var", l_set, g_ukelvin, SAF_SELF(g_db), 
		      SAF_NODAL(g_nodes, g_elems),
		      DSL_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
    saf_write_field(SAF_ALL, l_field, SAF_WHOLE_FIELD, 1, NULL, &pbuf, g_safFile); 


    for(i=0;i<l_numTimes;i++)
      {
	add_field_to_timestep_data( &l_field, &g_globalNodalVarFtmpl, i );
      }
  }
#endif


#if 0
  {
    SAF_FieldTmpl l_tmpl;
    saf_declare_field_tmpl(SAF_ALL, "template", g_db, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN,
			   SAF_QCURRENT, 1, NULL, &l_tmpl);
    for(i=0;i<l_numTimes;i++)
      {
	SAF_Field l_field;
	double buf[1];
	void *pbuf = &buf[0];
	buf[0]=(double)i;
	saf_declare_field(SAF_ALL, l_tmpl, "ramp_elem_var", l_set, g_uampere, SAF_SELF(g_db), 
			  SAF_ZONAL(g_elems),
			  SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
	saf_write_field(SAF_ALL, l_field, SAF_WHOLE_FIELD, 1, NULL, &pbuf, g_safFile); 
	add_field_to_timestep_data( &l_field, &l_tmpl, i );
      }
  }

 
  {
    SAF_FieldTmpl l_tmpl;
    saf_declare_field_tmpl(SAF_ALL, "template", g_db, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN,
			   SAF_QCURRENT, 1, NULL, &l_tmpl);
    for(i=0;i<l_numTimes;i++)
      {
	SAF_Field l_field;
	double buf[1];
	void *pbuf = &buf[0];
	buf[0]=(double)(-i*3);
	saf_declare_field(SAF_ALL, l_tmpl, "neg_ramp_elem_var", l_set, g_uampere, SAF_SELF(g_db), 
			  SAF_ZONAL(g_elems),
			  SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
	saf_write_field(SAF_ALL, l_field, SAF_WHOLE_FIELD, 1, NULL, &pbuf, g_safFile); 
	add_field_to_timestep_data( &l_field, &l_tmpl, i );
      }
  }

 
  {
    SAF_FieldTmpl l_tmpl;
    saf_declare_field_tmpl(SAF_ALL, "template", g_db, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN,
			   SAF_QCURRENT, 1, NULL, &l_tmpl);
    for(i=0;i<l_numTimes;i++)
      {
	SAF_Field l_field;
	double buf[1];
	void *pbuf = &buf[0];
	buf[0]=1.0;
	saf_declare_field(SAF_ALL, l_tmpl, "const_elem_var", l_set, g_uampere, SAF_SELF(g_db), 
			  SAF_ZONAL(g_elems),
			  SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
	saf_write_field(SAF_ALL, l_field, SAF_WHOLE_FIELD, 1, NULL, &pbuf, g_safFile); 
	add_field_to_timestep_data( &l_field, &l_tmpl, i );
      }
  }
 
  {
    SAF_FieldTmpl l_tmpl;
    saf_declare_field_tmpl(SAF_ALL, "template", g_db, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN,
			   SAF_QCURRENT, 1, NULL, &l_tmpl);
    for(i=0;i<l_numTimes;i++)
      {
	SAF_Field l_field;
	double buf[1];
	void *pbuf = &buf[0];
	buf[0]=-1.0;
	saf_declare_field(SAF_ALL, l_tmpl, "neg_const_elem_var", l_set, g_uampere, SAF_SELF(g_db), 
			  SAF_ZONAL(g_elems),
			  SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
	saf_write_field(SAF_ALL, l_field, SAF_WHOLE_FIELD, 1, NULL, &pbuf, g_safFile); 
	add_field_to_timestep_data( &l_field, &l_tmpl, i );
      }
  }

#endif
 


#if 0
  {
    SAF_FieldTmpl l_tmpl;
    saf_declare_field_tmpl(SAF_ALL, "template", g_db, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN,
			   SAF_QCURRENT, 1, NULL, &l_tmpl);
    for(i=0;i<l_numTimes;i++)
      {
	SAF_Field l_field;
	double buf[1];
	void *pbuf = &buf[0];
	double l_period = l_numTimes;

	buf[0]=0.0;
	for(j=0;j<1;j++)
	  {
	    buf[0] += sin( ((double)i)*2*3.14159265 / l_period );
	    l_period = l_period/2;
	  }

	saf_declare_field(SAF_ALL, l_tmpl, "low_white_noise_elem_var", l_set, g_uampere, SAF_SELF(g_db), 
			  SAF_ZONAL(g_elems),
			  SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
	saf_write_field(SAF_ALL, l_field, SAF_WHOLE_FIELD, 1, NULL, &pbuf, g_safFile); 
	add_field_to_timestep_data( &l_field, &l_tmpl, i );
      }
  }
#endif

  {
    SAF_FieldTmpl l_tmpl;
    saf_declare_field_tmpl(SAF_ALL, "template", g_db, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN,
			   SAF_QCURRENT, 1, NULL, &l_tmpl);
    for(i=0;i<l_numTimes;i++)
      {
	SAF_Field l_field;
	double buf[1];
	void *pbuf = &buf[0];
	double l_period = 4;

	buf[0]=0.0;
	for(j=0;j<1;j++)
	  {
	    buf[0] += sin( ((double)i)*2*3.14159265 / l_period );
	    l_period = l_period*2;
	  }

	saf_declare_field(SAF_ALL, l_tmpl, "hi_white_noise_elem_var", l_set, g_uampere, SAF_SELF(g_db), 
			  SAF_ZONAL(g_elems),
			  SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &l_field);
	saf_write_field(SAF_ALL, l_field, SAF_WHOLE_FIELD, 1, NULL, &pbuf, g_safFile); 
	add_field_to_timestep_data( &l_field, &l_tmpl, i );
      }
  }



#if 0

  /*Write the global scalar elem vector variable on the top set.*/
  {
    SAF_Field l_field;
    SAF_FieldTmpl l_temp[3];
    SAF_Field l_componentField[3];

    double buf[3];
    void *pbuf = &buf[0];
    buf[0]=  l_x;
    buf[1]=  8.0-l_x;
    buf[2]=  l_x;

    l_temp[0] = g_coordsComponentFtmpl;
    l_temp[1] = g_coordsComponentFtmpl;
    l_temp[2] = g_coordsComponentFtmpl;

    saf_declare_field(SAF_ALL, g_coordsComponentFtmpl, "Xglobal_elem_vector_var", l_set, g_umeter, SAF_SELF(g_db), 
		      SAF_ZONAL(g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
    saf_declare_field(SAF_ALL, g_coordsComponentFtmpl, "Yglobal_elem_vector_var", l_set, g_umeter, SAF_SELF(g_db), 
		      SAF_ZONAL(g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
    saf_declare_field(SAF_ALL, g_coordsComponentFtmpl, "Zglobal_elem_vector_var", l_set, g_umeter, SAF_SELF(g_db), 
		      SAF_ZONAL(g_elems),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));

    saf_declare_field(SAF_ALL, g_globalElemVectorVarFtmpl, "global_elem_vector_var", l_set, g_umeter, 
		      SAF_SELF(g_db), SAF_ZONAL(g_elems),
		      SAF_DOUBLE, l_componentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &l_field);

    saf_write_field(SAF_ALL, l_field, SAF_WHOLE_FIELD, 1, NULL, &pbuf, g_safFile); 

    add_field_to_timestep_data( &l_field, &g_globalElemVectorVarFtmpl, 0 );
  }





  /*Write the global scalar nodal vector variable on the top set.*/
  {
    SAF_Field l_field;
    SAF_FieldTmpl l_temp[3];
    SAF_Field l_componentField[3];

    double buf[3*8]= { 0,7,0,  1,6,1,  2,5,2,  3,4,3,  4,3,4,  5,2,5,  6,1,6,  7,0,7 };
    void *pbuf = &buf[0];


    l_temp[0] = g_globalNodalComponentVectorVarFtmpl;
    l_temp[1] = g_globalNodalComponentVectorVarFtmpl;
    l_temp[2] = g_globalNodalComponentVectorVarFtmpl;

    saf_declare_field(SAF_ALL, g_globalNodalComponentVectorVarFtmpl, "Xglobal_nodal_vector_var", l_set, g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(g_nodes),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[0]));
    saf_declare_field(SAF_ALL, g_globalNodalComponentVectorVarFtmpl, "Yglobal_nodal_vector_var", l_set, g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(g_nodes),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[1]));
    saf_declare_field(SAF_ALL, g_globalNodalComponentVectorVarFtmpl, "Zglobal_nodal_vector_var", l_set, g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(g_nodes),
		      SAF_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(l_componentField[2]));

    saf_declare_field(SAF_ALL, g_globalNodalVectorVarFtmpl, "global_nodal_vector_var", l_set, g_ukilogram, 
		      SAF_SELF(g_db), SAF_ZONAL(g_nodes),
		      SAF_DOUBLE, l_componentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &l_field);

    saf_write_field(SAF_ALL, l_field, SAF_WHOLE_FIELD, 1, NULL, &pbuf, g_safFile); 

    add_field_to_timestep_data( &l_field, &g_globalNodalVectorVarFtmpl, 0 );
  }


#endif



 
  {
    MY_PRECISION *l_timevals=(MY_PRECISION *)malloc(l_numTimes*sizeof(MY_PRECISION));

    for( i=0; i<l_numTimes; i++ )
      {
	l_timevals[i]=i*.1;
      }
    write_timestep_data(l_timevals);
  }

}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Create a Top Set for the Database
 *
 * Description: 
 * Creates a single top set (using parallel mode SAF_ALL: no matter how many processors 
 * are used, there is still only one top set), of which all of the structured blocks are subsets. This
 * top set is a dummy set that is used to unite all of the blocks under one suite.
 *
 * Return:      Returns the created top set.
 *
 *--------------------------------------------------------------------------------------------------- */
SAF_Set create_TOP_SET()
{
  SAF_Set l_topset=SAF_NOT_SET_SET;
  SAF_Cat l_cat=SAF_NOT_SET_CAT;
  int i, l_numBlocksAllProcs=0;

  /*create the suiteset: a top-topset above all others, a 1-d trivial set*/
  saf_declare_set(SAF_ALL, g_db, "TOP_SET", 1, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &l_topset);


  /*figure out how many blocks in the entire simulation*/
  l_numBlocksAllProcs = g_numBlocksAdded*g_nprocs;

  saf_declare_category(g_db, "blocks",  SAF_BLOCK, 0, &l_cat);
  saf_declare_collection(SAF_ALL, l_topset, l_cat, SAF_CELLTYPE_SET, l_numBlocksAllProcs,
			 _saf_indexspec(1, l_numBlocksAllProcs, 0,0), SAF_DECOMP_TRUE);

  /*relate each 'topset' to the suiteset as one block of many*/
  for(i=0;i<g_numBlocksAdded;i++)
    {
      int l_whichEntry = i+g_rank*g_numBlocksAdded;
      SAF_Rel l_rel;
      SAF_Set l_subset = g_blocksAndUnstrNodesetsAdded[i];

      saf_declare_collection(SAF_EACH, l_subset, l_cat, SAF_CELLTYPE_SET, 1,
			     _saf_indexspec(1,1,0,0), SAF_DECOMP_TRUE);
      saf_declare_subset_relation(SAF_EACH, l_topset, l_subset, 
				  l_cat, l_cat, SAF_BOUNDARY_FALSE,SAF_BOUNDARY_FALSE,
				  SAF_TUPLES, SAF_INT, NULL, NULL, NULL, &l_rel);
      saf_write_subset_relation(SAF_EACH, l_rel, SAF_INT, &l_whichEntry, NULL, NULL, g_safFile);
    }



  /*create a coords field that contains handles pointing to the coord fields
    of each block*/
  {
    SAF_FieldTmpl l_coordsTmpl;
    SAF_Field l_coordsField;
    SAF_Field *l_indivCoordFields = (SAF_Field *)malloc( g_numBlocksAdded * sizeof(SAF_Field));
    SAF_Field *l_allFields = (SAF_Field *)malloc( l_numBlocksAllProcs * sizeof(SAF_Field));


    saf_declare_field_tmpl(SAF_ALL, "coord_fields_tmpl", g_db, SAF_ALGTYPE_FIELD,
			   SAF_EMPTY_HANDLE, SAF_NOT_APPLICABLE_QUANTITY,
			   SAF_NOT_APPLICABLE_INT,/*This is the # of components.Set to 
						   SAF_NOT_APPLICABLE_INT because it is INhomogeneous?*/ 
			   NULL, &l_coordsTmpl );
    
    saf_declare_field(SAF_ALL, l_coordsTmpl, "coord_fields",l_topset,SAF_NOT_APPLICABLE_UNIT,
		      l_cat,SAF_ZONAL(l_cat),
		      DSL_HANDLE,NULL,SAF_INTERLEAVE_VECTOR,SAF_IDENTITY,NULL,&l_coordsField);

    saf_declare_coords(SAF_ALL,l_coordsField);
    saf_declare_default_coords(SAF_ALL,l_topset,l_coordsField);    

    for(i=0;i<g_numBlocksAdded;i++)
      {
	int l_numReturned=0;
	SAF_Field *l_returnedHandles=0;
	SAF_Set l_subset = g_blocksAndUnstrNodesetsAdded[i];
	saf_find_default_coords(SAF_EACH, l_subset, &l_indivCoordFields[i]);
	saf_allgather_handles(g_db, (void *)(&l_indivCoordFields[i]), &l_numReturned, 
			      (void **)(&l_returnedHandles));

	/*an unnecessary check*/
	if( g_nprocs != l_numReturned )
	  {
	    printf("error saf_allgather_handles g_nprocs=%d l_numReturned=%d\n",g_nprocs,l_numReturned);
	    exit(-1);
	  }

	memcpy( &l_allFields[i*g_nprocs], l_returnedHandles, g_nprocs*sizeof(SAF_Field) );
	free(l_returnedHandles);
      }

    /*just a printout*/
    /*for(i=0;i<l_numBlocksAllProcs;i++)
      { 
      int j;
      char *name=0;
      saf_describe_field(SAF_ALL, l_allFields[i], NULL, &name, NULL, 
      NULL, NULL, NULL, NULL, NULL, NULL, NULL,
      NULL, NULL, NULL, NULL, NULL, NULL);
      for(j=0;j<g_rank;j++) printf("\t\t\t\t");
      printf("\t i=%d %s theRow=%d\n",i,name,l_allFields[i].theRow);
      if(name) free(name);
      }*/


    saf_write_field(SAF_ALL, l_coordsField, SAF_WHOLE_FIELD, 1, SAF_HANDLE, (void **)(&l_allFields), g_safFile);


    free(l_indivCoordFields);
    free(l_allFields);
  }




  return( l_topset );

} /* create_TOP_SET */






/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Unstructured DSP Test File (create_unstr_dsp_test)
 * Purpose:     main Function for create_unstr_dsp_test
 *
 * Description: 
 * This function parses the command line options and then creates a 
 * test database for use with DSP tests.
 *
 * Usage   
 *
 *   ./create_unstr_dsp_tst [-h] [FILENAME]
 *
 * Command Line Options
 *
 *   -h, --help  print this help message and exit
 *
 *   FILENAME  create/overwrite this saf file, otherwise use file test_unstr_dsp.saf
 *
 *--------------------------------------------------------------------------------------------------- */
int main(int argc, char **argv)
{
#define MY_MAX_DBNAME_SIZE 1024

  char l_dbname[MY_MAX_DBNAME_SIZE];
  char l_dbnameDefault[]="test_unstr_dsp.saf";        /* Name of the SAF database file to be created. */
  SAF_DbProps l_dbprops;      /* Handle to the SAF database properties. */


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
	printf("Create a SAF unstructured mesh file for testing dsp functions.\n\n");
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
  g_safFile = SAF_MASTER(g_db);



  /****************************************************************************
   ****BEGIN THE UNSTRUCTURED MESH SPECIFIC PART OF MAIN************************
   ***************************************************************************/

  g_umeter = saf_find_one_unit(g_db, "meter");
  g_ukelvin = saf_find_one_unit(g_db, "kelvin");
  g_uampere = saf_find_one_unit(g_db, "ampere");
  g_ukilogram = saf_find_one_unit(g_db, "kilogram");

  saf_declare_category(g_db, "nodes", SAF_TOPOLOGY, 0, &g_nodes);
  saf_declare_category(g_db, "elems", SAF_TOPOLOGY, 2, &g_elems);




  {
    SAF_FieldTmpl l_temp[3];
    saf_declare_field_tmpl(SAF_ALL, "coords_component_ftmpl", g_db, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1,
			   NULL, &g_coordsComponentFtmpl);
    l_temp[0] = g_coordsComponentFtmpl;
    l_temp[1] = g_coordsComponentFtmpl;
    l_temp[2] = g_coordsComponentFtmpl;
    saf_declare_field_tmpl(SAF_ALL, "coords_ftmpl", g_db, SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 3,
			   l_temp, &g_coordsFtmpl);


    /*Declare a field template for a global (on every block) nodal var.
      The units are g_ukelvin */
    saf_declare_field_tmpl(SAF_ALL, "global_node_var_tmpl", g_db, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN,
			   SAF_QTEMP, 1, NULL, &g_globalNodalVarFtmpl);


    /*Declare a field template for a global (on every block) elem var.
      The units are g_uampere */
    saf_declare_field_tmpl(SAF_ALL, "global_elem_var_tmpl", g_db, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN,
			   SAF_QCURRENT, 1, NULL, &g_globalElemVarFtmpl);
  


    /*Declare a field template for a global (on every block) vector elem var.
      The units are g_umeter */
    saf_declare_field_tmpl(SAF_ALL, "global_elem_vector_var_tmpl", g_db, SAF_ALGTYPE_VECTOR, SAF_CARTESIAN,
			   SAF_QLENGTH, 3, l_temp, &g_globalElemVectorVarFtmpl);
  



    /*Declare a field template for a global (on every block) nodal vector var.
      The units are g_ukilogram */
    saf_declare_field_tmpl(SAF_ALL, "nodal_vector_var_component_ftmpl", g_db, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1,
			   NULL, &g_globalNodalComponentVectorVarFtmpl);
    l_temp[0] = g_globalNodalComponentVectorVarFtmpl;
    l_temp[1] = g_globalNodalComponentVectorVarFtmpl;
    l_temp[2] = g_globalNodalComponentVectorVarFtmpl;
    saf_declare_field_tmpl(SAF_ALL, "global_nodal_vector_var_tmpl", g_db, SAF_ALGTYPE_VECTOR, SAF_CARTESIAN,
			   SAF_QMASS, 3, l_temp, &g_globalNodalVectorVarFtmpl);



  }



  create_test_element_block();



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
