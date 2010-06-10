/*
//
//  Copyright 1997-2009 Torsten Rohlfing
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

#include <cmtkDataGrid.h>

namespace
cmtk
{

void
DataGrid::SetCropRegion( const Self::RegionType& region ) 
{
  this->m_CropRegion = region;
  for ( int dim = 0; dim < 3; ++dim )
    {
    if ( this->m_CropRegion.From()[dim] < 0 )
      this->m_CropRegion.From()[dim] = this->m_Dims[dim] + this->m_CropRegion.From()[dim];

    if ( this->m_CropRegion.To()[dim] < 0 )
      this->m_CropRegion.To()[dim] = this->m_Dims[dim] + this->m_CropRegion.To()[dim];
    }
}

const DataGrid::IndexType
DataGrid::GetCropRegionIncrements
() const
{
  DataGrid::IndexType increments;

  increments[0] = this->m_CropRegion.From()[0] + this->m_Dims[0] * ( this->m_CropRegion.From()[1] + this->m_Dims[1] * this->m_CropRegion.From()[2] );
  increments[1] = this->m_CropRegion.From()[0] + (this->m_Dims[0] - this->m_CropRegion.To()[0]);
  increments[2] = this->m_Dims[0] * ( this->m_CropRegion.From()[1] + ( this->m_Dims[1] - this->m_CropRegion.To()[1]) );
  
  return increments;
}

int
DataGrid::GetCropRegionNumVoxels() const 
{
  return (this->m_CropRegion.To()[0] - this->m_CropRegion.From()[0]) * (this->m_CropRegion.To()[1] - this->m_CropRegion.From()[1]) * (this->m_CropRegion.To()[2] - this->m_CropRegion.From()[2]);
}

void
DataGrid::AutoCrop
( const Types::DataItem threshold, const bool recrop, const int margin )
{
  const TypedArray* data = this->GetData();
  
  Self::IndexType cropFrom = this->CropRegion().From(), cropTo = this->CropRegion().To();
  
  if ( ! recrop ) 
    {
    for ( int dim = 0; dim < 3; ++dim ) 
      {
      cropFrom[dim] = 0;
      cropTo[dim] = this->m_Dims[dim];
      }
    }
  
  const size_t nextRow = this->m_Dims[0] - cropTo[0] + this->m_CropRegion.From()[0];
  const size_t nextPlane = this->m_Dims[0] * (this->m_Dims[1] - cropTo[1] + this->m_CropRegion.From()[1]);
  
  size_t offset = cropFrom[0] + this->m_Dims[0] * ( cropFrom[1] + this->m_Dims[1] * cropFrom[2] );

  Self::IndexType newCropFrom = cropTo, newCropTo = cropFrom;
  Self::IndexType xyz;
  for ( xyz[2] = cropFrom[2]; xyz[2] < cropTo[2]; ++xyz[2], offset += nextPlane )
    for ( xyz[1] = cropFrom[1]; xyz[1] < cropTo[1]; ++xyz[1], offset += nextRow )
      for ( xyz[0] = cropFrom[0]; xyz[0] < cropTo[0]; ++xyz[0], ++offset ) 
	{
	Types::DataItem value = 0;
	if ( ! data->Get( value, offset ) || (value < threshold) ) continue;
	
	for ( int i = 0; i < 3; ++i )
	  {
	  newCropFrom[i] = std::min( newCropFrom[i], xyz[i] );
	  newCropTo[i] = std::max( newCropTo[i], xyz[i] );
	  }
	}
  
  for ( int dim = 0; dim < 3; ++dim ) 
    {
    ++newCropTo[dim];
    }
  
  if ( margin ) 
    {
    for ( int dim = 0; dim < 3; ++dim ) 
      {
      newCropFrom[dim] = std::max<int>( 0, newCropFrom[dim] - margin );
      newCropTo[dim] = std::min<int>( this->m_Dims[dim], newCropTo[dim] + margin );
      }
    }
  
  this->m_CropRegion = Self::RegionType( newCropFrom, newCropTo );
}

void
DataGrid::FillCropBackground( const Types::DataItem value )
{
  const size_t planeSize = this->m_Dims[0] * this->m_Dims[1];

  size_t offset = this->m_CropRegion.From()[2] * planeSize;
  this->m_Data->BlockSet( value, 0, offset );

  for ( int z = this->m_CropRegion.From()[2]; z < this->m_CropRegion.To()[2]; ++z ) 
    {
    size_t planeOffset = offset + this->m_CropRegion.From()[1] * this->m_Dims[0];
    this->m_Data->BlockSet( value, offset, planeOffset );

    offset = planeOffset;
    for ( int y = this->m_CropRegion.From()[1]; y < this->m_CropRegion.To()[1]; ++y, offset += this->m_Dims[0] ) 
      {
      this->m_Data->BlockSet( value, offset, offset+this->m_CropRegion.From()[0] );
      this->m_Data->BlockSet( value, offset+this->m_CropRegion.To()[0], offset+this->m_Dims[0] );
      }
    
    planeOffset = offset + (this->m_Dims[1]-this->m_CropRegion.To()[1]) * this->m_Dims[0];
    this->m_Data->BlockSet( value, offset, planeOffset );
    offset = planeOffset;
    }
  
  this->m_Data->BlockSet( value, this->m_CropRegion.To()[2] * planeSize, this->m_Dims[2] * planeSize );
}

TypedArray::SmartPtr
DataGrid::GetCroppedData() const
{
  const TypedArray* srcData = this->GetData();
  if ( ! srcData ) 
    throw( Exception( "No input data in DataGrid::GetCroppedData()" ) );

  TypedArray::SmartPtr cropData = TypedArray::Create( srcData->GetType(), this->GetCropRegionNumVoxels() );
  
  const size_t lineLength = this->m_CropRegion.To()[0] - this->m_CropRegion.From()[0];
  const size_t nextPlane = this->m_Dims[0] * (this->m_Dims[1] - (this->m_CropRegion.To()[1] - this->m_CropRegion.From()[1]));
  
  size_t toOffset = 0;
  size_t fromOffset = this->m_CropRegion.From()[0] + this->m_Dims[0] * ( this->m_CropRegion.From()[1] + this->m_Dims[1] * this->m_CropRegion.From()[2] );

  for ( int z = this->m_CropRegion.From()[2]; z < this->m_CropRegion.To()[2]; ++z, fromOffset += nextPlane )
    for ( int y = this->m_CropRegion.From()[1]; y < this->m_CropRegion.To()[1]; ++y, fromOffset += this->m_Dims[0] ) 
      {
      srcData->BlockCopy( *cropData, toOffset, fromOffset, lineLength );
      toOffset += lineLength;
      }
  
  return cropData;
}

} // namespace cmtk
