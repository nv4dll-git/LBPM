#ifndef COMMON_H_INC
#define COMMON_H_INC

#include "common/Array.h"
#include "common/Communication.h"

#include <set>
#include <map>
#include <vector>


// Define types to use for blob ids
typedef int32_t BlobIDType;
typedef Array<BlobIDType> BlobIDArray;


/*!
 * @brief  Compute the blob
 * @details  Compute the blob (F>vf|S>vs) starting from (i,j,k) - oil blob
 * @return  Returns the number of cubes in the blob
 * @param[in] Phase         Phase
 * @param[in] SignDist      SignDist
 * @param[in] vF            vF
 * @param[in] vS            vS
 * @param[in] S             S
 * @param[out] LocalBlobID  The ids of the blobs
 * @return  Returns the number of blobs
 */
int ComputeLocalBlobIDs( const DoubleArray& Phase, const DoubleArray& SignDist, 
    double vF, double vS, BlobIDArray& LocalBlobID, bool periodic=true );

/*!
 *  @brief Compute blob of an arbitrary phase
 *  @details Compute the connected components for Phase(i,j,k)=VALUE
 *  @return the number of connected components of the phase
 *  @param[in] PhaseID
 *  @param[in] VALUE
 *  @param[out] ComponentLabel
 *  @param[in] periodic
 */
int ComputeLocalPhaseComponent( const IntArray &PhaseID, int &VALUE, IntArray &ComponentLabel, bool periodic );


/*!
 * @brief  Compute the blob
 * @details  Compute the blob (F>vf|S>vs) starting from (i,j,k) - oil blob
 * @return  Returns the number of cubes in the blob
 * @param[in] nx            Number of elements in the x-direction
 * @param[in] ny            Number of elements in the y-direction
 * @param[in] nz            Number of elements in the z-direction
 * @param[in] Phase         Phase
 * @param[in] SignDist      SignDist
 * @param[in] vF            vF
 * @param[in] vS            vS
 * @param[in] S             S
 * @param[out] LocalBlobID  The ids of the blobs
 * @return  Returns the number of blobs
 */
int ComputeGlobalBlobIDs( int nx, int ny, int nz, const RankInfoStruct& rank_info, 
    const DoubleArray& Phase, const DoubleArray& SignDist, double vF, double vS, 
    BlobIDArray& GlobalBlobID );


/*!
 * @brief Compute component of the specified phase
 * @details Compute component of specified phase PhaseID=VALUE
 * @return  Returns the number of cubes in the blob
 * @param[in] nx            Number of elements in the x-direction
 * @param[in] ny            Number of elements in the y-direction
 * @param[in] nz            Number of elements in the z-direction
 * @param[in] rank_in       MPI communication info
 * @param[in] PhaseID       Array that identifies the phases
 * @param[in] VALUE         Identifier for the phase to decompose
 * @param[out] GlobalBlobID The ids of the blobs for the phase
 * @return Return the number of components in the specified phase
 */
int ComputeGlobalPhaseComponent( int nx, int ny, int nz, const RankInfoStruct& rank_info,
    const IntArray &PhaseID, int VALUE, BlobIDArray &GlobalBlobID );


/*!
 * @brief  Reorder the blobs
 * @details  Reorder the blobs based on the number of cells they contain
 *    largest first.
 * @param[in] nx            Number of elements in the x-direction
 * @param[in] ny            Number of elements in the y-direction
 * @param[in] nz            Number of elements in the z-direction
 * @param[in/out] ID        The ids of the blobs
 */
void ReorderBlobIDs( BlobIDArray& ID );


typedef std::pair<BlobIDType,std::vector<BlobIDType> > BlobIDSplitStruct;
typedef std::pair<std::vector<BlobIDType>,BlobIDType> BlobIDMergeStruct;
typedef std::pair<std::vector<BlobIDType>,std::vector<BlobIDType> > BlobIDMergeSplitStruct;
struct ID_map_struct {
    std::vector<BlobIDType> created;                   // list of new blobs that were created
    std::vector<BlobIDType> destroyed;                 // list of blobs that disappeared
    std::vector<std::pair<BlobIDType,BlobIDType> > src_dst;   // one-one mapping of blobs (first,second timestep id)
    std::vector<BlobIDSplitStruct> split;       // list of blobs that split
    std::vector<BlobIDMergeStruct> merge;       // list of blobs that merged
    std::vector<BlobIDMergeSplitStruct> merge_split; // list of blobs that both merged and split
    //! Empty constructor
    ID_map_struct() {}
    //! Create initial map from N blobs (ordered 1:N-1)
    ID_map_struct( int N ) {
        created.resize(N);
        for (int i=0; i<N; i++) { created[i]=i; }
    }
};


/*!
 * @brief  Get the mapping of blob ids between iterations
 * @details  This functions computes the map of blob ids between iterations
 * @return  Returns the map of the blob ids.  Each final blob may have no source
 *    ids, one parent, or multiple parents.  Each src id may be a parent for multiple blobs.
 * @param[in] ID1           The blob ids at the first timestep
 * @param[in] ID2           The blob ids at the second timestep
 */
ID_map_struct computeIDMap( const BlobIDArray& ID1, const BlobIDArray& ID2 );


/*!
 * @brief  Compute the new global ids based on the map
 * @details  This functions computes the time-consistent global ids for the
 *     current global id index
 * @param[in/out] map       The timestep mapping for the ids
 * @param[in] id_max        The globally largest id used previously
 * @param[out] new_ids      The newly renumbered blob ids (0:ids.max())
 */
void getNewIDs( ID_map_struct& map, BlobIDType& id_max, std::vector<BlobIDType>& new_ids );


/*!
 * @brief  Update the blob ids based on mapping
 * @details  This functions computes the map of blob ids between iterations.
 *    Note: we also update the map to reflect the new ids
 * @param[out] new_ids          The newly renumbered blob ids (0:ids.max())
 * @param[in/out] IDs           The blob ids to renumber
 */
void renumberIDs( const std::vector<BlobIDType>& new_id_list, BlobIDArray& IDs );


/*!
 * @brief  Write the ID map
 * @details  This functions writes the id map fo an iteration.
 *    If no ids changed, then nothing will be written
 *    Note: only rank 0 writes, and the file is created on timestep 0.
 * @param[in] map           The timestep mapping for the ids
 * @param[in] timestep      The current timestep (timestep 0 creates the file)
 * @param[in] filename      The filename to write/append
 */
void writeIDMap( const ID_map_struct& map, long long int timestep, const std::string& filename );



#endif
