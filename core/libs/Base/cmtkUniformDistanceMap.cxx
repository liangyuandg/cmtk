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

#include <cmtkUniformDistanceMap.h>

#include <cmtkDataTypeTraits.h>

namespace
cmtk
{

/** \addtogroup Base */
//@{

template<class TDistanceDataType>
UniformDistanceMap<TDistanceDataType>
::UniformDistanceMap
( const UniformVolume* volume, const byte flags, const Types::DataItem value, const Types::DataItem window )
{
  this->BuildDistanceMap( volume, flags, value, window );
}

template<class TDistanceDataType>
void
UniformDistanceMap<TDistanceDataType>
::BuildDistanceMap
( const UniformVolume* volume, const byte flags, const Types::DataItem value, const Types::DataItem window )
{
  this->SetDims( volume->GetDims() ); // set and update internals
  memcpy( Size, volume->Size, sizeof( Size ) );
  memcpy( Delta, volume->GetDelta(), sizeof( Delta ) );

  TypedArray::SmartPtr distanceArray = TypedArray::SmartPtr( TypedArray::Create( DataTypeTraits<DistanceDataType>::DataTypeID, volume->GetNumberOfPixels() ) );
  DistanceDataType *Distance = static_cast<DistanceDataType*>( distanceArray->GetDataPtr() );

  byte inside = ( flags & UniformDistanceMap::INSIDE ) ? 0 : 1;
  byte outside = 1 - inside;

  const TypedArray* Feature = volume->GetData();

  Types::DataItem c;
  DistanceDataType *p = Distance;
  if ( flags & UniformDistanceMap::VALUE_EXACT ) 
    {
    for ( size_t i = 0; i < volume->GetNumberOfPixels(); i++, p++ ) 
      {
      if ( Feature->Get( c, i ) )
	{
	*p = (c == value) ? inside : outside;
	}
      else
	{
	*p = outside;
	}
      }
    } 
  else if ( flags & UniformDistanceMap::VALUE_THRESHOLD ) 
    {
    for ( size_t i = 0; i < volume->GetNumberOfPixels(); i++, p++ ) 
      {
      if ( Feature->Get( c, i ) )
	{
	*p = (c >= value) ? inside : outside;
	}
      else
	{
	*p = outside;
	}
      }
    } 
  else if ( flags & UniformDistanceMap::VALUE_WINDOW ) 
    {
    for ( size_t i = 0; i < volume->GetNumberOfPixels(); i++, p++ ) 
      {
      if ( Feature->Get( c, i ) )
	{
	*p = (fabs(c - value)<=window) ? inside : outside;
	}
      else
	{
	*p = outside;
	}
      }
    } 
  else
    {
    for ( size_t i = 0; i < volume->GetNumberOfPixels(); i++, p++ ) 
      {
      if ( Feature->Get( c, i ) )
	{
	*p = (c) ? inside : outside;
	}
      else
	{
	*p = outside;
	}
      }
    }
  
  this->ComputeEDT( Distance );

  p = Distance;
  for ( size_t i = 0; i < volume->GetNumberOfPixels(); ++i, ++p ) 
  {
#ifdef _MSC_VER
	  *p = static_cast<DistanceDataType>( sqrt( (double)*p ) );
#else
	  *p = static_cast<DistanceDataType>( sqrt( *p ) );
#endif
  }
  this->SetData( distanceArray );
}

template<class TDistanceDataType>
void
UniformDistanceMap<TDistanceDataType>
::ComputeEDT( DistanceDataType *const distance )
{
  const size_t numberOfThreads = Threads::GetNumberOfThreads();

  this->m_G.resize( numberOfThreads );
  this->m_H.resize( numberOfThreads );

  ThreadParameterArray<Self,typename Self::ThreadParametersEDT> params( this, numberOfThreads );
  for ( size_t idx = 0; idx < numberOfThreads; ++idx )
    {
    params[idx].m_Distance = distance;
    }

  params.RunInParallel( ComputeEDTThreadPhase1 );
  params.RunInParallel( ComputeEDTThreadPhase2 );
}

template<class TDistanceDataType>
CMTK_THREAD_RETURN_TYPE
UniformDistanceMap<TDistanceDataType>
::ComputeEDTThreadPhase1
( void* args )
{
  ThreadParametersEDT* params = static_cast<ThreadParametersEDT*>( args );
  Self* This = params->thisObject;
  const Self* ThisConst = This;
  const size_t threadIdx = params->ThisThreadIndex;
  const size_t threadCnt = params->NumberOfThreads;

  /* nXY is number of voxels in each plane (xy) */
  /* nXYZ is number of voxels in 3D image */
  const size_t nXY = ThisConst->Dims[0] * ThisConst->Dims[1];
  
  /* compute D_2 */
  /* call edtComputeEDT_2D for each plane */
  DistanceDataType *p = params->m_Distance + nXY * threadIdx;
  for ( int k = threadIdx; k < ThisConst->Dims[2]; k += threadCnt, p += nXY * threadCnt ) 
    {
    This->ComputeEDT2D( p, This->m_G[threadIdx], This->m_H[threadIdx] );
    }

  return CMTK_THREAD_RETURN_VALUE;
}

template<class TDistanceDataType>
CMTK_THREAD_RETURN_TYPE
UniformDistanceMap<TDistanceDataType>
::ComputeEDTThreadPhase2
( void* args )
{
  ThreadParametersEDT* params = static_cast<ThreadParametersEDT*>( args );
  Self* This = params->thisObject;
  const Self* ThisConst = This;
  const size_t threadIdx = params->ThisThreadIndex;
  const size_t threadCnt = params->NumberOfThreads;

  const size_t nXY = ThisConst->Dims[0] * ThisConst->Dims[1];
  /* compute D_3 */
  /* solve 1D problem for each column (z direction) */
  std::vector<DistanceDataType> f( This->Dims[2] );

  for ( size_t i = threadIdx; i < nXY; i += threadCnt) 
    {
    /* fill array f with D_2 distances in column */
    /* this is essentially line 4 in Procedure VoronoiEDT() in tPAMI paper */
    DistanceDataType *p = params->m_Distance + i;
    DistanceDataType *q = &f[0];
    for ( int k = 0; k < ThisConst->Dims[2]; k++, p += nXY, q++) 
      {
      *q = *p;
      }
    
    /* call edtVoronoiEDT */
    if ( This->VoronoiEDT( &f[0], ThisConst->Dims[2], static_cast<DistanceDataType>( ThisConst->Delta[2] ), This->m_G[threadIdx], This->m_H[threadIdx] ) ) 
      {
      p = params->m_Distance + i;
      DistanceDataType *q = &f[0];
      for ( int k = 0; k < ThisConst->Dims[2]; k++, p += nXY, q++ ) 
	{
	*p = *q;
	}
      }
    }
  
  return CMTK_THREAD_RETURN_VALUE;
}

template<class TDistanceDataType>
void
UniformDistanceMap<TDistanceDataType>
::ComputeEDT2D
( DistanceDataType *const plane, std::vector<DistanceDataType>& gTemp, std::vector<DistanceDataType>& hTemp )
  /*
   * This procedure computes the squared EDT of a 2D binary image with
   * anisotropic voxels. See notes for edtComputeEDT_2D. The difference 
   * relative to edtComputeEDT_2D is that the edt is a float array instead of 
   * a long array, and there are additional parameters for the image voxel 
   * dimensions wX and wY.
   */
{  
  /* compute D_1 as simple forward-and-reverse distance propagation */
  /* (instead of calling edtVoronoiEDT) */
  /* D_1 is distance to closest feature voxel in row (x direction) */
  /* it is possible to use a simple distance propagation for D_1  because */
  /* L_1 and L_2 norms are equivalent for 1D case */
  DistanceDataType *p;
  for ( int j = 0; j < Dims[1]; j++ ) 
    {
    /* forward pass */
    p = plane + j * Dims[0];
    DistanceDataType d = static_cast<DistanceDataType>( EDT_MAX_DISTANCE_SQUARED );
    for ( int i = 0; i < Dims[0]; i++, p++ ) 
      {
      /* set d = 0 when we encounter a feature voxel */
      if ( *p ) 
	{
	*p = d = 0;
	}
      /* increment distance ... */
      else 
	if ( d != EDT_MAX_DISTANCE_SQUARED ) 
	  {
	  *p = ++d;
	  }
      /* ... unless we haven't encountered a feature voxel yet */
	else
	  {
	  *p = static_cast<DistanceDataType>( EDT_MAX_DISTANCE_SQUARED );
	  }
      }
    
    /* reverse pass */
    if ( *(--p) != EDT_MAX_DISTANCE_SQUARED ) 
      {
      DistanceDataType d = static_cast<DistanceDataType>( EDT_MAX_DISTANCE_SQUARED );
      for ( int i = Dims[0] - 1; i >= 0; i--, p-- ) 
	{
	/* set d = 0 when we encounter a feature voxel */
	if ( *p == 0 ) 
	  {
	  d = 0;
	  }
	/* increment distance after encountering a feature voxel */
	else
	  if ( d != EDT_MAX_DISTANCE_SQUARED ) 
	    {
	    /* compare forward and reverse distances */
	    if ( ++d < *p ) 
	      {
	      *p = d;
	      }
	    }
	
	/* square distance */
	/* (we use squared distance in rest of algorithm) */
	*p = static_cast<DistanceDataType>( *p * Delta[0] );
	*p *= *p;
	}
      }
    }
  
  /* compute D_2 = squared EDT */
  /* solve 1D problem for each column (y direction) */
  std::vector<DistanceDataType> f( Dims[1] );
  for ( int i = 0; i < Dims[0]; i++ ) 
    {
    /* fill array f with D_1 distances in column */
    /* this is essentially line 4 in Procedure VoronoiEDT() in tPAMI paper */
    DistanceDataType *p = plane + i;
    DistanceDataType *q = &f[0];
    for ( int j = 0; j < Dims[1]; j++, p += Dims[0], q++) 
      {
      *q = *p;
      }
    
    /* call edtVoronoiEDT */
    if ( this->VoronoiEDT( &f[0], Dims[1], static_cast<DistanceDataType>( Delta[1] ), gTemp, hTemp  ) ) 
      {
      DistanceDataType *p = plane + i;
      DistanceDataType *q = &f[0];
      for ( int j = 0; j < Dims[1]; j++, p += Dims[0], q++ ) 
	{
	*p = *q;
	}
      }
    }
  
} /* edtComputeEDT_2D_anisotropic */

template<class TDistanceDataType>
bool
UniformDistanceMap<TDistanceDataType>
::VoronoiEDT
( DistanceDataType *const distanceSoFar, const int nSize, const DistanceDataType delta,
  std::vector<DistanceDataType>& gTemp, std::vector<DistanceDataType>& hTemp )
{
  long i, l, n_S;
  DistanceDataType a, b, c, v, lhs, rhs;
  
  /* alloc arrays if this is first call to procedure, or if arrays */
  /* are too small and need to be reallocated */
  gTemp.resize( nSize );
  hTemp.resize( nSize );
  
  DistanceDataType* g = &(gTemp[0]);
  DistanceDataType* h = &(hTemp[0]);

  /* construct partial Voronoi diagram */
  /* this loop is lines 1-14 in Procedure edtVoronoiEDT() in tPAMI paper */
  /* note we use 0 indexing in this program whereas paper uses 1 indexing */
  DistanceDataType deltai = 0;
  for (i = 0, l = -1; i < nSize; i++, deltai += delta) 
    {
    /* line 4 */
    if ( distanceSoFar[i] != EDT_MAX_DISTANCE_SQUARED ) 
      {
      /* line 5 */
      if ( l < 1 ) 
	{
	/* line 6 */
	g[++l] = distanceSoFar[i];
	h[l] = deltai;
	}
      /* line 7 */
      else 
	{
	/* line 8 */
	while (l >= 1) 
	  {
	  /* compute removeEDT() in line 8 */
	  v = h[l];
	  a = v - h[l-1];
	  b = deltai - v;
	  c = a + b;
	  /* compute Eq. 2 */
	  if ((c*g[l] - b*g[l-1] - a*distanceSoFar[i] - a*b*c) > 0) 
	    {
	    /* line 9 */
	    l--;
	    } 
	  else 
	    {
	    break;
	    }
	  }
	/* line 11 */
	g[++l] = distanceSoFar[i];
	h[l] = deltai;
	}
      }
    }
  /* query partial Voronoi diagram */
  /* this is lines 15-25 in Procedure edtVoronoiEDT() in tPAMI paper */
  /* lines 15-17 */
  if ((n_S = l + 1) == 0) 
    {
    return false;
    }
  
  /* lines 18-19 */
  deltai = 0;
  for (i = 0, l = 0; i < nSize; i++, deltai += delta) 
    {
    /* line 20 */
    /* we reduce number of arithmetic operations by taking advantage of */
    /* similarities in successive computations instead of treating them as */
    /* independent ones */
    a = h[l] -  deltai;
    lhs = g[l] + a * a;
    while (l < n_S - 1)
      {
      a = h[l+1] - deltai;
      rhs = g[l+1] + a * a;
      if (lhs > rhs) 
	{
	/* line 21 */
	l++;
	lhs = rhs;
	} 
      else
	{
	break;
	}
      }
    
    /* line 23 */
    /* we put distance into the 1D array that was passed; */
    /* must copy into EDT in calling procedure */
    distanceSoFar[i] = lhs;
    }
  
  /* line 25 */
  /* return 1 if we queried diagram, 0 if we returned because n_S = 0 */
  return true;
}

//@}

} // namespace cmtk

template class cmtk::UniformDistanceMap<float>;
template class cmtk::UniformDistanceMap<double>;
template class cmtk::UniformDistanceMap<long int>;
