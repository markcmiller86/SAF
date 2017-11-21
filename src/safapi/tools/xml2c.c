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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#   include "SAFconfig-WIN32.h"
#elif defined(JANUS)
#   include "SAFconfig-JANUS.h"
#else
#   include "SAFconfig.h"
#endif

#if defined(HAVE_GNOME_XML_TREE_H) /* libxml version 1 */
#include <gnome-xml/tree.h>
#elif defined (HAVE_LIBXML_TREE_H) /* libxml version 2 */
#include <libxml/tree.h>
#endif


#if !defined(HAVE_LIBXML) && !defined(HAVE_LIBXML2)
int main(int argc, char *argv[])
{
    fprintf(stderr, "%s: not configured when SAF was built\n", argv[0]);
    exit(1);
    return(-1);
}
#else

/* Accessors */
#define child_of(X)             ((X)->childs)
#define sibling_of(X)           ((X)->next)
#define namespace_of(X)         ((X)->ns)
#define name_of(X)              ((X)->name)
#define content_of(X)           ((X)->content)

/* Node predicates */
#define is_named(X,NAME,NS)     (namespace_of(X)==(NS) && !strcmp(name_of(X),NAME))
#define is_text(X)              ((X) && XML_TEXT_NODE==(X)->type)

/* Portability macros */
#define OR(X,Y)                 ((X)?(X):(Y))
#define MAX(X,Y)                ((X)>(Y)?(X):(Y))
#define MIN(X,Y)                ((X)<(Y)?(X):(Y))

/* Constants */
#define INDENT_AMOUNT           4
#define INDENT(L)               (L)*INDENT_AMOUNT,""            /*printf args for format "%*s" for indentation*/
#define PPINDENT(L)             (-(L)*INDENT_AMOUNT),"#"        /*indentation for format "#%*s" for cpp directive*/

/* Common arguments for parse functions */
typedef struct parse_t {
    FILE        *output;                /*stream for C source file*/
    xmlDocPtr   *doc;                   /*the entire XML document*/
    xmlNsPtr    *ns;                    /*the SAF name space*/
    int         instrument;             /*how verbose is the C source when run?*/
} parse_t;

/* Forward declarations for parse functions */
/*                       |-------- common arguments ----------|  |----- extra stuff ----...                       */
static int gen_q_power  (parse_t *p, xmlNodePtr node, int level, const char *qname);
static int gen_q_product(parse_t *p, xmlNodePtr node, int level, const char *qname);
static int gen_quantity (parse_t *p, xmlNodePtr node, int level, const char *qname);
static int gen_unit     (parse_t *p, xmlNodePtr node, int level, const char *uname);
static int gen_database (parse_t *p, xmlNodePtr node, int level, const char *func_name, const char *saf_name);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     xml2c
 * Purpose:     Usage message
 *
 * Description: Prints usage message to stderr.
 *
 * Programmer:  Robb Matzke
 *              Monday, March 5, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static void
usage(void)
{
    fprintf(stderr, "usage: xml2c [SWITCHES] XML\n");
    /*Ruler:         ================================================================================*/
    fprintf(stderr, "    -o C_FILE\n");
    fprintf(stderr, "    --output C_FILE\n");
    fprintf(stderr, "        The name of the resulting C file. By default the file will be named\n");
    fprintf(stderr, "        the same as the XML file except any `.xml' extension is removed and\n");
    fprintf(stderr, "        a `.c' extension is aded.\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     xml2c
 * Purpose:     Generate C code that defines a quantity
 *
 * Description: Given an XML `power' element within a `product' element within a `quantity' element, multiply the contained
 *              quantities into the quantity being defined.
 *
 * Return:      Non-negative on success; negative on failure.
 *
 * Programmer:  Robb Matzke
 *              Monday, March  5, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
gen_q_power(parse_t *p,                 /*common parsing information*/
            xmlNodePtr node,            /*the `power' element*/
            int level,                  /*indentation level*/
            const char *qname           /*C name for the handle of the quantity being defined*/
            )
{
    char *power = xmlGetProp(node, "n");

    for (node=child_of(node); node; node=sibling_of(node)) {
        if (is_named(node, "quantity", p->ns)) {
            gen_quantity(p, node, level, "q_opand");
            fprintf(p->output, "%*ssaf_multiply_quantity(%s, q_opand, %s);\n", INDENT(level), qname, OR(power, "1"));
        }
    }
    if (power) free(power);
    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     xml2c
 * Purpose:     Generate C code that defines a quantity
 *
 * Description: Given an XML `product' element within a `quantity' element, multiply the contained quantities
 *              into the quantity being defined.
 *
 * Return:      Non-negative on success; negative on failure.
 *
 * Programmer:  Robb Matzke
 *              Monday, March  5, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
gen_q_product(parse_t *p,               /*common parsing information*/
              xmlNodePtr node,          /*the `product' element*/
              int level,                /*indentation level*/
              const char *qname         /*C name for the handle of the quantity being defined*/
              )
{
    for (node=child_of(node); node; node=sibling_of(node)) {
        if (is_named(node, "power", p->ns)) {
            gen_q_power(p, node, level, qname);
        } else {
            assert(0 && "DTD problem");
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     xml2c
 * Purpose:     Generate C code that defines a quantity
 *
 * Description: Given an XML `quantity' element which is part of a `database' element, either define the quantity (if the node
 *              has children) or obtain a handle of an existing quantity.
 *
 * Return:      Non-negative on success; negative on failure.
 *
 * Programmer:  Robb Matzke
 *              Monday, March  5, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
gen_quantity(parse_t *p,                /*common parsing information*/
             xmlNodePtr node,           /*the `quantity' element*/
             int level,                 /*indentation level*/
             const char *qname          /*optional name of the C quantity handle*/
             )
{
    char *name = xmlGetProp(node, "name");
    const char *abbr = NULL;
    
    assert(name); /*DTD problem*/

    if (child_of(node)) {
        /* quantity definition */

        for (node=child_of(node); node; node=sibling_of(node)) {
            if (is_named(node, "abbr", p->ns)) {
                assert(child_of(node) && is_text(child_of(node))); /*DTD problem*/
                assert(!abbr); /*DTD problem*/
                abbr = content_of(child_of(node));
            } else if (is_named(node, "product", p->ns)) {
                fprintf(p->output, "%*s{\n", INDENT(level));
                fprintf(p->output, "%*s/* Quantity: %s */\n", INDENT(level+1), name);
                if (!qname) fprintf(p->output, "%*sSAF_Quantity %s=SAF_NOT_SET_QUANTITY;\n", INDENT(level+1), (qname="q"));
                fprintf(p->output, "%*sSAF_Quantity q_opand=SAF_NOT_SET_QUANTITY;\n", INDENT(level+1));
                if (p->instrument) fprintf(p->output, "%*sfprintf(stderr, \"quantity \\\"%s\\\"\\n\");\n", INDENT(level+1), name);
                if (!abbr) abbr = "";
                fprintf(p->output, "%*ssaf_declare_quantity(db, \"%s\", \"%s\", &%s);\n", INDENT(level+1), name, abbr, qname);
                gen_q_product(p, node, level+1, qname);
                fprintf(p->output, "%*ssaf_commit_quantity(&%s);\n", INDENT(level+1), qname);
                fprintf(p->output, "%*s}\n", INDENT(level));
            } else {
                assert(0 && "DTD problem");
            }
        }
    
    } else if (!strcmp(name, "VBT_BASEQ_TIME")    ||
               !strcmp(name, "VBT_BASEQ_MASS")    ||
               !strcmp(name, "VBT_BASEQ_CURRENT") ||
               !strcmp(name, "VBT_BASEQ_LENGTH")  ||
               !strcmp(name, "VBT_BASEQ_LIGHT")   ||
               !strcmp(name, "VBT_BASEQ_TEMP")    ||
               !strcmp(name, "VBT_BASEQ_AMOUNT")) {
        /* one of the seven basic quantities */
        const char *base=strrchr(name, '_')+1;
        fprintf(p->output, "%*s%s%sSAF_Q%s(db);\n", INDENT(level), qname, qname?"=":"", base);
    } else {
        /* reference to an existing quantity */
        fprintf(p->output, "%*s%s%ssaf_find_quantity(db, \"%s\");\n", INDENT(level), qname, qname?"=":"", name);
    }

    free(name);
    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     xml2c
 * Purpose:     Generate C code that defines a unit
 *
 * Description: Given an XML `unit' element which is part of a `database' element, either define the unit (if the node has
 *              children) or obtain a handle of an existing unit.
 *
 * Return:      Non-negative on success; negative on failure.
 *
 * Programmer:  Robb Matzke
 *              Monday, March  5, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
gen_unit(parse_t *p,                    /*common parsing information*/
         xmlNodePtr node,               /*the `unit' element*/
         int level,                     /*indentation level*/
         const char *uname              /*optional name of the C unit handle*/
         )
{
    const char *abbr=NULL, *scale=NULL, *offset=NULL, *lbase=NULL, *lcoef=NULL;
    char *name = xmlGetProp(node, "name");
    assert(name); /*DTD problem*/

    if (child_of(node)) {
        /* unit definition */
        for (node=child_of(node); node; node=sibling_of(node)) {
            if (is_named(node, "abbr", p->ns)) {
                assert(child_of(node) && is_text(child_of(node))); /*DTD problem*/
                assert(!abbr); /*DTD problem*/
                abbr = content_of(child_of(node));
            } else if (is_named(node, "scale", p->ns)) {
                assert(child_of(node) && is_text(child_of(node))); /*DTD problem*/
                assert(!scale); /*DTD problem*/
                scale = content_of(child_of(node));
            } else if (is_named(node, "offset", p->ns)) {
                assert(child_of(node) && is_text(child_of(node))); /*DTD problem*/
                assert(!offset); /*DTD problem*/
                offset = content_of(child_of(node));
            } else if (is_named(node, "logbase", p->ns)) {
                assert(child_of(node) && is_text(child_of(node))); /*DTD problem*/
                assert(!lbase); /*DTD problem*/
                lbase = content_of(child_of(node));
            } else if (is_named(node, "logcoef", p->ns)) {
                assert(child_of(node) && is_text(child_of(node))); /*DTD problem*/
                assert(!lcoef); /*DTD problem*/
                lcoef = content_of(child_of(node));
            } else if (is_named(node, "quantity", p->ns)) {
                fprintf(p->output, "%*s{\n", INDENT(level));
                fprintf(p->output, "%*s/* Unit: %s */\n", INDENT(level+1), name);
                if (!uname) fprintf(p->output, "%*sSAF_Unit %s=SAF_NOT_SET_UNIT;\n", INDENT(level+1), (uname="u"));
                fprintf(p->output, "%*sSAF_Quantity uq;\n", INDENT(level+1));
                if (p->instrument) fprintf(p->output, "%*sfprintf(stderr, \"unit \\\"%s\\\"\\n\");\n", INDENT(level+1), name);
                fprintf(p->output, "%*ssaf_declare_unit(db, \"%s\", \"%s\", &%s);\n", INDENT(level+1), name, abbr, uname);
                gen_quantity(p, node, level+1, "uq");
                fprintf(p->output, "%*ssaf_quantify_unit(%s, uq, %s);\n", INDENT(level+1), uname, OR(scale, "1.0"));
                if (offset) fprintf(p->output, "%*ssaf_ofset_unit(%s, %s);\n", INDENT(level+1), uname, offset);
                if (lbase) fprintf(p->output, "%*ssaf_log_unit(%s, %s, %s);\n", INDENT(level+1), uname, lbase, OR(lcoef, "1.0"));
                fprintf(p->output, "%*ssaf_commit_unit(&%s);\n", INDENT(level+1), uname);
                fprintf(p->output, "%*s}\n", INDENT(level));
            } else {
                assert(0 && "DTD problem");
                continue;
            }
        }
    } else {
        /* reference to existing unit */
        fprintf(stderr, "unit references not implemented\n");
        abort();
    }

    free(name);
    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     xml2c
 * Purpose:     Generate C code that defines a database
 *
 * Description: Given the root of an XML `database' element, produce C code that defines the database.
 *
 * Return:      Non-negative on success; arbitrary negative value on failure
 *
 * Programmer:  Robb Matzke
 *              Monday, March  5, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
gen_database(parse_t *p,                /*common parsing information*/
             xmlNodePtr node,           /*the `database' element*/
             int level,                 /*indentation level*/
             const char *func_name,     /*name of generated C function*/
             const char *saf_name       /*name of eventual SAF database*/
             )
{
    
    fprintf(p->output, "/* This source code is machine generated! */\n");
    fprintf(p->output, "%*sinclude <assert.h>\n", PPINDENT(level));
    fprintf(p->output, "%*sinclude <saf.h>\n", PPINDENT(level));
    fprintf(p->output, "\n");

    fprintf(p->output, "%*sint\n", INDENT(level));
    fprintf(p->output, "%*s%s(void)\n", INDENT(level), func_name);
    fprintf(p->output, "%*s{\n", INDENT(level));
    fprintf(p->output, "%*sSAF_Db db;\n", INDENT(level+1));
    fprintf(p->output, "%*sSAF_DbProps dbprops;\n", INDENT(level+1));
    fprintf(p->output, "%*sint status;\n", INDENT(level+1));
    fprintf(p->output, "\n");

    fprintf(p->output, "%*ssaf_init(SAF_DEFAULT_LIBPROPS);\n", INDENT(level+1));
    fprintf(p->output, "%*ssaf_createProps_database(&dbprops);\n", INDENT(level+1));
    fprintf(p->output, "%*ssaf_setProps_ImportFile(&dbprops,\"don't import\");\n", INDENT(level+1));
    fprintf(p->output, "%*sstatus = saf_open_database(\"%s\", dbprops, &db);\n", INDENT(level+1), saf_name);
    fprintf(p->output, "%*sassert(status>=0);\n", INDENT(level+1));
    fprintf(p->output, "\n");
    
    for (node=child_of(node); node; node=sibling_of(node)) {
        if (is_named(node, "quantity", p->ns)) {
            if (gen_quantity(p, node, level+1, NULL)<0) return -1;
        } else if (is_named(node, "unit", p->ns)) {
            if (gen_unit(p, node, level+1, NULL)<0) return -1;
        } else {
            fprintf(stderr, "%s:%u: node name \"%s\"\n", __FILE__, __LINE__, name_of(node));
        }
    }

    fprintf(p->output, "%*ssaf_close_database(db);\n", INDENT(level+1));
    fprintf(p->output, "%*s}\n\n", INDENT(level));
    return 0;
}
    
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     xml2c
 * Purpose:     Convert XML files to C source code.
 *
 * Description: This program should be called with one command-line argument: the name of the XML file which is to be
 *              converted to C source code.
 *
 * Return:      Exits with zero on success, non-zero otherwise.
 *
 * Programmer:  Robb Matzke
 *              Monday, March 5, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
    const char          *output_name=NULL;
    const char          *input_name=NULL;
    const char          *saf_name=NULL;
    int                 optarg;
    xmlNodePtr          root=NULL;
    parse_t             p;

    /* Default parse values */
    memset(&p, 0, sizeof p);
    p.instrument = 1;
    
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
    input_name = argv[optarg];

    /* Default output name is the same as the base name of the input file with any trailing `.xml' extension removed and
     * a `.c' extension added. */
    if (!output_name) {
        char *ext, *tmp;
        const char *base;
        base = strrchr(input_name, '/');
        base = base ? base+1 : input_name;
        tmp = malloc(strlen(base)+3);
        strcpy(tmp, base);
        if ((ext=strrchr(tmp, '.')) && !strcmp(ext, ".xml")) *ext = '\0';
        strcat(tmp, ".c");
        output_name = tmp;
    }

    /* If the output C source is compiled and run it will create a SAF database with the same name as the XML input file
     * except with a `.saf' extension (after any `.xml' extension is removed). The SAF database will be created in the
     * then-current working directory. */
    {
        char *ext, *tmp;
        const char *base;
        base = strrchr(input_name, '/');
        base = base ? base+1 : input_name;
        tmp = malloc(strlen(base)+5);
        strcpy(tmp, base);
        if ((ext=strrchr(tmp, '.')) && !strcmp(ext, ".xml")) *ext = '\0';
        strcat(tmp, ".saf");
        saf_name = tmp;
    }
    
    /* Open the output file */
    if (NULL==(p.output=fopen(output_name, "w"))) {
        fprintf(stderr, "%s: cannot open file\n", output_name);
        exit(1);
    }

    /* Read in the XML document and check version */
    if (NULL==(p.doc=xmlParseFile(input_name))) {
        fprintf(stderr, "%s: cannot parse XML\n", input_name);
        exit(1);
    }
    if (NULL==(root=xmlDocGetRootElement(p.doc))) {
        fprintf(stderr, "%s: empty document\n", input_name);
        exit(1);
    }
    if (NULL==(p.ns=xmlSearchNsByHref(p.doc, root, "http://saf.llnl.gov/1.0.0/ns"))) {
        fprintf(stderr, "%s: document of the wrong type, saf namespace not found\n", input_name);
        exit(1); 
    }
    if (strcmp(name_of(root), "database")) {
        fprintf(stderr, "%s: not a SAF document\n", input_name);
        exit(1);
    }

    /* Traverse the XML parse tree, generating C code as output */
    if (gen_database(&p, root, 0, "main", saf_name)<0) exit(1);
    xmlFreeDoc(p.doc);
    return 0;
}

#endif /* HAVE_LIBXML */
