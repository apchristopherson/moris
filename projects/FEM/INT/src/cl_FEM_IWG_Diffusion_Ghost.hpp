/*
 * cl_FEM_IWG_Diffusion_Ghost.hpp
 *
 *  Created on: May 2, 2019
 *      Author: wunsch/noel
 */

#ifndef SRC_FEM_CL_FEM_IWG_Diffusion_Ghost_HPP_
#define SRC_FEM_CL_FEM_IWG_Diffusion_Ghost_HPP_

#include <map>
//MRS/COR/src
#include "typedefs.hpp"
#include "cl_Cell.hpp"
//LINALG/src
#include "cl_Matrix.hpp"
#include "linalg_typedefs.hpp"
//FEM/INT/src
#include "cl_FEM_Field_Interpolator.hpp"
#include "cl_FEM_IWG.hpp"

namespace moris
{
    namespace fem
    {
        //------------------------------------------------------------------------------

        class IWG_Diffusion_Ghost : public IWG
        {
            private:

                // interpolation order
                uint mOrder;

                //------------------------------------------------------------------------------
            public:

                enum class IWG_Stabilization_Type
                {
                    GHOST_DISPL,
                    MAX_ENUM
                };

                // Local string to constitutive enum map
                std::map< std::string, IWG_Stabilization_Type > mStabilizationMap;

                //------------------------------------------------------------------------------
                /*
                 *  constructor
                 */
                IWG_Diffusion_Ghost();

                //------------------------------------------------------------------------------
                /**
                 * trivial destructor
                 */
                ~IWG_Diffusion_Ghost(){};

                //------------------------------------------------------------------------------
                /**
                 * set stabilization parameter
                 * @param[ in ] aStabilizationParameter a stabilization parameter pointer
                 * @param[ in ] aStabilizationString    a string defining the stabilization parameter
                 */
                void set_stabilization_parameter(
                        std::shared_ptr< Stabilization_Parameter > aStabilizationParameter,
                        std::string                                aStabilizationString );

                //------------------------------------------------------------------------------
                /**
                 * compute the residual
                 * @param[ in ] aResidual cell of residual vectors to fill
                 */
                void compute_residual( real aWStar );

                //------------------------------------------------------------------------------
                /**
                 * compute the jacobian
                 * @param[ in ] aWStar weight associated to the evaluation point
                 */
                void compute_jacobian( real aWStar );

                //------------------------------------------------------------------------------
                /**
                 * compute the residual and the jacobian
                 * @param[ in ] aWStar weight associated to the evaluation point
                 */
                void compute_jacobian_and_residual( real aWStar );

                //------------------------------------------------------------------------------
                /**
                 * compute the derivative of the residual wrt design variables
                 * @param[ in ] aWStar weight associated to the evaluation point
                 */
                void compute_dRdp( real aWStar );

                //------------------------------------------------------------------------------
                /**
                 * method to assemble "normal matrix" from normal vector needed for
                 * 2nd and 3rd order Ghost formulations
                 * @param[ in ] aOrderGhost Order of derivatives and ghost formulation
                 */
                Matrix< DDRMat > get_normal_matrix ( uint aOrderGhost );

                //------------------------------------------------------------------------------
        };
        //------------------------------------------------------------------------------
    } /* namespace fem */
} /* namespace moris */

#endif /* SRC_FEM_CL_FEM_IWG_Diffusion_Ghost_HPP_ */