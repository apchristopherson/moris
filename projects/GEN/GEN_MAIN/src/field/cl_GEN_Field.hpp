#ifndef MORIS_CL_GEN_Field_HPP
#define MORIS_CL_GEN_Field_HPP

#include "st_GEN_Field_Parameters.hpp"
#include "cl_GEN_Child_Node.hpp"
#include "cl_SOL_Dist_Vector.hpp"
#include "cl_MTK_Interpolation_Mesh.hpp"

namespace moris
{
    namespace ge
    {
        class Field
        {

        protected:
            Cell<real*>    mFieldVariables;
            Matrix<DDRMat> mSensitivities;

        private:
            Matrix<DDRMat>    mConstants;
            Field_Parameters  mParameters;
            Matrix<DDSMat>    mDeterminingADVIds;
            bool              mDependsOnADVs;
            sol::Dist_Vector* mSharedADVs = nullptr;

        protected:

            /**
             * Constructor, sets the field variable pointers to ADVs and constant parameters for evaluations.
             *
             * @tparam Vector_Type Type of vector where ADVs are stored
             * @param aADVs ADV vector
             * @param aFieldVariableIndices Indices of field variables to be filled by the ADVs
             * @param aADVIndices The indices of the ADV vector to fill in the field variables
             * @param aConstants The constant field variables not filled by ADVs
             * @param aParameters Additional parameters
             */
            template <typename Vector_Type>
            Field(Vector_Type&     aADVs,
                  Matrix<DDUMat>   aFieldVariableIndices,
                  Matrix<DDUMat>   aADVIndices,
                  Matrix<DDRMat>   aConstants,
                  Field_Parameters aParameters);

            /**
             * Constructor, sets all field variables as consecutive ADVs. A reference field is given for parameters.
             *
             * @param aSharedADVIds Shared ADV IDs needed for this field
             * @param aField Reference field
             */
            Field(const Matrix<DDSMat>&  aSharedADVIds,
                  std::shared_ptr<Field> aField);

            /**
             * Constructor using only constants (no ADVs).
             *
             * @param aConstants The parameters that define this field
             * @param aParameters Additional parameters
             */
            Field(Matrix<DDRMat>   aConstants,
                  Field_Parameters aParameters);

            /**
             * Copy constructor from a shared pointer.
             *
             * @param aField Field being copied
             */
            Field(std::shared_ptr<Field> aField);

            /**
             * Default constructor
             *
             * @error This only exists so the Blanca compiler does not complain, <b>DO NOT USE</b>.
             */
            Field();

        public:

            /**
             * Destructor
             */
            ~Field();

            /**
             * Given a node index or coordinate, returns the field value.
             *
             * @param aNodeIndex Node index
             * @param aCoordinates Vector of coordinate values
             * @return Field value
             */
            virtual real get_field_value(
                    uint                  aNodeIndex,
                    const Matrix<DDRMat>& aCoordinates) = 0;

            /**
             * Given a node index or coordinate, returns a matrix of all sensitivities.
             *
             * @param aNodeIndex Node index
             * @param aCoordinates Vector of coordinate values
             * @return Matrix of sensitivities
             */
            virtual const Matrix<DDRMat>& get_field_sensitivities(
                    uint                  aNodeIndex,
                    const Matrix<DDRMat>& aCoordinates) = 0;

            /**
             * Sets the ADVs and grabs the field variables needed from the ADV vector
             *
             * @tparam Vector_Type Type of vector where ADVs are stored
             * @param aADVs ADVs
             */
            template <typename Vector_Type>
            void set_advs(Vector_Type& aADVs);

            /**
             * Imports the local ADVs required from the full owned ADV distributed vector.
             *
             * @param aOwnedADVs Full owned distributed ADV vector
             */
            virtual void import_advs(sol::Dist_Vector* aOwnedADVs);

            /**
             * Add a new child node for evaluation, implemented for discrete fields
             *
             * @param aNodeIndex Index of the child node
             * @param aChildNode Contains information about how the child node was created
             */
            virtual void add_child_node(uint aNodeIndex, std::shared_ptr<Child_Node> aChildNode);

            /**
             * Resets all nodal information, called when a new XTK mesh is being created.
             */
            virtual void reset_nodal_information();

            /**
             * Gets if this field is to be turned into a stored geometry/property, in order to store field values.
             *
             * @return Logic for storing field values
             */
            bool intended_storage();

            /**
             * Gets if this field is to be used for seeding a B-spline field.
             *
             * @return Logic for B-spline creation
             */
            bool intended_discretization();

            /**
             * Gets the IDs of ADVs which this field depends on for evaluations.
             *
             * @param aNodeIndex Node index
             * @param aCoordinates Node coordinates
             * @return Determining ADV IDs at this node
             */
            virtual Matrix<DDSMat> get_determining_adv_ids(uint aNodeIndex, const Matrix<DDRMat>& aCoordinates);

            /**
             * If this field depends on ADVs
             *
             * @return if this field has ADV indices
             */
            bool depends_on_advs();

            /**
             * Gets the name of this field.
             *
             * @return Name
             */
            std::string get_name();

            /**
             * This function will return true when called less than the number of refinements set for this field,
             * and false otherwise. This is to determine for a given refinement call if this field needs refinement.
             *
             * @return if to perform an additional refinement with this field
             */
            const Matrix< DDSMat > & get_num_refinements();

            const Matrix< DDSMat > & get_refinement_mesh_indices();

            /**
             * Gets the index of a user-defined refinement function used within HMR.
             *
             * @return User-defined refinement function index
             */
            sint get_refinement_function_index();

            /**
             * Gets a discretization mesh index for a discretized field.
             *
             * @return Mesh index
             */
            uint get_discretization_mesh_index();

            /**
             * Gets the lower bound for a discretized field.
             *
             * @return Lower bound
             */
            real get_discretization_lower_bound();

            /**
             * Get the upper bound for a discretized field.
             *
             * @return Upper bound
             */
            real get_discretization_upper_bound();

            /**
             * Gets the mesh that this field depends on.
             *
             * @return Mesh, default nullptr
             */
            virtual mtk::Interpolation_Mesh* get_mesh();

        private:

            /**
             * Checks variable inputs and resizes the internal field variables based these inputs.
             *
             * @param aFieldVariableIndices Indices of Field variables to be filled by the ADVs
             * @param aADVIndices The indices of the ADV vector to fill in the Field variables
             */
            void assign_adv_dependencies(
                    Matrix<DDUMat> aFieldVariableIndices,
                    Matrix<DDUMat> aADVIndices);
            
            /**
             * Fills the remaining field variables with constant parameters.
             */
            void fill_constant_parameters();

        };
    }
}


#endif //MORIS_CL_GEN_Field_HPP
