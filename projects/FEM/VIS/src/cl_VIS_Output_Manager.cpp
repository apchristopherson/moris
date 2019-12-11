
#include "cl_VIS_Output_Manager.hpp"

#include "cl_VIS_Factory.hpp"

#include "cl_MDL_Model.hpp"
#include "cl_MSI_Equation_Set.hpp"
#include "cl_FEM_Set.hpp"

extern moris::Comm_Manager gMorisComm;

namespace moris
{
    namespace vis
    {

//-----------------------------------------------------------------------------------------------------------

    void Output_Manager::set_outputs( const uint                              aOutputIndex,
                                      const enum VIS_Mesh_Type                aMeshType,
                                      const std::string                     & aMeshName,
                                      const moris::Cell< std::string >      & aBlockNames,
                                      const moris::Cell< moris_index >      & aBlockIndices,
                                      const moris::Cell< std::string >      & aFieldNames,
                                      const moris::Cell< enum Field_Type >  & aFieldType,
                                      const moris::Cell< enum Output_Type > & aEnum )
    {
        // create output data object
        vis::Output_Data tOutputData;

        // fill output data object
        tOutputData.mMeshIndex  = aOutputIndex;
        tOutputData.mMeshType   = aMeshType;
        tOutputData.mMeshName   = aMeshName;
        tOutputData.mSetNames   = aBlockNames;
        tOutputData.mSetIndices = aBlockIndices;
        tOutputData.mFieldNames = aFieldNames;
        tOutputData.mFieldType  = aFieldType;
        tOutputData.mEnum       = aEnum;

        // resize list of output data objects
        uint tSize = mOutputData.size();
        uint OutputDataSize = std::max( tSize, aOutputIndex + 1 );

        mOutputData.resize( OutputDataSize );

        // assign output data object to list
        mOutputData( aOutputIndex ) = tOutputData;

        // resize mesh list
        mVisMesh.resize( mOutputData.size(), nullptr );
    }

//-----------------------------------------------------------------------------------------------------------

    void Output_Manager::create_visualization_mesh( const uint                aVisMeshIndex,
                                                          mtk::Mesh_Manager * aMesh,
                                                    const uint                aMeshPairIndex)
    {
        MORIS_ERROR( mOutputData( aVisMeshIndex ).mMeshIndex == ( sint )aVisMeshIndex, "create_visualization_meshes(), Visualization mesh not set" );

        vis::VIS_Factory tVisFactory( aMesh, aMeshPairIndex );

        mVisMesh( aVisMeshIndex ) = tVisFactory.create_visualization_mesh( mOutputData( aVisMeshIndex ) );

        mWriter = new Writer_Exodus(mVisMesh( aVisMeshIndex ));
    }

//-----------------------------------------------------------------------------------------------------------

    void Output_Manager::set_visualization_sets( const uint         aVisMeshIndex,
                                                       mdl::Model * aModel )
    {
        uint tRequestedSets = mOutputData( aVisMeshIndex ).mSetIndices.size();

        map< moris_index, moris_index > & tMeshSetToFemSetMap = aModel->get_mesh_set_to_fem_set_index_map( );

        moris::Cell< MSI::Equation_Set * > tEquationSets(aModel->get_equation_sets().size());
        for(moris::uint iSet = 0; iSet < aModel->get_equation_sets().size(); iSet++ )
        {
            tEquationSets( iSet ) = aModel->get_equation_sets()( iSet );
        }

        for( uint Ii = 0; Ii < tRequestedSets; Ii++ )
        {
            moris_index tBlockIndex = mOutputData( aVisMeshIndex ).mSetIndices( Ii );

            moris_index tEquationSetIndex = tMeshSetToFemSetMap.find( tBlockIndex );

            tEquationSets( tEquationSetIndex )->set_visualization_set( mVisMesh( aVisMeshIndex )->get_block_by_index( Ii ) );
        }
    }

//-----------------------------------------------------------------------------------------------------------

    void Output_Manager::write_mesh( const uint aVisMeshIndex,
                                     const real tTime )
    {
        std::string tPrefix = std::getenv("MORISROOT");
        std::string tMeshFilePath = tPrefix + "build";

        std::string tMeshFileName=  mOutputData( aVisMeshIndex ).mMeshName;

        mWriter->write_mesh( tMeshFilePath, mOutputData( aVisMeshIndex ).mMeshName );

        this->add_nodal_fields( aVisMeshIndex );
        this->add_elemetal_fields( aVisMeshIndex );
//        this->add_global_fields( aVisMeshIndex );

        mWriter->set_time( tTime );

        this->write_mesh_indices( aVisMeshIndex );
    }


//-----------------------------------------------------------------------------------------------------------

    void Output_Manager::add_nodal_fields( const uint aVisMeshIndex )
    {
        moris::Cell<std::string> tNodalFieldNames( 2 + mOutputData( aVisMeshIndex ).mFieldNames.size() );

        tNodalFieldNames( 0 ) = "Mesh_Id";
        tNodalFieldNames( 1 ) = "Mesh_Index";

        uint tCounter = 2;

        for( uint Ik = 0; Ik < mOutputData( aVisMeshIndex ).mFieldNames.size(); Ik++ )
        {
            if( mOutputData( aVisMeshIndex ).mFieldType( Ik ) == Field_Type::NODAL )
            {
                tNodalFieldNames( tCounter++ ) = mOutputData( aVisMeshIndex ).mFieldNames( Ik );
            }
        }

        tNodalFieldNames.resize( tCounter );

        mWriter->set_nodal_fields( tNodalFieldNames );
    }

//-----------------------------------------------------------------------------------------------------------

    void Output_Manager::add_elemetal_fields( const uint aVisMeshIndex )
    {
        moris::Cell<std::string> tElementalFieldNames( 2 + mOutputData( aVisMeshIndex ).mFieldNames.size() );

        tElementalFieldNames( 0 ) = "Mesh_Id";
        tElementalFieldNames( 1 ) = "Mesh_Index";

        uint tCounter = 2;

        for( uint Ik = 0; Ik < mOutputData( aVisMeshIndex ).mFieldNames.size(); Ik++ )
        {
            if( mOutputData( aVisMeshIndex ).mFieldType( Ik ) == Field_Type::ELEMENTAL )
            {
                tElementalFieldNames( tCounter++ ) = mOutputData( aVisMeshIndex ).mFieldNames( Ik );
            }
        }

        tElementalFieldNames.resize( tCounter );

        mWriter->set_elemental_fields( tElementalFieldNames );
    }

//-----------------------------------------------------------------------------------------------------------

    void Output_Manager::add_global_fields( const uint aVisMeshIndex )
    {
        moris::Cell<std::string> tGlobalFieldNames( mOutputData( aVisMeshIndex ).mFieldNames.size() );

        uint tCounter = 0;

        for( uint Ik = 0; Ik < mOutputData( aVisMeshIndex ).mFieldNames.size(); Ik++ )
        {
            if( mOutputData( aVisMeshIndex ).mFieldType( Ik ) == Field_Type::GLOBAL )
            {
            	tGlobalFieldNames( tCounter++ ) = mOutputData( aVisMeshIndex ).mFieldNames( Ik );
            }
        }

        tGlobalFieldNames.resize( tCounter );

        mWriter->set_global_variables( tGlobalFieldNames );
    }

//-----------------------------------------------------------------------------------------------------------

    void Output_Manager::write_mesh_indices( const uint aVisMeshIndex )
    {
        uint tRequestedSets = mOutputData( aVisMeshIndex ).mSetIndices.size();

        for( uint Ii = 0; Ii < tRequestedSets; Ii++ )
        {
            uint tBlockIndex = mOutputData( aVisMeshIndex ).mSetIndices( Ii );

            moris::mtk::Set * tSet = mVisMesh( aVisMeshIndex )->get_block_by_index( tBlockIndex );

            bool tOnlyPrimaryCells = true ;

            switch( mOutputData( aVisMeshIndex ).mMeshType )
            {
                case ( vis::VIS_Mesh_Type::STANDARD ):
                    tOnlyPrimaryCells = true ;
                    break;

                case ( vis::VIS_Mesh_Type::OVERLAPPING_INTERFACE ):
                    tOnlyPrimaryCells = false;
                    break;

                case ( vis::VIS_Mesh_Type::FULL_DISCONTINOUS ):
                     MORIS_ERROR( false, "create_visualization_mesh() - Mesh type FULL_DISCONTINOUS not implemented yet. " );
                     break;

                default:
                    MORIS_ERROR( false, "create_visualization_mesh() - Mesh type not specified. " );
                    break;
            }

            uint tNumCells = tSet->get_num_cells_on_set( tOnlyPrimaryCells );

            moris::Matrix< DDSMat > tCellIndex = tSet->get_cell_inds_on_block( tOnlyPrimaryCells );

            sint tMaxIndex = tCellIndex.max();

            Matrix< DDSMat > tCellAsseblyMap( tMaxIndex + 1, 1, -1 );

            for( uint Ik = 0; Ik < tNumCells; Ik++ )
            {
                tCellAsseblyMap( tCellIndex( Ik ) ) = Ik;
            }
            //----------------------------------

            moris::Cell< Matrix< DDRMat > > tIdIndex( 2 );
            tIdIndex( 0 ).set_size( tCellIndex.numel(), 1 );
            tIdIndex( 1 ).set_size( tCellIndex.numel(), 1 );

            moris::Cell< mtk::Cluster const* > tMeshClusterList = tSet->get_clusters_on_set();

            for( uint Ik = 0; Ik < tMeshClusterList.size(); Ik++ )
            {
                const moris::Cell<moris::mtk::Cell const *> & tPrimaryCells = tMeshClusterList( Ik )->get_primary_cells_in_cluster();

                for( uint Ia = 0; Ia < tPrimaryCells.size(); Ia++ )
                {
                    moris_index tIndex = tPrimaryCells( Ia )->get_index();

                    moris_id tMeshId =  reinterpret_cast< const vis::Cell_Visualization* >( tPrimaryCells( Ia ) )->get_mesh_cell_id();

                    moris_index tMeshIndex = reinterpret_cast< const vis::Cell_Visualization* >( tPrimaryCells( Ia ) )->get_mesh_cell_index();

                    tIdIndex( 0 )( tCellAsseblyMap( tIndex ) ) = tMeshId;
                    tIdIndex( 1 )( tCellAsseblyMap( tIndex ) ) = tMeshIndex;
                }

                const moris::Cell<moris::mtk::Cell const *> & tVoidCells = tMeshClusterList( Ik )->get_void_cells_in_cluster();

                for( uint Ia = 0; Ia < tVoidCells.size(); Ia++ )
                {
                    moris_index tIndex = tVoidCells( Ia )->get_index();

                    moris_id tMeshId = reinterpret_cast< const vis::Cell_Visualization* >( tVoidCells( Ia ) )->get_mesh_cell_id();

                    moris_index tMeshIndex = reinterpret_cast< const vis::Cell_Visualization* >( tVoidCells( Ia ) )->get_mesh_cell_index();

                    tIdIndex( 0 )( tCellAsseblyMap( tIndex ) ) = tMeshId;
                    tIdIndex( 1 )( tCellAsseblyMap( tIndex ) ) = tMeshIndex;
                }
            }

            mWriter->write_elemental_field( tSet->get_set_name() , "Mesh_Id", tIdIndex( 0 ) );
            mWriter->write_elemental_field( tSet->get_set_name() , "Mesh_Index", tIdIndex( 1 ) );
        }
    }

    }
}
