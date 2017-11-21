
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
#include <testutil.h>

int
main(int argc,
     char **argv)
{

    int i, rank, size;
    unsigned power[7];

    SAF_Quantity quant;
    SAF_DbProps *dbprops=NULL;
    SAF_Db *db=NULL;
    SAF_Set        domain;
    SAF_FieldTmpl  fld_tmpl;
    hbool_t  failed;

#ifdef HAVE_PARALLEL
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
#endif

#ifdef HAVE_PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    saf_init(SAF_DEFAULT_LIBPROPS);

    TESTING("commits of global-only objects during SAF_EACH calls");

    dbprops = saf_createProps_database();
    saf_setProps_Clobber(dbprops);
    db = saf_open_database("test.saf",dbprops);

    saf_declare_set(SAF_EACH, db, "domain", SAF_TOPODIM_3D, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &domain);

    saf_declare_field_tmpl(SAF_EACH, db, "field_tmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1, NULL, &fld_tmpl);
    saf_describe_field_tmpl(SAF_EACH, &fld_tmpl, NULL,  NULL, NULL, &quant, NULL, NULL); 
    saf_describe_quantity (SAF_EACH, &quant, NULL, NULL, NULL, NULL, power);
    failed = false;
    for (i=0; i<7; i++) {
        if (i == 3) {
            if (power[i] != 1)
                failed = true;
        } else {
            if (power[i] != 0)
                failed = true;
        }
    }

    if (failed)
        FAILED;
    else
        PASSED;

    saf_close_database(db);

    saf_final();

#ifdef HAVE_PARALLEL
    MPI_Finalize();
#endif

    return failed;

}
