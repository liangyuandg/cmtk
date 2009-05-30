/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//  Copyright 2004-2009 SRI International
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

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#include <cmtkConsole.h>
#include <cmtkProgress.h>

namespace
cmtk
{

/** \addtogroup Base */
//@{

TypedArray::SmartPtr
DataGrid::CreateDataArray( const ScalarDataType dataType, const bool setToZero )
{
  TypedArray::SmartPtr data( TypedArray::Create( dataType, this->GetNumberOfPixels() ) );
  if ( setToZero )
    data->ClearArray();
  this->SetData( data );
  return data;
}

DataGrid* 
DataGrid::GetDownsampled( const int (&downsample)[3] ) const
{
  const int newDims[3] = { (this->Dims[0]-1) / downsample[0] + 1, (this->Dims[1]-1) / downsample[1] + 1, (this->Dims[2]-1) / downsample[2] + 1 };

  DataGrid* newDataGrid = new DataGrid;
  newDataGrid->SetDims( newDims );
  
  const TypedArray* thisData = this->GetData();
  if ( thisData )
    {
    TypedArray* newData = thisData->NewTemplateArray( newDataGrid->GetNumberOfPixels() );

#pragma omp parallel for
    for ( int z = 0; z < newDims[2]; ++z )
      {
      size_t toOffset = z * newDims[0] * newDims[1];
      int oldZ = z * downsample[2];
      int oldY = 0;
      for ( int y = 0; y < newDims[1]; ++y, oldY += downsample[1] ) 
	{
	int oldX = 0;
	for ( int x = 0; x < newDims[0]; ++x, oldX += downsample[0], ++toOffset ) 
	  {
	  Types::DataItem sum = 0;
	  int count = 0;
	  for ( int zz = 0; (zz < downsample[2]) && (oldZ+zz<this->Dims[2]); ++zz )
	    {
	    const size_t ofsZ = (oldZ+zz)*this->nextK;
	    for ( int yy = 0; (yy < downsample[1]) && (oldY+yy<this->Dims[1]); ++yy )
	      {
	      const size_t ofsYZ = ofsZ + (oldY+yy)*this->nextJ;
	      for ( int xx = 0; (xx < downsample[0]) && (oldX+xx<this->Dims[0]); ++xx )
		{
		Types::DataItem value;
		if ( thisData->Get( value, (oldX+xx) + ofsYZ  ) )
		  {
		  sum += value;
		  ++count;
		  }
		}
	      }
	    }
	  if ( count )
	    {
	    newData->Set( sum / count, toOffset );
	    }
	  else
	    {
	    newData->SetPaddingAt( toOffset );
	    }
	  }
	}
      } // end for (z)
    newDataGrid->SetData( TypedArray::SmartPtr( newData ) );
    }

  newDataGrid->m_MetaInformation[CMTK_META_IMAGE_ORIENTATION]  = this->m_MetaInformation[CMTK_META_IMAGE_ORIENTATION];
  newDataGrid->m_MetaInformation[CMTK_META_IMAGE_ORIENTATION_ORIGINAL]  = this->m_MetaInformation[CMTK_META_IMAGE_ORIENTATION_ORIGINAL];
  
  return newDataGrid;
}

DataGrid*
DataGrid::GetReoriented( const char* newOrientation ) const
{
  std::string curOrientation = this->m_MetaInformation[CMTK_META_IMAGE_ORIENTATION];
  if ( curOrientation.length() != 3 ) 
    {
    curOrientation = std::string( AnatomicalOrientation::ORIENTATION_STANDARD );
    }

  if ( !newOrientation )
    {
    newOrientation = AnatomicalOrientation::ORIENTATION_STANDARD;
    }

  // 1. get a permutation matrix
  AnatomicalOrientation::PermutationMatrix pmatrix( this->Dims, NULL, curOrientation, newOrientation );
  
  int newDims[3] = { 0, 0, 0 }; 
  pmatrix.GetPermutedArray( this->Dims, newDims );
  DataGrid* newDataGrid = new DataGrid( newDims );
  
  const TypedArray* oldData = this->GetData();
  if ( oldData )
    {
    newDataGrid->CreateDataArray( oldData->GetType() );  
    TypedArray* newData = newDataGrid->GetData().GetPtr();
    
    if ( oldData->GetPaddingFlag() ) 
      {
      newData->SetPaddingValue( oldData->GetPaddingValue() );
      }
    
    const char* fromPtr = static_cast<const char*>( oldData->GetDataPtr() );
    char* toPtr = static_cast<char*>( newData->GetDataPtr() );
    
    // 2. loop through the data, applying transformation
    const size_t bytesPerPixel = oldData->GetItemSize();

    int fromPoint[3];
    for ( fromPoint[2] = 0; fromPoint[2] < this->GetDims(2); fromPoint[2]++ )
      {
      for ( fromPoint[1] = 0; fromPoint[1] < this->GetDims(1); fromPoint[1]++ )
	{
	for ( fromPoint[0] = 0; fromPoint[0] < this->GetDims(0); fromPoint[0]++, fromPtr += bytesPerPixel )
	  {
	  memcpy( toPtr + bytesPerPixel * pmatrix.NewOffsetFromPoint( fromPoint ), fromPtr, bytesPerPixel );
	  }
	}
      }
    }
  
  newDataGrid->m_MetaInformation = this->m_MetaInformation;
  newDataGrid->m_MetaInformation[CMTK_META_IMAGE_ORIENTATION] = newOrientation;
  newDataGrid->m_MetaInformation[CMTK_META_IMAGE_ORIENTATION_ORIGINAL] = this->m_MetaInformation[CMTK_META_IMAGE_ORIENTATION_ORIGINAL];
  return newDataGrid;
}

void
DataGrid::SetDims( const int dims[3] )
{
  memcpy( Dims, dims, sizeof( Dims ) );

  nextI = 1;
  nextJ = nextI * Dims[0];
  nextK = nextJ * Dims[1];
  nextIJ = nextI + nextJ;
  nextIK = nextI + nextK;
  nextJK = nextJ + nextK;
  nextIJK = nextI + nextJ + nextK;
}

bool
DataGrid::TrilinearInterpolation
( Types::DataItem& value, const int X, const int Y, const int Z,
  const Vector3D& Location, const Types::Coordinate* from, 
  const Types::Coordinate* to ) const
{
  Types::DataItem corners[8];

  const size_t offset = X+Dims[0]*(Y+Dims[1]*Z);

  const TypedArray* data = this->GetData();
  bool data_present = data->Get( corners[0], offset );
  
  if ( X<Dims[0]-1 ) 
    {
    data_present &= data->Get( corners[1] , offset+nextI );
    
    if ( Y<Dims[1]-1 ) 
      {
      data_present &= data->Get( corners[3], offset+nextIJ );
      
      if ( Z<Dims[2]-1 )
	data_present &= data->Get( corners[7], offset+nextIJK );
      else
	return false;
      } 
    else
      {
      return false;
      }
    data_present &= data->Get( corners[5], offset+nextIK );
    } 
  else
    {
    return false;
    }
  
  data_present &= data->Get( corners[2], offset+nextJ );
  data_present &= data->Get( corners[6], offset+nextJK );
  data_present &= data->Get( corners[4], offset+nextK );

  if (data_present) 
    {
    const Types::Coordinate deltaX=1/(to[0]-from[0]), deltaY=1/(to[1]-from[1]), deltaZ=1/(to[2]-from[2]);
    
    const Types::Coordinate revX = deltaX*(Location[0]-from[0]);
    const Types::Coordinate revY = deltaY*(Location[1]-from[1]);
    const Types::Coordinate revZ = deltaZ*(Location[2]-from[2]);
    const Types::Coordinate offsX = 1-revX;
    const Types::Coordinate offsY = 1-revY;
    const Types::Coordinate offsZ = 1-revZ;
    
    value = 
      static_cast<Types::DataItem>( offsZ*(offsY*(offsX*corners[0]+revX*corners[1])+ 
				   revY*(offsX*corners[2]+revX*corners[3]))+
			    revZ*(offsY*(offsX*corners[4]+revX*corners[5])+ 
				  revY*(offsX*corners[6]+revX*corners[7])));
    
    return true;
  }

  return false;
}

TypedArray*
DataGrid::GetDataMirrorPlane( const int axis ) const
{
  TypedArray* Result = this->GetData()->Clone();
  this->MirrorPlaneInPlace( Result, this->Dims, axis );
  
  return Result;
}

void
DataGrid::ApplyMirrorPlane( const int axis )
{
  this->MirrorPlaneInPlace( this->GetData().GetPtr(), this->Dims, axis );
}

void
DataGrid::MirrorPlaneInPlace
( TypedArray *const data, const int dims[3], const int axis )
{
  switch ( axis ) 
    {
    case AXIS_X: 
    {
    int offset = 0;
    for ( int z = 0; z < dims[2]; ++z )
      for ( int y = 0; y < dims[1]; ++y, offset += dims[0] )
	data->BlockReverse( offset, dims[0] );
    }
    break;
    case AXIS_Y: 
    {
    size_t zOffset = 0;
    for ( int z = 0; z < dims[2]; ++z, zOffset += dims[0] * dims[1] ) 
      {
      for ( int y = 0; y < (dims[1] / 2); ++y )
	data->BlockSwap( zOffset + y * dims[0], zOffset + (dims[1] - 1 - y) * dims[0], dims[0] );
      }
    }
    break;
    case AXIS_Z: 
    {
    size_t blockSize = dims[0] * dims[1];
    for ( int z = 0; z < (dims[2] / 2); ++z ) 
      {
      data->BlockSwap( z * blockSize, (dims[2] - 1 - z) * blockSize, blockSize );
      }
    }
    break;
    }
}

TypedArray*
DataGrid::GetDataMedianFiltered( const int radius ) const
{
  const TypedArray* data = this->GetData();
  if ( ! data ) return NULL;
  
  TypedArray *Result = data->NewTemplateArray( data->GetDataSize() );

  const int width = 1 + 2*radius;
  Types::DataItem *sort = Memory::AllocateArray<Types::DataItem>(  width*width*width  );
  
  int offset = 0;
  Progress::SetTotalSteps( Dims[2] );

  ProgressResult status = PROGRESS_OK;
  for ( int z = 0; z < Dims[2]; ++z ) 
    {
    status = Progress::SetProgress( z );
    if ( status != PROGRESS_OK ) break;
    
    int zFrom = ( z > radius ) ? ( z - radius ) : 0;
    int zTo = std::min( z+radius+1, Dims[2] );

    for ( int y = 0; y < Dims[1]; ++y ) 
      {      
      int yFrom = ( y > radius ) ? ( y - radius ) : 0;
      int yTo = std::min( y+radius+1, Dims[1] );
      
      for ( int x = 0; x < Dims[0]; ++x, ++offset ) 
	{
	int xFrom = ( x > radius ) ? ( x - radius ) : 0;
	int xTo = std::min( x+radius+1, Dims[0] );
	
	int source = 0;
	int ofsZ = yFrom + Dims[1] * zFrom;
	for ( int zz = zFrom; zz < zTo; ++zz, ofsZ += Dims[1] ) 
	  {
	  int ofsYZ = Dims[0] * ofsZ ;
	  for ( int yy = yFrom; yy < yTo; ++yy, ofsYZ += Dims[0] ) 
	    {
	    int toYZ = ofsYZ + xTo;
	    for ( int xx = xFrom + ofsYZ; xx < toYZ; ++xx, ++source ) 
	      {
	      data->Get( sort[source], xx );
	      }
	    }
	  }
	
#ifdef CMTK_DATA_FLOAT	
	qsort( sort, source, sizeof( *sort ), MathUtil::CompareFloat );
#else
	qsort( sort, source, sizeof( *sort ), MathUtil::CompareDouble );
#endif
	
	if ( source % 2 )
	  Result->Set( sort[source/2], offset );
	else
	  Result->Set( (Types::DataItem) ( 0.5 * (sort[source/2] + sort[source/2-1]) ), offset );
	}
      }
    }
  Progress::Done();
  
  delete[] sort;
  
  if ( status != PROGRESS_OK ) 
    {
    delete Result;
    Result = NULL;
    }
  
  return Result;
}

void
DataGrid::ApplyMedianFilter( const int radius )
{
  this->SetData( TypedArray::SmartPtr( this->GetDataMedianFiltered( radius ) ) );
}

TypedArray*
DataGrid::GetDataSobelFiltered() const
{
  const TypedArray* data = this->GetData();
  if ( ! data ) return NULL;

  TypedArray *Result = data->NewTemplateArray( data->GetDataSize() );

  Types::DataItem value = 0;
  Types::DataItem fov[3][3][3];

  Progress::SetTotalSteps( Dims[2] );

  size_t offset = 0;
  for ( int z = 0; z < Dims[2]; ++z ) 
    {
    Progress::SetProgress( z );
    for ( int y = 0; y < Dims[1]; ++y )
      for ( int x = 0; x < Dims[0]; ++x, ++offset ) 
	{
	if ( x && y && z && ( x<Dims[0]-1 ) && ( y<Dims[1]-1 ) && ( z<Dims[2]-1 ) ) 
	  {
	  for ( int dz=-1; dz<2; ++dz )
	    for ( int dy=-1; dy<2; ++dy )
	      for ( int dx=-1; dx<2; ++dx )
		if ( ! data->Get( fov[1+dx][1+dy][1+dz], offset+nextI*dx+nextJ*dy+nextK*dz ) )
		  fov[1+dx][1+dy][1+dz] = 0;
	  
	  value = (Types::DataItem)
	    ( fabs( fov[0][0][1] - fov[2][0][1] + 
		    2 * ( fov[0][1][1] - fov[2][1][1] ) +
		    fov[0][2][1] - fov[2][2][1] ) +
	      fabs( fov[0][0][1] - fov[0][2][1] + 
		    2 * ( fov[1][0][1] - fov[1][2][1] ) +
		    fov[2][0][1] - fov[2][2][1] )+
	      fabs( fov[0][1][0] - fov[2][1][0] + 
		    2 * ( fov[0][1][1] - fov[2][1][1] ) +
		    fov[0][1][2] - fov[2][1][2] ) +
	      fabs( fov[0][1][0] - fov[0][1][2] + 
		    2 * ( fov[1][1][0] - fov[1][1][2] ) +
		    fov[2][1][0] - fov[2][1][2] ) +
	      fabs( fov[1][0][0] - fov[1][2][0] + 
		    2 * ( fov[1][0][1] - fov[1][2][1] ) +
		    fov[1][0][2] - fov[1][2][2] ) +
	      fabs( fov[1][0][0] - fov[1][0][2] + 
		    2 * ( fov[1][1][0] - fov[1][1][2] ) +
		    fov[1][2][0] - fov[1][2][2] ) ) / 6;
	  
	  Result->Set( value, offset );
	  } 
	else
	  {
	  Result->Set( 0, offset );
	  }
	}
    }
  
  Progress::Done();
  return Result;
}

void 
DataGrid::ApplySobelFilter()
{
  this->SetData( TypedArray::SmartPtr( this->GetDataSobelFiltered() ) );
}

TypedArray* 
DataGrid::GetDataMirrored
( const int axis ) const
{
  const TypedArray* dataArray = this->GetData();
  if ( ! dataArray ) return NULL;
  
  TypedArray* mirroredArray = dataArray->NewTemplateArray();
  
  Progress::SetTotalSteps( Dims[2] );

  size_t offset = 0;
  switch ( axis )
    {
    case AXIS_X:
      for ( int z = 0; z < Dims[2]; ++z ) 
	{
	Progress::SetProgress( z );
	for ( int y = 0; y < Dims[1]; ++y ) 
	  {
	  for ( int x = 0; x < Dims[0]; ++x, ++offset ) 
	    {
	    dataArray->BlockCopy( mirroredArray, offset, offset, Dims[0] );
	    mirroredArray->BlockReverse( offset, Dims[0] );
	    }      
	  }
	}
      break;
    case AXIS_Y:
    case AXIS_Z:
    default:
      StdErr << "DataGrid::GetDataMirrored: Flip not yet implemented\n";
      break;
    }
  
  Progress::Done();
     
  return mirroredArray;
}

ScalarImage*
DataGrid::GetOrthoSlice
( const int axis, const unsigned int plane ) const
{
  unsigned int dims[2], depth, incX, incY, incZ;

  switch ( axis ) 
    {
    case AXIS_X:
      dims[0] = Dims[1];
      dims[1] = Dims[2];
      depth = Dims[0];
      incX = Dims[0];
      incY = Dims[0] * Dims[1];
      incZ = 1;
      break;
    case AXIS_Y:
      dims[0] = Dims[0];
      dims[1] = Dims[2];
      depth = Dims[1];
      incX = 1;
      incY = Dims[0] * Dims[1];
      incZ = Dims[0];
      break;
    case AXIS_Z:
    default:
      dims[0] = Dims[0];
      dims[1] = Dims[1];
      depth = Dims[2];
      incX = 1;
      incY = Dims[0];
      incZ = Dims[0] * Dims[1];
      break;
    }
  
  const TypedArray* data = this->GetData();
  TypedArray* sliceData = data->NewTemplateArray( dims[0] * dims[1] );
  if ( data->GetPaddingFlag() ) 
    {
    sliceData->SetPaddingValue( data->GetPaddingValue() );
    }

  //  if ( (plane < 0) || (plane >= depth) ) {
  if ( (plane >= depth) ) 
    { // need not check < 0 for unsigned int
    sliceData->ClearArray( true /* PaddingData */ );
    } 
  else
    {
    const unsigned int itemSize = data->GetItemSize();
    
    unsigned int sliceOffset = 0;
    unsigned int offset = plane * incZ;
    for ( unsigned int y = 0; y < dims[1]; ++y ) 
      {
      unsigned int offsetY = offset + incY;
      for ( unsigned int x = 0; x < dims[0]; ++x, ++sliceOffset ) 
	{
	unsigned int offsetX = offset + incX;
	
	memcpy( sliceData->GetDataPtr( sliceOffset ), data->GetDataPtr( offset ), itemSize );
	offset = offsetX;
	}
      offset = offsetY;
      }
    }
  
  ScalarImage* sliceImage = new ScalarImage( dims[0], dims[1] );
  sliceImage->SetPixelData( TypedArray::SmartPtr( sliceData ) );
  
  return sliceImage;
}

void 
DataGrid::SetOrthoSlice
( const int axis, const unsigned int idx, const ScalarImage* slice )
{
  const TypedArray* sliceData = slice->GetPixelData();
  if ( ! sliceData ) return;

  TypedArray::SmartPtr data = this->GetData();
  if ( ! data )
    {
    data = this->CreateDataArray( sliceData->GetType() );
    }

  unsigned int dims[2], depth, incX, incY, incZ;

  switch ( axis ) 
    {
    case AXIS_X:
      dims[0] = Dims[1];
      dims[1] = Dims[2];
      depth = Dims[0];
      incX = Dims[0];
      incY = Dims[0] * Dims[1];
      incZ = 1;
      break;
    case AXIS_Y:
      dims[0] = Dims[0];
      dims[1] = Dims[2];
      depth = Dims[1];
      incX = 1;
      incY = Dims[0] * Dims[1];
      incZ = Dims[0];
      break;
    case AXIS_Z:
    default:
      dims[0] = Dims[0];
      dims[1] = Dims[1];
      depth = Dims[2];
      incX = 1;
      incY = Dims[0];
      incZ = Dims[0] * Dims[1];
      break;
    }
  
  if ( (idx < depth) )
    { // need not check > 0 for unsigned int
    unsigned int sliceOffset = 0;
    unsigned int offset = idx * incZ;
    for ( unsigned int y = 0; y < dims[1]; ++y ) 
      {
      unsigned int offsetY = offset + incY;
      for ( unsigned int x = 0; x < dims[0]; ++x, ++sliceOffset ) 
	{
	unsigned int offsetX = offset + incX;
	
	sliceData->BlockCopy( data.GetPtr(), offset, sliceOffset, 1 );
	offset = offsetX;
	}
      offset = offsetY;
      }
    }
}

Vector3D
DataGrid
::GetCenterOfMass() const
{
  Vector3D com( 0, 0, 0 );

  double sumOfSamples = 0;
  size_t ofs = 0;
  for ( int z = 0; z < this->Dims[2]; ++z )
    for ( int y = 0; y < this->Dims[1]; ++y )
      for ( int x = 0; x < this->Dims[0]; ++x, ++ofs )
	{
	Types::DataItem value;
	if ( this->GetDataAt( value, x, y, z ) )
	  {
	  com += Vector3D( value * x, value * y, value * z );
	  sumOfSamples += value;
	  }
	}

  com *= (1.0 / sumOfSamples);

  return com;
}

Vector3D
DataGrid
::GetCenterOfMass( Vector3D& firstOrderMoment ) const
{
  Vector3D com = this->Self::GetCenterOfMass(); // do not use overloaded function
  firstOrderMoment.Set( 0, 0, 0 );

  double sumOfSamples = 0;
  size_t ofs = 0;
  for ( int z = 0; z < this->Dims[2]; ++z )
    for ( int y = 0; y < this->Dims[1]; ++y )
      for ( int x = 0; x < this->Dims[0]; ++x, ++ofs )
	{
	Types::DataItem value;
	if ( this->GetDataAt( value, x, y, z ) )
	  {
	  firstOrderMoment += 
	    Vector3D( value * fabs(x - com.XYZ[0]), value * fabs(y - com.XYZ[1]), value * fabs(z - com.XYZ[2]) );
	  sumOfSamples += value;
	  }
	}
  
  firstOrderMoment *= (1.0 / sumOfSamples);

  return com;
}

void 
DataGrid::Print() const
{
  StdErr.printf( "DataGrid at %p\n", this );

  StdErr.printf( "\tDims: [%d,%d,%d])\n", Dims[0], Dims[1], Dims[2] );
  StdErr.printf( "\tData array at %p\n", this );
}

} // namespace cmtk
