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
#include <math.h>

#define URL "standard"

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
    /* Define the seven basic quantities that are part of every registry */
    SAF_Quantity        *TIME    = _SAF_GLOBALS.quant[SAF_BASEQ_TIME];
    SAF_Quantity        *MASS    = _SAF_GLOBALS.quant[SAF_BASEQ_MASS];
    SAF_Quantity        *LENGTH  = _SAF_GLOBALS.quant[SAF_BASEQ_LENGTH];


    Q_START("not applicable", "NA");
    Q_END();

    Q_START("area", NULL);                              /* hectare                      */
    Q_NUM(LENGTH, 2);
    Q_END();

    Q_START("pressure", NULL); /* or stress */          /* pascal                       */
    Q_NUM(MASS, 1);
    Q_DEN(LENGTH, 1);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("force", NULL);                             /* newton                       */
    Q_NUM(LENGTH, 1);
    Q_NUM(MASS, 1);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("velocity", "vel"); /*or speed*/            /* meter per second             */
    Q_NUM(LENGTH, 1);
    Q_DEN(TIME, 1);
    Q_END();

    Q_START("acceleration", "accel");                   /* meter per second squared     */
    Q_NUM(LENGTH, 1);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("energy", NULL);                            /* joule                        */
    Q_NUM(LENGTH, 2);
    Q_NUM(MASS, 1);
    Q_DEN(TIME, 2);
    Q_END();

    Q_START("frequency", "freq");                       /* hertz                        */
    Q_DEN(TIME, 1);
    Q_END();

    Q_START("plane angle", NULL);                       /* radian                       */
    Q_NUM(LENGTH, 1);
    Q_DEN(LENGTH, 1);
    Q_END();

    return SAF_SUCCESS;
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

    /* Basic unit of acceleration has no name */
    U_START(meter per second squared, NULL);
    U_END(acceleration);

    U_START(newton, "N");
    U_END(force);
    U_SI_PREFIX(newton);

    U_START(pascal, "Pa");
    U_END(pressure);
    U_SI_PREFIX(pascal);

    U_START(joule, "J");
    U_END(energy);
    U_SI_PREFIX(joule);

    U_START(hertz, "Hz");
    U_END(frequency);
    U_SI_PREFIX(hertz);

    /* Basic unit of velocity has no name */
    U_START(meter per second, NULL);
    U_END(velocity);

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
    
    U_START(pound force, "lbf");
    U_MULT(1, pound, 1);
    U_MULT(1, force, 1);
    U_END(force);

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

    U_START(inch, "in");
    U_MULT(0.0254, meter, 1);
    U_END(length);

    U_START(mile, "mi");
    U_MULT(1609.344, meter, 1);
    U_END(length);

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

    /* English units of force are in terms of mass... */
    U_START(pound, "lb");
    U_MULT(453.59237, gram, 1);
    U_END(mass);

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
    
    U_START(pound per square inch, "psi");
    U_MULT(1, pound force, 1);
    U_MULT(1, inch, -2);
    U_END(pressure);

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

    U_START(gravity, NULL); /*conventional*/
    U_MULT(9.80665, meter per second squared, 1);
    U_END(acceleration);

    /* used to turn masses into forces */
    U_START(force, NULL);
    U_MULT(1, gravity, 1);
    U_END(acceleration);

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
    int          tmp;

    if ((tmp=define_special_units(database)) != SAF_SUCCESS) return tmp;
    if ((tmp=define_SI_basic_units(database)) != SAF_SUCCESS) return tmp;
    if ((tmp=define_SI_derived_units(database)) != SAF_SUCCESS) return tmp;
    if ((tmp=define_mass_derived_units(database)) != SAF_SUCCESS) return tmp;
    if ((tmp=define_length_derived_units(database)) != SAF_SUCCESS) return tmp;
    if ((tmp=define_acceleration_derived_units(database)) != SAF_SUCCESS) return tmp;
    if ((tmp=define_force_derived_units(database)) != SAF_SUCCESS) return tmp;
    if ((tmp=define_pressure_derived_units(database)) != SAF_SUCCESS) return tmp;

    return SAF_SUCCESS;
}

/* Initialize roles in the registry */
static int
initOR_role(SAF_Db *database)
{
    ROLE_DECL(topology);                        /*1*/
    ROLE_DECL(processor);                       /*2*/
    ROLE_DECL(block);                           /*3*/
    ROLE_DECL(domain);                          /*4*/
    ROLE_DECL(assembly);                        /*5*/
    ROLE_DECL(material);                        /*6*/
    ROLE_DECL(space_slice);                     /*7*/
    ROLE_DECL(param_slice);                     /*8*/

    return SAF_SUCCESS;
}

/* Initialize basis types in the registry */
static int
initOr_basis(SAF_Db *database)
{
    BASIS_DECL(unity);
    BASIS_DECL(cartesian);
    BASIS_DECL(spherical);
    BASIS_DECL(cylindrical);
    BASIS_DECL(uppertri);
    BASIS_DECL(varying);

    return SAF_SUCCESS;
}

/* Initialize algebraic types in the registry */
static int
initOr_algebraic(SAF_Db *database)
{
    ALGEBRAIC_DECL(scalar,              SAF_TRISTATE_FALSE);
    ALGEBRAIC_DECL(vector,              SAF_TRISTATE_FALSE);
    ALGEBRAIC_DECL(component,           SAF_TRISTATE_FALSE);
    ALGEBRAIC_DECL(tensor,              SAF_TRISTATE_FALSE);
    ALGEBRAIC_DECL(symmetric tensor,    SAF_TRISTATE_FALSE);
    ALGEBRAIC_DECL(tensor,              SAF_TRISTATE_FALSE);
    ALGEBRAIC_DECL(tuple,               SAF_TRISTATE_FALSE);
    ALGEBRAIC_DECL(field,               SAF_TRISTATE_TRUE);

    return SAF_SUCCESS;
}

/* Initialize evaluation types in the registry */
static int
initOr_evaluation(SAF_Db *database)
{
    EVAL_DECL(constant);
    EVAL_DECL(piecewise constant);
    EVAL_DECL(piecewise linear);
    EVAL_DECL(uniform);

    return SAF_SUCCESS;
}

/* Initialize relation representation types in the registry */
static int
initOR_relrep(SAF_Db *database)
{
    RELREP_DECL(structured,             SAF_STRUCTURED_ID);
    RELREP_DECL(unstructured,           SAF_UNSTRUCTURED_ID);
    RELREP_DECL(arbitrary,              SAF_ARBITRARY_ID);
    RELREP_DECL(hslab,                  SAF_HSLAB_ID);
    RELREP_DECL(tuples,                 SAF_TUPLES_ID);
    RELREP_DECL(totality,               SAF_TOTALITY_ID);

    return SAF_SUCCESS;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Pre-Defined Types
 * Purpose:	Define a bunch of pre-defined database objects for SAF
 *
 * Description: This routine was modified from the original, gen_std_types.c, which was a stand-alone executable that created
 *		a file on disk. This version is designed to build the contents of the would-be file, on the file, in
 *		memory, eliminating the need for a pre-exisiting file on disk as well as the I/O to open and read it.
 *
 *              SAF uses a bunch of pre-defined types for a bunch of different classes of objects. Examples, are units,
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
 *              We'd like to use the isRegistrar property here to accelerate registration of types. However, as currently
 *		designed, the isRegistrar property is a property of the library, not a single database. To work-around
 *		this, this function temporarily changes the global property by directly accessing the data member and
 *              then sets it back to whatever it was before this function was entered.
 *
 *		Since this code was original written as a /client/ of the library, it operates and returns public
 *		object handles instead of private ones. Likewise, this code was originally written assuming
 *              SAF_STRMODE_LIB for string allocation mode.
 *
 *
 * Programmer:  Mark C. Miller 
 *              Thursday, December 10th 2003 
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
_saf_gen_registry(SAF_Db *db)
{
    SAF_ENTER(_saf_gen_registry, -1);
    
    SAF_StrMode oldStrMode = _SAF_GLOBALS.p.StrMode;
    _SAF_GLOBALS.p.StrMode = SAF_STRMODE_LIB;

    /* build pre-defined quantities and units. */
    initOR_quantity(db);
    initOR_unit(db);
    initOR_role(db);
    initOr_basis(db);
    initOr_algebraic(db);
    initOr_evaluation(db);
    initOR_relrep(db);

    _SAF_GLOBALS.p.StrMode = oldStrMode;
    SAF_LEAVE(SAF_SUCCESS);
}
