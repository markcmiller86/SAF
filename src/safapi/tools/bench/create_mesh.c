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

#include <stdio.h>
#include <stdlib.h>
#if defined(HAVE_LIBGEN_H)
#include <libgen.h>
#endif
#include <saf.h>

#ifdef HAVE_EXODUS
#include "exodusII.h"
#endif

#ifndef TRUE
#define TRUE			(1==1)
#endif

#ifndef FALSE
#define FALSE			(0==1)
#endif

#define DEFAULT_FILE_NAME	"mesh"
#define DEFAULT_MAP_ORIGIN	0
#define DEFAULT_NUM_DOMAINS	1
#define DEFAULT_NUM_ELEMENTS_1D	100
#define DEFAULT_NUM_FIELDS	0

#define MAX_STRING_LEN		128
#define NUM_BYTES_PER_INT	4
#define NUM_NODES_PER_ELEM	4

#define EXODUS_FILE_TYPE	".ex2"
#define MASTER_FILE_TYPE	".saf"
#define SUPPLEMENTAL_FILE_TYPE	".ssf"

/*
 *	Prototypes
 */

void create_rr_elem_map(
	int loc_num_elements,
	int *elem_map,
	int map_origin,
	int num_domains,
	int current_domain
);

void create_elem_map (
        int loc_num_elems, 
        int start, 
        int *elem_map
);

void create_local_connect(
	int *node_map,
	int len_node_map, 
	int len_connect,
	int *domain_connect, 
        int *loc_connect,
	int map_origin
);

void extract_connect(
	int num_elem,
	int *elem_map,
	int *connect, 
        int *domain_connect,
	int map_origin
);

void make_mesh(
	float	*x,
	float	*y,
	int	*connect,
	int	map_origin,
	int	num_elements_1d
);

void parse_input(
	int	argc,
	char	*argv[],
	int	*debug,
	int	*map_origin,
	int	*num_elements_1d, 
	int	*num_domains,
	int	*num_fields, 
	int	*round_robin_map, 
	char	*device_name,
	char	*file_name,
	int	*supplemental,
	int	*exodus,
	int	*saf
);

#ifdef HAVE_EXODUS
void write_exo_mesh(
	int	debug,
	char	*file_name,
	int	map_origin,
	int	num_elements,
	int	num_domains,
	int	num_fields,
	float	*x,
	float	*y, 
	int	*connect
);
#endif

void write_saf_mesh(
	int	debug,
	char	*device_name,
	char	*file_name,
	int	map_origin,
	int	supplemental,
	int	num_nodes,
	int	num_elements, 
	int	num_domains,
	int	num_fields,
	int	round_robin_map,
	float	*x,
	float	*y,
	int	*connect
);

void create_node_map (int len_connect, int *domain_connect, int *node_map,
                      int *loc_num_nodes);

void sort_int(int n, int ra[]);

int bin_search2 (int value, int num, int List[]);

/***********************************************************************
 *
 *  Main function
 * 
 ***********************************************************************/

int
main( int argc, char *argv[] ) {
	int	*connect;
	int	debug =				FALSE;				/* TRUE, display debug information; FALSE	*/
										/* otherwise.					*/
	static char device_name[MAX_STRING_LEN];
	static char file_name[MAX_STRING_LEN] =	DEFAULT_FILE_NAME;
	int	exodus =			FALSE;
	int	map_origin =			DEFAULT_MAP_ORIGIN;	
	int	num_domains =			DEFAULT_NUM_DOMAINS;
	int	num_elements;
	int	num_elements_1d =		DEFAULT_NUM_ELEMENTS_1D;
	int	num_fields =			DEFAULT_NUM_FIELDS;
	int	num_nodes;
        int     round_robin_map =               FALSE;
 	int	saf =				FALSE;
	int	supplemental =			FALSE;				/* TRUE, create SAF supplemental file; False	*/
										/* otherwise.					*/
	float	*x;
	float	*y;	

	/*
	 *	Parse Input
	 */

	parse_input(
		argc,
		argv,
		&debug,
		&map_origin,
		&num_elements_1d, 
		&num_domains,
		&num_fields, 
		&round_robin_map, 
		device_name,
		file_name,
		&supplemental,
		&exodus,
		&saf
	);

	/*
	 *	Create Coordinates and Connectivity Array
	 */

  	num_nodes = 	(num_elements_1d + 1) * (num_elements_1d + 1);
	x =         	(float *)malloc( num_nodes * sizeof( float ) );
	y =         	(float *)malloc( num_nodes * sizeof( float ) );

	num_elements =	num_elements_1d * num_elements_1d;
	connect =	(int *)malloc( NUM_NODES_PER_ELEM * num_elements * sizeof( int ) );

	make_mesh(
		x,
		y,
		connect,
		map_origin,
		num_elements_1d
	);

	/*
	 *	Write Out Mesh
	 */

	if (exodus) {
#ifdef HAVE_EXODUS
		write_exo_mesh(
			debug,
			file_name,
			map_origin,
			num_elements,
			num_domains,
			num_fields,
			x,
			y,
			connect
		);
#else
		printf ("create_mesh not linked with EXODUS libraries\n");

		exit ( -1 );
#endif
	}

	if (saf) {
		write_saf_mesh(
			debug,
			device_name,
			file_name,
			map_origin,
			supplemental,
			num_nodes,
			num_elements, 
			num_domains,
			num_fields,
			round_robin_map,
			x,
			y,
			connect
		);
	}
        return 0;
} /* end of main() */


/***********************************************************************
 ***********************************************************************/
void parse_input(
	int  argc,
	char *argv[],
	int  *debug,
	int  *map_origin,
	int  *num_elements_1d, 
	int  *num_domains,
	int  *num_fields, 
        int  *round_robin_map,
	char *device_name,
	char *file_name,
	int  *supplemental,
	int  *exodus,
	int  *saf
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
        	else if ( strcmp( "-d", argv[arg] ) == 0 ) {
                	*debug = TRUE;
		}
        	else if ( strcmp( "-f", argv[arg] ) == 0 ) { 
                	if ( ++arg < argc ) strcpy( file_name, argv[arg] );
                }
        	else if ( strcmp( "-m", argv[arg] ) == 0 ) {
                	if ( ++arg < argc ) *map_origin = atoi( argv[arg] );
		}
        	else if ( strcmp( "-n", argv[arg] ) == 0 ) {
                	if ( ++arg < argc ) *num_elements_1d = atoi( argv[arg] );
		}
        	else if ( strcmp( "-p", argv[arg] ) == 0 ) {
                	if ( ++arg < argc ) *num_domains = atoi( argv[arg] );
		}
        	else if ( strcmp( "-r", argv[arg] ) == 0 ) {
                	*round_robin_map = TRUE;
		}
        	else if ( strcmp( "-s", argv[arg] ) == 0 ) {
                	*saf = TRUE;
		}
        	else if ( strcmp( "-v", argv[arg] ) == 0 ) {
                	if ( ++arg < argc ) strcpy( device_name, argv[arg] );
		}
        	else if ( strcmp( "-x", argv[arg] ) == 0 ) {
                	*exodus = TRUE;
		}
                else if ( (strcmp( "-h", argv[arg] ) == 0) || (strcmp( "-u", argv[arg] ) == 0) ) {
                	printf( "                                                                \n" );
                        printf( "NAME                                                            \n" );
                        printf( "                                                                \n" );
                        printf( "create_mesh - creates a mesh file for performance benchmarking. \n" );
                	printf( "                                                                \n" );
                        printf( "SYNOPSIS                                                        \n" );
                        printf( "                                                                \n" );
                	printf( "create_mesh [-S] [-c fields] [-d] [-f file_name] [-h]           \n" );
                        printf( "            [-m map_origin] [-n elements] [-p domains]          \n" );
                        printf( "            [-r] [-s] [-u] [-v device] [-x]                     \n" );
                	printf( "                                                                \n" );
                        printf( "DESCRIPTION                                                     \n" );
                        printf( "                                                                \n" );
                	printf( "This program creates a 2-D mesh for performance benchmarking.   \n" );
                        printf( "The SAF and/or EXODUS II database file(s) created by this       \n" );
			printf( "prrogram is/are read by the rd_wt_mesh program to perform the   \n" );
                       	printf( "actual benchmark.                                               \n" );
                        printf( "                                                                \n" );
                        printf( "OPTIONS                                                         \n" );
                        printf( "                                                                \n" );
 			printf( "-S             create a SAF supplemental file. The format of the\n" );
			printf( "               name of the supplemental file created is:        \n" );
                        printf( "                                                                \n" );
                        printf( "                                'file_name'%s                   \n", SUPPLEMENTAL_FILE_TYPE  );
			printf( "                                                                \n" );
                        printf( "               The '-S' option implies the '-s' option.         \n" );
                       	printf( "-c fields      number of fields. Default: %d                    \n", DEFAULT_NUM_FIELDS      );
			printf( "-d             display debug information.			 \n" );
			printf( "-f file_name   file name prefix for all created files:          \n" );
			printf( "                                                                \n" );
                        printf( "                  'file_name'_n%s [EXODUS II file]              \n", EXODUS_FILE_TYPE        );
			printf( "                  'file_name'%s   [SAF master file]             \n", MASTER_FILE_TYPE        );
			printf( "                  'file_name'%s   [SAF supplemental file]       \n", SUPPLEMENTAL_FILE_TYPE  );
			printf( "                                                                \n" );
			printf( "               where n varies from 0 to number of domains - 1.  \n" );
			printf( "               Default: %s                                      \n", DEFAULT_FILE_NAME	      );
                        printf( "-h             display help/usage information.                  \n" );
			printf( "-m map_origin  element map origin. Default: %d                  \n", DEFAULT_MAP_ORIGIN      );
			printf( "-n elements    number of elements per dimension. Total number   \n" );
			printf( "               of elements (2-D mesh) = elements*elements.      \n" );
			printf( "               Default: %d                                      \n", DEFAULT_NUM_ELEMENTS_1D );
			printf( "-p domains     number of domains. Default: %d                   \n", DEFAULT_NUM_DOMAINS     );
                        printf( "-r             create a round robin element map.                \n" );
                        printf( "-s             create SAF mesh file(s).                         \n" );
			printf( "-x             create EXODUS II mesh file(s).                   \n" );
                        printf( "-u             display help/usage information.                  \n" );
			printf( "-v device      location to create the SAF supplemental file. If \n" );
			printf( "               not specified, the supplemental file will be	 \n" );
			printf( "               created in the same location as the SAF master   \n" );
			printf( "               file.                                            \n" );

			exit( 0 );
		}   
                else {
			printf( "Unknown option: %s\n", argv[arg]                                    );
                        printf( "Enter create_mesh -h for description of valid options.\n"           );
                        
			exit( 0 );
		}
	}

	return;
}

/***********************************************************************
 *
 *  Create the coordinates and connectivity array for the mesh
 * 
 ***********************************************************************/

void make_mesh(
	float	*x,
	float	*y,
	int	*connect,
	int	map_origin,
	int	num_elements_1d
) {
  int i, j, k, base, cnt;

  /* create global coordinates */

  for (i=0, k=0; i < (num_elements_1d+1); i++)
  {
    for (j=0; j < (num_elements_1d+1); j++, k++) 
    {
      x[k] = (float)j;
      y[k] = (float)i;
    }
  }

  /* build connectivity array (node list) for mesh */

  for (i=0, k=0, cnt=0; i < num_elements_1d; i++)
  {
    for (j=0; j < num_elements_1d; j++, k++) 
    {
      base = k+i+map_origin;
      connect[cnt++] = base;
      connect[cnt++] = base+1;
      connect[cnt++] = base+num_elements_1d+2;
      connect[cnt++] = base+num_elements_1d+1;
    }
  }
} /* end of make_mesh() */

/***********************************************************************
 *
 *  Write saf mesh
 * 
 ***********************************************************************/

void write_saf_mesh(
	int	debug,
	char	*device_name,
	char	*file_name,
	int	map_origin,
	int	supplemental,
	int	num_nodes,
	int	num_elements, 
	int	num_domains,
	int	num_fields,
	int	round_robin_map,
	float	*x,
	float	*y,
	int	*connect
) {
  int i, j, len_connect;
  int num_nodes_per_elem[] = {NUM_NODES_PER_ELEM};
  int *elem_map, *node_map;
  int *domain_connect, *loc_connect;
  int loc_num_elements, loc_num_nodes;
  int top_set_topo_dim = 2;  /* topological dimension of the top set */
  int accum_num_elements;

  float *loc_x_coords, *loc_y_coords;

  char domain_set_name[MAX_STRING_LEN];


  SAF_Db *db=NULL;
  SAF_DbProps *p=NULL;

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

  SAF_Field xy_coords;
  SAF_Field x_coords;
  SAF_Field y_coords;
  SAF_Field *comp_flds;

	SAF_Db *supplemental_file=NULL;			/* Database non-meta data file handle.	*/
	char	 temporary_name[MAX_STRING_LEN];	/* Temporary name.			*/

	SAF_TRY_BEGIN 
	{
		/*
		 *	Start Session
		 */

		saf_init( SAF_DEFAULT_LIBPROPS );

		/*
		 *	Open Database
		 */

		p = saf_createProps_database();

		saf_setProps_Clobber(p);

		sprintf( temporary_name, "%s%s", file_name, MASTER_FILE_TYPE );

		db = saf_open_database( temporary_name, p );
    
	/*
	 *	If a supplemental file was specified, declare that file to SAF
         *	and write all non-meta data to it. Otherwise, all data is
         *	written to the SAF master file.
         */
         
		if ( supplemental ) {
			sprintf( temporary_name, "%s%s%s", device_name, file_name, SUPPLEMENTAL_FILE_TYPE );
			supplemental_file = saf_open_database(temporary_name,p);
		}
         else
         	supplemental_file = db;

    /* Declare collection categories */
    saf_declare_category(SAF_ALL, db, "nodes",   SAF_TOPOLOGY,  0, &node_cat   );
    saf_declare_category(SAF_ALL, db, "elems",   SAF_TOPOLOGY,  2, &elem_cat   );
    saf_declare_category(SAF_ALL, db, "domains", SAF_PROCESSOR, 2, &domain_cat );

    /* Declare top set */
    saf_declare_set(SAF_ALL, db, "TOP_SET", top_set_topo_dim, SAF_SPACE, 
                    SAF_EXTENDIBLE_FALSE, &top_set); 

    /* Declare collections on top set */
    saf_declare_collection(SAF_ALL, &top_set, &node_cat, SAF_CELLTYPE_POINT, 
                           num_nodes, SAF_1DC(num_nodes), SAF_DECOMP_FALSE);
    saf_declare_collection(SAF_ALL, &top_set, &elem_cat, SAF_CELLTYPE_QUAD, 
                           num_elements, SAF_1DC(num_elements), SAF_DECOMP_TRUE);
    saf_declare_collection(SAF_ALL, &top_set, &domain_cat, SAF_CELLTYPE_SET, 
                           num_domains, SAF_1DC(num_domains), SAF_DECOMP_TRUE);
                          
    /* For each domain:
         declare a set;
         declare collections;
         create element and node local/global maps 
         declare domain set as subset of top */


	for ( i = 0, accum_num_elements = 0; i < num_domains; i++ ) {
		sprintf (domain_set_name, "DOMAIN_SET_%d", i);

		saf_declare_set(SAF_ALL, db, domain_set_name, top_set_topo_dim, 
			SAF_SPACE, SAF_EXTENDIBLE_FALSE, &domain_set);

		/* Determine local number of elements */
		if (num_elements < num_domains) {
			printf ("number of elements is less than number of domains.\n");

			if (i < num_elements) loc_num_elements = 1;
			else loc_num_elements = 0;
		} else {
			loc_num_elements = num_elements / num_domains;

			if (i < (num_elements % num_domains)) loc_num_elements++;
		}

		len_connect = NUM_NODES_PER_ELEM * loc_num_elements;

		/* malloc things we need */

		if (i == 0) {	/* first time through; max size arrays occur on
				first iteration */
			elem_map =		(int *)malloc( loc_num_elements * sizeof(int) );
			domain_connect =	(int *)malloc( len_connect      * sizeof(int) );
			loc_connect =		(int *)malloc( len_connect      * sizeof(int) );
			node_map =		(int *)malloc( len_connect      * sizeof(int) );
		}

		/* Create element local/global map */

                if (round_robin_map) {
		  create_rr_elem_map(
		  	loc_num_elements,
			elem_map,
			map_origin,
			num_domains,
			i
  		  );
                } else {
		  create_elem_map(
		  	loc_num_elements,
                        accum_num_elements,
			elem_map
  		  );
                }

                accum_num_elements += loc_num_elements;

		/* Extract current domain's connectivity, referencing global node ids */

		extract_connect(
			num_elements,
			elem_map,
			connect,
			domain_connect,
			map_origin
		);

		/* The local/global node map is just the current domain's connectivity,
		   sorted with duplicate entries removed */

		create_node_map (len_connect, domain_connect, node_map, &loc_num_nodes);

		/* Using local/global node map, convert the domain connectivity 
		   (referencing global node ids) to local connectivity (referencing 
		   local node ids) */

		create_local_connect(
			node_map,
			loc_num_nodes, 
                	len_connect,
			domain_connect,
			loc_connect,
			map_origin
		);

		if ( debug ) {
		        printf ("\n\n\n");

			printf ("\n domain: %d\n", i);
		        printf ("\n loc_num_elements: %d\n", loc_num_elements);
		        printf ("\n loc_num_nodes: %d\n", loc_num_nodes);

		        printf ("\n element map:\n");
		        for (j=0; j<loc_num_elements; j++) printf (" %d,", elem_map[j]);

		        printf ("\n domain connectivity:\n");
		        for (j=0; j<len_connect; j++) printf (" %d,", domain_connect[j]);

		        printf ("\n node map:\n");
		        for (j=0; j<loc_num_nodes; j++) printf (" %d,", node_map[j]);

		        printf ("\n local connectivity:\n");
		        for (j=0; j<len_connect; j++) printf (" %d,", loc_connect[j]);
		}


		/* Declare collections of nodes, elements, and domains on current set */
		saf_declare_collection(SAF_ALL, &domain_set, &node_cat, SAF_CELLTYPE_POINT, 
			loc_num_nodes, SAF_1DC(loc_num_nodes), 
			SAF_DECOMP_FALSE);
		saf_declare_collection(SAF_ALL, &domain_set, &elem_cat, SAF_CELLTYPE_QUAD, 
			loc_num_elements, SAF_1DC(loc_num_elements), 
			SAF_DECOMP_TRUE);
		saf_declare_collection(SAF_ALL, &domain_set, &domain_cat, SAF_CELLTYPE_SET, 
			1, SAF_1DC(1), SAF_DECOMP_TRUE);
                          
		/* Declare subset relations of elements, nodes, and domains */

		saf_declare_subset_relation(SAF_ALL, db, &top_set, &domain_set, 
			SAF_COMMON(&elem_cat),
			SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
		saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, elem_map, H5I_INVALID_HID, NULL, supplemental_file );

		saf_declare_subset_relation(SAF_ALL, db, &top_set, &domain_set, 
			SAF_COMMON(&node_cat),
			SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
		saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, node_map, H5I_INVALID_HID, NULL, supplemental_file );

		saf_declare_subset_relation(SAF_ALL, db, &top_set, &domain_set, 
			SAF_COMMON(&domain_cat),
			SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
		saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, &i, H5I_INVALID_HID, NULL, supplemental_file );

		/* Declare topology relations;
		   local elements, local nodes */
		saf_declare_topo_relation(SAF_ALL, db, &domain_set, &elem_cat, 
			&domain_set, &node_cat,
			SAF_SELF(db), &domain_set, SAF_UNSTRUCTURED,
			H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &rel);
		saf_write_topo_relation(SAF_ALL, &rel, SAF_INT, num_nodes_per_elem, 
			SAF_INT, loc_connect, supplemental_file );

      /* Declare topology relations;
         local elements, global nodes;
         haven't tried this, yet */
      /******************************************************************
      saf_declare_topo_relation(SAF_ALL, db, &top_set, &elem_cat, 
                                &top_set, &node_cat,
                                domain_cat, domain_set, SAF_UNSTRUCTURED,
                                NULL, NULL, NULL, NULL, &rel);
      saf_write_topo_relation(SAF_ALL, &rel, SAF_INT, num_nodes_per_elem, 
                              SAF_INT, domain_connect, supplemental_file );
      ********************************************************************/

      /*
       * Declare Coordinate Field Templates
       */ 

      sprintf(
         temporary_name,
         "X_COORD_TMPL_%d",
         i
      );

      saf_declare_field_tmpl(
         SAF_ALL, db,
         temporary_name,
         SAF_ALGTYPE_SCALAR,
         SAF_CARTESIAN,
         /* SAF_NOT_SET_QUANTITY, */ SAF_QLENGTH,
         1, 
         NULL,
         &x_component_ftempl
      );

      sprintf(
         temporary_name,
         "Y_COORD_TMPL_%d",
         i
      );

      saf_declare_field_tmpl(
         SAF_ALL,
         /* domain_set, */
         db,
         temporary_name,
         SAF_ALGTYPE_SCALAR,
         SAF_CARTESIAN,
         /* was  SAF_NOT_SET_QUANTITY, */ SAF_QLENGTH,
         1, 
         NULL,
         &y_component_ftempl
      );

      sprintf(
         temporary_name,
         "XY_COORD_TMPL_%d",
         i
      );

      xy_component_ftempl[0] = x_component_ftempl;
      xy_component_ftempl[1] = y_component_ftempl;
      
      saf_declare_field_tmpl(
         SAF_ALL,
         /* domain_set, */
         db,
         temporary_name,
         SAF_ALGTYPE_VECTOR,
         SAF_CARTESIAN,
         /* was SAF_NOT_SET_QUANTITY, */ SAF_QLENGTH,
         2, 
         xy_component_ftempl,
         &xy_composite_ftempl
      );

      /* Declare coordinate fields:
           declare x and y component fields;
           write out x and y component fields;
           declare xy composite field as composed of x and y component fields 

         To simulate results variables, create and write out 
           "num_fields" fields;
           we'll just write out the x coordinates for this
      */

      /* Extract the local x and y coordinates */

      if (i == 0) {  /* first time through; max size occurs on
                        first iteration */
        loc_x_coords = (float *) malloc (loc_num_nodes * sizeof(float));
        loc_y_coords = (float *) malloc (loc_num_nodes * sizeof(float));
      }

      for (j=0; j<loc_num_nodes; j++) {
        loc_x_coords[j] = x[node_map[j]-map_origin];
        loc_y_coords[j] = y[node_map[j]-map_origin];
      }

      /* these are the coordinates (a "distinguished" field) */

      sprintf (temporary_name, "X_COORDS_%d", i);
      saf_declare_field(SAF_ALL,db, &x_component_ftempl, 
                        temporary_name, &domain_set, NULL, SAF_SELF(db), 
                        SAF_NODAL(&node_cat, &elem_cat), H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, 
                        SAF_IDENTITY, NULL, &x_coords);

      saf_write_field (SAF_ALL, &x_coords, SAF_WHOLE_FIELD, 1,  H5I_INVALID_HID, (void**)&loc_x_coords, 
                       supplemental_file);

      sprintf (temporary_name, "Y_COORDS_%d", i);
      saf_declare_field(SAF_ALL, db, &y_component_ftempl, 
                        temporary_name, &domain_set, NULL, SAF_SELF(db), 
                        SAF_NODAL(&node_cat, &elem_cat), H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, 
                        SAF_IDENTITY, NULL, &y_coords);

      saf_write_field (SAF_ALL, &y_coords, SAF_WHOLE_FIELD, 1,  H5I_INVALID_HID, (void**)&loc_y_coords, 
                       supplemental_file);

      if (i==0) {
        comp_flds = (SAF_Field *) malloc (2 * sizeof(SAF_Field));
      }
      comp_flds[0] = x_coords;
      comp_flds[1] = y_coords;
      sprintf (temporary_name, "XY_COORDS_%d", i);
      saf_declare_field(SAF_ALL, db, &xy_composite_ftempl, 
                        temporary_name, &domain_set, NULL, SAF_SELF(db), 
                        SAF_NODAL(&node_cat, &elem_cat), H5T_NATIVE_FLOAT, comp_flds, SAF_INTERLEAVE_VECTOR, 
                        SAF_IDENTITY, NULL, &xy_coords);

      saf_declare_coords(SAF_ALL, &xy_coords);

      /* these are the simulated results variables */

      for (j=0; j<num_fields; j++) {
        sprintf (temporary_name, "FIELD_%d_%d", i,j);
        saf_declare_field(SAF_ALL, db, &x_component_ftempl, 
                          temporary_name, &domain_set, NULL, SAF_SELF(db), 
                          SAF_NODAL(&node_cat, &elem_cat), H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, 
                          SAF_IDENTITY, NULL, &x_coords);

        saf_write_field (SAF_ALL, &x_coords, SAF_WHOLE_FIELD, 1,  H5I_INVALID_HID, (void**)&loc_x_coords, 
                         supplemental_file);

      }

    }

    /* Declare topology relations;
       global nodes, global elements */
    /******************************************************************
    saf_declare_topo_relation(SAF_ALL, top_set, elem_cat, 
                              top_set, node_cat,
                              SAF_SELF(db), top_set, SAF_UNSTRUCTURED,
                              NULL, NULL, NULL, NULL, &rel);
    saf_write_topo_relation(SAF_ALL, rel, SAF_INT, num_nodes_per_elem, 
                            SAF_INT, connect, supplemental_file);
    ********************************************************************/

    /* Close database */
    saf_close_database(db);


    /* Properly end session */
    saf_final();

	/*
	 * Free Memory
	 */

	free( comp_flds       );
	free( domain_connect );
	free( elem_map       );
	free( loc_connect    );
	free( loc_x_coords    );
	free( loc_y_coords    );
	free( node_map       );
  }
  SAF_CATCH
  {
    SAF_CATCH_ALL
    {
    }
  }
  SAF_TRY_END
}

#ifdef HAVE_EXODUS
/***********************************************************************
 ***********************************************************************/
void write_exo_mesh(
	int	debug,
	char	*file_name,
	int	map_origin,
	int	num_elements,
	int	num_domains,
	int	num_fields,
	float	*x,
	float	*y, 
	int	*connect
) {
  int CPU_word_size=0;
  int IO_word_size=4;
  int exoid, err, num_dim, num_elem_blk, num_node_sets, num_side_sets;
  int i, j, index, loc_num_elements, loc_num_nodes, len_connect;
  int *elem_map, *node_map, *domain_connect, *loc_connect;

  float *z, *loc_xcoords, *loc_ycoords;

  char temporary_name[MAX_STRING_LEN];
  char **var_name;

  for (i=0; i<num_domains; i++) {

    /* create the EXODUS file */

    sprintf (temporary_name, "%s_%d.ex2", file_name, i);
    exoid = ex_create (temporary_name, EX_CLOBBER, &CPU_word_size, &IO_word_size);

    if (exoid < 0) {
      printf ("after ex_create, error = %d\n", err);
      exit(-1);
    }

    /* Determine local number of elements */
    if (num_elements < num_domains) 
    {
      printf ("number of elements is less than number of domains.\n");
      if (i < num_elements) loc_num_elements = 1;
        else loc_num_elements = 0;
    } else {
      loc_num_elements = num_elements / num_domains;
      if (i < (num_elements % num_domains)) loc_num_elements++;
    }

    len_connect = NUM_NODES_PER_ELEM * loc_num_elements;

    /* malloc things we need */

    if (i == 0) {  /* first time through; max size arrays occur on
                      first iteration */
      elem_map = (int *) malloc (loc_num_elements * sizeof(int));
      domain_connect = (int *) malloc (len_connect * sizeof(int));
      loc_connect = (int *) malloc (len_connect * sizeof(int));
      node_map = (int *) malloc (len_connect * sizeof(int));
    }

    /* Create element local/global map */

	create_rr_elem_map(
		loc_num_elements,
		elem_map,
		map_origin,
		num_domains,
		i
	);

    /* Extract current domain's connectivity, referencing global node ids */

	extract_connect(
		num_elements,
		elem_map,
		connect,
		domain_connect,
		map_origin
	);

    /* The local/global node map is just the current domain's connectivity,
       sorted with duplicate entries removed */
    create_node_map (len_connect, domain_connect, node_map, &loc_num_nodes);

    /* Using local/global node map, convert the domain connectivity 
       (referencing global node ids) to local connectivity (referencing 
       local node ids) */

	create_local_connect(
		node_map,
		loc_num_nodes, 
		len_connect,
		domain_connect,
		loc_connect,
		map_origin
	);

	if ( debug ) {
		printf ("\n\n\n");

		printf ("\n domain: %d\n", i);
		printf ("\n loc_num_elements: %d\n", loc_num_elements);
		printf ("\n loc_num_nodes: %d\n", loc_num_nodes);

		printf ("\n element map:\n");
		for (j=0; j<loc_num_elements; j++) printf (" %d,", elem_map[j]);

		printf ("\n domain connectivity:\n");
		for (j=0; j<len_connect; j++) printf (" %d,", domain_connect[j]);

		printf ("\n node map:\n");
		for (j=0; j<loc_num_nodes; j++) printf (" %d,", node_map[j]);

		printf ("\n local connectivity:\n");
		for (j=0; j<len_connect; j++) printf (" %d,", loc_connect[j]);
	}

    num_dim = 2;
    num_elem_blk = 1;
    num_node_sets = 0;
    num_side_sets = 0;

    err = ex_put_init (exoid, "This is a SAF performance test.", num_dim, 
                       loc_num_nodes, loc_num_elements, num_elem_blk, 
                       num_node_sets, num_side_sets);

    if (err) {
      printf ("after ex_put_init, error = %d\n", err);
      ex_close (exoid);
      exit(-1);
    }

    /* Extract the local x and y coordinates */

    if (i == 0) {  /* first time through; max size occurs on
                      first iteration */
      loc_xcoords = (float *) malloc (loc_num_nodes * sizeof(float));
      loc_ycoords = (float *) malloc (loc_num_nodes * sizeof(float));
    }

    for (j=0; j<loc_num_nodes; j++) {
      index = node_map[j] - map_origin;
      loc_xcoords[j] = x[index];
      loc_ycoords[j] = y[index];
    }

    err = ex_put_coord (exoid, loc_xcoords, loc_ycoords, z);

    if (err) {
      printf ("after ex_put_coord, error = %d\n", err);
      ex_close (exoid);
      exit(-1);
    }

    err = ex_put_elem_block 
        (exoid, 10, "quad", loc_num_elements, NUM_NODES_PER_ELEM, 0);

    if (err) {
      printf ("after ex_put_elem_block, error = %d\n", err);
      ex_close (exoid);
      exit(-1);
    }

    err = ex_put_elem_conn (exoid, 10, loc_connect);

    if (err) {
      printf ("after ex_put_elem_conn, error = %d\n", err);
      ex_close (exoid);
      exit(-1);
    }

    /* write out element and node maps */

    err = ex_put_node_num_map (exoid, node_map);

    if (err) {
      printf ("after ex_put_node_num_map, error = %d\n", err);
      ex_close (exoid);
      exit(-1);
    }

    err = ex_put_elem_num_map (exoid, elem_map);

    if (err) {
      printf ("after ex_put_elem_num_map, error = %d\n", err);
      ex_close (exoid);
      exit(-1);
    }

    /* write out simulated results fields;
       we'll just write out the x coordinate field 'num_fields' times */

    if (num_fields > 0) {
      err = ex_put_var_param (exoid, "n", num_fields);
      if (err) {
        printf ("after ex_put_var_param, error = %d\n", err);
        ex_close (exoid);
        exit(-1);
      }

      var_name = (char **) malloc (num_fields * sizeof(char *));
      for (j=0; j<num_fields; j++) {
        var_name[j] = (char *) malloc ((MAX_STRING_LEN+1) * sizeof (char));
        sprintf (var_name[j], "field_%d", j+1);
        err = ex_put_nodal_var (exoid, 1, j+1, loc_num_nodes, loc_xcoords);
        if (err) {
          printf ("after ex_put_nodal_var, error = %d\n", err);
          ex_close (exoid);
          exit(-1);
        }
      }

      err = ex_put_var_names (exoid, "n", num_fields, var_name);
      if (err) {
        printf ("after ex_put_nodal_var, error = %d\n", err);
        ex_close (exoid);
        exit(-1);
      }
    }

    err = ex_close (exoid);

    if (err) {
      printf ("after ex_close, error = %d\n", err);
      exit(-1);
    }
  }

	/*
	 * Free Memory
	 */

	free( domain_connect );
	free( elem_map       );
	free( loc_connect    );
	free( loc_xcoords    );
	free( loc_ycoords    );
	free( node_map       );
	free( var_name       );
}
#endif

/***********************************************************************
 *
 * Create element local/global map 
 *
 * This doles out the elements to each domain in a round robin fashion.
 * This will be the worst case for recombining the elements to global
 * order.
 *
 ***********************************************************************/

void create_rr_elem_map(
	int loc_num_elements,
	int *elem_map,
	int map_origin,
	int num_domains,
	int current_domain
) {
	int i;

	for ( i = 0; i < loc_num_elements; i++ )
    		elem_map[i] = (i * num_domains) + current_domain + map_origin;

}

/***********************************************************************
 *
 * Create element local/global map
 *
 * This puts contiguous groups of elements in each domain.  This is
 * a reasonable map for a realistic application.
 *
 ***********************************************************************/
void create_elem_map (
        int loc_num_elems, 
        int elem_num, 
        int *elem_map)
{
  int i;

  for (i=0; i<loc_num_elems; i++) {
    elem_map[i] = elem_num++;
  }

}

/***********************************************************************
 *
 * Extract current domain's connectivity, referencing global node ids
 *
 * This extracts the "domain connectivity," that is, the connectivity 
 * of the elements in the current domain.  The node ids in the domain 
 * connectivity reference global node ids.
 *
 ***********************************************************************/

void extract_connect(
	int num_elem,
	int *elem_map,
	int *connect, 
        int *domain_connect,
	int map_origin
) {
  int i, j, k, m, offset;

  for (i=0, j=0, m=0; i<num_elem; i++) {
    if (elem_map[j] == i+map_origin) {  /* extract this element */
      offset = (i * NUM_NODES_PER_ELEM);
      for (k=offset; k < offset+NUM_NODES_PER_ELEM; k++) {
        domain_connect[m++] = connect[k];
      }
      j++;
    }
  }
}

/***********************************************************************
 *
 * The local/global node map is just the current domain's connectivity,
 * sorted, with duplicate entries removed.  This isn't obvious, but
 * trust me.
 *
 ***********************************************************************/
void create_node_map (int len_connect, int *domain_connect, int *node_map,
                      int *loc_num_nodes)
{
  int cnt, i;

  *loc_num_nodes = len_connect;

  /* copy the domain connectivity to the node map */
  memcpy (node_map, domain_connect, (size_t)(len_connect*NUM_BYTES_PER_INT));

  /* sort the node map */
  sort_int (*loc_num_nodes, node_map);

  /* now remove duplicate entries */
  for (cnt=0, i=1; i<(*loc_num_nodes); i++) {
    if (node_map[cnt] != node_map[i]) {
      node_map[++cnt] = node_map[i];
    }
  }

  *loc_num_nodes = cnt+1;

}

/***********************************************************************
 *
 * Using local/global node map, convert the domain connectivity 
 * (referencing global node ids) to local connectivity (referencing 
 * local node ids).
 *
 * This requires inverting the local/global map, a relatively expensive
 * operation.  The procedure is:
 *
 *   for every entry in the domain connectivity
 *     search the node map until found
 *     set the value of the entry in the local connectivity to 
 *       the index of the located value in the node map
 *
 ***********************************************************************/
void create_local_connect(
	int *node_map,
	int len_node_map, 
	int len_connect,
	int *domain_connect, 
        int *loc_connect,
	int map_origin
) {
  int i, index;

  for (i=0; i<len_connect; i++) {
    index = bin_search2 (domain_connect[i], len_node_map, node_map);
    if (index == -1) {  /* not found */
      fprintf (stderr, "error creating local connectivity; i = %d\n", i);
      exit (-1);
    } else {
      loc_connect[i] = index+map_origin;
    }
  }
}



/*****************************************************************************
 *
 *       Numerical Recipies in C source code
 *       modified to have first argument an integer array
 *
 *       Sorts the array ra[0,..,(n-1)] in ascending numerical order using
 *       heapsort algorithm.
 *
 *****************************************************************************/

void sort_int(int n, int ra[])

{
  int   l, j, ir, i;
  int   rra;

  /*
   *  No need to sort if one or fewer items.
   */
  if (n <= 1) return;

  l=n >> 1;
  ir=n-1;
  for (;;) {
    if (l > 0)
      rra=ra[--l];
    else {
      rra=ra[ir];
      ra[ir]=ra[0];
      if (--ir == 0) {
        ra[0]=rra;
        return;
      }
    }
    i=l;
    j=(l << 1)+1;
    while (j <= ir) {
      if (j < ir && ra[j] < ra[j+1]) ++j;
      if (rra < ra[j]) {
        ra[i]=ra[j];
        j += (i=j)+1;
      }
      else j=ir+1;
    }
    ra[i]=rra;
  }
}


/*****************************************************************************
 *
 * Searches a monotonic list of values for the value, value.
 * It returns the index (0-based) of the first position found, which 
 *   matches value.
 * The list is assumed to be monotonic, and consist of elements 
 *   list[0], ..., list[n-1].
 * If no position in list matches value, it returns the value -1.
 *
 *****************************************************************************/

int bin_search2 (int value, int num, int List[])

{

 register int top, bottom = 0, middle, g_mid;

 /***** execution begins *****/

 top = num - 1;
 while (bottom <= top) {
   middle = (bottom + top) >> 1;
   g_mid = List[middle];
   if (value < g_mid)
     top = middle - 1;
   else if (value > g_mid)
     bottom = middle + 1;
   else
     return middle;     /* found */
 }

 return -1;

} /* bin_search2 */

