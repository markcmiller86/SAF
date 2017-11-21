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
 * Chapter:     Variable Names
 * Description:
 *
 * This collection of functions is used to determine the variable name that will be
 * used to describe a group of fields, e.g. a group containing fields "pressure01" and 
 * "pressure02" would likely be called the "pressure" variable.
 *
 * C vs C++:    This file (variable_names.c) can be compiled in C or C++. In C, the global variables
 * are declared in the source file. In C++ the variables are members of the class VariableNames. In
 * C these functions can only be used for one database (per process) at a time (note that Ensight 
 * seems to spawn new processes for each database, so it is not an issue). Note that in C++, the
 * member variables are prefixed by "g_" instead of "m_", so that the variable names will be the
 * same in C and C++.
 *
 *--------------------------------------------------------------------------------------------------- */

#include "str_mesh_reader.h"
#include "variable_names.h"




#ifdef __cplusplus 
#  define VARIABLE_NAMES_CLASS_NAME_IF_CPP VariableNames::
#else
#  define VARIABLE_NAMES_CLASS_NAME_IF_CPP
#endif


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Variable Names
 * Purpose:     Global Variables
 * Description:
 *
 * These are global variables used by the C version and not the C++ version.
 *
 *--------------------------------------------------------------------------------------------------- */
#ifndef __cplusplus
int g_numAlreadyUsedVarNames=0;
int g_maxNumAlreadyUsedVarNames=0;
char **g_alreadyUsedNames=0;
#endif


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Variable Names
 * Purpose:     Empty the List of Variable Names
 * Description:
 * Frees and resets the global list of variable names.
 *
 *--------------------------------------------------------------------------------------------------- */
void 
VARIABLE_NAMES_CLASS_NAME_IF_CPP
free_variable_name_for_field_list()
{
  if(g_alreadyUsedNames)
    {
      int i;
      for(i=0;i<g_numAlreadyUsedVarNames;i++)
	{
	  if( g_alreadyUsedNames[i] )
	    {
	      free(g_alreadyUsedNames[i]);
	      g_alreadyUsedNames[i]=0;
	    }
	}
      free(g_alreadyUsedNames);
      g_alreadyUsedNames=0;
    }
  g_numAlreadyUsedVarNames=0;
  g_maxNumAlreadyUsedVarNames=0;
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Variable Names
 * Purpose:     Has this Variable Name Already Been Used?
 * Description: Prevents repeating variable names. 
 * 
 *
 *--------------------------------------------------------------------------------------------------- */
int 
VARIABLE_NAMES_CLASS_NAME_IF_CPP
has_this_name_been_used( const char *a_name )
{
  int j;
  for(j=0;j<g_numAlreadyUsedVarNames;j++)
    {
      if( g_alreadyUsedNames[j] && !strcmp(a_name,g_alreadyUsedNames[j]) )
	{
	  return(1);
	}
    }
  return(0);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Variable Names
 * Purpose:     Compose a Variable Name From a List of Field Names
 * Description:
 *
 * Pick a variable name based on a 1d array of fields. Global variables keep track of the
 * names used (see has_this_name_been_used), so that none are ever repeated (Ensight will 
 * ignore repeated names).
 *
 *--------------------------------------------------------------------------------------------------- */
char *
VARIABLE_NAMES_CLASS_NAME_IF_CPP
get_variable_name_for_field_list(int a_numFields, /*number of fields in the list*/
				       SAF_Field *a_fields, /*list of fields*/
				       unsigned int a_maxlen /*maximum number of characters allowed in returned name*/
				       )
{
  int i,l_first=1;
  char *l_commonName=0;

  if(!a_numFields||!a_maxlen) return(0);

  l_commonName = (char *)malloc((a_maxlen+1)*sizeof(char));
  l_commonName[0]='\0';
 
  /*allocate or resize the global array that keeps track of every name this
    function creates*/
  if(g_numAlreadyUsedVarNames>=g_maxNumAlreadyUsedVarNames)
    {
      if(g_alreadyUsedNames)
	{ /*resize the array larger*/
	  int l_newNum = g_maxNumAlreadyUsedVarNames*2;
	  char **l_newNames = (char **)malloc(l_newNum*sizeof(char *));

#ifndef __cplusplus
	  printinfo("resizing arrays in get_variable_name_for_field_list from size %d to %d\n",g_maxNumAlreadyUsedVarNames,l_newNum);
#endif

	  for(i=0;i<g_maxNumAlreadyUsedVarNames;i++) l_newNames[i] = g_alreadyUsedNames[i];
	  for(i=g_maxNumAlreadyUsedVarNames;i<l_newNum;i++) l_newNames[i] = 0;
	  free(g_alreadyUsedNames);
	  g_alreadyUsedNames = l_newNames;
	  g_maxNumAlreadyUsedVarNames = l_newNum;
	}
      else
	{
	  g_maxNumAlreadyUsedVarNames=128;
	  g_alreadyUsedNames = (char **)malloc(g_maxNumAlreadyUsedVarNames*sizeof(char *));
	  for(i=0;i<g_maxNumAlreadyUsedVarNames;i++) g_alreadyUsedNames[i]=0;
	}
    }


  /*find if there is a pattern in the first letters of the field names*/
  for(i=0;i<a_numFields;i++)
    {
      if( my_saf_is_valid_field_handle( &(a_fields[i])) )      
	{
	  char *l_name=0;
	  saf_describe_field(SAF_ALL, &(a_fields[i]), NULL, &l_name, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
			     NULL, NULL, NULL, NULL, NULL, NULL);		
	  if(l_first) 
	    {/*this is the first one, copy it entirely to l_commonName*/
	      strncpy(l_commonName,l_name,a_maxlen);
	      l_first=0;
	    }
	  else
	    {
	      int k,l_min = MIN(strlen(l_commonName),strlen(l_name));
	      for(k=0;k<l_min;k++) 
		{
		  if( l_commonName[k] != l_name[k] )
		    {
		      l_commonName[k]='\0';
		      break;
		    }
		}
	    }
	  if(l_name) free(l_name);
	}
    }

  if( !has_this_name_been_used(l_commonName) && strlen(l_commonName)>0 )
    {
      /*found a good name, ok to leave*/
      g_alreadyUsedNames[g_numAlreadyUsedVarNames++]=l_commonName;
      return( ens_strdup(l_commonName) );
    }

  /*create a name based on the quantity and the units of the first field*/
  {
    SAF_Unit l_unit;
    SAF_Quantity l_quantity;
    char *l_unitName=0;
    char *l_quantName=0;
    saf_describe_field(SAF_ALL, &(a_fields[0]), NULL,NULL,NULL, &l_unit, 
		       NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    saf_describe_unit(SAF_ALL,&l_unit,&l_unitName,NULL,NULL,NULL,NULL,NULL,NULL, &l_quantity );
    saf_describe_quantity(SAF_ALL,&l_quantity,&l_quantName,NULL,NULL,NULL,NULL);

    if( !l_unitName || !strlen(l_unitName) )
      {
	snprintf(l_commonName,a_maxlen,"%s",l_quantName );
      }
    else
      {
	snprintf(l_commonName,a_maxlen,"%s_%s",l_quantName,l_unitName );
      }
    if(l_unitName) free(l_unitName);
    if(l_quantName) free(l_quantName);

    /*Change spaces to underscores for the "not applicable" quantity case.
      I dont know if this is necessary. Perhaps should do it for field and unit names too?*/
    for(i=0;i<(int)strlen(l_commonName);i++) if(l_commonName[i]==' ') l_commonName[i]='_';
  }

  if( !has_this_name_been_used(l_commonName) && strlen(l_commonName)>0 )
    {
      /*found a good name, ok to leave*/
      g_alreadyUsedNames[g_numAlreadyUsedVarNames++]=l_commonName;
      return( ens_strdup(l_commonName) );
    }

  /*take the current name and keep incrementing a suffix until it has never been used*/
  {
    const int l_limit=999;
    int l_count=1;
    char *l_orig = ens_strdup(l_commonName);
    if( strlen(l_orig)>=a_maxlen-4 && a_maxlen>6 )
      {
	l_orig[a_maxlen-5]='\0';/*make sure there is room for the suffix at the end*/
      }
    while(l_count<l_limit)
      {
	snprintf(l_commonName,a_maxlen,"%s_%d",l_orig,l_count++);/*might add as many as 4 digits(_ + 999)*/

	if( !has_this_name_been_used(l_commonName) )
	  {
	    /*found a good name, ok to leave*/
	    g_alreadyUsedNames[g_numAlreadyUsedVarNames++]=l_commonName;
	    if(l_orig) free(l_orig);
	    return( ens_strdup(l_commonName) );
	  }
      }
    if(l_orig) free(l_orig);
  }

  /*all attempts to find a unique name failed*/
  snprintf(l_commonName,a_maxlen,"var_name_error");
  g_alreadyUsedNames[g_numAlreadyUsedVarNames++]= l_commonName;
  return( ens_strdup("var_name_error") );
}



