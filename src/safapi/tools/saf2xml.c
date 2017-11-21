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
#include <assert.h>
#include <saf.h>
#define RPM_DO_NOT_COMPILE_THIS_FILE

#if defined(HAVE_GNOME_XML_TREE_H) /* libxml version 1 */
#include <gnome-xml/tree.h>
#elif defined (HAVE_LIBXML_TREE_H) /* libxml version 2 */
#include <libxml/tree.h>
#endif


#define MAX(X,Y)        ((X)>(Y)?(X):(Y))
#define MAX3(X,Y,Z)     MAX(X,MAX(Y,Z))

#if defined(RPM_DO_NOT_COMPILE_THIS_FILE) || !defined(HAVE_LIBXML) && !defined(HAVE_LIBXML2)
int main(int argc, char *argv[])
{
    if( argc > 0 )
      fprintf(stderr, "%s: not configured when SAF was built\n", argv[0]);
    else
      fprintf(stderr, "?: not configured when SAF was built\n");
   
    exit(1);
    return(-1);
}
#else

#ifdef HAVE_LIBXML2
#define P_xmlNewDocNode(DOC,NS,NAME,CONTENT) ((DOC)->children=xmlNewDocNode((DOC),(NS),(NAME),(CONTENT)))
#else
#define P_xmlNewDocNode(DOC,NS,NAME,CONTENT) ((DOC)->root=xmlNewDocNode((DOC),(NS),(NAME),(CONTENT)))
#endif

static int define_quantity(SAF_Handle q, xmlDocPtr doc, xmlNsPtr ns);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     saf2xml
 * Purpose:     Usage message
 *
 * Description: Prints usage message to stderr.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February 27, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static void
usage(void)
{
    fprintf(stderr, "usage: saf2xml [SWITCHES] SAF_DATABASE\n");
    /*Ruler:         ================================================================================*/
    fprintf(stderr, "    -o XML_FILE\n");
    fprintf(stderr, "    --output XML_FILE\n");
    fprintf(stderr, "        The name of the resulting XML file. By default the file will be named\n");
    fprintf(stderr, "        the same as the SAF database except any `.saf' extension is removed and\n");
    fprintf(stderr, "        a `.xml' extension is aded.\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     saf2xml
 *              
 * Description: The saf2xml utility takes the name of a SAF database and dumps all definitions found in that database. See
 *              main() for usage information.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/* Global variables */
static struct {
    int         nalloc;
    int         *defined;
} defined_g[_SAF_OBJTYPE_NTYPES];

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     saf2xml
 * Purpose:     Determine if object is already defined
 *
 * Description: Given a handle to any SAF object, return non-zero if the object definition has already been added to the XML
 *              document.
 *
 * Return:      Positive if H is already defined in the XML output, zero if not defined. Failures are indicated by negative
 *              return value.
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 22, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
is_defined(void *_h)
{
    SAF_BaseHandle      *h = (SAF_BaseHandle*)_h;
    
    if (!h || h->theType>=_SAF_OBJTYPE_NTYPES) return -1;
    if (h->theRow>=defined_g[h->theType].nalloc) return 0;
    return defined_g[h->theType].defined[h->theRow];
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     saf2xml
 * Purpose:     Mark object as defined
 *
 * Description: Marks an object as being defined in the XML output.
 *
 * Return:      Non-negative on success; negative on failure.
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 22, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
mark_defined(void *_h)
{
    SAF_BaseHandle      *h = (SAF_BaseHandle*)_h;

    if (!h || h->theType>=_SAF_OBJTYPE_NTYPES) return -1;
    if (h->theRow>=defined_g[h->theType].nalloc) {
        /* Allocate new entries and set them to zero */
        size_t nalloc = MAX3(256, 2*defined_g[h->theType].nalloc, h->theRow+1);
        defined_g[h->theType].defined = realloc(defined_g[h->theType].defined, nalloc*sizeof(int));
        memset(defined_g[h->theType].defined+defined_g[h->theType].nalloc, 0, nalloc-defined_g[h->theType].nalloc);
        defined_g[h->theType].nalloc = nalloc;
    }
    defined_g[h->theType].defined[h->theRow] = 1;
    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     saf2xml
 * Purpose:     Convert base quantity number to a name
 *
 * Description: Given one of the seven basic quantity numbers return a static string which is the name of that quantity.
 *
 * Return:      A static-allocated string.
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 22, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static const char *
i2q(SAF_BaseQuant i)
{
    switch (i) {
    case VBT_BASEQ_TIME:
        return "VBT_BASEQ_TIME";
    case VBT_BASEQ_MASS:
        return "VBT_BASEQ_MASS";
    case VBT_BASEQ_CURRENT:
        return "VBT_BASEQ_CURRENT";
    case VBT_BASEQ_LENGTH:
        return "VBT_BASEQ_LENGTH";
    case VBT_BASEQ_LIGHT:
        return "VBT_BASEQ_LIGHT";
    case VBT_BASEQ_TEMP:
        return "VBT_BASEQ_TEMP";
    case VBT_BASEQ_AMOUNT:
        return "VBT_BASEQ_AMOUNT";
    default:
        assert(VBT_MAX_BASEQS==7);
        return "VBT_BASEQ_UNKNOWN";
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     saf2xml
 * Purpose:     Convert real number to string
 *
 * Description: Given a real number return a statically allocated string.
 *
 * Return:      A statically allocated string.
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 22, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static const char *
d2s(double d)
{
    static char buf[64];
    sprintf(buf, "%g", d);
    return buf;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     saf2xml
 * Purpose:     Generate XML for a unit
 *
 * Description: Generate XML for a unit unless the definition has already been output.
 *
 * Return:      Returns non-negative when successful; negative otherwise.
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 22, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
define_unit(SAF_Handle u, xmlDocPtr doc, xmlNsPtr ns)
{
    char                *name, *abbr;           /*unit name and abbreviation*/
    double              scale, offset;          /*unit scale and translation data*/
    double              logbase, logcoef;       /*unit logarithm information*/
    SAF_Handle          q;                      /*quantity that this unit measures*/
    xmlNodePtr          x_u;                    /*the unit being defined*/
    xmlNodePtr          x_q;                    /*the XML quantity reference*/

    if (is_defined(&u)) return 0; /*successful last time*/
    if (saf_describe_unit(u, &name, &abbr, &scale, &offset, &logbase, &logcoef, &q)<0) return -1;

    /* Define prerequisites */
    define_quantity(q, doc, ns);
    
    /* Define this unit */
    if (NULL==(x_u=xmlNewChild(xmlDocGetRootElement(doc), ns, "unit", NULL))) return -1;
    if (NULL==xmlSetProp(x_u, "name", name)) return -1;

    if (abbr && *abbr && NULL==xmlNewChild(x_u, ns, "abbr",    abbr))         return -1;
    if (scale!=1.0    && NULL==xmlNewChild(x_u, ns, "scale",   d2s(scale)))   return -1;
    if (offset        && NULL==xmlNewChild(x_u, ns, "offset",  d2s(offset)))  return -1;
    if (logbase       && NULL==xmlNewChild(x_u, ns, "logbase", d2s(logbase))) return -1;
    if (logcoef       && NULL==xmlNewChild(x_u, ns, "logcoef", d2s(logcoef))) return -1;
    if (name) free(name);
    if (abbr) free(abbr);

    /* Reference the quantity by name */
    if (saf_describe_quantity(q, &name, NULL, NULL, NULL)<0) return -1;
    if (NULL==(x_q=xmlNewChild(x_u, ns, "quantity", NULL))) return -1;
    if (NULL==xmlSetProp(x_q, "name", name)) return -1;
    if (name) free(name);

    /* Success -- mark unit as defined */
    mark_defined(&u);
    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     saf2xml
 * Purpose:     Generate XML for all units
 *
 * Description: Given a SAF database, add all units to the XML document.
 *
 * Return:      Non-negative for success; negative for failure.
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 22, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
define_units(SAF_Handle db, xmlDocPtr doc, xmlNsPtr ns)
{
    SAF_Unit            *u=NULL;                /*array of all units*/
    int                 nu;                     /*number of units*/
    int                 i;
    
    if (saf_find_units(db, NULL, NULL, DSL_WILD_DOUBLE, DSL_WILD_DOUBLE, DSL_WILD_DOUBLE, DSL_WILD_DOUBLE, SAF_NOT_SET_QUANTITY,
                       &nu, &u)<0) return -1;
    for (i=0; i<nu; i++) {
        if (define_unit(u[i], doc, ns)<0) return -1;
    }
    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     saf2xml
 * Purpose:     Generate XML for a quantity
 *
 * Description: Generate XML for a quantity. For example, volumetric flow would be:
 *
 *                  <quantity name="volumetric flow">
 *                    <product>
 *                      <power n="3">
 *                        <quantity name="VBT_BASEQ_LENGTH"/>
 *                      </power>
 *                      <power n="-1">
 *                        <quantity name="VBT_BASEQ_TIME"/>
 *                      </power>
 *                    </product>
 *                  </quantity>
 *
 * Return:      Returns non-negative when successful; negative otherwise.
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 22, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
define_quantity(SAF_Quantity q, xmlDocPtr doc, xmlNsPtr ns)
{
    char                *name, *abbr;           /*quantity name and abbreviation*/
    unsigned            num[VBT_MAX_BASEQS];    /*numerator base quantities*/
    unsigned            den[VBT_MAX_BASEQS];    /*denominator base quantities*/
    xmlNodePtr          x_q, x_bq;              /*XML quantity and base quantity nodes*/
    xmlNodePtr          x_product, x_power;     /*XML product and power nodes*/
    SAF_BaseQuant       bq;                     /*base quantity counter*/
    char                buf[16];

    if (is_defined(&q)) return 0; /*successful last time*/
    if (saf_describe_quantity(q, &name, &abbr, num, den)<0) return -1;
    if (NULL==(x_q=xmlNewChild(xmlDocGetRootElement(doc), ns, "quantity", NULL))) return -1;
    if (NULL==xmlSetProp(x_q, "name", name)) return -1;
    if (abbr && NULL==xmlNewChild(x_q, ns, "abbr", abbr)) return -1;
    if (NULL==(x_product=xmlNewChild(x_q, ns, "product", NULL))) return -1;
    
    for (bq=0; bq<VBT_MAX_BASEQS; bq++) {
        if (num[bq]) {
            sprintf(buf, "%u", num[bq]);
            if (NULL==(x_power=xmlNewChild(x_product, ns, "power", NULL))) return -1;
            if (NULL==xmlSetProp(x_power, "n", buf)) return -1;
            if (NULL==(x_bq=xmlNewChild(x_power, ns, "quantity", NULL))) return -1;
            if (NULL==xmlSetProp(x_bq, "name", i2q(bq))) return -1;
        }
        if (den[bq]) {
            sprintf(buf, "-%u", den[bq]);
            if (NULL==(x_power=xmlNewChild(x_product, ns, "power", NULL))) return -1;
            if (NULL==xmlSetProp(x_power, "n", buf)) return -1;
            if (NULL==(x_bq=xmlNewChild(x_power, ns, "quantity", NULL))) return -1;
            if (NULL==(xmlSetProp(x_bq, "name", i2q(bq)))) return -1;
        }
    }
    if (name) free(name);
    if (abbr) free(abbr);
    mark_defined(&q);
    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     saf2xml
 * Purpose:     Generate XML for all quantities
 *
 * Description: Given a SAF database, add all quantities to the XML document.
 *
 * Return:      Non-negative for success; negative for failure.
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 22, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
define_quantities(SAF_Handle db, xmlDocPtr doc, xmlNsPtr ns)
{
    SAF_Quantity        *q=NULL;                /*array of all quantities*/
    int                 nq;                     /*number of quantities*/
    int                 i;
    
    if (saf_find_quantities(db, NULL, NULL, NULL, NULL, &nq, &q)<0) return -1;
    for (i=0; i<nq; i++) {
        if (define_quantity(q[i], doc, ns)<0) return -1;
    }
    return 0;
}
    
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     saf2xml
 * Purpose:     Convert SAF files to XML
 *
 * Description: This program should be called with one command-line argument: the name of the SAF database which is to be
 *              dumped in XML format.
 *
 * Return:      Exits with zero on success, non-zero otherwise.
 *
 * Issues:      This application can only generate XML according to what is stored in a SAF database. For instance, if inches
 *              are defined in terms of feet in the C code that generates the SAF database, the XML output will define inches
 *              in terms of the base unit for the quantity `length'.
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 22, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
    const char          *output_name=NULL;
    const char          *db_name=NULL;
    SAF_Handle          dbprops;
    SAF_Handle          db;
    int                 optarg;
    xmlDocPtr           doc;
    xmlNodePtr          root;
    xmlNsPtr            ns;
    
    
    saf_init(SAF_DEFAULT_LIBPROPS);
    
    /* Parse arguments */
    for (optarg=1; optarg<argc && argv[optarg][0]=='-'; optarg++) {
        if (!strcmp(argv[optarg], "-o") && optarg+1<argc) {
            output_name = argv[++optarg];
        } else if (!strncmp(argv[optarg], "-o", 2)) {
            output_name = argv[optarg]+2;
        } else if (!strcmp(argv[optarg], "--output") && optarg+1<argc) {
            output_name = argv[++optarg];
        } else if (!strncmp(argv[optarg], "--output=", 9) && argv[optarg][9]) {
            output_name = argv[optarg]+9;
        } else if (!strcmp(argv[optarg], "--")) {
            optarg++;
            break;
        } else {
            fprintf(stderr, "%s: unrecognized argument\n", argv[optarg]);
            exit(1);
        }
    }
    if (optarg+1!=argc) {
        usage();
        exit(1);
    }
    db_name = argv[optarg];

    /* Default output name is the same as the base name of the input file with any trailing `.saf' extension removed and
     * an `.xml' extension added. */
    if (!output_name) {
        const char *base;
        char *ext, *tmp;
        base = strrchr(db_name, '/');
        base = base ? base+1 : db_name;
        tmp = malloc(strlen(base)+5);
        strcpy(tmp, base);
        if ((ext=strrchr(tmp, '.')) && !strcmp(ext, ".saf")) *ext = '\0';
        strcat(tmp, ".xml");
        output_name = tmp;
    }
    
    /* Create the XML document in memory */
    doc = xmlNewDoc("1.0");
    root = P_xmlNewDocNode(doc, NULL, "database", NULL);
    ns = xmlNewNs(root, "http://saf.llnl.gov/1.0.0/ns", "saf");

    /* Attempt to open the SAF database */
    dbprops = saf_createProps_database();
    saf_setProps_ReadOnly(dbprops);
    if ((db=saf_open_database(db_name, dbprops))<0) {
        fprintf(stderr, "%s: cannot open saf database\n", db_name);
        exit(1);
    }
    dbprops = saf_release(dbprops);

    /* Generate the XML document */
    define_units(db, doc, ns);

    /* Close the database, write the XML document, and exit */
    saf_close_database(db);
    if (xmlSaveFile(output_name, doc)<0) {
        fprintf(stderr, "%s: cannot save XML document\n", output_name);
        exit(1);
    }
    xmlFreeDoc(doc);
    return 0;
}

#endif /* HAVE_LIBXML */
