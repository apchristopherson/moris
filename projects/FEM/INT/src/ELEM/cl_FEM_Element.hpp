/*
 * cl_FEM_Element.hpp
 *
 *  Created on: Apr 20, 2019
 *      Author: Schmidt
 */

#ifndef SRC_FEM_CL_FEM_ELEMENT_HPP_
#define SRC_FEM_CL_FEM_ELEMENT_HPP_

#include "assert.h"
#include <cmath>

#include "typedefs.hpp"                     //MRS/COR/src
#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
#include "cl_Cell.hpp"
#include "cl_MTK_Cell.hpp"                  //MTK/src
#include "cl_MSI_Equation_Object.hpp"       //FEM/MSI/src
#include "cl_FEM_Enums.hpp"                 //FEM/INT/src
#include "cl_FEM_Node.hpp"                  //FEM/INT/src
#include "cl_FEM_IWG.hpp"                   //FEM/INT/src
#include "cl_FEM_Geometry_Interpolator.hpp" //FEM/INT/src
#include "cl_FEM_Field_Interpolator.hpp"    //FEM/INT/src
#include "cl_FEM_Field_Interpolator_Manager.hpp"    //FEM/INT/src
#include "IG/cl_MTK_Integrator.hpp"            //MTK/src

#include "cl_FEM_Set.hpp"   //FEM/INT/src
#include "cl_FEM_Cluster.hpp"   //FEM/INT/src

namespace moris
{
    namespace fem
    {
        class Set;
        //------------------------------------------------------------------------------
        /**
         * \brief element class that communicates with the mesh interface
         */
        class Element
        {

            protected:

                // pointer to master and slave integration cells on mesh
                const mtk::Cell * mMasterCell;
                const mtk::Cell * mSlaveCell;

                // index for the cell within the cluster
                moris::moris_index mCellIndexInCluster;

                Set      * mSet     = nullptr;
                Cluster  * mCluster = nullptr;

                // function pointers
                void ( Element:: * m_compute_jacobian )(
                        const std::shared_ptr< IWG > & aReqIWG,
                        real                           aWStar ) = nullptr;
                void ( Element:: * m_compute_dRdp )(
                        const std::shared_ptr< IWG >      & aReqIWG,
                        real                                aWStar,
                        Matrix< DDSMat >                  & aGeoLocalAssembly,
                        moris::Cell< Matrix< IndexMat > > & aVertexIndices ) = nullptr;
                void ( Element:: * m_compute_dQIdu )(
                        const std::shared_ptr< IQI > & aReqIQI,
                        real                           aWStar ) = nullptr;
                void ( Element:: * m_compute_dQIdp )(
                        const std::shared_ptr< IQI >      & aReqIQI,
                        real                                aWStar,
                        Matrix< DDSMat >                  & aGeoLocalAssembly,
                        moris::Cell< Matrix< IndexMat > > & aVertexIndices ) = nullptr;

                // finite difference scheme type for jacobian and dQIdu
                fem::FDScheme_Type mFAFDScheme = fem::FDScheme_Type::POINT_1_FORWARD;

                // finite difference perturbation size for jacobian and dQIdu
                real mFAFDPerturbation = 1e-6;

                // finite difference scheme type for dRdp and dQIdp
                fem::FDScheme_Type mSAFDScheme = fem::FDScheme_Type::POINT_1_FORWARD;

                // finite difference perturbation size for dRdp and dQIdp
                real mSAFDPerturbation = 1e-6;

                //------------------------------------------------------------------------------
            public:
                //------------------------------------------------------------------------------
                /**
                 * trivial constructor
                 */
                Element(){};

                //------------------------------------------------------------------------------
                /**
                 * constructor for master only
                 * @param[ in ] aCell               a mesh cell pointer
                 * @param[ in ] aSet                a fem set pointer
                 * @param[ in ] aCluster            a mesh cluster pointer
                 * @param[ in ] aCellIndexInCluster the index of the cell within the cluster
                 */
                Element(
                        const mtk::Cell    * aCell,
                        Set                * aSet,
                        Cluster            * aCluster,
                        moris::moris_index   aCellIndexInCluster )
                : mSet( aSet ),
                  mCluster( aCluster )
                {
                    // fill the cell index in cluster
                    mCellIndexInCluster = aCellIndexInCluster;

                    // fill the bulk mtk::Cell pointer //FIXME
                    mMasterCell = aCell;

                    // set function pointers
                    this->set_function_pointers();
                };

                //------------------------------------------------------------------------------
                /**
                 * constructor for slave only
                 * @param[ in ] aMasterCell         a master mesh cell pointer
                 * @param[ in ] aSlaveCell          a slave mesh cell pointer
                 * @param[ in ] aSet                a fem set pointer
                 * @param[ in ] aCluster            a mesh cluster pointer
                 * @param[ in ] aCellIndexInCluster the index of the cell within the cluster
                 */
                Element(
                        const mtk::Cell    * aMasterCell,
                        const mtk::Cell    * aSlaveCell,
                        Set                * aSet,
                        Cluster            * aCluster,
                        moris::moris_index   aCellIndexInCluster )
                : mSet( aSet ),
                  mCluster( aCluster )
                {
                    // fill the cell index in cluster
                    mCellIndexInCluster = aCellIndexInCluster;

                    // fill the master and slave cell pointers
                    mMasterCell = aMasterCell;
                    mSlaveCell  = aSlaveCell;

                    // set function pointers
                    this->set_function_pointers();
                };

                //------------------------------------------------------------------------------
                /**
                 * trivial destructor
                 */
                virtual ~Element(){};

                //------------------------------------------------------------------------------
                /**
                 * set function pointers for analytical and FD
                 */
                void set_function_pointers()
                {
                    // get bool for forward analysis evaluation type
                    bool tIsAnalyticalJacobian = mSet->get_is_analytical_forward_analysis();

                    if( tIsAnalyticalJacobian )
                    {
                        m_compute_jacobian = &Element::select_jacobian;
                        m_compute_dQIdu    = &Element::select_dQIdu;
                    }
                    else
                    {
                        // get finite difference scheme type
                        mFAFDScheme = mSet->get_finite_difference_scheme_for_forward_analysis();

                        // get the finite difference perturbation size
                        mFAFDPerturbation = mSet->get_finite_difference_perturbation_size_forward();

                        m_compute_jacobian = &Element::select_jacobian_FD;
                        m_compute_dQIdu    = &Element::select_dQIdu_FD;
                    }

                    // get bool for sensitivity analysis evaluation type
                    bool tIsAnalyticalSA = mSet->get_is_analytical_sensitivity_analysis();

                    if( tIsAnalyticalSA )
                    {
                        m_compute_dRdp  = &Element::select_dRdp;
                        m_compute_dQIdp = &Element::select_dQIdp;
                    }
                    else
                    {
                        m_compute_dRdp  = &Element::select_dRdp_FD;
                        m_compute_dQIdp = &Element::select_dQIdp_FD;

                        // get finite difference scheme type
                        mSAFDScheme = mSet->get_finite_difference_scheme_for_sensitivity_analysis();

                        // get the finite difference perturbation size
                        mSAFDPerturbation = mSet->get_finite_difference_perturbation_size();
                    }
                }

                //------------------------------------------------------------------------------
                /**
                 * select jacobian
                 */
                void select_jacobian(
                        const std::shared_ptr< IWG > & aReqIWG,
                        real                           aWStar )
                {
                    // compute Jacobian
                    aReqIWG->compute_jacobian( aWStar );
                }

                void select_jacobian_FD(
                        const std::shared_ptr< IWG > & aReqIWG,
                        real                           aWStar )
                {
                    // compute Jacobian
                    aReqIWG->compute_jacobian_FD( aWStar, mFAFDPerturbation, mFAFDScheme );
                }

                //------------------------------------------------------------------------------
                /**
                 * select dQIdu
                 */
                void select_dQIdu(
                        const std::shared_ptr< IQI > & aReqIQI,
                        real                           aWStar )
                {
                    // compute dQIdu
                    aReqIQI->compute_dQIdu( aWStar );
                }

                void select_dQIdu_FD(
                        const std::shared_ptr< IQI > & aReqIQI,
                        real                           aWStar )
                {
                    // compute dQIdu FD
                    aReqIQI->compute_dQIdu_FD( aWStar, mFAFDPerturbation, mFAFDScheme );
                }

                //------------------------------------------------------------------------------
                /**
                 * select dRdp
                 */
                void select_dRdp(
                        const std::shared_ptr< IWG >      & aReqIWG,
                        real                                aWStar,
                        Matrix< DDSMat >                  & aGeoLocalAssembly,
                        moris::Cell< Matrix< IndexMat > > & aVertexIndices )
                {
                    // compute Jacobian
                    aReqIWG->compute_jacobian( aWStar );
                }

                void select_dRdp_FD(
                        const std::shared_ptr< IWG >      & aReqIWG,
                        real                                aWStar,
                        Matrix< DDSMat >                  & aGeoLocalAssembly,
                        moris::Cell< Matrix< IndexMat > > & aVertexIndices )
                {
                    // compute dRdpMat at evaluation point
                    aReqIWG->compute_dRdp_FD_material(
                            aWStar,
                            mSAFDPerturbation,
                            mSAFDScheme );

                    // compute dRdpGeo at evaluation point
                    if( mSet->get_geo_pdv_assembly_flag() )
                    {
                        aReqIWG->compute_dRdp_FD_geometry(
                                aWStar,
                                mSAFDPerturbation,
                                mSAFDScheme,
                                aGeoLocalAssembly,
                                aVertexIndices );
                    }
                }

                //------------------------------------------------------------------------------
                /**
                 * select dQIdp
                 */
                void select_dQIdp(
                        const std::shared_ptr< IQI >      & aReqIQI,
                        real                                aWStar,
                        Matrix< DDSMat >                  & aGeoLocalAssembly,
                        moris::Cell< Matrix< IndexMat > > & aVertexIndices )
                {
                    // compute Jacobian
                    aReqIQI->compute_dQIdp( aWStar );
                }

                void select_dQIdp_FD(
                        const std::shared_ptr< IQI >      & aReqIQI,
                        real                                aWStar,
                        Matrix< DDSMat >                  & aGeoLocalAssembly,
                        moris::Cell< Matrix< IndexMat > > & aVertexIndices )
                {
                    // compute dQIdpMat at evaluation point
                    aReqIQI->compute_dQIdp_FD_material(
                            aWStar,
                            mSAFDPerturbation,
                            mSAFDScheme );

                    // compute dQIdpGeo at evaluation point
                    if( mSet->get_geo_pdv_assembly_flag() )
                    {
                        aReqIQI->compute_dQIdp_FD_geometry(
                                aWStar,
                                mSAFDPerturbation,
                                mSAFDScheme,
                                aGeoLocalAssembly,
                                aVertexIndices );
                    }
                }

                //------------------------------------------------------------------------------
                /**
                 * get mesh cell associated with the element
                 * param[ in ]  aIsMaster                 enum for master or slave
                 * param[ out ] mMasterCell or mSlaveCell a pointer to mtk cell
                 */
                const mtk::Cell * get_mtk_cell( mtk::Master_Slave aIsMaster = mtk::Master_Slave::MASTER )
                {
                    switch( aIsMaster )
                    {
                        case mtk::Master_Slave::MASTER :
                            return mMasterCell;

                        case mtk::Master_Slave::SLAVE :
                            return mSlaveCell;

                        default:
                            MORIS_ERROR( false, "Element::get_mtk_cell - can only be master or slave." );
                            return mMasterCell;
                    }
                }

                //------------------------------------------------------------------------------
                /**
                 * compute jacobian
                 */
                virtual void compute_jacobian() = 0;

                //------------------------------------------------------------------------------
                /**
                 * compute residual
                 */
                virtual void compute_residual() = 0;

                //------------------------------------------------------------------------------
                /**
                 * compute jacobian and residual
                 */
                virtual void compute_jacobian_and_residual() = 0;

                //------------------------------------------------------------------------------
                /**
                 * compute dRdp
                 */
                virtual void compute_dRdp()
                {
                    MORIS_ERROR( false, "Element::compute_dRdp - Not implemented for base class." );
                }

                //------------------------------------------------------------------------------
                /**
                 * compute quantity of interest
                 */
                virtual void compute_QI()
                {
                    MORIS_ERROR( false, "Element::compute_QI - Not implemented for base class." );
                }

                //------------------------------------------------------------------------------
                /**
                 * compute dQIdp
                 */
                virtual void compute_dQIdp_explicit()
                {
                    MORIS_ERROR( false, "Element::compute_dQIdp_explicit - Not implemented for base class." );
                }

                //------------------------------------------------------------------------------
                /**
                 * compute dRdp and dQIdp by finite difference
                 */
                virtual void compute_dRdp_and_dQIdp()
                {
                    MORIS_ERROR( false, "Element::compute_dRdp_and_dQIdp_FD - Not implemented for base class." );
                }

                //------------------------------------------------------------------------------
                /**
                 * compute dQIdu
                 */
                virtual void compute_dQIdu()
                {
                    MORIS_ERROR( false, "Element::compute_dQIdu - Not implemented for base class." );
                }

                //------------------------------------------------------------------------------
                /**
                 * compute quantity of interest
                 * @param[ in ] aMeshIndex  an index for the used IG mesh
                 * @param[ in ] aFieldType  an enum for computation/field type
                 */
                void compute_quantity_of_interest(
                        const uint                         aMeshIndex,
                        enum vis::Field_Type               aFieldType )
                {
                    switch ( aFieldType )
                    {
                        case vis::Field_Type::GLOBAL :
                        {
                            this->compute_quantity_of_interest_global( aMeshIndex );
                            break;
                        }
                        case vis::Field_Type::ELEMENTAL :
                        {
                            this->compute_quantity_of_interest_elemental( aMeshIndex );
                            break;
                        }
                        default :
                        {
                            MORIS_ERROR( false, "Element::compute_quantity_of_interest - unknown field type." );
                        }
                    }
                }

                //------------------------------------------------------------------------------
                /**
                 * compute quantity of interest in a global way
                 * @param[ in ] aMeshIndex  an index for the used IG mesh
                 */
                virtual void compute_quantity_of_interest_global( const uint aMeshIndex )
                {
                    MORIS_ERROR( false, "Element::compute_quantity_of_interest_global - Not implemented for base class." );
                }

                //------------------------------------------------------------------------------
                /**
                 * compute quantity of interest in a nodal way
                 * @param[ in ] aMeshIndex  an index for the used IG mesh
                 */
                virtual void compute_quantity_of_interest_nodal( const uint aMeshIndex )
                {
                    MORIS_ERROR( false, "Element::compute_quantity_of_interest_nodal - Not implemented for base class." );
                }

                //------------------------------------------------------------------------------
                /**
                 * compute quantity of interest in an elemental way
                 * @param[ in ] aMeshIndex  an index for the used IG mesh
                 */
                virtual void compute_quantity_of_interest_elemental( const uint aMeshIndex )
                {
                    MORIS_ERROR( false, "Element::compute_quantity_of_interest_elemental - Not implemented for base class." );
                }

                //------------------------------------------------------------------------------
                /**
                 * compute volume of the integration element
                 * @param[ in ] aIsMaster enum master or slave
                 */
                virtual real compute_volume( mtk::Master_Slave aIsMaster = mtk::Master_Slave::MASTER )
                {
                    MORIS_ERROR( false, "Element::compute_volume - Not implemented for base class." );
                    return 0.0;
                }

                //------------------------------------------------------------------------------
                /**
                 * compute size of the integration element
                 * @param[ in ] aSpaceDim space dimension
                 * FIXME should be held by the FEM Set
                 * @param[ in ] aIsMaster enum master or slave
                 */
                real compute_size(
                        uint              aSpaceDim,
                        mtk::Master_Slave aIsMaster = mtk::Master_Slave::MASTER )
                {
                    //evaluate the volume
                    real tVolume = this->compute_volume( aIsMaster );

                    // compute the element size
                    switch ( aSpaceDim )
                    {
                        case 3 :
                            return std::pow( 6 * tVolume / M_PI, 1.0 / 3.0 );

                        case 2 :
                            return std::pow( 4 * tVolume / M_PI, 1.0 / 2.0 );

                        case 1 :
                            return tVolume;

                        default:
                            MORIS_ERROR( false, "Element::compute_size - space dimension can only be 1, 2, or 3. ");
                            return tVolume;
                    }
                }

                //------------------------------------------------------------------------------
        };

        //------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */

#endif /* SRC_FEM_CL_FEM_ELEMENT_HPP_ */