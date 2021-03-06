/*
//
//  Copyright 2016 Google, Inc.
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2014 SRI International
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

#include "cmtkUniformVolume.h"

#include <Base/cmtkAffineXform.h>
#include <Base/cmtkEigenSystemSymmetricMatrix3x3.h>

namespace
cmtk
{

UniformVolume::UniformVolume 
( const UniformVolume& other ) : Volume( other ), m_Delta( other.m_Delta ), m_IndexToPhysicalMatrix( other.m_IndexToPhysicalMatrix ), m_AlternativeIndexToPhysicalMatrices( other.m_AlternativeIndexToPhysicalMatrices ) {}

UniformVolume::UniformVolume
( const DataGrid::IndexType& dims, const Self::CoordinateVectorType& size, TypedArray::SmartPtr& data )
  : Volume( dims, size, data )
{
  for ( int i=0; i<3; ++i ) 
    {
    this->m_Delta[i] = ( (this->m_Dims[i] > 1) && (this->m_Size[i] > 0) ) ? this->m_Size[i] / (this->m_Dims[i] - 1) : 1.0;
    }
  
  this->CropRegion() = this->GetWholeImageRegion();
  this->CreateDefaultIndexToPhysicalMatrix();
}

UniformVolume::UniformVolume
( const DataGrid::IndexType& dims, const Types::Coordinate deltaX, const Types::Coordinate deltaY, const Types::Coordinate deltaZ, TypedArray::SmartPtr& data )
  : Volume( dims, dims, data )
{
  this->m_Delta[0] = deltaX;
  this->m_Delta[1] = deltaY;
  this->m_Delta[2] = deltaZ;

  for ( int i=0; i<3; ++i )
    this->m_Size[i] = this->m_Delta[i] * (this->m_Dims[i]-1);

  this->CropRegion() = this->GetWholeImageRegion();
  this->CreateDefaultIndexToPhysicalMatrix();
}

UniformVolume*
UniformVolume::CloneVirtual( const bool copyData )
{
  if ( copyData ) 
    {
    return this->CloneVirtual();
    } 
  else 
    {
    UniformVolume *result = this->CloneGridVirtual();
    result->SetData( this->GetData() );
    
    return result;
    }
}

UniformVolume*
UniformVolume::CloneVirtual() const
{
  UniformVolume *result = this->CloneGridVirtual();
  
  if ( this->GetData() )
    {
    TypedArray::SmartPtr clonedData( this->GetData()->Clone() );
    result->SetData( clonedData );
    }
  else
    {
    result->SetData( TypedArray::SmartPtr::Null() );
    }
  
  return result;
}

UniformVolume*
UniformVolume::CloneGridVirtual() const
{
  UniformVolume* clone = new UniformVolume( this->m_Dims, this->m_Size );
  clone->m_Delta = this->m_Delta; // for single-slice images
  clone->SetOffset( this->m_Offset );
  clone->CopyMetaInfo( *this );
  clone->m_IndexToPhysicalMatrix = this->m_IndexToPhysicalMatrix;
  clone->m_AlternativeIndexToPhysicalMatrices = this->m_AlternativeIndexToPhysicalMatrices;
  return clone;
}

UniformVolume* 
UniformVolume::GetDownsampledAndAveraged( const Types::GridIndexType downsample, const bool approxIsotropic ) const
{
  if ( approxIsotropic )
    {
    const Types::Coordinate minDelta = std::min<Types::Coordinate>( this->m_Delta[0], std::min<Types::Coordinate>( this->m_Delta[1], this->m_Delta[2] ) );
    const Types::GridIndexType downsampleByAxis[3] = { std::max<Types::GridIndexType>( 1, downsample / std::max<Types::GridIndexType>( 1, static_cast<Types::GridIndexType>(this->m_Delta[0] / minDelta) ) ),
				      std::max<Types::GridIndexType>( 1, downsample / std::max<Types::GridIndexType>( 1, static_cast<Types::GridIndexType>(this->m_Delta[1] / minDelta) ) ),
				      std::max<Types::GridIndexType>( 1, downsample / std::max<Types::GridIndexType>( 1, static_cast<Types::GridIndexType>(this->m_Delta[2] / minDelta) ) ) };
    return this->GetDownsampledAndAveraged( downsampleByAxis );
    }
  else
    {
    const Types::GridIndexType downsampleByAxis[3] = { downsample, downsample, downsample };
    return this->GetDownsampledAndAveraged( downsampleByAxis );
    }
}

UniformVolume* 
UniformVolume::GetDownsampledAndAveraged( const Types::GridIndexType (&downsample)[3] ) const
{
  DataGrid::SmartPtr newDataGrid( this->DataGrid::GetDownsampledAndAveraged( downsample ) );
  TypedArray::SmartPtr newData = newDataGrid->GetData();

  // create downsample grid
  UniformVolume* dsVolume = new UniformVolume( newDataGrid->GetDims(), downsample[0] * this->m_Delta[0], downsample[1] * this->m_Delta[1], downsample[2] * this->m_Delta[2], newData );

  // compute shift of volume origin
  const Types::Coordinate shift[3] = { (downsample[0]-1)*this->m_Delta[0]/2, (downsample[1]-1)*this->m_Delta[1]/2, (downsample[2]-1)*this->m_Delta[2]/2 };
  
  // apply shift to origin
  Self::CoordinateVectorType offset( this->m_Offset );
  offset += Self::CoordinateVectorType::FromPointer( shift );
  dsVolume->SetOffset( offset );
  
  // set crop region while considering new image offset
  dsVolume->SetHighResCropRegion( this->GetHighResCropRegion() );

  dsVolume->CopyMetaInfo( *this );
  dsVolume->m_IndexToPhysicalMatrix = this->m_IndexToPhysicalMatrix;

  // apply offset shift to index-to-physical matrix
  for ( int axis = 0; axis < 3; ++axis )
    {
    for ( int i = 0; i < 3; ++i )
      {
      dsVolume->m_IndexToPhysicalMatrix[3][i] += (downsample[i]-1) * dsVolume->m_IndexToPhysicalMatrix[axis][i] / 2;
      // also update voxel size
      dsVolume->m_IndexToPhysicalMatrix[axis][i] *= downsample[i];
      }
    }

  // do the same for any alternative matrices
  dsVolume->m_AlternativeIndexToPhysicalMatrices = this->m_AlternativeIndexToPhysicalMatrices;
  for ( std::map<int,AffineXform::MatrixType>::iterator it = dsVolume->m_AlternativeIndexToPhysicalMatrices.begin(); it != dsVolume->m_AlternativeIndexToPhysicalMatrices.end(); ++it )
    {
    // apply offset shift to index-to-physical matrix
    for ( int axis = 0; axis < 3; ++axis )
      {
      for ( int i = 0; i < 3; ++i )
	{
	(it->second)[3][i] += (downsample[i]-1) * (it->second)[axis][i] / 2;
	// also update voxel size
	(it->second)[axis][i] *= downsample[i];
	}
      }
    }
  
  return dsVolume;
}

UniformVolume* 
UniformVolume::GetDownsampled( const Types::GridIndexType downsample, const bool approxIsotropic ) const
{
  if ( approxIsotropic )
    {
    const Types::Coordinate minDelta = std::min<Types::Coordinate>( this->m_Delta[0], std::min<Types::Coordinate>( this->m_Delta[1], this->m_Delta[2] ) );
    const Types::GridIndexType downsampleByAxis[3] = { std::max<Types::GridIndexType>( 1, downsample / std::max<Types::GridIndexType>( 1, static_cast<Types::GridIndexType>(this->m_Delta[0] / minDelta) ) ),
				      std::max<Types::GridIndexType>( 1, downsample / std::max<Types::GridIndexType>( 1, static_cast<Types::GridIndexType>(this->m_Delta[1] / minDelta) ) ),
				      std::max<Types::GridIndexType>( 1, downsample / std::max<Types::GridIndexType>( 1, static_cast<Types::GridIndexType>(this->m_Delta[2] / minDelta) ) ) };
    return this->GetDownsampled( downsampleByAxis );
    }
  else
    {
    const Types::GridIndexType downsampleByAxis[3] = { downsample, downsample, downsample };
    return this->GetDownsampled( downsampleByAxis );
    }
}

UniformVolume* 
UniformVolume::GetDownsampled( const Types::GridIndexType (&downsample)[3] ) const
{
  DataGrid::SmartPtr newDataGrid( this->DataGrid::GetDownsampled( downsample ) );
  TypedArray::SmartPtr newData = newDataGrid->GetData();
  
  // create downsample grid
  UniformVolume* dsVolume = new UniformVolume( newDataGrid->GetDims(), downsample[0] * this->m_Delta[0], downsample[1] * this->m_Delta[1], downsample[2] * this->m_Delta[2], newData );
  
  dsVolume->SetOffset( this->m_Offset );
  dsVolume->SetHighResCropRegion( this->GetHighResCropRegion() );
  
  dsVolume->CopyMetaInfo( *this );
  dsVolume->m_IndexToPhysicalMatrix = this->m_IndexToPhysicalMatrix;
  
  // apply offset shift to index-to-physical matrix
  for ( int axis = 0; axis < 3; ++axis )
    for ( int i = 0; i < 3; ++i )
      {
      dsVolume->m_IndexToPhysicalMatrix[3][i] += (downsample[i]-1) * dsVolume->m_IndexToPhysicalMatrix[axis][i] / 2;
      // also update voxel size
      dsVolume->m_IndexToPhysicalMatrix[axis][i] *= downsample[i];
      }
  
  // do the same for any alternative matrices
  dsVolume->m_AlternativeIndexToPhysicalMatrices = this->m_AlternativeIndexToPhysicalMatrices;
  for ( std::map<int,AffineXform::MatrixType>::iterator it = dsVolume->m_AlternativeIndexToPhysicalMatrices.begin(); it != dsVolume->m_AlternativeIndexToPhysicalMatrices.end(); ++it )
    {
    // apply offset shift to index-to-physical matrix
    for ( int axis = 0; axis < 3; ++axis )
      {
      for ( int i = 0; i < 3; ++i )
	{
	(it->second)[3][i] += (downsample[i]-1) * (it->second)[axis][i] / 2;
	// also update voxel size
	(it->second)[axis][i] *= downsample[i];
	}
      }
    }

  return dsVolume;
}

UniformVolume* 
UniformVolume::GetInterleavedSubVolume
( const int axis, const Types::GridIndexType factor, const Types::GridIndexType idx ) const
{
  Self::IndexType dims;
  Self::CoordinateVectorType delta;

  for ( int dim = 0; dim < 3; ++dim )
    {
    dims[dim] = this->m_Dims[dim];
    delta[dim] = this->m_Delta[dim];
    }
  dims[axis] = this->m_Dims[axis] / factor;
  if ( this->m_Dims[axis] % factor > idx )
    ++dims[axis];
  delta[axis] = factor * this->m_Delta[axis];
  
  Self::CoordinateVectorType offset( 0.0 );
  offset[axis] = idx * this->m_Delta[axis];
  
  UniformVolume* volume = new UniformVolume( dims, delta[0], delta[1], delta[2] );
  volume->SetOffset( offset );
  for ( Types::GridIndexType i = 0; i < dims[axis]; ++i )
    {
    ScalarImage::SmartPtr slice( this->GetOrthoSlice( axis, idx + i * factor ) );
    volume->SetOrthoSlice( axis, i, slice );
    }
  
  volume->CopyMetaInfo( *this );

  volume->m_IndexToPhysicalMatrix = this->m_IndexToPhysicalMatrix;
  // update coordinate offset according to sub-volume index
  for ( int i = 0; i < 3; ++i )
    volume->m_IndexToPhysicalMatrix[3][i] += idx * volume->m_IndexToPhysicalMatrix[axis][i];
  // scale direction vector along axis
  for ( int i = 0; i < 3; ++i )
    volume->m_IndexToPhysicalMatrix[axis][i] *= factor;

  // do the same for any alternative matrices
  volume->m_AlternativeIndexToPhysicalMatrices = this->m_AlternativeIndexToPhysicalMatrices;
  for ( std::map<int,AffineXform::MatrixType>::iterator it = volume->m_AlternativeIndexToPhysicalMatrices.begin(); it != volume->m_AlternativeIndexToPhysicalMatrices.end(); ++it )
    {
    // update coordinate offset according to sub-volume index
    for ( int i = 0; i < 3; ++i )
      (it->second)[3][i] += idx * (it->second)[axis][i];
    // scale direction vector along axis
    for ( int i = 0; i < 3; ++i )
      (it->second)[axis][i] *= factor;
    }  

  if ( this->GetData()->GetPaddingFlag() )
    {
    volume->GetData()->SetPaddingValue( this->GetData()->GetPaddingValue() );
    }
  
  return volume;
}

UniformVolume* 
UniformVolume::GetInterleavedPaddedSubVolume
( const int axis, const Types::GridIndexType factor, const Types::GridIndexType idx ) const
{
  Types::GridIndexType sDims = this->m_Dims[axis] / factor;
  if ( this->m_Dims[axis] % factor > idx )
    ++sDims;

  UniformVolume* volume = new UniformVolume( this->m_Dims, this->m_Size );
  (volume->CreateDataArray( this->GetData()->GetType() ))->Fill( 0.0 );
  volume->SetOffset( this->m_Offset );
  for ( Types::GridIndexType i = 0; i < sDims; ++i )
    {
      const Types::GridIndexType sliceIdx = idx + i * factor;
    ScalarImage::SmartPtr slice( this->GetOrthoSlice( axis, sliceIdx ) );
    volume->SetOrthoSlice( axis, sliceIdx, slice );
    }
  
  volume->CopyMetaInfo( *this );
  volume->m_IndexToPhysicalMatrix = this->m_IndexToPhysicalMatrix;
  volume->m_AlternativeIndexToPhysicalMatrices = this->m_AlternativeIndexToPhysicalMatrices;
  for ( std::map<int,AffineXform::MatrixType>::iterator it = volume->m_AlternativeIndexToPhysicalMatrices.begin(); it != volume->m_AlternativeIndexToPhysicalMatrices.end(); ++it )
    {
    // update coordinate offset according to sub-volume index
    for ( int i = 0; i < 3; ++i )
      (it->second)[3][i] += idx * (it->second)[axis][i];
    // scale direction vector along axis
    for ( int i = 0; i < 3; ++i )
      (it->second)[axis][i] *= factor;
    }

  return volume;
}

const UniformVolume::RegionType
UniformVolume::GetGridRange
( const Self::CoordinateRegionType& region ) const
{
  Self::IndexType from, to;

  for ( size_t i = 0; i < 3; ++i )
    {
    from[i] = std::max<IndexType::ValueType>( 0, static_cast<IndexType::ValueType>( (region.From()[i]-this->m_Offset[i]) / this->m_Delta[i] ) );
    to[i] = 1+std::min( this->m_Dims[i]-1, 1+static_cast<IndexType::ValueType>( (region.To()[i]-this->m_Offset[i]) / this->m_Delta[i] ) );
    }

  return UniformVolume::RegionType( from, to );
}

void
UniformVolume::Mirror ( const int axis )
{
  this->DataGrid::ApplyMirrorPlane( axis );
  
  this->CropRegion().From()[ axis ] = this->m_Dims[ axis ] - 1 - this->CropRegion().From()[ axis ];
  this->CropRegion().To()[ axis ] = this->m_Dims[ axis ] - 1 - this->CropRegion().To()[ axis ];
}

ScalarImage::SmartPtr
UniformVolume::GetOrthoSlice
( const int axis, const Types::GridIndexType plane ) const
{
  ScalarImage::SmartPtr sliceImage = DataGrid::GetOrthoSlice( axis, plane );
  sliceImage->SetImageSlicePosition( this->GetPlaneCoord( axis, plane ) );

  Self::CoordinateVectorType imageOffset( 0.0 );
  Self::CoordinateVectorType directionX( 0.0 );
  Self::CoordinateVectorType directionY( 0.0 );
  switch ( axis ) 
    {
    case AXIS_X:
      sliceImage->SetPixelSize( this->GetDelta( AXIS_Y ), this->GetDelta( AXIS_Z ) );
      imageOffset[0] = this->GetPlaneCoord( AXIS_X, plane );
      directionX[1] = 1;
      directionY[2] = 1;
      break;
    case AXIS_Y:
      sliceImage->SetPixelSize( this->GetDelta( AXIS_X ), this->GetDelta( AXIS_Z ) );
      imageOffset[1] = this->GetPlaneCoord( AXIS_X, plane );
      directionX[0] = 1;
      directionY[2] = 1;
      break;
    case AXIS_Z:
      sliceImage->SetPixelSize( this->GetDelta( AXIS_X ), this->GetDelta( AXIS_Y ) );
      imageOffset[2] = this->GetPlaneCoord( AXIS_X, plane );
      directionX[0] = 1;
      directionY[1] = 1;
      break;
    }

  sliceImage->SetImageDirectionX( directionX );
  sliceImage->SetImageDirectionY( directionY );
  
  sliceImage->SetImageOrigin( imageOffset );
  return sliceImage;
}

UniformVolume::SmartPtr
UniformVolume::ExtractSlice
( const int axis, const Types::GridIndexType plane ) const
{
  DataGrid::SmartPtr sliceGrid( this->DataGrid::ExtractSlice( axis, plane ) );
  Self::SmartPtr sliceVolume( new Self( sliceGrid->m_Dims, this->m_Delta[0], this->m_Delta[1], this->m_Delta[2], sliceGrid->m_Data ) );

  sliceVolume->m_Offset = this->m_Offset;
  sliceVolume->m_Offset[axis] += plane * this->m_Delta[axis];

  return sliceVolume;
}

ScalarImage::SmartPtr
UniformVolume::GetNearestOrthoSlice
( const int axis, const Types::Coordinate location ) const
{
  return this->GetOrthoSlice( axis, this->GetCoordIndex( axis, location ) );
}

ScalarImage::SmartPtr
UniformVolume::GetOrthoSliceInterp
( const int axis, const Types::Coordinate location ) const
{
  Types::GridIndexType baseSliceIndex = this->GetCoordIndex( axis, location );

  const Types::Coordinate baseSliceLocation = this->GetPlaneCoord( axis, baseSliceIndex );
  const Types::Coordinate nextSliceLocation = this->GetPlaneCoord( axis, baseSliceIndex+1 ); 

  // only bother to interpolate if we're more than 1% towards the next slice.
  if ( ((location - baseSliceLocation) / (nextSliceLocation-baseSliceLocation )) < 0.01 )
    return this->GetOrthoSlice( axis, baseSliceIndex );

  // only bother to interpolate if we're more than 1% towards the next slice.
  if ( ((nextSliceLocation - location) / (nextSliceLocation-baseSliceLocation )) < 0.01 )
    return this->GetOrthoSlice( axis, baseSliceIndex+1 );

  ScalarImage::SmartPtr image0 = this->GetOrthoSlice( axis, baseSliceIndex );
  ScalarImage::SmartPtr image1 = this->GetOrthoSlice( axis, baseSliceIndex+1 );

  TypedArray::SmartPtr data0 = image0->GetPixelData();
  TypedArray::SmartPtr data1 = image1->GetPixelData();

  Types::Coordinate weight0 = (nextSliceLocation - location) / (nextSliceLocation - baseSliceLocation );

  Types::DataItem value0, value1;
  for ( Types::GridIndexType idx = 0; idx < data0->GetDataSize(); ++idx ) 
    {
    if ( data0->Get( value0, idx ) && data1->Get( value1, idx ) ) 
      {
      data0->Set( weight0 * value0 + (1-weight0) * value1, idx );
      } 
    else
      {
      data0->SetPaddingAt( idx );
      }
    }
  
  image0->SetImageSlicePosition( location );
  image0->SetImageOrigin( weight0 * image0->GetImageOrigin() + (1-weight0) * image1->GetImageOrigin() );

  return image0;
}

void
UniformVolume
::GetPrincipalAxes( Matrix3x3<Types::Coordinate>& directions, Self::CoordinateVectorType& centerOfMass ) const
{
  centerOfMass = this->GetCenterOfMass();
  const Types::Coordinate xg = centerOfMass[0];
  const Types::Coordinate yg = centerOfMass[1];
  const Types::Coordinate zg = centerOfMass[2];

  Types::DataItem ixx = 0, iyy = 0, izz = 0, ixy = 0, iyz = 0, izx = 0;
  for ( Types::GridIndexType k = 0; k < this->m_Dims[2]; ++k )
    {
    const Types::Coordinate Dz = this->GetPlaneCoord( AXIS_Z, k ) - zg;
    const Types::Coordinate Dz2 = Dz * Dz;
    for ( Types::GridIndexType j = 0; j < this->m_Dims[1]; ++j )
      {
      const Types::Coordinate Dy = this->GetPlaneCoord( AXIS_Y, j ) - yg;
      const Types::Coordinate Dy2 = Dy * Dy;
      for ( Types::GridIndexType i = 0; i < this->m_Dims[0]; ++i )
	{
        const Types::Coordinate Dx = this->GetPlaneCoord( AXIS_X, i ) - xg;
	const Types::Coordinate Dx2 = Dx * Dx;

	Types::DataItem v;
	if ( this->GetDataAt( v, i, j, k ) )
          {
          ixx += v * ( Dy2 + Dz2 );
          iyy += v * ( Dz2 + Dx2 );
          izz += v * ( Dx2 + Dy2 );
          
          ixy += v * Dx * Dy;
          iyz += v * Dy * Dz;
          izx += v * Dz * Dx;
          }
	}
      }
    }

  Matrix3x3<Types::Coordinate> inertiaMatrix;
  inertiaMatrix[0][0] = ixx;
  inertiaMatrix[0][1] = -ixy;
  inertiaMatrix[0][2] = -izx;
  
  inertiaMatrix[1][0] = -ixy;
  inertiaMatrix[1][1] = iyy;
  inertiaMatrix[1][2] = -iyz;
  
  inertiaMatrix[2][0] = -izx;
  inertiaMatrix[2][1] = -iyz;
  inertiaMatrix[2][2] = izz;

  const EigenSystemSymmetricMatrix3x3<Types::Coordinate> eigensystem( inertiaMatrix );
  for ( int n = 0; n < 3; ++n )
    {
    const Self::CoordinateVectorType v = eigensystem.GetNthEigenvector( n );
    for ( int i = 0; i < 3; ++i )
      {
      directions[n][i] = v[i];
      }
    }
  
  // correct for negative determinant
  const Types::Coordinate det = directions.Determinant();
  for ( int i = 0; i < 3; i++ )
    {
    directions[2][i] *= det;
    }
   
  for ( int i = 0; i < 3; i++ )
    {
    const Types::Coordinate norm = sqrt( directions[i][0]*directions[i][0] + directions[i][1]*directions[i][1] + directions[i][2]*directions[i][2] );
    for ( int j = 0; j < 3; j++ )
      {
      directions[i][j] /= norm;
      }
    }
}

} // namespace cmtk
