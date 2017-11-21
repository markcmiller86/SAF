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
#include <stdio.h>

#ifdef HAVE_PARALLEL

#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>

/*	On ASCI Red, a specialized "stat", named "estat", was added to accommo-
 *	date file sizes upto 16GB. If "TFLOPS" is defined, this program will
 *	use the estat library instead of the standard stat library.   3/27/2002
 */

#ifdef TFLOPS
#include <sys/estat.h>
#else
#include <sys/stat.h>
#endif

#ifdef HAVE_EXODUS
#include "exodusII.h"
#endif

#ifndef FALSE
#define FALSE                  (0==1)
#endif

#ifndef TRUE
#define TRUE                   (1==1)
#endif

#define DEFAULT_NUM_FIELDS     0
#define DEFAULT_FILE_NAME      "mesh"
#define DEFAULT_NUM_ITERATIONS 1
#define EXODUS_FILE_TYPE       ".ex2"
#define FAILURE                FALSE
#define MBYTES                 (1024*1024)
#define MASTER_FILE_TYPE       ".saf"
#define MAX_STRING_LEN         128
#define NUM_BYTES_PER_INT      4
#define NUM_NODES_PER_ELEM     4
#define QUIT                   FALSE
#define SUCCESS                TRUE
#define SUPPLEMENTAL_FILE_TYPE ".ssf"
#define WRITE_FILE_TYPE        ".new"

/*
 *	Prototypes
 */

int parse_input(
	int	argc,
        char	*argv[],
	char	*device_name,
	int	*exodus,
	char	*file_name,
        int	*num_fields,
        int	*num_iterations,
        int	*saf,
        int	*supplemental
);

int read_saf_mesh(
	char	*device_name,
	int	num_domains,
	int	rank, 
        char	*file_name,
        int	num_fields,
	int	num_iterations,
        int	*glob_num_nodes,
	int	*loc_num_nodes,
	int	**node_rel,
        int	*glob_num_elems,
	int	*loc_num_elems,
	int	**elem_map,
        float	**x_coords,
	float	**y_coords,
	int	**loc_connect,
	int	supplemental
);

int write_saf_mesh(
	char	*device_name,
	int	num_domains,
	int	rank,
        char	*file_name,
	int	num_fields,
	int	num_iterations,
	int	glob_num_nodes,
	int	loc_num_nodes,
	int	*node_rel,
	int	glob_num_elems,
	int	loc_num_elems,
	int	*elem_map,
	float	*x_coords,
	float	*y_coords,
	int	*loc_connect,
	int	supplemental
);


#ifdef HAVE_EXODUS
int read_exo_mesh ( char *file_name, int rank, int num_fields, int num_iterations,
                    int *num_nodes, int **node_map,
                    int *num_elems, int **elem_map,
                    float **x_coords, float **y_coords, int **loc_connect );

int write_exo_mesh ( char *file_name, int rank, int num_fields, int num_iterations,
                     int loc_num_nodes, int *node_map,
                     int loc_num_elems, int *elem_map,
                     float *x_coords, float *y_coords, int *loc_connect );
#endif

/***********************************************************************
 *
 *  Main function
 * 
 ***********************************************************************/

int main( int argc, char **argv )
{
  int rank, num_domains;
  int quit=FALSE;
  int glob_num_nodes, loc_num_nodes, glob_num_elems, loc_num_elems;
  int *loc_connect;
  int *node_rel;

	char         device_name[MAX_STRING_LEN]; 				/* Location of SAF supplemental file.		*/
	MPI_Info     dsl_mpi_info_object;					/* Copy of DSL layer MPI Info object.		*/
	int         *elem_map;
	int          exodus =                     FALSE;			/* TRUE, perform EXODUS benchmark; FALSE	*/
										/* otherwise.					*/ 
	char         file_name[MAX_STRING_LEN] =  DEFAULT_FILE_NAME;		/* Input file name.				*/
	static const char *hints[] = {						/* List of MPI Info hints that if defined in	*/
			"cb_buffer_size",					/* the environment process 0, will be used to	*/
			"cb_nodes",						/* set key/value pairs in the DSL layer MPI	*/
			"ind_rd_buffer_size",					/* Info object.					*/
			"ind_wr_buffer_size",
			"cb_config_list",
			"romio_cb_read",
			"romio_cb_write",
			"romio_ds_read",
			"romio_ds_write",
			"romio_no_indep_rw"
		     };
	int          key;							/* MPI Info object key index.			*/
	int          key_exists;						/* TRUE, if the key exists in the MPI Info	*/
										/* object, FALSE otherwise. Should always be	*/
										/* TRUE in the current implementation.		*/
	char         key_name[MAX_STRING_LEN];					/* MPI Info object key name.			*/
	const int    nhints =                     10;                     	/* Number of items in hints list.		*/
	int          nkeys;							/* Number of keys in a MPI Info object.		*/
	int          num_fields =                 DEFAULT_NUM_FIELDS;
	int          num_iterations =             DEFAULT_NUM_ITERATIONS;
        int          saf =                        FALSE;			/* TRUE, perform SAF benchmark; FALSE		*/
										/* otherwise.					*/
        int          supplemental =               FALSE;			/* TRUE, use single SAF supplemental file for	*/
										/* benchmark; FALSE otherwise.			*/
	char         value[MAX_STRING_LEN];					/* Value of a key/value pair in a MPI Info	*/
										/* object.					*/
	float       *x_coords;
	float       *y_coords;
        MPI_Info    new_mpi_info_object;	

	/*
         *	Initialize Stuff
         */

	MPI_Init     ( &argc, &argv                 );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank        );
	MPI_Comm_size( MPI_COMM_WORLD, &num_domains );
	
        /*
         *	Processor 0: parse the command line arguments.
         */

	if ( rank == 0 ) {
		quit = !parse_input(
				argc,
				argv,
				device_name,
				&exodus,
				file_name,
				&num_fields,
				&num_iterations,
				&saf,
				&supplemental
			);
	}


	/*
         *	Broadcast Input
         */
         
        MPI_Bcast ( &quit,         	1,              MPI_INT,  0, MPI_COMM_WORLD );

	if (quit) {
    		MPI_Finalize();

		exit(SUCCESS);
	}

	MPI_Bcast( device_name,		MAX_STRING_LEN, MPI_CHAR, 0, MPI_COMM_WORLD );
	MPI_Bcast( &exodus,		1,              MPI_INT,  0, MPI_COMM_WORLD );
	MPI_Bcast( file_name,		MAX_STRING_LEN, MPI_CHAR, 0, MPI_COMM_WORLD );
	MPI_Bcast( &num_fields,		1,              MPI_INT,  0, MPI_COMM_WORLD );
	MPI_Bcast( &num_iterations,	1,              MPI_INT,  0, MPI_COMM_WORLD );
	MPI_Bcast( &saf,		1,              MPI_INT,  0, MPI_COMM_WORLD );
	MPI_Bcast( &supplemental,	1,              MPI_INT,  0, MPI_COMM_WORLD );


	/* DSL_env_mpi_hints( nhints, hints ); */
        {
        char            *env;                           /* Contents of environmental variable.  */
        int              hint;                          /* ROMIO hint index.                    */
        char             hint_value[MAX_STRING_LEN];    /* ROMIO hint value.                    */
        int              rank;                          /* MPI process rank.                    */

                                                                                                                                                                                                                                                                               
        MPI_Comm_rank( MPI_COMM_WORLD, &rank );
                                                                                                                                                                                                                                                                               
        /*      The "value" of the hint is obtained from the environment of
         *      processor 0 only. The value is broadcast to the other
         *      processors.
         */
                                                                                                                                                                                                                                                                               
            for ( hint = 0; hint < nhints; hint++ ) {
                if ( rank == 0 ) {
                        env = getenv( hints[hint] );
                                                                                                                                                                                                                                                                               
                        if ( env != NULL )
                                strcpy( hint_value, env );
                        else
                                hint_value[0] = 0;
                }
                                                                                                                                                                                                                                                                               
                MPI_Bcast( hint_value, MAX_STRING_LEN, MPI_CHAR, 0, MPI_COMM_WORLD );
                                                                                                                                                                                                                                                                               
                if ( hint_value[0] ) {
                        if ( dsl_mpi_info_object == MPI_INFO_NULL )
                                MPI_Info_create( &dsl_mpi_info_object );
                                                                                                                                                                                                                                                                               
                        MPI_Info_set( dsl_mpi_info_object, hints[hint], hint_value );
                }
           }
        }

	if ( rank == 0 ) {
		printf( "\nSAF 2-D Benchmark\n\n"                       );
        	printf( "   Number of fields    \t%d\n", num_fields     );
        	printf( "   Number of Iterations\t%d\n", num_iterations );
                 
                if ( dsl_mpi_info_object != MPI_INFO_NULL )
                        MPI_Info_dup( dsl_mpi_info_object, &new_mpi_info_object );
                else
                        new_mpi_info_object = MPI_INFO_NULL;

		dsl_mpi_info_object = new_mpi_info_object;


		if ( dsl_mpi_info_object != MPI_INFO_NULL ) {
			printf( "   MPI Hint Status\n" );

			MPI_Info_get_nkeys( dsl_mpi_info_object, &nkeys );

			for ( key = 0; key < nkeys; key++ ) {
				MPI_Info_get_nthkey( dsl_mpi_info_object, key, key_name                                );
				MPI_Info_get       ( dsl_mpi_info_object, key_name, MAX_STRING_LEN, value, &key_exists );

				printf( "      %s\t\t\t%s\n", key_name, value );
			}

			MPI_Info_free( &dsl_mpi_info_object );
		}
		else
			printf( "   MPI Hint Status\tMPI_INFO_NULL\n" );     		          
	}

	if ( exodus ) {
#ifdef HAVE_EXODUS
	int         *node_map;
		if (	read_exo_mesh(
				file_name,
				rank,
				num_fields,
				num_iterations,
                   		&loc_num_nodes,
				&node_map,
                   		&loc_num_elems,
				&elem_map,
                   		&x_coords,
				&y_coords,
				&loc_connect
			)
		) {
			write_exo_mesh(
				file_name,
				rank,
				num_fields,
				num_iterations,
                    		loc_num_nodes,
				node_map, 
				loc_num_elems,
				elem_map, 
                    		x_coords,
				y_coords,
				loc_connect
			);
			
			if ( saf ) {
				free( elem_map    );
				free( loc_connect );
				free( node_map    );
				free( x_coords    );
				free( y_coords    );
			}
		}
#else
		printf( "rd_wt_mesh not linked with EXODUS libraries\n" );
#endif
	}

	/*
         *	SAF Benchmark
         */

	if ( saf ) {
        	if (	read_saf_mesh(
				device_name,
				num_domains,
                		rank,
                		file_name,
                		num_fields,
                		num_iterations,
                		&glob_num_nodes,
                		&loc_num_nodes,
                		&node_rel,
                		&glob_num_elems,
                		&loc_num_elems,
                		&elem_map,
                		&x_coords,
                		&y_coords,
                		&loc_connect,
				supplemental
			)
		) {

		/*
		 *	Write Mesh
		 */

			write_saf_mesh(
				device_name,
                		num_domains,
                        	rank, 
                        	file_name,
                        	num_fields,
                        	num_iterations,
				glob_num_nodes,
                        	loc_num_nodes,
                        	node_rel,
				glob_num_elems,
                        	loc_num_elems,
                        	elem_map,
				x_coords,
                        	y_coords,
                        	loc_connect,
				supplemental
			);
		}
	}

	/* DSL_set_mpi_hints( MPI_INFO_NULL ); */
        {
        if ( dsl_mpi_info_object != MPI_INFO_NULL )
                MPI_Info_free( &dsl_mpi_info_object );
                                                                                
        if ( new_mpi_info_object != MPI_INFO_NULL )
                MPI_Info_dup( new_mpi_info_object, &dsl_mpi_info_object );
        else
                dsl_mpi_info_object = MPI_INFO_NULL;
}


	MPI_Finalize();

  return(0);
}

/***********************************************************************
 *
 *  Parse Input
 * 
 ***********************************************************************/

int parse_input (
	int	argc,
        char	*argv[],
	char	*device_name,
	int	*exodus,
	char	*file_name,
        int	*num_fields,
        int	*num_iterations,
        int	*saf,
        int	*supplemental
) {
	int arg = 0;	/* Argument index.	*/
        
        while ( ++arg < argc ) {
        	if ( strcmp( "-S", argv[arg] ) == 0 ) { 
               		*saf =	        TRUE;
			*supplemental =	TRUE;
                }
        	else if ( strcmp( "-c", argv[arg] ) == 0 ) {
                	if ( ++arg < argc ) *num_fields = atoi( argv[arg] );
		}
        	else if ( strcmp( "-f", argv[arg] ) == 0 ) { 
                	if ( ++arg < argc ) strcpy( file_name, argv[arg] );
                }
		else if ( strcmp( "-i", argv[arg] ) == 0 ) { 
			if ( ++arg < argc ) *num_iterations = atoi( argv[arg] );
                }
        	else if ( strcmp( "-s", argv[arg] ) == 0 ) { 
			*saf = TRUE;
                }
		else if ( strcmp( "-x", argv[arg] ) == 0 ) {
			*exodus = TRUE;
		}
        	else if ( strcmp( "-v", argv[arg] ) == 0 ) { 
                	if ( ++arg < argc ) strcpy( device_name, argv[arg] );
                }
                else if ( (strcmp( "-h", argv[arg] ) == 0) || (strcmp( "-u", argv[arg] ) == 0) ) {
                	printf( "                                                                \n" );
                        printf( "NAME                                                            \n" );
                        printf( "                                                                \n" );
                        printf( "rd_wt_mesh - reads and writes a mesh in parallel for performance\n" );
                        printf( "             benchmarking.                                      \n" );
                	printf( "                                                                \n" );
                        printf( "SYNOPSIS                                                        \n" );
                        printf( "                                                                \n" );
                	printf( "rd_wt_mesh [-S] [-c fields] [-f file_name] [-h] [-i iterations] \n" );
                        printf( "           [-s] [-u] [-x]                                       \n" );
                	printf( "                                                                \n" );
                        printf( "DESCRIPTION                                                     \n" );
                        printf( "                                                                \n" );
                	printf( "This program reads and writes a mesh in parallel for performance\n" );
                        printf( "benchmarking. The first SAF and/or Exodus database file read by \n" );
			printf( "this program is created by create_mesh. Performance summaries   \n" );
			printf( "are written to stdout.                                          \n" );
                        printf( "                                                                \n" );
                        printf( "OPTIONS                                                         \n" );
                        printf( "                                                                \n" );
 			printf( "-S             read and write a single SAF supplemental file.   \n" );
			printf( "               The format of the name of the supplemental file  \n" );
			printf( "               read in is:                                      \n" );
                        printf( "                                                                \n" );
                        printf( "                                'file_name'%s                   \n", SUPPLEMENTAL_FILE_TYPE );
			printf( "                                                                \n" );
			printf( "               The format of the name of the supplemental file  \n" );
			printf( "               written out is the same with a %s extension. The \n", WRITE_FILE_TYPE        );
                        printf( "               '-S' option implies the '-s' option.             \n" );
                        printf( "-c fields      number of fields. Default: %d                    \n", DEFAULT_NUM_FIELDS     );
			printf( "-f file_name   file name prefix for all read files:             \n" );
			printf( "                                                                \n" );
                        printf( "               'file_name'_n%s [EXODUS II file]                 \n", EXODUS_FILE_TYPE       );
			printf( "               'file_name'%s   [SAF master file]                \n", MASTER_FILE_TYPE       );
			printf( "               'file_name'%s   [single SAF supplemental file]   \n", SUPPLEMENTAL_FILE_TYPE );
			printf( "                                                                \n" );
			printf( "               where n varies from 0 to number of domains - 1.  \n" );
			printf( "               Default: %s                                      \n", DEFAULT_FILE_NAME      );
                        printf( "-h             display help/usage information                   \n" );
                        printf( "-i iterations  number of iterations or SAF Read/write cycles.  \n" );
                        printf( "               Default: %d                                      \n", DEFAULT_NUM_ITERATIONS );
                        printf( "-s             perform SAF benchmark.                           \n" );
			printf( "-x             perform EXODUS II benchmark.                     \n" );
                        printf( "-u             display help/usage information                   \n" );
			printf( "-v device      location to read and write the SAF supplemental  \n" );
			printf( "		file. If not specified, this location is the	 \n" );
			printf( "		same as the SAF master file.                     \n" );    
                        
                        return( QUIT );
                }
                else {
			printf( "Unknown option: %s\n", argv[arg]                         );
                        printf( "Enter rd_wt_mesh -h for description of valid options.\n" );
                        
			return( FAILURE );
		}
        }
	
        return( SUCCESS );
}

/***********************************************************************
 *
 *  Read saf mesh
 * 
 ***********************************************************************/

int read_saf_mesh(
	char	*device_name,
	int	num_domains,
	int	rank, 
        char	*file_name,
        int	num_fields,
	int	num_iterations, 
        int	*glob_num_nodes,
	int	*loc_num_nodes,
	int	**node_rel,
        int	*glob_num_elems,
	int	*loc_num_elems,
	int	**elem_map,
	float	**x_coords,
	float	**y_coords,
	int	**loc_connect,
	int	supplemental
) {
  int i, len_connect, iter;
  int num_sets, num_cats, num_colls, num_rels, num_flds, num_comps;
  int *num_nodes_per_elem;
	
        double file_size;
	double glob_raw_data_vol;
        double raw_data_vol;
 
  char domain_set_name[MAX_STRING_LEN];
  char fld_name[MAX_STRING_LEN];

  SAF_LibProps *lib_p;

  SAF_Db *db=NULL;
  SAF_DbProps *p=NULL;

  SAF_Cat *domain_cats;
  SAF_Cat *node_cats;
  SAF_Cat *elem_cats;

  SAF_Set *top_sets;
  SAF_Set *domain_set;

  SAF_Rel *rels;

  SAF_Field *coord_fld;
  SAF_Field *comp_flds;

	double		cum_dt_db_close;			/* Cumulative database close time (sec).		*/
        double		cum_dt_db_open;				/* Cumulative database open time (sec).			*/
	double		cum_dt_lib_init;			/* Cumulative library initialization time (sec).	*/
	double		cum_dt_mesh_read;			/* Cumulative overall SAF Read time (sec).		*/
        double		cum_dt_other;				/* Cumulative other time: cumulative SAF Read time	*/
        							/* minus cumulative database close, database open,	*/
                                        			/* library initialization and raw data read times	*/
                                        			/* (sec).						*/
        double		cum_dt_raw_data;			/* Cumulative raw data read time (sec).			*/
        
	double		dt_db_close;				/* Database close time (sec).				*/
        double		dt_db_open;				/* Database open time (sec).				*/
	double		dt_elem_rel;				/* Element subset relation read time (sec).		*/
	double		dt_lib_init;				/* Library initialization time (sec).			*/
	double		dt_mesh_read;				/* Overall SAF Read time (sec).			*/
        double		dt_node_rel;				/* Node subset relation read time (sec).		*/
	double  	dt_raw_data;				/* Raw data read time (sec).				*/
	double		dt_results;				/* Result simulation read time (sec).			*/
        double		dt_topo_rel;				/* Topological relations read time (sec).		*/
        double		dt_x_field;				/* X-coordinate field read time (sec).			*/
        double		dt_y_field;				/* Y-coordinate field read time (sec).			*/

/*	On ASCI Red, a specialized "stat", named "estat", was added to
 *	accommodate file sizes upto 16GB.                    3/27/2002
 */

#ifdef TFLOPS
	struct estat file_status;				/* Arbitrary file status on ASCI Red.                   */
#else
	struct stat  file_status;                               /* Arbitrary file status.                               */
#endif
    
        int          first;					/* TRUE, for first iteration; FALSE for all subsequent  */
        							/* iterations.		                                */

	char         master_file_name[MAX_STRING_LEN];		/* SAF master file name.				*/					

	double       max_dt_db_close;				/* Maximum database close time (sec).			*/
        double       max_dt_db_open;				/* Maximum database open time (sec).			*/
	double       max_dt_lib_init;				/* Maximum library initialization time (sec).		*/
	double       max_dt_mesh_read;				/* Maximum overall SAF Read time (sec).		*/
        double       max_dt_raw_data;				/* Maximum raw data read (sec.)				*/

	double       min_dt_db_close;				/* Minimum database close time (sec).			*/
        double       min_dt_db_open;				/* Minimum database open time (sec).			*/
	double       min_dt_lib_init;				/* Minimum library initialization time (sec).		*/
	double       min_dt_mesh_read;				/* Minimum overall SAF Read time (sec).		*/
        double       min_dt_raw_data;				/* minimum raw data write time (sec).			*/

	char         supplemental_file_name[MAX_STRING_LEN];	/* SAF supplemental file name.				*/					
	
	double       t_finish;					/* Arbitrary finish time (sec).				*/
	double       t_start;					/* Arbitrary start time (sec).				*/
	double       t0;					/* Starting wall-clock time (sec).			*/

	sprintf( master_file_name, "%s%s", file_name, MASTER_FILE_TYPE );

	if ( supplemental ) sprintf( supplemental_file_name, "%s%s%s", device_name, file_name, SUPPLEMENTAL_FILE_TYPE );

	first =	TRUE;

	for ( iter = 0; iter < num_iterations; iter++ ) {
		raw_data_vol =	0.;
		t0 =		MPI_Wtime();

		/*
		 *	Start SAF Session and Open Database
		 */

		lib_p = saf_createProps_lib();

	        saf_setProps_ErrorMode( lib_p, SAF_ERRMODE_RETURN );

		t_start = MPI_Wtime();
		
		if ( saf_init( lib_p ) != SAF_SUCCESS ) {
			if ( rank == 0 )
				fprintf( stderr, "SAF Read (saf_init): unable to initialize SAF library.\n" );
                
			return( FAILURE );
		}

		t_finish =	MPI_Wtime();
		dt_lib_init =	t_finish - t_start;

		p = saf_createProps_database();

		if ( saf_setProps_ReadOnly( p ) != SAF_SUCCESS ) {
			if ( rank == 0 )
				fprintf( stderr, "SAF Read (saf_setProps_ReadOnly): unable to set read-only property.\n" );
                
			return( FALSE );
		}

		t_start = MPI_Wtime();

		db = saf_open_database( master_file_name, p );

		t_finish =	MPI_Wtime();
		dt_db_open =	t_finish - t_start;

		/*
		 *	Find Top Set
		 *
		 *	Currently, the SAF database generated by `create_mesh' contains
		 *	only one top set.
		 */
         
		top_sets = NULL;

		if (	saf_find_matching_sets(
				SAF_ALL,
				db,
				SAF_ANY_NAME,
				SAF_ANY_SILROLE,
				SAF_ANY_TOPODIM, 
				SAF_EXTENDIBLE_TORF,
				SAF_TOP_TRUE, 
				&num_sets,
				&top_sets
			) != SAF_SUCCESS ) {
			if ( rank == 0 )
	                	fprintf( stderr, "SAF Read (saf_find_matching_sets): unable to find top set.\n" );
                
		        return( FAILURE );
		}
                
		if ( num_sets != 1 ) {
			if ( rank == 0 )
				fprintf( stderr, "SAF Read: invalid number of top sets; value must be 1.\n" );
                        
			return( FAILURE );
		}

		free( coord_fld ); 

		coord_fld = NULL;

		sprintf( fld_name, "XY_COORDS_%d", rank );

		saf_find_fields(
			SAF_EACH,db,
			SAF_UNIVERSE(db),
			fld_name,
			SAF_ANY_QUANTITY,
			SAF_ALGTYPE_ANY,
			SAF_ANY_BASIS,
			SAF_ANY_UNIT,
			SAF_ANY_CAT,
			SAF_ANY_RATIO,
			SAF_ANY_CAT,
			SAF_ANY_EFUNC,
			&num_flds, 
			&coord_fld
		);

		if ( num_flds != 1 ) {
			if ( rank == 0 )
				fprintf( stderr, "SAF Read: number of fields is %d; should be 1.\n", num_flds );
                        
			return( FAILURE );
		}

		/*
         	 *	Make sure that the number of domain sets is equal to the number
		 *	of processors. First, find the SAF_PROCESSOR collection
		 *	category.
		 */

		domain_cats = NULL;
      
		if (	saf_find_categories(SAF_ALL,db,
				top_sets,
				SAF_ANY_NAME,
				SAF_PROCESSOR, 
				SAF_ANY_TOPODIM,
				&num_cats,
				&domain_cats
			) != SAF_SUCCESS ) {
			if ( rank == 0 )
	                	fprintf( stderr, "SAF Read (saf_find_categories): unable to find SAF_PROCESSOR collection.\n" );
                
	                return( FAILURE );
	        }
  

		if ( num_cats != 1 ) {
			if ( rank == 0 )
				fprintf( stderr, "SAF Read: Number of domain categories is %d; should be 1.\n", num_cats );
                        
			return( FAILURE );
		}

		/* Now find the number of subsets (domain sets) of the top set by the 
		SAF_PROCESSOR collection category */ 

		saf_find_sets (SAF_ALL,SAF_FSETS_SUBS, top_sets, domain_cats, 
                     &num_sets, NULL);

		free( domain_cats );
  
		if ( num_sets != num_domains ) {
			if ( rank == 0 )
				fprintf( stderr, "SAF Read: number of domain sets (%d) does not equal number od processors (%d).\n\n", num_sets, num_domains );
 	
        		return( FAILURE );
		}
  
		/* Read global info: number of nodes and elements, etc. */
  
		/* Find the node collection and inquire its size;
			this is the global number of nodes */
  
		node_cats = NULL;

		saf_find_collections (SAF_ALL, top_sets, SAF_ANY_ROLE, 
                            SAF_CELLTYPE_ANY, 0, SAF_DECOMP_TORF, &num_colls, 
                            &node_cats);

		if ( num_colls != 1 ) {
			if ( rank == 0 )
				fprintf( stderr, "SAF Read: number of node collections is %d; should be 1.\n", num_colls );
                        
			return( FAILURE );
		}

		saf_describe_collection (SAF_ALL, top_sets, node_cats, NULL,
                               glob_num_nodes, NULL, NULL, NULL);
  
		/* Find the element collection and inquire its size;
		this is the global number of elements */
  
		elem_cats = NULL;

		saf_find_collections (SAF_ALL, top_sets, SAF_TOPOLOGY, 
                            SAF_CELLTYPE_ANY, 2, SAF_DECOMP_TORF, &num_colls, 
                            &elem_cats);
 
		if ( num_colls != 1 ) {
			if ( rank == 0 )
				fprintf( stderr, "SAF Read: number of element collections is %d; should be 1.\n", num_colls );
        
			return( FAILURE );                
		}
  
		saf_describe_collection (SAF_ALL, top_sets, elem_cats, NULL,
                               glob_num_elems, NULL, NULL, NULL);
  
		/* For each domain set:
		read local number of nodes and elements
           	read the node local/global map
		read the element local/global map
		read the local topology relations
		read the coordinate fields */
  
		/* I already know the name of each domain (cause I wrote the file);
		how would I do this if I didn't know the domain set name?? */
  
 		sprintf (domain_set_name, "DOMAIN_SET_%d", rank);
  
		domain_set = NULL;
  
		saf_find_matching_sets (SAF_EACH, db, domain_set_name,
                              SAF_ANY_SILROLE, SAF_ANY_TOPODIM, 
                              SAF_EXTENDIBLE_TORF,
                              SAF_TOP_TORF, &num_sets, &domain_set);
  
		if ( num_sets != 1 ) {
			if ( rank == 0 )
				fprintf( stderr, "SAF Read: number of domain sets named %s is %d; should be 1.\n", domain_set_name, num_sets );
                
			return( FAILURE );
		}
  
		/* Find the node collection on the domain set and inquire its size;
		this is the local number of nodes */
 
		free(node_cats); 
		node_cats = NULL;
		saf_find_collections (SAF_EACH, domain_set, SAF_ANY_ROLE, 
                            SAF_CELLTYPE_POINT, 0, SAF_DECOMP_TORF, &num_colls, 
                            &node_cats);

		if ( num_colls != 1 ) {
			if ( rank == 0 )
				fprintf( stderr, "SAF Read: number of node collections is %d; should be 1.\n", num_colls );
                        
			return( FAILURE );
		}
  
		saf_describe_collection (SAF_EACH, domain_set, node_cats, NULL,
                               loc_num_nodes, NULL, NULL, NULL);
  
		/* Find the element collection and inquire its size;
		this is the global number of elements */
 
		free(elem_cats); 
		elem_cats = NULL;
		saf_find_collections (SAF_EACH, domain_set, SAF_ANY_ROLE, 
                            SAF_CELLTYPE_QUAD, 2, SAF_DECOMP_TORF, &num_colls, 
                            &elem_cats);

		if ( num_colls != 1 ) {
			if ( rank == 0 )
				fprintf( stderr, "SAF Read: number of element collections is %d; should be 1.\n", num_colls );
                        
                	return( FAILURE );
       		}
  
		saf_describe_collection (SAF_EACH, domain_set, elem_cats, NULL,
                               loc_num_elems, NULL, NULL, NULL);
  
		/* Read the node local/global map */
  
		rels = NULL;
      
		saf_find_subset_relations(
			SAF_EACH,db,
			top_sets,
			domain_set,
			SAF_COMMON( node_cats ),
			&num_rels,
			&rels
		);
        
		free( node_cats );

		if ( num_rels != 1 ) {
			if ( rank == 0 )
				fprintf( stderr, "SAF Read: number of subset relations is %d; should be 1.\n", num_rels );
                        
                	return( FAILURE );
		}
  
		if (first) *node_rel = (int *) malloc (*loc_num_nodes * sizeof(int));
        
		t_start = MPI_Wtime();
  
		saf_read_subset_relation( SAF_EACH, rels, NULL, (void**) node_rel, NULL );

		t_finish =	MPI_Wtime();
		dt_node_rel =	t_finish - t_start;
		raw_data_vol +=	(double)(*loc_num_nodes * sizeof(int));
  
		/* Read the element local/global map */
 
		free( rels ); 

		rels = NULL;

		saf_find_subset_relations(
			SAF_EACH,db,
			top_sets,
			domain_set,
			SAF_COMMON( elem_cats ),
			&num_rels,
			&rels
		);
        
		free( elem_cats );
		free( top_sets  );
  
		if ( num_rels != 1 ) {
			if ( rank == 0 )
				fprintf( stderr, "SAF Read: number of subset relations is %d; should be 1.\n", num_rels );
                        
			return( FAILURE );
		}
  
		if (first) *elem_map = (int *) malloc (*loc_num_elems * sizeof(int));

 		t_start = MPI_Wtime();
  
		saf_read_subset_relation( SAF_EACH, rels, NULL, (void**) elem_map, NULL );

		t_finish =	MPI_Wtime();
		dt_elem_rel =	t_finish - t_start;
		raw_data_vol +=	(double)(*loc_num_elems * sizeof(int));
  
		/* Read the local topology relations */
 
		free(rels); 
		rels = NULL;
		saf_find_topo_relations (SAF_EACH,db,domain_set,NULL,&num_rels,
                               &rels);
  
		if ( num_rels != 1 ) {
			if ( rank == 0 )
				fprintf( stderr, "SAF Read: number of topological relations is %d; should be 1.\n", num_rels );
                        
			return( FAILURE );
		}
  
		len_connect = NUM_NODES_PER_ELEM * (*loc_num_elems);

		if (first) {
			*loc_connect =		(int *) malloc (len_connect * sizeof(int));
			num_nodes_per_elem =	(int *) malloc (sizeof(int));
		}

		t_start = MPI_Wtime();
  
		saf_read_topo_relation(
	        	SAF_EACH,
	                rels, NULL, 
	                (void**)&num_nodes_per_elem,
	                (void**)loc_connect
	        );

		t_finish =	MPI_Wtime();
		dt_topo_rel =	t_finish - t_start;
		raw_data_vol += (double)len_connect * sizeof(int);

		free( rels );
  
		if ( num_nodes_per_elem[0] != NUM_NODES_PER_ELEM ) {
			if ( rank == 0 )
				fprintf( stderr, "SAF Read: num_nodes_per_elem is %d; should be %d.\n", num_nodes_per_elem[0], NUM_NODES_PER_ELEM );
                        
			return( FAILURE );
		}
  
		if ( first ) {
		        *x_coords = (float *) malloc (*loc_num_nodes * sizeof(float));
		        *y_coords = (float *) malloc (*loc_num_nodes * sizeof(float));
		}

		comp_flds = NULL;
  
		saf_describe_field (SAF_EACH, coord_fld, NULL, NULL, NULL, 
                          NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
                          &num_comps, &comp_flds, NULL, NULL);

		if ( num_comps != 2 ) {
			if ( rank == 0 )
				fprintf( stderr, "SAF Read: number of coordinate component fields is %d; should be 2.\n", num_comps );
                        
			return( FAILURE );
		}

		t_start = MPI_Wtime();
  
		saf_read_field(
        		SAF_EACH,
                	comp_flds,NULL,
                	SAF_WHOLE_FIELD,
                	(void**)x_coords
		);
 
		t_finish =	MPI_Wtime();
		dt_x_field = 	t_finish - t_start;
 		raw_data_vol +=	(double)((*loc_num_nodes) * sizeof(float));

		t_start = MPI_Wtime();
  
		saf_read_field(
        		SAF_EACH,
        		&comp_flds[1],NULL,
                	SAF_WHOLE_FIELD,
                	(void**)y_coords
		);

		t_finish =	MPI_Wtime();
		dt_y_field =	t_finish - t_start;
		raw_data_vol +=	(double)((*loc_num_nodes) * sizeof(float));
  
		free( comp_flds );
  
		/*
		 * To simulate reading in results variables, read the x coordinates
		 * in "num_fields" times.
		 */

		dt_results = 0.;
                  
		for ( i = 0; i < num_fields; i++ ) {
			free( coord_fld );

			coord_fld = NULL;

			sprintf( fld_name, "FIELD_%d_%d", rank, i );

			saf_find_fields(
	                	SAF_EACH,db,
	                        SAF_UNIVERSE(db),
	                        fld_name,
				SAF_ANY_QUANTITY,
	                        SAF_ALGTYPE_ANY,
	                        SAF_ANY_BASIS,
				SAF_ANY_UNIT,
	                        SAF_ANY_CAT,
	                        SAF_ANY_RATIO,
				SAF_ANY_CAT,
	                        SAF_ANY_EFUNC,
	                        &num_flds, 
				&coord_fld
			);

			if ( num_flds != 1 ) {
				if ( rank == 0 )
					fprintf( stderr, "SAF Read: couldn't find field %s\n", fld_name );
                        
				return( FAILURE );
			}

			t_start = MPI_Wtime();
  
			saf_read_field(
        			SAF_EACH,
        			coord_fld,NULL,
                		SAF_WHOLE_FIELD,
                		(void**)x_coords
        		);
  
			t_finish =	MPI_Wtime();
			dt_results +=	(t_finish - t_start);
			raw_data_vol +=	(double)(*loc_num_nodes * sizeof(float));
		}
	
		free( coord_fld );
  
		/*
	         *	Close Database and Library
	         */

		t_start = MPI_Wtime();
         
		saf_close_database( db );

		t_finish =	MPI_Wtime();
		dt_db_close =	t_finish - t_start;
  
	  	saf_final();

		t_finish =	MPI_Wtime();
		dt_mesh_read =	t_finish - t0;
	
		/*
	         *	Perform Timings
	         */

		dt_raw_data = dt_node_rel + dt_elem_rel + dt_topo_rel + dt_x_field + dt_y_field + dt_results;
                
		if ( first ) {
			cum_dt_db_close	=	dt_db_close;
			cum_dt_db_open =	dt_db_open;
			cum_dt_lib_init =	dt_lib_init;	
			cum_dt_mesh_read =	dt_mesh_read;	
			cum_dt_raw_data =	dt_raw_data;
        
			max_dt_db_close	=	dt_db_close;
			max_dt_db_open =	dt_db_open;
			max_dt_lib_init =	dt_lib_init;	
			max_dt_mesh_read =	dt_mesh_read;	
			max_dt_raw_data =	dt_raw_data;

			min_dt_db_close	=	dt_db_close;
			min_dt_db_open =	dt_db_open;
			min_dt_lib_init =	dt_lib_init;	
			min_dt_mesh_read =	dt_mesh_read;	
			min_dt_raw_data =	dt_raw_data;

			first =			FALSE;
               	}
                else {
			cum_dt_db_close	+=	dt_db_close;
			cum_dt_db_open +=	dt_db_open;
			cum_dt_lib_init +=	dt_lib_init;	
			cum_dt_mesh_read +=	dt_mesh_read;	
			cum_dt_raw_data +=	dt_raw_data;
        
			if ( max_dt_db_close	<	dt_db_close	) max_dt_db_close =	dt_db_close;
			if ( max_dt_db_open 	<	dt_db_open	) max_dt_db_open =	dt_db_open;
			if ( max_dt_lib_init	<	dt_lib_init	) max_dt_lib_init =	dt_lib_init;	
			if ( max_dt_mesh_read	<	dt_mesh_read	) max_dt_mesh_read =	dt_mesh_read;	
			if ( max_dt_raw_data	<	dt_raw_data	) max_dt_raw_data =	dt_raw_data;
        
			if ( min_dt_db_close	>	dt_db_close	) min_dt_db_close =	dt_db_close;
			if ( min_dt_db_open 	>	dt_db_open	) min_dt_db_open =	dt_db_open;
			if ( min_dt_lib_init	>	dt_lib_init	) min_dt_lib_init =	dt_lib_init;	
			if ( min_dt_mesh_read	>	dt_mesh_read	) min_dt_mesh_read =	dt_mesh_read;	
			if ( min_dt_raw_data	>	dt_raw_data	) min_dt_raw_data =	dt_raw_data;
               }
	}  /* end of for (iter...) */

	MPI_Allreduce(
        	&raw_data_vol,
                &glob_raw_data_vol,
                1,
                MPI_DOUBLE,
                MPI_SUM,
                MPI_COMM_WORLD
	);

/*
 *	Get File Sizes
 *
 *	Note: On ASCI Red, a specialized "stat", named "estat", was added to
 *	accommodate file sizes upto 16GB.                          3/27/2002
 */

#ifdef TFLOPS
	if ( estat( master_file_name, &file_status ) ) {
#else
	if (  stat( master_file_name, &file_status ) ) {
#endif
        	if ( rank == 0 ) 
			fprintf ( stderr, "SAF Read: cannot get %s file size.\n", master_file_name );
  
		return( FAILURE );
	}
        else
        	file_size = (double)file_status.st_size;

	if ( supplemental ) {
#ifdef TFLOPS
        	if ( estat( supplemental_file_name, &file_status ) ) {
#else
        	if (  stat( supplemental_file_name, &file_status ) ) {
#endif
        		if ( rank == 0 ) 
				fprintf ( stderr, "SAF Read: cannot get %s file size.\n", supplemental_file_name );
                        
			return( FAILURE );
		}
        	else
        		file_size += (double)file_status.st_size;
        }
        
        if ( rank == 0 ) {
		cum_dt_other =	cum_dt_mesh_read - cum_dt_lib_init - cum_dt_db_open - cum_dt_db_close - cum_dt_raw_data;

                printf( "                                                                \n" );
                printf( "   SAF Read Results                                             \n" );
                printf( "                                                                \n" );
		printf( "      Sizes (bytes)                                             \n" );
                printf( "         File         %16.12g                                   \n", file_size                     );
                printf( "         Raw Data     %16.12g                                   \n", glob_raw_data_vol             );
                printf( "         Difference   %16.12g                                   \n", file_size - glob_raw_data_vol );
                printf( "                                                                \n" );
		printf( "      Times (sec)                 \t Minimum\t Maximum\t Average\n" );
                printf( "         Library Initialization   \t%8.4g\t%8.4g\t%8.4g         \n", min_dt_lib_init,  max_dt_lib_init,  cum_dt_lib_init  / num_iterations );
                printf( "         Database Open            \t%8.4g\t%8.4g\t%8.4g         \n", min_dt_db_open,   max_dt_db_open,   cum_dt_db_open   / num_iterations );
                printf( "         Database Close           \t%8.4g\t%8.4g\t%8.4g         \n", min_dt_db_close,  max_dt_db_close,  cum_dt_db_close  / num_iterations );
                printf( "         Raw Data Read            \t%8.4g\t%8.4g\t%8.4g         \n", min_dt_raw_data,  max_dt_raw_data,  cum_dt_raw_data  / num_iterations );
                printf( "         All Other                \t        \t        \t%8.4g   \n",                                     cum_dt_other     / num_iterations );
                printf( "         Total                    \t%8.4g\t%8.4g\t%8.4g         \n", min_dt_mesh_read, max_dt_mesh_read, cum_dt_mesh_read / num_iterations );
                printf( "                                                                \n" );
		printf( "      Input Bandwidths (MB/sec)   \t Minimum\t Maximum\t Average\n" );
                printf( "         Raw Data                 \t%8.4g\t%8.4g\t%8.4g         \n", glob_raw_data_vol / max_dt_raw_data / MBYTES,
                                                                                              glob_raw_data_vol / min_dt_raw_data / MBYTES,
		                                                                              glob_raw_data_vol / cum_dt_raw_data / MBYTES * num_iterations
                );
                printf( "         Raw + Meta Data          \t%8.4g\t%8.4g\t%8.4g         \n", file_size / max_dt_mesh_read / MBYTES,
                                                                                              file_size / min_dt_mesh_read / MBYTES,
		                                                                              file_size / cum_dt_mesh_read / MBYTES * num_iterations
                );

    	}

	return( SUCCESS );
}

/***********************************************************************
 *
 *  Write saf mesh
 * 
 ***********************************************************************/

int write_saf_mesh(
	char	*device_name,
	int	num_domains,
	int	rank,
	char	*file_name,
	int	num_fields,
	int	num_iterations,
	int	glob_num_nodes,
	int	loc_num_nodes,
	int	*node_rel,
	int	glob_num_elems,
	int	loc_num_elems,
	int	*elem_map,
	float	*x_coords,
	float	*y_coords,
	int	*loc_connect,
	int	supplemental
) {
  int num_nodes_per_elem[] = {NUM_NODES_PER_ELEM};
  int top_set_topo_dim = 2;  /* topological dimension of the top set */
  int j, iter;

	double file_size;
	double glob_raw_data_vol;
        double raw_data_vol;

  char domain_set_name[MAX_STRING_LEN];
  char fld_name[MAX_STRING_LEN];

  SAF_LibProps *lib_p;

  SAF_Db *db;
  SAF_DbProps *p;

  SAF_Cat node_cat;   /* collection category for the nodes */
  SAF_Cat elem_cat;   /* collection category for the elements */
  SAF_Cat domain_cat; /* collection category for the domains */

  SAF_Set top_set;
  SAF_Set domain_set;

  SAF_Rel rel;        /* generic relation handle */

  SAF_FieldTmpl x_component_ftempl;
  SAF_FieldTmpl y_component_ftempl;
  SAF_FieldTmpl xy_component_ftempl[2];
  SAF_FieldTmpl xy_composite_ftempl;

  SAF_Field xy_coords_fld;
  SAF_Field x_coords_fld;
  SAF_Field y_coords_fld;
  SAF_Field *comp_flds;

	double       cum_dt_db_close;				/* Cumulative database close time (sec).		*/
        double       cum_dt_db_open;				/* Cumulative database open time (sec).			*/
	double       cum_dt_lib_init;				/* Cumulative library initialization time (sec).	*/
	double       cum_dt_mesh_write;				/* Cumulative overall SAF Write time (sec).		*/
        double       cum_dt_other;				/* Cumulative other time: cumulative SAF Write time	*/
        							/* minus cumulative database close,database open,	*/
                                                		/* library initialization and raw data write times	*/
                                                		/* (sec).						*/
        double       cum_dt_raw_data;				/* Cumulative raw data write time (sec).		*/
        
	double       dt_db_close;				/* Database close time (sec).				*/
        double       dt_db_open;				/* Database open time (sec).				*/
	double       dt_elem_rel;				/* Element map write time (sec).			*/
	double       dt_lib_init;				/* Library initialization time (sec).			*/
	double       dt_mesh_write;				/* Overall SAF Write time (sec).			*/
        double       dt_node_rel;				/* Node subset relations write time (sec).		*/
	double       dt_raw_data;				/* Raw data write time (sec).				*/
	double       dt_results;				/* Result simulation write time (sec).			*/
        double       dt_topo_rel;				/* Topological relations write time (sec).		*/
        double       dt_x_field;				/* X-coordinate field write time (sec).			*/
        double       dt_y_field;				/* Y-coordinate field write time (sec).			*/

/*	On ASCI Red, a specialized "stat", named "estat", was added to
 *	accommodate file sizes upto 16GB.                    3/27/2002
 */

#ifdef TFLOPS
	struct estat file_status;				/* Arbitrary file status on ASCI Red.                   */
#else
	struct stat  file_status;                               /* Arbitrary file status.                               */
#endif
        
        int          first;					/* TRUE,  for first iteration,				*/
        							/* FALSE, for all subsequent iterations.		*/

	char         master_file_name[MAX_STRING_LEN];		/* SAF master file name.				*/					

	double       max_dt_db_close;				/* Maximum database close time (sec).			*/
        double       max_dt_db_open;				/* Maximum database open time (sec).			*/
	double       max_dt_lib_init;				/* Maximum library initialization time (sec).		*/
	double       max_dt_mesh_write;				/* Maximum overall SAF Write time (sec).		*/
        double       max_dt_raw_data;				/* Maximum raw data write time (sec).			*/

	double       min_dt_db_close;				/* Minimum database close time (sec).			*/
        double       min_dt_db_open;				/* Minimum database open time (sec).			*/
	double       min_dt_lib_init;				/* Minimum library initialization time (sec).		*/
	double       min_dt_mesh_write;				/* Minimum overall SAF Write time (sec).		*/
        double       min_dt_raw_data;				/* minimum raw data write time (sec).			*/
                
	SAF_Db      *supplemental_file;				/* Database supplemental file handle.			*/
	char         supplemental_file_name[MAX_STRING_LEN];	/* SAF supplemental file name.				*/					

	double       t_finish;					/* Arbitrary finish time (sec).				*/
	double       t_start;					/* Arbitrary start time (sec).				*/
	double       t0;					/* Starting wall-clock time (sec).			*/

	sprintf( master_file_name, "%s%s%s", file_name, MASTER_FILE_TYPE, WRITE_FILE_TYPE );

	if ( supplemental )
		sprintf( supplemental_file_name, "%s%s%s%s", device_name, file_name, SUPPLEMENTAL_FILE_TYPE, WRITE_FILE_TYPE );

	first = TRUE;
        
	for ( iter = 0; iter < num_iterations; iter++ ) {
		raw_data_vol =	0.;
		t0 =		MPI_Wtime();
  
		/*
		 *	Start Saf Session and Open Database
		 */

		lib_p = saf_createProps_lib();

	        saf_setProps_ErrorMode( lib_p, SAF_ERRMODE_RETURN );

		t_start = MPI_Wtime();
		
		if ( saf_init( lib_p ) != SAF_SUCCESS ) {
			if ( rank == 0 )
				fprintf( stderr, "SAF Write (saf_init): unable to initialize SAF library.\n" );
                
			return( FAILURE );
		}

		t_finish =	MPI_Wtime();
		dt_lib_init =	t_finish - t_start;

		p = saf_createProps_database();

		saf_setProps_Clobber( p );

		t_start = MPI_Wtime();
		
		db = saf_open_database( master_file_name, p );
                                 
		/*
		 *	If a supplemental file was specified, declare that file
                 *	to SAF and write all non-meta data to it. Otherwise,
                 *	write all non-meta data to the SAF master file.
         	 */


         	if ( supplemental )
                    supplemental_file = saf_open_database(supplemental_file_name, p);
          	else 
                    supplemental_file = db;

		t_finish =	MPI_Wtime();
		dt_db_open =	t_finish - t_start;

		saf_declare_category(SAF_ALL, db, "nodes",   SAF_TOPOLOGY,  0, &node_cat   );	/* Declare collection categories.	*/
		saf_declare_category(SAF_ALL, db, "elems",   SAF_TOPOLOGY,  2, &elem_cat   );
		saf_declare_category(SAF_ALL, db, "domains", SAF_PROCESSOR, 2, &domain_cat );

		saf_declare_set(		/* Declare top set.			*/
                	SAF_ALL,
                	db,
                        "TOP_SET",
                        top_set_topo_dim,
                        SAF_SPACE, 
			SAF_EXTENDIBLE_FALSE,
                        &top_set
                ); 

		saf_declare_collection(		/* Declare collections on top set.	 */
			SAF_ALL,
                	&top_set,
                	&node_cat,
                	SAF_CELLTYPE_POINT, 
                	glob_num_nodes,
			SAF_1DC(glob_num_nodes), 
                	SAF_DECOMP_FALSE
		);
		saf_declare_collection(
                	SAF_ALL,
                        &top_set,
                        &elem_cat,
                        SAF_CELLTYPE_QUAD, 
                        glob_num_elems,
                        SAF_1DC(glob_num_elems), 
                        SAF_DECOMP_TRUE
		);
		saf_declare_collection(
                	SAF_ALL,
                        &top_set,
                        &domain_cat,
                        SAF_CELLTYPE_SET, 
			num_domains,
                        SAF_1DC(num_domains), 
			SAF_DECOMP_TRUE
		);

		/* Each processor will:
			declare a domain set;
			declare collections on domain set;
			declare domain set as subset of top via node and element maps */
  
		sprintf (domain_set_name, "DOMAIN_SET_%d", rank);
		saf_declare_set(SAF_EACH, db, domain_set_name, top_set_topo_dim, 
                      SAF_SPACE, SAF_EXTENDIBLE_FALSE, &domain_set);

		/* Declare collections of nodes, elements, and domains on domain set */
  
		saf_declare_collection(SAF_EACH, &domain_set, &node_cat, SAF_CELLTYPE_POINT, 
                             loc_num_nodes, SAF_1DC(loc_num_nodes), 
                             SAF_DECOMP_FALSE);
		saf_declare_collection(SAF_EACH, &domain_set, &elem_cat, SAF_CELLTYPE_QUAD, 
                             loc_num_elems, SAF_1DC(loc_num_elems), 
                             SAF_DECOMP_TRUE);
		saf_declare_collection(SAF_EACH, &domain_set, &domain_cat, SAF_CELLTYPE_SET, 
                             1, SAF_1DC(1), 
                             SAF_DECOMP_TRUE);

		/* Declare subset relations of elements, nodes, and domains */
  
		saf_declare_subset_relation(SAF_EACH, db, &top_set, &domain_set, 
                                  SAF_COMMON(&elem_cat),
                                  SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);

		t_start = MPI_Wtime();
  
		saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, elem_map, H5I_INVALID_HID, NULL, supplemental_file );

		t_finish =	MPI_Wtime();
		dt_elem_rel =	t_finish - t_start;
		raw_data_vol +=	(double)(loc_num_elems * sizeof(int));

		saf_declare_subset_relation(SAF_EACH, db, &top_set, &domain_set, 
                                  SAF_COMMON(&node_cat),
                                  SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);

		t_start = MPI_Wtime();
  
		saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, node_rel, H5I_INVALID_HID, NULL, supplemental_file );

		t_finish =	MPI_Wtime();
		dt_node_rel =	t_finish - t_start;
		raw_data_vol += (double)(loc_num_nodes * sizeof(int));
  
		saf_declare_subset_relation(SAF_EACH, db, &top_set, &domain_set, 
                                  SAF_COMMON(&domain_cat),
                                  SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
		saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, &rank, H5I_INVALID_HID, NULL, supplemental_file );
  

		/* Declare topology relations;
		   local elements, local nodes */

		saf_declare_topo_relation(SAF_EACH,db, &domain_set, &elem_cat, 
                                &domain_set, &node_cat,
                                SAF_SELF(db), &domain_set, SAF_UNSTRUCTURED,
                                H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &rel);
		
                t_start = MPI_Wtime();
  
		saf_write_topo_relation(SAF_EACH, &rel, SAF_INT, num_nodes_per_elem, 
                              SAF_INT, loc_connect, supplemental_file );

		t_finish =	MPI_Wtime();
		dt_topo_rel =	t_finish - t_start;
		raw_data_vol +=	(double)(loc_num_elems * NUM_NODES_PER_ELEM * sizeof(int));
  
		/*
		 * Declare Coordinate Field Templates
		 */
  
		sprintf(
			fld_name,
			"X_COORD_TMPL_%d",
			rank
		);

		saf_declare_field_tmpl(
			SAF_EACH,
			/* domain_set, */
                        db,
			fld_name,
			SAF_ALGTYPE_SCALAR,
			SAF_CARTESIAN,
			SAF_QLENGTH,
			1, 
			NULL,
			&x_component_ftempl
		);
  
		sprintf(
			fld_name,
			"Y_COORD_TMPL_%d",
			rank
		);

		saf_declare_field_tmpl(
			SAF_EACH,
			/* domain_set, */
			db,
			fld_name,
			SAF_ALGTYPE_SCALAR,
			SAF_CARTESIAN,
			SAF_QLENGTH,
			1, 
			NULL,
			&y_component_ftempl
		);

		xy_component_ftempl[0] = x_component_ftempl;
		xy_component_ftempl[1] = y_component_ftempl;
      
		sprintf(
			fld_name,
			"XY_COORD_TMPL_%d",
			rank
		);
      
		saf_declare_field_tmpl(
			SAF_EACH,
			/* domain_set, */
			db,
			fld_name,
			SAF_ALGTYPE_VECTOR,
			SAF_CARTESIAN,
			SAF_QLENGTH,
			2,
			xy_component_ftempl,
			&xy_composite_ftempl
		);
  
		/* Declare coordinate fields:
			declare x and y component fields;
			write out x and y component fields;
			declare xy composite field as composed of x and y component fields */
  
		saf_declare_field(
			SAF_EACH,db,
			&x_component_ftempl, 
			"X_COORDS",
			&domain_set,
			NULL,
			SAF_SELF( db ),
			SAF_NODAL( &node_cat, &elem_cat ),
			H5T_NATIVE_FLOAT,
			NULL,
			SAF_INTERLEAVE_NONE, 
			SAF_IDENTITY,
			NULL,
			&x_coords_fld
		);
  
		t_start = MPI_Wtime();
 
		saf_write_field(
			SAF_EACH,
                	&x_coords_fld,
                	SAF_WHOLE_FIELD,
                	1,
                	H5I_INVALID_HID,
                	(void**)&x_coords, 
                	supplemental_file
		);

		t_finish =	MPI_Wtime();
		dt_x_field =	t_finish - t_start;
		raw_data_vol +=	(double)(loc_num_nodes * sizeof(float));

		saf_declare_field(
			SAF_EACH,db,
                	&y_component_ftempl, 
                 	"Y_COORDS",
			&domain_set,
                 	NULL,
                 	SAF_SELF(db),
                 	SAF_NODAL( &node_cat, &elem_cat ),
                 	H5T_NATIVE_FLOAT,
                 	NULL,
                 	SAF_INTERLEAVE_NONE, 
                 	SAF_IDENTITY,
                 	NULL,
                	&y_coords_fld
		);

		t_start = MPI_Wtime();
  
		saf_write_field(
			SAF_EACH,
			&y_coords_fld,
			SAF_WHOLE_FIELD,
			1,
			H5I_INVALID_HID,
			(void**)&y_coords, 
			supplemental_file
		);

		t_finish =	MPI_Wtime();
		dt_y_field =	t_finish - t_start;
		raw_data_vol +=	(double)(loc_num_nodes * sizeof(float));

		if ( first ) comp_flds = (SAF_Field *) malloc (2 * sizeof(SAF_Field));

		comp_flds[0] = x_coords_fld;
		comp_flds[1] = y_coords_fld;

		saf_declare_field(
			SAF_EACH,db,
			&xy_composite_ftempl, 
			"XY_COORDS",
			&domain_set,
			NULL,
			SAF_SELF( db ),
			SAF_NODAL( &node_cat, &elem_cat ),
			H5T_NATIVE_FLOAT,
			comp_flds,
			SAF_INTERLEAVE_VECTOR,
			SAF_IDENTITY,
			NULL,
			&xy_coords_fld
		);
  
		saf_declare_coords( SAF_EACH, &xy_coords_fld );
  
		/* write out the simulated results variables */
  
		dt_results = 0.;

		for ( j = 0; j < num_fields; j++ ) {
			sprintf(
                		fld_name,
                        	"FIELD_%d",
                        	j
			);

			saf_declare_field(
        		 	SAF_EACH,db,
              	 	 	&x_component_ftempl, 
             	    	 	fld_name,
				&domain_set,
             	 	 	NULL,
             	    	 	SAF_SELF( db ),
                 	 	SAF_NODAL( &node_cat, &elem_cat ),
                 	 	H5T_NATIVE_FLOAT,
                 	 	NULL,
			 	SAF_INTERLEAVE_NONE,
			 	SAF_IDENTITY,
                 		NULL,
                		&x_coords_fld
			);
  
	        	t_start = MPI_Wtime();
  
			saf_write_field(SAF_EACH, &x_coords_fld, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, (void**)&x_coords, 
                        		supplemental_file );
			
                        t_finish =	MPI_Wtime();
			dt_results +=	t_finish - t_start;
			raw_data_vol +=	(double)(loc_num_nodes * sizeof(float));
		}
  
		/*
	         *	Close Database and Library
	         */

		t_start = MPI_Wtime();
         
		saf_close_database( db );

		t_finish =	MPI_Wtime();
		dt_db_close =	t_finish - t_start;
  
	  	saf_final();

		t_finish =	MPI_Wtime();
		dt_mesh_write =	t_finish - t0;
	
		/*
	         *	Perform Timings
	         */

		dt_raw_data = dt_node_rel + dt_elem_rel + dt_topo_rel + dt_x_field + dt_y_field + dt_results;
                
		if ( first ) {
			cum_dt_db_close	=	dt_db_close;
			cum_dt_db_open =	dt_db_open;
			cum_dt_lib_init =	dt_lib_init;	
			cum_dt_mesh_write =	dt_mesh_write;	
			cum_dt_raw_data =	dt_raw_data;
        
			max_dt_db_close	=	dt_db_close;
			max_dt_db_open =	dt_db_open;
			max_dt_lib_init =	dt_lib_init;	
			max_dt_mesh_write =	dt_mesh_write;	
			max_dt_raw_data =	dt_raw_data;

			min_dt_db_close	=	dt_db_close;
			min_dt_db_open =	dt_db_open;
			min_dt_lib_init =	dt_lib_init;	
			min_dt_mesh_write =	dt_mesh_write;	
			min_dt_raw_data =	dt_raw_data;

			first =			FALSE;
               	}
                else {
			cum_dt_db_close	+=	dt_db_close;
			cum_dt_db_open +=	dt_db_open;
			cum_dt_lib_init +=	dt_lib_init;	
			cum_dt_mesh_write +=	dt_mesh_write;	
			cum_dt_raw_data +=	dt_raw_data;
        
			if ( max_dt_db_close	<	dt_db_close	) max_dt_db_close =	dt_db_close;
			if ( max_dt_db_open 	<	dt_db_open	) max_dt_db_open =	dt_db_open;
			if ( max_dt_lib_init	<	dt_lib_init	) max_dt_lib_init =	dt_lib_init;	
			if ( max_dt_mesh_write	<	dt_mesh_write	) max_dt_mesh_write =	dt_mesh_write;	
			if ( max_dt_raw_data	<	dt_raw_data	) max_dt_raw_data =	dt_raw_data;
        
			if ( min_dt_db_close	>	dt_db_close	) min_dt_db_close =	dt_db_close;
			if ( min_dt_db_open 	>	dt_db_open	) min_dt_db_open =	dt_db_open;
			if ( min_dt_lib_init	>	dt_lib_init	) min_dt_lib_init =	dt_lib_init;	
			if ( min_dt_mesh_write	>	dt_mesh_write	) min_dt_mesh_write =	dt_mesh_write;	
			if ( min_dt_raw_data	>	dt_raw_data	) min_dt_raw_data =	dt_raw_data;
               }
	}  /* end of for (iter...) */

	MPI_Allreduce(
        	&raw_data_vol,
                &glob_raw_data_vol,
                1,
                MPI_DOUBLE,
                MPI_SUM,
                MPI_COMM_WORLD
        );

/*
 *	Get File Sizes
 *
 *	Note: On ASCI Red, a specialized "stat", named "estat", was added to
 *	accommodate file sizes upto 16GB.                          3/27/2002
 */

#ifdef TFLOPS
	if ( estat( master_file_name, &file_status ) ) {
#else
	if (  stat( master_file_name, &file_status ) ) {
#endif
        	if ( rank == 0 ) 
			fprintf ( stderr, "SAF Write: cannot get %s file size.\n", master_file_name );
  
		return( FAILURE );
	}
        else
        	file_size = (double)file_status.st_size;

	if ( supplemental ) {
#ifdef TFLOPS
        	if ( estat( supplemental_file_name, &file_status ) ) {
#else
        	if (  stat( supplemental_file_name, &file_status ) ) {
#endif
        		if ( rank == 0 ) 
				fprintf ( stderr, "SAF Write: cannot get %s file size.\n", supplemental_file_name );
                        
			return( FAILURE );
		}
        	else
        		file_size += (double)file_status.st_size;
        }
        
        if ( rank == 0 ) {
		cum_dt_other =	cum_dt_mesh_write - cum_dt_lib_init - cum_dt_db_open - cum_dt_db_close - cum_dt_raw_data;

                printf( "                                                                \n" );
                printf( "   SAF Write Results                                            \n" );
                printf( "                                                                \n" );
		printf( "      Sizes (bytes)                                             \n" );
                printf( "         File         %16.12g                                   \n", file_size                     );
                printf( "         Raw Data     %16.12g                                   \n", glob_raw_data_vol             );
                printf( "         Difference   %16.12g                                   \n", file_size - glob_raw_data_vol );
                printf( "                                                                \n" );
		printf( "      Times (sec)                 \t Minimum\t Maximum\t Average\n" );
                printf( "         Library Initialization   \t%8.4g\t%8.4g\t%8.4g         \n", min_dt_lib_init,  max_dt_lib_init,    cum_dt_lib_init   / num_iterations );
                printf( "         Database Open            \t%8.4g\t%8.4g\t%8.4g         \n", min_dt_db_open,   max_dt_db_open,     cum_dt_db_open    / num_iterations );
                printf( "         Database Close           \t%8.4g\t%8.4g\t%8.4g         \n", min_dt_db_close,  max_dt_db_close,    cum_dt_db_close   / num_iterations );
                printf( "         Raw Data Write           \t%8.4g\t%8.4g\t%8.4g         \n", min_dt_raw_data,  max_dt_raw_data,    cum_dt_raw_data   / num_iterations );
                printf( "         All Other                \t        \t        \t%8.4g   \n",                                       cum_dt_other      / num_iterations );
                printf( "         Total                    \t%8.4g\t%8.4g\t%8.4g         \n", min_dt_mesh_write, max_dt_mesh_write, cum_dt_mesh_write / num_iterations );
                printf( "                                                                \n" );
		printf( "      Output Bandwidths (MB/sec)  \t Minimum\t Maximum\t Average\n" );
                printf( "         Raw Data                 \t%8.4g\t%8.4g\t%8.4g         \n", glob_raw_data_vol / max_dt_raw_data / MBYTES,
                                                                                              glob_raw_data_vol / min_dt_raw_data / MBYTES,
		                                                                              glob_raw_data_vol / cum_dt_raw_data / MBYTES * num_iterations
                );
                printf( "         Raw + Meta Data          \t%8.4g\t%8.4g\t%8.4g         \n", file_size / max_dt_mesh_write / MBYTES,
                                                                                              file_size / min_dt_mesh_write / MBYTES,
		                                                                              file_size / cum_dt_mesh_write / MBYTES * num_iterations
                );
    	}

	return( SUCCESS );
}


#ifdef HAVE_EXODUS
/***********************************************************************
 ***********************************************************************/
int read_exo_mesh (char *file_name, int rank, int num_fields, int num_iterations,
                    int *num_nodes, int **node_map,
                    int *num_elems, int **elem_map,
                    float **x_coords, float **y_coords, int **connect )

{
  int CPU_word_size=0;
  int IO_word_size=4;
  int exoid, err, num_dim, num_elem_blk, num_node_sets, num_side_sets;
  int num_nodes_per_elem, num_attrs, num_node_vars, i, iter;
  int len_connect;

	double file_size;

/*	On ASCI Red, a specialized "stat", named "estat", was added to
 *	accommodate file sizes upto 16GB.                    3/27/2002
 */

#ifdef TFLOPS
	struct estat file_status;				/* Arbitrary file status on ASCI Red.                   */
#else
	struct stat  file_status;                               /* Arbitrary file status.                               */
#endif

	double glob_file_size;
	double glob_raw_data_vol;
	double raw_data_vol;

  float *z, version;

  double tstart, tend, t_tmp1, t_tmp2;
  double raw_read_time, max_raw_read_time=0.0, min_raw_read_time=DBL_MAX;
  double cum_raw_read_time=0.0;
  double total_time, max_total_time=0.0, min_total_time=DBL_MAX;
  double cum_total_time=0.0;

  char tmp_name[MAX_STRING_LEN], title[MAX_STRING_LEN+1]; 
  char type[MAX_STRING_LEN+1];

	for (iter=0; iter<num_iterations; iter++) {
		raw_read_time = 0.0;
		raw_data_vol = 0.;

		tstart = MPI_Wtime();

		/* open the EXODUS file */

		sprintf (tmp_name, "%s_%d.ex2", file_name, rank);
		exoid = ex_open (tmp_name, EX_READ, &CPU_word_size, &IO_word_size, &version);
  
		if (exoid < 0) {
			printf ("after ex_open\n");

			return( FAILURE );
		}
  
		err = ex_get_init (exoid, title, &num_dim, 
                       num_nodes, num_elems, &num_elem_blk, 
                       &num_node_sets, &num_side_sets);
  
		if (err) {
			printf ("after ex_get_init, error = %d\n", err);
			ex_close (exoid);

			return( FAILURE);
		}
  
		len_connect = NUM_NODES_PER_ELEM * (*num_elems);
  
		/* malloc things we need */
  
		if( iter == 0 ) {
			*elem_map = (int *  )malloc( (*num_elems) * sizeof( int   ) );
			*connect =  (int *  )malloc( len_connect  * sizeof( int   ) );
			*node_map = (int *  )malloc( len_connect  * sizeof( int   ) );

			*x_coords = (float *)malloc( (*num_nodes) * sizeof( float ) );
			*y_coords = (float *)malloc( (*num_nodes) * sizeof( float ) );
		}
  
		t_tmp1 = MPI_Wtime();
  
		err = ex_get_coord (exoid, *x_coords, *y_coords, z);
  
    		t_tmp2 = MPI_Wtime();
  
		raw_read_time += t_tmp2-t_tmp1;
  		raw_data_vol += (double)(2 * (*num_nodes) * sizeof(float));
  
		if (err) {
			printf ("after ex_get_coord, error = %d\n", err);
			ex_close (exoid);
			return (FAILURE );
		}
  
		err = ex_get_elem_block 
			(exoid, 10, type, num_elems, &num_nodes_per_elem, &num_attrs);
  
		if (err) {
			printf ("after ex_get_elem_block, error = %d\n", err);
			ex_close (exoid);

			return( FAILURE );
		}
  
		t_tmp1 = MPI_Wtime();
  
		err = ex_get_elem_conn (exoid, 10, *connect);
  
		t_tmp2 = MPI_Wtime();
  
		raw_read_time += t_tmp2-t_tmp1;
		raw_data_vol += (double)(len_connect * sizeof(int));
  
		if (err) {
			printf ("after ex_get_elem_conn, error = %d\n", err);
			ex_close (exoid);

			return( FAILURE );
		}
  
		/* read element and node maps */
  
		t_tmp1 = MPI_Wtime();
  
		err = ex_get_node_num_map (exoid, *node_map);
  
		t_tmp2 = MPI_Wtime();
  
		raw_read_time += t_tmp2-t_tmp1;
		raw_data_vol += (double)((*num_nodes) * sizeof(int));
  
		if (err) {
			printf ("after ex_get_node_num_map, error = %d\n", err);
			ex_close (exoid);

			return( FAILURE );
		}
  
    		t_tmp1 = MPI_Wtime();
  
    		err = ex_get_elem_num_map (exoid, *elem_map);
  
    		t_tmp2 = MPI_Wtime();
  
    		raw_read_time += t_tmp2-t_tmp1;
    		raw_data_vol += (double)((*num_elems) * sizeof(int));
  
		if (err) {
			printf ("after ex_get_elem_num_map, error = %d\n", err);
			ex_close (exoid);

			return( FAILURE);
    		}
  
		/* read nodal results variables */
  
		if (num_fields > 0) {
			err = ex_get_var_param (exoid, "n", &num_node_vars);

			if (err) {
				printf ("after ex_get_var_param, error = %d\n", err);
				ex_close (exoid);

				return( FAILURE );
			}
  
			if (num_node_vars != num_fields) {
				printf ("wrong number of nodal variables; %d, should be %d\n",
						num_node_vars, num_fields);
				ex_close (exoid);

			exit( FAILURE );
			}
  
			for (i=1; i<=num_node_vars; i++) {
				t_tmp1 = MPI_Wtime();
  
				err = ex_get_nodal_var (exoid, 1, i, *num_nodes, *x_coords);
  
				t_tmp2 = MPI_Wtime();
  
				raw_read_time += t_tmp2-t_tmp1;
				raw_data_vol += (double)((*num_nodes) * sizeof(int));
  
				if (err) {
					printf ("after ex_get_nodal_var, error = %d\n", err);
					ex_close (exoid);

					return( FAILURE );
				}
			}
		}
  
 		err = ex_close (exoid);
  
		if (err) {
			printf ("after ex_close, error = %d\n", err);
			
			return ( FAILURE );
		}
  
		tend = MPI_Wtime();
  
		total_time = tend - tstart;
 		if (total_time > max_total_time) max_total_time = total_time;
		if (total_time < min_total_time) min_total_time = total_time;
		cum_total_time += total_time;

		if (raw_read_time > max_raw_read_time) max_raw_read_time = raw_read_time;
		if (raw_read_time < min_raw_read_time) min_raw_read_time = raw_read_time;
		cum_raw_read_time += raw_read_time;
	}  /* end of for (iter...) */

	MPI_Allreduce (&raw_data_vol, &glob_raw_data_vol, 1, MPI_DOUBLE, MPI_SUM,
                 MPI_COMM_WORLD);

/*
 *	Get File Sizes
 *
 *	Note: On ASCI Red, a specialized "stat", named "estat", was added to
 *	accommodate file sizes upto 16GB.                          3/27/2002
 */

#ifdef TFLOPS
	if ( estat( tmp_name, &file_status ) ) {
#else
	if (  stat( tmp_name, &file_status ) ) {
#endif
        	if ( rank == 0 ) 
			fprintf ( stderr, "Exodus Read: cannot get %s file size.\n", tmp_name );
  
		return( FAILURE );
	}
        else
        	file_size = (double)file_status.st_size;

	MPI_Allreduce(
		&file_size,
		&glob_file_size,
		1,
		MPI_DOUBLE,
		MPI_SUM,
                MPI_COMM_WORLD
	);
        
        if ( rank == 0 ) {
                printf( "                                                                \n" );
                printf( "   Exodus Read Results                                          \n" );
                printf( "                                                                \n" );
		printf( "      Sizes (bytes)                                             \n" );
                printf( "         File         %14.12g                                   \n", glob_file_size                     );
                printf( "         Raw Data     %14.12g                                   \n", glob_raw_data_vol                  );
                printf( "         Difference   %14.12g                                   \n", glob_file_size - glob_raw_data_vol );
                printf( "                                                                \n" );
		printf( "      Times (sec)                 \t Minimum\t Maximum\t Average\n" );
                printf( "         Raw Data Read            \t%8.4g\t%8.4g\t%8.4g         \n", min_raw_read_time, max_raw_read_time, cum_raw_read_time / num_iterations );
                printf( "         All Other                \t        \t        \t%8.4g   \n", (cum_total_time - cum_raw_read_time) / num_iterations );
		printf( "         Total                    \t        \t        \t%8.4g   \n", cum_total_time / num_iterations );
                printf( "                                                                \n" );
		printf( "      Input Bandwidths (MB/sec)   \t Minimum\t Maximum\t Average\n" );
                printf( "         Raw Data                 \t%8.4g\t%8.4g\t%8.4g         \n", glob_raw_data_vol / max_raw_read_time / MBYTES,
                                                                                              glob_raw_data_vol / min_raw_read_time / MBYTES,
		                                                                              glob_raw_data_vol / cum_raw_read_time / MBYTES * num_iterations
                );
                printf( "         Raw + Meta Data          \t        \t        \t%8.4g   \n", glob_file_size / cum_total_time / MBYTES * num_iterations );
	}

	return( SUCCESS );
}

/***********************************************************************
 ***********************************************************************/
int write_exo_mesh (char *file_name, int rank, int num_fields, int num_iterations,
                     int num_nodes, int *node_map,
                     int num_elems, int *elem_map,
                     float *x_coords, float *y_coords, int *connect )

{
  int CPU_word_size=0;
  int IO_word_size=4;
  int j, exoid, err, num_dim, num_elem_blk, num_node_sets, num_side_sets;
  int iter;

	double file_size;

/*	On ASCI Red, a specialized "stat", named "estat", was added to
 *	accommodate file sizes upto 16GB.                    3/27/2002
 */

#ifdef TFLOPS
	struct estat file_status;				/* Arbitrary file status on ASCI Red.                   */
#else
	struct stat  file_status;                               /* Arbitrary file status.                               */
#endif

	double glob_file_size;
	double glob_raw_data_vol;
	double raw_data_vol;

  float *z;

  double tstart, tend, t_tmp1, t_tmp2;
  double raw_write_time, max_raw_write_time=0.0, min_raw_write_time=DBL_MAX;
  double cum_raw_write_time=0.0;
  double total_time, max_total_time=0.0, min_total_time=DBL_MAX;
  double cum_total_time=0.0;

  char tmp_name[MAX_STRING_LEN];
  char **var_name;

	for (iter=0; iter<num_iterations; iter++) {
		raw_write_time = 0.0;
		raw_data_vol = 0;

		tstart = MPI_Wtime();
  
		/* create the EXODUS file */
  
		sprintf ( tmp_name, "%s_%d%s%s", file_name, rank, EXODUS_FILE_TYPE, WRITE_FILE_TYPE );
		exoid = ex_create (tmp_name, EX_CLOBBER, &CPU_word_size, &IO_word_size);
  
		if (exoid < 0) {
			printf ("after ex_create\n");

			return( FAILURE );
 		}
  
		num_dim = 2;
		num_elem_blk = 1;
		num_node_sets = 0;
		num_side_sets = 0;
  
		err = ex_put_init (exoid, "This is a SAF performance test.", num_dim, 
                       num_nodes, num_elems, num_elem_blk, 
                       num_node_sets, num_side_sets);
  
		if (err) {
			printf ("after ex_put_init, error = %d\n", err);
			ex_close (exoid);

			return( FAILURE);
		}
  
		t_tmp1 = MPI_Wtime();
  
		err = ex_put_coord (exoid, x_coords, y_coords, z);
  
		t_tmp2 = MPI_Wtime();
  
		raw_write_time += t_tmp2-t_tmp1;
		raw_data_vol += (2 * num_nodes * sizeof(float));
  
		if (err) {
			printf ("after ex_put_coord, error = %d\n", err);
 			ex_close (exoid);

			return( FAILURE );
		}
  
		err = ex_put_elem_block 
			(exoid, 10, "quad", num_elems, NUM_NODES_PER_ELEM, 0);
  
		if (err) {
			printf ("after ex_put_elem_block, error = %d\n", err);
			ex_close (exoid);

			return( FAILURE );
		}
  
		t_tmp1 = MPI_Wtime();
  
		err = ex_put_elem_conn (exoid, 10, connect);
  
		t_tmp2 = MPI_Wtime();
  
		raw_write_time += t_tmp2-t_tmp1;
		raw_data_vol += (num_elems * NUM_NODES_PER_ELEM * sizeof(int));
  
		if (err) {
			printf ("after ex_put_elem_conn, error = %d\n", err);
			ex_close (exoid);

			return( FAILURE);
    		}
  
		/* write out element and node maps */
  
		t_tmp1 = MPI_Wtime();
  
		err = ex_put_node_num_map (exoid, node_map);
  
		t_tmp2 = MPI_Wtime();
  
		raw_write_time += t_tmp2-t_tmp1;
		raw_data_vol += (num_nodes * sizeof(int));
  
		if (err) {
			printf ("after ex_put_node_num_map, error = %d\n", err);
			ex_close (exoid);

			return( FAILURE );
		}
  
		t_tmp1 = MPI_Wtime();
  
		err = ex_put_elem_num_map (exoid, elem_map);
  
		t_tmp2 = MPI_Wtime();
  
		raw_write_time += t_tmp2-t_tmp1;
		raw_data_vol += (num_elems * sizeof(int));
  
		if (err) {
			printf ("after ex_put_elem_num_map, error = %d\n", err);
			ex_close (exoid);

			return( FAILURE );
		}
  
		/* write out simulated results fields;
			we'll just write out the x coordinate field 'num_fields' times */
  
 		if (num_fields > 0) {
			err = ex_put_var_param (exoid, "n", num_fields);

			if (err) {
				printf ("after ex_put_var_param, error = %d\n", err);
 				ex_close (exoid);

        			return( FAILURE );
			}
  
			var_name = (char **) malloc (num_fields * sizeof(char *));

			for (j=0; j<num_fields; j++) {
				var_name[j] = (char *) malloc ((MAX_STRING_LEN+1) * sizeof (char));
				sprintf (var_name[j], "field_%d", j+1);
  
				t_tmp1 = MPI_Wtime();
  
				err = ex_put_nodal_var (exoid, 1, j+1, num_nodes, x_coords);
  
				t_tmp2 = MPI_Wtime();
  
				raw_write_time += t_tmp2-t_tmp1;
				raw_data_vol += (num_nodes * sizeof(float));
  
				if (err) {
					printf ("after ex_put_nodal_var, error = %d\n", err);
					ex_close (exoid);

					return( FAILURE);
				}
			}
  
			err = ex_put_var_names (exoid, "n", num_fields, var_name);

			if (err) {
				printf ("after ex_put_nodal_var, error = %d\n", err);
				ex_close (exoid);

				return( FAILURE );
			}
		}
  
		err = ex_close (exoid);
  
		if (err) {
			printf ("after ex_close, error = %d\n", err);
			
			return( FAILURE);
		}
  
		tend = MPI_Wtime();
  
		total_time = tend - tstart;
		
		if (total_time > max_total_time) max_total_time = total_time;
    		if (total_time < min_total_time) min_total_time = total_time;

		cum_total_time += total_time;

		if (raw_write_time > max_raw_write_time) max_raw_write_time = raw_write_time;
		if (raw_write_time < min_raw_write_time) min_raw_write_time = raw_write_time;

		cum_raw_write_time += raw_write_time;

	}  /* end of for (iter...) */

	MPI_Allreduce(
		&raw_data_vol,
		&glob_raw_data_vol,
		1,
		MPI_DOUBLE,
		MPI_SUM,
               	MPI_COMM_WORLD
	);

/*
 *	Get File Sizes
 *
 *	Note: On ASCI Red, a specialized "stat", named "estat", was added to
 *	accommodate file sizes upto 16GB.                          3/27/2002
 */

#ifdef TFLOPS
	if ( estat( tmp_name, &file_status ) ) {
#else
	if (  stat( tmp_name, &file_status ) ) {
#endif
        	if ( rank == 0 ) 
			fprintf ( stderr, "Exodus Write: cannot get %s file size.\n", tmp_name );
  
		return( FAILURE );
	}
        else
        	file_size = (double)file_status.st_size;

	MPI_Allreduce (&file_size, &glob_file_size, 1, MPI_DOUBLE, MPI_SUM,
                 MPI_COMM_WORLD);
        
        if ( rank == 0 ) {
                printf( "                                                                \n" );
                printf( "   Exodus Write Results                                         \n" );
                printf( "                                                                \n" );
		printf( "      Sizes (bytes)                                             \n" );
                printf( "         File         %14.12g                                   \n", glob_file_size                     );
                printf( "         Raw Data     %14.12g                                   \n", glob_raw_data_vol                  );
                printf( "         Difference   %14.12g                                   \n", glob_file_size - glob_raw_data_vol );
                printf( "                                                                \n" );
		printf( "      Times (sec)                 \t Minimum\t Maximum\t Average\n" );
                printf( "         Raw Data Write           \t%8.4g\t%8.4g\t%8.4g         \n", min_raw_write_time, max_raw_write_time, cum_raw_write_time / num_iterations );
                printf( "         All Other                \t        \t        \t%8.4g   \n", (cum_total_time - cum_raw_write_time) / num_iterations );
		printf( "         Total                    \t        \t        \t%8.4g   \n", cum_total_time / num_iterations );
                printf( "                                                                \n" );
		printf( "      Output Bandwidths (MB/sec)  \t Minimum\t Maximum\t Average\n" );
                printf( "         Raw Data                 \t%8.4g\t%8.4g\t%8.4g         \n", glob_raw_data_vol / max_raw_write_time / MBYTES,
                                                                                              glob_raw_data_vol / min_raw_write_time / MBYTES,
		                                                                              glob_raw_data_vol / cum_raw_write_time / MBYTES * num_iterations
                );
                printf( "         Raw + Meta Data          \t        \t        \t%8.4g   \n", glob_file_size / cum_total_time / MBYTES * num_iterations );
	}

	return( SUCCESS );
}
#endif

#else

/*
 *	Serial "rd_wt_mesh"
 *
 *	If this program is compiled with a non-parallel compiler or a parallel
 *	compiler in serial mode, the following stub program is compiled instead
 *	of the benchmark program.                                     3/27/2002
 */

main( int argc, char **argv )
{
	printf ( "This program was compiled with a non-parallel compiler\n" );
        printf ( "or a parallel compiler in serial mode. Re-compile     \n" );
        printf ( "'rd_wt_mesh' and link to the appropriate MPI          \n" );
        printf ( "libraries.                                            \n" );
        return 1;
} 

#endif
 
