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
#include <readutils.h>

/* Global arrays with the expected results from the saf_read_XXX calls. */
float coords_on_top_buf[45] = {0., 1., 2., 0., 1., 2., 0., 1., 2.,
                               0., 1., 0., 1., 0., 1., 0., 0., 0.,
                               0., 0., 0., 0., 0., 0., 1., 1., 1.,
                               1., 1., 1., 0., 0., 0., 1., 1., 1.,
                               2., 2., 2., 2., 2., 0., 0., 1., 1.};
float disp_on_top_s0_buf[45] = {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
                                0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
                                0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.};
float disp_on_top_s1_buf[45] = {1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 1.10, 1.11, 1.12, 1.13, 1.14,
                                2.0, 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9, 2.10, 2.11, 2.12, 2.13, 2.14,
                                3.0, 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 3.8, 3.9, 3.10, 3.11, 3.12, 3.13, 3.14};
float vel_on_top_s0_buf[45] = { 0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,
                                0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,
                                0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.};
float vel_on_top_s1_buf[45] = {10.0, 10.1, 10.2, 10.3, 10.4, 10.5, 10.6, 10.7, 10.8, 10.9, 10.10, 10.11, 10.12, 10.13, 10.14,
                               20.0, 20.1, 20.2, 20.3, 20.4, 20.5, 20.6, 20.7, 20.8, 20.9, 20.10, 20.11, 20.12, 20.13, 20.14,
                               30.0, 30.1, 30.2, 30.3, 30.4, 30.5, 30.6, 30.7, 30.8, 30.9, 30.10, 30.11, 30.12, 30.13, 30.14};
int elem_ids_on_top_s0_buf[12] = {100, 101, 102, 103, 104, 105, 106, 207, 208, 209, 210, 211};
int elem_ids_on_top_s1_buf[12] = {100, 101, 102, 103, 104, 105, 106, 207, 208, 209, 210, 211};
float cent_on_top_s0_buf[36] = {0.5,  0.5,  0.5,  0.5, 1.67, 1.33, 1.33, 1.67, 1.33, 1.33, 1.5 , 1.5 ,
                                0. ,  0. ,  0.5,  0.5, 0.  , 0.  , 0.  , 0.  , 0.33, 0.33, 0.25, 0.25,
                                1.5,  0.5,  1.5,  0.5, 1.67, 1.33,  .67, 0.33, 0.67, 1.33, 1.75, 1.25};
float cent_on_top_s1_buf[36] = {0.5,  0.5,  0.5,  0.5, 1.67, 1.33, 1.33, 1.67, 1.33, 1.33, 1.5 , 1.5 ,
                                0. ,  0. ,  0.5,  0.5, 0.  , 0.  , 0.  , 0.  , 0.33, 0.33, 0.25, 0.25,
                                1.5,  0.5,  1.5,  0.5, 1.67, 1.33,  .67, 0.33, 0.67, 1.33, 1.75, 1.25};
/*
float strain_on_blk1_s0_buf[12] = {10.1, 10.2, 11.1, 11.2, 12.1, 12.2, 13.1, 13.2, 14.1, 14.2, 15.1, 15.2};
float strain_on_blk1_s1_buf[12] = {10.1, 10.2, 11.1, 11.2, 12.1, 12.2, 13.1, 13.2, 14.1, 14.2, 15.1, 15.2};
*/
float strain_on_blk1_s0_buf[12] = {10.0, 10.1, 20.0, 20.1, 30.0, 30.1, 40.0, 40.1, 50.0, 50.1, 60.0, 60.1};
float strain_on_blk1_s1_buf[12] = {10.0, 10.1, 20.0, 20.1, 30.0, 30.1, 40.0, 40.1, 50.0, 50.1, 60.0, 60.1};
float press_on_blk1_s0_buf[2] = {1000., 2000.};
float press_on_blk1_s1_buf[2] = {1000., 2000.};
/*
float stress_on_blk2_s0_buf[12] = {100., 101., 200., 201., 300., 301., 400., 401., 500., 501., 600., 601.};
float stress_on_blk2_s1_buf[12] = {100., 101., 200., 201., 300., 301., 400., 401., 500., 501., 600., 601.};
*/
float stress_on_blk2_s0_buf[12] = {100.2, 100.3, 200.2, 200.3, 300.2, 300.3, 400.2, 400.3, 500.2, 500.3, 600.2, 600.3};
float stress_on_blk2_s1_buf[12] = {100.2, 100.3, 200.2, 200.3, 300.2, 300.3, 400.2, 400.3, 500.2, 500.3, 600.2, 600.3};
float press_on_blk2_s0_buf[2] = {3000., 4000.};
float press_on_blk2_s1_buf[2] = {3000., 4000.};
/*
float strain_on_blk3_s0_buf[24] = {0.1, 0.2, 0.3, 0.4,
                                   1.1, 1.2, 1.3, 1.4,
                                   2.1, 2.2, 2.3, 2.4,
                                   3.1, 3.2, 3.3, 3.4,
                                   4.1, 4.2, 4.3, 4.4,
                                   5.1, 5.2, 5.3, 5.4};
float strain_on_blk3_s1_buf[24] = {0.1, 0.2, 0.3, 0.4,
                                   1.1, 1.2, 1.3, 1.4,
                                   2.1, 2.2, 2.3, 2.4,
                                   3.1, 3.2, 3.3, 3.4,
                                   4.1, 4.2, 4.3, 4.4,
                                   5.1, 5.2, 5.3, 5.4};
*/
float strain_on_blk3_s0_buf[24] = {10.4, 10.5, 10.6, 10.7,
                                   20.4, 20.5, 20.6, 20.7,
                                   30.4, 30.5, 30.6, 30.7,
                                   40.4, 40.5, 40.6, 40.7,
                                   50.4, 50.5, 50.6, 50.7,
                                   60.4, 60.5, 60.6, 60.7};
float strain_on_blk3_s1_buf[24] = {10.4, 10.5, 10.6, 10.7,
                                   20.4, 20.5, 20.6, 20.7,
                                   30.4, 30.5, 30.6, 30.7,
                                   40.4, 40.5, 40.6, 40.7,
                                   50.4, 50.5, 50.6, 50.7,
                                   60.4, 60.5, 60.6, 60.7};
/*
float stress_on_blk5_s0_buf[12] = {100.,101.,200.,201.,300.,301.,400.,401.,500.,501.,600.,601.};
float stress_on_blk5_s1_buf[12] = {100.,101.,200.,201.,300.,301.,400.,401.,500.,501.,600.,601.};
*/
float stress_on_blk5_s0_buf[12] = {100.1,100.11,200.1,200.11,300.1,300.11,400.1,400.11,500.1,500.11,600.1,600.11};
float stress_on_blk5_s1_buf[12] = {100.1,100.11,200.1,200.11,300.1,300.11,400.1,400.11,500.1,500.11,600.1,600.11};
int block1_topo_buf[8] = {3, 4, 7, 6, 0, 1, 4, 3};
int block2_topo_buf[16] = {3, 4, 14, 13, 6, 7, 10, 9, 0, 1, 12, 11, 3, 4, 14, 13};
int block3_topo_buf[12] = {7, 5, 8, 4, 5, 7, 4, 1, 5, 1, 2, 5};
int block4_topo_buf[10] = {4, 14, 10, 7, 5, 1, 12, 14, 4, 5};
int block5_topo_buf[8] = {7, 5, 10, 8, 1, 2, 12, 5};


int
main(int argc,
     char **argv)
{
  char dbname[1024] ="\0", *dbname_p=dbname;
  char sfdirname[1024] ="\0", *sfdirname_p=sfdirname;
  hbool_t do_describes = false;
  hbool_t do_reads = false;
  int i, j, rank=0, nerrors=0;
  int tdim, num_colls;
  SAF_SilRole srole;
  SAF_DbProps *dbprops;

   
#ifndef HAVE_PARALLEL
  saf_init(SAF_DEFAULT_LIBPROPS);
  saf_final();
  SKIPPED;
  return(0);
#endif


#ifdef HAVE_PARALLEL
  /* the MPI_init comes first because on some platforms MPICH's mpirun doesn't pass
     the same argc, argv to all processors. However, the MPI spec says nothing about
     what it does or might do to argc or argv. In fact, there is no "const" in the
     function prototypes for either the pointers or the things they're pointing too.
     I would rather pass NULL here and the spec says this is perfectly acceptable.
     However, that too has caused MPICH to core on certain platforms.  */
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
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
			 STU_END_OF_ARGS);


  /* for convenience, set working directory to the test file directory unless file was specified on command line */
  if (!strlen(dbname))
    chdir(TEST_FILE_PATH);
#ifdef HAVE_PARALLEL
  MPI_Barrier(MPI_COMM_WORLD);
#endif

  saf_init(SAF_DEFAULT_LIBPROPS);

  if (!strlen(dbname))
    strcpy(dbname, "exo_par_wt.saf");

  {
    char l_str[256];
    sprintf(l_str,"setProps and open database %s",dbname);
    TESTING(l_str);
  }

  /* note: because we are in a try block here, all failures will send us to the one and only
     catch block at the end of this test */

  dbprops = saf_createProps_database();
  saf_setProps_ReadOnly(dbprops);
  db = saf_open_database(dbname,dbprops);

#ifdef SSLIB_SUPPORT_PENDING
  hbool_t multifile;
  /* determine if the database was generated in a multifile mode */
  saf_find_files(*db, SAF_ANY_NAME, &i, NULL);
  if (i > 0)
    multifile = true;
  else
    multifile = false;

  saf_self = SAF_SELF(*db);
#endif /*SSLIB_SUPPORT_PENDING*/


  PASSED;

  TESTING("finding all sets");

  all_sets = NULL;
  saf_find_matching_sets(SAF_ALL, db, SAF_ANY_NAME, SAF_ANY_SILROLE, SAF_ANY_TOPODIM, \
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

  /* printf("num top sets is %d\n",num_top_sets); */

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
#ifdef SSLIB_SUPPORT_PENDING

  TESTING("Read relational data");

  for( i = 0 ; i < num_all_sets; i++ ) {
      void *rel_data;
      SAF_Rel *sub_rel;
      SAF_Set sup_set; SAF_Set sub_set; SAF_Cat sup_cat; SAF_Cat sub_cat;
     
      for( j = 0 ; j < allsets[i].num_colls ; j++ )
         { sub_rel = allsets[i].collections[j].subset_relation;
           if( sub_rel ) 
              { printf("set: %s  cat: %s \n",allsets[i].set_name,allsets[i].collections[j].cat_name); 
               saf_describe_subset_relation(SAF_ALL,sub_rel,&sup_set,&sub_set,&sup_cat,&sub_cat,NULL, NULL, NULL, NULL);
               if (SS_SET(&sup_set)) printf("sup_set ");
               if (SS_SET(&sub_set)) printf("sub_set ");
               if (SS_CAT(&sup_cat)) printf("sup_cat ");
               if (SS_CAT(&sub_cat)) printf("sub_cat \n");
               rel_data = NULL;
               saf_read_subset_relation(SAF_ALL,sub_rel, NULL, (void **)&rel_data, NULL);
               free(rel_data);
              }
        }
  }
  PASSED;
#endif /*SSLIB_SUPPORT_PENDING*/

/*
           sub_rel = collection_info->subset_relation;
  Collection_Info *collection_info = NULL;
  int ii;
  SAF_Set saf_set;
  SAF_Cat saf_cat;
  static size_t abuf_sz = 0;
  static size_t bbuf_sz = 0;
  hid_t abuf_type, bbuf_type;
         saf_set = allsets[i].the_set;
         saf_cat = collection_info->the_cat;
               saf_get_count_and_type_for_subset_relation(SAF_ALL,sub_rel,NULL,&abuf_sz,&abuf_type,&bbuf_sz,&bbuf_type);
             for (ii=0;ii<abuf_sz;ii++)
               printf("rel_data[%d]=%d\n",ii,(int)&rel_data[ii]); 
*/

    

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
    float fvalue, evalue;
    int ivalue;
    char *fname;
    Set_Info *base_space, *subsets;
    float *index_value;
    int num_indexes, in, sn, fn,  dc, a_state, num_fields_in_state;
    int nc;
    int num_procs = 0;
    Collection_Info *procs_collection;

    for( i = 1 ; i < num_suites; i++ ) {
      suite_index = suites[i]->fields + 0;
      read_field( suite_index );
      num_indexes = suite_index->data_size;
	
      if( num_indexes > 0 ) {

	printf("\n\n\nSuite %s index values: ",suites[i]->set_name);
	for( in = 0 ; in < num_indexes ; in ++ ) {
	  index_value = *((float **)(suite_index->field_data));
	  printf("%.2f ",index_value[in]);
	    
	}

	for( sn = 1 ; sn < suites[i]->num_fields ; sn++ ) {


	  for( a_state = 0 ; a_state < num_indexes ; a_state++ ) {
	    state_field = suites[i]->fields + sn;
	    printf("\n\nSuite %s, State %s:%d contains fields:\n\n",suites[i]->set_name, state_field->field_name,a_state);
	    num_fields_in_state = state_field->template_info->num_comps;
	    read_state_field( state_field );



	    if( num_fields_in_state > 0 ) {
	      printf("\tBase Space\t\t:  Field Name\n");
	      printf("-----------------------------------------------------\n");
	    }

	    for( fn = 0 ; fn < num_fields_in_state ; fn++ ) { 
	      fields = (Field_Info **)(state_field->field_data) + (a_state * num_fields_in_state);
	      /* base_space = (Set_Info *)(fields[fn]->template_info->base_space); */
	      base_space = (Set_Info *)(fields[fn]->base_space);
	      printf("\t%-25s: %-30s", base_space->set_name, fields[fn]->field_name);
	     
	      if( strncmp("nodal_rot_in_blk", fields[fn]->field_name, 16) ) {

		read_field( fields[fn] );

		printf("size: %d\n",fields[fn]->data_size);

		fname = fields[fn]->field_name ;


		/* now that the field has been REMAPPED and READ, check read values against expected */

		if( !(strcmp(fname, "cent_on_top")) ) {
		  for( dc = 0 ; dc < fields[fn]->data_size; dc++ ) {
		    fvalue = ((float **)(fields[fn]->field_data))[0][dc] ;
		    if(  fvalue !=  cent_on_top_s0_buf[dc] ) {
		      printf("ERROR: %s index %d does not match expected value of %f\n",fname, dc, cent_on_top_s0_buf[dc]);
		    }
		  }
		}
	     
		if( !(strcmp(fname, "disp_on_top")) ) {
		  for( dc = 0 ; dc < fields[fn]->data_size; dc++ ) {
		    fvalue = ((float **)(fields[fn]->field_data))[0][dc] ;
		    if( a_state == 0 )
		      evalue = disp_on_top_s0_buf[dc];
		    else
		      evalue = disp_on_top_s1_buf[dc];
		    if(  fvalue !=  evalue ) {
		      printf("ERROR: %s index %d does not match expected value of %f\n",fname, dc, evalue);
		    }
		  }
		}
		if( !(strcmp(fname, "vel_on_top")) ) {
		  for( dc = 0 ; dc < fields[fn]->data_size; dc++ ) {
		    fvalue = ((float **)(fields[fn]->field_data))[0][dc] ;
		    if( a_state == 0 )
		      evalue = vel_on_top_s0_buf[dc];
		    else
		      evalue = vel_on_top_s1_buf[dc];
		    if(  fvalue !=  evalue ) {
		      printf("ERROR: %s index %d does not match expected value of %f\n",fname, dc, evalue);
		    }
		  }
		}
		if( !(strcmp(fname, "stress_on_blk2")) ) {
		  for( dc = 0 ; dc < fields[fn]->data_size; dc++ ) {
		    fvalue = ((float **)(fields[fn]->field_data))[0][dc] ;
		    if(  fvalue !=  stress_on_blk2_s0_buf[dc] ) {
		      printf("ERROR: %s index %d does not match expected value of %f\n",fname, dc, stress_on_blk2_s0_buf[dc]);
		    }
		  }
		}
		if( !(strcmp(fname, "stress_on_blk5")) ) {
		  for( dc = 0 ; dc < fields[fn]->data_size; dc++ ) {
		    fvalue = ((float **)(fields[fn]->field_data))[0][dc] ;
		    if(  fvalue !=  stress_on_blk5_s0_buf[dc] ) {
		      printf("ERROR: %s index %d does not match expected value of %f\n",fname, dc, stress_on_blk5_s0_buf[dc]);
		    }
		  }
		}
		if( !(strcmp(fname, "strain_on_blk1")) ) {
		  for( dc = 0 ; dc < fields[fn]->data_size; dc++ ) {
		    fvalue = ((float **)(fields[fn]->field_data))[0][dc] ;
		    if(  fvalue !=  strain_on_blk1_s0_buf[dc] ) {
		      printf("ERROR: %s index %d does not match expected value of %f\n",fname, dc, strain_on_blk1_s0_buf[dc]);
		    }
		  }
		}
		if( !(strcmp(fname, "strain_on_blk3")) ) {
		  for( dc = 0 ; dc < fields[fn]->data_size; dc++ ) {
		    fvalue = ((float **)(fields[fn]->field_data))[0][dc] ;
		    if(  fvalue !=  strain_on_blk3_s0_buf[dc] ) {
		      printf("ERROR: %s index %d does not match expected value of %f\n",fname, dc, strain_on_blk3_s0_buf[dc]);
		    }
		  }
		}
		if( !(strcmp(fname, "press_on_blk1")) ) {
		  for( dc = 0 ; dc < fields[fn]->data_size; dc++ ) {
		    fvalue = ((float **)(fields[fn]->field_data))[0][dc] ;
		    if(  fvalue !=  press_on_blk1_s0_buf[dc] ) {
		      printf("ERROR: %s index %d does not match expected value of %f\n",fname, dc, press_on_blk1_s0_buf[dc]);
		    }
		  }
		}
		if( !(strcmp(fname, "press_on_blk2")) ) {
		  for( dc = 0 ; dc < fields[fn]->data_size; dc++ ) {
		    fvalue = ((float **)(fields[fn]->field_data))[0][dc] ;
		    if(  fvalue !=  press_on_blk2_s0_buf[dc] ) {
		      printf("ERROR: %s index %d does not match expected value of %f\n",fname, dc, press_on_blk2_s0_buf[dc]);
		    }
		  }
		}
		if( !(strcmp(fname, "elem_ids_on_top")) ) {
		  for( dc = 0 ; dc < fields[fn]->data_size; dc++ ) {
		    ivalue = ((int **)(fields[fn]->field_data))[0][dc] ;
		    if(  ivalue !=  elem_ids_on_top_s0_buf[dc] ) {
		      printf("ERROR: %s index %d does not match expected value of %d\n",fname, dc, elem_ids_on_top_s0_buf[dc]);
		    }
		  }
		}

		/*
		  if( fields[fn]->data_type == SAF_FLOAT && ! strncmp("disp_", fields[fn]->field_name, 5) ) {

		  for( dc = 0; dc < fields[fn]->data_size; dc++ ) {
		  printf("%f, ",((float **)(fields[fn]->field_data))[0][dc]);
		  }
		   
		  }
		*/
		  


	      }
	      else
		printf("\n");
	     
 
	      for( nc = 0 ; nc < base_space->num_colls; nc++ ) {

		if( (base_space->collections[nc].celltype == SAF_CELLTYPE_SET) && \
		    (!strcmp(base_space->collections[nc].cat_name,"domains"))  ) {

		  procs_collection = &(base_space->collections[nc]);
		  subsets = (Set_Info *)(base_space->collections[nc].child_setlink);
		  num_procs = base_space->collections[nc].num_children;
		}
	      }

	      

	    }



	
	    /*{ 
	      Collection_Info *subset_collection;
	      void *rel_data;
	      SAF_Rel *sub_rel;
	      int subset_coll_num = 0, ns;

	      printf("\n\t%s elems are distributed across %d processor subsets\n",base_space->set_name, num_procs);
	      for( ns = 0; ns < num_procs; ns++ ) {
	      for( nc = 0 ; nc < subsets[ns].num_colls; nc++ ) {
	      if( !strcmp(subsets[ns].collections[nc].cat_name, "elems") ) {
	      subset_coll_num = nc;
	      subset_collection = subsets[ns].collections + nc;
	      }
	      }

	      sub_rel = subset_collection->subset_relation; 
	      rel_data = NULL; 
	      saf_read_subset_relation(SAF_ALL, *sub_rel, &rel_data, NULL); 
	      subset_collection->relation_data = rel_data;
	      printf("\t\t%s:%s has %d %s elems\n",subsets[ns].set_name, subset_collection->cat_name,
	      subset_collection->count,base_space->set_name );
	      
	      }
	      }*/



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


