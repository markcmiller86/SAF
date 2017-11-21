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
#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <saf.h>
#include <testutil.h>


/* this function is needed for testing, here but is otherwise private to SAF and we don't want to include all of safP.h here */
hbool_t _saf_is_self_decomp(SAF_Cat *cat);

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Tests 
 * Purpose:     Simple structured grid for terrain height field
 *
 * Description: This test creates some basic structured grid meshes to test SAF's support for structured topology
 *		relations. This test relies upon a data file, 47N4E.raw, containing the terrain height data for
 *		a 257x257 (thats 257 nodes and 256 zones) region of terrain near 47N latitude, 4E longitude
 *		(somewhere in France). The source for this data is the Defense Mapping Agency's Digital Terrain
 *		Elevation Data (DTED). This is a file of byte values indicating the terrain elevation in meters.
 *		The samples are spaced 100 meters apart in lat/lon.
 *
 *		This test is meant to be extended to include a few different representations for the terrain elevation data.
 *		The first (and currently only implemented) representation is a simple piecewise linear height field.
 *
 *		The second is a piecewise flat representation in which each cell is bisected into two triangles by a diagonal
 *		connecting either the NW & SE corners or the NE & SW corners. This representation creates some problems for
 *		SAF in that the evaluation function representing the resulting piecewise flat field depends on two sets of
 *		dofs that are associated with different collections. The first set of is the terrain heights known at the nodes.
 *		The second set is a rather unusual, binary dof, indicating which of the two diagonals was chosen in the cell.
 *		See ??? for a detailed description of this representation and the issues it reveals.
 *
 *		The third is a pyramid of resolution representation in which we define new collections for each level in
 *		the pyramid. Unfortunately, SAF has an upper limit, determined at compile time, on the number of collection
 *		categories one can define. So, we don't define collections for every level in the pyramid. We define them
 *		for every other level. Note in all cases, the height field is defined on the same base-space set. It is
 *		just implemented differently for each level of resolution.
 *---------------------------------------------------------------------------------------------------------------------------------
 */

int
main(int argc, char **argv)
{
    char dbname[1024]="terrain.saf";
    int i, fd, ntasks=1;
    SAF_Db *db=NULL;
    SAF_DbProps *dbprops=NULL;
    SAF_Cat nodes, cells;
    SAF_Cat resNodes[16], resCells[16]; 
    SAF_Set base_mesh, mountain, box;
    SAF_Rel rel, trel;
    SAF_FieldTmpl coords_ftmpl, coords_ctmpl, tmp_ftmpl[2], height_tmpl;
    SAF_Field coords, coord_compon[2], height_field;
    SAF_Unit umeter;
    int Nx = 257, Ny = 257;
    int L = 0;
    unsigned char *tmpBuf;
    int result = 0;

#ifdef HAVE_PARALLEL
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ntasks);
#endif
    ntasks = ntasks; /* quiet the compiler */

    chdir(TEST_FILE_PATH);
#ifdef HAVE_PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    /* lets read the raw elevation data to make sure we've got it before we continue */
    TESTING("reading raw elevation data file");
    tmpBuf = (unsigned char *) malloc(Nx * Ny * sizeof(unsigned char));
    if ((fd = open("47N4E.raw", O_RDONLY)) < 0) {
        FAILED;
        return 1;
    }
    if (read(fd, tmpBuf, Nx*Ny*sizeof(unsigned char)) < Nx*Ny*(int)sizeof(unsigned char)) {
        FAILED;
        return 1;
    }
    close(fd);
    PASSED;

    /* for convenience, set working directory to the test file directory */

    saf_init(SAF_DEFAULT_LIBPROPS);

    /*only use TEST_FILE_NAME if the command-line didnt contain a filename*/
    if(!strlen(dbname)) strcpy(dbname, TEST_FILE_NAME);

    SAF_TRY_BEGIN {

        /* note: because we are in a try block here, all failures will send us to the one and only
	 catch block at the end of this test */

        dbprops = saf_createProps_database();
        saf_setProps_Clobber(dbprops);
        db = saf_open_database(dbname,dbprops);

        TESTING("declaring base categories");
        saf_declare_category(SAF_ALL, db, "nodes", SAF_TOPOLOGY, 0, &nodes);
        saf_declare_category(SAF_ALL, db, "cells", SAF_TOPOLOGY, 2, &cells);
        PASSED;

        /* compute number of levels in resolution pyramid */
        while ((1<<L) < Nx)
            L++;

        /* we declare every other resolution level because we'd wind up using too many categories */
        TESTING("declaring resolution categories");
        for (i = L-1; i >= 0; i -= 2) {
            char tmpCatName[32];
            sprintf(tmpCatName,"nodes%d", i);
            saf_declare_category(SAF_ALL, db, tmpCatName, SAF_TOPOLOGY, 0, &resNodes[i]);
            sprintf(tmpCatName,"cells%d", i);
            saf_declare_category(SAF_ALL, db, tmpCatName, SAF_TOPOLOGY, 2, &resCells[i]);
        }
        PASSED;


        /*------------------------------------------------------------------------------------------------------------------------ 
         *                                     DECLARE BASE TERRAIN MESH AND ELEVATION DATA
         *---------------------------------------------------------------------------------------------------------------------  */
        TESTING("declaring base terrain mesh");
        saf_declare_set(SAF_ALL, db, "terrain_mesh", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &base_mesh);
        saf_declare_collection(SAF_ALL, &base_mesh, &nodes, SAF_CELLTYPE_POINT, Nx*Ny, SAF_2DC(Ny,Nx), SAF_DECOMP_FALSE); 
        saf_declare_collection(SAF_ALL, &base_mesh, &cells, SAF_CELLTYPE_QUAD, (Nx-1)*(Ny-1), SAF_2DC(Ny-1,Nx-1), SAF_DECOMP_TRUE);
        saf_declare_topo_relation(SAF_ALL, db, &base_mesh, &cells, &base_mesh, &nodes, SAF_SELF(db), &base_mesh,
                                  SAF_STRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &trel);

        /* coordinate field templates */
        saf_declare_field_tmpl(SAF_ALL, db, "coords_ctmpl", SAF_ALGTYPE_SCALAR,SAF_CARTESIAN,SAF_QLENGTH,1, NULL, &coords_ctmpl);
        tmp_ftmpl[0] = coords_ctmpl;
        tmp_ftmpl[1] = coords_ctmpl;
        saf_declare_field_tmpl(SAF_ALL, db, "coords_ftmpl", SAF_ALGTYPE_VECTOR,SAF_CARTESIAN,SAF_QLENGTH,2,tmp_ftmpl,
                               &coords_ftmpl);
        saf_find_one_unit(db, "meter", &umeter);

        /* coordinate field (we declare uniform coordinates: an origin and a delta for each coordinate compononent) */ 
        saf_declare_field(SAF_ALL, db, &coords_ctmpl,      "X", &base_mesh, &umeter, SAF_SELF(db), SAF_SELF(db), 2, &cells,
                          SAF_SPACE_UNIFORM, H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(coord_compon[0]));
        saf_declare_field(SAF_ALL, db, &coords_ctmpl,      "Y", &base_mesh, &umeter, SAF_SELF(db), SAF_SELF(db), 2, &cells,
                          SAF_SPACE_UNIFORM, H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(coord_compon[1]));
        saf_declare_field(SAF_ALL, db, &coords_ftmpl, "coords", &base_mesh, &umeter, SAF_SELF(db), SAF_SELF(db), 2, &cells,
                          SAF_SPACE_UNIFORM, H5T_NATIVE_FLOAT, coord_compon, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &coords); 
        {
            float buf[] = {0., 1.,   0., 1.}; /* origin = 0.0, delta = 1.0 along each axis */
            void *pbuf = &buf[0];
            saf_write_field(SAF_ALL, &coords, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, NULL); 
        }

        /* height field (the reason for the whole example) */
        saf_declare_field_tmpl(SAF_ALL, db, "height_tmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1, NULL, &height_tmpl);
        saf_declare_field(SAF_ALL, db, &height_tmpl, "elevation", &base_mesh, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &cells),
                          H5T_NATIVE_UCHAR, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &height_field);
        {
            void *pbuf = &tmpBuf[0];
            saf_write_field(SAF_ALL, &height_field, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, NULL); 
        }
        PASSED;

        /*------------------------------------------------------------------------------------------------------------------------ 
         *                                     DECLARE A COUPLE OF INTERESTING SUBSETS 
         *---------------------------------------------------------------------------------------------------------------------  */
        TESTING("writing a SAF_TUPLES subset");
        saf_declare_set(SAF_ALL, db, "mountain", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &mountain);
        saf_declare_collection(SAF_ALL, &mountain, &cells, SAF_CELLTYPE_QUAD, 72, SAF_1DC(72), SAF_DECOMP_TRUE);
        saf_declare_subset_relation(SAF_ALL, db, &base_mesh, &mountain, SAF_COMMON(&cells), SAF_TUPLES, SAF_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel);
        {
            int buf[] = { 214,97,215,97,216,97,217,97,218,97,219,97,220,97,221,97,222,97,223,97,224,97,225,97,226,97,227,97,228,97,
                          229,97,230,97,
		     
                          214,98,215,98,216,98,217,98,218,98,219,98,220,98,221,98,222,98,223,98,224,98,225,98,226,98,227,98,228,98,
                          229,98,230,98,231,98,
		     
                          215,99,216,99,217,99,218,99,219,99,220,99,221,99,222,99,223,99,224,99,225,99,226,99,227,99,228,99,229,99,
                          230,99,231,99,232,99,
		     
                          215,100,216,100,217,100,218,100,219,100,220,100,221,100,222,100,223,100,224,100,225,100,226,100,227,100,
		     
                          228,100,229,100,230,100,231,100,232,100,233,100};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, NULL);
        }
        PASSED;

        TESTING("writing a SAF_HSLAB subset");
        saf_declare_set(SAF_ALL, db, "some box", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &box);
        saf_declare_collection(SAF_ALL, &box, &cells, SAF_CELLTYPE_QUAD, 105*77, SAF_2DC(105,77), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &box, &nodes, SAF_CELLTYPE_POINT, 106*78, SAF_2DC(106,78), SAF_DECOMP_FALSE);
        saf_declare_subset_relation(SAF_ALL, db, &base_mesh, &box, SAF_COMMON(&cells), SAF_HSLAB, SAF_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel);
        {
            int buf[] = {68,77,1, 47,105,1};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, NULL);
        }
        saf_declare_subset_relation(SAF_ALL, db, &base_mesh, &box, SAF_COMMON(&nodes), SAF_HSLAB, SAF_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel);
        {
            int buf[] = {68,78,1, 47,106,1};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, NULL);
        }
        PASSED;

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            result = 1;
	}
    } SAF_TRY_END;

    saf_close_database(db);
    saf_final();

#ifdef HAVE_PARALLEL
    /* make sure everyone returns the same error status */
    MPI_Bcast(&result, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Finalize();
#endif

    return result;

}
