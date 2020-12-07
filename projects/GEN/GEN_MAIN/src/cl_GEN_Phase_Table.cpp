#include "cl_GEN_Phase_Table.hpp"
#include "fn_linspace.hpp"

namespace moris
{
    namespace ge
    {
        //--------------------------------------------------------------------------------------------------------------

        Phase_Table::Phase_Table(
                uint              aNumGeometries,
                Matrix<DDUMat>    aBulkPhases,
                Cell<std::string> aPhaseNames)
        {
            this->initialize(aNumGeometries,aBulkPhases,aPhaseNames);
        }

        //--------------------------------------------------------------------------------------------------------------

        Phase_Table::Phase_Table(
                uint              aNumGeometries,
                Cell<std::string> aPhaseNames)
        {
            this->initialize(
                    aNumGeometries,
                    linspace<uint>(0, std::pow(2, aNumGeometries) - 1,std::pow(2, aNumGeometries)),
                    aPhaseNames);
        }

        //--------------------------------------------------------------------------------------------------------------

        Phase_Table::Phase_Table(
                uint                     aNumPhases,
                MORIS_GEN_PHASE_FUNCTION aPhaseFunction,
                Cell<std::string>        aPhaseNames)
        {
            this->initialize(
                    0,
                    {{aNumPhases - 1}},
                    aPhaseNames);

            mPhaseFunction = aPhaseFunction;
        }

       //--------------------------------------------------------------------------------------------------------------

        void Phase_Table::initialize(
                uint              aNumGeometries,
                Matrix<DDUMat>    aBulkPhases,
                Cell<std::string> aPhaseNames)
        {
            mNumGeometries= aNumGeometries;
            mBulkPhases   = aBulkPhases;
            mPhaseNames   = aPhaseNames;

            if (mBulkPhases.numel() > 0)
            {
                // Number of phases
                mNumPhases = mBulkPhases.max() + 1;

                // Default phase names
                if (mPhaseNames.size() == 0)
                {
                    this->set_default_phase_names();
                }

                // Check for phase table size
                MORIS_ERROR(mBulkPhases.length() == std::pow(2, mNumGeometries),
                        "Must provide bulk phase information for each of the 2^n geometry combinations.");
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        void Phase_Table::initialize(
                uint              aNumGeometries,
                Cell<std::string> aPhaseNames)
        {
            this->initialize(
                    aNumGeometries,
                    linspace<uint>(0, std::pow(2, aNumGeometries) - 1,std::pow(2, aNumGeometries)),
                    aPhaseNames);
        }

        //--------------------------------------------------------------------------------------------------------------

        void Phase_Table::initialize(
                uint                     aNumPhases,
                MORIS_GEN_PHASE_FUNCTION aPhaseFunction,
                Cell<std::string>        aPhaseNames)
        {
            mPhaseFunction = aPhaseFunction;

            this->initialize(
                    0,
                    {{aNumPhases - 1}},
                    aPhaseNames);
        }

        //--------------------------------------------------------------------------------------------------------------

        uint Phase_Table::get_num_phases()
        {
            return mNumPhases;
        }

        //--------------------------------------------------------------------------------------------------------------

        uint Phase_Table::get_phase_index(const Bitset<512>& aGeometrySigns)
        {
            if (mPhaseFunction)
            {
                return mPhaseFunction(aGeometrySigns);
            }
            else
            {
                uint tPhaseEntry = 0;
                for (uint tGeometryIndex = 0; tGeometryIndex < mNumGeometries; tGeometryIndex++)
                {
                    tPhaseEntry += std::pow(2, mNumGeometries) / std::pow(2, tGeometryIndex + 1) * aGeometrySigns.test(tGeometryIndex);
                }

                return mBulkPhases(tPhaseEntry);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

        std::string Phase_Table::get_phase_name(uint aPhaseIndex)
        {
            MORIS_ASSERT(aPhaseIndex < mPhaseNames.size(), "Phase index out of bounds");
            return mPhaseNames(aPhaseIndex);
        }

        //--------------------------------------------------------------------------------------------------------------

        void Phase_Table::print()
        {
            // Print table information
            std::cout << "Phase Table Info:" << std::endl;
            std::cout << "  Number of Geometries:  "  << mNumGeometries << std::endl;
            std::cout << "  Number of Bulk Phases: " << mNumPhases << std::endl;

            // Print table header
            std::cout << std::setw(8) << "Entry"  << " | "
                    << std::setw(8) << "BP Index" << " | "
                    << std::setw(8) << "BP Name" << " | " ;

            // Print geometry index
            for (uint tGeometryIndex  = 0; tGeometryIndex < mNumGeometries; tGeometryIndex++)
            {
                std::cout << std::setw(8) << "Geom " + std::to_string(tGeometryIndex)  <<  " | ";
            }
            std::cout << std::endl;

            // Print bulk phase information
            for (uint tPhaseEntry = 0; tPhaseEntry < mBulkPhases.length(); tPhaseEntry++)
            {
                // Print phase entry
                std::cout << std::setw(8) << tPhaseEntry << " | "
                        << std::setw(8) << mBulkPhases(tPhaseEntry) << " | "
                        << std::setw(8) << mPhaseNames(mBulkPhases(tPhaseEntry)).substr(0, 8) << " | ";

                // Geometry signs (reversed)
                Bitset<512> tGeometrySigns(tPhaseEntry);

                // Print geometry signs
                for (uint tGeometryIndex = mNumGeometries; tGeometryIndex-- > 0;)
                {
                    if (tGeometrySigns.test(tGeometryIndex))
                    {
                        std::cout << std::setw(8) << "+" << " | ";
                    }
                    else
                    {
                        std::cout << std::setw(8) << "-" << " | ";
                    }
                }
                std::cout << std::endl;
            }

            std::cout << "  +   -> greater than threshold"  <<  std::endl;
            std::cout << "  -   -> less than threshold"     <<  std::endl;
        }

        //--------------------------------------------------------------------------------------------------------------

        void Phase_Table::set_default_phase_names()
        {
            mPhaseNames = Cell<std::string>(mNumPhases);
            std::string tBase = "p_";

            for (uint tPhaseIndex = 0; tPhaseIndex < mNumPhases; tPhaseIndex++)
            {
                mPhaseNames(tPhaseIndex) = tBase + std::to_string(tPhaseIndex);
            }
        }

        //--------------------------------------------------------------------------------------------------------------

    }
}
