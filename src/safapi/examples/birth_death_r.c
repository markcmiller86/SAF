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
#include <saf.h>
#include <../test/testutil.h>
#include <readutils.h>

SAF_Db *db=NULL; /*This global variable is referenced from readutils.c and must not be static*/

int
main(int argc,
     char **argv)
{
  char dbname[1024] ="\0", *dbname_p=dbname;
  char sfdirname[1024] ="\0", *sfdirname_p=sfdirname;
  hbool_t do_describes = false;
  hbool_t do_reads = false;
  hbool_t multifile;
  int i, j, rank=0, nerrors=0, nprocs=1;
  int tdim, num_colls;
  SAF_SilRole srole;
  SAF_DbProps *dbprops;
  int l_quiet=0;

  multifile = false;
 
   

#ifdef HAVE_PARALLEL
  /* the MPI_init comes first because on some platforms MPICH's mpirun doesn't pass
     the same argc, argv to all processors. However, the MPI spec says nothing about
     what it does or might do to argc or argv. In fact, there is no "const" in the
     function prototypes for either the pointers or the things they're pointing too.
     I would rather pass NULL here and the spec says this is perfectly acceptable.
     However, that too has caused MPICH to core on certain platforms.  */
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
#endif

  STU_ProcessCommandLine(1, argc, argv,
			 "do_reads",
			 "if present, if present, test the saf_read_xxx() calls",
			 &do_reads,
			 "do_describes",
			 "if present,test saf_describe_xxx() calls",
			 &do_describes,
			 "-db %s",
			 "specify a database name",
			 &dbname_p,
			 "-sfdir %s",
			 "specify a supplemental file directory",
			 &sfdirname_p,
			 "-quiet",
			 "if present, print no commentary",
			 &l_quiet,
			 STU_END_OF_ARGS);


  /* for convenience, set working directory to the test file directory unless file was specified on command line */
  if (!strlen(dbname))
    chdir(TEST_FILE_PATH);

#ifdef HAVE_PARALLEL
  MPI_Barrier(MPI_COMM_WORLD);
#endif

  saf_init(SAF_DEFAULT_LIBPROPS);

  if (!strlen(dbname))
    strcpy(dbname, "birth_death.saf");


  TESTING("setProps and open database"); 

  /* note: because we are in a try block here, all failures will send us to the one and only
     catch block at the end of this test */

  dbprops = saf_createProps_database();
  saf_setProps_ReadOnly(dbprops);
#ifdef SSLIB_SUPPORT_PENDING
  if (strlen(sfdirname))
    saf_setProps_SFileDir(dbprops, sfdirname);
#endif /*SSLIB_SUPPORT_PENDING*/
  db = saf_open_database(dbname,dbprops);

#ifdef SSLIB_SUPPORT_PENDING
  /* determine if the database was generated in a multifile mode */
  saf_find_files(db, SAF_ANY_NAME, &i, NULL);
  if (i > 0)
    multifile = true;
  else
    multifile = false;

  saf_self = SAF_SELF(db);
#endif /*SSLIB_SUPPORT_PENDING*/

  PASSED;

  TESTING("finding all sets");

  all_sets = NULL;
  saf_find_matching_sets(SAF_ALL, db, SAF_ANY_NAME, SAF_ANY_SILROLE, SAF_ANY_TOPODIM,
			 SAF_EXTENDIBLE_TORF, SAF_TOP_TORF, &num_all_sets, &all_sets);


  allsets = (Set_Info *)malloc(num_all_sets * sizeof(Set_Info));
  for( i = 0 ; i < num_all_sets; i++ ) {

    allsets[i].the_set = all_sets[i];
      
    set_name=NULL;
    saf_describe_set(SAF_ALL, &(all_sets[i]), &set_name, &tdim, &srole, &e_mode, &t_mode, &num_colls, NULL);
    /* printf("%d: %s\n",i,set_name); */

    allsets[i].the_set = all_sets[i];
    allsets[i].num_sets = num_all_sets;
    allsets[i].my_num_in_set_list = i;
    allsets[i].set_name = set_name;
    allsets[i].max_topo_dim = tdim;
    allsets[i].silrole = srole;
    allsets[i].extmode = e_mode;
    allsets[i].topo_relation = NULL;
    allsets[i].collections = NULL;
    allsets[i].fields = NULL;
    allsets[i].num_fields = 0;
  }
  PASSED;

  TESTING("finding collections, relations, and fields");
  for( i = 0 ; i < num_all_sets; i++ ) {
    find_collections( allsets, i);
    find_fields_on_set( allsets, i);
    find_topo_relations_on_set( allsets + i );
  }
  PASSED;


  TESTING("finding top sets");

  /* find all the top sets */
  /* in order to eliminate finding suites, we only look for sets with SILROLE == SAF_SPACE */
  saf_find_matching_sets(SAF_ALL, db, SAF_ANY_NAME, SAF_SPACE, SAF_ANY_TOPODIM, \
			 SAF_EXTENDIBLE_TORF, SAF_TOP_TRUE, &num_top_sets, &top_sets);


  tops = (Sets *)malloc( num_top_sets * sizeof(Sets));
  top_nums = (int *)malloc(num_top_sets * sizeof(int));
  for( i = 0 ; i < num_top_sets; i++ ) {
    for( j = 0 ; j < num_all_sets; j++ ) {
      if( SAF_EQUIV( &(top_sets[i]), &(allsets[j].the_set)) ) {
	tops[i] = allsets + j;
	top_nums[i] = j;
      }
    }
  }


  for( i = 0; i < num_top_sets; i++ ) {
    for( j = 0 ; j < tops[i]->num_colls; j++ ) {
      get_subsets( tops[i], j);
    }
  }

  PASSED;
    

  TESTING("finding suites");

  /* find all suites */
  saf_find_suites( SAF_ALL, db, SAF_ANY_NAME, &num_suites, &suite_sets );
    
  suite_nums = (int *)malloc(num_suites * sizeof(int));
  suites = (Sets *)malloc( num_suites * sizeof(Sets));
  for( i = 0 ; i < num_suites ; i++ ) {
    for( j = 0; j < num_all_sets ; j++ ) {
      if( SAF_EQUIV( &(suite_sets[i]), &(allsets[j].the_set))  ) {
	suites[i] = allsets + j;
	suite_nums[i] = j;
      }
    }
  }
    
  PASSED;





  {

    Field_Info *suite_index;
    Field_Info *state_field;
    Field_Info **fields;
    Set_Info *base_space, *subsets;
    float *index_value;
    int num_indexes, in, is, sn, fn,  num_fields_in_state;
    int nc, ns;
    int subset_coll_num = 0;
    int num_procs = 0;
    SAF_Rel *sub_rel;
    void *rel_data;
    SAF_AltIndexSpec *altIndexSpecs;
    int num_alt_indexspecs;
    char *indexspec_name;
    hbool_t is_explicit;
    SAF_IndexSpec implicit_ispec;
    hbool_t is_compact;
    hbool_t is_sorted;
    void *indexspec_data;

    Collection_Info *procs_collection, *subset_collection;

    for( i = 0 ; i < num_suites; i++ ) {
      suite_index = suites[i]->fields + 0;
      read_field( suite_index );
      num_indexes = suite_index->data_size;
	
      if( num_indexes > 0 ) {

	if(!l_quiet)
	  {
	    printf("\n\n\nSuite %s index values: ",suites[i]->set_name);
	    for( in = 0 ; in < num_indexes ; in ++ ) {
	      index_value = *((float **)(suite_index->field_data));
	      printf("%.2f ",index_value[in]); 
	    
	    }
	  }


	for( sn = 1 ; sn < suites[i]->num_fields ; sn++ ) {

	  state_field = suites[i]->fields + sn;
	  if(!l_quiet) printf("\n\nSuite %s, State %s contains fields:\n\n",suites[i]->set_name, state_field->field_name);
	  num_fields_in_state = state_field->template_info->num_comps;
	  read_state_field( state_field );



	  if( !l_quiet && num_fields_in_state > 0 ) {
	    printf("\tBase Space\t\t:  Field Name\n");
	    printf("-----------------------------------------------------\n");
	  }

	  for( fn = 0 ; fn < num_fields_in_state ; fn++ ) {
	    fields = (Field_Info **)(state_field->field_data);
	    /* base_space = (Set_Info *)(fields[fn]->template_info->base_space); */
	    base_space = (Set_Info *)(fields[fn]->base_space); 
	    if(!l_quiet) printf("\t%-25s: %s\n", base_space->set_name, fields[fn]->field_name);
	      
	    for( nc = 0 ; nc < base_space->num_colls; nc++ ) {

	      if( (base_space->collections[nc].celltype == SAF_CELLTYPE_SET) && \
		  (!strcmp(base_space->collections[nc].cat_name,"blocks"))  ) {

		procs_collection = &(base_space->collections[nc]);
		subsets = (Set_Info *)(base_space->collections[nc].child_setlink);
		num_procs = base_space->collections[nc].num_children;
	      }
	    }
	  }


	  if(!l_quiet) printf("\n\t%s elems are distributed across %d processor subsets\n\n",base_space->set_name, num_procs);

	  for( nc = 0 ; nc < base_space->num_colls; nc++ ) {
	    altIndexSpecs = NULL;
	    saf_find_alternate_indexspecs(SAF_ALL,&(base_space->the_set), &(base_space->collections[nc].the_cat), 
					  SAF_ANY_NAME, &num_alt_indexspecs, &altIndexSpecs);

	    if( num_alt_indexspecs == 1 ) 
	      {

		indexspec_name = NULL;
		indexspec_data = NULL;
		saf_describe_alternate_indexspec( SAF_ALL, altIndexSpecs, NULL, NULL, &indexspec_name, 
						  NULL, &is_explicit, &implicit_ispec, &is_compact, &is_sorted);
		saf_read_alternate_indexspec( SAF_ALL, altIndexSpecs, &indexspec_data);
		if(!l_quiet) 
		  {
		    printf("\t%d %s on %s:%s:\n\t\t",base_space->collections[nc].count, indexspec_name, 
			   base_space->set_name, base_space->collections[nc].cat_name);
		    for( is = 0 ; is < base_space->collections[nc].count; is++ ) {
		      printf(" %d ",((int *)indexspec_data)[is]);

		    }
		    printf("\n");
		  }
	      }
	      

	  }


	  for( ns = 0; ns < num_procs; ns++ ) {
	    for( nc = 0 ; nc < subsets[ns].num_colls; nc++ ) {
	      if( !strcmp(subsets[ns].collections[nc].cat_name, "elems") ) {
		subset_coll_num = nc;
		subset_collection = subsets[ns].collections + nc;
	      }
	    }


	    sub_rel = subset_collection->subset_relation;
	    rel_data = NULL;
	    saf_read_subset_relation(SAF_ALL, sub_rel,NULL, &rel_data, NULL);
	    subset_collection->relation_data = rel_data;
	    if(!l_quiet) printf("\t\t%s:%s has %d elems\n",subsets[ns].set_name, subset_collection->cat_name, \
				subset_collection->count);
	      
	  }


	}
      }
    }

  }

  saf_final();

#ifdef HAVE_PARALLEL
  /* make sure everyone returns the same error status */
  MPI_Bcast(&nerrors, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Finalize();
#endif


  return nerrors?1:0; 
}


