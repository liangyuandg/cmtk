/*
//
//  Copyright 2016 Google, Inc.
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2011 SRI International
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

#include "cmtkUniformVolumeInterpolatorPartialVolume.h"

#include <math.h>

#ifdef HAVE_IEEEFP_H
#  include <ieeefp.h>
#endif

namespace cmtk
{

/** \addtogroup Base */
//@{

bool
UniformVolumeInterpolatorPartialVolume
::GetDataAt(const Vector3D& v, Types::DataItem& value) const
{
  value=0;

  Types::Coordinate lScaled[3];
  Types::GridIndexType imageGridPoint[3];
  for ( int n = 0; n < 3; ++n )
    {
    lScaled[n] = (v[n]-this->m_VolumeOffset[n]) / this->m_VolumeDeltas[n];
    imageGridPoint[n] = (Types::GridIndexType) floor( lScaled[n] );
    if ( ( imageGridPoint[n] < 0 ) || ( imageGridPoint[n] >= this->m_VolumeDims[n]-1 ) )
      return false;
    }
  
  const size_t offset = this->GetOffsetFromIndex( imageGridPoint[0], imageGridPoint[1], imageGridPoint[2] );
  
  Types::DataItem corners[8];
  bool dataPresent = false;

  size_t idx = 0;
  for ( Types::GridIndexType k = 0; k < 2; ++k )
    {
    for ( Types::GridIndexType j = 0; j < 2; ++j )
      {
      for ( Types::GridIndexType i = 0; i < 2; ++i, ++idx )
	{
	corners[idx] = this->m_VolumeDataArray[offset + this->GetOffsetFromIndex( i, j, k )];
	const bool dataHere = (finite( corners[idx] ) != 0);
	dataPresent |= dataHere;
	}
      }
    }
  
  if (dataPresent) 
    {
    const Types::Coordinate revX = lScaled[0]-imageGridPoint[0];
    const Types::Coordinate revY = lScaled[1]-imageGridPoint[1];
    const Types::Coordinate revZ = lScaled[2]-imageGridPoint[2];
    const Types::Coordinate offsX = 1-revX;
    const Types::Coordinate offsY = 1-revY;
    const Types::Coordinate offsZ = 1-revZ;

    const Types::Coordinate weights[8] = 
      { offsX * offsY * offsZ, revX * offsY * offsZ, offsX * revY * offsZ, revX * revY * offsZ,
	offsX * offsY * revZ, revX * offsY * revZ, offsX * revY * revZ, revX * revY * revZ 
      };

    bool done[8];
    memset( done, 0, sizeof( done ) );
    
    Types::Coordinate maxWeight = 0;
    for ( Types::GridIndexType j = 0; j < 8; ++j ) 
      {
      if ( done[j] ) continue;
      Types::Coordinate weight = weights[j];
      for ( Types::GridIndexType i = j+1; i < 8; ++i ) 
	{
	if ( done[i] ) continue;
	if ( corners[i] == corners[j] ) 
	  {
	  weight += weights[i];
	  done[i] = true;
	  }
	}
      if ( weight > maxWeight ) 
	{
	value = corners[j];
	maxWeight = weight;
	}
      }
    
    return true;
    }
  
  return false;
}

Types::DataItem
UniformVolumeInterpolatorPartialVolume
::GetDataDirect( const Types::GridIndexType* imageGridPoint, const Types::Coordinate* insidePixel ) const
{
  Types::DataItem value = 0;

  const size_t offset = this->GetOffsetFromIndex( imageGridPoint[0], imageGridPoint[1], imageGridPoint[2] );
  
  bool done[8];
  Types::DataItem corners[8];
  bool dataPresent = false;

  size_t idx = 0;
  for ( Types::GridIndexType k = 0; k < 2; ++k )
    {
    for ( Types::GridIndexType j = 0; j < 2; ++j )
      {
      for ( Types::GridIndexType i = 0; i < 2; ++i, ++idx )
	{
	corners[idx] = this->m_VolumeDataArray[offset + this->GetOffsetFromIndex( i, j, k )];
	const bool dataHere = (finite( corners[idx] ) != 0);
	done[idx] = !dataHere;
	dataPresent |= dataHere;
	}
      }
    }
  
  if (dataPresent) 
    {
    const Types::Coordinate offsX = 1-insidePixel[0];
    const Types::Coordinate offsY = 1-insidePixel[1];
    const Types::Coordinate offsZ = 1-insidePixel[2];

    const Types::Coordinate weights[8] = 
      { offsX * offsY * offsZ, 
	insidePixel[0] * offsY * offsZ, 
	offsX * insidePixel[1] * offsZ, 
	insidePixel[0] * insidePixel[1] * offsZ,
	offsX * offsY * insidePixel[2], 
	insidePixel[0] * offsY * insidePixel[2], 
	offsX * insidePixel[1] * insidePixel[2], 
	insidePixel[0] * insidePixel[1] * insidePixel[2] 
      };

    Types::Coordinate maxWeight = 0;
    for ( Types::GridIndexType j = 0; j < 8; ++j ) 
      {
      if ( done[j] ) continue;
      Types::Coordinate weight = weights[j];
      for ( Types::GridIndexType i = j+1; i < 8; ++i ) 
	{
	if ( done[i] ) continue;
	if ( corners[i] == corners[j] ) 
	  {
	  weight += weights[i];
	  done[i] = true;
	  }
	}
      if ( weight > maxWeight ) 
	{
	value = corners[j];
	maxWeight = weight;
	}
      }
    }
  
  return value;
};

} // namespace cmtk

