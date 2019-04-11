/*
 * cl_FEM_Element_Block.hpp
 *
 *  Created on: Apr 11, 2019
 *      Author: schmidt
 */
#include <iostream>

#include "cl_FEM_Element_Block.hpp"   //FEM/INT/src
#include "cl_FEM_Element_Factory.hpp"               //FEM/INT/src

namespace moris
{
    namespace fem
    {
//------------------------------------------------------------------------------
    Element_Block::Element_Block( moris::Cell< mtk::Cell* > & aCell,
                                 enum fem::Element_Type      aElementType,
                                 Cell< IWG* >              & aIWGs,
                                 Cell< Node_Base* >        & aNodes) : mMeshElementPointer(aCell),
                                                                       mNodes(aNodes),
                                                                       mIWGs( aIWGs ),
                                                                       mElementType(aElementType)
    {
        this->create_unique_dof_type_lists();
        this->create_unique_list_of_first_dof_type_of_group();
        this->create_dof_type_lists();
    }

//------------------------------------------------------------------------------

    Element_Block::~Element_Block()
    {
        this->delete_pointers();
    }

//------------------------------------------------------------------------------

    void Element_Block::delete_pointers()
    {
        // delete the geometry interpolator pointer
        if ( mGeometryInterpolator != nullptr )
        {
            delete mGeometryInterpolator;
        }

        mFieldInterpolators.clear();
    }

    void Element_Block::finalize()
    {
        this->delete_pointers();

        if( mMeshElementPointer.size() > 0)
        {
            Interpolation_Rule tGeometryInterpolationRule( mMeshElementPointer( 0 )->get_geometry_type(),       // FIXME change to block information
                                                            Interpolation_Type::LAGRANGE,
                                                            this->get_auto_interpolation_order(),           // FIXME change to block information
                                                            Interpolation_Type::LAGRANGE,
                                                            mtk::Interpolation_Order::LINEAR );

            bool tSpaceSideset = false;

            if (mElementType==fem::Element_Type::SIDESET)
            {
                tSpaceSideset=true;
            }

            // create the element geometry intepolator
            mGeometryInterpolator = new Geometry_Interpolator( tGeometryInterpolationRule, tSpaceSideset );

            // create the element field interpolators
            mFieldInterpolators = this->create_field_interpolators( mGeometryInterpolator );

            mElements.resize( mMeshElementPointer.size(), nullptr);

            // a factory to create the elements
            fem::Element_Factory tElementFactory;

            for( luint k=0; k < mMeshElementPointer.size(); ++k )
            {
                // create the element
                mElements( k ) = tElementFactory.create_element( mElementType,
                                                                 mMeshElementPointer( k ),
                                                                 mIWGs,
                                                                 mNodes,
                                                                 this );
            }
        }
    }

//------------------------------------------------------------------------------

    void Element_Block::create_unique_dof_type_lists()
    {
        // set the size of the element active dof type list
        uint tCounter = 0;
        for ( IWG * tIWG : mIWGs )
        {
            tCounter = tCounter + tIWG->get_residual_dof_type().size();
        }
        mEqnObjDofTypeList.reserve( tCounter );

        // loop over the IWGs
        tCounter = 0;
        for ( IWG * tIWG : mIWGs )
        {
            mEqnObjDofTypeList.append( tIWG->get_residual_dof_type() );
        }

        auto last = std::unique( ( mEqnObjDofTypeList.data() ).data(),
                                 ( mEqnObjDofTypeList.data() ).data() + mEqnObjDofTypeList.size() );
        auto pos  = std::distance( ( mEqnObjDofTypeList.data() ).data(), last );
        mEqnObjDofTypeList.resize( pos );
    }

    void Element_Block::create_unique_list_of_first_dof_type_of_group()
    {
//        // get the number of IWGs
//        uint tNumOfIWGs = this->get_num_IWG();
//
//        // create a list of the groups of dof types provided by the IWGs----------------
//        // FIXME works as long as the dof type are always grouped in the same way
//        moris::Cell< MSI::Dof_Type > tInterpDofTypeListBuild( tNumOfIWGs );
//
//        // loop over the IWGs
//        for ( uint i = 0; i < tNumOfIWGs; i++ )
//        {
//            // get the first dof type of each group
//            tInterpDofTypeListBuild( i ) = mIWGs( i )->get_residual_dof_type()( 0 );
//        }
//
//        // get a unique list of the first dof type of each group
//        Cell<moris::moris_index> tUniqueDofTypeGroupsIndices = unique_index( tInterpDofTypeListBuild );
//
//        // get the number of unique dof type groups
//        uint tNumOfUniqueDofTypeGroupsIndices = tUniqueDofTypeGroupsIndices.size();
//
//        // set the size of the list of unique dof type groups
//        mInterpDofTypeList.resize( tNumOfUniqueDofTypeGroupsIndices );
//
//        // loop over the list of unique dof type groups
//        for ( uint i = 0; i < tNumOfUniqueDofTypeGroupsIndices; i++ )
//        {
//            // get the unique residual dof type groups
//            mInterpDofTypeList( i ) = mIWGs( tUniqueDofTypeGroupsIndices( i ) )->get_residual_dof_type();
//        }
    }

//------------------------------------------------------------------------------

    void Element_Block::create_dof_type_lists()
    {
        // get the number of IWGs
        uint tNumOfIWGs = this->get_num_IWG();

        // create a list of the groups of dof types provided by the IWGs----------------
        // FIXME works as long as the dof type are always grouped in the same way
        moris::Cell< MSI::Dof_Type > tInterpDofTypeListBuild( tNumOfIWGs );

        // loop over the IWGs
        for ( uint i = 0; i < tNumOfIWGs; i++ )
        {
            // get the first dof type of each group
            tInterpDofTypeListBuild( i ) = mIWGs( i )->get_residual_dof_type()( 0 );
        }

        // get a unique list of the first dof type of each group
        Cell<moris::moris_index> tUniqueDofTypeGroupsIndices = unique_index( tInterpDofTypeListBuild );

        // get the number of unique dof type groups
        uint tNumOfUniqueDofTypeGroupsIndices = tUniqueDofTypeGroupsIndices.size();

        // set the size of the list of unique dof type groups
        mInterpDofTypeList.resize( tNumOfUniqueDofTypeGroupsIndices );

        // loop over the list of unique dof type groups
        for ( uint i = 0; i < tNumOfUniqueDofTypeGroupsIndices; i++ )
        {
            // get the unique residual dof type groups
            mInterpDofTypeList( i ) = mIWGs( tUniqueDofTypeGroupsIndices( i ) )->get_residual_dof_type();
        }

        // create a map of the element active dof type list------------------------
        // set number of unique pdof type of the element
        mNumOfInterp = tNumOfUniqueDofTypeGroupsIndices;

        // get maximal dof type enum number
        sint tMaxDofTypeEnumNumber = 0;

        // loop over all pdof types to get the highest enum index
        for ( uint i = 0; i < mNumOfInterp; i++ )
        {
            tMaxDofTypeEnumNumber = std::max( tMaxDofTypeEnumNumber, static_cast< int >( mInterpDofTypeList( i )( 0 ) ) );
        }

        // +1 because c++ is 0 based
        tMaxDofTypeEnumNumber = tMaxDofTypeEnumNumber + 1;

        // set size of mapping matrix
        mInterpDofTypeMap.set_size( tMaxDofTypeEnumNumber, 1, -1 );

        // loop over all dof types to create the mapping matrix
        for ( uint i = 0; i < mNumOfInterp; i++ )
        {
            mInterpDofTypeMap( static_cast< int >( mInterpDofTypeList( i )( 0 ) ), 0 ) = i;
        }
    }

//------------------------------------------------------------------------------

    mtk::Interpolation_Order Element_Block::get_auto_interpolation_order()
    {
        switch( mMeshElementPointer( 0 )->get_geometry_type() )                                 // FIXME change to block information
        {
            case( mtk::Geometry_Type::LINE ) :
                switch( mMeshElementPointer( 0 )->get_number_of_vertices() )
                {
                   case( 2 ) :
                       return mtk::Interpolation_Order::LINEAR;
                       break;

                   case( 3 ) :
                       return mtk::Interpolation_Order::QUADRATIC;
                       break;

                   default :
                       MORIS_ERROR( false, " Element::get_auto_interpolation_order - not defined for LINE and number of vertices. ");
                       return mtk::Interpolation_Order::UNDEFINED;
                       break;
                }

            case( mtk::Geometry_Type::QUAD ) :
                switch( mMeshElementPointer( 0 )->get_number_of_vertices() )
                {
                    case( 4 ) :
                        return mtk::Interpolation_Order::LINEAR;
                        break;

                    case( 8 ) :
                        return mtk::Interpolation_Order::SERENDIPITY;
                        break;

                    case( 9 ) :
                        return mtk::Interpolation_Order::QUADRATIC;
                        break;

                    case( 16 ) :
                        return mtk::Interpolation_Order::CUBIC;
                        break;

                    default :
                        MORIS_ERROR( false, " Element::get_auto_interpolation_order - not defined for QUAD and number of vertices. ");
                        return mtk::Interpolation_Order::UNDEFINED;
                        break;
                }

            case( mtk::Geometry_Type::HEX ) :
                switch( mMeshElementPointer( 0 )->get_number_of_vertices() )
                {
                    case( 8 ) :
                        return mtk::Interpolation_Order::LINEAR;
                        break;

                    case( 20 ) :
                        return mtk::Interpolation_Order::SERENDIPITY;
                        break;

                    case( 27 ) :
                        return mtk::Interpolation_Order::QUADRATIC;
                        break;

                    case( 64 ) :
                        return mtk::Interpolation_Order::CUBIC;
                        break;

                    default :
                        MORIS_ERROR( false, " Element::get_auto_interpolation_order - not defined for HEX and number of vertices. ");
                        return mtk::Interpolation_Order::UNDEFINED;
                        break;
                }

            default :
                MORIS_ERROR( false, " Element::get_auto_interpolation_order - not defined for this geometry type. ");
                return mtk::Interpolation_Order::UNDEFINED;
                break;
        }
    }

//------------------------------------------------------------------------------

    Cell< Field_Interpolator* > Element_Block::create_field_interpolators( Geometry_Interpolator* aGeometryInterpolator )
    {
        // cell of field interpolators
        Cell< Field_Interpolator* > tFieldInterpolators( mNumOfInterp, nullptr );

        // loop on the dof type groups and create a field interpolator for each
        for( uint i = 0; i < mNumOfInterp; i++ )
        {
            // get the ith dof type group
            Cell< MSI::Dof_Type > tDofTypeGroup = mInterpDofTypeList( i );

            // create the field interpolation rule for the ith dof type group
            //FIXME: space interpolation based on the mtk::Cell
            //FIXME: time  interpolation set to constant
            Interpolation_Rule tFieldInterpolationRule( mMeshElementPointer( 0 )->get_geometry_type(),           //FIXME
                                                        Interpolation_Type::LAGRANGE,
                                                        this->get_auto_interpolation_order(),
                                                        Interpolation_Type::CONSTANT,
                                                        mtk::Interpolation_Order::CONSTANT );

            // get number of field interpolated by the ith field interpolator
            uint tNumOfFields = tDofTypeGroup.size();

            // create an interpolator for the ith dof type group
            tFieldInterpolators( i ) = new Field_Interpolator( tNumOfFields,
                                                               tFieldInterpolationRule,
                                                               aGeometryInterpolator );
        }
        return tFieldInterpolators;
    }

//------------------------------------------------------------------------------

    } /* namespace fem */
} /* namespace moris */
