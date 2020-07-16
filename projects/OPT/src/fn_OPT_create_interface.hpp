#ifndef MORIS_FN_OPT_CREATE_INTERFACE_HPP
#define MORIS_FN_OPT_CREATE_INTERFACE_HPP

#include "cl_OPT_Criteria_Interface.hpp"
#include "cl_Param_List.hpp"

namespace moris
{
    namespace opt
    {
        /**
         * Creates an instance of an Interface class or Interface_Manager and returns a shared pointer to it.
         *
         * @param aParameterLists Parameter lists for individual interfaces
         * @return Interface class (can be manager)
         */
        std::shared_ptr<Criteria_Interface> create_interface(
                Cell<ParameterList> aParameterLists,
                Cell<std::shared_ptr<Criteria_Interface>> aInterfaces = Cell<std::shared_ptr<Criteria_Interface>>(0));

        /**
         * Creates an instance of the specified Interface class and returns a shared pointer to it
         *
         * @param aParameterList A single interface parameter list
         * @return Interface class
         */
        std::shared_ptr<Criteria_Interface> create_interface(ParameterList aParameterList);

    }
}

#endif //MORIS_FN_OPT_CREATE_INTERFACE_HPP
