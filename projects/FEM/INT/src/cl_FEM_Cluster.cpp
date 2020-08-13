/*
 * cl_FEM_Cluster.hpp
 *
 *  Created on: Apr 11, 2019
 *      Author: schmidt
 */
#include <iostream>

#include "cl_FEM_Element.hpp" //FEM/INT/src
#include "cl_FEM_Cluster.hpp"   //FEM/INT/src
#include "cl_FEM_Field_Interpolator_Manager.hpp" //FEM/INT/src

namespace moris
{
    namespace fem
    {

        //------------------------------------------------------------------------------

        Cluster::Cluster(
                const Element_Type         aElementType,
                const mtk::Cluster        * aMeshCluster,
                Set                       * aSet,
                MSI::Equation_Object      * aEquationObject )
        : mInterpolationElement( aEquationObject ),
          mSet( aSet ),
          mElementType( aElementType )
        {
            // fill the cell cluster pointer
            mMeshCluster = aMeshCluster;

            // fill the master integration cells
            mMasterIntegrationCells = aMeshCluster->get_primary_cells_in_cluster();

            // get number of subelements (IG cells)
            uint tNumMasterIGCells = mMasterIntegrationCells.size();

            // element factory
            fem::Element_Factory tElementFactory;

            // set size for the number of IG cells
            mElements.resize( tNumMasterIGCells, nullptr );

            // switch on the element type
            switch ( mElementType )
            {
                case fem::Element_Type::BULK :
                case fem::Element_Type::TIME_SIDESET :
                {
                    // loop over the IG cells
                    for ( uint iIGCell = 0; iIGCell < tNumMasterIGCells; iIGCell++ )
                    {
                        // create an element
                        mElements( iIGCell ) = tElementFactory.create_element(
                                mElementType,
                                mMasterIntegrationCells( iIGCell ),
                                mSet,
                                this,
                                iIGCell );
                    }
                    break;
                }
                case fem::Element_Type::SIDESET :
                {
                    // set the side ordinals for the IG cells in the cluster
                    mMasterListOfSideOrdinals = aMeshCluster->get_cell_side_ordinals();

                    // loop over the IG cells
                    for ( uint iIGCell = 0; iIGCell < tNumMasterIGCells; iIGCell++ )
                    {
                        // create an element
                        mElements( iIGCell ) = tElementFactory.create_element(
                                mElementType,
                                mMasterIntegrationCells( iIGCell ),
                                mSet,
                                this,
                                iIGCell );
                    }
                    break;
                }
                case fem::Element_Type::DOUBLE_SIDESET :
                {
                    // fill the slave integration cells
                    mSlaveIntegrationCells  = aMeshCluster->get_primary_cells_in_cluster( mtk::Master_Slave::SLAVE );

                    // set the side ordinals for the master and slave IG cells
                    mMasterListOfSideOrdinals = aMeshCluster->get_cell_side_ordinals( mtk::Master_Slave::MASTER );
                    mSlaveListOfSideOrdinals  = aMeshCluster->get_cell_side_ordinals( mtk::Master_Slave::SLAVE );

                    // loop over the IG cells
                    for( moris::uint iIGCell = 0; iIGCell < tNumMasterIGCells; iIGCell++)
                    {
                        // create element
                        mElements( iIGCell ) = tElementFactory.create_element(
                                mElementType,
                                mMasterIntegrationCells( iIGCell ),
                                mSlaveIntegrationCells( iIGCell ),
                                mSet,
                                this,
                                iIGCell );
                    }
                    break;
                }
                default:
                    MORIS_ERROR( false, "Cluster::Cluster - No element type specified" );
            }
        }

        //------------------------------------------------------------------------------

        Cluster::~Cluster()
        {
            for( auto tElements : mElements )
            {
                delete tElements;
            }
            mElements.clear();
        }

        //------------------------------------------------------------------------------

        moris::Matrix< moris::DDRMat > Cluster::get_vertices_local_coordinates_wrt_interp_cell()
        {
            // check that side cluster
            MORIS_ASSERT( mElementType == fem::Element_Type::BULK,
                    "Cluster::get_primary_cell_local_coords_on_side_wrt_interp_cell - not a bulk cluster.");

            // check that the mesh cluster was set
            MORIS_ASSERT( mMeshCluster != NULL,
                    "Cluster::get_vertices_local_coordinates_wrt_interp_cell - empty cluster.");

            // if trivial cluster IP cell = IG cell
            if( mMeshCluster->is_trivial( mtk::Master_Slave::MASTER ) )
            {
                // get the vertices param coords from the IP geometry interpolator
                return mSet->get_field_interpolator_manager( mtk::Master_Slave::MASTER )
                        ->get_IP_geometry_interpolator()
                        ->extract_space_param_coeff( mSet->get_IG_space_interpolation_order() );

            }
            // if non trivial cluster
            else
            {
                // check that the mesh cluster was set
                MORIS_ASSERT( mMeshCluster != NULL,
                        "Cluster::get_vertices_local_coordinates_wrt_interp_cell - empty cluster.");

                // get the vertices param coords from the cluster
                return mMeshCluster->get_vertices_local_coordinates_wrt_interp_cell();
            }
        }

        //------------------------------------------------------------------------------

        void Cluster::get_vertex_indices_in_cluster_for_sensitivity(
                moris::Matrix< moris::IndexMat > & aVerticesIndices )
        {
            // if mesh cluster is not trivial
            if( !mMeshCluster->is_trivial( mtk::Master_Slave::MASTER ) )
            {
                // get vertices indices on cluster
                aVerticesIndices = mMeshCluster->get_vertex_indices_in_cluster();

                // FIXME! Mesh should return this
                if( mElementType == fem::Element_Type::DOUBLE_SIDESET )
                {
                    aVerticesIndices.set_size( 1, 0 );
                    uint tVertexCounter = 0;

                    // loop over the IG cells
                    for( moris::uint iIGCell = 0; iIGCell < mMasterIntegrationCells.size(); iIGCell++)
                    {
                        Matrix< IndexMat > tMasterVertexIndices =
                                mMasterIntegrationCells( iIGCell )->
                                get_vertices_ind_on_side_ordinal( mMasterListOfSideOrdinals( iIGCell ) );

                        for(moris::uint iVertex = 0; iVertex < tMasterVertexIndices.numel(); iVertex++ )
                        {
                            tVertexCounter += 1;
                            aVerticesIndices.resize( 1, tVertexCounter );
                            // get the vertex index
                            aVerticesIndices( tVertexCounter - 1 ) = tMasterVertexIndices( iVertex );
                        }

                        Matrix< IndexMat > tSlaveVertexIndices  =
                                mSlaveIntegrationCells( iIGCell )->
                                get_vertices_ind_on_side_ordinal( mSlaveListOfSideOrdinals( iIGCell ) );

                        for(moris::uint iVertex = 0; iVertex < tSlaveVertexIndices.numel(); iVertex++ )
                        {
                            tVertexCounter += 1;
                            aVerticesIndices.resize( 1, tVertexCounter );
                            // get the vertex index
                            aVerticesIndices( tVertexCounter - 1 ) = tSlaveVertexIndices( iVertex );
                        }
                    }
                }
            }
        }

        //------------------------------------------------------------------------------

        moris::Cell< moris_index > Cluster::get_vertex_indices_in_cluster()
        {
            // check that side cluster
            MORIS_ASSERT( mElementType == fem::Element_Type::BULK,
                    "Cluster::get_vertex_indices_in_cluster - not a bulk cluster.");

            // check that the mesh cluster was set
            MORIS_ASSERT( mMeshCluster != NULL,
                    "Cluster::get_vertex_indices_in_cluster - empty cluster.");

            // init container for vertex indices
            moris::Cell< moris_index > tVerticesIndices;

            // if trivial cluster IP cell = IG cell
            if( mMeshCluster->is_trivial( mtk::Master_Slave::MASTER ) )
            {
                // get the integration cell on cluster
                moris::Cell<moris::mtk::Cell const *> const & tPrimaryCells =
                        mMeshCluster->get_primary_cells_in_cluster();

                // check that there is only once IG cell on trivial cluster
                MORIS_ASSERT( tPrimaryCells.size(),
                        "Cluster::get_vertex_indices_in_cluster - There needs to be exactly 1 primary cell in cluster for the trivial case");

                // an assumption is made here that the vertices on the primary correspond to the index in pdof vector
                moris::Cell< moris::mtk::Vertex* > tVerticesOnPrimaryCell =
                        tPrimaryCells( 0 )->get_vertex_pointers();

                // get number of vertices
                uint tNumVertices = tVerticesOnPrimaryCell.size();

                // set size for cell of vertex indices
                tVerticesIndices.resize( tNumVertices );

                // loop over vertices
                for(moris::uint iVertex = 0; iVertex < tNumVertices; iVertex++ )
                {
                    // get the vertex index
                    tVerticesIndices( iVertex ) = tVerticesOnPrimaryCell( iVertex )->get_index();
                }
            }
            // if non trivial cluster
            else
            {
                // get the vertices in the cluster
                const moris::Cell< const moris::mtk::Vertex * > tVertices =
                        mMeshCluster->get_vertices_in_cluster();

                // get number of vertices
                uint tNumVertices = tVertices.size();

                // set size for cell of vertex indices
                tVerticesIndices.resize( tNumVertices );

                // loop over vertices
                for( uint iVertex = 0; iVertex < tNumVertices; iVertex++ )
                {
                    // get the vertex index
                    tVerticesIndices( iVertex ) = tVertices( iVertex )->get_index();
                }
            }

            return tVerticesIndices;
        }

        //------------------------------------------------------------------------------

        moris::Matrix< moris::DDRMat > Cluster::get_cell_local_coords_on_side_wrt_interp_cell(
                moris::moris_index aCellIndexInCluster,
                moris::moris_index aSideOrdinal,
                mtk::Master_Slave  aIsMaster )
        {
            // check that side cluster
            MORIS_ASSERT( ( mElementType == fem::Element_Type::DOUBLE_SIDESET ) || ( mElementType == fem::Element_Type::SIDESET ),
                    "Cluster::get_cell_local_coords_on_side_wrt_interp_cell - not a side or double side cluster.");

            // check that the mesh cluster was set
            MORIS_ASSERT( mMeshCluster != NULL,
                    "Cluster::get_cell_local_coords_on_side_wrt_interp_cell - empty cluster.");

            // if trivial cluster IP cell = IG cell
            if( mMeshCluster->is_trivial( aIsMaster ) )
            {
                // get the side param coords from the IG geometry interpolator
                return mSet->get_field_interpolator_manager( aIsMaster )->
                        get_IP_geometry_interpolator()->
                        extract_space_side_space_param_coeff( aSideOrdinal, mSet->get_IG_space_interpolation_order() );
            }
            // if non trivial cluster
            else
            {
                // check that the mesh cluster was set
                MORIS_ASSERT( mMeshCluster != NULL, "Cluster::get_cell_local_coords_on_side_wrt_interp_cell - empty cluster.");

                // get the side param coords from the side cluster
                return mMeshCluster->get_cell_local_coords_on_side_wrt_interp_cell( aCellIndexInCluster, aIsMaster );
            }
        }

        //------------------------------------------------------------------------------

        moris::Matrix< moris::DDRMat > Cluster::get_primary_cell_local_coords_on_side_wrt_interp_cell(
                moris::moris_index aPrimaryCellIndexInCluster )
        {
            // check that bulk cluster
            MORIS_ASSERT( mElementType == fem::Element_Type::BULK || mElementType == fem::Element_Type::TIME_SIDESET,
                    "Cluster::get_primary_cell_local_coords_on_side_wrt_interp_cell - not a bulk cluster.");

            // check that the mesh cluster was set
            MORIS_ASSERT( mMeshCluster != NULL,
                    "Cluster::get_primary_cell_local_coords_on_side_wrt_interp_cell - empty cluster.");

            // if trivial cluster IP cell = IG cell
            if( mMeshCluster->is_trivial( mtk::Master_Slave::MASTER ) )
            {
                // get the side param coords from the IG geometry interpolator
                return mSet->get_field_interpolator_manager()->
                        get_IP_geometry_interpolator()->
                        extract_space_param_coeff( mSet->get_IG_space_interpolation_order() );
            }
            // if non trivial cluster
            else
            {
                // check that the mesh cluster was set
                MORIS_ASSERT( mMeshCluster != NULL, "Cluster::get_primary_cell_local_coords_on_side_wrt_interp_cell - empty cluster.");

                // get the side param coords from the side cluster
                return mMeshCluster->get_primary_cell_local_coords_on_side_wrt_interp_cell( aPrimaryCellIndexInCluster );
            }
        }

        //------------------------------------------------------------------------------

        Matrix< DDRMat > Cluster::get_side_normal(
                const mtk::Cell    * aCell,
                moris::moris_index   aSideOrdinal )
        {
            // init normal
            Matrix < DDRMat > tNormal;

            //            // if interpolation cell is linear
            //            if( mSet->get_IG_space_interpolation_order() == mtk::Interpolation_Order::LINEAR )
            //            {
            //                // get normal from the mesh
            //                tNormal = aCell->compute_outward_side_normal( aSideOrdinal );
            //            }
            //            // if integration cell is higher order
            //            else
            //            {
            // get normal from the integration cell geometry interpolator
            mSet->get_field_interpolator_manager()->get_IG_geometry_interpolator()->get_normal( tNormal );
            //            }

            return tNormal;
        }

        //------------------------------------------------------------------------------

        moris::mtk::Vertex const * Cluster::get_left_vertex_pair(
                moris::mtk::Vertex const * aLeftVertex )
        {
            // check that a double sided cluster
            MORIS_ASSERT( mElementType == fem::Element_Type::DOUBLE_SIDESET,
                    "Cluster::get_left_vertex_pair - not a double side cluster.");

            // check that the mesh cluster was set
            MORIS_ASSERT( mMeshCluster != NULL,
                    "Cluster::get_left_vertex_pair - empty cluster.");

            // get the paired vertex on the right
            return mMeshCluster->get_master_vertex_pair( aLeftVertex );
        }

        //------------------------------------------------------------------------------

        moris::moris_index Cluster::get_right_vertex_ordinal_on_facet(
                moris_index                aCellIndexInCluster,
                moris::mtk::Vertex const * aVertex )
        {
            // check that a double sided cluster
            MORIS_ASSERT( mElementType == fem::Element_Type::DOUBLE_SIDESET,
                    "Cluster::get_left_vertex_pair - not a double side cluster.");

            // check that the mesh cluster was set
            MORIS_ASSERT( mMeshCluster != NULL,
                    "Cluster::get_right_vertex_ordinal_on_facet - empty cluster.");

            // return the index of the paired vertex on the right
            return mMeshCluster->get_slave_vertex_ord_on_facet(aCellIndexInCluster, aVertex);
        }

        //------------------------------------------------------------------------------

        void Cluster::compute_jacobian()
        {
            // loop over the IG elements
            for ( uint iElem = 0; iElem < mElements.size(); iElem++ )
            {
                // compute the jacobian for the IG element
                mElements( iElem )->compute_jacobian();
            }
        }

        //------------------------------------------------------------------------------

        void Cluster::compute_residual()
        {
            // loop over the IG elements
            for ( uint iElem = 0; iElem < mElements.size(); iElem++ )
            {
                // compute the residual for the IG element
                mElements( iElem )->compute_residual();
            }
        }

        //------------------------------------------------------------------------------

        void Cluster::compute_jacobian_and_residual()
        {
            // loop over the IG elements
            for ( uint iElem = 0; iElem < mElements.size(); iElem++ )
            {
                // compute the jacobian and residual for the IG element
                mElements( iElem )->compute_jacobian_and_residual();
            }
        }

        //------------------------------------------------------------------------------

        void Cluster::compute_dRdp()
        {
            // loop over the IG elements
            for ( uint iElem = 0; iElem < mElements.size(); iElem++ )
            {
                // compute the dRdp for the IG element
                mElements( iElem )->compute_dRdp();
            }
        }

        //------------------------------------------------------------------------------

        void Cluster::compute_dQIdp_explicit()
        {
            // loop over the IG elements
            for ( uint iElem = 0; iElem < mElements.size(); iElem++ )
            {
                // compute the dQIdp for the IG element
                mElements( iElem )->compute_dQIdp_explicit();
            }
        }

        //------------------------------------------------------------------------------

        void Cluster::compute_dQIdu()
        {
            // loop over the IG elements
            for ( uint iElem = 0; iElem < mElements.size(); iElem++ )
            {
                // compute the dQIdu for the IG element
                mElements( iElem )->compute_dQIdu();
            }
        }

        //------------------------------------------------------------------------------

        void Cluster::compute_QI()
        {
            // loop over the IG elements
            for ( uint iElem = 0; iElem < mElements.size(); iElem++ )
            {
                // compute the quantities of interest for the IG element
                mElements( iElem )->compute_QI();
            }
        }

        //------------------------------------------------------------------------------

        void Cluster::compute_quantity_of_interest(
                const uint            aMeshIndex,
                enum vis::Output_Type aOutputType,
                enum vis::Field_Type  aFieldType )
        {
            // loop over the IG elements
            for ( uint iElem = 0; iElem < mElements.size(); iElem++ )
            {
                // compute the quantity of interest for the IG element
                mElements( iElem )->compute_quantity_of_interest( aMeshIndex, aOutputType, aFieldType );
            }
        }

        //------------------------------------------------------------------------------

        real Cluster::compute_volume()
        {
            // init cluster volume
            real tClusterVolume = 0;

            // loop over the IG elements
            for ( uint iElem = 0; iElem < mElements.size(); iElem++ )
            {
                // add volume contribution for the IG element
                tClusterVolume += mElements( iElem )->compute_volume();
            }

            // return cluster volume value
            return tClusterVolume;
        }

        //------------------------------------------------------------------------------

        moris::real Cluster::compute_cluster_cell_measure(
                const mtk::Primary_Void aPrimaryOrVoid ,
                const mtk::Master_Slave aIsMaster ) const
        {
            // check that the mesh cluster was set
            MORIS_ASSERT( mMeshCluster != NULL,
                    "Cluster::compute_cluster_cell_measure - empty cluster.");

            return mMeshCluster->compute_cluster_cell_measure( aPrimaryOrVoid, aIsMaster );
        }

        //------------------------------------------------------------------------------

        moris::real Cluster::compute_cluster_cell_side_measure(
                const mtk::Primary_Void aPrimaryOrVoid ,
                const mtk::Master_Slave aIsMaster ) const
        {
            // check that the mesh cluster was set
            MORIS_ASSERT( mMeshCluster != NULL,
                    "Cluster::compute_cluster_cell_side_measure - empty cluster.");

            return mMeshCluster->compute_cluster_cell_side_measure(aPrimaryOrVoid,aIsMaster);
        }

        //------------------------------------------------------------------------------

        moris::real Cluster::compute_cluster_cell_length_measure(
                const mtk::Primary_Void aPrimaryOrVoid,
                const mtk::Master_Slave aIsMaster ) const
        {
            // check that the mesh cluster was set
            MORIS_ASSERT( mMeshCluster != NULL,
                    "Cluster::compute_cluster_cell_length_measure - empty cluster.");

            // init volume of the custer
            real tVolume = 0.0;

            // get spatial dimension from IP geometry interpolator
            uint tSpaceDim = mSet->get_field_interpolator_manager( aIsMaster )->
                    get_IP_geometry_interpolator()->
                    get_number_of_space_dimensions();

            // switch on set type
            fem::Element_Type tElementType = mSet->get_element_type();
            switch( tElementType )
            {
                case fem::Element_Type::BULK :
                case fem::Element_Type::TIME_SIDESET :
                case fem::Element_Type::TIME_BOUNDARY :
                {
                    tVolume = mMeshCluster->compute_cluster_cell_measure( aPrimaryOrVoid, aIsMaster );
                    break;
                }
                case fem::Element_Type::SIDESET :
                case fem::Element_Type::DOUBLE_SIDESET :
                {
                    tVolume = mMeshCluster->compute_cluster_cell_side_measure( aPrimaryOrVoid, aIsMaster );
                    //tSpaceDim = tSpaceDim - 1;
                    break;
                }
                default:
                    MORIS_ERROR( false, "Undefined set type" );
            }

            // compute element size
            switch ( tSpaceDim )
            {
                case 3 :
                {
                    // compute length from volume
                    return std::pow( 6.0 * tVolume / M_PI, 1.0 / 3.0 );
                }
                case 2 :
                {
                    // compute length from volume
                    return std::pow( 4.0 * tVolume / M_PI, 1.0 / 2.0 );
                }
                case 1 :
                {
                    return tVolume;
                }

                default:
                    MORIS_ERROR( false, "Cluster::compute_cluster_cell_length_measure - space dimension can only be 1, 2, or 3. ");
                    return 0.0;
            }
        }

        //------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */
