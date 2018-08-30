
/*
 * cl_Dof_Manager_Test.cpp
 *
 *  Created on: Jul 14, 2018
 *      Author: schmidt
 */
#ifdef MORIS_HAVE_PARALLEL
 #include "Epetra_MpiComm.h"
 #include <mpi.h>
#endif

#include "catch.hpp"
#include "fn_equal_to.hpp"
#include "typedefs.hpp"
#include "cl_Mat.hpp"
#include "cl_Communication_Tools.hpp"
#include "cl_Communication_Manager.hpp"

#define protected public
#define private   public
#include "cl_MSI_Equation_Object.hpp"
#include "cl_MSI_Node_Obj.hpp"
#include "cl_MSI_Model_Solver_Interface.hpp"
#include "cl_MSI_Dof_Manager.hpp"
#include "cl_MSI_Pdof_Host.hpp"
#undef protected
#undef private

namespace moris
{
    namespace MSI
    {
    TEST_CASE("Dof_Manager_Max_Pdof_Host","[MSI],[Dof_max_pdof_hosts]")
    {
        // Create node obj
        moris::uint tNodeId1 = 0;
        moris::uint tNodeId2 = 1;

        mtk::Vertex * Node1;
        mtk::Vertex * Node2;

        // Create generic adofs to this nodes pdof
        moris::Mat< moris::sint> tAdofs1( 2, 1 );
        moris::Mat< moris::sint> tAdofs2( 2, 1 );

        tAdofs1( 0, 0 ) = 0;
        tAdofs1( 1, 0 ) = 1;
        tAdofs2( 0, 0 ) = 0;
        tAdofs2( 1, 0 ) = 1;

        // Create generic T-matrices
        moris::Mat< moris::real> tMatrix1( 2, 1 );
        moris::Mat< moris::real> tMatrix2( 2, 1 );

        // Create generic T-matrices
        tMatrix1( 0, 0 ) = 1.0;
        tMatrix1( 1, 0 ) = 1.0;
        tMatrix2( 0, 0 ) = 1.0;
        tMatrix2( 1, 0 ) = -2.0;

        // Create generic adof owning processor
        moris::Mat< moris::uint> tAdofOwningProcessor1( 2, 1 );
        moris::Mat< moris::uint> tAdofOwningProcessor2( 2, 1 );

        tAdofOwningProcessor1( 0, 0 ) = 0;
        tAdofOwningProcessor1( 1, 0 ) = 0;
        tAdofOwningProcessor2( 0, 0 ) = 0;
        tAdofOwningProcessor2( 1, 0 ) = 0;

        // Create generic Node Object
        Node1 = new Node_Obj( tNodeId1, tAdofs1, tMatrix1, tAdofOwningProcessor1 );
        Node2 = new Node_Obj( tNodeId2, tAdofs2, tMatrix2, tAdofOwningProcessor2 );

        moris::uint tNumEquationObjects = 2;

        moris::uint tNumNodes = 2;

        moris::Cell < Equation_Object* >tListEqnObj( tNumEquationObjects, nullptr );

        // Create List with node pointern correponding to generic equation object
        moris::Cell< mtk::Vertex* > tNodeIds_1( tNumNodes );
        tNodeIds_1( 0 ) = Node1;
        tNodeIds_1( 1 ) = Node2;

        moris::Cell< mtk::Vertex* > tNodeIds_2( tNumNodes );
        tNodeIds_2( 0 ) = Node1;
        tNodeIds_2( 1 ) = Node2;

        // Create generic equation objects
        Equation_Object EquObj_1( tNodeIds_1 );
        Equation_Object EquObj_2( tNodeIds_2 );

        // Create List with equation objects
        tListEqnObj( 0 ) = & EquObj_1;
        tListEqnObj( 1 ) = & EquObj_2;

        moris::Mat< moris::uint > tCommTable( 1, 1, 0 );

        Dof_Manager tDofMgn;

        CHECK( equal_to( tDofMgn.initialize_max_number_of_possible_pdof_hosts( tListEqnObj ), 2 ) );

        delete tNodeIds_1(0);
        delete tNodeIds_1(1);
    }

    TEST_CASE("Dof_Manager_Pdof_Host_Time_Level","[MSI],[Dof_time_level]")
    {
        // Create dof maager
        Dof_Manager tDofMgn;

        // Set pdof type list
        tDofMgn.mPdofTypeList.resize( 2 );
        tDofMgn.mPdofTypeList( 0 ) = Dof_Type::TEMP;
        tDofMgn.mPdofTypeList( 1 ) = Dof_Type::UX;

        // create pdof host list and fill it with needed information
        tDofMgn.mPdofHostList.resize( 2 );
        tDofMgn.mPdofHostList( 0 ) = new Pdof_Host();
        tDofMgn.mPdofHostList( 1 ) = new Pdof_Host();

        tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType.resize( 2 );
        tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 ).resize( 2 );
        tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 ).resize( 3 );
        tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType.resize( 2 );
        tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 ).resize( 2 );

        (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 )) = new Pdof;
        (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 1 )) = new Pdof;
        (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 )) = new Pdof;
        (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 1 )) = new Pdof;
        (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 2 )) = new Pdof;
        (tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 )( 0 )) = new Pdof;
        (tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 )( 1 )) = new Pdof;

        // Call the function which shall be tested
        tDofMgn.initialize_pdof_host_time_level_list();

        // Check length and entries of the resulting vector
        CHECK( equal_to( tDofMgn.mPdofHostTimeLevelList.length(), 2 ) );

        CHECK( equal_to( tDofMgn.mPdofHostTimeLevelList( 0, 0 ), 2 ) );
        CHECK( equal_to( tDofMgn.mPdofHostTimeLevelList( 1, 0 ), 3 ) );
    }

    TEST_CASE("Dof_Mgn_ini_pdof_host_list","[MSI],[Dof_ini_pdof_host_list]")
    {
        // Create node obj
        moris::uint tNodeId1 = 0;
        moris::uint tNodeId2 = 1;

        mtk::Vertex * Node1;
        mtk::Vertex * Node2;

        // Create generic adofs to this nodes pdof
        moris::Mat< moris::sint> tAdofs1( 2, 1 );
        moris::Mat< moris::sint> tAdofs2( 2, 1 );

        tAdofs1( 0, 0 ) = 0;
        tAdofs1( 1, 0 ) = 1;
        tAdofs2( 0, 0 ) = 0;
        tAdofs2( 1, 0 ) = 1;

        // Create generic T-matrices
        moris::Mat< moris::real> tMatrix1( 2, 1 );
        moris::Mat< moris::real> tMatrix2( 2, 1 );

        // Create generic T-matrices
        tMatrix1( 0, 0 ) = 1.0;
        tMatrix1( 1, 0 ) = 1.0;
        tMatrix2( 0, 0 ) = 1.0;
        tMatrix2( 1, 0 ) = -2.0;

        // Create generic adof owning processor
        moris::Mat< moris::uint> tAdofOwningProcessor1( 2, 1 );
        moris::Mat< moris::uint> tAdofOwningProcessor2( 2, 1 );

        tAdofOwningProcessor1( 0, 0 ) = 0;
        tAdofOwningProcessor1( 1, 0 ) = 0;
        tAdofOwningProcessor2( 0, 0 ) = 0;
        tAdofOwningProcessor2( 1, 0 ) = 0;

        // Create generic Node Object
        Node1 = new Node_Obj( tNodeId1, tAdofs1, tMatrix1, tAdofOwningProcessor1 );
        Node2 = new Node_Obj( tNodeId2, tAdofs2, tMatrix2, tAdofOwningProcessor2 );

        moris::uint tNumEquationObjects = 2;

        moris::uint tNumNodes = 2;

        moris::Cell < Equation_Object* >tListEqnObj;
        tListEqnObj.resize( tNumEquationObjects, nullptr );

        // Create List with node pointern correponding to generic equation object
        moris::Cell< mtk::Vertex* > tNodeIds_1( tNumNodes );
        tNodeIds_1( 0 ) = Node1;
        tNodeIds_1( 1 ) = Node2;

        moris::Cell< mtk::Vertex* > tNodeIds_2( tNumNodes );
        tNodeIds_2( 0 ) = Node1;
        tNodeIds_2( 1 ) = Node2;

        // Create generic equation objects
        Equation_Object EquObj_1( tNodeIds_1 );
        Equation_Object EquObj_2( tNodeIds_2 );

        // Create List with equation objects
        tListEqnObj( 0 ) = & EquObj_1;
        tListEqnObj( 1 ) = & EquObj_2;

        Dof_Manager tDofMgn;

        tDofMgn.initialize_pdof_host_list( tListEqnObj );

        // Check size of pdof host list
        CHECK( equal_to( tDofMgn.mPdofHostList.size(), 2 ) );

        CHECK( equal_to( ((tDofMgn.mPdofHostList(0))->mNodeObj)->get_id(), 0 ) );
        CHECK( equal_to( ((tDofMgn.mPdofHostList(1))->mNodeObj)->get_id(), 1 ) );

        delete Node1;
        delete Node2;
    }

    TEST_CASE("Dof_Mgn_create_adofs","[MSI],[Dof_create_adofs]")
    {
        if( par_size() == 1 )
        {
            // Create node obj
            moris::uint tNodeId1 = 0;
            moris::uint tNodeId2 = 1;

            mtk::Vertex * Node1;
            mtk::Vertex * Node2;

            // Create generic adofs to this nodes pdof
            moris::Mat< moris::sint> tAdofs1( 2, 1 );
            moris::Mat< moris::sint> tAdofs2( 2, 1 );

            tAdofs1( 0, 0 ) = 0;
            tAdofs1( 1, 0 ) = 5;
            tAdofs2( 0, 0 ) = 3;
            tAdofs2( 1, 0 ) = 0;

            // Create generic T-matrices
            moris::Mat< moris::real> tMatrix1( 2, 1 );
            moris::Mat< moris::real> tMatrix2( 2, 1 );

            // Create generic T-matrices
            tMatrix1( 0, 0 ) = 1.0;
            tMatrix1( 1, 0 ) = -4.0;
            tMatrix2( 0, 0 ) = 2.0;
            tMatrix2( 1, 0 ) = -2.0;

            // Create generic adof owning processor
            moris::Mat< moris::uint> tAdofOwningProcessor1( 2, 1 );
            moris::Mat< moris::uint> tAdofOwningProcessor2( 2, 1 );

            tAdofOwningProcessor1( 0, 0 ) = 0;
            tAdofOwningProcessor1( 1, 0 ) = 0;
            tAdofOwningProcessor2( 0, 0 ) = 0;
            tAdofOwningProcessor2( 1, 0 ) = 0;

            // Create generic Node Object
            Node1 = new Node_Obj( tNodeId1, tAdofs1, tMatrix1, tAdofOwningProcessor1 );
            Node2 = new Node_Obj( tNodeId2, tAdofs2, tMatrix2, tAdofOwningProcessor2 );

            // Create dof manager and hardcode initial values
            Dof_Manager tDofMgn;

            tDofMgn.mPdofTypeList.resize( 2 );
            tDofMgn.mPdofTypeList( 0 ) = Dof_Type::TEMP;
            tDofMgn.mPdofTypeList( 1 ) = Dof_Type::UX;

            tDofMgn.mPdofHostList.resize( 2 );
            tDofMgn.mPdofHostList( 0 ) = new Pdof_Host( 2, Node1 );
            tDofMgn.mPdofHostList( 1 ) = new Pdof_Host( 2, Node2 );

            tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType.resize( 2 );
            tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 ).resize( 1 );
            tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 ).resize( 1 );
            tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType.resize( 2 );
            tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 ).resize( 1 );

            (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 )) = new Pdof;
            (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 )) = new Pdof;
            (tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 )( 0 )) = new Pdof;

            (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 ))->mDofTypeIndex = 0;
            (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 ))->mDofTypeIndex = 1;
            (tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 )( 0 ))->mDofTypeIndex = 0;

            tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 )->mAdofIds = tAdofs1;
            tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 )->mAdofIds = tAdofs1;
            tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 )( 0 )->mAdofIds = tAdofs2;
            // end hardcoding stuff

            // Create adofs and build adof lists
            tDofMgn.create_adofs();

            CHECK( equal_to( tDofMgn.mAdofList.size(), 5 ) );
            CHECK( equal_to( tDofMgn.mAdofList( 0 )->mAdofId, 0 ) );
            CHECK( equal_to( tDofMgn.mAdofList( 4 )->mAdofId, 4 ) );

            CHECK( equal_to( tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 )->mAdofIds( 0 ), 0 ) );
            CHECK( equal_to( tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 )->mAdofIds( 1 ), 2 ) );
            CHECK( equal_to( tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 )->mAdofIds( 0 ), 3 ) );
            CHECK( equal_to( tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 )->mAdofIds( 1 ), 4 ) );
            CHECK( equal_to( tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 )( 0 )->mAdofIds( 0 ), 1 ) );
            CHECK( equal_to( tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 )( 0 )->mAdofIds( 1 ), 0 ) );

//            CHECK( equal_to( tDofMgn.mPdofHostList( 0 )->mUniqueAdofList.length(), 4 ) );
//            CHECK( equal_to( tDofMgn.mPdofHostList( 1 )->mUniqueAdofList.length(), 2 ) );
//
//            CHECK( equal_to( tDofMgn.mPdofHostList( 0 )->mUniqueAdofList( 0, 0 ), 0 ) );
//            CHECK( equal_to( tDofMgn.mPdofHostList( 0 )->mUniqueAdofList( 1, 0 ), 2 ) );
//            CHECK( equal_to( tDofMgn.mPdofHostList( 0 )->mUniqueAdofList( 2, 0 ), 3 ) );
//            CHECK( equal_to( tDofMgn.mPdofHostList( 0 )->mUniqueAdofList( 3, 0 ), 4 ) );
//            CHECK( equal_to( tDofMgn.mPdofHostList( 1 )->mUniqueAdofList( 0, 0 ), 0 ) );
//            CHECK( equal_to( tDofMgn.mPdofHostList( 1 )->mUniqueAdofList( 1, 0 ), 1 ) );

            delete Node1;
            delete Node2;
        }
    }

    TEST_CASE("Dof_Mgn_Set_T_Matrix","[MSI],[Dof_set_t_matrix]")
    {
        // Create node obj
        moris::uint tNodeId1 = 0;
        moris::uint tNodeId2 = 1;

        mtk::Vertex * Node1;
        mtk::Vertex * Node2;

        // Create generic adofs to this nodes pdof
        moris::Mat< moris::sint> tAdofs1( 2, 1 );
        moris::Mat< moris::sint> tAdofs2( 2, 1 );

        tAdofs1( 0, 0 ) = 0;
        tAdofs1( 1, 0 ) = 5;
        tAdofs2( 0, 0 ) = 3;
        tAdofs2( 1, 0 ) = 0;

        // Create generic T-matrices
        moris::Mat< moris::real> tMatrix1( 2, 1 );
        moris::Mat< moris::real> tMatrix2( 2, 1 );

        // Create generic T-matrices
        tMatrix1( 0, 0 ) = 1.0;
        tMatrix1( 1, 0 ) = -4.0;
        tMatrix2( 0, 0 ) = 2.0;
        tMatrix2( 1, 0 ) = -2.0;

        // Create generic adof owning processor
        moris::Mat< moris::uint> tAdofOwningProcessor1( 2, 1 );
        moris::Mat< moris::uint> tAdofOwningProcessor2( 2, 1 );

        tAdofOwningProcessor1( 0, 0 ) = 0;
        tAdofOwningProcessor1( 1, 0 ) = 0;
        tAdofOwningProcessor2( 0, 0 ) = 0;
        tAdofOwningProcessor2( 1, 0 ) = 0;

        // Create generic Node Object
        Node1 = new Node_Obj( tNodeId1, tAdofs1, tMatrix1, tAdofOwningProcessor1 );
        Node2 = new Node_Obj( tNodeId2, tAdofs2, tMatrix2, tAdofOwningProcessor2 );

        // Create dof manager and hardcode initial values
        Dof_Manager tDofMgn;

        tDofMgn.mPdofHostList.resize( 2 );
        tDofMgn.mPdofHostList( 0 ) = new Pdof_Host( 2, Node1 );
        tDofMgn.mPdofHostList( 1 ) = new Pdof_Host( 2, Node2 );

        tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType.resize( 2 );
        tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 ).resize( 1 );
        tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 ).resize( 1 );
        tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType.resize( 1 );
        tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 ).resize( 1 );

        (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 )) = new Pdof;
        (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 )) = new Pdof;
        (tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 )( 0 )) = new Pdof;
        // end hardcoding stuff

        // Create adofs and build adof lists
        tDofMgn.set_pdof_t_matrix();

        CHECK( equal_to( (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 )->mTmatrix)( 0, 0 ),  1 ) );
        CHECK( equal_to( (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 )->mTmatrix)( 1, 0 ), -4 ) );
        CHECK( equal_to( (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 )->mTmatrix)( 0, 0 ),  1 ) );
        CHECK( equal_to( (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 )->mTmatrix)( 1, 0 ), -4 ) );
        CHECK( equal_to( (tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 )( 0 )->mTmatrix)( 0, 0 ),  2 ) );
        CHECK( equal_to( (tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 )( 0 )->mTmatrix)( 1, 0 ), -2 ) );

        delete Node1;
        delete Node2;
    }

    TEST_CASE("Dof_Mgn_create_unique_dof_type_list","[MSI],[Dof_create_dof_type_list][MSI_parallel]")
     {
         // Create generic equation objects
         Equation_Object EquObj_1;
         Equation_Object EquObj_2;

         moris::Cell < Equation_Object* >tListEqnObj;

         // Determine process rank
         size_t tRank = par_rank();
         size_t tSize = par_size();

         // Hardcode input test values
         switch( tRank )
             {
             case 0:
                 EquObj_1.mEqnObjDofTypeList.resize( 1 );
                 EquObj_2.mEqnObjDofTypeList.resize( 2 );
                 EquObj_1.mEqnObjDofTypeList( 0 ) = Dof_Type::TEMP;
                 EquObj_2.mEqnObjDofTypeList( 0 ) = Dof_Type::UX;
                 EquObj_2.mEqnObjDofTypeList( 1 ) = Dof_Type::UZ;
                 tListEqnObj.resize( 2, nullptr );
                 tListEqnObj( 0 ) = & EquObj_1;
                 tListEqnObj( 1 ) = & EquObj_2;
               break;
             case 1:
                 EquObj_1.mEqnObjDofTypeList.resize( 2 );
                 EquObj_2.mEqnObjDofTypeList.resize( 2 );
                 EquObj_1.mEqnObjDofTypeList( 0 ) = Dof_Type::TEMP;
                 EquObj_1.mEqnObjDofTypeList( 1 ) = Dof_Type::UX;
                 EquObj_2.mEqnObjDofTypeList( 0 ) = Dof_Type::UX;
                 EquObj_2.mEqnObjDofTypeList( 1 ) = Dof_Type::UZ;
                 tListEqnObj.resize( 2, nullptr );
                 tListEqnObj( 0 ) = & EquObj_1;
                 tListEqnObj( 1 ) = & EquObj_2;
               break;
             case 2:
                 EquObj_1.mEqnObjDofTypeList.resize( 3 );
                 EquObj_1.mEqnObjDofTypeList( 0 ) = Dof_Type::UX;
                 EquObj_1.mEqnObjDofTypeList( 1 ) = Dof_Type::TEMP;
                 EquObj_1.mEqnObjDofTypeList( 2 ) = Dof_Type::UZ;
                 tListEqnObj.resize( 1, nullptr );
                 tListEqnObj( 0 ) = & EquObj_1;
               break;
             case 3:
                 EquObj_1.mEqnObjDofTypeList.resize( 1 );
                 EquObj_2.mEqnObjDofTypeList.resize( 2 );
                 EquObj_1.mEqnObjDofTypeList( 0 ) = Dof_Type::TEMP;
                 EquObj_2.mEqnObjDofTypeList( 0 ) = Dof_Type::UX;
                 EquObj_2.mEqnObjDofTypeList( 1 ) = Dof_Type::UZ;
                 tListEqnObj.resize( 2, nullptr );
                 tListEqnObj( 0 ) = & EquObj_1;
                 tListEqnObj( 1 ) = & EquObj_2;
               break;
              }

         // Create dof manager
         Dof_Manager tDofMgn;

         // Call initialize pdof type list function
         tDofMgn.initialize_pdof_type_list( tListEqnObj );

         // Check pdof type list
         CHECK( equal_to( static_cast<int>( tDofMgn.mPdofTypeList( 0 ) ), 0 ) );
         CHECK( equal_to( static_cast<int>( tDofMgn.mPdofTypeList( 1 ) ), 2 ) );
         CHECK( equal_to( static_cast<int>( tDofMgn.mPdofTypeList( 2 ) ), 3 ) );

     }

    TEST_CASE("Dof_Mgn_create_unique_dof_type_map_matrix","[MSI],[Dof_create_dof_type_map][MSI_parallel]")
    {
        // Create generic equation objects
        Equation_Object EquObj_1;
        Equation_Object EquObj_2;

        moris::Cell < Equation_Object* >tListEqnObj;

        // Determine process rank
        size_t tRank = par_rank();
        size_t tSize = par_size();

        // Hardcode input test values
        switch( tRank )
            {
            case 0:
                EquObj_1.mEqnObjDofTypeList.resize( 1 );
                EquObj_2.mEqnObjDofTypeList.resize( 2 );
                EquObj_1.mEqnObjDofTypeList( 0 ) = Dof_Type::TEMP;
                EquObj_2.mEqnObjDofTypeList( 0 ) = Dof_Type::UX;
                EquObj_2.mEqnObjDofTypeList( 1 ) = Dof_Type::UZ;
                tListEqnObj.resize( 2, nullptr );
                tListEqnObj( 0 ) = & EquObj_1;
                tListEqnObj( 1 ) = & EquObj_2;
              break;
            case 1:
                EquObj_1.mEqnObjDofTypeList.resize( 2 );
                EquObj_2.mEqnObjDofTypeList.resize( 2 );
                EquObj_1.mEqnObjDofTypeList( 0 ) = Dof_Type::TEMP;
                EquObj_1.mEqnObjDofTypeList( 1 ) = Dof_Type::UX;
                EquObj_2.mEqnObjDofTypeList( 0 ) = Dof_Type::UX;
                EquObj_2.mEqnObjDofTypeList( 1 ) = Dof_Type::UZ;
                tListEqnObj.resize( 2, nullptr );
                tListEqnObj( 0 ) = & EquObj_1;
                tListEqnObj( 1 ) = & EquObj_2;
              break;
            case 2:
                EquObj_1.mEqnObjDofTypeList.resize( 3 );
                EquObj_1.mEqnObjDofTypeList( 0 ) = Dof_Type::UX;
                EquObj_1.mEqnObjDofTypeList( 1 ) = Dof_Type::TEMP;
                EquObj_1.mEqnObjDofTypeList( 2 ) = Dof_Type::UZ;
                tListEqnObj.resize( 1, nullptr );
                tListEqnObj( 0 ) = & EquObj_1;
              break;
            case 3:
                EquObj_1.mEqnObjDofTypeList.resize( 1 );
                EquObj_2.mEqnObjDofTypeList.resize( 2 );
                EquObj_1.mEqnObjDofTypeList( 0 ) = Dof_Type::TEMP;
                EquObj_2.mEqnObjDofTypeList( 0 ) = Dof_Type::UX;
                EquObj_2.mEqnObjDofTypeList( 1 ) = Dof_Type::UZ;
                tListEqnObj.resize( 2, nullptr );
                tListEqnObj( 0 ) = & EquObj_1;
                tListEqnObj( 1 ) = & EquObj_2;
              break;
             }

        // Create dof manager
        Dof_Manager tDofMgn;

        // Call initialize pdof type list function
        tDofMgn.initialize_pdof_type_list( tListEqnObj );

        // Call initialize pdof type list function
        tDofMgn.create_dof_type_map();

        // Check pdof type list
        CHECK( equal_to( tDofMgn.mPdofTypeMap( 0, 0 ), 0 ) );
        CHECK( equal_to( tDofMgn.mPdofTypeMap( 1, 0 ), -1 ) );
        CHECK( equal_to( tDofMgn.mPdofTypeMap( 2, 0 ), 1 ) );
        CHECK( equal_to( tDofMgn.mPdofTypeMap( 3, 0 ), 2 ) );
    }

    TEST_CASE("Dof_Mgn_create_adofs_parallell_1","[MSI],[Dof_create_adofs_parallel_1][MSI_parallel]")
    {
        size_t tSize = par_size();

        if( tSize == 2 )
        {
            // Create node obj
            moris::uint tNodeId1 = 0;
            moris::uint tNodeId2 = 1;
            moris::uint tNodeId3 = 2;

            mtk::Vertex * Node1;
            mtk::Vertex * Node2;

            // Create generic adofs to this nodes pdof
            moris::Mat< moris::sint> tAdofs1( 2, 1 );
            moris::Mat< moris::sint> tAdofs2( 2, 1 );

            // Create generic T-matrices
            moris::Mat< moris::real> tMatrix1( 2, 1 );
            moris::Mat< moris::real> tMatrix2( 2, 1 );

            // Create generic adof owning processor
            moris::Mat< moris::uint> tAdofOwningProcessor1( 2, 1 );
            moris::Mat< moris::uint> tAdofOwningProcessor2( 2, 1 );

            // Determine process rank
            size_t tRank = par_rank();
            size_t tSize = par_size();

            // Hardcode input test values
            switch( tRank )
            {
            case 0:
                tAdofs1( 0, 0 ) = 0;
                tAdofs1( 1, 0 ) = 5;
                tAdofs2( 0, 0 ) = 3;
                tAdofs2( 1, 0 ) = 0;

                tMatrix1( 0, 0 ) = 1.0;
                tMatrix1( 1, 0 ) = -4.0;
                tMatrix2( 0, 0 ) = 2.0;
                tMatrix2( 1, 0 ) = -2.0;

                tAdofOwningProcessor1( 0, 0 ) = 0;
                tAdofOwningProcessor1( 1, 0 ) = 0;
                tAdofOwningProcessor2( 0, 0 ) = 1;
                tAdofOwningProcessor2( 1, 0 ) = 0;

                // Create generic Node Object
                Node1 = new Node_Obj( tNodeId1, tAdofs1, tMatrix1, tAdofOwningProcessor1 );
                Node2 = new Node_Obj( tNodeId2, tAdofs2, tMatrix2, tAdofOwningProcessor2 );
              break;
            case 1:
                tAdofs1( 0, 0 ) = 3;
                tAdofs1( 1, 0 ) = 5;

                tMatrix1( 0, 0 ) = 1.0;
                tMatrix1( 1, 0 ) = 3.0;

                tAdofOwningProcessor1( 0, 0 ) = 1;
                tAdofOwningProcessor1( 1, 0 ) = 0;

                // Create generic Node Object
                Node1 = new Node_Obj( tNodeId1, tAdofs1, tMatrix1, tAdofOwningProcessor1 );
                Node2 = new Node_Obj( tNodeId1, tAdofs1, tMatrix1, tAdofOwningProcessor1 );
              break;
            }

            // Create dof manager and hardcode initial values
            Dof_Manager tDofMgn;

            tDofMgn.mPdofTypeList.resize( 2 );
            tDofMgn.mPdofTypeList( 0 ) = Dof_Type::TEMP;
            tDofMgn.mPdofTypeList( 1 ) = Dof_Type::UX;

            switch( tRank )
            {
            case 0:
                tDofMgn.mPdofHostList.resize( 2 );
                tDofMgn.mPdofHostList( 0 ) = new Pdof_Host( 2, Node1 );
                tDofMgn.mPdofHostList( 1 ) = new Pdof_Host( 2, Node2 );

                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType.resize( 2 );
                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 ).resize( 1 );
                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 ).resize( 1 );
                tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType.resize( 2 );
                tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 ).resize( 1 );

                (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 )) = new Pdof;
                (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 )) = new Pdof;
                (tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 )( 0 )) = new Pdof;

                (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 ))->mDofTypeIndex = 0;
                (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 ))->mDofTypeIndex = 1;
                (tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 )( 0 ))->mDofTypeIndex = 0;

                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 )->mAdofIds = tAdofs1;
                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 )->mAdofIds = tAdofs1;
                tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 )( 0 )->mAdofIds = tAdofs2;

                tDofMgn.mCommTable.set_size( 2, 1, 0);
                tDofMgn.mCommTable( 1, 0 ) = 1;

              break;
            case 1:
                tDofMgn.mPdofHostList.resize( 1 );
                tDofMgn.mPdofHostList( 0 ) = new Pdof_Host( 2, Node1 );

                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType.resize( 2 );
                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 ).resize( 1 );
                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 ).resize( 1 );

                (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 )) = new Pdof;
                (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 )) = new Pdof;

                (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 ))->mDofTypeIndex = 0;
                (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 ))->mDofTypeIndex = 1;

                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 )->mAdofIds = tAdofs1;
                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 )->mAdofIds = tAdofs1;

                tDofMgn.mCommTable.set_size( 2, 1, 1);
                tDofMgn.mCommTable( 1, 0 ) = 0;

              break;
            }
            // end hardcoding stuff

            // Create adofs and build adof lists
            tDofMgn.create_adofs();

            if ( par_rank() == 0 )
            {
                CHECK( equal_to( tDofMgn.mAdofList.size(), 5 ) );
                CHECK( equal_to( tDofMgn.mAdofList( 0 )->mAdofId, 0 ) );
                CHECK( equal_to( tDofMgn.mAdofList( 1 )->mAdofId, 4 ) );
                CHECK( equal_to( tDofMgn.mAdofList( 2 )->mAdofId, 1 ) );
                CHECK( equal_to( tDofMgn.mAdofList( 3 )->mAdofId, 2 ) );
                CHECK( equal_to( tDofMgn.mAdofList( 4 )->mAdofId, 3 ) );
            }
            if ( par_rank() == 1 )
            {
                CHECK( equal_to( tDofMgn.mAdofList.size(), 4 ) );
                CHECK( equal_to( tDofMgn.mAdofList( 0 )->mAdofId, 4 ) );
                CHECK( equal_to( tDofMgn.mAdofList( 1 )->mAdofId, 1 ) );
                CHECK( equal_to( tDofMgn.mAdofList( 2 )->mAdofId, 5 ) );
                CHECK( equal_to( tDofMgn.mAdofList( 3 )->mAdofId, 3 ) );
            }
            delete Node1;
            delete Node2;
        }
    }


    TEST_CASE("Dof_Mgn_create_adofs_parallell_2","[MSI],[Dof_create_adofs_parallel_2][MSI_parallel]")
    {
        size_t tSize = par_size();
        if ( tSize == 2 )
        {
            // Create node obj
            moris::uint tNodeId1 = 0;
            moris::uint tNodeId2 = 1;
            moris::uint tNodeId3 = 2;

            mtk::Vertex * Node1;
            mtk::Vertex * Node2;

            // Create generic adofs to this nodes pdof
            moris::Mat< moris::sint> tAdofs1( 2, 1 );
            moris::Mat< moris::sint> tAdofs2( 2, 1 );

            // Create generic T-matrices
            moris::Mat< moris::real> tMatrix1( 2, 1 );
            moris::Mat< moris::real> tMatrix2( 2, 1 );

            // Create generic adof owning processor
            moris::Mat< moris::uint> tAdofOwningProcessor1( 2, 1 );
            moris::Mat< moris::uint> tAdofOwningProcessor2( 2, 1 );

            // Determine process rank
            size_t tRank = par_rank();
            size_t tSize = par_size();

            // Hardcode input test values
            switch( tRank )
            {
            case 0:
                tAdofs1( 0, 0 ) = 0;
                tAdofs1( 1, 0 ) = 5;
                tAdofs2( 0, 0 ) = 4;
                tAdofs2( 1, 0 ) = 0;

                tMatrix1( 0, 0 ) = 1.0;
                tMatrix1( 1, 0 ) = -4.0;
                tMatrix2( 0, 0 ) = 2.0;
                tMatrix2( 1, 0 ) = -2.0;

                tAdofOwningProcessor1( 0, 0 ) = 0;
                tAdofOwningProcessor1( 1, 0 ) = 0;
                tAdofOwningProcessor2( 0, 0 ) = 1;
                tAdofOwningProcessor2( 1, 0 ) = 0;

                // Create generic Node Object
                Node1 = new Node_Obj( tNodeId1, tAdofs1, tMatrix1, tAdofOwningProcessor1 );
                Node2 = new Node_Obj( tNodeId2, tAdofs2, tMatrix2, tAdofOwningProcessor2 );
              break;
            case 1:
                tAdofs1( 0, 0 ) = 3;
                tAdofs1( 1, 0 ) = 5;

                tMatrix1( 0, 0 ) = 1.0;
                tMatrix1( 1, 0 ) = 3.0;

                tAdofOwningProcessor1( 0, 0 ) = 1;
                tAdofOwningProcessor1( 1, 0 ) = 0;

                // Create generic Node Object
                Node1 = new Node_Obj( tNodeId1, tAdofs1, tMatrix1, tAdofOwningProcessor1 );
                Node2 = new Node_Obj( tNodeId1, tAdofs1, tMatrix1, tAdofOwningProcessor1 );
              break;
            }

            // Create dof manager and hardcode initial values
            Dof_Manager tDofMgn;

            tDofMgn.mPdofTypeList.resize( 2 );
            tDofMgn.mPdofTypeList( 0 ) = Dof_Type::TEMP;
            tDofMgn.mPdofTypeList( 1 ) = Dof_Type::UX;

            switch( tRank )
            {
            case 0:
                tDofMgn.mPdofHostList.resize( 2 );
                tDofMgn.mPdofHostList( 0 ) = new Pdof_Host( 2, Node1 );
                tDofMgn.mPdofHostList( 1 ) = new Pdof_Host( 2, Node2 );

                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType.resize( 2 );
                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 ).resize( 1 );
                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 ).resize( 1 );
                tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType.resize( 2 );
                tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 ).resize( 1 );

                (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 )) = new Pdof;
                (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 )) = new Pdof;
                (tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 )( 0 )) = new Pdof;

                (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 ))->mDofTypeIndex = 0;
                (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 ))->mDofTypeIndex = 1;
                (tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 )( 0 ))->mDofTypeIndex = 0;

                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 )->mAdofIds = tAdofs1;
                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 )->mAdofIds = tAdofs1;
                tDofMgn.mPdofHostList( 1 )->mListOfPdofTimePerType( 0 )( 0 )->mAdofIds = tAdofs2;

                tDofMgn.mCommTable.set_size( 2, 1, 0);
                tDofMgn.mCommTable( 1, 0 ) = 1;

              break;
            case 1:
                tDofMgn.mPdofHostList.resize( 1 );
                tDofMgn.mPdofHostList( 0 ) = new Pdof_Host( 2, Node1 );

                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType.resize( 2 );
                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 ).resize( 1 );
                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 ).resize( 1 );

                (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 )) = new Pdof;
                (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 )) = new Pdof;

                (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 ))->mDofTypeIndex = 0;
                (tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 ))->mDofTypeIndex = 1;

                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 0 )( 0 )->mAdofIds = tAdofs1;
                tDofMgn.mPdofHostList( 0 )->mListOfPdofTimePerType( 1 )( 0 )->mAdofIds = tAdofs1;

                tDofMgn.mCommTable.set_size( 2, 1, 1);
                tDofMgn.mCommTable( 1, 0 ) = 0;

              break;
            }
            // end hardcoding stuff

            // Create adofs and build adof lists
            tDofMgn.create_adofs();

            if ( par_rank() == 0 )
            {
                CHECK( equal_to( tDofMgn.mAdofList.size(), 5 ) );
                CHECK( equal_to( tDofMgn.mAdofList( 0 )->mAdofId, 0 ) );
                CHECK( equal_to( tDofMgn.mAdofList( 1 )->mAdofId, 5 ) );
                CHECK( equal_to( tDofMgn.mAdofList( 2 )->mAdofId, 1 ) );
                CHECK( equal_to( tDofMgn.mAdofList( 3 )->mAdofId, 2 ) );
                CHECK( equal_to( tDofMgn.mAdofList( 4 )->mAdofId, 3 ) );
            }
            if ( par_rank() == 1 )
            {
                CHECK( equal_to( tDofMgn.mAdofList.size(), 5 ) );
                CHECK( equal_to( tDofMgn.mAdofList( 0 )->mAdofId, 4 ) );
                CHECK( equal_to( tDofMgn.mAdofList( 1 )->mAdofId, 5 ) );
                CHECK( equal_to( tDofMgn.mAdofList( 2 )->mAdofId, 1 ) );
                CHECK( equal_to( tDofMgn.mAdofList( 3 )->mAdofId, 6 ) );
                CHECK( equal_to( tDofMgn.mAdofList( 4 )->mAdofId, 3 ) );
            }
            delete Node1;
            delete Node2;
        }
    }

    }
}

