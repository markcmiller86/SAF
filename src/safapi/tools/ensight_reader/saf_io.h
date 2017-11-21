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

//## begin module%1.3%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.3%.codegen_version

//## begin module%3CC417390148.cm preserve=yes
//	 $Id: saf_io.h,v 1.5 2005/05/10 18:30:46 gdsjaar Exp $
//## end module%3CC417390148.cm

//## begin module%3CC417390148.cp preserve=no
//## end module%3CC417390148.cp

//## Module: saf_io%3CC417390148; Package specification
//## Subsystem: framework%3C55990A032C
//## Source file: \\Cerberus\Projects\Emphasis\Nevada\framework\saf_io.h

#ifndef saf_ioH
#define saf_ioH 1

//## begin module%3CC417390148.additionalIncludes preserve=no
//## end module%3CC417390148.additionalIncludes

//## begin module%3CC417390148.includes preserve=yes
#ifdef USE_SAF_IO
# include "saf.h"
#endif
#include "String.h"
#include "region.h"
#include "variable_attributes.h"
//## end module%3CC417390148.includes

// region_io
#include "region_io.h"
//## begin module%3CC417390148.declarations preserve=no
//## end module%3CC417390148.declarations

//## begin module%3CC417390148.additionalDeclarations preserve=yes
#define SAF_Real SAF_DOUBLE
#ifndef USE_SAF_IO
typedef int SAF_Db;
typedef int SAF_File;
typedef int SAF_Cat;
typedef int SAF_Field;
typedef int SAF_FieldTmpl;
typedef int SAF_StateGrp;
typedef int SAF_Set;
typedef int SAF_Suite;

extern SAF_Set SAF_NOT_SET_SET;
extern SAF_FieldTmpl SAF_NOT_SET_FIELDTMPL;
extern SAF_Field SAF_NOT_SET_FIELD;
extern SAF_Cat SAF_NOT_SET_CAT;

# define SAF_EQUIV(A,B)  (A) == (B)
#endif
//## end module%3CC417390148.additionalDeclarations


//## begin SAF_IO%3CC41527036B.preface preserve=yes
//## end SAF_IO%3CC41527036B.preface

//## Class: SAF_IO%3CC41527036B
//	Region I/O class encapsulating the ASCI Sets and Fields
//	format.
//## Category: framework%3C55965E009C
//## Subsystem: framework%3C55990A032C
//## Persistence: Transient
//## Cardinality/Multiplicity: n

class SAF_IO : public Region_IO  //## Inherits: <unnamed>%3CC417EB006D
{
  //## begin SAF_IO%3CC41527036B.initialDeclarations preserve=yes
  //## end SAF_IO%3CC41527036B.initialDeclarations

    //## begin SAF_IO::SAF_Request%3CF6ACD3022F.preface preserve=yes
    //## end SAF_IO::SAF_Request%3CF6ACD3022F.preface

    //## Class: SAF_Request%3CF6ACD3022F; private
    //	An object containing all the information to process a
    //	SAF field for a single block. Implemented as an entirely
    //	public class.
    //## Category: framework%3C55965E009C
    //## Subsystem: framework%3C55990A032C
    //## Persistence: Transient
    //## Cardinality/Multiplicity: n

    class SAF_Request 
    {
      //## begin SAF_IO::SAF_Request%3CF6ACD3022F.initialDeclarations preserve=yes
      //## end SAF_IO::SAF_Request%3CF6ACD3022F.initialDeclarations

      public:
        //## Constructors (specified)
          //## Operation: SAF_Request%3CF6B4580154
          //	Constructor.
          SAF_Request (String n, 	// The name associated with this variable.
          SAF_Set on_s = SAF_NOT_SET_SET, 	// The SAF set that this field is defined on.
          SAF_FieldTmpl t = SAF_NOT_SET_FIELDTMPL, 	// The SAF field template associated with this field.
          SAF_Cat c = SAF_NOT_SET_CAT, 	// The SAF category associated with this field.
          Real *d = 0	// A pointer to the data associated with this field.
          )
            //## begin SAF_IO::SAF_Request::SAF_Request%3CF6B4580154.hasinit preserve=no
                : tmpl(t),
                  category(c),
                  name(n),
                  first(0),
                  last(-1),
                  step(1),
                  data(d),
                  on_set(on_s)
            //## end SAF_IO::SAF_Request::SAF_Request%3CF6B4580154.hasinit
            //## begin SAF_IO::SAF_Request::SAF_Request%3CF6B4580154.initialization preserve=yes
            //## end SAF_IO::SAF_Request::SAF_Request%3CF6B4580154.initialization
          {
            //## begin SAF_IO::SAF_Request::SAF_Request%3CF6B4580154.body preserve=yes
            //## end SAF_IO::SAF_Request::SAF_Request%3CF6B4580154.body
          }


        //## Other Operations (specified)
          //## Operation: Is_Output_Step%3CF6B4F700B8
          bool Is_Output_Step (int timestep) const;

        // Data Members for Class Attributes

          //## Attribute: tmpl%3CF6AF840387
          //	The SAF field template associated with this field.
          //## begin SAF_IO::SAF_Request::tmpl%3CF6AF840387.attr preserve=no  public: SAF_FieldTmpl {UA} t
          SAF_FieldTmpl tmpl;
          //## end SAF_IO::SAF_Request::tmpl%3CF6AF840387.attr

          //## Attribute: category%3CF6AFE10164
          //	The SAF category associated with this field.
          //## begin SAF_IO::SAF_Request::category%3CF6AFE10164.attr preserve=no  public: SAF_Cat {UA} c
          SAF_Cat category;
          //## end SAF_IO::SAF_Request::category%3CF6AFE10164.attr

          //## Attribute: name%3CF6B05D0387
          //	The name associated with this variable.
          //## begin SAF_IO::SAF_Request::name%3CF6B05D0387.attr preserve=no  public: String {UA} n
          String name;
          //## end SAF_IO::SAF_Request::name%3CF6B05D0387.attr

          //## Attribute: first%3CF6B072004B
          //	index of 1st timestep for output
          //## begin SAF_IO::SAF_Request::first%3CF6B072004B.attr preserve=no  public: int {UA} 0
          int first;
          //## end SAF_IO::SAF_Request::first%3CF6B072004B.attr

          //## Attribute: last%3CF6B0A4030A
          //	index of last timestep for output
          //## begin SAF_IO::SAF_Request::last%3CF6B0A4030A.attr preserve=no  public: int {UA} -1
          int last;
          //## end SAF_IO::SAF_Request::last%3CF6B0A4030A.attr

          //## Attribute: step%3CF6B0B2003B
          //	number of timesteps between writing output
          //## begin SAF_IO::SAF_Request::step%3CF6B0B2003B.attr preserve=no  public: int {UA} 1
          int step;
          //## end SAF_IO::SAF_Request::step%3CF6B0B2003B.attr

          //## Attribute: data%3CF6B1260145
          //	A pointer to the data associated with this field.
          //## begin SAF_IO::SAF_Request::data%3CF6B1260145.attr preserve=no  public: Real * {UA} d
          Real *data;
          //## end SAF_IO::SAF_Request::data%3CF6B1260145.attr

          //## Attribute: on_set%3E1B483E02BF
          //	The SAF set that this field is defined on.
          //## begin SAF_IO::SAF_Request::on_set%3E1B483E02BF.attr preserve=no  public: SAF_Set {UA} on_s
          SAF_Set on_set;
          //## end SAF_IO::SAF_Request::on_set%3E1B483E02BF.attr

        // Additional Public Declarations
          //## begin SAF_IO::SAF_Request%3CF6ACD3022F.public preserve=yes
          //## end SAF_IO::SAF_Request%3CF6ACD3022F.public

      protected:
        // Additional Protected Declarations
          //## begin SAF_IO::SAF_Request%3CF6ACD3022F.protected preserve=yes
          //## end SAF_IO::SAF_Request%3CF6ACD3022F.protected

      private:
        // Additional Private Declarations
          //## begin SAF_IO::SAF_Request%3CF6ACD3022F.private preserve=yes
          //## end SAF_IO::SAF_Request%3CF6ACD3022F.private

      private: //## implementation
        // Additional Implementation Declarations
          //## begin SAF_IO::SAF_Request%3CF6ACD3022F.implementation preserve=yes
          //## end SAF_IO::SAF_Request%3CF6ACD3022F.implementation

    };

    //## begin SAF_IO::SAF_Request%3CF6ACD3022F.postscript preserve=yes
    //## end SAF_IO::SAF_Request%3CF6ACD3022F.postscript

    //## begin SAF_IO::SAF_Str_Block%3CF6B7C7026D.preface preserve=yes
    //## end SAF_IO::SAF_Str_Block%3CF6B7C7026D.preface

    //## Class: SAF_Str_Block%3CF6B7C7026D; private
    //## Category: framework%3C55965E009C
    //## Subsystem: framework%3C55990A032C
    //## Persistence: Transient
    //## Cardinality/Multiplicity: n

    class SAF_Str_Block 
    {
      //## begin SAF_IO::SAF_Str_Block%3CF6B7C7026D.initialDeclarations preserve=yes
      //## end SAF_IO::SAF_Str_Block%3CF6B7C7026D.initialDeclarations

      public:
        //## Constructors (specified)
          //## Operation: SAF_Str_Block%3CF6B92F004B
          SAF_Str_Block (SAF_Set set = SAF_NOT_SET_SET	// SAF set associated with this block.
          );

        // Data Members for Class Attributes

          //## Attribute: blockSet%3CF6B82C0339
          //	SAF set associated with this block.
          //## begin SAF_IO::SAF_Str_Block::blockSet%3CF6B82C0339.attr preserve=no  public: SAF_Set {UA} set
          SAF_Set blockSet;
          //## end SAF_IO::SAF_Str_Block::blockSet%3CF6B82C0339.attr

          //## Attribute: edge_subset%3CF6B8C60367
          //	SAF subsets for the 3 edge directions.
          //## begin SAF_IO::SAF_Str_Block::edge_subset%3CF6B8C60367.attr preserve=no  public: SAF_Set[3] {UA} 
          SAF_Set edge_subset[3];
          //## end SAF_IO::SAF_Str_Block::edge_subset%3CF6B8C60367.attr

          //## Attribute: face_subset%3CF6BA6E0387
          //	SAF subsets for the 3 face directions.
          //## begin SAF_IO::SAF_Str_Block::face_subset%3CF6BA6E0387.attr preserve=no  public: SAF_Set[3] {UA} 
          SAF_Set face_subset[3];
          //## end SAF_IO::SAF_Str_Block::face_subset%3CF6BA6E0387.attr

        // Additional Public Declarations
          //## begin SAF_IO::SAF_Str_Block%3CF6B7C7026D.public preserve=yes
          //## end SAF_IO::SAF_Str_Block%3CF6B7C7026D.public

      protected:
        // Additional Protected Declarations
          //## begin SAF_IO::SAF_Str_Block%3CF6B7C7026D.protected preserve=yes
          //## end SAF_IO::SAF_Str_Block%3CF6B7C7026D.protected

      private:
        // Additional Private Declarations
          //## begin SAF_IO::SAF_Str_Block%3CF6B7C7026D.private preserve=yes
          //## end SAF_IO::SAF_Str_Block%3CF6B7C7026D.private

      private: //## implementation
        // Additional Implementation Declarations
          //## begin SAF_IO::SAF_Str_Block%3CF6B7C7026D.implementation preserve=yes
          //## end SAF_IO::SAF_Str_Block%3CF6B7C7026D.implementation

    };

    //## begin SAF_IO::SAF_Str_Block%3CF6B7C7026D.postscript preserve=yes
    //## end SAF_IO::SAF_Str_Block%3CF6B7C7026D.postscript

    //## begin SAF_IO::Hyperslab%3CFBAAC3025E.preface preserve=yes
    //## end SAF_IO::Hyperslab%3CFBAAC3025E.preface

    //## Class: Hyperslab%3CFBAAC3025E; private
    //## Category: framework%3C55965E009C
    //## Subsystem: framework%3C55990A032C
    //## Persistence: Transient
    //## Cardinality/Multiplicity: n

    class Hyperslab 
    {
      //## begin SAF_IO::Hyperslab%3CFBAAC3025E.initialDeclarations preserve=yes
      //## end SAF_IO::Hyperslab%3CFBAAC3025E.initialDeclarations

      public:
        // Data Members for Class Attributes

          //## Attribute: x_start%3CFBAAF402CB
          //	X-direction start index
          //## begin SAF_IO::Hyperslab::x_start%3CFBAAF402CB.attr preserve=no  public: int {UA} 0
          int x_start;
          //## end SAF_IO::Hyperslab::x_start%3CFBAAF402CB.attr

          //## Attribute: x_count%3CFBAB610089
          //	X-direction count.
          //## begin SAF_IO::Hyperslab::x_count%3CFBAB610089.attr preserve=no  public: int {UA} 0
          int x_count;
          //## end SAF_IO::Hyperslab::x_count%3CFBAB610089.attr

          //## Attribute: x_skip%3CFBAB630193
          //	X-direction index skip factor
          //## begin SAF_IO::Hyperslab::x_skip%3CFBAB630193.attr preserve=no  public: int {UA} 1
          int x_skip;
          //## end SAF_IO::Hyperslab::x_skip%3CFBAB630193.attr

          //## Attribute: y_start%3CFBABD301F0
          //	Y-direction start index
          //## begin SAF_IO::Hyperslab::y_start%3CFBABD301F0.attr preserve=no  public: int {UA} 0
          int y_start;
          //## end SAF_IO::Hyperslab::y_start%3CFBABD301F0.attr

          //## Attribute: y_count%3CFBABF50099
          //	Y-direction count.
          //## begin SAF_IO::Hyperslab::y_count%3CFBABF50099.attr preserve=no  public: int {UA} 0
          int y_count;
          //## end SAF_IO::Hyperslab::y_count%3CFBABF50099.attr

          //## Attribute: y_skip%3CFBAC33003B
          //	Y-direction index skip factor
          //## begin SAF_IO::Hyperslab::y_skip%3CFBAC33003B.attr preserve=no  public: int {UA} 1
          int y_skip;
          //## end SAF_IO::Hyperslab::y_skip%3CFBAC33003B.attr

          //## Attribute: z_start%3CFBABD501C2
          //	Z-direction start index
          //## begin SAF_IO::Hyperslab::z_start%3CFBABD501C2.attr preserve=no  public: int {UA} 0
          int z_start;
          //## end SAF_IO::Hyperslab::z_start%3CFBABD501C2.attr

          //## Attribute: z_count%3CFBABF60396
          //	Z-direction count.
          //## begin SAF_IO::Hyperslab::z_count%3CFBABF60396.attr preserve=no  public: int {UA} 0
          int z_count;
          //## end SAF_IO::Hyperslab::z_count%3CFBABF60396.attr

          //## Attribute: z_skip%3CFBAC340358
          //	Z-direction index skip factor
          //## begin SAF_IO::Hyperslab::z_skip%3CFBAC340358.attr preserve=no  public: int {UA} 1
          int z_skip;
          //## end SAF_IO::Hyperslab::z_skip%3CFBAC340358.attr

        // Additional Public Declarations
          //## begin SAF_IO::Hyperslab%3CFBAAC3025E.public preserve=yes
          //## end SAF_IO::Hyperslab%3CFBAAC3025E.public

      protected:
        // Additional Protected Declarations
          //## begin SAF_IO::Hyperslab%3CFBAAC3025E.protected preserve=yes
          //## end SAF_IO::Hyperslab%3CFBAAC3025E.protected

      private:
        // Additional Private Declarations
          //## begin SAF_IO::Hyperslab%3CFBAAC3025E.private preserve=yes
          //## end SAF_IO::Hyperslab%3CFBAAC3025E.private

      private: //## implementation
        // Additional Implementation Declarations
          //## begin SAF_IO::Hyperslab%3CFBAAC3025E.implementation preserve=yes
          //## end SAF_IO::Hyperslab%3CFBAAC3025E.implementation

    };

    //## begin SAF_IO::Hyperslab%3CFBAAC3025E.postscript preserve=yes
    //## end SAF_IO::Hyperslab%3CFBAAC3025E.postscript

  public:
    //## Constructors (specified)
      //## Operation: SAF_IO%3CC416910222
      //	Constructor.
      SAF_IO (Region *region);

    //## Destructor (specified)
      //## Operation: ~SAF_IO%3CC416C6002E
      //	Destructor.
      virtual ~SAF_IO ();


    //## Other Operations (specified)
      //## Operation: Write_Prologue%3CC41838006D
      //	Virtual method to handle object setup and initialization.
      virtual void Write_Prologue ();

      //## Operation: Write_Epilogue%3CC4183B0000
      //	Virtual method to handle end-of-run and cleanup tasks.
      virtual void Write_Epilogue ();

      //## Operation: Write_Time%3CC4183D01B5
      //	Virtual method to write requested output at a single
      //	timestep.
      virtual void Write_Time ();

    // Additional Public Declarations
      //## begin SAF_IO%3CC41527036B.public preserve=yes
      //## end SAF_IO%3CC41527036B.public

  protected:

    //## Other Operations (specified)
      //## Operation: get_dir_from_var%3D08D53701F4
      //	Function to extract directionality of edge or face
      //	variable from its attributes.
      int get_dir_from_var (Variable_Attributes *v_attr	// Attributes associated w/ edge or face variable.
      );

      //## Operation: get_all_SAF_structured_blocks%3D08D61C01A5
      //	Function to initialize all SAF output requests for all
      //	structured regions.
      void get_all_SAF_structured_blocks (Region *region	// The region to be initialized for SAF.
      );

      //## Operation: create_template%3CFBB2B5006A
      //	Helper function for creating the SAF field template
      //	associated with a specific output request.
      SAF_FieldTmpl create_template (const char *a_templateName, 	// Name of template
      const char *a_varUnits	// units of the field associated with the template
      );

      //## Operation: add_block%3D08D93B02DE
      //	Create a block (a 3d separable mesh), given the x, y & z
      //	dimensions and 3 1d arrays containing the x, y & z
      //	coordinates.
      //
      //	SAF Definition: Create a (curvilinear) set of nodes with
      //	3d coordinate fields. This function takes the 3 1d
      //	coordinate arrays and creates a node set with a 3
      //	component (xyz) 3d array of size a_xDim*a_yDim*a_zDim,
      //	in fortran order This is the most inefficient way of
      //	storing the 3d separable mesh, but will suffice for now.
      //
      //	This function creates nodes and elems collections on the
      //	set. It also creates x-pointing, y-pointing, and
      //	z-pointing edge collections, and x-normal, y-normal, and
      //	z-normal face collections.
      //
      //	Note: every function defined here assumes that the data
      //	was created by on of these functions. That is, some
      //	checks of the data are made, but other conditions are
      //	just assumed.
      SAF_Set add_block (const char *a_blockName, 	// The name to give to the block node set.
      const char *a_units, 	// The units of length associated with the nodal
      	// coordinate values.
      int a_xDim, 	// Number of nodes in the x coordinate direction.
      int a_yDim, 	// Number of nodes in the y coordinate direction.
      int a_zDim, 	// Number of nodes in the z coordinate direction.
      Real *a_xCoords, 	// Coordinate values for nodes in the x coordinate
      	// direction.
      Real *a_yCoords, 	// Coordinate values for nodes in the y coordinate
      	// direction.
      Real *a_zCoords	// Coordinate values for nodes in the z coordinate
      	// direction.
      );

      //## Operation: create_node_subset%3CFBB3BE01C2
      //	Helper function for creating the SAF subset associated
      //	with a specific centering.
      SAF_Set create_node_subset (SAF_Set a_blockNodeSet, 	// The original node set.
      Hyperslab &a_doList, 	// Definition of subset of original node set.
      const char *a_sub_name	// The name for the subset created.
      );

      //## Operation: write_variable_using_existing_template%3CFBB495006A
      //	Helper function for writing the SAF field associated
      //	with a specific output request at a single timestep.
      SAF_Field write_variable_using_existing_template (SAF_Set a_onSet, 	// The SAF set associated with this field.
      SAF_FieldTmpl a_varFieldTemplate, 	// The SAF field template associated with this field.
      Real *a_varData, 	// array of variable values, column-major ordered, 1 for
      	// each member of a_set
      Real a_time, 	// Time value of output request.
      SAF_Cat a_cat	// The SAF category associated with this field.
      );

      //## Operation: write_timestep_data%3CFBB6CB0339
      //	This function writes a SAF StateGroup that describes all
      //	the fields written at this single timestep.
      void write_timestep_data (Real a_timeValue	// The time value associated with this output request.
      );

      //## Operation: create_top_set%3E1B5C75006D
      //	Helper function for creating the SAF set that contains
      //	the SAF sets associated with all blocks.
      void create_top_set ();

      //## Operation: create_suite%3E1B5CCD034B
      //	Helper function for creating the SAF suite that contains
      //	the output for the entire simulation.
      void create_suite ();

    // Data Members for Class Attributes

      //## Attribute: db%3CFBAD9801C2
      //	 Handle to the SAF database.
      //## begin SAF_IO::db%3CFBAD9801C2.attr preserve=no  protected: SAF_Db {UA} 
      SAF_Db db;
      //## end SAF_IO::db%3CFBAD9801C2.attr

      //## Attribute: file%3CFBADDA02BC
      //	Handle to the saf file.
      //## begin SAF_IO::file%3CFBADDA02BC.attr preserve=no  protected: SAF_File {UA} 
      SAF_File file;
      //## end SAF_IO::file%3CFBADDA02BC.attr

      //## Attribute: nodes%3CFBADF901C2
      //	 Handle to the nodes category.
      //## begin SAF_IO::nodes%3CFBADF901C2.attr preserve=no  protected: SAF_Cat {UA} 
      SAF_Cat nodes;
      //## end SAF_IO::nodes%3CFBADF901C2.attr

      //## Attribute: elems%3CFBAE140125
      //	Handle to the elements category.
      //## begin SAF_IO::elems%3CFBAE140125.attr preserve=no  protected: SAF_Cat {UA} 
      SAF_Cat elems;
      //## end SAF_IO::elems%3CFBAE140125.attr

      //## Attribute: edge_cat%3CFBAE500145
      //	Array of handles to SAF categories for the 3 edge
      //	directions.
      //## begin SAF_IO::edge_cat%3CFBAE500145.attr preserve=no  protected: SAF_Cat[3] {UA} 
      SAF_Cat edge_cat[3];
      //## end SAF_IO::edge_cat%3CFBAE500145.attr

      //## Attribute: face_cat%3CFBAE98030A
      //	Array of handles to SAF categories for the 3 face
      //	directions.
      //## begin SAF_IO::face_cat%3CFBAE98030A.attr preserve=no  protected: SAF_Cat[3] {UA} 
      SAF_Cat face_cat[3];
      //## end SAF_IO::face_cat%3CFBAE98030A.attr

      //## Attribute: saved_fields%3CFBAEC402BC
      //	Array to save all SAF fields written at a single
      //	timestep so they can be added to the SAF State that is
      //	written at the end of the timestep.
      //## begin SAF_IO::saved_fields%3CFBAEC402BC.attr preserve=no  protected: SAF_Field * {UA} 0
      SAF_Field *saved_fields;
      //## end SAF_IO::saved_fields%3CFBAEC402BC.attr

      //## Attribute: field_count%3CFBB04F02EA
      //## begin SAF_IO::field_count%3CFBB04F02EA.attr preserve=no  protected: int {UA} 0
      int field_count;
      //## end SAF_IO::field_count%3CFBB04F02EA.attr

      //## Attribute: saf_req%3CFBB0B100A8
      //## begin SAF_IO::saf_req%3CFBB0B100A8.attr preserve=no  protected: STLvector(SAF_Request *) {UA} 
      STLvector(SAF_Request *) saf_req;
      //## end SAF_IO::saf_req%3CFBB0B100A8.attr

      //## Attribute: saf_blks%3CFBB2540089
      //## begin SAF_IO::saf_blks%3CFBB2540089.attr preserve=no  protected: STLvector(SAF_Str_Block *) {UA} 
      STLvector(SAF_Str_Block *) saf_blks;
      //## end SAF_IO::saf_blks%3CFBB2540089.attr

      //## Attribute: state_count%3E1B580503D8
      //	counter for number of SAF states that have been written
      //	to the SAF suite.
      //## begin SAF_IO::state_count%3E1B580503D8.attr preserve=no  protected: int {UA} 0
      int state_count;
      //## end SAF_IO::state_count%3E1B580503D8.attr

      //## Attribute: num_fields_per_state%3E1B58C2038A
      //	Number of SAF fields for SAF state.
      //## begin SAF_IO::num_fields_per_state%3E1B58C2038A.attr preserve=no  protected: int {UA} -1
      int num_fields_per_state;
      //## end SAF_IO::num_fields_per_state%3E1B58C2038A.attr

      //## Attribute: suite%3E1B59230196
      //	 Handle to the SAF suite for this run.
      //## begin SAF_IO::suite%3E1B59230196.attr preserve=no  protected: SAF_Suite {UA} 
      SAF_Suite suite;
      //## end SAF_IO::suite%3E1B59230196.attr

      //## Attribute: state_group%3E1B595A00AB
      //	 Handle to the SAF state group for this run.
      //## begin SAF_IO::state_group%3E1B595A00AB.attr preserve=no  protected: SAF_StateGrp {UA} 
      SAF_StateGrp state_group;
      //## end SAF_IO::state_group%3E1B595A00AB.attr

      //## Attribute: top_set%3E1B59870261
      //	 Handle to the SAF set that contains the sets associated
      //	with all the simulation blocks.
      //## begin SAF_IO::top_set%3E1B59870261.attr preserve=no  protected: SAF_Set {UA} 
      SAF_Set top_set;
      //## end SAF_IO::top_set%3E1B59870261.attr

      //## Attribute: coords_field_template%3E1B5BBD0290
      //	SAF field template for the combined 3D coordinate grid
      //	composed of 3 1D coordinate grids.
      //## begin SAF_IO::coords_field_template%3E1B5BBD0290.attr preserve=no  protected: SAF_FieldTmpl {UA} 
      SAF_FieldTmpl coords_field_template;
      //## end SAF_IO::coords_field_template%3E1B5BBD0290.attr

      //## Attribute: coords_indiv_field_templates%3E1B5B490186
      //	Array to save a SAF field template for the coordinate
      //	grid in each coordinate direction.
      //## begin SAF_IO::coords_indiv_field_templates%3E1B5B490186.attr preserve=no  protected: SAF_FieldTmpl[3] {UA} 
      SAF_FieldTmpl coords_indiv_field_templates[3];
      //## end SAF_IO::coords_indiv_field_templates%3E1B5B490186.attr

    // Additional Protected Declarations
      //## begin SAF_IO%3CC41527036B.protected preserve=yes
      //## end SAF_IO%3CC41527036B.protected

  private:
    // Additional Private Declarations
      //## begin SAF_IO%3CC41527036B.private preserve=yes
      //## end SAF_IO%3CC41527036B.private

  private: //## implementation
    // Additional Implementation Declarations
      //## begin SAF_IO%3CC41527036B.implementation preserve=yes
      //## end SAF_IO%3CC41527036B.implementation

};

//## begin SAF_IO%3CC41527036B.postscript preserve=yes
//## end SAF_IO%3CC41527036B.postscript

//## begin module%3CC417390148.epilog preserve=yes
//## end module%3CC417390148.epilog


#endif
