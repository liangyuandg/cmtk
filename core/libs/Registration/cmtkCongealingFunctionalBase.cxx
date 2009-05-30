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

//#define DEBUG_COMM

#include <cmtkCongealingFunctionalBase.h>

#include <cmtkMathUtil.h>
#include <cmtkVolumeIO.h>

#ifdef CMTK_BUILD_MPI
#  include <mpi.h>
#  include <cmtkMPI.h>
#endif

namespace
cmtk
{

/** \addtogroup Registration */
//@{

template<class TXform,class THistogramBinType>
CongealingFunctionalBase<TXform,THistogramBinType>::CongealingFunctionalBase() :
  m_CropImageHistograms( false )
{
}

template<class TXform,class THistogramBinType>
CongealingFunctionalBase<TXform,THistogramBinType>::~CongealingFunctionalBase()
{
}

template<class TXform,class THistogramBinType>
void
CongealingFunctionalBase<TXform,THistogramBinType>
::SetNumberOfHistogramBins( const size_t numberOfHistogramBins )
{
  this->m_HistogramBins = numberOfHistogramBins;
  if ( this->m_OriginalImageVector.size() )
    {
    std::cerr << "WARNING: you called GroupwiseRegistrationFunctionalBase::SetNumberOfHistogramBins(),\n"
	      << "         but target images were already set. To be safe, I am re-generating\n"
	      << "         pre-scaled images.\n\n";
    this->SetTargetImages( this->m_OriginalImageVector );
    }
}

template<class TXform,class THistogramBinType>
void
CongealingFunctionalBase<TXform,THistogramBinType>
::InterpolateImage
( const size_t idx, byte* const destination )
{
  const size_t numberOfThreads = Threads::GetNumberOfThreads();
  ThreadParameterArray<Self,InterpolateImageThreadParameters> 
    params( this, numberOfThreads );

  for ( size_t thread = 0; thread < numberOfThreads; ++thread )
    {
    params[thread].m_Idx = idx;    
    params[thread].m_Destination = destination;    
    }

  if ( m_ProbabilisticSamples.size() )
    params.RunInParallel( &InterpolateImageProbabilisticThread );
  else
    params.RunInParallel( &InterpolateImageThread );    
}

template<class TXform,class THistogramBinType>
UniformVolume*
CongealingFunctionalBase<TXform,THistogramBinType>
::PrepareSingleImage( UniformVolume::SmartPtr& image )
{
  UniformVolume* newTargetImage = this->Superclass::PrepareSingleImage( image );

  TypedArray::SmartPtr data = newTargetImage->GetData();
  if ( this->m_CropImageHistograms )
    {
    data->PruneHistogram( true, false, this->m_HistogramBins );
    }
  
  Types::DataItem min, max;
  data->GetRange( min, max );
  data->Rescale( 1.0 * (this->m_HistogramBins-1) / (max-min), this->m_HistogramKernelRadiusMax );
  
  newTargetImage->SetData( TypedArray::SmartPtr( data->Convert( TYPE_BYTE ) ) );
  return newTargetImage;
}

template<class TXform,class THistogramBinType>
void
CongealingFunctionalBase<TXform,THistogramBinType>
::PrepareTargetImages()
{
  this->m_ImageVector = this->m_OriginalImageVector;

#ifdef CMTK_BUILD_MPI
  // using MPI, prepare only some of the images locally, obtain others from other nodes
  const size_t imageFrom = this->m_RankMPI;
  const size_t imageSkip = this->m_SizeMPI;
#else
  const size_t imageFrom = 0;
  const size_t imageSkip = 1;
#endif
  for ( size_t i = imageFrom; i < this->m_ImageVector.size(); i += imageSkip )
    {
    this->m_ImageVector[i] = UniformVolume::SmartPtr( this->PrepareSingleImage( this->m_ImageVector[i] ) );
    }
  
#ifdef CMTK_BUILD_MPI
  // obtain filtered, scaled image data from other nodes
  for ( size_t i = 0; i < this->m_ImageVector.size(); ++i )
    {
    cmtk::mpi::Broadcast( MPI::COMM_WORLD, this->m_ImageVector[i], i % this->m_SizeMPI );
    }
#endif

  this->m_PrivateUserBackgroundValue = this->m_UserBackgroundValue + this->m_HistogramKernelRadiusMax;
}

template<class TXform,class THistogramBinType>
CMTK_THREAD_RETURN_TYPE
CongealingFunctionalBase<TXform,THistogramBinType>::InterpolateImageThread
( void* args )
{
  InterpolateImageThreadParameters* threadParameters = static_cast<InterpolateImageThreadParameters*>( args );
  
  const Self* This = threadParameters->thisObject;
  const int threadID = threadParameters->ThisThreadIndex;
  const int numberOfThreads = threadParameters->NumberOfThreads;
  const size_t idx = threadParameters->m_Idx;
  byte* destination = threadParameters->m_Destination;

  const TXform* xform = This->GetXformByIndex(idx);
  const UniformVolume* target = This->m_ImageVector[idx];

  const byte paddingValue = This->m_PaddingValue;
  const byte backgroundValue = This->m_UserBackgroundFlag ? This->m_PrivateUserBackgroundValue : paddingValue;

  Vector3D v;
  byte value;
  const byte* dataPtr = static_cast<const byte*>( target->GetData()->GetDataPtr() );

  const int dimsX = This->m_TemplateGrid->GetDims( AXIS_X );
  const int dimsY = This->m_TemplateGrid->GetDims( AXIS_Y );
  const int dimsZ = This->m_TemplateGrid->GetDims( AXIS_Z );

  const int rowCount = ( dimsY * dimsZ );
  const int rowFrom = ( rowCount / numberOfThreads ) * threadID;
  const int rowTo = ( threadID == (numberOfThreads-1) ) ? rowCount : ( rowCount / numberOfThreads ) * ( threadID + 1 );
  int rowsToDo = rowTo - rowFrom;
  
  int yFrom = rowFrom % dimsY;
  int zFrom = rowFrom / dimsY;
  
  byte *wptr = destination + rowFrom * dimsX;
  for ( int z = zFrom; (z < dimsZ) && rowsToDo; ++z ) 
    {
    for ( int y = yFrom; (y < dimsY) && rowsToDo; yFrom = 0, ++y, --rowsToDo )
      {
      for ( int x = 0; x < dimsX; ++x )
	{
	This->m_TemplateGrid->GetGridLocation( v, x, y, z );
	xform->ApplyInPlaceNonVirtual( v );
	
	if ( target->ProbeData( value, dataPtr, v ) )
	  {
	  *wptr = value;
	  }
	else
	  {
	  *wptr = backgroundValue;
	  }
	
	++wptr;
	}
      }
    }

  return CMTK_THREAD_RETURN_VALUE;
}

template<class TXform,class THistogramBinType>
CMTK_THREAD_RETURN_TYPE
CongealingFunctionalBase<TXform,THistogramBinType>::InterpolateImageProbabilisticThread
( void* args )
{
  InterpolateImageThreadParameters* threadParameters = static_cast<InterpolateImageThreadParameters*>( args );
  
  const Self* This = threadParameters->thisObject;
  const int threadID = threadParameters->ThisThreadIndex;
  const int numberOfThreads = threadParameters->NumberOfThreads;
  const size_t idx = threadParameters->m_Idx;
  byte* destination = threadParameters->m_Destination;

  const TXform* xform = This->GetXformByIndex(idx);
  const UniformVolume* target = This->m_ImageVector[idx];

  const byte paddingValue = This->m_PaddingValue;
  const byte backgroundValue = This->m_UserBackgroundFlag ? This->m_PrivateUserBackgroundValue : paddingValue;

  Vector3D v;
  byte value;
  const byte* dataPtr = static_cast<const byte*>( target->GetData()->GetDataPtr() );

  const size_t startIdx = threadID * (This->m_ProbabilisticSamples.size() / numberOfThreads);
  const size_t endIdx = ( threadID == numberOfThreads ) ? This->m_ProbabilisticSamples.size() : (threadID+1) * (This->m_ProbabilisticSamples.size() / numberOfThreads);

  byte *wptr = destination + startIdx;
  for ( size_t i = startIdx; i < endIdx; ++i, ++wptr )
    {
    const size_t offset = This->m_ProbabilisticSamples[i];
    This->m_TemplateGrid->GetGridLocation( v, offset );
    xform->ApplyInPlaceNonVirtual( v );
    
    if ( target->ProbeData( value, dataPtr, v ) )
      {
      *wptr = value;
      }
    else
      {
      *wptr = backgroundValue;
      }
    }

  return CMTK_THREAD_RETURN_VALUE;
}

//@}

} // namespace cmtk

#include <cmtkAffineXform.h>
#include <cmtkSplineWarpXform.h>

template class cmtk::CongealingFunctionalBase<cmtk::AffineXform>;
template class cmtk::CongealingFunctionalBase<cmtk::SplineWarpXform>;

template class cmtk::CongealingFunctionalBase<cmtk::AffineXform,int>;
template class cmtk::CongealingFunctionalBase<cmtk::SplineWarpXform,int>;

