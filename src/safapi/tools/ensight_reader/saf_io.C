# Copyright(C) 1999-2005 The Regents of the University of California.
#     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
#     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
#     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
#     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
#     dissemination, commercialization  through formal licensing, or other  disposition under terms of
#     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
#     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
#     University.
# 
# Copyright(C) 1999-2005 Sandia Corporation.  
#     Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive license for use of this work
#     on behalf of the U.S. Government.  Export  of this program may require a license from the United
#     States Government.
# 
# Disclaimer:
#     This document was  prepared as an account of  work sponsored by an agency  of  the United States
#     Government. Neither the United States  Government nor the United States Department of Energy nor
#     the  University  of  California  nor  Sandia  Corporation nor any  of their employees  makes any
#     warranty, expressed  or  implied, or  assumes   any  legal liability  or responsibility  for the
#     accuracy,  completeness,  or  usefulness  of  any  information, apparatus,  product,  or process
#     disclosed,  or  represents that its  use would   not infringe  privately owned rights. Reference
#     herein  to any  specific commercial  product,  process,  or  service by  trade  name, trademark,
#     manufacturer,  or  otherwise,  does  not   necessarily  constitute  or  imply  its  endorsement,
#     recommendation, or favoring by the  United States Government   or the University of  California.
#     The views and opinions of authors expressed herein do not necessarily state  or reflect those of
#     the  United  States Government or  the   University of California   and shall  not be  used  for
#     advertising or product endorsement purposes.
# 
# 
# Active Developers:
#     Peter K. Espen              SNL
#     Eric A. Illescas            SNL
#     Jake S. Jones               SNL
#     Robb P. Matzke              LLNL
#     Greg Sjaardema              SNL
# 
# Inactive Developers:
#     William J. Arrighi          LLNL
#     Ray T. Hitt                 SNL
#     Mark C. Miller              LLNL
#     Matthew O'Brien             LLNL
#     James F. Reus               LLNL
#     Larry A. Schoof             SNL
# 
# Acknowledgements:
#     Marty L. Barnaby            SNL - Red parallel perf. study/tuning
#     David M. Butler             LPS - Data model design/implementation Spec.
#     Albert K. Cheng             NCSA - Parallel HDF5 support
#     Nancy Collins               IBM - Alpha/Beta user
#     Linnea M. Cook              LLNL - Management advocate
#     Michael J. Folk             NCSA - Management advocate 
#     Richard M. Hedges           LLNL - Blue-Pacific parallel perf. study/tuning 
#     Wilbur R. Johnson           SNL - Early developer
#     Quincey Koziol              NCSA - Serial HDF5 Support 
#     Celeste M. Matarazzo        LLNL - Management advocate
#     Tyce T. McLarty             LLNL - parallel perf. study/tuning
#     Tom H. Robey                SNL - Early developer
#     Reinhard W. Stotzer         SNL - Early developer
#     Judy Sturtevant             SNL - Red parallel perf. study/tuning 
#     Robert K. Yates             LLNL - Blue-Pacific parallel perf. study/tuning
# 

//## begin module%1.3%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.3%.codegen_version

//## begin module%3CC4178203C8.cm preserve=yes
//	 $Id: saf_io.C,v 1.5 2005/05/10 18:30:46 gdsjaar Exp $
//## end module%3CC4178203C8.cm

//## begin module%3CC4178203C8.cp preserve=no
//## end module%3CC4178203C8.cp

//## Module: saf_io%3CC4178203C8; Package body
//## Subsystem: framework%3C55990A032C
//## Source file: \\Cerberus\Projects\Emphasis\Nevada\framework\saf_io.C

//## begin module%3CC4178203C8.additionalIncludes preserve=no
//## end module%3CC4178203C8.additionalIncludes

//## begin module%3CC4178203C8.includes preserve=yes
#ifdef CODE_MP
# define HAVE_PARALLEL 1
#endif

#define ZERO_3DF(nx,ny,nz)  _saf_indexspec(3, nx,ny,nz, 0,0,0, SAF_F_ORDER)

#include "file_io.h"
#include "processors.h"
#include "math.h"
#include "StrDataMesh.h"
#include "structured_region.h"
#include "StrArrayVar.h"
#include "unit.h"
#include "index.h"
//## end module%3CC4178203C8.includes

// saf_io
#include "saf_io.h"
//## begin module%3CC4178203C8.declarations preserve=no
//## end module%3CC4178203C8.declarations

//## begin module%3CC4178203C8.additionalDeclarations preserve=yes
//## end module%3CC4178203C8.additionalDeclarations


// Class SAF_IO::SAF_Request 










//## Other Operations (implementation)
bool SAF_IO::SAF_Request::Is_Output_Step (int timestep) const
{
  //## begin SAF_IO::SAF_Request::Is_Output_Step%3CF6B4F700B8.body preserve=yes
  if ( timestep < first || (last >= 0 && timestep > last) ) return false;
  if ( (timestep - first) % step != 0 ) return false;
  return true;
  //## end SAF_IO::SAF_Request::Is_Output_Step%3CF6B4F700B8.body
}

// Additional Declarations
  //## begin SAF_IO::SAF_Request%3CF6ACD3022F.declarations preserve=yes
  //## end SAF_IO::SAF_Request%3CF6ACD3022F.declarations

// Class SAF_IO::SAF_Str_Block 




SAF_IO::SAF_Str_Block::SAF_Str_Block (SAF_Set set)
  //## begin SAF_IO::SAF_Str_Block::SAF_Str_Block%3CF6B92F004B.hasinit preserve=no
      : blockSet(set)
  //## end SAF_IO::SAF_Str_Block::SAF_Str_Block%3CF6B92F004B.hasinit
  //## begin SAF_IO::SAF_Str_Block::SAF_Str_Block%3CF6B92F004B.initialization preserve=yes
  //## end SAF_IO::SAF_Str_Block::SAF_Str_Block%3CF6B92F004B.initialization
{
  //## begin SAF_IO::SAF_Str_Block::SAF_Str_Block%3CF6B92F004B.body preserve=yes
  int i;
  for(i=0;i<3;i++) {
    edge_subset[i] = SAF_NOT_SET_SET;
    face_subset[i] = SAF_NOT_SET_SET;
  }
  //## end SAF_IO::SAF_Str_Block::SAF_Str_Block%3CF6B92F004B.body
}


// Additional Declarations
  //## begin SAF_IO::SAF_Str_Block%3CF6B7C7026D.declarations preserve=yes
  //## end SAF_IO::SAF_Str_Block%3CF6B7C7026D.declarations

// Class SAF_IO::Hyperslab 










// Additional Declarations
  //## begin SAF_IO::Hyperslab%3CFBAAC3025E.declarations preserve=yes
  //## end SAF_IO::Hyperslab%3CFBAAC3025E.declarations

// Class SAF_IO 


















SAF_IO::SAF_IO (Region *region)
  //## begin SAF_IO::SAF_IO%3CC416910222.hasinit preserve=no
      : saved_fields(0),
        field_count(0),
        state_count(0),
        num_fields_per_state(-1)
  //## end SAF_IO::SAF_IO%3CC416910222.hasinit
  //## begin SAF_IO::SAF_IO%3CC416910222.initialization preserve=yes
     , Region_IO(region)
  //## end SAF_IO::SAF_IO%3CC416910222.initialization
{
  //## begin SAF_IO::SAF_IO%3CC416910222.body preserve=yes
  TASK_0_cout << "SAF_IO::SAF_IO(Region *) called." << endl;

#ifdef USE_SAF_IO
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);
#endif
  //## end SAF_IO::SAF_IO%3CC416910222.body
}


SAF_IO::~SAF_IO ()
{
  //## begin SAF_IO::~SAF_IO%3CC416C6002E.body preserve=yes
  TASK_0_cout << "SAF_IO::~SAF_IO() called." << endl;

#ifdef USE_SAF_IO
  // FIXME move to somewhere else
  int i;

  delete [] saved_fields;

  for(i=0;i<saf_blks.size();i++) {
    delete saf_blks[i];
    saf_blks[i] = 0;
  }
  for(i=0;i<saf_req.size();i++) {
    delete saf_req[i];
    saf_req[i] = 0;
  }

  saf_close_database(db);
  saf_final();
#endif
  //## end SAF_IO::~SAF_IO%3CC416C6002E.body
}



//## Other Operations (implementation)
void SAF_IO::Write_Prologue ()
{
  //## begin SAF_IO::Write_Prologue%3CC41838006D.body preserve=yes
  TASK_0_cout << "SAF_IO::Write_Prologue() called." << endl;
#ifdef USE_SAF_IO
  SAF_DbProps l_dbprops; /* Handle to the SAF database properties. */
  const char *l_dbname = filename.c_str();

  saf_init(SAF_DEFAULT_LIBPROPS);

  /* Create the database properties. */
  SAF_TRY_BEGIN {

    coords_field_template = SAF_NOT_SET_FIELDTMPL;
    coords_indiv_field_templates[0] = SAF_NOT_SET_FIELDTMPL;
    coords_indiv_field_templates[1] = SAF_NOT_SET_FIELDTMPL;
    coords_indiv_field_templates[2] = SAF_NOT_SET_FIELDTMPL;
    suite = SAF_NOT_SET_SUITE;
    state_group = SAF_NOT_SET_STATEGRP;
    top_set = SAF_NOT_SET_SET;

    l_dbprops = saf_createProps_database();

  /* Set the clobber database property so any existing file
   * will be overwritten. */
    saf_setProps_Clobber(l_dbprops);

  /* Open the SAF database. Give it name l_dbname, properties p and
   * set db to be a handle to this database. */
    db = saf_open_database(l_dbname,l_dbprops);

  /* Get the handle to the master file. */
    file = SAF_MASTER(db);

  saf_declare_category(db, "nodes",  SAF_TOPOLOGY, 0, &nodes);
  saf_declare_category(db, "xEdges",  SAF_TOPOLOGY, 1, edge_cat);
  saf_declare_category(db, "yEdges",  SAF_TOPOLOGY, 1, edge_cat+1);
  saf_declare_category(db, "zEdges",  SAF_TOPOLOGY, 1, edge_cat+2);
  saf_declare_category(db, "xFaces",  SAF_TOPOLOGY, 2, face_cat);
  saf_declare_category(db, "yFaces",  SAF_TOPOLOGY, 2, face_cat+1);
  saf_declare_category(db, "zFaces",  SAF_TOPOLOGY, 2, face_cat+2);
  saf_declare_category(db, "elems",  SAF_TOPOLOGY, 3, &elems);

  get_all_SAF_structured_blocks( my_region );
    create_top_set();
    create_suite();
  }
  SAF_CATCH {
    TASK_0_cout << "SAF_IO::Write_Prologue(): SAF error caught." << endl;
  }
  SAF_TRY_END
#endif
  //## end SAF_IO::Write_Prologue%3CC41838006D.body
}

void SAF_IO::Write_Epilogue ()
{
  //## begin SAF_IO::Write_Epilogue%3CC4183B0000.body preserve=yes
  TASK_0_cout << "SAF_IO::Write_Epilogue() called." << endl;  
  //## end SAF_IO::Write_Epilogue%3CC4183B0000.body
}

void SAF_IO::Write_Time ()
{
  //## begin SAF_IO::Write_Time%3CC4183D01B5.body preserve=yes
  TASK_0_cout << "SAF_IO::Write_Time() called." << endl;

#ifdef USE_SAF_IO
  int l_timestep = my_region->Cycle();
  Real tvalue = my_region->Time();

  SAF_TRY_BEGIN {
    STLvector(SAF_Request *)::iterator iter;
    int count = 0; // This is for the future when requests might be conditional
    field_count = 0;
    for (iter=saf_req.begin(); iter!=saf_req.end(); iter++) {

      SAF_Field fld = SAF_NOT_APPLICABLE_FIELD;
      if ( (*iter)->Is_Output_Step(l_timestep) ) {
        fld = write_variable_using_existing_template((*iter)->on_set,
                                                     (*iter)->tmpl, 
                                                     (*iter)->data, tvalue,
                                                     (*iter)->category);
        count++;
      }
      saved_fields[field_count++] = fld;
    }

    if ( count > 0 ) write_timestep_data(tvalue);
  }
  SAF_CATCH {
    TASK_0_cout << "SAF_IO::Write_Time(): SAF error caught!" << endl;
  }
  SAF_TRY_END
#endif
  //## end SAF_IO::Write_Time%3CC4183D01B5.body
}

int SAF_IO::get_dir_from_var (Variable_Attributes *v_attr)
{
  //## begin SAF_IO::get_dir_from_var%3D08D53701F4.body preserve=yes
  String var_name = v_attr->Name();
  // String doesn't support //  char last = tolower( *var_name.rbegin() );
  char last = tolower( var_name[var_name.size()-1] );
  switch (last) {
    case 'i' : case '1' : case 'x':
      return 0;
    case 'j' : case '2' : case 'y':
      return 1;
    case 'k' : case '3' : case 'z':
      return 2;
  }
  return 0;
  //## end SAF_IO::get_dir_from_var%3D08D53701F4.body
}

void SAF_IO::get_all_SAF_structured_blocks (Region *region)
{
  //## begin SAF_IO::get_all_SAF_structured_blocks%3D08D61C01A5.body preserve=yes
#ifdef TWO_D
  Real zero = 0.0;
#endif

  int nregions = region->Number_of_Child_Regions(STRUCTURED_REGIONS);
  if ( nregions > 0 ) {
    for(int i=0;i<nregions;i++) {
      get_all_SAF_structured_blocks(region->Child(i));
    }
  }
  else {
    /* FIXME -- hard wiring of output for structured blocks */
    int number_of_strings = 6;
    String * variable_strings = new String[number_of_strings];
    variable_strings[0] = String("ELECTRIC_FIELD_I"); 
    variable_strings[1] = String("ELECTRIC_FIELD_J"); 
    variable_strings[2] = String("ELECTRIC_FIELD_K"); 
    variable_strings[3] = String("MAGNETIC_FLUX_DENSITY_I"); 
    variable_strings[4] = String("MAGNETIC_FLUX_DENSITY_J"); 
    variable_strings[5] = String("MAGNETIC_FLUX_DENSITY_K");

    Structured_Region *sreg = dynamic_cast<Structured_Region *>(region);
    StrBlock *b = sreg->getMesh()->getBlock(0);
    StrArrayVar<BlockVar>* bv = b->getBlockVar();
    int num_nodes_i, num_nodes_j, num_nodes_k;
    num_nodes_i = b->getControl(NNPI_TOT);
    num_nodes_j = b->getControl(NNPJ_TOT);
#ifdef THREE_D
    num_nodes_k = b->getControl(NNPK_TOT);
#else
    num_nodes_k = 1;
#endif

    char blk_number[] = "0000000000000000";
    sprintf(blk_number,"%d",b->GlobalId());
    String blk_str = String("Block_") + String(blk_number);

    char cunit[] = "meter";
    SAF_Set blockSet = add_block( blk_str.c_str(), cunit,
                        num_nodes_i,
                        num_nodes_j,
                        num_nodes_k,
                        bv->Scalar_Array(bv->getVarIndex(sreg->Get_Scalar_Index_1())), 
                        bv->Scalar_Array(bv->getVarIndex(sreg->Get_Scalar_Index_2())),
#ifdef THREE_D
                        bv->Scalar_Array(bv->getVarIndex(sreg->Get_Scalar_Index_3()))
#else
                        &zero
#endif
                                   );
    // char ctname[] = "Title";
    // saf_put_set_att(SAF_ALL,blockSet,ctname,DSL_STRING,1,(void *) ctmp );
    // int ghosts[] = { 1,1,1,1,1,1 };
    // saf_put_set_att(SAF_ALL,blockSet,"Ghosts",DSL_INT,6,(void *) ghosts);

    SAF_Str_Block *l_blk = new SAF_Str_Block(blockSet);

    Hyperslab l_slab;
  
    l_slab.x_start = 0;
    l_slab.x_skip = 1;
    l_slab.y_start = 0;
    l_slab.y_skip = 1;
    l_slab.z_start = 0;
    l_slab.z_skip = 1;

    Variable_Attributes *v_attr = 0;
    SAF_FieldTmpl l_template;
    Real *field_data = 0;
    SAF_Request *next = 0;

    int i;
    char cdirs[] = "XYZ";
    for ( i=0;i<number_of_strings;i++) {
      String var_name = variable_strings[i];
      v_attr = sreg->getDataRegister().Find_Variable(var_name);
      if ( v_attr ) {
        Variable_Centering centering = v_attr->Centering();
        int dir = get_dir_from_var(v_attr);
        Unit unit = v_attr->Units();
        char *uname = unit.Unit_Label();
        Variable_Index index = v_attr->Index();
        StrArrayVar<EdgeVar> *edgevar;
        StrArrayVar<FaceVar> *facevar;
        switch  (centering) {
#ifdef THREE_D
          case EDGE_VAR:
            edgevar = b->getEdgeVar();
            field_data = edgevar->getPointer(index);
            if ( SAF_EQUIV(l_blk->edge_subset[dir],SAF_NOT_SET_SET) ) {
              l_slab.x_count = dir == 0 ? num_nodes_i : num_nodes_i-1;
              l_slab.y_count = dir == 1 ? num_nodes_j : num_nodes_j-1;
              l_slab.z_count = dir == 2 ? num_nodes_k : num_nodes_k-1;
              String str = blk_str + "_" + cdirs[dir] + "edge_subset";
              l_blk->edge_subset[dir] = create_node_subset(blockSet, l_slab,
                                                           str.c_str());
            }
            l_template = create_template(var_name.c_str(),uname);
            next = new SAF_Request(var_name, l_blk->edge_subset[dir], 
                                   l_template, edge_cat[dir], field_data);
            // next->first = next->step = 200;  // TEMPORARY
        saf_req.push_back(next);
            break;
#endif
          case FACE_VAR:
            facevar = b->getFaceVar();
            field_data = facevar->getPointer(index);
            if ( SAF_EQUIV(l_blk->face_subset[dir],SAF_NOT_SET_SET) ) {
              l_slab.x_count = dir != 0 ? num_nodes_i : num_nodes_i-1;
              l_slab.y_count = dir != 1 ? num_nodes_j : num_nodes_j-1;
              l_slab.z_count = dir != 2 ? num_nodes_k : num_nodes_k-1;
              String str = blk_str + "_" + cdirs[dir] + "face_subset";
              l_blk->face_subset[dir] = create_node_subset(blockSet, l_slab,
                                                           str.c_str());
            }
            l_template = create_template(var_name.c_str(),uname);
            next = new SAF_Request(var_name, l_blk->face_subset[dir], 
                                   l_template, face_cat[dir], field_data);
            // next->first = next->step = 300;  // TEMPORARY
            saf_req.push_back(next);
            break;
        }
      }
      else { 
        return;
      }
    }
    saf_blks.push_back(l_blk);
    /* FIXME -- hard wiring of output for structured blocks */
    delete [] variable_strings;
  }
  //## end SAF_IO::get_all_SAF_structured_blocks%3D08D61C01A5.body
}

SAF_FieldTmpl SAF_IO::create_template (const char *a_templateName, const char *a_varUnits)
{
  //## begin SAF_IO::create_template%3CFBB2B5006A.body preserve=yes
  SAF_FieldTmpl l_varFieldTemplate = SAF_NOT_SET_FIELDTMPL;

#ifdef USE_SAF_IO
   SAF_Quantity l_quantity;
   SAF_Unit l_unit;

   l_unit = saf_find_one_unit(db, a_varUnits);
   saf_describe_unit(l_unit, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
                     &l_quantity);
   saf_declare_field_tmpl(SAF_ALL, a_templateName, db,
                          SAF_ALGTYPE_SCALAR, SAF_UNITY, 
                          l_quantity, 1,
                          NULL, &l_varFieldTemplate );
   char cuname[] = "Units";
   saf_put_field_tmpl_att(SAF_ALL,l_varFieldTemplate, cuname,
                          DSL_STRING,1,(void *) a_varUnits);

#endif
   return(l_varFieldTemplate);
  //## end SAF_IO::create_template%3CFBB2B5006A.body
}

SAF_Set SAF_IO::add_block (const char *a_blockName, const char *a_units, int a_xDim, int a_yDim, int a_zDim, Real *a_xCoords, Real *a_yCoords, Real *a_zCoords)
{
  //## begin SAF_IO::add_block%3D08D93B02DE.body preserve=yes
   SAF_Set l_blockNodeSet = SAF_NOT_SET_SET;
#ifdef USE_SAF_IO

  //ensure that there will be no errors if this is dummy set (i.e. size 0x0x0)
  int l_xMinusOne = (a_xDim-1>=0)?(a_xDim-1):0;
  int l_yMinusOne = (a_yDim-1>=0)?(a_yDim-1):0;
  int l_zMinusOne = (a_zDim-1>=0)?(a_zDim-1):0;

  int l_numNodes = a_xDim*a_yDim*a_zDim;
  int l_numXEdges = l_xMinusOne*a_yDim*a_zDim;
  int l_numYEdges = a_xDim*l_yMinusOne*a_zDim;
  int l_numZEdges = a_xDim*a_yDim*l_zMinusOne;
  int l_numXFaces = a_xDim*l_yMinusOne*l_zMinusOne;
  int l_numYFaces = l_xMinusOne*a_yDim*l_zMinusOne;
  int l_numZFaces = l_xMinusOne*l_yMinusOne*a_zDim;
  int l_numElems = l_xMinusOne*l_yMinusOne*l_zMinusOne;
  SAF_Field l_compositeField, l_componentField[3]; 
  SAF_Unit l_units;

  /* note: if we wanted to make these coordinates time dependent, we would have
     to declare l_coordsField and l_coordsFieldTemplate as pointers, and 
     allocate them here, so that they still exist when we create the states & 
     suites */


  saf_declare_set(SAF_EACH, db, a_blockName, 3, SAF_SPACE, 
                  SAF_EXTENDIBLE_FALSE, &l_blockNodeSet );

  saf_declare_collection(SAF_EACH, l_blockNodeSet, nodes,  SAF_CELLTYPE_POINT, 
                         l_numNodes, ZERO_3DF(a_xDim,a_yDim,a_zDim), 
			 SAF_DECOMP_FALSE);
  saf_declare_collection(SAF_EACH, l_blockNodeSet, edge_cat[0], 
                         SAF_CELLTYPE_LINE, l_numXEdges,
			 ZERO_3DF(l_xMinusOne,a_yDim,a_zDim),SAF_DECOMP_FALSE);
  saf_declare_collection(SAF_EACH, l_blockNodeSet, edge_cat[1], 
                         SAF_CELLTYPE_LINE, l_numYEdges,
			 ZERO_3DF(a_xDim,l_yMinusOne,a_zDim),SAF_DECOMP_FALSE);
  saf_declare_collection(SAF_EACH, l_blockNodeSet, edge_cat[2],
                         SAF_CELLTYPE_LINE, l_numZEdges,
			 ZERO_3DF(a_xDim,a_yDim,l_zMinusOne),SAF_DECOMP_FALSE);
  saf_declare_collection(SAF_EACH, l_blockNodeSet, face_cat[0],
                         SAF_CELLTYPE_QUAD, l_numXFaces,
			 ZERO_3DF(a_xDim,l_yMinusOne,l_zMinusOne),
                         SAF_DECOMP_FALSE);
  saf_declare_collection(SAF_EACH, l_blockNodeSet, face_cat[1],
                         SAF_CELLTYPE_QUAD, l_numYFaces,
			 ZERO_3DF(l_xMinusOne,a_yDim,l_zMinusOne),
                         SAF_DECOMP_FALSE);
  saf_declare_collection(SAF_EACH, l_blockNodeSet, face_cat[2],
                         SAF_CELLTYPE_QUAD, l_numZFaces,
			 ZERO_3DF(l_xMinusOne,l_yMinusOne,a_zDim),
                         SAF_DECOMP_FALSE);
  saf_declare_collection(SAF_EACH, l_blockNodeSet, elems,
                         SAF_CELLTYPE_HEX, l_numElems,
			 ZERO_3DF(l_xMinusOne,l_yMinusOne,l_zMinusOne),
                         SAF_DECOMP_FALSE);

  if( SAF_EQUIV(coords_field_template,SAF_NOT_SET_FIELDTMPL) ) {
    saf_declare_field_tmpl(SAF_ALL, "coords_component_template", db,
                           SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_QLENGTH, 1, 
                           NULL, &coords_indiv_field_templates[0] );
    coords_indiv_field_templates[1] = coords_indiv_field_templates[0];
    coords_indiv_field_templates[2] = coords_indiv_field_templates[0];
    saf_declare_field_tmpl(SAF_ALL, "coords_composite_template",  db,
                           SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 3,
                           coords_indiv_field_templates, 
                           &coords_field_template);
  }

  /* Get a handle to the units for this field. */
  l_units = saf_find_one_unit(db, a_units);

  /* Declare the fields. */
  saf_declare_field(SAF_EACH, coords_indiv_field_templates[0],
                    "X_separable_coord_field",l_blockNodeSet,l_units,
                    SAF_SELF(db), SAF_SELF(db), a_xDim, SAF_SELF(db),   
		    SAF_SPACE_PWCONST,SAF_Real,NULL,SAF_INTERLEAVE_NONE,
		    SAF_IDENTITY,NULL, l_componentField);
  saf_declare_field(SAF_EACH, coords_indiv_field_templates[0], 
                    "Y_separable_coord_field",l_blockNodeSet,l_units,
                    SAF_SELF(db), SAF_SELF(db), a_yDim, SAF_SELF(db), 
		    SAF_SPACE_PWCONST,SAF_Real,NULL,SAF_INTERLEAVE_NONE,
		    SAF_IDENTITY,NULL, l_componentField+1);
  saf_declare_field(SAF_EACH, coords_indiv_field_templates[0],
                    "Z_separable_coord_field",l_blockNodeSet,l_units,   
		    SAF_SELF(db),SAF_SELF(db), a_zDim, SAF_SELF(db), 
		    SAF_SPACE_PWCONST,SAF_Real,NULL,SAF_INTERLEAVE_NONE,
                    SAF_IDENTITY,NULL, l_componentField+2);

  saf_declare_field(SAF_EACH, coords_field_template, "separable_coord_field", 
                    l_blockNodeSet, l_units,SAF_SELF(db),  
		    SAF_NODAL(nodes,elems), SAF_Real, l_componentField, 
		    SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL,
		    &l_compositeField);


  /*write the fields*/
  saf_write_field(SAF_EACH, l_componentField[0], SAF_WHOLE_FIELD, 1,
		  NULL, (void**)&a_xCoords, file);
  saf_write_field(SAF_EACH, l_componentField[1], SAF_WHOLE_FIELD, 1,
		  NULL, (void**)&a_yCoords, file);
  saf_write_field(SAF_EACH, l_componentField[2], SAF_WHOLE_FIELD, 1,
		  NULL, (void**)&a_zCoords, file);


  saf_declare_coords(SAF_EACH, l_compositeField);
  saf_declare_coords(SAF_EACH, l_componentField[0] );
  saf_declare_coords(SAF_EACH, l_componentField[1] );
  saf_declare_coords(SAF_EACH, l_componentField[2] );
   
  saf_declare_default_coords(SAF_EACH,l_blockNodeSet,l_compositeField);

#endif
   return(l_blockNodeSet);
  //## end SAF_IO::add_block%3D08D93B02DE.body
}

SAF_Set SAF_IO::create_node_subset (SAF_Set a_blockNodeSet, Hyperslab &a_doList, const char *a_sub_name)
{
  //## begin SAF_IO::create_node_subset%3CFBB3BE01C2.body preserve=yes
  SAF_Set l_subBlockNodeSet = SAF_NOT_SET_SET;
#ifdef USE_SAF_IO 
  int l_numNodes,l_numXEdges, l_numYEdges, l_numZEdges;
  int l_numXFaces, l_numYFaces, l_numZFaces, l_numElems;
  char l_str[256];
  SAF_Rel rel;
 

  //ensure that there will be no errors if this is dummy set (i.e. size 0x0x0)
  int l_x = (a_doList.x_count>=0) ? a_doList.x_count:0;
  int l_y = (a_doList.y_count>=0) ? a_doList.y_count:0;
  int l_z = (a_doList.z_count>=0) ? a_doList.z_count:0;
  int l_xMinusOne = (l_x-1>=0)?(l_x-1):0;
  int l_yMinusOne = (l_y-1>=0)?(l_y-1):0;
  int l_zMinusOne = (l_z-1>=0)?(l_z-1):0;

  l_numNodes =  l_x*l_y*l_z;
  l_numXEdges = l_xMinusOne*l_y*l_z;
  l_numYEdges = l_x*l_yMinusOne*l_z;
  l_numZEdges = l_x*l_y*l_zMinusOne;
  l_numXFaces = l_x*l_yMinusOne*l_zMinusOne;
  l_numYFaces = l_xMinusOne*l_y*l_zMinusOne;
  l_numZFaces = l_xMinusOne*l_yMinusOne*l_z;
  l_numElems =  l_xMinusOne*l_yMinusOne*l_zMinusOne;

 
  fprintf(stderr,
          "create_node_subset:%s hslab=(%d,%d,%d)(%d,%d,%d)(%d,%d,%d)\n",
	  a_sub_name, a_doList.x_start,a_doList.x_count,a_doList.x_skip,
	  a_doList.y_start,a_doList.y_count,a_doList.y_skip,
	  a_doList.z_start,a_doList.z_count,a_doList.z_skip );


  saf_declare_set(SAF_EACH, db, a_sub_name, 3, SAF_SPACE, SAF_EXTENDIBLE_FALSE,
		  &l_subBlockNodeSet );

  /*
  **    NOTE: because the node subsets have all 8 collections on them, just
  **    like the blocks themselves, one could create subset elems that span
  **    many of the elems in the top level block!
  */
  saf_declare_collection(SAF_EACH, l_subBlockNodeSet, nodes,
			 SAF_CELLTYPE_POINT, l_numNodes,
			 ZERO_3DF(a_doList.x_count,a_doList.y_count,
                                  a_doList.z_count),
			 SAF_DECOMP_FALSE);

  saf_declare_collection(SAF_EACH, l_subBlockNodeSet, edge_cat[0],
			 SAF_CELLTYPE_LINE, l_numXEdges,
			 ZERO_3DF(l_xMinusOne,l_y,l_z), SAF_DECOMP_FALSE);
  saf_declare_collection(SAF_EACH, l_subBlockNodeSet, edge_cat[1],
			 SAF_CELLTYPE_LINE, l_numYEdges,
			 ZERO_3DF(l_x,l_yMinusOne,l_z), SAF_DECOMP_FALSE);
  saf_declare_collection(SAF_EACH, l_subBlockNodeSet, edge_cat[2],
			 SAF_CELLTYPE_LINE, l_numZEdges,
			 ZERO_3DF(l_x,l_y,l_zMinusOne), SAF_DECOMP_FALSE);

  saf_declare_collection(SAF_EACH, l_subBlockNodeSet, face_cat[0],
			 SAF_CELLTYPE_QUAD, l_numXFaces,
			 ZERO_3DF(l_x,l_yMinusOne,l_zMinusOne),
                         SAF_DECOMP_FALSE);
  saf_declare_collection(SAF_EACH, l_subBlockNodeSet, face_cat[1],
			 SAF_CELLTYPE_QUAD, l_numYFaces,
			 ZERO_3DF(l_xMinusOne,l_y,l_zMinusOne), 
                         SAF_DECOMP_FALSE);
  saf_declare_collection(SAF_EACH, l_subBlockNodeSet, face_cat[2],
			 SAF_CELLTYPE_QUAD, l_numZFaces,
			 ZERO_3DF(l_xMinusOne,l_yMinusOne,l_z), 
                         SAF_DECOMP_FALSE);

  saf_declare_collection(SAF_EACH, l_subBlockNodeSet, elems,
			 SAF_CELLTYPE_HEX, l_numElems,
			 ZERO_3DF(l_xMinusOne,l_yMinusOne,l_zMinusOne), 
                         SAF_DECOMP_FALSE);


  saf_declare_subset_relation(SAF_EACH, a_blockNodeSet, l_subBlockNodeSet, 
			      SAF_COMMON(nodes), SAF_HSLAB, SAF_INT,
			      NULL, NULL, NULL, &rel);

  saf_write_subset_relation(SAF_EACH, rel, SAF_INT, (int *)&a_doList,
			    NULL, NULL, file);


#endif
  return(l_subBlockNodeSet);
  //## end SAF_IO::create_node_subset%3CFBB3BE01C2.body
}

SAF_Field SAF_IO::write_variable_using_existing_template (SAF_Set a_onSet, SAF_FieldTmpl a_varFieldTemplate, Real *a_varData, Real a_time, SAF_Cat a_cat)
{
  //## begin SAF_IO::write_variable_using_existing_template%3CFBB495006A.body preserve=yes
   SAF_Field l_varField=SAF_NOT_SET_FIELD;
#ifdef USE_SAF_IO
   SAF_Unit l_units;
   DSL_Type l_type = DSL_STRING;
   char l_str[256];
   char *l_strptr = 0;
   int l_maxTopoDim=0;
   SAF_Cat *l_highestCat;
   int l_isDegenerateSubset=0;

   char cuname[] = "Units";
   saf_get_field_tmpl_att(SAF_EACH,a_varFieldTemplate, cuname,&l_type,
                          NULL,(void **) &l_strptr);
   l_units = saf_find_one_unit(db, l_strptr);

   l_strptr = 0;
   saf_describe_field_tmpl(SAF_EACH,a_varFieldTemplate,&l_strptr,
                           NULL, NULL, NULL, NULL, NULL);

   sprintf(l_str,"%s -- t = %10.3e",l_strptr,a_time);

   if( ! processor_number() ) 
     printf("\n write_variable_using_existing_template %s\n",l_str);


   /*figure out whether the set is nodes,edges,faces or elems*/
   saf_describe_set(SAF_EACH,a_onSet,NULL,&l_maxTopoDim,
                    NULL,NULL,NULL,NULL,NULL );

   if( l_maxTopoDim==3 ) l_highestCat = &elems;
   else if( l_maxTopoDim==0 ) l_highestCat = &nodes;
   else
     {
       /* since the highest dim collection on this set is apparently a edge or
	  face collection, (i.e. this is an edge or face subset), we must
          assume it can no longer be mapped back to a node set (p.s.
          degenerate is probably not the right term to use */
       l_isDegenerateSubset=1;
     }

   if( l_isDegenerateSubset )
     saf_declare_field(SAF_EACH, a_varFieldTemplate, l_str,  a_onSet, l_units,
                       SAF_SELF(db), SAF_ZONAL(a_cat),  SAF_Real, NULL,
		       SAF_INTERLEAVE_NONE,SAF_IDENTITY, NULL, &l_varField);
   else
     saf_declare_field(SAF_EACH, a_varFieldTemplate, l_str,  a_onSet, l_units,
		       SAF_SELF(db), SAF_NODAL(a_cat,*l_highestCat),
		       SAF_Real, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
		       &l_varField);

 /*
   int l_slab[9];
  
   l_slab[0] = 1;
   l_slab[1] = 1;
   l_slab[2] = 3;
   l_slab[3] = 5;
   l_slab[4] = 9;
   l_slab[5] = 5;
   l_slab[6] = 1;
   l_slab[7] = 1;
   l_slab[8] = 1;
   saf_write_field(SAF_EACH, l_varField, -1 ,SAF_HSLAB, l_slab, 1,
 */
   saf_write_field(SAF_EACH, l_varField, SAF_WHOLE_FIELD, 1,
                   NULL, (void**)&a_varData, file);
#endif
   return l_varField;
  //## end SAF_IO::write_variable_using_existing_template%3CFBB495006A.body
}

void SAF_IO::write_timestep_data (Real a_timeValue)
{
  //## begin SAF_IO::write_timestep_data%3CFBB6CB0339.body preserve=yes
#ifdef USE_SAF_IO

  int i,j;
  SAF_Field *l_allProcFields=0;
  SAF_FieldTmpl *l_allProcFieldTemplates=0;
  int l_allProcNumFields=0;
  int l_rank=0, l_nprocs=1;

  l_rank = processor_number();
  l_nprocs = number_of_processors();

  // create_top_set();

  //If necessary, gather all field handles from all procs, so each proc is 
  //writing the same data
  if(l_nprocs>1) {
    l_allProcNumFields = field_count*l_nprocs;  
    l_allProcFields = new SAF_Field[l_allProcNumFields];

    for(j=0;j<field_count;j++){
      int l_numReturned=0;
      SAF_Field *l_fptr = 0;
      SAF_FieldTmpl *l_tptr = 0;
      saf_allgather_handles(db, (void *)(&saved_fields[j]), &l_numReturned, 
				(void **)&l_fptr );
      for(i=0;i<l_nprocs;i++) l_allProcFields[j*l_nprocs+i] = l_fptr[i];
    }
  }
  else {
    l_allProcNumFields=field_count;
    l_allProcFields=saved_fields;
  }

  if( num_fields_per_state != l_allProcNumFields ) {
    //not the first time through, and found an unexpected # of fields
    printf("error: for this version, must write the same # fields per timestep\n");
    exit(0);
  }


  //Write one state per timestep
  saf_write_state(SAF_ALL,state_group,state_count++,top_set,SAF_Real, 
                  &a_timeValue, l_allProcFields );

  if(l_nprocs > 1) delete [] l_allProcFields;

  /* Reset the list of fields for which to write timesteps. This step is
     required if you want to write timestep data (i.e. call this function)
     after each timestep */
  field_count=0;
#endif
  return;
  //## end SAF_IO::write_timestep_data%3CFBB6CB0339.body
}

void SAF_IO::create_top_set ()
{
  //## begin SAF_IO::create_top_set%3E1B5C75006D.body preserve=yes
  // Create a SAF_ALL top set, of which all blocks (from all procs)
  // are subsets. 
  //
  // This is to handle the case when the individual processes dont already 
  // know the structure of all blocks, they just know their own blocks. 
  //
  // This is necessary because SAF would like to have only one suite
  // when possible, and will not handle states&suites in SAF_EACH mode,
  // and because each suite needs an associated top mesh set.

  //The topset (and the suite) should only be created once.
  //This means that something must be rewritten here, if you 
  //want to add blocks later in the simulation. Among other things,
  //the set below would need to be SAF_EXTENDIBLE_TRUE, and the 
  //collection would need to be saf_extend_collection'ed each time.
#ifdef USE_SAF_IO
    if( !SAF_EQUIV(top_set,SAF_NOT_SET_SET) ) return;

  SAF_Cat l_cat=SAF_NOT_SET_CAT;
  int i, l_numBlocksAllProcs=0;

  int l_rank = processor_number();
  int l_nprocs = number_of_processors();
  int l_set_count = saf_blks.size();

  /*create the suiteset: a top-topset above all others, a 1-d trivial set*/
  saf_declare_set(SAF_ALL, db, "TOP_SET", 1, SAF_SPACE, SAF_EXTENDIBLE_FALSE,
                  &top_set);

  /*figure out how many blocks in the entire simulation*/
  l_numBlocksAllProcs = l_set_count*l_nprocs;

  saf_declare_category(db, "blocks",  SAF_BLOCK, 0, &l_cat);
  saf_declare_collection(SAF_ALL, top_set, l_cat, SAF_CELLTYPE_SET, 
                         l_numBlocksAllProcs,
			 _saf_indexspec(1, l_numBlocksAllProcs, 0,0), 
                         SAF_DECOMP_FALSE);

  /*relate each 'topset' to the suiteset as one block of many*/
  for(i=0;i<l_set_count;i++) {
    int l_whichEntry = i+l_rank*l_set_count;
    SAF_Rel l_rel;

    saf_declare_collection(SAF_EACH, saf_blks[i]->blockSet, l_cat, 
                           SAF_CELLTYPE_SET, 1, _saf_indexspec(1,1,0,0), 
                           SAF_DECOMP_FALSE);
    saf_declare_subset_relation(SAF_EACH, top_set, saf_blks[i]->blockSet, 
                                l_cat, l_cat, SAF_BOUNDARY_FALSE,
                                SAF_BOUNDARY_FALSE,SAF_TUPLES, SAF_INT, NULL, 
                                NULL, NULL, &l_rel);
    saf_write_subset_relation(SAF_EACH, l_rel, SAF_INT, &l_whichEntry, NULL,
                                NULL, file);
  }



  /*create a coords field that contains handles pointing to the coord fields
    of each block*/
  SAF_FieldTmpl l_coordsTmpl;
  SAF_Field l_coordsField;
  SAF_Field *l_indivCoordFields = new SAF_Field[l_set_count];
  SAF_Field *l_allFields = new SAF_Field[l_numBlocksAllProcs];

  saf_declare_field_tmpl(SAF_ALL, "coord_fields_tmpl", db, SAF_ALGTYPE_FIELD,
			 SAF_EMPTY_HANDLE, SAF_NOT_APPLICABLE_QUANTITY,
			 SAF_NOT_APPLICABLE_INT, NULL, &l_coordsTmpl );
    
  saf_declare_field(SAF_ALL, l_coordsTmpl, "coord_fields",top_set,
                    SAF_NOT_APPLICABLE_UNIT,l_cat,SAF_ZONAL(l_cat),
		    DSL_HANDLE,NULL,SAF_INTERLEAVE_VECTOR,SAF_IDENTITY,NULL,
                    &l_coordsField);

  saf_declare_coords(SAF_ALL,l_coordsField);
  saf_declare_default_coords(SAF_ALL,top_set,l_coordsField);    

  for(i=0;i<l_set_count;i++) {
    int l_numReturned=0;
    SAF_Field *l_returnedHandles=0;
    saf_find_default_coords(SAF_EACH, saf_blks[i]->blockSet, 
                            &l_indivCoordFields[i]);
    saf_allgather_handles(db, (void *)(&l_indivCoordFields[i]), 
                          &l_numReturned, (void **)(&l_returnedHandles));

    memcpy( &l_allFields[i*l_nprocs], l_returnedHandles, 
            l_nprocs*sizeof(SAF_Field) );
    free(l_returnedHandles);
  }

  saf_write_field(SAF_ALL, l_coordsField, SAF_WHOLE_FIELD, 1, SAF_HANDLE, 
                  (void **)(&l_allFields), file);

  delete [] l_allFields;
  delete [] l_indivCoordFields;
#endif
  //## end SAF_IO::create_top_set%3E1B5C75006D.body
}

void SAF_IO::create_suite ()
{
  //## begin SAF_IO::create_suite%3E1B5CCD034B.body preserve=yes
#ifdef USE_SAF_IO
  int i, j;
  int l_rank = processor_number();
  int l_nprocs = number_of_processors();
  int l_req_count = saf_req.size();

  int l_allProcNumFields = l_req_count*l_nprocs;  
  SAF_FieldTmpl* l_allProcFieldTemplates=new SAF_FieldTmpl[l_allProcNumFields];

  int k = 0;
  for(i=0;i<l_nprocs;i++) {
    for(j=0;j<l_req_count;j++) l_allProcFieldTemplates[k++] = saf_req[j]->tmpl;
  }

  if( SAF_EQUIV(suite,SAF_NOT_SET_SUITE) ) {
    SAF_StateTmpl l_stateTemplate;
    num_fields_per_state=l_allProcNumFields;
    saf_declare_suite(SAF_ALL, db, "SUITE", top_set, NULL, &suite);
    saf_declare_state_tmpl(SAF_ALL, "state_template", db, 
                           num_fields_per_state, l_allProcFieldTemplates, 
                           &l_stateTemplate);
    SAF_Unit l_unitsSeconds = saf_find_one_unit(db, "second");
    saf_declare_state_group(SAF_ALL, "state_group", suite, db, top_set, 
                            l_stateTemplate, SAF_QTIME, 
                            l_unitsSeconds, SAF_Real, &state_group);
  }
  // allocate saved_fields array
  saved_fields = new SAF_Field[l_req_count];
  printf("p%d: saved_fields allocated w/ %d\n",l_rank,l_req_count);
  delete [] l_allProcFieldTemplates;
#endif
  //## end SAF_IO::create_suite%3E1B5CCD034B.body
}

// Additional Declarations
  //## begin SAF_IO%3CC41527036B.declarations preserve=yes
#ifndef USE_SAF_IO
SAF_Set SAF_NOT_SET_SET = 0;
SAF_FieldTmpl SAF_NOT_SET_FIELDTMPL = 0;
SAF_Field SAF_NOT_SET_FIELD = 0;
SAF_Cat SAF_NOT_SET_CAT = 0;
#endif
  //## end SAF_IO%3CC41527036B.declarations
//## begin module%3CC4178203C8.epilog preserve=yes
//## end module%3CC4178203C8.epilog
