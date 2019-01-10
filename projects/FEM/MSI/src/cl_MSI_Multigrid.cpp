/*
 * cl_MSI_Multigrid.cpp
 *
 *  Created on: Dez 11, 2018
 *      Author: schmidt
 */
#include "cl_MSI_Adof.hpp"
#include "cl_MSI_Dof_Manager.hpp"
#include "cl_MSI_Model_Solver_Interface.hpp"
#include "cl_MSI_Multigrid.hpp"

#include "cl_MTK_Mesh.hpp"
#include "cl_HMR_Database.hpp"

namespace moris
{
    namespace MSI
    {
    Multigrid::Multigrid( moris::MSI::Model_Solver_Interface * aModelSolverInterface,
                          moris::mtk::Mesh                   * aMesh ) : mMesh( aMesh ),
                                                                         mModelSolverInterface( aModelSolverInterface )
    {
        // Set number of desired multigrid levels
        mMultigridLevels = 2;   //FIXME Input

        mNumDofsRemain.set_size( mMultigridLevels, 1 );

        this->determine_mesh_index_by_order();
    }

//------------------------------------------------------------------------------------------------------------------------------------

    void Multigrid::multigrid_initialize()
    {
        // get list of owned adofs
        moris::Cell < Adof * > tOwnedAdofList = mModelSolverInterface->get_dof_manager()->get_owned_adofs();

        // get number of owned adofs
        moris::uint tNumOwnedAdofs = tOwnedAdofList.size();

        // List which relates every adof to its external index. For fines level
        mListAdofExtIndMap         .resize( mMultigridLevels + 1 );
        // List which relates every adof to its type time identifier. For fines level
        mListAdofTypeTimeIdentifier.resize( mMultigridLevels + 1 );

        // Set size of maps defined above
        mListAdofExtIndMap( 0 )         .set_size( tNumOwnedAdofs, 1 );
        mListAdofTypeTimeIdentifier( 0 ).set_size( tNumOwnedAdofs, 1 );

        // Loop over all adofs on fines mesh. Fill lists witrh external adof indices and type/time identifier
        moris::uint Ik = 0;
        for ( Adof* tAdof : tOwnedAdofList )
        {
            mListAdofExtIndMap( 0 )( Ik, 0) = tAdof->get_adof_external_ind();

            mListAdofTypeTimeIdentifier( 0 )( Ik++, 0) = tAdof->get_adof_type_time_identifier();
        }

        // get number if type/time identifiers
        mMaxDofTypes = mListAdofTypeTimeIdentifier( 0 ).max()+1;

        // Type time identifiers are initialize with -1. Check if all of them are set
        MORIS_ASSERT( mListAdofTypeTimeIdentifier( 0 ).min() != -1, "moris::MSI::Multigrid: Type/time identifier not specified");

        this->create_multigrid_level_dof_ordering();

        this->create_multigrid_maps();
    }

//------------------------------------------------------------------------------------------------------------------------------------

    void Multigrid::create_multigrid_level_dof_ordering()
    {
        // Get unique used dof type orders
        moris::Matrix< DDSMat > tDofTypeOrders = mModelSolverInterface->get_dof_manager()->get_unique_dof_type_orders();

        moris::sint tMaxMeshLevel = -1;
        // Maximal existing basis index
        moris::sint tMaxIndex = -1;

        // Loop over all used dof type orders
        for ( moris::uint Ia = 0; Ia < tDofTypeOrders.length(); Ia++ )
        {
            // Get mesh index for this mesh order
            moris::sint tMeshIndex = mMeshOrderIndexMap( tDofTypeOrders( Ia, 0 ), 0 );

            MORIS_ASSERT( tMeshIndex != -1, "Multigrid::create_multigrid_level_dof_ordering(): tMeshIndex is -1. check Multigrid::determine_mesh_index_by_order" );

            // Gets the maximal mesh level for this order
            moris::sint tMaxMeshLevelForOrder = mMesh->get_HMR_database()->get_bspline_mesh_by_index( tMeshIndex )
                                                                         ->get_max_level();

            tMaxMeshLevel = std::max( tMaxMeshLevel, tMaxMeshLevelForOrder );

            // get the maximal index of external indices. External indices are numbered consecutive.
            moris::sint tMaxIndexForOrder = mMesh->get_HMR_database()->get_bspline_mesh_by_index( tMeshIndex )
                                                                     ->get_number_of_indexed_basis();

            tMaxIndex = std::max( tMaxIndex, tMaxIndexForOrder );
        }

        MORIS_ASSERT( tMaxIndex != -1    , "Multigrid::create_multigrid_level_dof_ordering(): tMaxIndex is -1. check Multigrid::determine_mesh_index_by_order" );
        MORIS_ASSERT( tMaxMeshLevel != -1, "Multigrid::create_multigrid_level_dof_ordering(): tMaxMeshLevel is -1. check Multigrid::determine_mesh_index_by_order" );

        // Loop over all multigrid levels starting with the finest one
        for ( moris::sint Ik = 0; Ik < mMultigridLevels; Ik++ )
        {
            // get the number of the adofs on the actual level
            moris::uint tNumDofsOnLevel = mListAdofExtIndMap( Ik ).length();

            // Create list of of doftypes/time to check if a coarse level adof was created earlier. 0 means it was not created
            moris::Cell < Matrix < DDUMat> > tCoarseDofExist( mMaxDofTypes );
            for ( moris::sint Ib = 0; Ib < mMaxDofTypes; Ib++ )
            {
                tCoarseDofExist( Ib ).set_size( tMaxIndex, 1, 0 );
            }

            // Creates counters
            moris::uint tCounter = 0;
            moris::uint tCounterTooFine = 0;

            // Set size of list which relates every adof to its external index for next level.
            mListAdofExtIndMap( Ik + 1 )         .set_size( tNumDofsOnLevel, 1 );
            // List which relates every adof to its type time identifier. For next level
            mListAdofTypeTimeIdentifier( Ik + 1 ).set_size( tNumDofsOnLevel, 1 );

            // Create list to store indeces which are too fine temporarily.
            Matrix< DDSMat > tEntryOfTooFineDofs( tNumDofsOnLevel, 1, -1);

            // Loop over all dofs on this level
            for ( moris::uint Ii = 0; Ii < tNumDofsOnLevel; Ii++ )
            {
                //-----------------------------------------------------------------------------------------------------
                // Get this dof type
                moris::sint tDofType = mModelSolverInterface->get_dof_manager()
                                                            ->get_typetime_identifier_to_type_map()( mListAdofTypeTimeIdentifier( Ik )( Ii, 0 ), 0 );

                // Get order of this dof type
                moris::sint tDofOrder = mModelSolverInterface->get_adof_order_for_type(tDofType);

                // Get mesh index for this dof type
                moris::sint tMeshIndex = mMeshOrderIndexMap( tDofOrder, 0 );
                //---------------------------------------------------------------------------------------------------

                // Ask mesh for the level of this mesh index
                moris::sint tDofLevel = mMesh->get_HMR_database()->get_bspline_mesh_by_index( tMeshIndex )
                                                                 ->get_basis_by_index( mListAdofExtIndMap( Ik )( Ii, 0 ) )
                                                                 ->get_level();

                // If Index is inside of the set of dofs on this multigrid level, than add it to list.
                if( tDofLevel < tMaxMeshLevel - Ik )
                {
                    // Add external dof index to list
                    mListAdofExtIndMap( Ik + 1 )( tCounter ) = mListAdofExtIndMap( Ik )( Ii, 0 );

                    // Add type/time identifiert to list
                    mListAdofTypeTimeIdentifier( Ik + 1 )( tCounter++ ) = mListAdofTypeTimeIdentifier( Ik )( Ii, 0 );

                    // Set this adof( external index ) to one to indicate that it was created for the right type and time.
                    tCoarseDofExist( mListAdofTypeTimeIdentifier( Ik )( Ii, 0 ) )( mListAdofExtIndMap( Ik )( Ii, 0 ), 0 ) = 1;
                }
                else
                {
                    // Adof is on a level which is too fine. Remember it in this list.
                    tEntryOfTooFineDofs( tCounterTooFine++, 0 ) = Ii;
                }
            }

            // save tCounter in this list for this level. This is done to speed up the assembly
            mNumDofsRemain( Ik, 0 ) = tCounter;

            // resize this list to the number of adofs which are too fine.
            tEntryOfTooFineDofs.resize( tCounterTooFine, 1 );

            // Loop over all adofs which were too fine in the first loop
            for ( moris::uint Ij = 0; Ij < tCounterTooFine; Ij++ )
            {
                //-----------------------------------------------------------------------------------------------------
                // Get this dof type
                moris::sint tDofType = mModelSolverInterface->get_dof_manager()
                                                            ->get_typetime_identifier_to_type_map()( mListAdofTypeTimeIdentifier( Ik )( tEntryOfTooFineDofs( Ij, 0 ), 0 ), 0 );

                // Get order of this dof type
                moris::sint tDofOrder = mModelSolverInterface->get_adof_order_for_type(tDofType);

                // Get mesh index for this dof type
                moris::sint tMeshIndex = mMeshOrderIndexMap( tDofOrder, 0 );
                //---------------------------------------------------------------------------------------------------

                // get basis pointer for adof which was too fine.
                moris::hmr::Basis * tBasis = mMesh->get_HMR_database()
                                                  ->get_bspline_mesh_by_index( tMeshIndex )
                                                  ->get_basis_by_index( mListAdofExtIndMap( Ik )( tEntryOfTooFineDofs( Ij, 0 ), 0 ) );

                // Get type/time identifier for this dof
                moris::uint tTypeIdentifier = mListAdofTypeTimeIdentifier( Ik )( tEntryOfTooFineDofs( Ij, 0 ), 0 );

                // get the number of carse adofs which are interpolating into this fine adof.
                moris:: uint tNumCoarseDofs = tBasis ->get_number_of_parents() ;

                // Loop over these coarse adofs
                for ( moris::uint Ia = 0; Ia < tNumCoarseDofs; Ia++ )
                {
                    // Get external index of coarse adof
                    moris:: uint tCoarseDofIndex = tBasis->get_parent( Ia )->get_index();

                    // Check if this coarse index was not created earlier on coarse level
                    if ( tCoarseDofExist( tTypeIdentifier )( tCoarseDofIndex , 0 ) == 0 )
                    {
                        // Put index in list
                        mListAdofExtIndMap( Ik + 1 )( tCounter ) = tCoarseDofIndex;

                        // Put type time identifier in list
                        mListAdofTypeTimeIdentifier( Ik + 1 )( tCounter++ ) = tTypeIdentifier;

                        // Set this coarse index to one to indicate that it was created.
                        tCoarseDofExist( tTypeIdentifier )( tCoarseDofIndex, 0 ) = 1;
                    }
                }
            }

            // All coarse adofs are created. resize list to the number of created adofs
            mListAdofExtIndMap( Ik + 1 ).resize( tCounter, 1 );

            // Do the same for the type time identifier list.
            mListAdofTypeTimeIdentifier( Ik + 1 ).resize( tCounter, 1 );
        }
    }

    //------------------------------------------------------------------------------------------------------------------------------------

    void Multigrid::create_multigrid_maps()
    {
        // Get unique used dof type orders
        moris::Matrix< DDSMat > tDofTypeOrders = mModelSolverInterface->get_dof_manager()->get_unique_dof_type_orders();

        // Maximal existing basis index
        moris::sint tMaxIndex = -1;

        // Loop over all used dof type orders
        for ( moris::uint Ia = 0; Ia < tDofTypeOrders.length(); Ia++ )
        {
            // Get mesh index for this mesh order
            moris::sint tMeshIndex = mMeshOrderIndexMap( tDofTypeOrders( Ia, 0 ), 0 );

            MORIS_ASSERT( tMeshIndex != -1, "Multigrid::create_multigrid_level_dof_ordering(): tMeshIndex is -1. check Multigrid::determine_mesh_index_by_order" );

            // get the maximal index of external indices. External indices are numbered consecutive.
            moris::sint tMaxIndexForOrder = mMesh->get_HMR_database()->get_bspline_mesh_by_index( tMeshIndex )
                                                                     ->get_number_of_indexed_basis();

            tMaxIndex = std::max( tMaxIndex, tMaxIndexForOrder );
        }

        // Resize this lists outer layer to the number of multigrid levels.
        mMultigridMap.resize( mMultigridLevels + 1 );

        // Loop over all multigrid levels
        for ( moris::sint Ik = 0; Ik < mMultigridLevels + 1; Ik++ )
        {
            // Set the inner list number of types/time used.
            mMultigridMap( Ik ).resize( mMaxDofTypes );

            // Loop all dof types/time
            for ( moris::uint Ii = 0; Ii < mMultigridMap( Ik ).size(); Ii++ )
            {
                // set size of level/type/time list
                mMultigridMap( Ik )( Ii ).set_size( tMaxIndex, 1, -1 );
            }
        }

        // Loop over all multigrid levels
        for ( moris::uint Ik = 0; Ik < mListAdofExtIndMap.size(); Ik++ )
        {
            // Loop over all dof types on this level
            for ( moris::uint Ij = 0; Ij < mListAdofExtIndMap( Ik ).length(); Ij++ )
            {
                // Get type/time identifier and external index of this adof
                moris::sint tTypeIdentifier = mListAdofTypeTimeIdentifier( Ik )( Ij, 0 );
                moris::sint tExtIndex = mListAdofExtIndMap( Ik )( Ij, 0 );

                // Set position of this adof on level and type time identifier
                mMultigridMap( Ik )( tTypeIdentifier )( tExtIndex, 0 ) = Ij;
            }
        }

//        //////////////////////// printing/////////////////////
//        for ( moris::uint Ik = 0; Ik < mListAdofExtIndMap.size(); Ik++ )
//        {
//            print( mListAdofExtIndMap( Ik ), "mListAdofExtIndMap Ik");
//
//            for ( moris::uint Ij = 0; Ij < mMultigridMap( Ik ).size(); Ij++ )
//            {
//               print( mMultigridMap( Ik )( Ij ), "mMultigridMap Ik Ij");
//            }
//        }
    }

    //------------------------------------------------------------------------------------------------------------------------------------

    void Multigrid::read_multigrid_maps( const moris::uint               aLevel,
                                         const moris::Matrix< DDSMat > & aExtFineIndices,
                                         const moris::sint               aTypeTimeIdentifier,
                                               moris::Matrix< DDSMat > & aInternalFineIndices )
    {
        moris::uint tNumOfIndicesAsk = aExtFineIndices.length();

        aInternalFineIndices.set_size( tNumOfIndicesAsk, 1, -1 );

        for ( moris::uint Ik = 0; Ik < tNumOfIndicesAsk; Ik++ )
        {
            aInternalFineIndices( Ik, 0 ) = mMultigridMap( aLevel )( aTypeTimeIdentifier )( aExtFineIndices( Ik, 0 ), 0 );
        }
    }

    //------------------------------------------------------------------------------------------------------------------------------------

    void Multigrid::determine_mesh_index_by_order()
    {
        // Get number of meshes
        moris::uint tNumMeshes = mMesh->get_HMR_database()->get_number_of_bspline_meshes();

        // HMR creates one input and one output mesh. These meshes are ordered in: first input mehses, then output meshes
        moris::uint tFirstOutputMeshIndexs = tNumMeshes/2;

        // Set size of order to index map to 4. We only have maximal 3 orders in HMR
        mMeshOrderIndexMap.set_size( 4, 1 , -1);

        // Loop over all meshes. Ask for the order of each index and create map.
        for ( moris::uint Ik = tFirstOutputMeshIndexs; Ik < tNumMeshes; Ik++ )
        {
            moris::uint tMeshOrder = mMesh->get_HMR_database()->get_bspline_mesh_by_index( Ik )
                                                              ->get_order();

            mMeshOrderIndexMap( tMeshOrder, 0 ) = Ik;
        }
    }

}
}
