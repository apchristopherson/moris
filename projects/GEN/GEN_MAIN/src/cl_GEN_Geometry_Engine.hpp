#ifndef MORIS_CL_Geometry_Engine_HPP_
#define MORIS_CL_Geometry_Engine_HPP_

// MRS
#include "cl_Param_List.hpp"
#include "fn_Exec_load_user_library.hpp"
#include "fn_trans.hpp"

// WRK
#include "cl_WRK_Performer.hpp"

// GEN
#include "cl_GEN_Geometry.hpp"
#include "cl_GEN_Property.hpp"
#include "cl_GEN_Phase_Table.hpp"
#include "pdv/cl_GEN_Pdv_Host_Manager.hpp"
#include "cl_GEN_Pdv_Enums.hpp"

// MTK
#include "cl_MTK_Mesh_Core.hpp"
#include "cl_MTK_Cluster.hpp"
#include "cl_MTK_Mesh_Manager.hpp"
#include "cl_Mesh_Enums.hpp"

namespace xtk
{
    class Topology;
}

namespace moris
{

    //------------------------------------------------------------------------------------------------------------------

    namespace MSI
    {
        class Design_Variable_Interface;
    }

    //------------------------------------------------------------------------------------------------------------------

    namespace ge
    {

        class Geometry_Engine : public wrk::Performer
        {
        private:

            // Level-set
            real mIsocontourThreshold;
            real mErrorFactor;
            Matrix<DDUMat> mBSplineMeshIndices;
            std::string mLevelSetFile = "";

            // Spatial dimensions
            uint mSpatialDim;

            // ADVs/IQIs
            Matrix<DDRMat> mADVs;
            Matrix<DDRMat> mLowerBounds;
            Matrix<DDRMat> mUpperBounds;
            Cell<std::string> mRequestedIQIs;
            bool mShapeSensitivities = false;

            // Library
            std::shared_ptr<Library_IO> mLibrary;

            // Geometry
            size_t mActiveGeometryIndex = 0;
            Cell<std::shared_ptr<Geometry>> mGeometries;
            Cell<ParameterList> mGeometryParameterLists;

            // Property
            Cell<std::shared_ptr<Property>> mProperties;
            Cell<ParameterList> mPropertyParameterLists;

            // PDVs
            Pdv_Host_Manager mPdvHostManager;
            std::shared_ptr<Intersection_Node> mQueuedIntersectionNode;

            // Phase Table
            Phase_Table mPhaseTable;

        public:

            /**
             * Constructor using cell of cell of parameter lists
             *
             * @param aParameterLists GEN parameter lists (see fn_PRM_GEN_Parameters.hpp)
             * @param aLibrary Library used for pulling user-defined functions
             */
            Geometry_Engine(Cell< Cell<ParameterList> > aParameterLists,
                            std::shared_ptr<Library_IO> aLibrary = nullptr);

            /**
             * Constructor using externally-created geometry and phase table
             *
             * @param aGeometry Geometry instances to use
             * @param aPhaseTable Phase table for determining bulk phases
             * @param aMesh Mesh for computing level-set values
             * @param aADVs ADV vector
             * @param aIsocontourThreshold Threshold for setting the level-set isocontour
             * @param aErrorFactor Error factor for determining if a node is on an interface
             */
            Geometry_Engine(Cell< std::shared_ptr<Geometry> > aGeometry,
                            Phase_Table                       aPhaseTable,
                            mtk::Interpolation_Mesh*          aMesh,
                            Matrix<DDRMat>                    aADVs = {{}},
                            real                              aIsocontourThreshold = 0.0,
                            real                              aErrorFactor = 0.0);

            /**
             * Destructor
             */
            ~Geometry_Engine();

            /**
             * Sets new advs for the geometry engine
             *
             * @param aNewADVs vector of new advs to use
             */
            void set_advs(Matrix<DDRMat> aNewADVs);

            /**
             * Gets the advs from the geometry engine
             *
             * @return vector of advs
             */
            Matrix<DDRMat>& get_advs();

            /**
             * Gets the lower bounds from the geometry engine
             *
             * @return vector of lower bounds
             */
            Matrix<DDRMat>& get_lower_bounds();

            /**
             * Gets the upper bounds from the geometry engine
             *
             * @return vector of upper bounds
             */
            Matrix<DDRMat>& get_upper_bounds();

            /**
             * Lets MDL know about the stored requested IQIs through the PDV host manager
             */
            void communicate_requested_IQIs();
            void communicate_requested_IQIs(Cell<std::string> aIQINames);

            /**
             * Gets the sensitivities of the critieria with respect to the advs
             *
             * @return Matrix of sensitivities
             */
            Matrix<DDRMat> get_dcriteria_dadv();

            /**
             * Gets the design variable interface from the geometry engine
             *
             * @return member pdv host manager pointer
             */
            MSI::Design_Variable_Interface* get_design_variable_interface();

            /**
             * Determines if the element consisting of the given node coordinates is intersected.
             *
             * @param aNodeIndices Node indices
             * @param aNodeCoordinates Node coordinates
             * @return If the element is intersected
             */
            bool is_intersected(const Matrix<IndexMat>& aNodeIndices, const Matrix<DDRMat>& aNodeCoordinates);

            /**
             * Determines if the given edge is intersected, and queues an intersection node if it is. If an intersection
             * node has been queued, questions can be asked about the queued node:
             *
             * @param aNodeIndex1 First node index
             * @param aNodeIndex2 Second node index
             * @param aNodeCoordinates1 First node coordinate
             * @param aNodeCoordinates2 Second node coordinate
             * @return If the edge is intersected and a node has been queued
             */
            bool queue_intersection(
                    uint aNodeIndex1,
                    uint aNodeIndex2,
                    const Matrix<DDRMat>& aNodeCoordinates1,
                    const Matrix<DDRMat>& aNodeCoordinates2);

            /**
             * Returns if the queued intersection has the first parent node on the active geometry interface.
             *
             * @return If the first parent node is on the interface
             */
            bool queued_intersection_first_parent_on_interface();

            /**
             * Returns if the queued intersection has the second parent node on the active geometry interface.
             *
             * @return If the second parent node is on the interface
             */
            bool queued_intersection_second_parent_on_interface();

            /**
             * Gets the local coordinate of the queued intersection node.
             *
             * @return Intersection node local coordinate (between -1 and 1)
             */
            real get_queued_intersection_local_coordinate();

            /**
             * Gets the global coordinates of the queued intersection node.
             *
             * @return Intersection node global coordinates
             */
            Matrix<DDRMat> get_queued_intersection_global_coordinates();

            /**
             * Admit the queued intersection as a unique, permanent node(s) for sensitivity calculations.
             */
            void admit_queued_intersection(uint aNodeIndex);

            /**
             * Gets all of the geometry field values at the specified coordinates
             *
             * @param aNodeIndices Node indices on the mesh
             * @param aCoordinates Coordinate values for evaluating the geometry fields
             * @param aGeometryIndex Index of the geometry for evaluating the field of
             * @return Field values
             */
            real get_geometry_field_value(      uint            aNodeIndex,
                                          const Matrix<DDRMat>& aCoordinates,
                                                uint            aGeometryIndex = 0);

            /**
             * create new node geometry objects
             * @param[ in ] aNodeCoords node coordinates
             */
            void create_new_child_nodes( const Cell<moris_index>&    aNewNodeIndices,
                                         const Cell<xtk::Topology*>& aParentTopo,
                                         const Cell<Matrix<DDRMat>>& aParamCoordRelativeToParent,
                                         const Matrix<DDRMat>&       aGlobalNodeCoord );

            /**
             * @brief Get the total number of phases in the phase table
             */
            size_t get_num_phases();

            /**
             * @brief Get the 0 or 1 value associated with a given phase and geometry index
             */
            moris_index get_phase_sign_of_given_phase_and_geometry( moris_index aPhaseIndex,
                                                        moris_index aGeometryIndex );

            /**
              * For a given node index, return the phase index relative to each geometry (i.e. inside/outside indicator)
              */
            size_t get_phase_index(moris_index aNodeIndex, const Matrix<DDRMat>& aCoordinates);

            /**
             * @brief Provided the inside and out phase values for an entity, return the phase index
             */
            moris_index get_elem_phase_index(Matrix< IndexMat > const & aElemOnOff);

            /**
             * @brief Returns whether a node is inside or outside wrt to a given geometry index
             */
            size_t get_node_phase_index_wrt_a_geometry(uint aNodeIndex, const Matrix<DDRMat>& aCoordinates, uint aGeometryIndex);

            /**
             * @brief Returns the number of geometries
             */
            size_t get_num_geometries();

            /**
             * @brief Returns the number of phases
             */
            size_t get_num_bulk_phase();

            /**
             * @brief Returns the active geometry index
             */
            size_t get_active_geometry_index();

            /**
             * @brief Advance the active geometry index
             */
            void advance_geometry_index();

            /**
             * Return the number of fields that can be used for refinement
             *
             * @return Number of fields for refinement
             */
            uint get_num_refinement_fields();

            /**
             * Gets a flag to determine if refinement should continue
             *
             * @param aFieldIndex The index of a field
             * @param aRefinementIndex The current refinement step being performed
             * @return If refinement is needed for this field
             */
            bool refinement_needed(uint aFieldIndex,
                                   uint aRefinementIndex);

            /**
             * Returns fields so that HMR can perform refinement based on the data from this performer
             *
             * @param aFieldIndex Index of the field
             * @param aNodeIndex Index of the node
             * @param aCoordinates Coordinates of the node
             */
            real get_field_value(uint aFieldIndex,
                                 uint aNodeIndex,
                                 const Matrix<DDRMat>& aCoordinates);

            /**
             * Gets the index of an HMR user-defined refinement function for the given field index
             *
             * @param aFieldIndex Index of the field
             * @param aRefinementIndex The current refinement step being performed
             * @return User-defined function index, or -1 to use default refinement
             */
            sint get_refinement_function_index(uint aFieldIndex,
                                               uint aRefinementIndex);

            /**
             * Computes and saves the current level-set field data based on the given interpolation mesh.
             *
             * @param aMesh Mesh for computing level set data
             */
            void compute_level_set_data(mtk::Interpolation_Mesh* aMesh);

            /**
             * Assign PDV hosts based on properties constructed through parameter lists
             *
             * @param aMeshManager Mesh manager
             */
            void create_pdvs(std::shared_ptr<mtk::Mesh_Manager> aMeshManager);

        private:

            /**
             * Create PDV_Type hosts with the specified PDV_Type types on the interpolation mesh
             *
             * @param aPdvTypes PDV_Type types; set->group->individual
             * @param aMeshIndex Interpolation mesh index
             */
            void create_ip_pdv_hosts(mtk::Interpolation_Mesh* aInterpolationMesh,
                                     mtk::Integration_Mesh* aIntegrationMesh,
                                     Cell<Cell<Cell<PDV_Type>>> aPdvTypes);

            /**
             * Create PDV_Type hosts with PDVs for each of the spatial dimensions on the integration mesh
             *
             * @param aMeshIndex Integration mesh index
             */
            void create_ig_pdv_hosts(mtk::Integration_Mesh* aIntegrationMesh);

            /**
             * @brief assign the pdv type and property for each pdv host in a given set
             */
            void assign_property_to_pdv_hosts(std::shared_ptr<Property> aPropertyPointer,
                                              PDV_Type                  aPdvType,
                                              mtk::Integration_Mesh*    aIntegrationMesh,
                                              Matrix<DDUMat>            aSetIndices);

        };
    }
}

#endif /* MORIS_CL_Geometry_Engine_HPP_ */
