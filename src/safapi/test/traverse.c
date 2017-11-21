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
#include <testutil.h>

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Tests 
 * Purpose:    	Traverse Set Inclusion Lattice to test saf_find_sets
 *
 * Description: You pass this function a set in the set inclusion lattice (SIL) and
 * it will printf that sets name and all of the subsets of the set, then do this
 * recursively on the subsets of the subsets, etc.
 *
 *
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien August 14, 2001
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void Traverse_SIL(SAF_Set *set, SAF_Cat *cat, char **tab, int *tab_buffer_len,  hbool_t do_traversal);


void 
Traverse_SIL(
	     SAF_Set *set, /*The set in the subset inclusion lattice at which to begin searching. */
	     SAF_Cat *cat, /*The collection category upon which to search for subsets, supersets, or leaf sets.*/ 
	     char **tab, /*The spacing in front of the printf's, so subsets are indented*/ 
	     int *tab_buffer_len, /*The available length of tab*/ 
	     hbool_t do_traversal /*if true, display traversal of SIL*/
)
{

      char *found_name=NULL;
      int max_topo_dim,num_sets,j;
      SAF_Set *found_sets = NULL;  

      saf_describe_set(SAF_ALL,set,&found_name,&max_topo_dim,NULL,NULL,NULL,NULL,NULL);
      
      found_sets=NULL;
      saf_find_sets(SAF_ALL,SAF_FSETS_SUBS,set,cat,&num_sets,&found_sets);

      if(num_sets && do_traversal)
	printf("%sTraversing %s, of max topo dim %i\n", *tab,found_name, max_topo_dim);


      for(j=0; j< num_sets; j++)
      {
	  free(found_name);
	  found_name=NULL;
	  saf_describe_set(SAF_ALL,found_sets+j,&found_name,&max_topo_dim,NULL,NULL,NULL,NULL,NULL);
	  if(do_traversal)
	    printf("%s%i/%i, of max topo dim %i, and name %s\n", *tab,j,num_sets,max_topo_dim, found_name);
	  
      }

      free(found_name);

      {
         int tab_len=0;
         tab_len = (int)strlen(*tab);
	 if( tab_len+1 >= *tab_buffer_len )
         { /*reallocate*/
            char *new_tab;
            *tab_buffer_len += 256;
            new_tab = (char *)malloc(*tab_buffer_len*sizeof(char));
            strcpy(new_tab,*tab);
            free(*tab);
            *tab=new_tab;
         }
         tab[0][tab_len] = '\t';
         tab[0][tab_len+1] = '\0';

         for(j=0; j< num_sets; j++)
         {
	     Traverse_SIL(found_sets+j,cat,tab,tab_buffer_len,do_traversal);
         }
         tab[0][tab_len] = '\0';
      }


      free(found_sets);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Tests 
 * Purpose:    	Test saf_find_sets 
 *
 * Description: This started as larry1r.c and was modified by Mattehw O'Brien to test
 * the function saf_find_sets.  Just like larry1r, traverse needs to be run *after*
 * larr1w, because it used the output of larry1w.  Tests all the modes of saf_find_sets
 * and checks the results for correctness.
 *
 *
 *
 *
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL, September 2001.
 *
 * Modifications: Matthew O'Brien, LLNL, October 23, 2001.  This file had a bunch of tests not related to saf_find_sets,
 * 			they have been moved into misc.c.  Now this test client only test various modes of saf_find_sets
 *			and checks the output of the function for correctness. 
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
main(int argc,
     char **argv)
{
   char *dbname=0;
   char *sfdirname=0;
   hbool_t do_describes = false;
   hbool_t do_reads = false;
   hbool_t do_traversal=false; 
   int nerrors=0;
   int num_cats, num_sets;
   int command_line_result=0;

   SAF_LibProps *libprops; 
   SAF_Set *found_sets = NULL;
   char *found_name;  
   int max_topo_dim, j;
   int found_correct=0;

   SAF_Db *db=NULL;
   SAF_DbProps *dbprops=NULL;
   SAF_Cat(cats, p_cats, 2);
   SAF_Set(sets, p_sets, 2);
   SAF_Cat nodes;
   SAF_Set top, cell_2, cell_2_tri;
   SAF_Set ss1;

#ifdef HAVE_PARALLEL
   /* the MPI_init comes first because on some platforms MPICH's mpirun doesn't pass
      the same argc, argv to all processors. However, the MPI spec says nothing about
      what it does or might do to argc or argv. In fact, there is no "const" in the
      function prototypes for either the pointers or the things they're pointing too.
      I would rather pass NULL here and the spec says this is perfectly acceptable.
      However, that too has caused MPICH to core on certain platforms.  */
   MPI_Init(&argc,&argv);
#endif

   /* since we want to see whats happening make sure stdout and stderr are unbuffered */
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);

   /* process command line args */
   command_line_result = STU_ProcessCommandLine(1, argc, argv,
           "do_reads",
              "if present, test the saf_read_xxx() calls",
              &do_reads,
           "do_describes",
              "if present, test saf_describe_xxx() calls",
              &do_describes,
           "do_traversal",
              "if present, display traversal of the SIL",
              &do_traversal,
           "-sfdir %s",
              "-sfdir %%s    : specify a supplemental file directory",
              &sfdirname,
           "-db %s",
              "-db %%s    : specify a database name",
              &dbname,
           STU_END_OF_ARGS);

   if( command_line_result < 0 )
   {
     goto theExit;
   }

   if( !dbname )
   {
      /*did not get dbname in command line reader: allocate here*/
      dbname = (char *)malloc(1024*sizeof(char));
      dbname[0]='\0';
   }
   if( !sfdirname )
   {
      /*did not get sfdirname in command line reader: allocate here*/
      sfdirname = (char *)malloc(1024*sizeof(char));
      sfdirname[0]='\0';
   }

  /* for convenience, set working directory to the test file directory unless file was specified on command line */
  if (!strlen(dbname))
     chdir(TEST_FILE_PATH);

#ifdef HAVE_PARALLEL
  MPI_Barrier(MPI_COMM_WORLD);
#endif

  libprops = saf_createProps_lib();
  saf_setProps_ErrorMode(libprops,SAF_ERRMODE_THROW);
  saf_init(libprops);
  saf_freeProps_lib(libprops);

  if (!strlen(dbname))
     strcpy(dbname, TEST_FILE_NAME);

  SAF_TRY_BEGIN
  {

    /* note: because we are in a try block here, all failures will send us to the one and only
       catch block at the end of this test */

    dbprops = saf_createProps_database();
    saf_setProps_ReadOnly(dbprops);
#ifdef SSLIB_SUPPORT_PENDING
    if (strlen(sfdirname))
       saf_setProps_SFileDir(dbprops, sfdirname);
#endif /*SSLIB_SUPPORT_PENDING*/
    db = saf_open_database(dbname,dbprops);

    /*
     -----------------------------------------------------------------------------------------------------------------------------
     *                                                    FIND CATEGORIES
     -----------------------------------------------------------------------------------------------------------------------------
     */

    /* find the topological category whose name is "nodes" */
    TESTING("finding categories [name=\"nodes\",{role,tdim}=any]");
    num_cats=2;
    saf_find_categories(SAF_ALL,db,SAF_UNIVERSE(db), "nodes", SAF_ANY_ROLE, SAF_ANY_TOPODIM, &num_cats, &p_cats);
    if (num_cats != 1)
    {
	printf("\n Note, larry1w must be run before traverse\n");
       FAILED;
       nerrors++;
    }
    else
       PASSED;
    nodes = cats[0];

    /* find the topological category whose name is "elems" */
    TESTING("finding categories [name=\"elems\",{role,tdim}=any]");
    num_cats = 2;
    saf_find_categories(SAF_ALL,db,SAF_UNIVERSE(db), "elems", SAF_ANY_ROLE, SAF_ANY_TOPODIM, &num_cats, &p_cats);
    if (num_cats != 1)
    {
	printf("\n Note, larry1w must be run before traverse\n");
       FAILED;
       nerrors++;
    }
    else
       PASSED;



    /*
     -----------------------------------------------------------------------------------------------------------------------------
     *                                                    FIND MATCHING SETS 
     -----------------------------------------------------------------------------------------------------------------------------
     */

    TESTING("finding matching sets [name_grep=\"CELL_2\"]");
    saf_find_matching_sets(SAF_ALL, db, "CELL_2", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
       SAF_TOP_TORF, &num_sets, NULL); 
    if (num_sets != 1)
    {
        FAILED;
        nerrors++;
    }
    else
       PASSED;

#if defined WIN32 || defined JANUS /*WIN32 does not have regcomp,regcmp,regex (@)*/
    TESTING("finding matching sets [name_grep=\"@^CELL_2$\"]");
	SKIPPED;
    TESTING("finding matching sets [name_grep=\"@CELL\"]");
	SKIPPED;
    TESTING("finding matching sets [name_grep=\"@^CELL_[123]$\"]");
	SKIPPED;
#else
    TESTING("finding matching sets [name_grep=\"@^CELL_2$\"]");
    saf_find_matching_sets(SAF_ALL, db, "@^CELL_2$", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
       SAF_TOP_TORF, &num_sets, NULL); 
    if (num_sets != 1)
    {
        FAILED;
        nerrors++;
    }
    else
       PASSED;


    TESTING("finding matching sets [name_grep=\"@CELL\"]");
    saf_find_matching_sets(SAF_ALL, db, "@CELL", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
       SAF_TOP_TORF, &num_sets, NULL); 
    if (num_sets != 6)
    {
        FAILED;
        nerrors++;
    }
    else
       PASSED;


    TESTING("finding matching sets [name_grep=\"@^CELL_[123]$\"]");
    saf_find_matching_sets(SAF_ALL, db, "@^CELL_[123]$", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
       SAF_TOP_TORF, &num_sets, NULL); 
    if (num_sets != 3)
    {
        FAILED;
        nerrors++;
    }
    else
       PASSED;
#endif/*WIN32*/


    TESTING("finding matching sets [name_grep=\"TOP_CELL\"]");
    num_sets=2;
    saf_find_matching_sets(SAF_ALL, db, "TOP_CELL", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
       SAF_TOP_TORF, &num_sets, &p_sets); 
    top = sets[0];
    if (num_sets != 1)
    {
	printf("\n Note, larry1w must be run before traverse\n");	
       FAILED;
       nerrors++;
    }
    else
       PASSED;



    /* find some other sets needed for tests below */
    saf_find_matching_sets(SAF_ALL, db, "CELL_1", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
       SAF_TOP_TORF, &num_sets, &p_sets); 
    saf_find_matching_sets(SAF_ALL, db, "CELL_2", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
       SAF_TOP_TORF, &num_sets, &p_sets);
    cell_2 = sets[0];
    saf_find_matching_sets(SAF_ALL, db, "CELL_2_TRIS", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
       SAF_TOP_TORF, &num_sets, &p_sets);
    cell_2_tri = sets[0];
    saf_find_matching_sets(SAF_ALL, db, "CELL_2_QUADS", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
       SAF_TOP_TORF, &num_sets, &p_sets);
    saf_find_matching_sets(SAF_ALL, db, "CELL_3", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
       SAF_TOP_TORF, &num_sets, &p_sets); 
    saf_find_matching_sets(SAF_ALL, db, "NODE_SET_1", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
       SAF_TOP_TORF, &num_sets, &p_sets); 
    saf_find_matching_sets(SAF_ALL, db, "TIME", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
       SAF_TOP_TORF, &num_sets, &p_sets); 

    saf_find_matching_sets(SAF_ALL, db, "SIDE_SET_1", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
       SAF_TOP_TORF, &num_sets, &p_sets); 
    ss1 = sets[0];


    /*
     -----------------------------------------------------------------------------------------------------------------------------
     *                                                   TESTING saf_find_sets 
     *			
     -----------------------------------------------------------------------------------------------------------------------------
     */
     /*
       If fmode is SAF_FSETS_TOP or SAF_FSETS_BOUNDARY, then cat
       must be SAF_NOT_APPLICABLE_CAT.  and conversely.

       if fmode is SAF_FSETS_SUPS or  SAF_FSETS_SUBS or  SAF_FSETS_LEAVES
       then cat must be a valid handle (i.e. cat must not be SAF_NOT_APPLICABLE_CAT)


       if fmode is SAF_FSETS_TOP or SAF_FSETS_BOUNDARY) 
       then cat must be SAF_NOT_APPLICABLE_CAT

     
     **************************************************************************
		   
     SAF_FSETS_TOP 
         find the top-level from the given set 

     SAF_FSETS_BOUNDARY 
         find the boundary of the given set 

     SAF_FSETS_SUBS 
         find the immediate subsets of the given set 

     SAF_FSETS_SUPS 
         find the immediate supersets of the given set 

     SAF_FSETS_LEAVES 
         find all the bottom most sets in the tree rooted at the given set 


    */


    /*---------------saf_find_sets SUPS--------------------*/
      TESTING("saf_find_sets, SUPS");
      found_sets=NULL;
      found_correct=0;
      num_sets=0;

      saf_find_sets(SAF_ALL,SAF_FSETS_SUPS,&cell_2_tri,&nodes,&num_sets,&found_sets);
      
      for(j=0; j< num_sets; j++){
	found_name=NULL;
	saf_describe_set(SAF_ALL,found_sets+j,&found_name,&max_topo_dim,NULL,NULL,NULL,NULL,NULL);
	if( !strcmp(found_name, "TOP_CELL") || !strcmp(found_name,"CELL_2")){
	  found_correct++;
	}
	free(found_name);
      }

      if(found_correct!=2){
	printf("Should have found both TOP_CELL and CELL_2 as supersets of cell_2_tri\n");
	FAILED;
      }
      free(found_sets);
      PASSED;

      TESTING("saf_find_sets, SUPS");
      found_sets=NULL;
      num_sets=0;
      found_correct=0;
      saf_find_sets(SAF_ALL,SAF_FSETS_SUPS,&cell_2_tri,&nodes,&num_sets,&found_sets);
      
      for(j=0; j< num_sets; j++){
	found_name=NULL;
	saf_describe_set(SAF_ALL,found_sets+j,&found_name,&max_topo_dim,NULL,NULL,NULL,NULL,NULL);
	if( !strcmp(found_name, "TOP_CELL") || !strcmp(found_name,"CELL_2")){
	  found_correct++;
	}
	free(found_name);
      }

      if(found_correct!=2){
	printf("Should have found both TOP_CELL and CELL_2 as supersets of cell_2_tri, only found %i set(s)\n", num_sets);
	FAILED;
      }
      free(found_sets);
      PASSED;
    

    /*---------------saf_find_sets TOP--------------------*/
    TESTING("saf_find_sets, TOP");
    found_sets = NULL;
    num_sets = 0;
    saf_find_matching_sets(SAF_ALL, db, "TOP_CELL", SAF_ANY_SILROLE, SAF_ANY_TOPODIM,SAF_EXTENDIBLE_TORF,
			   SAF_TOP_TORF, &num_sets, &found_sets);

    found_name=NULL;
    saf_describe_set(SAF_ALL,found_sets,&found_name,&max_topo_dim,NULL,NULL,NULL,NULL,NULL);
    free(found_sets);
    if(!strcmp(found_name, "TOP_CELL"))
      {
	PASSED;
      }
    else
      {
	FAILED;
      }
    free(found_name);



    TESTING("saf_find_sets, TOP");
    found_sets = NULL;
    num_sets = 0;
    saf_find_matching_sets(SAF_ALL, db, "TOP_CELL", SAF_ANY_SILROLE, SAF_ANY_TOPODIM,SAF_EXTENDIBLE_TORF,
			   SAF_TOP_TORF, &num_sets, &found_sets);


    found_name=NULL;
    saf_describe_set(SAF_ALL,found_sets,&found_name,&max_topo_dim,NULL,NULL,NULL,NULL,NULL);
    top=found_sets[0];
    if(!strcmp(found_name, "TOP_CELL"))
      {
	PASSED;
      }
    else
      {
	FAILED;
      }
	free(found_name);
	free(found_sets);


    /*---------------saf_find_sets SUBS--------------------*/
    TESTING("saf_find_sets, SUBS (Traversing top set to find all subsets)");
    found_sets = NULL;
    num_sets = 0;
    saf_find_matching_sets(SAF_ALL, db, "TOP_CELL", SAF_ANY_SILROLE, SAF_ANY_TOPODIM,SAF_EXTENDIBLE_TORF,
			   SAF_TOP_TORF, &num_sets, &found_sets);
    top = found_sets[0];

	if(do_traversal) printf("\n");

	{
	   int tab_buffer_len = 256;
           char *tab = (char *)malloc(tab_buffer_len*sizeof(char));
	   tab[0]='\0';
	   Traverse_SIL(&top, &nodes, &tab, &tab_buffer_len, do_traversal);
	   free(tab);
  	}

    PASSED;

    /*---------------saf_find_sets SUBS of top--------------------*/
    TESTING("saf_find_sets, SUBS of top, using nodes category");
    found_sets = NULL;
    num_sets=0;
    saf_find_sets(SAF_ALL,SAF_FSETS_SUBS,&top,&nodes,&num_sets,&found_sets);

    found_correct=0;
    for(j=0; j< num_sets; j++){
      found_name=NULL;
      saf_describe_set(SAF_ALL,found_sets+j,&found_name,&max_topo_dim,NULL,NULL,NULL,NULL,NULL);
      if( !strcmp(found_name, "CELL_2_TRIS") || !strcmp(found_name,"CELL_2_QUADS") ||
          !strcmp(found_name, "CELL_1") || !strcmp(found_name,"CELL_2") ||
          !strcmp(found_name, "CELL_3") || !strcmp(found_name,"SIDE_SET_1") ||
          !strcmp(found_name, "SIDE_SET_2") || !strcmp(found_name,"NODE_SET_1")  ){
	found_correct++;
      }
      free(found_name);
    }
    free(found_sets);

    if(found_correct!=8){
      printf("top should have 8 subsets (using nodes category)\n");
      FAILED;
    }else{
      PASSED;
    }    

    /*---------------saf_find_sets SUBS of top, using SAF_ANY_CAT--------------------*/
    /* NOTE: In the VBT/DSL version of SAF the saf_find_sets() call would not find all the `proc *' sets, but rather only the
     *       one that larry1w created in the same MPI task number. In fact, if larry1w was run with a different number of MPI
     *       tasks than what we are now running with, this test fails. The SSlib version finds all the `proc *' sets no matter
     *       how many MPI tasks were present when the file was created. */
    TESTING("saf_find_sets, SUBS of top, using SAF_ANY_CAT");
    found_sets = NULL;
    num_sets=0;
    saf_find_sets(SAF_ALL,SAF_FSETS_SUBS,&top,SAF_ANY_CAT,&num_sets,&found_sets);

    found_correct=0;
    for(j=0; j< num_sets; j++){
      found_name=NULL;
      saf_describe_set(SAF_ALL,found_sets+j,&found_name,&max_topo_dim,NULL,NULL,NULL,NULL,NULL);
      if (!strcmp(found_name, "CELL_2_TRIS") || !strcmp(found_name,"CELL_2_QUADS") ||
          !strcmp(found_name, "CELL_1")      || !strcmp(found_name,"CELL_2") ||
          !strcmp(found_name, "CELL_3")      || !strcmp(found_name,"SIDE_SET_1") ||
          !strcmp(found_name, "SIDE_SET_2")  || !strcmp(found_name,"NODE_SET_1") ||
          !strcmp(found_name, "empty set")) { 
          found_correct++;
      }
      free(found_name);
    }
    if(found_sets) free(found_sets);

    if(found_correct!=9 || num_sets<10){
      printf("top should have 9 subsets plus at least one \"proc\" set (using SAF_ANY_CAT), "
             "instead num_sets=%d found_correct=%d\n", num_sets,found_correct);
      FAILED;
    }else{
      PASSED;
    }    



    /*---------------saf_find_sets LEAVES of cell_2--------------------*/
    TESTING("saf_find_sets, LEAVES of cell_2");
	if(do_traversal) printf("\n");

    found_sets=NULL;
    num_sets=0;
    saf_find_sets(SAF_ALL,SAF_FSETS_LEAVES,  &cell_2,&nodes,&num_sets,&found_sets); 

    found_correct=0;
    for(j=0; j< num_sets; j++){
      found_name=NULL;
      saf_describe_set(SAF_ALL,found_sets+j,&found_name,&max_topo_dim,NULL,NULL,NULL,NULL,NULL);
      if( !strcmp(found_name, "CELL_2_TRIS") || !strcmp(found_name,"CELL_2_QUADS")){
	found_correct++;
      }
      if(do_traversal)
	printf("leaves %s, %i/%i \n", found_name, j,num_sets);

      free(found_name);

    }
    free(found_sets);

    if(found_correct!=2){
      printf("cell_2 should have 2 leaves, cell_2_tris and cell_2_quqds\n");
      FAILED;
    }else{
      PASSED;
    }    


    {
      /*      SAF_Set found_sets[10];
	      SAF_Set *p_sets = found_sets;*/


    TESTING("saf_find_sets, LEAVES of cell_2_tri, client allocates memory");

#if 1
    found_sets = NULL;
    num_sets=10;
    
    saf_find_sets(SAF_ALL,SAF_FSETS_LEAVES,  &cell_2_tri,&nodes,&num_sets,&found_sets);

    found_correct=0;
    for(j=0; j< num_sets; j++){
      found_name=NULL;
      saf_describe_set(SAF_ALL,found_sets+j,&found_name,&max_topo_dim,NULL,NULL,NULL,NULL,NULL);
      if( !strcmp(found_name, "CELL_2_TRIS") ){
	found_correct++;
      }
      if(do_traversal)
	printf("leaves %s, %i/%i \n", found_name, j,num_sets);

      free(found_name);
    }
    free(found_sets);
    if(found_correct!=1){
      printf("cell_2_tri should have 1 leaf, cell_2_tris \n");
      FAILED;
    }else{
      PASSED;
      }
#else	 
   SKIPPED;
#endif


/*-------------------blantant repeat of above test, to make sure saf_find_sets works correctly
		with the SAF_FSETS_LEAVES argument--------------------------------------------*/

    TESTING("saf_find_sets, LEAVES of cell_2_tri, client allocates memory (2nd time)");

#if 1
    found_sets = NULL;
    num_sets=10;
    
    saf_find_sets(SAF_ALL,SAF_FSETS_LEAVES,  &cell_2_tri,&nodes,&num_sets,&found_sets);

    found_correct=0;
    for(j=0; j< num_sets; j++){
      found_name=NULL;
      saf_describe_set(SAF_ALL,found_sets+j,&found_name,&max_topo_dim,NULL,NULL,NULL,NULL,NULL);
      if( !strcmp(found_name, "CELL_2_TRIS") ){
	found_correct++;
      }
      if(do_traversal)
	printf("leaves %s, %i/%i \n", found_name, j,num_sets);

      free(found_name);
    }
    free(found_sets);
    if(found_correct!=1){
      printf("cell_2_tri should have 1 leaf, cell_2_tris \n");
      FAILED;
    }else{
      PASSED;
      }
#else	 
   SKIPPED;
#endif




    }
    /*---------------saf_find_sets LEAVES of top--------------------*/

    
    TESTING("saf_find_sets, LEAVES of TOP");
    if(do_traversal) printf("\n");


    found_sets = NULL;
    num_sets=0;


    saf_find_sets(SAF_ALL,SAF_FSETS_LEAVES,  &top,&nodes,&num_sets,&found_sets);

    found_correct=0;
    for(j=0; j< num_sets; j++){
      found_name=NULL;
      saf_describe_set(SAF_ALL,found_sets+j,&found_name,&max_topo_dim,NULL,NULL,NULL,NULL,NULL);
      if (!strcmp(found_name, "CELL_1") ||
          !strcmp(found_name, "CELL_2_QUADS") ||
          !strcmp(found_name, "CELL_2_TRIS") ||
          !strcmp(found_name, "CELL_3") ||
          !strcmp(found_name, "NODE_SET_1") ||
          !strcmp(found_name, "SIDE_SET_1") ||
          !strcmp(found_name, "SIDE_SET_2")) {
          found_correct++;
      }
      if (do_traversal)
          printf("leaves %s, %i/%i \n", found_name, j,num_sets);
      free(found_name);
    }
    free(found_sets);
    if (found_correct!=7 || num_sets<8) {
        printf("found %d, expected 7 plus additional 2 per writer task\n", found_correct);
        FAILED;
    } else {
        PASSED;
    }
    

    /*---------------saf_find_sets BOUNDARY--------------------*/
    TESTING("saf_find_sets, BOUNDARY (no boundary)");
    found_sets=NULL;
    saf_find_sets(SAF_ALL,SAF_FSETS_BOUNDARY,&cell_2_tri,SAF_NOT_APPLICABLE_CAT,&num_sets,&found_sets);
    if(num_sets!=0){
      printf("should not have found any boundary subset\n");
      FAILED;
    }
    free(found_sets);
    PASSED;

    /*---------------saf_find_sets BOUNDARY --------------------*/

    TESTING("saf_find_sets, BOUNDARY (defined boundary)");
    found_sets=NULL;
    saf_find_sets(SAF_ALL,SAF_FSETS_BOUNDARY, &ss1,SAF_NOT_APPLICABLE_CAT,&num_sets,&found_sets);


      found_name=NULL;
      saf_describe_set(SAF_ALL,found_sets,&found_name,&max_topo_dim,NULL,NULL,NULL,NULL,NULL);

      if( !strcmp(found_name, "SIDE_SET_1_BOUNDARY")){
	PASSED;
      } else {
	printf("could not find the boundary of side_set_1\n");
	FAILED;
      }

	free(found_name);
	free(found_sets);

      /*end of saf_find_sets-------------------------------------------------*/



   saf_close_database(db);


  }
  SAF_CATCH
  {
    SAF_CATCH_ALL
    {
       FAILED;
       nerrors++;
    }
  }
  SAF_TRY_END

  saf_final();

theExit:

#ifdef HAVE_PARALLEL
   /* make sure everyone returns the same error status */
   MPI_Bcast(&nerrors, 1, MPI_INT, 0, MPI_COMM_WORLD);
   MPI_Finalize();
#endif

  return nerrors?1:0;
}
