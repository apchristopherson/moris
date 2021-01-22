#include "cl_Stopwatch.hpp" //CHR/src

// fixme: temporary
#include "cl_Map.hpp"
#include "fn_unique.hpp"
#include "fn_sum.hpp" // for check
#include "fn_iscol.hpp"
#include "fn_trans.hpp"
#include "op_equal_equal.hpp"

#include "MTK_Tools.hpp"
#include "cl_MTK_Enums.hpp"
#include "cl_MTK_Mesh_Manager.hpp"
#include "cl_MTK_Integration_Mesh.hpp"
#include "cl_MTK_Writer_Exodus.hpp"

#include "cl_HMR.hpp"

#include "cl_MDL_Model.hpp"

#include "cl_GEN_Geometry_Engine.hpp"

#include "cl_XTK_Model.hpp"
#include "cl_XTK_Enriched_Integration_Mesh.hpp"

#include "cl_WRK_Performer_Manager.hpp"

#include "fn_Exec_load_user_library.hpp"

namespace moris
{
    namespace wrk
    {
        //------------------------------------------------------------------------------

        Performer_Manager::Performer_Manager( std::shared_ptr< Library_IO > aLibrary )
        : mLibrary(aLibrary)
        {
        }

        //------------------------------------------------------------------------------

        Performer_Manager::~Performer_Manager()
        {
        }

        //------------------------------------------------------------------------------
    } /* namespace mdl */
} /* namespace moris */
