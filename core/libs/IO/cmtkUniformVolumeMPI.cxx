/*
//
//  Copyright 1997-2010 Torsten Rohlfing
//
//  Copyright 2004-2010 SRI International
//
//  This file is part of the Computational Morphometry Toolkit.
//
//  http://www.nitrc.org/projects/cmtk/
//
//  The Computational Morphometry Toolkit is free software: you can
//  redistribute it and/or modify it under the terms of the GNU General Public
//  License as published by the Free Software Foundation, either version 3 of
//  the License, or (at your option) any later version.
//
//  The Computational Morphometry Toolkit is distributed in the hope that it
//  will be useful, but WITHOUT ANY WARRANTY; without even the implied
//  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with the Computational Morphometry Toolkit.  If not, see
//  <http://www.gnu.org/licenses/>.
//
//  $Revision$
//
//  $LastChangedDate$
//
//  $LastChangedBy$
//
*/

#include <cmtkconfig.h>

#ifdef CMTK_BUILD_MPI

#include <cmtkMPI.h>
#include <cmtkUniformVolume.h>

#include <mpi.h>

namespace cmtk
{

/** \addtogroup IO */
//@{
namespace mpi
{

template<>
void 
Broadcast
( MPI::Intracomm& comm, UniformVolume::SmartPtr& inOutPtr, const int root )
{
  const size_t msgBufferHdrSize = 
    3 * sizeof(int) + // Dims[0..2]
    6 * sizeof(int) + // CropRegion.From[0..2], CropRegion.To[0..2]
    3 * sizeof( Types::Coordinate ) + // Size[0..2]
    3 * sizeof( Types::Coordinate ) + // m_Offset[0..2]
    sizeof( ScalarDataType ); // data type

  char msgBufferHdr[ msgBufferHdrSize ];

  if ( comm.Get_rank() == root )
    {
    // send from this process
    int position = 0;
    MPI::CHAR.Pack( inOutPtr->GetDims(), 3 * sizeof( inOutPtr->GetDims()[0] ), msgBufferHdr, msgBufferHdrSize, position, comm );

    const cmtk::DataGrid::RegionType& cropRegion = inOutPtr->CropRegion();

    const int crop[6] = { cropRegion.From()[0], cropRegion.From()[1], cropRegion.From()[2], cropRegion.To()[0], cropRegion.To()[1], cropRegion.To()[2] };
    MPI::CHAR.Pack( crop, sizeof( crop ), msgBufferHdr, msgBufferHdrSize, position, comm );

    MPI::CHAR.Pack( inOutPtr->Size, 3 * sizeof( inOutPtr->Size[0] ), msgBufferHdr, msgBufferHdrSize, position, comm );

    MPI::CHAR.Pack( inOutPtr->m_Offset.XYZ, 3 * sizeof( inOutPtr->m_Offset[0] ), msgBufferHdr, msgBufferHdrSize, position, comm );

    ScalarDataType dataType = TYPE_NONE;
    const TypedArray* inData = inOutPtr->GetData();
    if ( inData )
      {
      dataType = inData->GetType();
      }
    MPI::CHAR.Pack( &dataType, sizeof( dataType ), msgBufferHdr, msgBufferHdrSize, position, comm );
    }
    
  comm.Bcast( msgBufferHdr, msgBufferHdrSize, MPI::CHAR, root );

  if ( comm.Get_rank() != root )
    {
    // build received volume
    int dims[3];
    int cropRegionFrom[3];
    int cropRegionTo[3];
    Types::Coordinate size[3];
    Types::Coordinate offset[3];
    ScalarDataType dataType;

    int position = 0;
    MPI::CHAR.Unpack( msgBufferHdr, msgBufferHdrSize, dims, sizeof( dims ), position, comm );
    MPI::CHAR.Unpack( msgBufferHdr, msgBufferHdrSize, cropRegionFrom, sizeof( cropRegionFrom ), position, comm );
    MPI::CHAR.Unpack( msgBufferHdr, msgBufferHdrSize, cropRegionTo, sizeof( cropRegionTo ), position, comm );
    MPI::CHAR.Unpack( msgBufferHdr, msgBufferHdrSize, size, sizeof( size ), position, comm );
    MPI::CHAR.Unpack( msgBufferHdr, msgBufferHdrSize, offset, sizeof( offset ), position, comm );
    MPI::CHAR.Unpack( msgBufferHdr, msgBufferHdrSize, &dataType, sizeof( dataType ), position, comm );

    inOutPtr = UniformVolume::SmartPtr( new UniformVolume( dims, size ) );
    inOutPtr->SetOffset( Vector3D( offset ) );
    inOutPtr->CropRegion() = cmtk::DataGrid::RegionType( cmtk::DataGrid::IndexType( cropRegionFrom ), cmtk::DataGrid::IndexType( cropRegionTo ) );
    inOutPtr->CreateDataArray( dataType );
    }
  
  comm.Bcast( inOutPtr->GetData()->GetDataPtr(), inOutPtr->GetData()->GetDataSizeBytes(), MPI::CHAR, root );
}

template void Broadcast<UniformVolume>( ::MPI::Intracomm&, SmartPointer<UniformVolume>&, const int );

} // namespace mpi

//@}

} // namespace cmtk


#endif // #ifdef CMTK_BUILD_MPI
