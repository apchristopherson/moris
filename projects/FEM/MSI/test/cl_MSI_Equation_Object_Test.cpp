/*
 * cl_Equation_Object_Test.cpp
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
    TEST_CASE("Eqn_Obj","[MSI],[Eqn_Obj]")
    {
        // Create node obj
        moris::uint tNodeId1 = 0;
        moris::uint tNodeId2 = 1;

        mtk::Vertex * Node1;
        mtk::Vertex * Node2;

        //---------------------------------------------------------------------------------
        // Create generic adofs to this nodes pdof
        moris::Mat< moris::sint> tAdofsList1( 2, 1 );
        moris::Mat< moris::sint> tAdofsList2( 2, 1 );

        tAdofsList1( 0, 0 ) = 0;
        tAdofsList1( 1, 0 ) = 1;
        tAdofsList2( 0, 0 ) = 0;
        tAdofsList2( 1, 0 ) = 1;

        //---------------------------------------------------------------------------------
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
        Node1 = new Node_Obj( tNodeId1, tAdofsList1, tMatrix1, tAdofOwningProcessor1 );
        Node2 = new Node_Obj( tNodeId2, tAdofsList2, tMatrix2, tAdofOwningProcessor2 );
        //---------------------------------------------------------------------------------

        moris::uint tNumNodes = 2;

        // Create List with node pointern correponding to generic equation object
        moris::Cell< mtk::Vertex* > tNodeIds_1( tNumNodes );
        tNodeIds_1( 0 ) = Node1;
        tNodeIds_1( 1 ) = Node2;

        // Create generic equation objects
        Equation_Object EquObj( tNodeIds_1 );

        // Check Node Ids of this equation object
        CHECK( equal_to( ( EquObj.mNodeObj( 0 ) )->get_id(), 0 ) );
        CHECK( equal_to( ( EquObj.mNodeObj( 1 ) )->get_id(), 1 ) );

        // Check number of possible pdof hosts of this equation object
        CHECK( equal_to( EquObj.get_num_pdof_hosts(), 2 ));
        delete Node1;
        delete Node2;
    }

    TEST_CASE("Eqn_Obj_create_pdof_host","[MSI],[Eqn_Obj_create_pdof_host]")
    {
        // Create node obj
        moris::uint tNodeId1 = 0;
        moris::uint tNodeId2 = 2;

        mtk::Vertex * Node1;
        mtk::Vertex * Node2;

        //---------------------------------------------------------------------------------
        // Create generic adofs to this nodes pdof
        moris::Mat< moris::sint> tAdofsList1( 2, 1 );
        moris::Mat< moris::sint> tAdofsList2( 2, 1 );

        tAdofsList1( 0, 0 ) = 0;
        tAdofsList1( 1, 0 ) = 1;
        tAdofsList2( 0, 0 ) = 0;
        tAdofsList2( 1, 0 ) = 1;

        //---------------------------------------------------------------------------------
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
        Node1 = new Node_Obj( tNodeId1, tAdofsList1, tMatrix1, tAdofOwningProcessor1 );
        Node2 = new Node_Obj( tNodeId2, tAdofsList2, tMatrix2, tAdofOwningProcessor2 );
        //---------------------------------------------------------------------------------

        moris::uint tNumNodes = 2;

        // Create List with node pointern correponding to generic equation object
        moris::Cell< mtk::Vertex* > tNodeIds_1( tNumNodes );
        tNodeIds_1( 0 ) = Node1;
        tNodeIds_1( 1 ) = Node2;

        // Create generic equation objects
        Equation_Object EquObj( tNodeIds_1 );

        // Create pdof hosts of this equation object
        moris::Cell < Pdof_Host * > tPdofHostList;
        tPdofHostList.resize( 3, nullptr );
        moris::uint tNumMaxPdofTypes = 1;

        moris::Mat< moris::sint> tDofTypeIndexMap(4, 1, -1);
        tDofTypeIndexMap(3, 0) = 0;

        EquObj.create_my_pdof_hosts( tNumMaxPdofTypes, tDofTypeIndexMap, tPdofHostList );

        // Check if right pdof host was created in given pdof host list
        CHECK( equal_to( tPdofHostList( 0 )->mNodeID, 0 ) );
        REQUIRE( tPdofHostList( 1 ) == NULL );
        CHECK( equal_to( tPdofHostList( 2 )->mNodeID, 2 ) );

        // Check equation objects internal pdof host list
        CHECK( equal_to( EquObj.mMyPdofHosts.size(), 2 ) );
        CHECK( equal_to( EquObj.mMyPdofHosts( 0 )->mNodeID, 0 ) );
        CHECK( equal_to( EquObj.mMyPdofHosts( 1 )->mNodeID, 2 ) );
        delete Node1;
        delete Node2;
        delete tPdofHostList(0);
        delete tPdofHostList(1);
        delete tPdofHostList(2);
    }

    TEST_CASE("Eqn_Obj_create_my_pdof_list","[MSI],[Eqn_Obj_create_my_pdof_list]")
    {
        // Create node obj
        moris::uint tNodeId1 = 0;
        moris::uint tNodeId2 = 2;

        mtk::Vertex * Node1;
        mtk::Vertex * Node2;

        //---------------------------------------------------------------------------------
        // Create generic adofs to this nodes pdof
        moris::Mat< moris::sint> tAdofsList1( 2, 1 );
        moris::Mat< moris::sint> tAdofsList2( 2, 1 );

        tAdofsList1( 0, 0 ) = 0;
        tAdofsList1( 1, 0 ) = 1;
        tAdofsList2( 0, 0 ) = 0;
        tAdofsList2( 1, 0 ) = 1;

        //---------------------------------------------------------------------------------
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
        Node1 = new Node_Obj( tNodeId1, tAdofsList1, tMatrix1, tAdofOwningProcessor1 );
        Node2 = new Node_Obj( tNodeId2, tAdofsList2, tMatrix2, tAdofOwningProcessor2 );
        //---------------------------------------------------------------------------------

        moris::uint tNumNodes = 2;

        // Create List with node pointern correponding to generic equation object
        moris::Cell< mtk::Vertex* > tNodeIds_1( tNumNodes );
        tNodeIds_1( 0 ) = Node1;
        tNodeIds_1( 1 ) = Node2;

        // Create generic equation objects
        Equation_Object EquObj( tNodeIds_1 );

        // Create pdof hosts of this equation object
        moris::Cell < Pdof_Host * > tPdofHostList;
        tPdofHostList.resize( 3, nullptr );
        moris::uint tNumMaxPdofTypes = 2;

        moris::Mat< moris::sint> tDofTypeIndexMap(4, 1, -1);
        tDofTypeIndexMap(0, 0) = 0;
        tDofTypeIndexMap(3, 0) = 1;

        EquObj.create_my_pdof_hosts( tNumMaxPdofTypes, tDofTypeIndexMap, tPdofHostList);

        // resize pdof host list. Shortcut. Functionality is tested in another test
        tPdofHostList( 0 )->mListOfPdofTimePerType.resize( 1 );
        tPdofHostList( 0 )->mListOfPdofTimePerType( 0 ).resize( 1 );
        tPdofHostList( 2 )->mListOfPdofTimePerType.resize( 1 );
        tPdofHostList( 2 )->mListOfPdofTimePerType( 0 ).resize( 1 );

        // Create my pdof list
        EquObj.create_my_pdof_list();

        // Check if right pdof host was created in given pdof host list
        CHECK( equal_to( EquObj.mFreePdofs.size(), 2 ) );
        delete Node1;
        delete Node2;
        delete tPdofHostList(0);
        delete tPdofHostList(1);
        delete tPdofHostList(2);

        // FIXME extend this test
    }

    TEST_CASE("Eqn_Obj_create_my_pdof_list_2","[MSI],[Eqn_Obj_create_my_pdof_list_2]")
    {
        // Create node obj
        moris::uint tNodeId1 = 0;
        moris::uint tNodeId2 = 2;

        mtk::Vertex * Node1;
        mtk::Vertex * Node2;

        //---------------------------------------------------------------------------------
        // Create generic adofs to this nodes pdof
        moris::Mat< moris::sint> tAdofsList1( 2, 1 );
        moris::Mat< moris::sint> tAdofsList2( 2, 1 );

        tAdofsList1( 0, 0 ) = 0;
        tAdofsList1( 1, 0 ) = 1;
        tAdofsList2( 0, 0 ) = 0;
        tAdofsList2( 1, 0 ) = 1;

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
        Node1 = new Node_Obj( tNodeId1, tAdofsList1, tMatrix1, tAdofOwningProcessor1 );
        Node2 = new Node_Obj( tNodeId2, tAdofsList2, tMatrix2, tAdofOwningProcessor2 );


        moris::uint tNumNodes = 2;

        // Create List with node pointern correponding to generic equation object
        moris::Cell< mtk::Vertex* > tNodeIds_1( tNumNodes );
        tNodeIds_1( 0 ) = Node1;
        tNodeIds_1( 1 ) = Node2;

        // Create generic equation objects
        Equation_Object EquObj( tNodeIds_1 );

        // Create pdof hosts of this equation object
        moris::Cell < Pdof_Host * > tPdofHostList;
        tPdofHostList.resize( 3, nullptr );
        moris::uint tNumMaxPdofTypes = 2;

        moris::Mat< moris::sint> tDofTypeIndexMap(4, 1, -1);
        tDofTypeIndexMap(3, 0) = 0;

        EquObj.create_my_pdof_hosts( tNumMaxPdofTypes, tDofTypeIndexMap, tPdofHostList );

        // resize pdof host list. Shortcut. Functionality is tested in another test
        tPdofHostList( 0 )->mListOfPdofTimePerType.resize( 1 );
        tPdofHostList( 0 )->mListOfPdofTimePerType( 0 ).resize( 1 );
        tPdofHostList( 2 )->mListOfPdofTimePerType.resize( 1 );
        tPdofHostList( 2 )->mListOfPdofTimePerType( 0 ).resize( 1 );

        // Create my pdof list
        EquObj.create_my_pdof_list();

        // Check if right pdof host was created in given pdof host list
        CHECK( equal_to( EquObj.mFreePdofs.size(), 2 ) );

        delete Node1;
        delete Node2;
        delete tPdofHostList(0);
        delete tPdofHostList(1);
        delete tPdofHostList(2);
    }

    TEST_CASE("Eqn_Obj_create_my_list_of_adof_ids","[MSI],[Eqn_Obj_create_my_list_of_adof_ids]")
    {
        // Create generic equation objects
        Equation_Object EquObj;

        EquObj.mFreePdofs.resize( 4 );
        EquObj.mFreePdofs( 0 ) = new Pdof;
        EquObj.mFreePdofs( 1 ) = new Pdof;
        EquObj.mFreePdofs( 2 ) = new Pdof;
        EquObj.mFreePdofs( 3 ) = new Pdof;

        EquObj.mFreePdofs( 0 )->mAdofIds.set_size( 2, 1 );
        EquObj.mFreePdofs( 1 )->mAdofIds.set_size( 1, 1 );
        EquObj.mFreePdofs( 2 )->mAdofIds.set_size( 4, 1 );
        EquObj.mFreePdofs( 3 )->mAdofIds.set_size( 2, 1 );

        EquObj.mFreePdofs( 0 )->mAdofIds( 0, 0 ) = 0;
        EquObj.mFreePdofs( 0 )->mAdofIds( 1, 0 ) = 1;
        EquObj.mFreePdofs( 1 )->mAdofIds( 0, 0 ) = 3;
        EquObj.mFreePdofs( 2 )->mAdofIds( 0, 0 ) = 0;
        EquObj.mFreePdofs( 2 )->mAdofIds( 1, 0 ) = 2;
        EquObj.mFreePdofs( 2 )->mAdofIds( 2, 0 ) = 6;
        EquObj.mFreePdofs( 2 )->mAdofIds( 3, 0 ) = 7;
        EquObj.mFreePdofs( 0 )->mAdofIds( 0, 0 ) = 1;
        EquObj.mFreePdofs( 0 )->mAdofIds( 1, 0 ) = 5;

        EquObj.create_my_list_of_adof_ids();

        CHECK( equal_to( EquObj.mUniqueAdofList.length(), 7 ) );
        CHECK( equal_to( EquObj.mUniqueAdofList( 0 ), 0 ) );
        CHECK( equal_to( EquObj.mUniqueAdofList( 1 ), 1 ) );
        CHECK( equal_to( EquObj.mUniqueAdofList( 2 ), 2 ) );
        CHECK( equal_to( EquObj.mUniqueAdofList( 3 ), 3 ) );
        CHECK( equal_to( EquObj.mUniqueAdofList( 4 ), 5 ) );
        CHECK( equal_to( EquObj.mUniqueAdofList( 5 ), 6 ) );
        CHECK( equal_to( EquObj.mUniqueAdofList( 6 ), 7 ) );

        delete EquObj.mFreePdofs(0);
        delete EquObj.mFreePdofs(1);
        delete EquObj.mFreePdofs(2);
        delete EquObj.mFreePdofs(3);
    }

    TEST_CASE("Eqn_Obj_create_adof_map","[MSI],[Eqn_Obj_create_adof_map]")
    {
        // Create generic equation objects
        Equation_Object EquObj;

        // Hadcode values into the mUniqueAdofList for test purposes
        EquObj.mUniqueAdofList.set_size( 6, 1 );
        EquObj.mUniqueAdofList( 0, 0 ) = 1;
        EquObj.mUniqueAdofList( 1, 0 ) = 5;
        EquObj.mUniqueAdofList( 2, 0 ) = 10;
        EquObj.mUniqueAdofList( 3, 0 ) = 15;
        EquObj.mUniqueAdofList( 4, 0 ) = 16;
        EquObj.mUniqueAdofList( 5, 0 ) = 19;

        // Create map for adofs
        EquObj.set_unique_adof_map();

        // Check if map works
        CHECK( equal_to( EquObj.mUniqueAdofMap[ 1 ],  0 ) );
        CHECK( equal_to( EquObj.mUniqueAdofMap[ 5 ],  1 ) );
        CHECK( equal_to( EquObj.mUniqueAdofMap[ 10 ], 2 ) );
        CHECK( equal_to( EquObj.mUniqueAdofMap[ 15 ], 3 ) );
        CHECK( equal_to( EquObj.mUniqueAdofMap[ 16 ], 4 ) );
        CHECK( equal_to( EquObj.mUniqueAdofMap[ 19 ], 5 ) );
    }

    TEST_CASE("Eqn_Obj_PADofMap","[MSI],[Eqn_Obj_PADofMap]")
    {
        // Create generic equation objects
        Equation_Object EquObj;

        // Hadcode values into the mUniqueAdofList for test purposes
        EquObj.mUniqueAdofList.set_size( 6, 1 );
        EquObj.mUniqueAdofList( 0, 0 ) = 0;
        EquObj.mUniqueAdofList( 1, 0 ) = 5;
        EquObj.mUniqueAdofList( 2, 0 ) = 10;
        EquObj.mUniqueAdofList( 3, 0 ) = 15;
        EquObj.mUniqueAdofList( 4, 0 ) = 16;
        EquObj.mUniqueAdofList( 5, 0 ) = 19;

        // Create map for adofs
        EquObj.set_unique_adof_map();

        // Hardcoding pdofs
        EquObj.mFreePdofs.resize( 4 );
        EquObj.mFreePdofs( 0 ) = new Pdof;
        EquObj.mFreePdofs( 1 ) = new Pdof;
        EquObj.mFreePdofs( 2 ) = new Pdof;
        EquObj.mFreePdofs( 3 ) = new Pdof;

        // Set values into mAdofIds of these pdofs
        EquObj.mFreePdofs( 0 )->mAdofIds.set_size( 2, 1 );
        EquObj.mFreePdofs( 1 )->mAdofIds.set_size( 1, 1 );
        EquObj.mFreePdofs( 2 )->mAdofIds.set_size( 4, 1 );
        EquObj.mFreePdofs( 3 )->mAdofIds.set_size( 2, 1 );

        EquObj.mFreePdofs( 0 )->mAdofIds( 0, 0 ) = 0;
        EquObj.mFreePdofs( 0 )->mAdofIds( 1, 0 ) = 5;
        EquObj.mFreePdofs( 1 )->mAdofIds( 0, 0 ) = 15;
        EquObj.mFreePdofs( 2 )->mAdofIds( 0, 0 ) = 0;
        EquObj.mFreePdofs( 2 )->mAdofIds( 1, 0 ) = 16;
        EquObj.mFreePdofs( 2 )->mAdofIds( 2, 0 ) = 19;
        EquObj.mFreePdofs( 2 )->mAdofIds( 3, 0 ) = 10;
        EquObj.mFreePdofs( 3 )->mAdofIds( 0, 0 ) = 19;
        EquObj.mFreePdofs( 3 )->mAdofIds( 1, 0 ) = 5;

        // Set values into mTmatrix of these pdofs
        moris::Mat< moris::real > tTmatrix_1( 2, 1 );
        moris::Mat< moris::real > tTmatrix_2( 1, 1 );
        moris::Mat< moris::real > tTmatrix_3( 4, 1 );
        moris::Mat< moris::real > tTmatrix_4( 2, 1 );

        tTmatrix_1( 0, 0 ) = 1.0;
        tTmatrix_1( 1, 0 ) = 0.5;
        tTmatrix_2( 0, 0 ) = 2.0;
        tTmatrix_3( 0, 0 ) = 5.0;
        tTmatrix_3( 1, 0 ) = 6.5;
        tTmatrix_3( 2, 0 ) = 0.2;
        tTmatrix_3( 3, 0 ) = 0.1;
        tTmatrix_4( 0, 0 ) = 3.0;
        tTmatrix_4( 1, 0 ) = 10.1;

        EquObj.mFreePdofs( 0 )->mTmatrix = tTmatrix_1;
        EquObj.mFreePdofs( 1 )->mTmatrix = tTmatrix_2;
        EquObj.mFreePdofs( 2 )->mTmatrix = tTmatrix_3;
        EquObj.mFreePdofs( 3 )->mTmatrix = tTmatrix_4;

        // Bulding PADofMap
        moris::Mat< moris::real > tPADofMap;
        EquObj.build_PADofMap( tPADofMap );

        // Checking entries of APDofMap
        CHECK( equal_to( tPADofMap( 0, 0 ), 1.0 ) );
        CHECK( equal_to( tPADofMap( 0, 1 ), 0.5 ) );
        CHECK( equal_to( tPADofMap( 1, 3 ), 2.0 ) );
        CHECK( equal_to( tPADofMap( 2, 0 ), 5.0 ) );
        CHECK( equal_to( tPADofMap( 2, 2 ), 0.1 ) );
        CHECK( equal_to( tPADofMap( 2, 4 ), 6.5 ) );
        CHECK( equal_to( tPADofMap( 2, 5 ), 0.2 ) );
        CHECK( equal_to( tPADofMap( 3, 1 ), 10.1 ) );
        CHECK( equal_to( tPADofMap( 3, 5 ), 3.0 ) );

        delete EquObj.mFreePdofs(0);
        delete EquObj.mFreePdofs(1);
        delete EquObj.mFreePdofs(2);
        delete EquObj.mFreePdofs(3);
    }

    }
}

