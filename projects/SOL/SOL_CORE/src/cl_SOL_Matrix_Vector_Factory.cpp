/*
 * cl_Matrix_Vector_Factory.cpp
 *
 *  Created on: Jun 28, 2018
 *      Author: schmidt
 */
#include "cl_SOL_Matrix_Vector_Factory.hpp"

#include "cl_Sparse_Matrix_EpetraFECrs.hpp"
#include "cl_MatrixPETSc.hpp"
#include "cl_Vector_Epetra.hpp"
#include "cl_Vector_PETSc.hpp"
#include "cl_Map_Epetra.hpp"
#include "cl_Map_PETSc.hpp"
#include "cl_SOL_Dist_Map.hpp"

namespace moris
{
    namespace sol
    {
        sol::Matrix_Vector_Factory::Matrix_Vector_Factory( const enum MapType aMapType )
        {
            mMapType = aMapType;
        }

        Dist_Matrix * sol::Matrix_Vector_Factory::create_matrix(
                Solver_Interface * aInput,
                std::shared_ptr<Dist_Map> aMap )
        {
            Dist_Matrix * tSparseMatrix = nullptr;

            switch( mMapType )
            {
                case (MapType::Epetra):
                    tSparseMatrix = new Sparse_Matrix_EpetraFECrs( aInput, aMap );
                    break;
                case (MapType::Petsc):
                    tSparseMatrix = new Matrix_PETSc( aInput, aMap );
                    break;
                default:
                    MORIS_ERROR( false, "No matrix type specified." );
                    break;
            }
            return tSparseMatrix;
        }

        Dist_Matrix * sol::Matrix_Vector_Factory::create_matrix(
                std::shared_ptr<Dist_Map> aRowMap,
                std::shared_ptr<Dist_Map> aColMap )
        {
            Dist_Matrix * tSparseMatrix = nullptr;

            switch( mMapType )
            {
                case (MapType::Epetra):
                    tSparseMatrix = new Sparse_Matrix_EpetraFECrs( aRowMap, aColMap );
                    break;
                    //case (MapType::Petsc):
                    //    tSparseMatrix = new Matrix_PETSc( aInput, aMap );
                    //    break;
                default:
                    MORIS_ERROR( false, "No matrix type specified." );
                    break;
            }
            return tSparseMatrix;
        }

        Dist_Matrix * sol::Matrix_Vector_Factory::create_matrix(
                const uint aRows,
                const uint aCols )
        {
            Dist_Matrix * tSparseMatrix = nullptr;

            switch( mMapType )
            {
                case (MapType::Epetra):
                    tSparseMatrix = new Sparse_Matrix_EpetraFECrs( aRows, aCols );
                    break;
                case (MapType::Petsc):
                    tSparseMatrix = new Matrix_PETSc( aRows, aCols );
                    break;
                default:
                    MORIS_ERROR( false, "No matrix type specified." );
                    break;
            }
            return tSparseMatrix;
        }

        //-------------------------------------------------------------------------------------------------
        Dist_Vector * sol::Matrix_Vector_Factory::create_vector(
                Solver_Interface * aInput,
                std::shared_ptr<Dist_Map> aMap,
                const sint                      aNumVectors )
        {
            Dist_Vector * tDistVector = nullptr;

            switch( mMapType )
            {
                case (MapType::Epetra):
                    tDistVector = new Vector_Epetra( aMap, aNumVectors );
                    break;
                case (MapType::Petsc):
                    MORIS_ERROR( aNumVectors == 1, "Multivector not implemented for petsc");
                    tDistVector = new Vector_PETSc( aInput, aMap, aNumVectors );
                    break;
                default:
                    MORIS_ERROR( false, "No vector type specified." );
                    break;
            }
            return tDistVector;
        }
        //-------------------------------------------------------------------------------------------------
        Dist_Vector * sol::Matrix_Vector_Factory::create_vector(
                std::shared_ptr<Dist_Map> aMap,
                const sint aNumVectors )
        {
            Dist_Vector * tDistVector = nullptr;

            switch( mMapType )
            {
                case (MapType::Epetra):
                    tDistVector = new Vector_Epetra( aMap, aNumVectors );
                    break;
                    //    case (MapType::Petsc):
                    //        MORIS_ERROR( aNumVectors == 1, "Multivector not implemented for petsc");
                    //        tDistVector = new Vector_PETSc( aInput, aMap, aNumVectors );
                    //        break;
                default:
                    MORIS_ERROR( false, "No vector type specified." );
                    break;
            }
            return tDistVector;
        }

        //-------------------------------------------------------------------------------------------------
        std::shared_ptr<Dist_Map> sol::Matrix_Vector_Factory::create_map(
                const Matrix< DDSMat > & aMyGlobalIds,
                const Matrix< DDUMat > & aMyConstraintIds )
        {
            std::shared_ptr<Dist_Map> tMap = nullptr;

            switch( mMapType )
            {
                case (MapType::Epetra):
                    tMap = std::make_shared<Map_Epetra>( aMyGlobalIds, aMyConstraintIds );
                    break;
                case (MapType::Petsc):
                    tMap = std::make_shared<Map_PETSc>( aMyGlobalIds, aMyConstraintIds );
                    break;
                default:
                    MORIS_ERROR( false, "Matrix_Vector_Factory::create_map(), map type not specified" );
                    break;
            }
            return tMap;
        }

        //-------------------------------------------------------------------------------------------------
        std::shared_ptr<Dist_Map> sol::Matrix_Vector_Factory::create_map( const Matrix< DDSMat > & aMyGlobalIds )
        {
            std::shared_ptr<Dist_Map> tMap = nullptr;

            switch( mMapType )
            {
                case (MapType::Epetra):
                    tMap = std::make_shared<Map_Epetra>( aMyGlobalIds );
                    break;
                case (MapType::Petsc):
                    tMap = std::make_shared<Map_PETSc>( aMyGlobalIds );
                    break;
                default:
                    MORIS_ERROR( false, "No map type specified" );
                    break;
            }
            return tMap;
        }
    }
}

