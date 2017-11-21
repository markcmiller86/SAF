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
#include <safP.h>
#include <math.h>
#include <testutil.h>

#ifdef M_PI
#   define SAF_M_PI     M_PI
#else
#   define SAF_M_PI     3.14159265358979323846  /* pi */
#endif

#ifdef HAVE_ATTRIBUTE
#   define __UNUSED__ __attribute__((__unused__))
#else
#   define __UNUSED__ 
#endif

#define URL "standard"

/* used in place of numeric 0 to quiet compiler warnings */

/*--------------------------------------------------------------------------------------------------------------------------------
 * Small utility macros
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define SQUARE(X)       ((X)*(X))

/*--------------------------------------------------------------------------------------------------------------------------------
 * Macros for defining quantities
 *--------------------------------------------------------------------------------------------------------------------------------
 */

/* Start a new quantity definition */
#define Q_START(DESC,ABBR) {                                                                                                   \
    SAF_Quantity *quantity;                                                                                                    \
    if (NULL==(quantity=saf_declare_quantity(SAF_ALL, database, DESC, ABBR, URL, NULL))) {                                     \
        fprintf(stderr, "%s:%u: cannot declare quantity \"%s\"\n", __FILE__, __LINE__, DESC);                                  \
        return -1;                                                                                                             \
    }

/* End a quantity definition */
#define Q_END()                                                                                                                \
    _saf_free(quantity);                                                                                                       \
}

/* Multiply or divide quantity WHAT into quantity being defined */
#define Q_NUM(WHAT,POWER) saf_multiply_quantity(SAF_ALL, quantity, WHAT, POWER)
#define Q_DEN(WHAT,POWER) saf_divide_quantity(SAF_ALL, quantity, WHAT, POWER)

/*--------------------------------------------------------------------------------------------------------------------------------
 * Macros for defining units
 *--------------------------------------------------------------------------------------------------------------------------------
 */


/* Beginning of a function that declares some units, matched with U_FINISH */
#define U_DCLS(ME) {                                                                                                           \
    SAF_Unit *unit=NULL;                                                                                                       \
    use_unit_variables(database);
 
/* Beginning of a unit declaration. The ABBR is optional (may be NULL).  Matched with U_END(). */
#define U_START(NAME,ABBR)                                                                                                     \
    do {                                                                                                                       \
        if (unit) _saf_free(unit); /*previous definition*/                                                                     \
        if (NULL==(unit=saf_declare_unit(SAF_ALL, database, #NAME, ABBR, URL, NULL))) {                                        \
            fprintf(stderr, "%s:%u: unable to declare unit \"%s\"\n", __FILE__, __LINE__, #NAME);                              \
            return -1;                                                                                                         \
        }

/* Multiply COEF * UNIT^POWER into the unit being defined. */
#define U_MULT(COEF,UNIT,POWER)                                                                                                \
        {                                                                                                                      \
            /* Find multiplicand by singular name or abbreviation */                                                           \
            SAF_Unit *mult = saf_find_one_unit(database, #UNIT, NULL);                                                         \
            if (!mult) {                                                                                                       \
                fprintf(stderr, "%s:%u: cannot locate multiplicand unit \"%s\"\n", __FILE__, __LINE__, #UNIT);                 \
                return -1;                                                                                                     \
            }                                                                                                                  \
            if (saf_multiply_unit(SAF_ALL, unit, (double)COEF, mult, POWER)<0) {                                               \
                fprintf(stderr, "%s:%u: unable to multply unit by \"%s\"\n", __FILE__, __LINE__, #UNIT);                       \
                return -1;                                                                                                     \
            }                                                                                                                  \
            _saf_free(mult);                                                                                                   \
        }

/* End of a unit definition. QUANTITY is the quantity that the unit measures */
#define U_END(QUANTITY)                                                                                                        \
        {                                                                                                                      \
            SAF_Quantity *quantity;                                                                                            \
            if (NULL==(quantity=saf_find_one_quantity(database, #QUANTITY, NULL))) {                                           \
                fprintf(stderr, "%s:%u: unable to find quantity \"%s\"\n", __FILE__, __LINE__, #QUANTITY);                     \
                return -1;                                                                                                     \
            }                                                                                                                  \
            if (saf_quantify_unit(SAF_ALL, unit, quantity, 1.0)<0) {                                                           \
                fprintf(stderr, "%s:%u: saf_quantify_unit failed for quantity \"%s\"\n", __FILE__, __LINE__, #QUANTITY);       \
                return -1;                                                                                                     \
            }                                                                                                                  \
            _saf_free(quantity);                                                                                               \
        }                                                                                                                      \
    } while (false); /*matches `do' in U_START()*/

/* Define additional units like the previos definition except with various scale factors. */
#define U_SI_PREFIX(UNIT)       U_PREFIX(UNIT,si_prefix)
#define U_EIC_PREFIX(UNIT)      U_PREFIX(UNIT,eic_prefix)
#define U_PREFIX(UNIT,PREFIX)                                                                                                  \
    do {                                                                                                                       \
        size_t i;                                                                                                              \
        char *base_name=NULL, *base_abbr=NULL, *url=NULL, full_name[256], full_abbr[256];                                      \
        double scale, offset, logbase, logcoef;                                                                                \
        SAF_Quantity uquant;                                                                                                   \
        SAF_Unit derived;                                                                                                      \
                                                                                                                               \
        for (i=0; i<NELMTS(PREFIX); i++) {                                                                                     \
            base_name=base_abbr=url=NULL; /*or following call fails*/                                                          \
            saf_describe_unit(SAF_ALL, unit, &base_name, &base_abbr, &url, &scale, &offset, &logbase, &logcoef, &uquant);      \
            sprintf(full_name, "%s%s", base_name?PREFIX[i].prefix:"", base_name?base_name:"");                                 \
            if (PREFIX[i].abbr && base_abbr)                                                                                   \
                sprintf(full_abbr, "%s%s", PREFIX[i].abbr, base_abbr);                                                         \
            else                                                                                                               \
                full_abbr[0] = '\0';                                                                                           \
            if (base_name) free(base_name);                                                                                    \
            if (base_abbr) free(base_abbr);                                                                                    \
            if (url) free(url);                                                                                    \
            if (NULL==saf_declare_unit(SAF_ALL, database, full_name, full_abbr, URL, &derived)) {                              \
                fprintf(stderr, "%s:%u: unable to declare unit \"%s\"\n", __FILE__, __LINE__, full_name);                      \
                return -1;                                                                                                     \
            }                                                                                                                  \
            if (saf_quantify_unit(SAF_ALL, &derived, &uquant, 1.0)<0) {                                                        \
                fprintf(stderr, "%s:%u: unable to quantify unit \"%s\"\n", __FILE__, __LINE__, full_name);                     \
                return -1;                                                                                                     \
            }                                                                                                                  \
            if (saf_multiply_unit(SAF_ALL, &derived, scale*PREFIX[i].coef, NULL, 1)<0) {                                       \
                fprintf(stderr, "%s:%u: unable to set coefficient for unit \"%s\"\n", __FILE__, __LINE__, full_name);          \
                return -1;                                                                                                     \
            }                                                                                                                  \
            if (saf_offset_unit(SAF_ALL, &derived, offset)<0) {                                                                \
                fprintf(stderr, "%s:%u: unable to set offset for unit \"%s\"\n", __FILE__, __LINE__, full_name);               \
                return -1;                                                                                                     \
            }                                                                                                                  \
            if (saf_log_unit(SAF_ALL, &derived, logbase, logcoef)<0) {                                                         \
                fprintf(stderr, "%s:%u: unable to set logorithm for unit \"%s\"\n", __FILE__, __LINE__, full_name);            \
                return -1;                                                                                                     \
            }                                                                                                                  \
        }                                                                                                                      \
    } while (false);

/* End a unit declaration function */
#define U_FINISH                                                                                                               \
    if (unit) _saf_free(unit);                                                                                                 \
}


/*--------------------------------------------------------------------------------------------------------------------------------
 * Macros for defining collection roles
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define ROLE_DECL(NAME) {                                                                                                      \
    SAF_Role role;                                                                                                             \
    saf_declare_role(SAF_ALL, database, #NAME, URL, &role);                                                                    \
}

/*--------------------------------------------------------------------------------------------------------------------------------
 * Macros for defining basis types
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define BASIS_DECL(NAME) {                                                                                                     \
    SAF_Basis basis;                                                                                                           \
    saf_declare_basis(SAF_ALL, database, #NAME, URL, &basis);                                                                  \
}

/*--------------------------------------------------------------------------------------------------------------------------------
 * Macros for defining algebraic types
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define ALGEBRAIC_DECL(NAME,INDIRECT) {                                                                                        \
    SAF_Algebraic atype;                                                                                                       \
    saf_declare_algebraic(SAF_ALL, database, #NAME, URL, INDIRECT, &atype);                                                    \
}

/*--------------------------------------------------------------------------------------------------------------------------------
 * Macros for defining evaluation types
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define EVAL_DECL(NAME) {                                                                                                      \
    SAF_Eval eval;                                                                                                             \
    saf_declare_evaluation(SAF_ALL, database, #NAME, URL, &eval);                                                              \
}

/*--------------------------------------------------------------------------------------------------------------------------------
 * Macros for defining relation representation types
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define RELREP_DECL(NAME,ID) {                                                                                                 \
    SAF_RelRep relrep;                                                                                                         \
    saf_declare_relrep(SAF_ALL, database, #NAME, URL, ID, &relrep);                                                            \
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Some tables used locally to provide the various prefixes common to units...
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/* SI prefixes */
static struct {
    double          coef;
    const char      *prefix;
    const char      *abbr;
} si_prefix[] = {{1e24,     "yotta",        "Y"},
                 {1e21,     "zetta",        "Z"},
                 {1e18,     "exa",          "E"},
                 {1e15,     "peta",         "P"},
                 {1e12,     "tera",         "T"},
                 {1e9,      "giga",         "G"},
                 {1e6,      "mega",         "M"},
                 {1e4,      "myria",        NULL},
                 {1e3,      "kilo",         "k"},
                 {1e2,      "hecto",        "h"},
                 {1e1,      "deca",         "da"}, /*alternate spelling*/
                 {1e1,      "deka",         "da"},
                 {1e-1,     "deci",         "d"},
                 {1e-2,     "centi",        "c"},
                 {1e-3,     "milli",        "m"},
                 {1e-6,     "micro",        NULL},
                 {1e-9,     "nano",         "n"},
                 {1e-12,    "pico",         "p"},
                 {1e-15,    "femto",        "f"},
                 {1e-18,    "atto",         "a"},
                 {1e-21,    "zopto",        "z"},
                 {1e-24,    "yocto",        "y"},
                     
};

/* IEC prefixes */
static struct {
    double          coef;
    const char      *prefix;
    const char      *abbr;
} eic_prefix[] = {{1024,            "kibi",         "Ki"},          /* 2^10 */
                  {1048576,         "mebi",         "Mi"},          /* 2^20 */
                  {1073741824,      "gibi",         "Gi"},          /* 2^30 */
                  {1.099512e12,     "tebi",         "Ti"},          /* 2^40 */
                  {1.1259e15,       "pebi",         "Pi"},          /* 2^50 */
                  {1.152922e18,     "exbi",         "Ei"},          /* 2^60 */
};

/* Just so `unused variable' warnings occur only in one place (instead of in every stubbed function) */
/*ARGSUSED*/
static int
use_unit_variables(SAF_Db __UNUSED__ *database)
{
    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Quantities
 * Purpose:     Initialize a new database
 *
 * Description: All new databases get the seven basic quantities defined.
 *
 * Issues:      Since this database might be on read-only media when it is browsed and since this quantity implementation only
 *              supports permanent quantities, we must also define all the quantities that can be used as memory-targets by
 *              the reader.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Programmer:  Robb Matzke
 *              Thursday, March 30, 2000
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
initOR_quantity(SAF_Db *database)
{
    int retval = SAF_SUCCESS;

    TESTING("quantities");
    SAF_TRY_BEGIN {

    /* Get handles for the seven basic quantities that are part of every registry */
    SAF_Quantity        *TIME    = SAF_QTIME;
    SAF_Quantity        *MASS    = SAF_QMASS;
    SAF_Quantity        *CURRENT = SAF_QCURRENT;
    SAF_Quantity        *LENGTH  = SAF_QLENGTH;
    SAF_Quantity        *LIGHT   = SAF_QLIGHT;
    SAF_Quantity        *TEMP    = SAF_QTEMP;
    SAF_Quantity        *AMOUNT  = SAF_QAMOUNT;
    
    /* DEFINITION                                       EXAMPLE UNIT
     * --------------------------------------------     ------------------------------- */

    Q_START("not applicable", "NA");
    Q_END();
    
    Q_START("area", NULL);                              /* hectare                      */
    Q_NUM(LENGTH, 2);
    Q_END();

    Q_START("volume", NULL);                            /* liter                        */
    Q_NUM(LENGTH, 3);
    Q_END();

    Q_START("velocity", "vel"); /*or speed*/            /* meter per second             */
    Q_NUM(LENGTH, 1);
    Q_DEN(TIME, 1);
    Q_END();

    Q_START("acceleration", "accel");                   /* meter per second squared     */
    Q_NUM(LENGTH, 1);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("wave number", NULL);                       /* reciprocal meter             */
    Q_DEN(LENGTH, 1);
    Q_END();

    Q_START("mass density", NULL);                      /* kilogram per cubic meter     */
    Q_NUM(MASS, 1);
    Q_DEN(LENGTH, 3);
    Q_END();

    Q_START("specific length", NULL);                   /* meter per kilogram           */
    Q_NUM(LENGTH, 1);
    Q_DEN(MASS, 1);
    Q_END();

    Q_START("specific volume", NULL);                   /* cubic meter per kilogram     */
    Q_NUM(LENGTH, 3);
    Q_DEN(MASS, 1);
    Q_END();

    Q_START("current density", NULL);                   /* ampere per square meter      */
    Q_NUM(CURRENT, 1);
    Q_DEN(LENGTH, 2);
    Q_END();

    Q_START("magnetic field strength", NULL);           /* ampere per meter             */
    Q_NUM(CURRENT, 1);
    Q_DEN(LENGTH, 1);
    Q_END();

    Q_START("amount concentration", "concentration");    /* mole per cubic meter         */
    Q_NUM(AMOUNT, 1);
    Q_DEN(LENGTH, 3);
    Q_END();

    Q_START("luminance", NULL);                         /* candela per square meter     */
    Q_NUM(LIGHT, 1);
    Q_DEN(LENGTH, 2);
    Q_END();

    Q_START("mass fraction", "mf");                     /* kilogram per kilogram        */
    Q_NUM(MASS, 1);
    Q_DEN(MASS, 1);
    Q_END();

    Q_START("volume fraction", "vf");                   /* cubic meter per cubic meter  */
    Q_NUM(LENGTH, 3);
    Q_DEN(LENGTH, 3);
    Q_END();

    Q_START("plane angle", NULL);                       /* radian                       */
    Q_NUM(LENGTH, 1);
    Q_DEN(LENGTH, 1);
    Q_END();

    Q_START("solid angle", NULL);                       /* steradian                    */
    Q_NUM(LENGTH, 2);
    Q_DEN(LENGTH, 2);
    Q_END();
    
    Q_START("frequency", "freq");                       /* hertz                        */
    Q_DEN(TIME, 1);
    Q_END();

    Q_START("force", NULL);                             /* newton                       */
    Q_NUM(LENGTH, 1);
    Q_NUM(MASS, 1);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("weight", NULL);                            /* force per volume             */
    Q_NUM(MASS, 1);
    Q_DEN(LENGTH, 2);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("pressure", NULL); /* or stress */          /* pascal                       */
    Q_NUM(MASS, 1);
    Q_DEN(LENGTH, 1);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("volumetric flow", NULL);                   /* liter per second             */
    Q_NUM(LENGTH, 3);
    Q_DEN(TIME, 1);
    Q_END();

    Q_START("vapor permeability", NULL);                /* seconds per meter            */
    Q_NUM(TIME, 1);
    Q_DEN(LENGTH, 1);
    Q_END();

    Q_START("electric charge", "charge");               /* coulomb                      */
    Q_NUM(CURRENT, 1);
    Q_NUM(TIME, 1);
    Q_END();

    Q_START("energy", NULL);                            /* joule                        */
    Q_NUM(LENGTH, 2);
    Q_NUM(MASS, 1);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("power", NULL); /* or radiant flux */       /* watt                         */
    Q_NUM(LENGTH, 2);
    Q_NUM(MASS, 1);
    Q_DEN(TIME, 3);
    Q_END();

    Q_START("electric potential", "potential");         /* volt                         */
    Q_NUM(LENGTH, 2);
    Q_NUM(MASS, 1);
    Q_DEN(CURRENT, 1);
    Q_DEN(TIME, 3);
    Q_END();

    Q_START("capacitance", NULL);                       /* farad                        */
    Q_NUM(CURRENT, 2);
    Q_NUM(TIME, 4);
    Q_DEN(LENGTH, 2);
    Q_DEN(MASS, 1);
    Q_END();

    Q_START("electric resistance", "resistance");       /* ohm                          */
    Q_NUM(LENGTH, 2);
    Q_NUM(MASS, 1);
    Q_DEN(CURRENT, 2);
    Q_DEN(TIME, 3);
    Q_END();

    Q_START("electric conductance", "conductance");     /* siemens                      */
    Q_NUM(CURRENT, 2);
    Q_NUM(TIME, 3);
    Q_DEN(LENGTH, 2);
    Q_DEN(MASS, 1);
    Q_END();
    
    Q_START("magnetic flux", NULL);                     /* weber                        */
    Q_NUM(LENGTH, 2);
    Q_NUM(MASS, 1);
    Q_DEN(CURRENT, 1);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("magnetic flux density", NULL);             /* tesla                        */
    Q_NUM(MASS, 1);
    Q_DEN(CURRENT, 1);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("inductance", NULL);                        /* henry                        */
    Q_NUM(LENGTH, 2);
    Q_NUM(MASS, 1);
    Q_DEN(CURRENT, 2);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("luminous flux", NULL);                     /* lumen                        */
    Q_NUM(LIGHT, 1);
    Q_END();

    Q_START("illuminance", NULL);                       /* lux                          */
    Q_NUM(LIGHT, 1);
    Q_DEN(LENGTH, 2);
    Q_END();

    Q_START("activity of a radionuclide", NULL);        /* becquerel                    */
    Q_DEN(TIME, 1);
    Q_END();

    Q_START("absorbed dose", "dose");                   /* gray                         */
    Q_NUM(LENGTH, 2);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("dose equivalent", "doeseq");               /* sievert                      */
    Q_NUM(LENGTH, 2);
    Q_DEN(TIME, 2);
    Q_END();
    
    Q_START("dynamic viscosity", NULL);                 /* pascal second                */
    Q_NUM(MASS, 1);
    Q_DEN(LENGTH, 1);
    Q_DEN(TIME, 1);
    Q_END();

    Q_START("moment of force", NULL);                   /* newton meter                 */
    Q_NUM(LENGTH, 2);
    Q_NUM(MASS, 1);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("surface tension", NULL);                   /* newton per meter             */
    Q_NUM(MASS, 1);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("angular velocity", NULL);                  /* radian per second            */
    Q_DEN(TIME, 1);
    Q_END();

    Q_START("angular acceleration", NULL);              /* radian per second squared    */
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("irradiance", NULL);                        /* watt per square meter        */
    Q_NUM(MASS, 1);
    Q_DEN(TIME, 3);
    Q_END();

    Q_START("entropy", NULL);                           /* joule per kelvin             */
    Q_NUM(LENGTH, 2);
    Q_NUM(MASS, 1);
    Q_DEN(TIME, 2);
    Q_DEN(TEMP, 1);
    Q_END();

    Q_START("specific entropy", NULL);                  /* joule per kilogram kelvin    */
    Q_NUM(LENGTH, 2);
    Q_DEN(TIME, 2);
    Q_DEN(TEMP, 1);
    Q_END();

    Q_START("specific energy", NULL);                   /* joule per kilogram           */
    Q_NUM(LENGTH, 2);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("thermal conductivity", NULL);              /* watt per meter kelvin        */
    Q_NUM(LENGTH, 1);
    Q_NUM(MASS, 1);
    Q_DEN(TIME, 3);
    Q_DEN(TEMP, 1);
    Q_END();

    Q_START("energy density", NULL);                    /* joule per cubic meter        */
    Q_NUM(MASS, 1);
    Q_DEN(LENGTH, 1);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("electric field strength", NULL);           /* volt per meter       */
    Q_NUM(LENGTH, 1);
    Q_NUM(MASS, 1);
    Q_DEN(CURRENT, 1);
    Q_DEN(TIME, 3);
    Q_END();

    Q_START("electric charge density", NULL);           /* coulomb per cubic meter      */
    Q_NUM(CURRENT, 1);
    Q_NUM(TIME, 1);
    Q_DEN(LENGTH, 3);
    Q_END();

    Q_START("electric flux density", NULL);             /* coulomb per square meter     */
    Q_NUM(CURRENT, 1);
    Q_NUM(TIME, 1);
    Q_DEN(LENGTH, 2);
    Q_END();
    
    Q_START("permittivity", NULL);                      /* farad per meter              */
    Q_NUM(CURRENT, 2);
    Q_NUM(TIME, 4);
    Q_DEN(LENGTH, 3);
    Q_DEN(MASS, 1);
    Q_END();

    Q_START("permeability", NULL);                      /* henry per meter              */
    Q_NUM(LENGTH, 1);
    Q_NUM(MASS, 1);
    Q_DEN(CURRENT, 2);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("molar energy", NULL);                      /* joule per mole               */
    Q_NUM(LENGTH, 2);
    Q_NUM(MASS, 1);
    Q_DEN(AMOUNT, 1);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("molar entropy", NULL);                     /* joule per mole kelvin        */
    Q_NUM(LENGTH, 2);
    Q_NUM(MASS, 1);
    Q_DEN(AMOUNT, 1);
    Q_DEN(TIME, 2);
    Q_DEN(TEMP, 1);
    Q_END();

    Q_START("exposure (X and gamma ray)", "exposure");  /* coulomb per kilogram         */
    Q_NUM(CURRENT, 1);
    Q_NUM(TIME, 1);
    Q_DEN(MASS, 1);
    Q_END();

    Q_START("radioactive concentration", NULL);         /* eman                         */
    Q_DEN(LENGTH, 3);
    Q_DEN(TIME, 1);
    Q_END();

    Q_START("absorbed dose rate", NULL);                /* gray per second              */
    Q_NUM(LENGTH, 2);
    Q_DEN(TIME, 3);
    Q_END();

    Q_START("radiant intensity", NULL);                 /* watt per steradian           */
    Q_NUM(LENGTH, 2);
    Q_NUM(MASS, 1);
    Q_DEN(TIME, 3);
    Q_END();

    Q_START("radiance", NULL);                          /* watt per square meter steradian */
    Q_NUM(MASS, 1);
    Q_DEN(TIME, 3);
    Q_END();
    
    
    /*----------------------------------------------------------------------------------------------------
     * The following are from the GNU units database.
     *----------------------------------------------------------------------------------------------------
     */

    Q_START("information", NULL);                       /* bit                          */
    Q_NUM(AMOUNT, 1);
    Q_END();

    Q_START("information rate", NULL);                  /* bit per second               */
    Q_NUM(AMOUNT, 1);
    Q_DEN(TIME, 1);
    Q_END();

    Q_START("stress-optical coefficient", NULL);        /* brewster                     */
    Q_NUM(LENGTH, 1);
    Q_NUM(MASS, 1);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("reciprocal lens focal length", NULL);      /* diopter                      */
    Q_DEN(LENGTH, 1);
    Q_END();

    Q_START("resistivity", NULL);                       /* preece                       */
    Q_NUM(LENGTH, 3);
    Q_NUM(MASS, 1);
    Q_DEN(CURRENT, 2);
    Q_DEN(TIME, 3);
    Q_END();

    Q_START("magnetic reluctance", NULL);               /* sturgeon                     */
    Q_NUM(CURRENT, 2);
    Q_NUM(TIME, 2);
    Q_DEN(LENGTH, 2);
    Q_DEN(MASS, 1);
    Q_END();

    Q_START("elastance", NULL);                         /* daraf                        */
    Q_NUM(LENGTH, 2);
    Q_NUM(MASS, 1);
    Q_DEN(CURRENT, 2);
    Q_DEN(TIME, 4);
    Q_END();
    
    Q_START("viscosity", NULL);                         /* poiseuille                   */
    Q_NUM(MASS, 1);
    Q_DEN(TIME, 1);
    Q_END();

    Q_START("specific heat", NULL);                     /* mayer                        */
    Q_NUM(LENGTH, 2);
    Q_DEN(TIME, 2);
    Q_DEN(TEMP, 1);
    Q_END();

    Q_START("reciprocal color temperature", NULL);      /* mired                        */
    Q_DEN(TEMP, 1);
    Q_END();
    
    Q_START("concentration", NULL);                     /* percent                      */
    Q_NUM(AMOUNT, 1);
    Q_DEN(AMOUNT, 1);
    Q_END();
    
    Q_START("mechanical mobility", NULL);               /* mohm                         */
    Q_NUM(TIME, 1);
    Q_DEN(MASS, 1);
    Q_END();

    Q_START("mechanical resistance", NULL);             /* mechanical ohm               */
    Q_NUM(MASS, 1);
    Q_DEN(TIME, 1);
    Q_END();
    
    Q_START("reciprocal viscosity", NULL);              /* rhe                          */
    Q_NUM(LENGTH, 1);
    Q_NUM(TIME, 1);
    Q_DEN(MASS, 1);
    Q_END();

    Q_START("kinematic viscosity", NULL);               /* stokes                       */
    Q_NUM(LENGTH, 2);
    Q_DEN(TIME, 1);
    Q_END();
    
    Q_START("momentum", NULL);                          /* bole                         */
    Q_NUM(LENGTH, 1);
    Q_NUM(MASS, 1);
    Q_DEN(TIME, 1);
    Q_END();

    Q_START("luminous energy", NULL);                   /* talbot                       */
    Q_NUM(LIGHT, 1);
    Q_NUM(TIME, 1);
    Q_END();

    Q_START("luminous energy per area", NULL);          /* metercandle second           */
    Q_NUM(LIGHT, 1);
    Q_NUM(TIME, 1);
    Q_DEN(LENGTH, 2);
    Q_END();
    
    Q_START("entropy flow", NULL);                      /* watt per kelvin              */
    Q_NUM(LENGTH, 2);
    Q_NUM(MASS, 1);
    Q_DEN(TIME, 3);
    Q_DEN(TEMP, 1);
    Q_END();

    Q_START("thermal resistance", NULL);                /* fourier                      */
    Q_NUM(TEMP, 2);
    Q_NUM(TIME, 3);
    Q_DEN(LENGTH, 2);
    Q_DEN(MASS, 1);
    Q_END();
    
    Q_START("thermal inductance", NULL);                /* joule kelvin squared per watt squared */
    Q_NUM(TEMP, 2);
    Q_NUM(TIME, 1);
    Q_END();

    Q_START("electric dipole moment", NULL);            /* coulomb meter                */
    Q_NUM(CURRENT, 1);
    Q_NUM(TIME, 1);
    Q_NUM(LENGTH, 1);
    Q_END();

    Q_START("electric dipole moment per area", NULL);   /* helmholtz                    */
    Q_NUM(CURRENT, 1);
    Q_NUM(TIME, 1);
    Q_DEN(LENGTH, 1);
    Q_END();
    
    PASSED;

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
	    retval = SAF_FAILURE;
        }
    } SAF_TRY_END;

    return retval;
}

/**/
static int
define_special_units(SAF_Db *database)
{
    SAF_Quantity q;
    SAF_Unit u;
    
    saf_find_one_quantity(database, "not applicable", &q);
    saf_declare_unit(SAF_ALL, database, "not applicable", "NA", URL, &u);
    saf_quantify_unit(SAF_ALL, &u, &q, 1.0);

    return SAF_SUCCESS;
    
}

static int
define_SI_basic_units(SAF_Db *database)
{
    U_DCLS("define_SI_basic_units");

    /*--------------------------------------------------------------------------------
     * SI Basic Units (see http://physics.nist.gov/cuu/Units/units.html)
     * One for each basic quantity.
     *-------------------------------------------------------------------------------- */
    U_START(second, "s");
    U_END(time);
    U_SI_PREFIX(second);
    
    U_START(gram, "g");
    saf_multiply_unit(SAF_ALL, unit, 0.001, NULL, 1); /*kg is actual basic unit*/
    U_END(mass);
    U_SI_PREFIX(gram);
    
    U_START(ampere, "A");
    U_END(electric current);
    U_SI_PREFIX(ampere);

    U_START(meter, "m");
    U_END(length);
    U_SI_PREFIX(meter);

    U_START(candela, "cd");
    U_END(luminous intensity);
    U_SI_PREFIX(candela);

    U_START(kelvin, "K");
    U_END(thermodynamic temperature);
    U_SI_PREFIX(kelvin);

    U_START(mole, "mol");
    U_END(amount of a substance);
    U_SI_PREFIX(mole);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_SI_derived_units(SAF_Db *database)
{
    U_DCLS("define_SI_derived_units");

    /*--------------------------------------------------------------------------------
     * SI Derived Units (see http://physics.nist.gov/cuu/Units/units.html)
     * There should be one for each predefined quantity.
     *-------------------------------------------------------------------------------- */
    
    /* Basic unit of area has no name */
    U_START(square meter, NULL);
    U_END(area);

    /* Basic unit of volume has no name */
    U_START(cubic meter, NULL);
    U_END(volume);

    /* Basic unit of velocity has no name */
    U_START(meter per second, NULL);
    U_END(velocity);

    /* Basic unit of acceleration has no name */
    U_START(meter per second squared, NULL);
    U_END(acceleration);

    /* Basic unit for wave number has no name */
    U_START(per meter, NULL);
    U_END(wave number);

    /* Basic unit for mass density has no name */
    U_START(kilogram per cubic meter, NULL);
    U_END(mass density);

    /* Basic unit for specific length has no name */
    U_START(meter per gram, NULL);
    U_END(specific length);

    /* Basic unit for specific volume has no name */
    U_START(cubic meter per gram, NULL);
    U_END(specific volume);

    /* Basic unit for current density has no name */
    U_START(ampere per square meter, NULL);
    U_END(current density);

    /* Basic unit for magnetic field strength has no name */
    U_START(ampere per meter, NULL);
    U_END(magnetic field strength);

    /* Basic unit for amount concentration has no name */
    U_START(mole per cubic meter, NULL);
    U_END(amount concentration);

    /* candela per square meter */
    U_START(nit, NULL);
    U_END(luminance);

    /* Basic unit for mass fraction has no name */
    U_START(gram per gram, NULL);
    U_END(mass fraction);

    /* Basic unit for volume fraction has no name */
    U_START(cubic meter per cubic meter, NULL);
    U_END(volume fraction);
    
    U_START(radian, "rad");
    U_END(plane angle);
    U_SI_PREFIX(radian);

    U_START(steradian, "sr");
    U_END(solid angle);
    U_SI_PREFIX(steradian);

    U_START(hertz, "Hz");
    U_END(frequency);
    U_SI_PREFIX(hertz);

    U_START(newton, "N");
    U_END(force);
    U_SI_PREFIX(newton);

    U_START(water weight, "wc");
    U_END(weight);

    U_START(pascal, "Pa");
    U_END(pressure);
    U_SI_PREFIX(pascal);

    /* Basic unit of volumentric flow has no name */
    U_START(cubic meter per second, NULL);
    U_END(volumetric flow);

    /* Basic unit of vapor permeability has no name */
    U_START(second per meter, NULL);
    U_END(vapor permeability);

    U_START(coulomb, "C");
    U_END(electric charge);
    U_SI_PREFIX(coulomb);

    U_START(joule, "J");
    U_END(energy);
    U_SI_PREFIX(joule);

    U_START(watt, "W");
    U_END(power);
    U_SI_PREFIX(watt);

    U_START(volt, "V");
    U_END(electric potential);
    U_SI_PREFIX(volt);

    U_START(farad, "F");
    U_END(capacitance);
    U_SI_PREFIX(farad);

    U_START(ohm, NULL); /* Omega */
    U_END(electric resistance);
    U_SI_PREFIX(ohm);

    U_START(siemens, "S");
    U_END(electric conductance);
    U_SI_PREFIX(siemens);

    U_START(weber, "Wb");
    U_END(magnetic flux);
    U_SI_PREFIX(weber);

    U_START(tesla, "T");
    U_END(magnetic flux density);
    U_SI_PREFIX(tesla);

    U_START(henry, "H");
    U_END(inductance);
    U_SI_PREFIX(henry);

    U_START(lumen, "lm");
    U_END(luminous flux);
    U_SI_PREFIX(lumen);

    U_START(lux, "lx");
    U_END(illuminance);
    U_SI_PREFIX(lux);

    U_START(becquerel, "Bq");
    U_END(activity of a radionuclide);
    U_SI_PREFIX(becquerel);

    U_START(gray, "Gy");
    U_END(absorbed dose);
    U_SI_PREFIX(gray);

    U_START(sievert, "Sv");
    U_END(dose equivalent);
    U_SI_PREFIX(sievert);

    /* Basic unit for dynamic viscosity has no name */
    U_START(pascal second, NULL);
    U_END(dynamic viscosity);

    /* Basic unit for moment of force has no name */
    U_START(newton meter, NULL);
    U_END(moment of force);

    /* Basic unit of surface tension has no name */
    U_START(newton per meter, NULL);
    U_END(surface tension);

    /* radian per second */
    U_START(pulsatance, NULL);
    U_END(angular velocity);

    /* Basic unit of angular acceleration has no name */
    U_START(radian per second squared, NULL);
    U_END(angular acceleration);

    /* Basic unit of irradiance has no name */
    U_START(watt per square meter, NULL);
    U_END(irradiance);

    /* Basic unit of entropy has no name */
    U_START(joule per kelvin, NULL);
    U_END(entropy);

    /* Basic unit of specific entroy has no name */
    U_START(joule per gram kelvin, NULL);
    U_END(specific entropy);

    /* Basic unit of specific energy has no name */
    U_START(joule per gram, NULL);
    U_END(specific energy);

    /* Basic unit of thermal conductivity has no name */
    U_START(watt per meter kelvin, NULL);
    U_END(thermal conductivity);

    /* Basic unit of energy density has no name */
    U_START(joule per cubic meter, NULL);
    U_END(energy density);

    /* Basic unit of electric field strength has no name */
    U_START(volt per meter, NULL);
    U_END(electric field strength);

    /* Basic unit of electric charge density has no name */
    U_START(coulomb per cubic meter, NULL);
    U_END(electric charge density);

    /* Basic unit of electric flux density has no name */
    U_START(coulomb per square meter, NULL);
    U_END(electric flux density);

    /* Basic unit of permittivity has no name */
    U_START(farad per meter, NULL);
    U_END(permittivity);

    /* Basic unit of permeability has no name */
    U_START(henry per meter, NULL);
    U_END(permeability);

    /* Basic unit of molar energy has no name */
    U_START(joule per mole, NULL);
    U_END(molar energy);

    /* Basic unit of molar entropy has no name */
    U_START(joule per mole kelvin, NULL);
    U_END(molar entropy);

    /* Basic unit of exposure has no name */
    U_START(coulomb per gram, NULL);
    U_END(exposure);

    /* Basic unit of radioactive concentration has no name */
    U_START(per cubic meter second, NULL);
    U_END(radioactive concentration);

    /* Basic unit of absorbed dose rate has no name */
    U_START(gray per second, NULL);
    U_END(absorbed dose rate);

    /* Basic unit of radiant intensity has no name */
    U_START(watt per steradian, NULL);
    U_END(radiant intensity);

    /* Basic unit of radiance has no name */
    U_START(watt per square meter steradian, NULL);
    U_END(radiance);

    /* Basic unit of information (entropy). The entropy in bits of a random variable over a finite alphabet is defined to be
     * the sum of -p(i)*log2(p(i)) over the alphabet where p(i) is the probability that the random variable takes on the value
     * i */
    U_START(bit, "b");
    U_END(information);
    U_SI_PREFIX(bit);  /*none of the fractional multipliers make sense. oh well*/
    U_EIC_PREFIX(bit);

    /* Basic unit of information rate has no name */
    U_START(bit per second, "bps");
    U_END(information rate);
    U_SI_PREFIX(bit per second);
    U_EIC_PREFIX(bit per second);

    /* Basic unit of stress-optical coefficient has no name */
    U_START(square meter per newton, NULL);
    U_END(stress-optical coefficient);
    
    /* reciprocal of meter */
    U_START(diopter, NULL);
    U_END(reciprocal lens focal length);

    /* basic unit of resistivity has no name */
    U_START(ohm meter, NULL);
    U_END(resistivity);

    /* reciprocal of henry */
    U_START(sturgeon, NULL);
    U_END(magnetic reluctance);
    U_SI_PREFIX(sturgeon);
    
    /* reciprocal of farad */
    U_START(daraf, NULL);
    U_END(elastance);
    U_SI_PREFIX(daraf);

    /* newton second per meter */
    U_START(poiseuille, NULL);
    U_END(viscosity);
    U_SI_PREFIX(poiseuille);

    /* joule per gram kelvin */
    U_START(mayer, NULL);
    U_END(specific heat);
    U_SI_PREFIX(mayer);

    /* basic unit of reciprocal of color temperature has no name */
    U_START(reciprocal degree, NULL);
    U_END(reciprocal color temperature);

    /* basic unit of concentration has no name */
    U_START(ratio, NULL);
    U_END(concentration);

    /* basic unit of mechanical mobility has no name */
    U_START(second per gram, NULL);
    U_END(mechanical mobility);
    
    /* basic unit of mechanical resistance has no name */
    U_START(gram per second, NULL);
    U_END(mechanical resistance);

    /* basic unit of reciprocal viscosity has no name */
    U_START(meter second per gram, NULL);
    U_END(reciprocal viscosity);

    /* basic unit of kinematic viscosity has no name */
    U_START(square meters per second, NULL);
    U_END(kinematic viscosity);

    /* basic unit of momentum has no name */
    U_START(gram meter per second, NULL);
    U_END(momentum);

    /* lumen second */
    U_START(talbot, NULL);
    U_END(luminous energy);
    U_SI_PREFIX(talbot);        /*appropriate?*/

    /* lumen second per square meter */
    U_START(metercandle second, "mcs");
    U_END(luminous energy per area);
    
    /* basic unit of entropy flow has no name */
    U_START(watt per kelvin, NULL);
    U_END(entropy flow);

    /* K^2/W */
    U_START(fourier, NULL);
    U_END(thermal resistance);
    U_SI_PREFIX(fourier);       /*appropriate?*/

    /* basic unit for thermal inductance has no name */
    U_START(joule kelvin square per watt squared, NULL);
    U_END(thermal inductance);

    /* basic unit for electric dipole moment has no name */
    U_START(coulomb meter, NULL);
    U_END(electric dipole moment);

    /* basic unit for electric dipole moment per area has no name */
    U_START(coulomb per meter, NULL);
    U_END(electric dipole moment per area);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_time_derived_units(SAF_Db *database)
{
    U_DCLS("define_time_derived_units");

    /*--------------------------------------------------------------------------------
     * Time derived units
     *-------------------------------------------------------------------------------- */

    /* Alternate abbreviation for `s' */
    U_START(second, "sec");
    U_END(time);

    U_START(minute, "min");
    U_MULT(60, second, 1);
    U_END(time);

    U_START(hour, "hr");
    U_MULT(60, minute, 1);
    U_END(time);

    U_START(day, "d");
    U_MULT(24, hour, 1);
    U_END(time);

    /* Alternate abbreviation for `day' */
    U_START(day, "da");
    U_MULT(1, day, 1);
    U_END(time);

    U_START(week, "wk");
    U_MULT(7, day, 1);
    U_END(time);

    U_START(shake, NULL);
    U_MULT(1e-8, second, 1);
    U_END(time);

    /* Used for measuring the sedimentation coefficient for centrifuging */
    U_START(svedberg, NULL);
    U_MULT(1e-13, second, 1);
    U_END(time);

    U_START(sennight, NULL);
    U_MULT(7, day, 1);
    U_END(time);

    U_START(fortnight, NULL);
    U_MULT(7, day, 1);
    U_END(time);

    U_START(blink, "li");
    U_MULT(1e-5, day, 1);
    U_END(time);

    U_START(ce, NULL);
    U_MULT(1e-2, day, 1);
    U_END(time);

    /* The mean interval between vernal equinoxes. Differs from the solar year by 1 part in 26000 due to precession of the
     * earth about its rotational exis combined with precession of the perihelion of the earth's orbit. */
    U_START(tropicalyear, NULL);
    U_MULT(365.242198781, day, 1);
    U_END(time);

    U_START(year, "yr");
    U_MULT(1, tropicalyear, 1);
    U_END(time);

    U_START(solaryear, NULL);
    U_MULT(1, year, 1);
    U_END(time);

    U_START(month, "mo");
    U_MULT(1/12.0, year, 1);
    U_END(time);

    U_START(decade, NULL);
    U_MULT(10, year, 1);
    U_END(time);

    U_START(century, NULL);
    U_MULT(100, year, 1);
    U_END(time);

    U_START(millenium, NULL);
    U_MULT(1000, year, 1);
    U_END(time);

    U_START(gregorianyear, NULL);
    U_MULT(365.2425, day, 1);
    U_END(time);

    U_START(julianyear, NULL);
    U_MULT(365.25, day, 1);
    U_END(time);

    U_START(leapyear, NULL);
    U_MULT(366, day, 1);
    U_END(time);

    U_START(commonyear, NULL);
    U_MULT(365, day, 1);
    U_END(time);

    U_START(calendaryear, NULL);
    U_MULT(365, day, 1);
    U_END(time);

    /* The time between successive perihelion passages of the earth. */
    U_START(anomalisticyear, NULL);
    U_MULT(265.2596, day, 1); /*should this be 365.2596? (265.2596 comes from GNU units.dat 1.9) */
    U_END(time);

    /* The time for the earth to make one revolution around the sun relative to the stars. */
    U_START(siderealyear, NULL);
    U_MULT(365.256360417, day, 1);
    U_END(time);

    /* The sidereal day is the time required for the earth to make one rotation relative to the stars. The more usual solar
     * day is the time required to make a rotation relative to the sun. Because the earth moves in its orbit, it has to turn a
     * bit extra to face the sun again, hence the solar day is slightly longer. */
    U_START(siderealday, NULL);
    U_MULT(23.934459444, hour, 1);
    U_END(time);

    U_START(siderealhour, NULL);
    U_MULT(1/24.0, siderealday, 1);
    U_END(time);

    U_START(siderealminute, NULL);
    U_MULT(1/60.0, siderealhour, 1);
    U_END(time);

    U_START(siderealsecond, NULL);
    U_MULT(1/60.0, siderealminute, 1);
    U_END(time);

    /* Time required for the moon to orbit the earth. */
    U_START(siderealmonth, NULL);
    U_MULT(27.321661, day, 1);
    U_END(time);
    
    /* A lunar month is the time between full moons. Full moons occur when the sun and moon are on opposite sides of the
     * earth. Since the earth moves around the sun, the moon has to rotate a bit farther to get into the full moon
     * configuration. */
    U_START(lunarmonth, NULL);
    U_MULT(29.5305555, day, 1);
    U_END(time);

    /* Another name for lunarmonth */
    U_START(synodicmonth, NULL);
    U_MULT(1, lunarmonth, 1);
    U_END(time);

    U_START(lunaryear, NULL);
    U_MULT(12, lunarmonth, 1);
    U_END(time);

    /* Planetary sidereal days */
    U_START(mercuryday, NULL);
    U_MULT(58.6462, day, 1);
    U_END(time);

    U_START(venusday, NULL);
    U_MULT(243.01, day, 1);
    U_END(time);

    U_START(earthday, NULL);
    U_MULT(1, siderealday, 1);
    U_END(time);

    U_START(marsday, NULL);
    U_MULT(1.02595675, day, 1);
    U_END(time);

    U_START(jupiterday, NULL);
    U_MULT(0.41354, day, 1);
    U_END(time);

    U_START(saturnday, NULL);
    U_MULT(0.4375, day, 1);
    U_END(time);

    U_START(uranusday, NULL);
    U_MULT(0.65, day, 1);
    U_END(time);

    U_START(plutoday, NULL);
    U_MULT(6.3867, day, 1);
    U_END(time);

    /* Planetary sidereal years */
    U_START(mercuryyear, NULL);
    U_MULT(88, day, 1);
    U_END(time);

    U_START(venusyear, NULL);
    U_MULT(224.7, day, 1);
    U_END(time);

    U_START(earthyear, NULL);
    U_MULT(1, siderealyear, 1);
    U_END(time);

    U_START(marsyear, NULL);
    U_MULT(687, day, 1);
    U_END(time);

    U_START(jupiteryear, NULL);
    U_MULT(11.86, tropicalyear, 1); /* correctness is unknown */
    U_END(time);

    U_START(saturnyear, NULL);
    U_MULT(29.46, tropicalyear, 1); /* correctness is unknown */
    U_END(time);

    U_START(uranusyear, NULL);
    U_MULT(84.01, tropicalyear, 1); /* correctness is unknown */
    U_END(time);

    U_START(neptuneyear, NULL);
    U_MULT(164.8, tropicalyear, 1); /* correctness is unknown */
    U_END(time);

    U_START(plutoyear, NULL);
    U_MULT(247.7, tropicalyear, 1); /* correctness is unknown */
    U_END(time);

    U_START(cron, NULL);
    U_MULT(1e6, year, 1);
    U_END(time);

    U_START(planck time, "t_P");
    U_MULT(5.39056e-44, second, 1);
    U_END(time);
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_mass_derived_units(SAF_Db *database)
{
    U_DCLS("define_mass_derived_units");

    /*--------------------------------------------------------------------------------
     * Mass derived units
     *-------------------------------------------------------------------------------- */

    /* alternate spelling */
    U_START(gramme, "g");
    U_MULT(1, gram, 1);
    U_END(mass);
    U_SI_PREFIX(gramme);
    
    /* alternate abbreviation */
    U_START(gram, "gm");
    U_MULT(1, gram, 1);
    U_END(mass);

    U_START(tonne, "t");
    U_MULT(1000, kilogram, 1);
    U_END(mass);

    U_START(metricton, NULL);
    U_MULT(1, tonne, 1);
    U_END(mass);

    U_START(gamma, NULL);
    U_MULT(1, microgram, 1);
    U_END(mass);

    U_START(quntal, NULL);
    U_MULT(100, kilogram, 1);
    U_END(mass);

    /* Defined to be 1/12 the mass of carbon 12 */
    U_START(atomic mass unit, "u");
    U_MULT(1.6605402e-27, kilogram, 1);
    U_END(mass);

    /* Another abbreviation for atomic mass unit */
    U_START(atomic mass unit, "amu");
    U_MULT(1, atomic mass unit, 1);
    U_END(mass);

    /* 1/16 of the weights average mass of the 3 natuarlly occuring neutral isotopes of oxygen */
    U_START(amu_chem, NULL);
    U_MULT(1.66026e-27, kilogram, 1);
    U_END(mass);

    /* 1/16 of the mass of a neutral oxygen 16 atom */
    U_START(amu_phys, NULL);
    U_MULT(1.65981e-27, kilogram, 1);
    U_END(mass);

    U_START(dalton, NULL);
    U_MULT(1, amu_chem, 1);
    U_END(mass);

    U_START(planck mass, "m_P");
    U_MULT(2.17671e-8, kilogram, 1);
    U_END(mass);
    
    /* Masses of elementray particles */
    U_START(electronmass, "m_e");
    U_MULT(9.1093897e-31, kilogram, 1);
    U_END(mass);

    U_START(protonmass, "m_p");
    U_MULT(1.6726231e-27, kilogram, 1);
    U_END(mass);

    U_START(neutronmass, "m_n");
    U_MULT(1.6749286e-27, kilogram, 1);
    U_END(mass);

    U_START(muonmass, "m_mu");
    U_MULT(1.8835327e-28, kilogram, 1);
    U_END(mass);

    U_START(deuteronmass, "m_d");
    U_MULT(3.3435860e-27, kilogram, 1);
    U_END(mass);

    /* Astronomical masses */
    U_START(sunmass, NULL);
    U_MULT(1.9891e30, kilogram, 1);
    U_END(mass);

    U_START(earthmass, NULL);
    U_MULT(5.9742e24, kilogram, 1);
    U_END(mass);

    U_START(moonmass, NULL);
    U_MULT(7.3483e22, kilogram, 1);
    U_END(mass);

    U_START(mercurymass, NULL);
    U_MULT(0.33022e24, kilogram, 1);
    U_END(mass);

    U_START(venusmass, NULL);
    U_MULT(4.8690e24, kilogram, 1);
    U_END(mass);

    U_START(marsmass, NULL);
    U_MULT(0.64191e24, kilogram, 1);
    U_END(mass);

    U_START(jupitermass, NULL);
    U_MULT(1898.8e24, kilogram, 1);
    U_END(mass);

    U_START(saturnmass, NULL);
    U_MULT(568.5e24, kilogram, 1);
    U_END(mass);

    U_START(neptunemass, NULL);
    U_MULT(102.78e24, kilogram, 1);
    U_END(mass);

    U_START(plutomass, NULL);
    U_MULT(0.015e24, kilogram, 1);
    U_END(mass);

    /* The Hartree system of atomic units, derived from fundamental units of mass (of electron), action (plank's constant),
     * charge, and the coulomb constant. */
    U_START(atomicmass, NULL);
    U_MULT(1, electronmass, 1);
    U_END(mass);

    /* English units of force are in terms of mass... */
    U_START(pound, "lb");
    U_MULT(0.45359237, kilogram, 1);
    U_END(mass);

    /* The grain is the same in all three weight systems. It was originally defined as the weight of a barleycorn taken from
     * the middle of the ear. */
    U_START(grain, NULL);
    U_MULT(1/7000.0, pound, 1);
    U_END(mass);

    U_START(ounce, "oz");
    U_MULT(1/16.0, pound, 1);
    U_END(mass);

    U_START(dram, "dr");
    U_MULT(1/16.0, ounce, 1);
    U_END(mass);

    U_START(hundredweight, "cwt");
    U_MULT(100, pound, 1);
    U_END(mass);

    U_START(short hundredweight, "cwt");
    U_MULT(1, hundredweight, 1);
    U_END(mass);

    U_START(ton, NULL);
    U_MULT(2000, pound, 1);
    U_END(mass);

    U_START(short ton, NULL);
    U_MULT(1, ton, 1);
    U_END(mass);

    U_START(quarter, NULL);
    U_MULT(1/4.0, ton, 1);
    U_END(mass);

    U_START(short quarter, NULL);
    U_MULT(1/4.0, short ton, 1);
    U_END(mass);
    
    U_START(metricounce, "mounce");
    U_MULT(25, gram, 1);
    U_END(mass);

    /* Troy wieight: In 1828 the troy pound was made the first United States standard weight. It was to be used to regulate
     * coinage. */
    U_START(troy pound, "lb");
    U_MULT(5760, grain, 1);
    U_END(mass);

    U_START(troy ounce, "ozt");
    U_MULT(1/12.0, troy pound, 1);
    U_END(mass);

    U_START(pennyweight, "dwt");
    U_MULT(1/20.0, troy ounce, 1);
    U_END(mass);

    U_START(metric carat, NULL);
    U_MULT(0.2, gram, 1);
    U_END(mass);

    U_START(metric grain, NULL);
    U_MULT(50, milligram, 1);
    U_END(mass);

    U_START(carat, "ct");
    U_MULT(1, metric carat, 1);
    U_END(mass);

    /* Alternate abbrev */
    U_START(carat, "c");
    U_MULT(1, carat, 1);
    U_END(mass);

    U_START(jewlers point, NULL);
    U_MULT(1/100.0, carat, 1);
    U_END(mass);

    U_START(assay ton, NULL);
    U_MULT(1, milligram, 1);
    U_MULT(1, ton, 1);
    U_MULT(1, troy ounce, -1);
    U_END(mass);

    /* Apothecaries' weight */
    U_START(apothecary pound, "lb ap");
    U_MULT(1, troy pound, 1);
    U_END(mass);

    U_START(apothecary ounce, "oz ap");
    U_MULT(1, troy ounce, 1);
    U_END(mass);

    U_START(apothecary dram, "dr ap");
    U_MULT(1/8.0, apothecary ounce, 1);
    U_END(mass);

    U_START(apothecary scruple, "s ap");
    U_MULT(1/3.0, apothecary dram, 1);
    U_END(mass);

    /* British Imperial weight is mostly the same as US weight. A few extra units are added here */
    U_START(clove, NULL);
    U_MULT(7, pound, 1);
    U_END(mass);

    /* British Imperial weight is mostly the same as US weight. A few extra units are added here */
    U_START(stone, NULL);
    U_MULT(14, pound, 1);
    U_END(mass);
    
    /* British Imperial weight is mostly the same as US weight. A few extra units are added here */
    U_START(British hundredweight, "cwt");
    U_MULT(8, stone, 1);
    U_END(mass);

    /* British Imperial weight is mostly the same as US weight. A few extra units are added here */
    U_START(long hundredweight, "cwt");
    U_MULT(20, British hundredweight, 1);
    U_END(mass);

    /* British Imperial weight is mostly the same as US weight. A few extra units are added here */
    U_START(long ton, NULL);
    U_MULT(20, British hundredweight, 1);
    U_END(mass);

    /* British Imperial weight is mostly the same as US weight. A few extra units are added here */
    U_START(British ton, NULL);
    U_MULT(1, long ton, 1);
    U_END(mass);

    /* British Imperial weight is mostly the same as US weight. A few extra units are added here */
    U_START(British quartermass, NULL);
    U_MULT(1/4.0, British hundredweight, 1);
    U_END(mass);

    /* British Imperial weight is mostly the same as US weight. A few extra units are added here */
    U_START(British assay ton, NULL);
    U_MULT(1, milligram, 1);
    U_MULT(1, British ton, 1);
    U_MULT(1, troy ounce, -1);
    U_END(mass);

    /* Obscure British unit. Originally intended to be 4 grain but this value ended up being used in the London diamond
     * market. */
    U_START(English crat, "ct");
    U_MULT(3.163, grain, 1);
    U_END(mass);

    /* American slang */
    U_START(key, "kg");
    U_MULT(1, kilogram, 1);
    U_END(mass);

    /* American slang. 60's weed unit */
    U_START(lid, NULL);
    U_MULT(1, ounce, 1);
    U_END(mass);
    
    /* Chinese cooking unit */
    U_START(catty, NULL);
    U_MULT(0.5, kilogram, 1);
    U_END(mass);
    
    /* Chinese cooking unit, before metric conversion */
    U_START(catty, NULL);
    U_MULT(4/3.0, pound, 1);
    U_END(mass);

    /* Chinese cooking unit (should this be defined for after metric conversion too?) */
    U_START(tael, NULL);
    U_MULT(1/16.0, catty, 1); /*the pre-SI catty*/
    U_END(mass);

    /* The crith is the mass of one liter of hydrogen at standard temperature and pressure */
    U_START(crith, NULL);
    U_MULT(0.089885, gram, 1);
    U_END(mass);

    U_START(cental, NULL);
    U_MULT(100, pound, 1);
    U_END(mass);

    U_START(centner, NULL);
    U_MULT(100, pound, 1);
    U_END(mass);
    
    /*--------------------------------------------------------------------------------
     * Electric current derived units
     *-------------------------------------------------------------------------------- */

    /* alternate abbreviation */
    U_START(ampere, "amp");
    U_MULT(1, ampere, 1);
    U_END(electric current);

    /* Named after Luigi Galvani */
    U_START(galvat, NULL);
    U_MULT(1, ampere, 1);
    U_END(electric current);

    /* Current which produces a force of 2 dyne/cm between two infinitely long wires that are 1 cm apart. */
    U_START(abampere, "abamp");
    U_MULT(10, ampere, 1);
    U_END(electric current);

    /* Another abbreviation for abampere */
    U_START(abampere, "aA");
    U_MULT(1, abampere, 1);
    U_END(electric current);

    U_START(gilbert, "Gi");
    U_MULT(0.25*SAF_M_PI, abampere, 1);
    U_END(electric current);
    U_SI_PREFIX(gilbert);

    /* alternate abbreviation */
    U_START(gilbert, "Gb");
    U_MULT(1, gilbert, 1);
    U_END(electric current);
    U_SI_PREFIX(gilbert);

    /* alternative name for abamp */
    U_START(biot, "Bi");
    U_MULT(1, abampere, 1);
    U_END(electric current);

    /* Historical: the current which in one second deposits 0.001118 gram of silver from an aqueous solution of silver
     *             nitrate. */
    U_START(intampere, "intamp");
    U_MULT(0.999835, ampere, 1);
    U_END(electric current);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_length_derived_units(SAF_Db *database)
{
    U_DCLS("define_length_derived_units");

    /*--------------------------------------------------------------------------------
     * Length derived units
     *-------------------------------------------------------------------------------- */

    /* Alternate spelling */
    U_START(metre, "m");
    U_MULT(1, meter, 1);
    U_END(length);
    U_SI_PREFIX(metre);
    
    U_START(micron, NULL);
    U_MULT(1, micrometer, 1);
    U_END(length);

    U_START(bicron, NULL);
    U_MULT(1, picometer, 1);
    U_END(length);

    U_START(angstrom, "A");
    U_MULT(1e-10, meter, 1);
    U_END(length);

    /* Used for measuring wavelengths of X-rays. It is defined to be 1/2029.45 of the spacing of calcite planes at 18 degrees
     * Celsius. It was intended to be exactly 1e-13 meters, but was later found to be off slightly. */
    U_START(xunit, NULL);
    U_MULT(1.00202e-13, meter, 1);
    U_END(length);

    U_START(siegbahn, NULL);
    U_MULT(1, xunit, 1);
    U_END(length);

    U_START(fermi, NULL);
    U_MULT(1e-15, meter, 1);
    U_END(length);

    /* Rarely used for astronomical measurements */
    U_START(spat, NULL);
    U_MULT(1e12, meter, 1);
    U_END(length);

    U_START(planck length, "l_P");
    U_MULT(1.61605e-35, meter, 1);
    U_END(length);

    U_START(inch, "in");
    U_MULT(2.54, centimeter, 1);
    U_END(length);

    /* According to 1844 bronze bar */
    U_START(British inch, "in");
    U_MULT(120000/120000.18672, inch, 1);
    U_END(length);

    U_START(foot, "ft");
    U_MULT(12, inch, 1);
    U_END(length);

    /* According to 1844 bronze bar */
    U_START(British foot, "ft");
    U_MULT(120000/120000.18672, foot, 1);
    U_END(length);

    U_START(yard, "yd");
    U_MULT(3, foot, 1);
    U_END(length);

    /* According to 1844 bronze rod */
    U_START(British yard, "yd");
    U_MULT(3, British foot, 1);
    U_END(length);

    U_START(mile, "mi");
    U_MULT(5280, foot, 1);
    U_END(length);

    U_START(line, NULL);
    U_MULT(1/12.0, inch, 1);
    U_END(length);

    /* alternate definition of line */
    U_START(line, NULL);
    U_MULT(0.1, inch, 1);
    U_END(length);

    /* The US Metric Law of 1866 gave the exact relation `1 meter = 39.37 inches'. From 1893 until 1959 the foot was exactly
     * 1200/3937 meters. In 1959 the definition was changed to bring the US into agreement with other countries. Since then,
     * the foot has been exactly 0.3048 meters. At the same time it was decided that any data expressed in feet derived from
     * geodetic surveys within the US would continue to use the old definition. */
    U_START(survey yard, "yd");
    U_MULT(1200/119999.76, yard, 1);
    U_END(length);

    U_START(rod, "rd");
    U_MULT(5.5, survey yard, 1);
    U_END(length);

    U_START(perch, NULL);
    U_MULT(1, rod, 1);
    U_END(length);

    U_START(furlong, "fur");
    U_MULT(40, rod, 1);
    U_END(length);

    U_START(statute mile, "mi");
    U_MULT(1200/119999.76, mile, 1);
    U_END(length);

    U_START(league, NULL);
    U_MULT(3, statute mile, 1);
    U_END(length);

    U_START(survey foot, "ft");
    U_MULT(1200/119999.76, foot, 1);
    U_END(length);

    U_START(survey chain, "chain");
    U_MULT(66, survey foot, 1);
    U_END(length);

    /* another abbreviation for chain */
    U_START(survey chain, "ch");
    U_MULT(1, survey chain, 1);
    U_END(length);

    U_START(survey pole, "pole");
    U_MULT(1/4.0, survey chain, 1);
    U_END(length);

    U_START(survey link, "link");
    U_MULT(1/100.0, survey chain, 1);
    U_END(length);

    U_START(gunters chain, NULL);
    U_MULT(1, survey chain, 1);
    U_END(length);

    U_START(engineers chain, NULL);
    U_MULT(100, foot, 1);
    U_END(length);

    U_START(engineers link, NULL);
    U_MULT(1/100.0, engineers chain, 1);
    U_END(length);

    U_START(ramsdens chain, NULL);
    U_MULT(1, engineers chain, 1);
    U_END(length);

    U_START(ramsdens link, NULL);
    U_MULT(1, engineers link, 1);
    U_END(length);

    /* Originally defined as the distance from fingertip to fingertip with arms fully extended */
    U_START(fathom, "fath");
    U_MULT(6, survey foot, 1);
    U_END(length);

    /* Supposed to be one minute of latitude at the equator. That value is about 1855 m. Early estimates of the earth's
     * circumference were a bit off. The value of 1852 m was made the international standard in 1929. The US did not accept
     * this value until 1954. The UK still uses a different value. */
    U_START(nautical mile, NULL);
    U_MULT(1852, meter, 1);
    U_END(length);

    U_START(cable, NULL);
    U_MULT(1/10.0, nautical mile, 1);
    U_END(length);

    U_START(international cable, NULL);
    U_MULT(1, cable, 1);
    U_END(length);

    U_START(cablelength, NULL);
    U_MULT(1, cable, 1);
    U_END(length);

    U_START(survey cable, NULL);
    U_MULT(100, fathom, 1);
    U_END(length);

    U_START(navy cablelength, "navy cable");
    U_MULT(720, survey foot, 1);
    U_END(length);

    U_START(marine league, NULL);
    U_MULT(3, nautical mile, 1);
    U_END(length);

    /* American slang */
    U_START(footballfield, NULL);
    U_MULT(100, yard, 1);
    U_END(length);

    /* The UK lengths were defined by a bronze bar manufactured in 1844. Measurement of that bar revealed the dimensions given
     * here. */
    U_START(British nautical mile, NULL);
    U_MULT(6080, foot, 1);
    U_END(length);

    U_START(British cable, NULL);
    U_MULT(1/10.0, British nautical mile, 1);
    U_END(length);

    U_START(admiralty mile, NULL);
    U_MULT(1, British nautical mile, 1);
    U_END(length);

    U_START(admiralty cable, NULL);
    U_MULT(1, British cable, 1);
    U_END(length);

    U_START(sea mile, NULL);
    U_MULT(6000, foot, 1);
    U_END(length);

    /* Obscure British length. Given in Realm of Measure as the difference between successive shoe sizes */
    U_START(barleycorn, NULL);
    U_MULT(1/3.0, British inch, 1);
    U_END(length);

    /* Obscure British length. Originally the width of the thumbnail or 1/16 ft. This took on the general meaning of 1/16 and
     * settled on the nail or a yard, or 1/16 yard as its final value. "The World of Measurements" by H. Arthur Klein. */
    U_START(nail, NULL);
    U_MULT(2.25, British inch, 1);
    U_END(length);

    /* Obscure British length. */
    U_START(pole, NULL);
    U_MULT(16.5, British foot, 1);
    U_END(length);

    /* Obscure British length */
    U_START(rope, NULL);
    U_MULT(20, British foot, 1);
    U_END(length);

    /* Obscure British length. Supposed to be the distance from elbow to fingertips. */
    U_START(English ell, NULL);
    U_MULT(45, British inch, 1);
    U_END(length);

    /* Obscure British length. Supposed to be the distance from elbow to fingertips. */
    U_START(Flemish ell, NULL);
    U_MULT(27, British inch, 1);
    U_END(length);

    /* By `ell' we mean English ell, not Flemish ell. */
    U_START(ell, NULL);
    U_MULT(1, English ell, 1);
    U_END(length);

    /* Obscure British length. Supposed to be the distance from thumb to pinky with full hand extension. */
    U_START(span, NULL);
    U_MULT(9, British inch, 1);
    U_END(length);

    /* Old french distance measures, from French Weights and Measures Before the Revolution, by Zupko. */
    U_START(French foot, NULL);
    U_MULT(324.839, millimeter, 1);
    U_END(length);

    /* Old french distance measures, from French Weights and Measures Before the Revolution, by Zupko. */
    U_START(pied, NULL);
    U_MULT(1, French foot, 1);
    U_END(length);
    
    /* Old french distance measures, from French Weights and Measures Before the Revolution, by Zupko. */
    U_START(French inch, NULL);
    U_MULT(1/12.0, French foot, 1);
    U_END(length);
    
    /* Old french distance measures, from French Weights and Measures Before the Revolution, by Zupko. */
    U_START(French thumb, NULL);
    U_MULT(1, French inch, 1);
    U_END(length);
    
    /* Old french distance measures, from French Weights and Measures Before the Revolution, by Zupko. */
    U_START(pouce, NULL);
    U_MULT(1, French thumb, 1);
    U_END(length);
    
    /* Old french distance measures, from French Weights and Measures Before the Revolution, by Zupko. This is supposed to be
     * the size of the average barleycorn. */
    U_START(French line, NULL);
    U_MULT(1/12.0, French inch, 1);
    U_END(length);
    
    /* Old french distance measures, from French Weights and Measures Before the Revolution, by Zupko. This is supposed to be
     * the size of the average barleycorn. */
    U_START(ligne, NULL);
    U_MULT(1, French line, 1);
    U_END(length);
    
    /* Old french distance measures, from French Weights and Measures Before the Revolution, by Zupko. */
    U_START(French point, NULL);
    U_MULT(1/12.0, French line, 1);
    U_END(length);
    
    /* Old french distance measures, from French Weights and Measures Before the Revolution, by Zupko. */
    U_START(toise, NULL);
    U_MULT(6, French foot, 1);
    U_END(length);
    
    /* Human body measurement. Distance between points where the same foot hits the ground. May not be accurate. */
    U_START(geometric pace, NULL);
    U_MULT(5, foot, 1);
    U_END(length);

    /* Human body measurement. Distance between points where alternate feet touch the ground. May not be accurate. */
    U_START(pace, NULL);
    U_MULT(2.5, foot, 1);
    U_END(length);

    /* United States official military pace */
    U_START(US military pace, NULL);
    U_MULT(30, inch, 1);
    U_END(length);

    /* United States official military doubletime pace */
    U_START(US military doubletime pace, NULL);
    U_MULT(36, inch, 1);
    U_END(length);

    /* Human body measurement. The `finger' is either length or width. */
    U_START(fingerbreadth, "finger");
    U_MULT(7/8.0, inch, 1);
    U_END(length);

    /* Human body measurement. The `finger' is either length or width. */
    U_START(fingerlength, "finger");
    U_MULT(4.5, inch, 1);
    U_END(length);
    
    /* Human body measurement. The `palm' is either length or width. */
    U_START(palmwidth, "palm");
    U_MULT(4, inch, 1);
    U_END(length);

    /* Human body measurement. The `palm' is either length or width. */
    U_START(palmlength, "palm");
    U_MULT(8, inch, 1);
    U_END(length);

    /* Human body measurement. */
    U_START(hand, NULL);
    U_MULT(1, palmwidth, 1);
    U_END(length);

    /* Printing */
    U_START(computer point, NULL);
    U_MULT(1/72.0, inch, 1);
    U_END(length);

    /* Printing */
    U_START(printer point, NULL);
    U_MULT(1/72.27, inch, 1);
    U_END(length);

    /* By `point' we mean the printer's definition */
    U_START(point, NULL);
    U_MULT(1, printer point, 1);
    U_END(length);

    /* Printing */
    U_START(pica, NULL);
    U_MULT(12, point, 1);
    U_END(length);

    /* Printing */
    U_START(computer pica, NULL);
    U_MULT(12, computer point, 1);
    U_END(length);

    /* Printing */
    U_START(didopoint, NULL);
    U_MULT(1/72.0, French inch, 1);
    U_END(length);

    /* Printing */
    U_START(cicero, NULL);
    U_MULT(12, didopoint, 1);
    U_END(length);

    /* Printing */
    U_START(French printer point, NULL);
    U_MULT(1, didopoint, 1);
    U_END(length);

    U_START(astronomical unit, "au");
    U_MULT(1.49597871e11, meter, 1);
    U_END(length);

    U_START(mil, NULL);
    U_MULT(0.001, inch, 1);
    U_END(length);

    /* for measuring bullets */
    U_START(caliber, NULL);
    U_MULT(0.01, inch, 1);
    U_END(length);

    /* The `point' is used to measure rainfall in Australia */
    U_START(point, NULL);
    U_MULT(0.01, inch, 1);
    U_END(length);

    /* 80 turns of thread on a reel with a 54 inch circumference (varies for other kinds of thread) */
    U_START(skein cotton, NULL);
    U_MULT(80*54, inch, 1);
    U_END(length);

    /* cloth measurement */
    U_START(bolt, NULL);
    U_MULT(120, foot, 1);
    U_END(length);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_luminosity_derived_units(SAF_Db *database)
{
    U_DCLS("define_luminosity_derived_units");

    /*--------------------------------------------------------------------------------
     * Luminous intensity derived units
     *-------------------------------------------------------------------------------- */

    U_START(candle, NULL);
    U_MULT(1.02, candela, 1);
    U_END(luminous intensity);

    /* in use before candela */
    U_START(hefnerunit, NULL);
    U_MULT(0.9, candle, 1);
    U_END(luminous intensity);

    U_START(hefnercandle, NULL);
    U_MULT(1, hefnerunit, 1);
    U_END(luminous intensity);

    /* luminous intensity of 1 cm^2 of platinum at its temperature of solidification (2045 K) */
    U_START(violle, NULL);
    U_MULT(20.17, candela, 1);
    U_END(luminous intensity);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_thermo_temperature_derived_units(SAF_Db *database)
{
    U_DCLS("define_thermo_temperature_derived_units");

    /*--------------------------------------------------------------------------------
     * Thermodynamic temperature derived units
     *-------------------------------------------------------------------------------- */


    U_START(degree kelvin, NULL); /*not absolute temperature*/
    U_MULT(1, kelvin, 1);
    U_END(thermodynamic temperature);

    U_START(degree celsius, NULL); /*not absolute temperature*/
    U_MULT(1, kelvin, 1);
    U_END(thermodynamic temperature);

    U_START(degree fahrenheit, NULL); /*not absolute temperature*/
    U_MULT(5/9.0, degree celsius, 1);
    U_END(thermodynamic temperature);

    U_START(degree rankine, NULL); /*not absolute temperature*/
    U_MULT(1, degree fahrenheit, 1);
    U_END(thermodynamic temperature);

    /* The Reaumur scale was used in Europe and particularly in France.  It is defined to be 0 at the freezing point of water
     * and 80 at the boiling point.  Reaumur apparently selected 80 because it is divisible by many numbers. */
    U_START(degree reaumur, NULL); /*not absolute temperature*/
    U_MULT(10/8.0, degree celsius, 1);
    U_END(thermodynamic temperature);

    U_START(standard temperature, "stdtemp"); /*conventional*/
    U_MULT(273.15, kelvin, 1);
    U_END(thermodynamic temperature);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_amount_derived_units(SAF_Db *database)
{
    U_DCLS("define_amount_derived_units");

    /*--------------------------------------------------------------------------------
     * Amount of a substance derived units
     *-------------------------------------------------------------------------------- */

    /* Counting basic unit, these probably shouldn't be here since they're unitless */
    U_START(each, "ea");
    U_END(amount of a substance);

    U_START(pair, "pr");
    U_MULT(2, each, 1);
    U_END(amount of a substance);

    U_START(nest, NULL);
    U_MULT(3, each, 1);
    U_END(amount of a substance);

    U_START(dickers, NULL);
    U_MULT(10, each, 1);
    U_END(amount of a substance);

    U_START(dozen, "doz");
    U_MULT(12, each, 1);
    U_END(amount of a substance);

    U_START(bakers dozen, NULL);
    U_MULT(13, each, 1);
    U_END(amount of a substance);

    U_START(score, NULL);
    U_MULT(20, each, 1);
    U_END(amount of a substance);

    U_START(flock, NULL);
    U_MULT(40, each, 1);
    U_END(amount of a substance);

    U_START(timer, NULL);
    U_MULT(40, each, 1);
    U_END(amount of a substance);

    U_START(shock, NULL);
    U_MULT(60, each, 1);
    U_END(amount of a substance);

    U_START(gross, NULL);
    U_MULT(144, each, 1);
    U_END(amount of a substance);

    U_START(great gross, NULL);
    U_MULT(12, gross, 1);
    U_END(amount of a substance);

    /* Paper measure basic unit */
    U_START(quire, NULL);
    U_END(amount of a substance);

    /* Paper measure */
    U_START(ream, NULL);
    U_MULT(20, quire, 1);
    U_END(amount of a substance);

    /* Paper measure */
    U_START(bundle, NULL);
    U_MULT(2, ream, 1);
    U_END(amount of a substance);

    /* Paper measure */
    U_START(bale, NULL);
    U_MULT(5, bundle, 1);
    U_END(amount of a substance);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_area_derived_units(SAF_Db *database)
{
    U_DCLS("define_area_derived_units");

    /*--------------------------------------------------------------------------------
     * Area derived units
     *-------------------------------------------------------------------------------- */

    U_START(are, "a");
    U_MULT(100, square meter, 1);
    U_END(area);
    U_SI_PREFIX(are);

    /* Another name for `hectoare' */
    U_START(hectare, "ha");
    U_MULT(1, hectoare, 1);
    U_END(area);
    
    /* Used to measure cross section for particle physics collision. */
    U_START(barn, NULL);
    U_MULT(1e-28, square meter, 1);
    U_END(area);

    U_START(shed, NULL);
    U_MULT(1e-24, barn, 1);
    U_END(area);

    U_START(acre, NULL);
    U_MULT(10, chain, 2);
    U_END(area);

    /* Acre based on international foot */
    U_START(international acre, "acre");
    U_MULT(43560, foot, 2);
    U_END(area);

    U_START(section, NULL);
    U_MULT(1, statute mile, 2);
    U_END(area);

    U_START(township, NULL);
    U_MULT(36, section, 1);
    U_END(area);

    /* Area of land granted by the 1862 Homestead Act of the United States Congress */
    U_START(homestead, NULL);
    U_MULT(160, acre, 1);
    U_END(area);

    /* Obscure British unit */
    U_START(rood, NULL);
    U_MULT(1/4.0, acre, 1);
    U_END(area);

    /* Obscure British unit used in metal plating */
    U_START(basebox, NULL);
    U_MULT(31360, inch, 2);     /*should this be British Imperial inch? --rpm */
    U_END(area);
    
    /* Old french distance measures, from French Weights and Measures Before the Revolution, by Zupko. The arpent is 100
     * square perches, but the perche seems to vary a lot and can be 18 feet, 20 feet, or 22 feet. This measure was described
     * as being in common use in Canada in 1934 (Websters 2nd). The value given here is the Paris standard arpent. */
    U_START(arpent, NULL);
    U_MULT(SQUARE(180), pied, 2);
    U_END(area);

    /* Area of a one-inch diameter circle */
    U_START(circular inch, NULL);
    U_MULT(SAF_M_PI/4, inch, 2);
    U_END(area);

    /* Area of one-mil diameter circle */
    U_START(circular mil, "cir mil");
    U_MULT(SAF_M_PI/4, mil, 2);
    U_END(area);

    U_START(thousand circular mil, "mcm");
    U_MULT(1000, circular mil, 1);
    U_END(area);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_volume_derived_units(SAF_Db *database)
{
    U_DCLS("define_volume_derived_units");

    /*--------------------------------------------------------------------------------
     * Volume derived units
     *-------------------------------------------------------------------------------- */
    
    U_START(liter, "l");
    U_MULT(1e-3, cubic meter, 1);
    U_END(volume);
    U_SI_PREFIX(liter);

    /* Alternate spelling */
    U_START(litre, "l");
    U_MULT(1, liter, 1);
    U_END(volume);
    U_SI_PREFIX(litre);
    
    /* A liter before 1964 was the space occupied by 1 kg of pure water at the temperature of its maximum density under a
     * pressure of 1 atm. This was supposed to be 1000 cm^3 but it was discovered that the original measurement was off. */
    U_START(oldliter, "l");
    U_MULT(1.000028, liter, 1);
    U_END(volume);
    U_SI_PREFIX(oldliter);

    /* Abbreviation for cm^3 */
    U_START(cubic centimeter, "cc");
    U_MULT(1e-6, cubic meter, 1);
    U_END(volume);

    U_START(lambda, NULL);
    U_MULT(1, microliter, 1);
    U_END(volume);

    U_START(acre foot, NULL);
    U_MULT(1, acre, 1);
    U_MULT(1, survey foot, 1);
    U_END(volume);

    U_START(gallon, "gal");
    U_MULT(231, inch, 3);
    U_END(volume);

    U_START(quart, "qt");
    U_MULT(1/4.0, gallon, 1);
    U_END(volume);

    U_START(pint, "pt");
    U_MULT(1/2.0, quart, 1);
    U_END(volume);

    U_START(gill, NULL);
    U_MULT(1/4.0, pint, 1);
    U_END(volume);

    U_START(fluid ounce, "fl oz");
    U_MULT(1/16.0, pint, 1);
    U_END(volume);

    U_START(fluid dram, "fl dr");
    U_MULT(1/8.0, fluid ounce, 1);
    U_END(volume);

    U_START(minim, "min");
    U_MULT(1/60.0, fluid dram, 1);
    U_END(volume);

    U_START(liquid barrel, "bbl");
    U_MULT(31.5,  gallon, 1);
    U_END(volume);

    U_START(petroleum barrel, NULL);
    U_MULT(42, gallon, 1);
    U_END(volume);

    U_START(barrel, "bbl");
    U_MULT(1, petroleum barrel, 1);
    U_END(volume);

    U_START(hogshead, "hhd");
    U_MULT(63, gallon, 1);
    U_END(volume);

    U_START(firkin, NULL);
    U_MULT(9, gallon, 1);
    U_END(volume);

    U_START(dry barrel, NULL);
    U_MULT(7056, inch, 3);
    U_END(volume);

    U_START(dry gallon, NULL);
    U_MULT(268.8025, inch, 3);
    U_END(volume);

    U_START(dry quart, NULL);
    U_MULT(1/4.0, dry gallon, 1);
    U_END(volume);

    U_START(dry pint, NULL);
    U_MULT(1/8.0, dry quart, 1);
    U_END(volume);

    U_START(peck, "pk");
    U_MULT(8, dry quart, 1);
    U_END(volume);

    /* The Winchester Bushel was defined by William III in 1702 and legally adopted in the US in 1836. */
    U_START(bushel, "bu");
    U_MULT(4, peck, 1);
    U_END(volume);

    /* Wine and/or spirit measure */
    U_START(pony, NULL);
    U_MULT(1, fluid ounce, 1);
    U_END(volume);

    /* Wine and/or spirit measure */
    U_START(jigger, NULL);
    U_MULT(1.5, fluid ounce, 1);
    U_END(volume);

    U_START(shot, NULL);
    U_MULT(1, jigger, 1);
    U_END(volume);

    /* Alternate definition of shot */
    U_START(shot, NULL);
    U_MULT(1, fluid ounce, 1);
    U_END(volume);

    U_START(fifth, NULL);
    U_MULT(1/5.0, gallon, 1);
    U_END(volume);

    /* US industry standard, 1979 */
    U_START(wine bottle, NULL);
    U_MULT(740, milliliter, 1);
    U_END(volume);

    U_START(wine split, NULL);
    U_MULT(1/4.0, wine bottle, 1);
    U_END(volume);

    U_START(wine glass, NULL);
    U_MULT(4, fluid ounce, 1);
    U_END(volume);

    /* Standard 1979 definition */
    U_START(magnum, NULL);
    U_MULT(1.5, liter, 1);
    U_END(volume);

    /* Some references before 1979 */
    U_START(magnum, NULL);
    U_MULT(2, quart, 1);
    U_END(volume);

    U_START(metric tenth, NULL);
    U_MULT(375, milliliter, 1);
    U_END(volume);

    U_START(metric fifth, NULL);
    U_MULT(750, milliliter, 1);
    U_END(volume);

    U_START(metric quart, NULL);
    U_MULT(1, liter, 1);
    U_END(volume);

    /* French champagne size */
    U_START(split, NULL);
    U_MULT(200, milliliter, 1);
    U_END(volume);

    /* French champagne size */
    U_START(jeroboam, NULL);
    U_MULT(2, magnum, 1);
    U_END(volume);

    /* French champagne size */
    U_START(rehoboam, NULL);
    U_MULT(3, magnum, 1);
    U_END(volume);

    /* French champagne size */
    U_START(methuselah, NULL);
    U_MULT(4, magnum, 1);
    U_END(volume);

    /* French champagne size */
    U_START(salmanazar, NULL);
    U_MULT(6, magnum, 1);
    U_END(volume);

    /* French champagne size */
    U_START(balthazar, NULL);
    U_MULT(8, magnum, 1);
    U_END(volume);

    /* French champagne size */
    U_START(nebuchadnezzar, NULL);
    U_MULT(10, magnum, 1);
    U_END(volume);

    /* The British Imperial gallon was defined in 1824 to be the volume of water with weight 10 pounds at 62 deg F with a
     * pressure of 30 inHg. In 1963 it was defined to be the space occupied by 10 pounds of distilled water of density
     * 0.998859 g/ml weighed in air of density 0.001217 g/ml against weights of density 8.136 g/ml. The value given here is
     * given as an exact value by NIST Special Publication 811, 1995 Edition. */
    U_START(British gallon, "gal");
    U_MULT(4.54609, liter, 1);
    U_END(volume);

    U_START(Canadian gallon, "gal");
    U_MULT(1, British gallon, 1);
    U_END(volume);

    U_START(British quart, "qt");
    U_MULT(1/4.0, British gallon, 1);
    U_END(volume);

    U_START(British pint, "pt");
    U_MULT(1/2.0, British quart, 1);
    U_END(volume);

    U_START(British gill, NULL);
    U_MULT(1/4.0, British pint, 1);
    U_END(volume);

    U_START(British fluid ounce, "fl oz");
    U_MULT(1/20.0, British pint, 1);
    U_END(volume);

    U_START(British dram, "dr");
    U_MULT(1/8.0,  British fluid ounce, 1);
    U_END(volume);

    U_START(British minim, "min");
    U_MULT(1/60.0, British dram, 1);
    U_END(volume);

    U_START(British peck, "pk");
    U_MULT(2, British gallon, 1);
    U_END(volume);

    U_START(British bushel, "bu");
    U_MULT(4, British peck, 1);
    U_END(volume);

    U_START(British quarter, NULL);
    U_MULT(8, British bushel, 1);
    U_END(volume);

    U_START(British chaldron, NULL);
    U_MULT(36, British bushel, 1);
    U_END(volume);

    /* The following are obscure British volume measures. These units are generally traditional measures whose definitions
     * have fluctuated over the years. Often they depend on the quantity being measured. They are given here in terms of
     * British Imperial measures. For example: the puncheon may have historically been defined relative to the wine gallon or
     * beer gallon or ale gallon rather than the British Imperial gallon. */
    U_START(bag, NULL);
    U_MULT(4, British bushel, 1);
    U_END(volume);

    U_START(bucket, NULL);
    U_MULT(4, British gallon, 1);
    U_END(volume);

    U_START(last, NULL);
    U_MULT(40, British bushel, 1);
    U_END(volume);

    U_START(noggin, NULL);
    U_MULT(1, British gill, 1);
    U_END(volume);

    U_START(pottle, NULL);
    U_MULT(0.5, British gallon, 1);
    U_END(volume);

    U_START(puncheon, NULL);
    U_MULT(72, British gallon, 1);
    U_END(volume);

    U_START(seam, NULL);
    U_MULT(8, British bushel, 1);
    U_END(volume);

    U_START(coomb, NULL);
    U_MULT(4, British bushel, 1);
    U_END(volume);

    /* Obscure British measurement for ale and beer */
    U_START(British firkin, NULL);
    U_MULT(9, British gallon, 1);
    U_END(volume);

    /* Obscure British measurement for herring (about 750 fish) */
    U_START(cran, NULL);
    U_MULT(37.5, British gallon, 1);
    U_END(volume);
    
    U_START(British hogshead, "hhd");
    U_MULT(63, British gallon, 1);
    U_END(volume);

    /* Obscure British measurement for internal capacity of ships. Derived from the "tun cask" of wine. */
    U_START(register ton, NULL);
    U_MULT(100, foot, 3);
    U_END(volume);

    /* Obscure British measurement for ship's cargo freight or timber. Derived from the "tun cask" of wine */
    U_START(shipping ton, NULL);
    U_MULT(40, foot, 3);
    U_END(volume);

    /* Obscure British measurement of the approximate volume of a long-ton weight of sea water used to measure ship
     * displacement. */
    U_START(displacement ton, NULL);
    U_MULT(35, foot, 3);
    U_END(volume);

    /* Obscure British measurement from the 16th century. Variously defined depending on location. It probably doesn't make a
     * lot of sense to define in terms of imperial bushels (.5, 2, or 4). Ronald Zupko gives a value of 2 Winchester grain
     * bushels or about 70.5 liters in "French Weights and Measures Before the Revolution: A Dictionary of Provincial and Local
     * Units". */
    U_START(strike, NULL);
    U_MULT(70.5, liter, 1);
    U_END(volume);
    
    /* US cooking unit */
    U_START(cup, "c");
    U_MULT(8, fluid ounce, 1);
    U_END(volume);

    /* US cooking unit */
    U_START(tablespoon, "tbl");
    U_MULT(1/16.0, cup, 1);
    U_END(volume);

    /* Alternate abbr. */
    U_START(tablespoon, "tbsp");
    U_MULT(1, tablespoon, 1);
    U_END(volume);
    
    /* US cooking unit */
    U_START(teaspoon, "tsp");
    U_MULT(1/3.0, tablespoon, 1);
    U_END(volume);

    /* US cooking unit */
    U_START(metric cup, NULL);
    U_MULT(250, milliliter, 1);
    U_END(volume);

    /* British cooking unit */
    U_START(British cup, NULL);
    U_MULT(1/2.0, British pint, 1);
    U_END(volume);

    /* British cooking unit */
    U_START(British teacup, NULL);
    U_MULT(1/3.0, British pint, 1);
    U_END(volume);

    /* British cooking unit */
    U_START(British tablespoon, NULL);
    U_MULT(15, milliliter, 1);
    U_END(volume);

    /* Alternate definition */
    U_START(British tablespoon, "tbl");
    U_MULT(5/8.0, British fluid ounce, 1);
    U_END(volume);

    /* British cooking unit */
    U_START(British teaspoon, "tsp");
    U_MULT(1/3.0, British tablespoon, 1);
    U_END(volume);

    /* British cooking unit */
    U_START(dessert spoon, "dsp");
    U_MULT(2, British teaspoon, 1);
    U_END(volume);

    /* Australian cooking  unit */
    U_START(Australian tablespoon, "tbl");
    U_MULT(20, milliliter, 1);
    U_END(volume);

    /* Wood volume */
    U_START(cord, NULL);
    U_MULT(4*4*8, foot, 3);
    U_END(volume);

    /* Wood volume */
    U_START(cordfoot, NULL);
    U_MULT(1/8.0, cord, 1);
    U_END(volume);

    /* Wood volume */
    U_START(board foot, "fbm");
    U_MULT(1, foot, 2);
    U_MULT(1, inch, 1);
    U_END(volume);

    /* Wood volume */
    U_START(stere, "s");
    U_MULT(1, meter, 3);
    U_END(volume);
    U_SI_PREFIX(stere);

    U_START(standard, NULL);
    U_MULT(165, foot, 3);
    U_END(volume);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_velocity_derived_units(SAF_Db *database)
{
    U_DCLS("define_velocity_derived_units");

    /*--------------------------------------------------------------------------------
     * Velocity derived units
     *-------------------------------------------------------------------------------- */

    U_START(speed of light, "c");
    U_MULT(2.99792458e8, meter per second, 1);
    U_END(velocity);

    /* speed of sound */
    U_START(mach, NULL);
    U_MULT(331.46, meter per second, 1);
    U_END(velocity);

    /* CGS unit */
    U_START(kine, NULL);
    U_MULT(1, centimeter, 1);
    U_MULT(1, second, -1);
    U_END(velocity);
    U_SI_PREFIX(kine);

    U_START(knot, NULL);
    U_MULT(1, nautical mile, 1);
    U_MULT(1, hour, -1);
    U_END(velocity);

    /* British measurement based on bronze bar */
    U_START(British knot, NULL);
    U_MULT(1, British nautical mile, 1);
    U_MULT(1, hour, -1);
    U_END(velocity);

    U_START(admiralty knot, NULL);
    U_MULT(1, British knot, 1);
    U_END(velocity);

    U_START(mile per hour, "mph");
    U_MULT(1, mile, 1);
    U_MULT(1, hour, -1);
    U_END(velocity);

    U_START(kilometer per hour, "kph");
    U_MULT(1, kilometer, 1);
    U_MULT(1, hour, -1);
    U_END(velocity);

    U_START(kilometer per second, "kmps");
    U_MULT(1, kilometer, 1);
    U_MULT(1, second, -1);
    U_END(velocity);

    U_START(foot per minute, "fpm");
    U_MULT(1, foot, 1);
    U_MULT(1, minute, -1);
    U_END(velocity);

    U_START(foot per second, "fps");
    U_MULT(1, foot, 1);
    U_MULT(1, second, -1);
    U_END(velocity);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_acceleration_derived_units(SAF_Db *database)
{
    U_DCLS("define_acceleration_derived_units");

    /*--------------------------------------------------------------------------------
     * Acceleration derived units
     *-------------------------------------------------------------------------------- */

    U_START(leo, NULL);
    U_MULT(10, meter per second squared, 1);
    U_END(acceleration);
    U_SI_PREFIX(leo);

    U_START(gravity, NULL); /*conventional*/
    U_MULT(9.80665, meter per second squared, 1);
    U_END(acceleration);

    /* used to turn masses into forces */
    U_START(force, NULL);
    U_MULT(1, gravity, 1);
    U_END(acceleration);

    /* CGS unit used in geophysics. */
    U_START(galileo, "Gal");
    U_MULT(1, centimeter, 1);
    U_MULT(1, second, -2);
    U_END(acceleration);
    U_SI_PREFIX(galileo);

    /* Standard acceleration of gravety (exact) */
    U_START(gravity, NULL);
    U_MULT(9.80665, meter per second squared, 1);
    U_END(acceleration);

    /* A convenience unit which multiplies a mass to get a force */
    U_START(force, NULL);
    U_MULT(1, gravity, 1);
    U_END(acceleration);

    U_START(celo, NULL);
    U_MULT(1, foot, 1);
    U_MULT(1, second, -2);
    U_END(acceleration);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_wave_number_derived_units(SAF_Db *database)
{
    U_DCLS("define_wave_number_derived_units");

    /*--------------------------------------------------------------------------------
     * Wave number derived units
     *-------------------------------------------------------------------------------- */

    /* Proportionality constant in formula for distance between spectral lines. 1/lambda = R(1/m^2 - 1/n^2) is computed from
     * m_e c alpha^2 / 2h, but this figure is more accurate than the results of that computation. */
    U_START(Rinfinity, NULL);
    U_MULT(10973731.534, per meter, 1);
    U_END(wave number);

    /* Proposed as a CGS unit of wave number */
    U_START(kayser, NULL);
    U_MULT(1, centimeter, -1);
    U_END(wave number);
    U_SI_PREFIX(kayser);

    /* Even less common than kayser */
    U_START(balmer, NULL);
    U_MULT(1, kayser, 1);
    U_END(wave number);
    U_SI_PREFIX(balmer);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_mass_density_derived_units(SAF_Db *database)
{
    U_DCLS("define_mass_density_derived_units");

    /*--------------------------------------------------------------------------------
     * Mass density derived units
     *-------------------------------------------------------------------------------- */

    U_START(gammil, NULL);
    U_MULT(1e-3, kilogram per cubic meter, 1); /*mg/l*/
    U_END(mass density);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_specific_length_derived_units(SAF_Db *database)
{
    U_DCLS("define_specific_length_derived_units");

    /*--------------------------------------------------------------------------------
     * Specific length derived units
     *-------------------------------------------------------------------------------- */

    U_START(cotton yarncount, NULL);
    U_MULT(2520, foot, 1);
    U_MULT(1, pound, -1);
    U_END(specific length);

    U_START(linen yarncount, NULL);
    U_MULT(900, foot, 1);
    U_MULT(1, pound, -1);
    U_END(specific length);

    U_START(worsted yarncount, NULL);
    U_MULT(1680, foot, 1);
    U_MULT(1, pound, -1);
    U_END(specific length);

    U_START(metric yarncount, NULL);
    U_MULT(1, meter, 1);
    U_MULT(1, gram, -1);
    U_END(specific length);

    U_START(typp, NULL);
    U_MULT(1000, yard, 1);
    U_MULT(1, pound, -1);
    U_END(specific length);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_specific_volume_derived_units(SAF_Db *database)
{
    U_DCLS("define_specific_volume_derived_units");

    /*--------------------------------------------------------------------------------
     * Specific volume derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_current_density_derived_units(SAF_Db *database)
{
    U_DCLS("define_current_density_derived_units");

    /*--------------------------------------------------------------------------------
     * Current density derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_magnetic_field_strength_derived_units(SAF_Db *database)
{
    U_DCLS("define_magnetic_field_strength_derived_units");

    /*--------------------------------------------------------------------------------
     * Magnetic field strength derived units
     *-------------------------------------------------------------------------------- */

    U_START(oersted, "Oe");
    U_MULT(0.25*SAF_M_PI, abampere, 1);
    U_MULT(1, centimeter, -1);
    U_END(magnetic field strength);
    U_SI_PREFIX(oersted);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_amount_concentration_derived_units(SAF_Db *database)
{
    U_DCLS("define_amount_concentration_derived_units");

    /*--------------------------------------------------------------------------------
     * Amount concentration derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_luminence_derived_units(SAF_Db *database)
{
    U_DCLS("define_luminence_derived_units");

    /*--------------------------------------------------------------------------------
     * Luminance derived units
     *-------------------------------------------------------------------------------- */

    /* Intensity per projected area of an extended luminous source */
    U_START(stilb, "sb");
    U_MULT(1, candela, 1);
    U_MULT(1, centimeter, -2);
    U_END(luminance);
    U_SI_PREFIX(stilb);

    U_START(apostilb, "asb");
    U_MULT(1/SAF_M_PI, candela, 1);
    U_MULT(1, meter, -2);
    U_END(luminance);

    U_START(blondel, NULL);
    U_MULT(1, apostilb, 1);
    U_END(luminance);

    /* Measurements relating to dark adapted eyes */
    U_START(skot, NULL);
    U_MULT(1e-3, apostilb, 1);
    U_END(luminance);

    /* Equivalent luminance measures. These units are units which measure the luminance of a surface with a specified exitance
     * which obeys Lambert's law. Lambert's law specifies that luminous intensity of a perfectly diffuse luminous surface is
     * proportional to the cosine of the angle at which you view the luminous surface. */
    U_START(equivalent lux, NULL);
    U_MULT(1/SAF_M_PI, candela, 1);
    U_MULT(1, meter, -2);
    U_END(luminance);

    U_START(equivalanet phot, NULL);
    U_MULT(1/SAF_M_PI, candela, 1);
    U_MULT(1, centimeter, -2);
    U_END(luminance);

    U_START(lambert, "L");
    U_MULT(1/SAF_M_PI, candela, 1);
    U_MULT(1, centimeter, -2);
    U_END(luminance);

    U_START(footlambert, "fL");
    U_MULT(1/SAF_M_PI, candela, 1);
    U_MULT(1, foot, -2);
    U_END(luminance);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_mass_fraction_derived_units(SAF_Db *database)
{
    U_DCLS("define_mass_fraction_derived_units");

    /*--------------------------------------------------------------------------------
     * Mass fraction derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_volume_fraction_derived_units(SAF_Db *database)
{
    U_DCLS("define_volume_fraction_derived_units");

    /*--------------------------------------------------------------------------------
     * Volume fraction derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_plane_angle_derived_units(SAF_Db *database)
{
    U_DCLS("define_plane_angle_derived_units");

    /*--------------------------------------------------------------------------------
     * Plane angle derived units
     *-------------------------------------------------------------------------------- */

    U_START(circle, NULL);
    U_MULT(2*SAF_M_PI, radian, 1);
    U_END(plane angle);

    U_START(degree, "arcdeg");
    U_MULT(1/360.0, circle, 1);
    U_END(plane angle);

    U_START(arcminute, "arcmin");
    U_MULT(1/60.0, degree, 1);
    U_END(plane angle);

    U_START(arcsecond, "arcsec");
    U_MULT(1/60.0, arcminute, 1);
    U_END(plane angle);

    U_START(right angle, NULL);
    U_MULT(90, degree, 1);
    U_END(plane angle);

    U_START(quadrant, NULL);
    U_MULT(1/4.0, circle, 1);
    U_END(plane angle);

    U_START(sextant, NULL);
    U_MULT(1/6.0, circle, 1);
    U_END(plane angle);

    /* angular extent of one sign of the zodiac */
    U_START(sign, NULL);
    U_MULT(1/12.0, circle, 1);
    U_END(plane angle);

    /* another name for circle */
    U_START(turn, NULL);
    U_MULT(1, circle, 1);
    U_END(plane angle);

    /* another name for circle */
    U_START(revolution, "rev");
    U_MULT(1, circle, 1);
    U_END(plane angle);

    /* measure of grade */
    U_START(gon, NULL);
    U_MULT(1/100.0, right angle, 1);
    U_END(plane angle);

    /* another name for gon */
    U_START(grade, NULL);
    U_MULT(1, gon, 1);
    U_END(plane angle);

    U_START(centesimal minute, NULL);
    U_MULT(1/100.0, grade, 1);
    U_END(plane angle);

    U_START(centesimal second, NULL);
    U_MULT(1/100.0, centesimal minute, 1);
    U_END(plane angle);

    /* Official NIST definition. Another choice is 1e-3 rad. */
    U_START(milangle, NULL);
    U_MULT(1/6400.0, circle, 1);
    U_END(plane angle);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_solid_angle_derived_units(SAF_Db *database)
{
    U_DCLS("define_solid_angle_derived_units");

    /*--------------------------------------------------------------------------------
     * Solid angle derived units
     *-------------------------------------------------------------------------------- */

    U_START(sphere, NULL);
    U_MULT(4*SAF_M_PI, steradian, 1);
    U_END(solid angle);

    U_START(square degree, NULL);
    U_MULT(SQUARE(SAF_M_PI/180), steradian, 1);
    U_END(solid angle);

    U_START(square minute, "square min");
    U_MULT(SQUARE(1/60.0), square degree, 1);
    U_END(solid angle);

    U_START(square second, "square sec");
    U_MULT(SQUARE(1/60.0), square minute, 1);
    U_END(solid angle);

    /* another name for square minute */
    U_START(square arcminute, "square arcmin");
    U_MULT(1, square minute, 1);
    U_END(solid angle);

    /* another name for square second */
    U_START(square arcsecond, "square arcsec");
    U_MULT(1, square second, 1);
    U_END(solid angle);

    U_START(spherical right angle, NULL);
    U_MULT(SAF_M_PI/2, steradian, 1);
    U_END(solid angle);

    U_START(octant, NULL);
    U_MULT(SAF_M_PI/2, steradian, 1);
    U_END(solid angle);
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_frequency_derived_units(SAF_Db *database)
{
    U_DCLS("define_frequency_derived_units");

    /*--------------------------------------------------------------------------------
     * Frequency derived units
     *-------------------------------------------------------------------------------- */

    /* Occasionally used in spectroscopy */
    U_START(fresnel, NULL);
    U_MULT(1e12, hertz, 1);
    U_END(frequency);
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_force_derived_units(SAF_Db *database)
{
    U_DCLS("define_force_derived_units");

    /*--------------------------------------------------------------------------------
     * Force derived units
     *-------------------------------------------------------------------------------- */
    
    U_START(sthene, NULL);
    U_MULT(1000, newton, 1);
    U_END(force);

    U_START(funal, NULL);
    U_MULT(1, sthene, 1);
    U_END(force);

    /* CGS force */
    U_START(dyne, "dyn");
    U_MULT(1, centimeter, 1);
    U_MULT(1, gram, 1);
    U_MULT(1, second, -2);
    U_END(force);
    U_SI_PREFIX(dyne);

    /* Imperial force */
    U_START(ouncedal, NULL);
    U_MULT(1, ounce, 1);
    U_MULT(1, foot, 1);
    U_MULT(1, second, -2);
    U_END(force);
    
    /* Imperial force */
    U_START(poundal, "pdl");
    U_MULT(1, pound, 1);
    U_MULT(1, foot, 1);
    U_MULT(1, second, -2);
    U_END(force);

    /* Imperial force */
    U_START(tondal, NULL);
    U_MULT(1, ton, 1);
    U_MULT(1, foot, 1);
    U_MULT(1, second, -2);
    U_END(force);

    /* Mass units as forces */
    U_START(gram force, "gf");
    U_MULT(1, gram, 1);
    U_MULT(1, force, 1);
    U_END(force);
    U_SI_PREFIX(gram force);

    U_START(pound force, "lbf");
    U_MULT(1, pound, 1);
    U_MULT(1, force, 1);
    U_END(force);

    /* From kilopound? */
    U_START(kip, NULL);
    U_MULT(1000, pound force, 1);
    U_END(force);

    U_START(ton force, "tonf");
    U_MULT(1, ton, 1);
    U_MULT(1, force, 1);
    U_END(force);

    /* This is a mass, but we define it here because it's in terms of force */
    U_START(hyl, NULL);
    U_MULT(1, kilogram force, 1);
    U_MULT(1, second, 2);
    U_MULT(1, meter, -1);
    U_END(mass);

    /* CGS force */
    U_START(pond, NULL);
    U_MULT(1, gram force, 1);
    U_END(force);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_weight_derived_units(SAF_Db *database)
{
    U_DCLS("define_weight_derived_units");

    /*--------------------------------------------------------------------------------
     * Weight derived units
     *-------------------------------------------------------------------------------- */

    /* Standard weight of mercury (exact) */
    U_START(mercury weight, "Hg");
    U_MULT(13.5951, gram force, 1);
    U_MULT(1, centimeter, -3);
    U_END(weight);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_pressure_derived_units(SAF_Db *database)
{
    U_DCLS("define_pressure_derived_units");

    /*--------------------------------------------------------------------------------
     * Pressure derived units
     *-------------------------------------------------------------------------------- */
    
    U_START(pieze, NULL);
    U_MULT(1000, pascal, 1);
    U_END(pressure);

    U_START(atmosphere, "atm");
    U_MULT(101325, pascal, 1);
    U_END(pressure);

    U_START(technical atmosphere, "at");
    U_MULT(1, kilogram force, 1);
    U_MULT(1, centimeter, -2);
    U_END(pressure);
    
    /* About 1 atm */
    U_START(bar, NULL);
    U_MULT(1e5, pascal, 1);
    U_END(pressure);
    U_SI_PREFIX(bar);

    U_START(vac, NULL);
    U_MULT(1, millibar, 1);
    U_END(pressure);

    /* CGS pressure */
    U_START(barye, NULL);
    U_MULT(1, dyne, 1);
    U_MULT(1, centimeter, -2);
    U_END(pressure);
    U_SI_PREFIX(barye);

    /* old name for barye */
    U_START(barad, NULL);
    U_MULT(1, barye, 1);
    U_END(pressure);

    /* Pressure exerted by column of mercury */
    U_START(meter mercury, "mHg");
    U_MULT(1, meter, 1);
    U_MULT(1, mercury weight, 1);
    U_END(pressure);
    U_SI_PREFIX(meter mercury);

    /* Pressure exerted by 1 inch column of mercury */
    U_START(inch mercury, "inHg");
    U_MULT(1, inch, 1);
    U_MULT(1, mercury weight, 1);
    U_END(pressure);

    /* Pressure exerted by column of water */
    U_START(meter water, "mH2O");
    U_MULT(1, meter, 1);
    U_MULT(1, water weight, 1);
    U_END(pressure);
    U_SI_PREFIX(meter water);

    /* Pressure exerted by 1 inch column of water */
    U_START(inch water, "inH2O");
    U_MULT(1, inch, 1);
    U_MULT(1, water weight, 1);
    U_END(pressure);

    /* This unit should not be confused with `tor'. Both are named after Torricelli. */
    U_START(torr, NULL);
    U_MULT(1, millimeter mercury, 1);
    U_END(pressure);
    
    /* This unit should not be confused with `torr'. Both are named after Torricelli. */
    U_START(tor, NULL);
    U_MULT(1, pascal, 1);
    U_END(pressure);

    U_START(pound per square inch, "psi");
    U_MULT(1, pound force, 1);
    U_MULT(1, inch, -2);
    U_END(pressure);

    U_START(ton per square inch, "tsi");
    U_MULT(1, ton force, 1);
    U_MULT(1, inch, -2);
    U_END(pressure);
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_volumetric_flow_derived_units(SAF_Db *database)
{
    U_DCLS("define_volumetric_flow_derived_units");

    /*--------------------------------------------------------------------------------
     * Volumetric flow derived units
     *-------------------------------------------------------------------------------- */

    /* The miner's inch is defined in the OED as flow under 6 inches of pressure through a hole with an area of one square
     * inch, which appears to be inconsistent with the value given here (water flow measure, varies from 1.36 to 1.73 ft^3/min */
    U_START(miners inch, NULL);
    U_MULT(1.5, foot, 3);
    U_MULT(1, minute, -1);
    U_END(volumetric flow);

    /* volumetric gas flow unit */
    U_START(cumec, NULL);
    U_MULT(1, meter, 3);
    U_MULT(1, second, -1);
    U_END(volumetric flow);

    /* volumetric gas flow unit */
    U_START(cusec, NULL);
    U_MULT(1, foot, 3);
    U_MULT(1, second, -1);
    U_END(volumetric flow);
    
    U_START(gallon per hour, "gph");
    U_MULT(1, gallon, 1);
    U_MULT(1, hour, -1);
    U_END(volumetric flow);

    U_START(gallon per minute, "gpm");
    U_MULT(1, gallon, 1);
    U_MULT(1, minute, -1);
    U_END(volumetric flow);

    U_START(cubic foot per hour, "cfh");
    U_MULT(1, foot, 3);
    U_MULT(1, hour, -1);
    U_END(volumetric flow);

    U_START(cubic foot per minute, "cfm");
    U_MULT(1, foot, 3);
    U_MULT(1, minute, -1);
    U_END(volumetric flow);

    U_START(cubic foot per second, "cfs");
    U_MULT(1, foot, 3);
    U_MULT(1, second, -1);
    U_END(volumetric flow);

    U_START(liter per minute, "lpm");
    U_MULT(1, liter, 1);
    U_MULT(1, minute, -1);
    U_END(volumetric flow);
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_vapor_permeability_derived_units(SAF_Db *database)
{
    U_DCLS("define_vapor_permeability_derived_units");

    /*--------------------------------------------------------------------------------
     * vapor permeability derived units
     *-------------------------------------------------------------------------------- */

    /* The permeability or permeance of a substance determines how fast vapor flows through the substance. */
    U_START(perm zero, "perm 0C");
    U_MULT(1, grain, 1);
    U_MULT(1, hour, -1);
    U_MULT(1, foot, -2);
    U_MULT(1, inch mercury, -1);
    U_END(vapor permeability);

    U_START(perm, NULL);
    U_MULT(1, perm zero, 1);
    U_END(vapor permeability);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_electric_charge_derived_units(SAF_Db *database)
{
    U_DCLS("define_vapor_permeability_derived_units");

    /*--------------------------------------------------------------------------------
     * Electric charge derived units
     *-------------------------------------------------------------------------------- */

    U_START(electron charge, "e");
    U_MULT(1.60217733e-19, coulomb, 1);
    U_END(electric charge);

    U_START(abcoulomb, "abcoul");
    U_MULT(1, abampere, 1);
    U_MULT(1, second, 1);
    U_END(electric charge);

    /* Historical: charge that must flow to deposit or liberate one gram equivalent of any element. */
    U_START(faraday, NULL);
    U_MULT(96485.308, coulomb, 1);
    U_END(electric charge);

    /* Historical: this value is from a 1991 NIST publication */
    U_START(faraday_phys, NULL);
    U_MULT(96521.9, coulomb, 1);
    U_END(electric charge);

    /* Historical: this value is from a 1991 NIST publication */
    U_START(faraday_chem, NULL);
    U_MULT(96495.7, coulomb, 1);
    U_END(electric charge);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_energy_derived_units(SAF_Db *database)
{
    U_DCLS("define_energy_derived_units");

    /*--------------------------------------------------------------------------------
     * Energy derived units
     *-------------------------------------------------------------------------------- */

    /* CGS energy */
    U_START(erg, "erg");
    U_MULT(1, centimeter, 1);
    U_MULT(1, dyne, 1);
    U_END(energy);
    U_SI_PREFIX(erg);

    /* International Table calorie */
    U_START(calorie International Table, "cal");
    U_MULT(4.1868, joule, 1);
    U_END(energy);
    U_SI_PREFIX(calorie International Table);

    /* Thermochemical calorie */
    U_START(calorie thermochemical, "cal");
    U_MULT(4.184, joule, 1);
    U_END(energy);
    U_SI_PREFIX(calorie thermochemical);

    /* Energy to go from 14.5 to 15.5 degree Celsius */
    U_START(calorie fifteen, "cal");
    U_MULT(4.18580, joule, 1);
    U_END(energy);
    U_SI_PREFIX(calorie fifteen);

    /* Energy to go from 19.5 to 20.5 degree Celsius */
    U_START(calorie twenty, "cal");
    U_MULT(4.18190, joule, 1);
    U_END(energy);
    U_SI_PREFIX(calorie twenty);

    /* 1/100 energy to go from 0 to 100 degree Celsius */
    U_START(calorie mean, "cal");
    U_MULT(4.19002, joule, 1);
    U_END(energy);
    U_SI_PREFIX(calorie mean);

    /* By `calorie' we mean International Table calorie */
    U_START(calorie, "cal");
    U_MULT(1, calorie International Table, 1);
    U_END(energy);
    U_SI_PREFIX(calorie);

    /* By `Calorie' we mean kilocalorie */
    U_START(Calorie, "Cal");
    U_MULT(1, kilocalorie, 1);
    U_END(energy);

    /* Heat required to raise the temperature of a tonne of water from 14.5 to 15.5 degree Celsius */
    U_START(thermie, NULL);
    U_MULT(1e6, calorie fifteen, 1);
    U_END(energy);

    /* Used in refrigeration engineering */
    U_START(frigorie, NULL);
    U_MULT(1000, calorie fifteen, 1);
    U_END(energy);

    /* Energy acquired by a particle with charge e when it is accelerated through 1 V */
    U_START(electronvolt, "eV");
    U_MULT(1, electron charge, 1);
    U_MULT(1, volt, 1);
    U_END(energy);
    U_SI_PREFIX(electronvolt);

    /* International table BTU */
    U_START(British thermal unit, "Btu");
    U_MULT(1, calorie, 1);
    U_MULT(1, pound, 1);
    U_MULT(1, gram, -1);
    U_MULT(1, degree fahrenheit, 1);
    U_MULT(1, kelvin, -1);
    U_END(energy);

    /* Alternate abbreviation */
    U_START(British thermal unit, "B");
    U_MULT(1, British thermal unit, 1);
    U_END(energy);

    U_START(quad, NULL);
    U_MULT(1e15, British thermal unit, 1);
    U_END(energy);

    /* Exact definition, close to 1e5 btu */
    U_START(EC therm, NULL);
    U_MULT(1.05506e8, joule, 1);
    U_END(energy);

    /* Exact definition */
    U_START(US therm, NULL);
    U_MULT(1.054804e8, joule, 1);
    U_END(energy);

    U_START(therm, NULL);
    U_MULT(1, US therm, 1);
    U_END(energy);

    U_START(duty, NULL);
    U_MULT(1, foot, 1);
    U_MULT(1, pound force, 1);
    U_END(energy);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_power_derived_units(SAF_Db *database)
{
    U_DCLS("define_power_derived_units");

    /*--------------------------------------------------------------------------------
     * Power derived units
     *-------------------------------------------------------------------------------- */

    U_START(horsepower, "hp");
    U_MULT(550, foot, 1);
    U_MULT(1, pound force, 1);
    U_MULT(1, second, -1);
    U_END(power);

    U_START(metric horsepower, "hp");
    U_MULT(75, kilogram force, 1);
    U_MULT(1, meter, 1);
    U_MULT(1, second, -1);
    U_END(power);

    /* Another horse */
    U_START(electric horsepower, "hp");
    U_MULT(746, watt, 1);
    U_END(power);

    /* Another horse */
    U_START(boiler horsepower, "hp");
    U_MULT(9809.50, watt, 1);
    U_END(power);

    /* Yet another horse */
    U_START(water horsepower, "hp");
    U_MULT(746.043, watt, 1);
    U_END(power);

    U_START(British horsepower, "hp");
    U_MULT(745.70, watt, 1);
    U_END(power);

    U_START(donkeypower, NULL);
    U_MULT(250, watt, 1);
    U_END(power);

    U_START(poncelet, NULL);
    U_MULT(100, kilogram force, 1);
    U_MULT(1, meter, 1);
    U_MULT(1, second, -1);
    U_END(power);

    /* One ton refrigeration is the rate of heat extraction required to turn one ton of water to ice in a day. Ice is defined
     * to have a latent heat of 144 btu/lb. */
    U_START(tonrefrigeration, "tonref");
    U_MULT(1, ton, 1);
    U_MULT(144, British thermal unit, 1);
    U_MULT(1, pound, -1);
    U_MULT(1, day, -1);
    U_END(power);

    /* In vacuum science and some other applications, gas flow is measured as the product of volumetric flow and pressure.
     * This is useful because it makes it easy to compare with the flow at standard pressure (one atmosphere). It also
     * directly relates to the number of gas molecules per unit time, and hence to the mass flow if the molecular mass is
     * known. */
    U_START(sccm, NULL);
    U_MULT(1, atmosphere, 1);
    U_MULT(1, cubic centimeter, 1);
    U_MULT(1, minute, -1);
    U_END(power);

    U_START(sccs, NULL);
    U_MULT(1, atmosphere, 1);
    U_MULT(1, cubic centimeter, 1);
    U_MULT(1, second, -1);
    U_END(power);

    U_START(scfh, NULL);
    U_MULT(1, atmosphere, 1);
    U_MULT(1, foot, 3);
    U_MULT(1, hour, -1);
    U_END(power);

    U_START(scfm, NULL);
    U_MULT(1, atmosphere, 1);
    U_MULT(1, foot, 3);
    U_MULT(1, minute, -1);
    U_END(power);

    U_START(slpm, NULL);
    U_MULT(1, atmosphere, 1);
    U_MULT(1, liter, 1);
    U_MULT(1, minute, -1);
    U_END(power);

    U_START(lusec, NULL);
    U_MULT(1, liter, 1);
    U_MULT(1, micron, 1);
    U_MULT(1, mercury weight, 1);
    U_MULT(1, second, -1);
    U_END(power);
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_electric_potential_derived_units(SAF_Db *database)
{
    U_DCLS("define_electric_potential_derived_units");

    /*--------------------------------------------------------------------------------
     * Electric potential derived units
     *-------------------------------------------------------------------------------- */

    /* Used informally in UK physics labs */
    U_START(crocodile, NULL);
    U_MULT(1, megavolt, 1);
    U_END(electric potential);

    U_START(abvolt, NULL);
    U_MULT(1, dyne, 1);
    U_MULT(1, centimeter, 1);
    U_MULT(1, abampere, -1);
    U_MULT(1, second, -1);
    U_END(electric potential);

    U_START(intvolt, NULL);
    U_MULT(1.00033, volt, 1);
    U_END(electric potential);

    /* Meant to be electromotive force of a Daniell cell, but in error by 0.04 volt */
    U_START(daniell, NULL);
    U_MULT(1.042, volt, 1);
    U_END(electric potential);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_capacitance_derived_units(SAF_Db *database)
{
    U_DCLS("define_capacitance_derived_units");

    /*--------------------------------------------------------------------------------
     * Capacitance derived units
     *-------------------------------------------------------------------------------- */

    U_START(abfarad, NULL);
    U_MULT(1, abampere, 1);
    U_MULT(1, second, 1);
    U_MULT(1, abvolt, -1);
    U_END(capacitance);

    U_START(intfarad, NULL);
    U_MULT(0.999505, farad, 1);
    U_END(capacitance);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_electric_resistance_derived_units(SAF_Db *database)
{
    U_DCLS("define_electric_resistance_derived_units");

    /*--------------------------------------------------------------------------------
     * Electric resistance derived units
     *-------------------------------------------------------------------------------- */

    /* Alternate spelling */
    U_START(megohm, NULL);
    U_MULT(1, megaohm, 1);
    U_END(electric resistance);

    /* Alternate spelling */
    U_START(kilohm, NULL);
    U_MULT(1, kiloohm, 1);
    U_END(electric resistance);

    /* Alternate spelling */
    U_START(microhm, NULL);
    U_MULT(1, microohm, 1);
    U_END(electric resistance);

    U_START(abohm, NULL);
    U_MULT(1, abvolt, 1);
    U_MULT(1, abamp, -1);
    U_END(electric resistance);

    /* The resistance of a uniform column of mercury containing 14.4521 gram in a column 1.063 meters long and maintained at
     * zero degrees Celsius. */
    U_START(intohm, NULL);
    U_MULT(1.000495, ohm, 1);
    U_END(electric resistance);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_electric_conductance_derived_units(SAF_Db *database)
{
    U_DCLS("define_electric_conductance_derived_units");

    /*--------------------------------------------------------------------------------
     * Electric conductance derived units
     *-------------------------------------------------------------------------------- */
    
    /* another name for siemens */
    U_START(mho, NULL);
    U_MULT(1, siemens, 1);
    U_END(electric conductance);
    U_SI_PREFIX(mho);

    U_START(abmho, NULL);
    U_MULT(1, abohm, -1);
    U_END(electric conductance);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_magnetic_flux_derived_units(SAF_Db *database)
{
    U_DCLS("define_magnetic_flux_derived_units");

    /*--------------------------------------------------------------------------------
     * Magnetic flux derived units
     *-------------------------------------------------------------------------------- */

    U_START(maxwell, "Mx");
    U_MULT(1, abvolt, 1);
    U_MULT(1, second, 1);
    U_END(magnetic flux);
    U_SI_PREFIX(maxwell);

    /* Historical: named by and for Gisbert Kapp */
    U_START(kappline, NULL);
    U_MULT(6000, maxwell, 1);
    U_END(magnetic flux);
    
    U_START(unit pole, NULL);
    U_MULT(4*SAF_M_PI, maxwell, 1);
    U_END(magnetic flux);

    U_START(line, NULL);
    U_MULT(1e-8, weber, 1);
    U_END(magnetic flux);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_magnetic_flux_density_derived_units(SAF_Db *database)
{
    U_DCLS("define_magnetic_flux_density_derived_units");

    /*--------------------------------------------------------------------------------
     * Magnetic flux density derived units
     *-------------------------------------------------------------------------------- */

    U_START(gauss, "Gs");
    U_MULT(1, abvolt, 1);
    U_MULT(1, second, 1);
    U_MULT(1, centimeter, -2);
    U_END(magnetic flux density);
    U_SI_PREFIX(gauss);

    /* Alternate appreviation */
    U_START(gauss, "G");
    U_MULT(1, gauss, 1);
    U_END(magnetic flux density);
    U_SI_PREFIX(gauss);
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_inductance_derived_units(SAF_Db *database)
{
    U_DCLS("define_inductance_derived_units");

    /*--------------------------------------------------------------------------------
     * Inductance derived units
     *-------------------------------------------------------------------------------- */

    U_START(abhenry, NULL);
    U_MULT(1, abvolt, 1);
    U_MULT(1, second, 1);
    U_MULT(1, abampere, -1);
    U_END(inductance);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_luminous_flux_derived_units(SAF_Db *database)
{
    U_DCLS("define_luminous_flux_derived_units");

    /*--------------------------------------------------------------------------------
     * Luminous flux derived units
     *-------------------------------------------------------------------------------- */

    /* Alternate (old) abbreviation */
    U_START(lumen, "l");
    U_MULT(1, lumen, 1);
    U_END(luminous flux);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_illuminance_derived_units(SAF_Db *database)
{
    U_DCLS("define_illuminance_derived_units");

    /*--------------------------------------------------------------------------------
     * Illuminance derived units
     *-------------------------------------------------------------------------------- */

    U_START(phot, "ph");
    U_MULT(1, lumen, 1);
    U_MULT(1, centimeter, -2);
    U_END(illuminance);
    U_SI_PREFIX(phot);

    /* Illuminance from a 1 candela source at a distance of one foot */
    U_START(footcandle, NULL);
    U_MULT(1, lumen, 1);
    U_MULT(1, foot, -2);
    U_END(illuminance);

    /* Illuminance from a 1 candela source at a distance of one meter */
    U_START(metercandle, NULL);
    U_MULT(1, lumen, 1);
    U_MULT(1, meter, -2);
    U_END(illuminance);

    /* Proposed measure relating to dark adapted eyes */
    U_START(nox, NULL);
    U_MULT(1e-3, lux, 1);
    U_END(illuminance);
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_activity_of_radionuclide_derived_units(SAF_Db *database)
{
    U_DCLS("define_activity_of_radionuclide_derived_units");

    /*--------------------------------------------------------------------------------
     * Activity of a radionuclide derived units
     *-------------------------------------------------------------------------------- */

    /* Defined in 1910 as the radioactivity emitted by the amount of radon that is in equilibrium with 1 gram of radium. */
    U_START(curie, "Ci");
    U_MULT(3.7e10, becquerel, 1);
    U_END(activity of a radionuclide);

    U_START(rutherford, NULL);
    U_MULT(1e6, becquerel, 1);
    U_END(activity of a radionuclide);
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_absorbed_dose_derived_units(SAF_Db *database)
{
    U_DCLS("define_absorbed_dose_derived_units");

    /*--------------------------------------------------------------------------------
     * Absorbed dose derived units
     *-------------------------------------------------------------------------------- */

    U_START(rad, NULL);
    U_MULT(1e-2, gray, 1);
    U_END(absorbed dose);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_dose_equivalent_derived_units(SAF_Db *database)
{
    U_DCLS("define_dose_equivalent_derived_units");

    /*--------------------------------------------------------------------------------
     * Dose equivalent derived units
     *-------------------------------------------------------------------------------- */

    U_START(rem, NULL);
    U_MULT(1e-2, sievert, 1);
    U_END(dose equivalent);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_dynamic_viscosity_derived_units(SAF_Db *database)
{
    U_DCLS("define_dynamic_viscosity_derived_units");

    /*--------------------------------------------------------------------------------
     * Dynamic viscosity derived units
     *-------------------------------------------------------------------------------- */

    /* CGS viscosity, honors Jean Poiseuille */
    U_START(poise, "P");
    U_MULT(1, gram, 1);
    U_MULT(1, centimeter, -1);
    U_MULT(1, second, -1);
    U_END(dynamic viscosity);
    U_SI_PREFIX(poise);

    U_START(reyn, NULL);
    U_MULT(1, pound per square inch, 1);
    U_MULT(1, second, 1);
    U_END(dynamic viscosity);
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_moment_of_force_derived_units(SAF_Db *database)
{
    U_DCLS("define_moment_of_force_derived_units");

    /*--------------------------------------------------------------------------------
     * Moment of force derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_surface_tension_derived_units(SAF_Db *database)
{
    U_DCLS("define_surface_tension_derived_units");

    /*--------------------------------------------------------------------------------
     * Surface tension derived units
     *-------------------------------------------------------------------------------- */

    U_START(langley, NULL);
    U_MULT(1, calorie thermochemical, 1);
    U_MULT(1, centimeter, -2);
    U_END(surface tension);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_angular_velocity_derived_units(SAF_Db *database)
{
    U_DCLS("define_angular_velocity_derived_units");

    /*--------------------------------------------------------------------------------
     * Angular velocity derived units
     *-------------------------------------------------------------------------------- */

    U_START(revolution per minute, "rpm");
    U_MULT(1, revolution, 1);
    U_MULT(1, minute, -1);
    U_END(angular velocity);

    U_START(revolution per second, "rps");
    U_MULT(1, revolution, 1);
    U_MULT(1, second, -1);
    U_END(angular velocity);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_angular_acceleration_derived_units(SAF_Db *database)
{
    U_DCLS("define_angular_acceleration_derived_units");

    /*--------------------------------------------------------------------------------
     * Angular acceleration derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_irradiance_derived_units(SAF_Db *database)
{
    U_DCLS("define_irradiance_derived_units");

    /*--------------------------------------------------------------------------------
     * Irradiance derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_entropy_derived_units(SAF_Db *database)
{
    U_DCLS("define_entropy_derived_units");

    /*--------------------------------------------------------------------------------
     * Entropy derived units
     *-------------------------------------------------------------------------------- */

    U_START(clausius, NULL);
    U_MULT(1000, calorie, 1);
    U_MULT(1, kelvin, -1);
    U_END(entropy);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_specific_entropy_derived_units(SAF_Db *database)
{
    U_DCLS("define_specific_entropy_derived_units");

    /*--------------------------------------------------------------------------------
     * Specific entropy derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_specific_energy_derived_units(SAF_Db *database)
{
    U_DCLS("define_specific_energy_derived_units");

    /*--------------------------------------------------------------------------------
     * Specific energy derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_thermal_conductivity_derived_units(SAF_Db *database)
{
    U_DCLS("define_thermal_conductivity_derived_units");

    /*--------------------------------------------------------------------------------
     * Thermal conductivity derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_energy_density_derived_units(SAF_Db *database)
{
    U_DCLS("define_energy_density_derived_units");

    /*--------------------------------------------------------------------------------
     * Energy density derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_electric_field_strength_derived_units(SAF_Db *database)
{
    U_DCLS("define_energy_density_derived_units");

    /*--------------------------------------------------------------------------------
     * Electric field strength derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_electric_charge_density_derived_units(SAF_Db *database)
{
    U_DCLS("define_electric_charge_density_derived_units");

    /*--------------------------------------------------------------------------------
     * Electric charge density derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_electric_flux_density_derived_units(SAF_Db *database)
{
    U_DCLS("define_electric_flux_density_derived_units");

    /*--------------------------------------------------------------------------------
     * Electric flux density derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_permittivity_derived_units(SAF_Db *database)
{
    U_DCLS("define_permittivity_derived_units");

    /*--------------------------------------------------------------------------------
     * Permittivity derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_permeability_derived_units(SAF_Db *database)
{
    U_DCLS("define_permeability_derived_units");

    /*--------------------------------------------------------------------------------
     * Permeability derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_molar_energy_derived_units(SAF_Db *database)
{
    U_DCLS("define_molar_energy_derived_units");

    /*--------------------------------------------------------------------------------
     * Molar energy derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_molar_entropy_derived_units(SAF_Db *database)
{
    U_DCLS("define_molar_entropy_derived_units");

    /*--------------------------------------------------------------------------------
     * Molar entropy derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_exposure_derived_units(SAF_Db *database)
{
    U_DCLS("define_exposure_derived_units");

    /*--------------------------------------------------------------------------------
     * Exposure derived units
     *-------------------------------------------------------------------------------- */

    /* Ionizing radiation that produces 1 statcoulomb of charge in 1 cc of dry air at stp */
    U_START(roentgen, NULL);
    U_MULT(2.58e-4, coulomb, 1);
    U_MULT(1, kilogram, -1);
    U_END(exposure);

    /* Alternate spelling */
    U_START(rontgen, NULL);
    U_MULT(1, roentgen, 1);
    U_END(exposure);

    /* Unit of gamma ray dose delivered in one hour at a distance of 1 cm from a point source of 1 mg of radium enclosed in
     * platinum 0.5 mm thick. */
    U_START(sievertunit, NULL);
    U_MULT(8.38, rontgen, 1);
    U_END(exposure);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_radioactive_concentration_derived_units(SAF_Db *database)
{
    U_DCLS("define_radioactive_concentration_derived_units");

    /*--------------------------------------------------------------------------------
     * Radioactive concentration derived units
     *-------------------------------------------------------------------------------- */

    U_START(eman, NULL);
    U_MULT(1e-7, curie, 1);
    U_MULT(1, meter, -3);
    U_END(radioactive concentration);

    U_START(mache, NULL);
    U_MULT(3.7e-7, curie, 1);
    U_MULT(1, meter, -3);
    U_END(radioactive concentration);
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_absorbed_dose_rate_derived_units(SAF_Db *database)
{
    U_DCLS("define_absorbed_dose_rate_derived_units");

    /*--------------------------------------------------------------------------------
     * Absorbed dose rate derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_radiant_intensity_derived_units(SAF_Db *database)
{
    U_DCLS("define_radiant_intensity_derived_units");

    /*--------------------------------------------------------------------------------
     * Radiant intensity derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_radiance_derived_units(SAF_Db *database)
{
    U_DCLS("define_radiance_derived_units");

    /*--------------------------------------------------------------------------------
     * Radiance derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_information_derived_units(SAF_Db *database)
{
    U_DCLS("define_information_derived_units");

    /*--------------------------------------------------------------------------------
     * Information derived units
     *-------------------------------------------------------------------------------- */

    /* Not all machines had 8 bit bytes, but these days most of them do. But beware: for transmission over modems, a few extra
     * bits are used so there are actually 10 bits per byte. */
    U_START(byte, "B");
    U_MULT(8, bit, 1);
    U_END(information);
    U_SI_PREFIX(byte); /*none of the fractional multipliers make sense. oh well*/
    U_EIC_PREFIX(byte);

    /* Another name for mebibyte */
    U_START(meg, "MiB");
    U_MULT(1, mebibyte, 1);
    U_END(information);

    /* Entropy measured base e */
    U_START(nat, NULL);
    U_MULT(0.69314718056, bit, 1);
    U_END(information);

    /* log2(10) bits, or the entropy of a uniformly distributed random variable over 10 symbols. */
    U_START(hartley, NULL);
    U_MULT(3.32192809488, bit, 1);
    U_END(information);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_information_rate_derived_units(SAF_Db *database)
{
    U_DCLS("define_information_rate_derived_units");

    /*--------------------------------------------------------------------------------
     * Information rate derived units
     *-------------------------------------------------------------------------------- */
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_stress_optical_derived_units(SAF_Db *database)
{
    U_DCLS("define_stress_optical_derived_units");

    /*--------------------------------------------------------------------------------
     * Stress-optical coefficient derived units
     *-------------------------------------------------------------------------------- */

    /* square micron per newton */
    U_START(brewster, NULL);
    U_MULT(1e-12, square meter per newton, 1); /* micron^2/N */
    U_END(stress-optical coefficient);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_reciprocal_lens_focal_length_derived_units(SAF_Db *database)
{
    U_DCLS("define_reciprocal_lens_focal_length_derived_units");

    /*--------------------------------------------------------------------------------
     * Reciprocal lens focal length derived units
     *-------------------------------------------------------------------------------- */

    /* Alternate spelling */
    U_START(dioptre, NULL);
    U_MULT(1, diopter, 1);
    U_END(reciprocal lens focal length);
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_resistivity_derived_units(SAF_Db *database)
{
    U_DCLS("define_resistivity_derived_units");

    /*--------------------------------------------------------------------------------
     * Resistivity derived units
     *-------------------------------------------------------------------------------- */

    U_START(preece, NULL);
    U_MULT(1e13, ohm meter, 1);
    U_END(resistivity);
    U_SI_PREFIX(preece);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_magnetic_reluctance_derived_units(SAF_Db *database)
{
    U_DCLS("define_magnetic_reluctance_derived_units");

    /*--------------------------------------------------------------------------------
     * Magnetic reluctance derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_elastance_derived_units(SAF_Db *database)
{
    U_DCLS("define_elastance_derived_units");

    /*--------------------------------------------------------------------------------
     * Elastance derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_viscosity_derived_units(SAF_Db *database)
{
    U_DCLS("define_viscosity_derived_units");

    /*--------------------------------------------------------------------------------
     * Viscosity derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_specific_heat_derived_units(SAF_Db *database)
{
    U_DCLS("define_specific_heat_derived_units");

    /*--------------------------------------------------------------------------------
     * Specific heat derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_reciprocal_color_temperature_derived_units(SAF_Db *database)
{
    U_DCLS("define_reciprocal_color_temperature_derived_units");

    /*--------------------------------------------------------------------------------
     * Reciprocal color temperature derived units
     *-------------------------------------------------------------------------------- */

    U_START(mired, NULL);
    U_MULT(1e-6, reciprocal degree, 1);
    U_END(reciprocal color temperature);
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_concentration_derived_units(SAF_Db *database)
{
    U_DCLS("define_concentration_derived_units");

    /*--------------------------------------------------------------------------------
     * Concentration derived units
     *-------------------------------------------------------------------------------- */

    U_START(percent, "%");
    U_MULT(1/100.0, ratio, 1);
    U_END(concentration);

    /* another name for percent */
    U_START(parts per hundred, "%");
    U_MULT(1, percent, 1);
    U_END(concentration);

    /* alcohol content */
    U_START(proof, NULL);
    U_MULT(0.5, percent, 1);
    U_END(concentration);

    U_START(parts per million, "ppm");
    U_MULT(1e-6, ratio, 1);
    U_END(concentration);

    U_START(parts per billion, "ppb");
    U_MULT(1e-9, ratio, 1);
    U_END(concentration);

    U_START(parts per trillion, "ppt");
    U_MULT(1e-12, ratio, 1);
    U_END(concentration);

    /* gold purity */
    U_START(karat, NULL);
    U_MULT(1/24.0, ratio, 1);
    U_END(concentration);
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_mechanical_mobility_derived_units(SAF_Db *database)
{
    U_DCLS("define_mechanical_mobility_derived_units");

    /*--------------------------------------------------------------------------------
     * Mechanical mobility derived units
     *-------------------------------------------------------------------------------- */

    /* CGS unit */
    U_START(mobile ohm, "mohm");
    U_MULT(1, centimeter, 1);
    U_MULT(1, dyne, -1);
    U_MULT(1, second, -1);
    U_END(mechanical mobility);
    U_SI_PREFIX(mobile ohm);
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_mechanical_resistance_derived_units(SAF_Db *database)
{
    U_DCLS("define_mechanical_resistance_derived_units");

    /*--------------------------------------------------------------------------------
     * Mechanical resistance derived units
     *-------------------------------------------------------------------------------- */

    /* CGS unit */
    U_START(mechanical ohm, NULL);
    U_MULT(1, dyne, 1);
    U_MULT(1, second, 1);
    U_MULT(1, centimeter, -1);
    U_END(mechanical resistance);
    U_SI_PREFIX(mechanical ohm);
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_reciprocal_viscosity_derived_units(SAF_Db *database)
{
    U_DCLS("define_reciprocal_viscosity_derived_units");

    /*--------------------------------------------------------------------------------
     * Reciprocal viscosity derived units
     *-------------------------------------------------------------------------------- */

    U_START(rhe, NULL);
    U_MULT(1, poise, -1);
    U_END(reciprocal viscosity);
    U_SI_PREFIX(rhe);
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_kinematic_viscosity_derived_units(SAF_Db *database)
{
    U_DCLS("define_kinematic_viscosity_derived_units");

    /*--------------------------------------------------------------------------------
     * Kinematic viscosity derived units
     *-------------------------------------------------------------------------------- */

    /* CGS kinematic viscosity */
    U_START(stokes, "St");
    U_MULT(1, centimeter, 2);
    U_MULT(1, second, -1);
    U_END(kinematic viscosity);
    U_SI_PREFIX(stokes);

    /* another name for stokes */
    U_START(stoke, "St");
    U_MULT(1, stokes, 1);
    U_END(kinematic viscosity);
    U_SI_PREFIX(stoke);

    /* the old name for stokes */
    U_START(lentor, NULL);
    U_MULT(1, stokes, 1);
    U_END(kinematic viscosity);
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_momentum_derived_units(SAF_Db *database)
{
    U_DCLS("define_momentum_derived_units");

    /*--------------------------------------------------------------------------------
     * Momentum derived units
     *-------------------------------------------------------------------------------- */

    /* CGS unit */
    U_START(bole, NULL);
    U_MULT(1, gram, 1);
    U_MULT(1, centimeter, 1);
    U_MULT(1, second, -1);
    U_END(momentum);
    U_SI_PREFIX(bole);
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_luminous_energy_derived_units(SAF_Db *database)
{
    U_DCLS("define_luminous_energy_derived_units");

    /*--------------------------------------------------------------------------------
     * Luminous energy derived units
     *-------------------------------------------------------------------------------- */

    U_START(lumberg, NULL);
    U_MULT(1, talbot, 1);
    U_END(luminous energy);
    
    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_luminous_energy_per_area_derived_units(SAF_Db *database)
{
    U_DCLS("define_luminous_energy_per_area_derived_units");

    /*--------------------------------------------------------------------------------
     * Luminous energy per area derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_entropy_flow_derived_units(SAF_Db *database)
{
    U_DCLS("define_entropy_flow_derived_units");

    /*--------------------------------------------------------------------------------
     * Entropy flow derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_thermal_resistance_derived_units(SAF_Db *database)
{
    U_DCLS("define_thermal_resistance_derived_units");

    /*--------------------------------------------------------------------------------
     * Thermal resistance derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_thermal_inducatance_derived_units(SAF_Db *database)
{
    U_DCLS("define_thermal_inducatance_derived_units");

    /*--------------------------------------------------------------------------------
     * Thermal inducatance derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_electric_dipole_moment_derived_units(SAF_Db *database)
{
    U_DCLS("define_electric_dipole_moment_derived_units");

    /*--------------------------------------------------------------------------------
     * Electric dipole moment derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_electric_dipole_moment_per_area_derived_units(SAF_Db *database)
{
    U_DCLS("define_electric_dipole_moment_per_area_derived_units");

    /*--------------------------------------------------------------------------------
     * Electric dipole moment per area derived units
     *-------------------------------------------------------------------------------- */

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_gaussian_system_units(SAF_Db *database)
{
    U_DCLS("define_gaussian_system_units");

    /*--------------------------------------------------------------------------------
     * Units from the Gaussian system
     *-------------------------------------------------------------------------------- */

    U_START(statampere, "statamp");
    U_MULT(10, ampere, 1);
    U_MULT(1, centimeter, 1);
    U_MULT(1, second, -1);
    U_MULT(1, speed of light, -1);      /*forward*/
    U_END(electric current);
    
    U_START(statcoulomb, "esu");
    U_MULT(1, statamp, 1);
    U_MULT(1, second, 1);
    U_END(electric charge);

    U_START(franklin, NULL);
    U_MULT(1, statcoulomb, 1);
    U_END(electric charge);

    U_START(statvolt, NULL);
    U_MULT(1, dyne, 1);
    U_MULT(1, centimeter, 1);
    U_MULT(1, statampere, -1);
    U_MULT(1, second, -1);
    U_END(electric potential);

    U_START(statfarad, NULL);
    U_MULT(1, statamp, 1);
    U_MULT(1, second, 1);
    U_MULT(1, statvolt, -1);
    U_END(capacitance);

    U_START(jar, NULL);
    U_MULT(1000, statfarad, 1);
    U_END(capacitance);

    U_START(statohm, NULL);
    U_MULT(1, statvolt, 1);
    U_MULT(1, statamp, -1);
    U_END(electric resistance);
    
    U_START(statmho, NULL);
    U_MULT(1, statohm, -1);
    U_END(electric conductance);
    
    U_START(statmaxwell, NULL);
    U_MULT(1, statvolt, 1);
    U_MULT(1, second, 1);
    U_END(magnetic flux);
    
    U_START(stathenry, NULL);
    U_MULT(1, statvolt, 1);
    U_MULT(1, second, 1);
    U_MULT(1, statamp, -1);
    U_END(inductance);

    U_START(debye, NULL);
    U_MULT(1e-18, statcoulomb, 1);
    U_MULT(1, centimeter, 1);
    U_END(electric dipole moment);

    U_START(helmholtz, NULL);
    U_MULT(1, debye, 1);
    U_MULT(1, angstrom, -2);
    U_END(electric dipole moment per area);

    U_FINISH;
    return SAF_SUCCESS;
}

static int
define_miscellaneous_definitions_units(SAF_Db *database)
{
    U_DCLS("define_miscellaneous_definitions_units");

    /*--------------------------------------------------------------------------------
     * Miscellaneous definitions that would have had forward references
     *-------------------------------------------------------------------------------- */

    /* CGS mass which is accelerated at 1 cm/s^2 by 1 gram force */
    U_START(glug, NULL);
    U_MULT(1, gram force, 1);           /*forward*/
    U_MULT(1, second, 2);
    U_MULT(1, centimeter, -1);
    U_END(mass);

    U_START(slug, NULL);
    U_MULT(1, pound force, 1);          /*forward*/
    U_MULT(1, second, 2);
    U_MULT(1, foot, -1);
    U_END(mass);

    U_START(geepound, NULL);
    U_MULT(1, slug, 1);
    U_END(mass);

    /* The 365.25 day year is specified in NIST publication 811 */
    U_START(lightyear, NULL);
    U_MULT(1, speed of light, 1);       /*forward*/
    U_MULT(365.25, day, 1);
    U_END(length);

    U_START(lightsecond, NULL);
    U_MULT(1, speed of light, 1);       /*forward*/
    U_MULT(1, second, 1);
    U_END(length);

    U_START(lightminute, NULL);
    U_MULT(1, speed of light, 1);       /*forward*/
    U_MULT(1, minute, 1);
    U_END(length);

    /* Unit of length equal to distance from the sun to a point having heliocentric parallax of 1 arcsec (derived from
     * parallax second). The formula should use tangent, but the error is about 1e-12. */
    U_START(parsec, "pc");
    U_MULT(1, astronomical unit, 1);
    U_MULT(1, radian, 1);
    U_MULT(1, arcsecond, -1);           /*forward*/
    U_END(length);

    /* Measures permeability to fluid flow. One darcy is the permeability of a medium that allows a flow of cc/s of a liquid
     * of centipoise viscosity under a pressure of one atm/cm. */
    U_START(darcy, NULL);
    U_MULT(1, centipoise, 1);           /*forward*/
    U_MULT(1, centimeter, 2);
    U_MULT(1, second, -1);
    U_MULT(1, atmosphere, -1);
    U_END(area);

    U_FINISH;
    return SAF_SUCCESS;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Units
 * Purpose:     Initialize a new database
 *
 * Description: Since this database might be on read-only media when it is browsed and since this unit implementation only
 *              supports permanent units, we must define all the units that can be used as memory-targets by the reader.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Programmer:  Robb Matzke
 *              Friday, April 21, 2000
 *
 * Modifications:
 *              Robb Matzke, 2000-06-13
 *              Added additional definitions.
 *
 *              Jim Reus, 28Jul2000
 *              Split-out from original units.c source file to
 *              ease compiler resource burden.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
initOR_unit(SAF_Db *database)
{
    int retval = SAF_SUCCESS;

    TESTING("units");
    SAF_TRY_BEGIN {

    if (define_special_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_SI_basic_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_SI_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_time_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_mass_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_length_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_luminosity_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_thermo_temperature_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_amount_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_area_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_volume_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_velocity_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_acceleration_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_wave_number_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_mass_density_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_specific_length_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_specific_volume_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_current_density_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_magnetic_field_strength_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_amount_concentration_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_luminence_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_mass_fraction_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_volume_fraction_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_plane_angle_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_solid_angle_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_frequency_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_force_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_weight_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_pressure_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_volumetric_flow_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_vapor_permeability_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_electric_charge_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_energy_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_power_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_electric_potential_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_capacitance_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_electric_resistance_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_electric_conductance_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_magnetic_flux_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_magnetic_flux_density_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_inductance_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_luminous_flux_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_illuminance_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_activity_of_radionuclide_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_absorbed_dose_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_dose_equivalent_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_dynamic_viscosity_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_moment_of_force_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_surface_tension_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_angular_velocity_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_angular_acceleration_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_irradiance_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_entropy_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_specific_entropy_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_specific_energy_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_thermal_conductivity_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_energy_density_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_electric_field_strength_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_electric_charge_density_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_electric_flux_density_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_permittivity_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_permeability_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_molar_energy_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_molar_entropy_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_exposure_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_radioactive_concentration_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_absorbed_dose_rate_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_radiant_intensity_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_radiance_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_information_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_information_rate_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_stress_optical_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_reciprocal_lens_focal_length_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_resistivity_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_magnetic_reluctance_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_elastance_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_viscosity_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_specific_heat_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_reciprocal_color_temperature_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_concentration_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_mechanical_mobility_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_mechanical_resistance_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_reciprocal_viscosity_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_kinematic_viscosity_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_momentum_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_luminous_energy_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_luminous_energy_per_area_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_entropy_flow_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_thermal_resistance_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_thermal_inducatance_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_electric_dipole_moment_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_electric_dipole_moment_per_area_derived_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_gaussian_system_units(database) != SAF_SUCCESS) goto gotFailure;
    if (define_miscellaneous_definitions_units(database) != SAF_SUCCESS) goto gotFailure;

    PASSED;
    goto theExit;

gotFailure:
    retval = SAF_FAILURE;
    FAILED;

theExit:
    ;

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
	    retval = SAF_FAILURE;
        }
    } SAF_TRY_END;

    return retval;
}

/* Initialize roles in the registry */
static int
initOR_role(SAF_Db *database)
{
    int retval = SAF_SUCCESS;

    TESTING("roles");
    SAF_TRY_BEGIN {

    ROLE_DECL(topology);
    ROLE_DECL(processor);
    ROLE_DECL(block);
    ROLE_DECL(domain);
    ROLE_DECL(assembly);
    ROLE_DECL(material);
    ROLE_DECL(space_slice);
    ROLE_DECL(param_slice);

    PASSED;

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
	    retval = SAF_FAILURE;
        }
    } SAF_TRY_END;

    return retval;
}

/* Initialize basis types in the registry */
static int
initOr_basis(SAF_Db *database)
{
    int retval = SAF_SUCCESS;

    TESTING("bases");
    SAF_TRY_BEGIN {

    BASIS_DECL(unity);
    BASIS_DECL(cartesian);
    BASIS_DECL(spherical);
    BASIS_DECL(cylindrical);
    BASIS_DECL(uppertri);
    BASIS_DECL(varying);

    PASSED;

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
	    retval = SAF_FAILURE;
        }
    } SAF_TRY_END;

    return retval;
}

/* Initialize algebraic types in the registry */
static int
initOr_algebraic(SAF_Db *database)
{
    int retval = SAF_SUCCESS;

    TESTING("algebraic types");
    SAF_TRY_BEGIN {

    ALGEBRAIC_DECL(scalar,              SAF_TRISTATE_FALSE);
    ALGEBRAIC_DECL(vector,              SAF_TRISTATE_FALSE);
    ALGEBRAIC_DECL(component,           SAF_TRISTATE_FALSE);
    ALGEBRAIC_DECL(tensor,              SAF_TRISTATE_FALSE);
    ALGEBRAIC_DECL(symmetric tensor,    SAF_TRISTATE_FALSE);
    ALGEBRAIC_DECL(tensor,              SAF_TRISTATE_FALSE);
    ALGEBRAIC_DECL(tuple,               SAF_TRISTATE_FALSE);
    ALGEBRAIC_DECL(field,               SAF_TRISTATE_TRUE);

    PASSED;

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
	    retval = SAF_FAILURE;
        }
    } SAF_TRY_END;

    return retval;
}

/* Initialize evaluation types in the registry */
static int
initOr_evaluation(SAF_Db *database)
{
    int retval = SAF_SUCCESS;

    TESTING("evaluation schemes");
    SAF_TRY_BEGIN {

    EVAL_DECL(constant);
    EVAL_DECL(piecewise constant);
    EVAL_DECL(piecewise linear);
    EVAL_DECL(uniform);

    PASSED;

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
	    retval = SAF_FAILURE;
        }
    } SAF_TRY_END;

    return retval;
}

/* Initialize relation representation types in the registry */
static int
initOR_relrep(SAF_Db *database)
{
    int retval = SAF_SUCCESS;

    TESTING("relation representation types");
    SAF_TRY_BEGIN {

    RELREP_DECL(structured,             SAF_STRUCTURED_ID);
    RELREP_DECL(unstructured,           SAF_UNSTRUCTURED_ID);
    RELREP_DECL(arbitrary,              SAF_ARBITRARY_ID);
    RELREP_DECL(hslab,                  SAF_HSLAB_ID);
    RELREP_DECL(tuples,                 SAF_TUPLES_ID);
    RELREP_DECL(totality,               SAF_TOTALITY_ID);

    PASSED;

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
	    retval = SAF_FAILURE;
        }
    } SAF_TRY_END;

    return retval;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Pre-Defined Types
 * Purpose:	Define a bunch of pre-defined database objects for SAF
 *
 * Description:	SAF uses a bunch of pre-defined types for a bunch of different classes of objects. Examples, are units,
 *		quantities, cell-types, algebraic types, structured topologies, field interpolation functions, etc.
 *
 *		By and large, in early implementation's of SAF, most of these predefined types will be implemented as macro
 *		constants in SAF's public, saf.h file.
 *
 *		However, we have already developed a whole units interface for SAF to deal with pre-defined units as SAF
 *		objects, rather some macro constants in SAF's saf.h file. This utility function is used to generate a .saf
 *		file with these pre-defined units in them.
 *
 *		As more pre-defined types are dealt with as full-fledged SAF objects, rather than macro constants, this utility
 *		will be expanded to include the work necessary to define them as SAF objects as well.
 *
 * Issues:	To begin an empty .saf file, we need to disable the ImportFile property before we call saf_open_database.
 *		Otherwise, during the call to saf_open_database, we'll wind up copying the units in some existing .saf file
 *		rather than defining new ones here.
 *
 *		This should really only ever be run as a serial SAF client; not parallel. There is no point in having this
 *		run parallel as all this will do is make it take longer. 
 *
 * Programmer:  Robb Matzke
 *              Thursday, March 30, 2000
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
main(int __UNUSED__ argc, char __UNUSED__ **argv)
{
    SAF_Db       *db;
    SAF_DbProps  *db_props;
    SAF_LibProps *lib_props;

#ifdef HAVE_PARALLEL
    MPI_Init(&argc,&argv);
#endif

    /* initialize SAF */
    if (NULL==(lib_props=saf_createProps_lib())) {
        fprintf(stderr, "saf_createProps_library() failed\n");
        exit(1);
    }
    if (SAF_SUCCESS!=saf_init(lib_props)) {
        fprintf(stderr, "saf_init() failed\n");
        exit(1);
    }

    /* Open the database */
    if (NULL==(db_props=saf_createProps_database())) {
        fprintf(stderr, "saf_createProps_database() failed\n");
        exit(1);
    }
    if (SAF_SUCCESS!=saf_setProps_Clobber(db_props)) {
        fprintf(stderr, "saf_setProps_Clobber() failed\n");
        exit(1);
    }
    if (NULL==(db=saf_open_database("RegistryHeavy.saf", db_props))) {
        fprintf(stderr, "saf_open_database() failed\n");
        exit(1);
    }

    /* build pre-defined quantities and units. This stuff will be dissappearing when the OR is done. */
    initOR_quantity(db);
    initOR_unit(db);
    initOR_role(db);
    initOr_basis(db);
    initOr_algebraic(db);
    initOr_evaluation(db);
    initOR_relrep(db);
    
    /* Close the database */
    if (SAF_SUCCESS!=saf_close_database(db)) {
        fprintf(stderr, "saf_close_database() failed\n");
        exit(1);
    }

    saf_final();

#ifdef HAVE_PARALLEL
    MPI_Finalize();
#endif

    return 0;
}
