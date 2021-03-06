/*
//
//  Copyright 2016 Google, Inc.
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2011, 2013 SRI International
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

#include "cmtkVolumeInjectionReconstruction.h"

#include <Base/cmtkHistogramBase.h>
#include <Base/cmtkTypedArrayNoiseEstimatorNaiveGaussian.h>

#include <IO/cmtkVolumeIO.h>

#include <System/cmtkProgress.h>
#include <System/cmtkExitException.h>

#include <algorithm>

#ifdef CMTK_USE_GCD
#  include <dispatch/dispatch.h>
#endif

namespace
cmtk
{

/** \addtogroup Registration */
//@{

VolumeInjectionReconstruction
::VolumeInjectionReconstruction( const UniformVolume* originalImage, const Types::GridIndexType interleaveFactor, const int interleaveAxis )
  : m_NumberOfPasses( interleaveFactor ),
    m_PassWeights( interleaveFactor ),
    m_OriginalImageRange( 0, 0 ),
    m_OriginalImageHistogram(),
    m_CorrectedImageHistogram()
{
  this->m_OriginalImageHistogram = Histogram<double>::SmartPtr( new Histogram<double>( Self::NumberOfHistogramBins ) );
  this->m_CorrectedImageHistogram = Histogram<double>::SmartPtr( new Histogram<double>( Self::NumberOfHistogramBins ) );

  const TypedArray* originalData = originalImage->GetData();
  this->SetupHistogramKernels( originalData );

  this->m_CorrectedImage = UniformVolume::SmartPtr( originalImage->CloneGrid() );
  this->m_CorrectedImage->CreateDataArray( TYPE_FLOAT );
  
  // split original image into subvolumes and store these.
  this->m_OriginalPassImages.clear();
  for ( int pass = 0; pass < this->m_NumberOfPasses; ++pass )
    {
    UniformVolume::SmartPtr passImage( originalImage->GetInterleavedSubVolume( interleaveAxis, this->m_NumberOfPasses, pass ) );
    this->m_OriginalPassImages.push_back( passImage );
    }

  std::fill( m_PassWeights.begin(), m_PassWeights.end(), 1.0 );

  this->m_TransformationsToPassImages.clear();
  for ( int pass = 0; pass < this->m_NumberOfPasses; ++pass )
    {
    this->m_TransformationsToPassImages.push_back( AffineXform::SmartPtr( new AffineXform ) );
    }
}

VolumeInjectionReconstruction
::VolumeInjectionReconstruction( const UniformVolume* reconstructionGrid, std::vector<UniformVolume::SmartPtr>& images )
  : m_NumberOfPasses( images.size() ),
    m_PassWeights( images.size() ),
    m_OriginalImageRange( 0, 0 ),
    m_OriginalImageHistogram( new Histogram<double>( Self::NumberOfHistogramBins ) ),
    m_CorrectedImageHistogram( new Histogram<double>( Self::NumberOfHistogramBins ) )
{
  const TypedArray* originalData = reconstructionGrid->GetData();
  if ( !originalData )
    originalData = images[0]->GetData();
  this->SetupHistogramKernels( originalData );

  this->m_CorrectedImage = UniformVolume::SmartPtr( reconstructionGrid->CloneGrid() );
  this->m_CorrectedImage->CreateDataArray( TYPE_FLOAT );

  this->m_OriginalPassImages = images;
  std::fill( m_PassWeights.begin(), m_PassWeights.end(), 1.0 );

  this->m_TransformationsToPassImages.clear();
  for ( int pass = 0; pass < this->m_NumberOfPasses; ++pass )
    {
    this->m_TransformationsToPassImages.push_back( AffineXform::SmartPtr( new AffineXform ) );
    }
}

int
VolumeInjectionReconstruction
::GuessInterleaveAxis
( const UniformVolume* image, const int defaultAxis )
{
  if ( (image->m_Dims[0] == image->m_Dims[1]) && (image->m_Dims[1] != image->m_Dims[2]) )
    return 2;
  if ( (image->m_Dims[0] == image->m_Dims[2]) && (image->m_Dims[1] != image->m_Dims[2]) )
    return 1;
  if ( (image->m_Dims[1] == image->m_Dims[2]) && (image->m_Dims[1] != image->m_Dims[0]) )
    return 0;

  if ( (image->m_Delta[0] == image->m_Delta[1]) && (image->m_Delta[1] != image->m_Delta[2]) )
    return 2;
  if ( (image->m_Delta[0] == image->m_Delta[2]) && (image->m_Delta[1] != image->m_Delta[2]) )
    return 1;
  if ( (image->m_Delta[1] == image->m_Delta[2]) && (image->m_Delta[1] != image->m_Delta[0]) )
    return 0;

  return defaultAxis;
}

void
VolumeInjectionReconstruction
::SetupHistogramKernels( const TypedArray* originalData )
{
  this->m_OriginalImageRange = originalData->GetRange();
  this->m_CorrectedImageHistogram->SetRange( this->m_OriginalImageRange );
  this->m_OriginalImageHistogram->SetRange( this->m_OriginalImageRange );
  originalData->GetEntropy( *this->m_OriginalImageHistogram, true /*fractional*/ );
  
  const HistogramType::BinType noiseSigma = TypedArrayNoiseEstimatorNaiveGaussian( *originalData, Self::NumberOfHistogramBins ).GetNoiseLevelSigma();
  const HistogramType::BinType kernelSigma = Self::NumberOfHistogramBins * noiseSigma / this->m_OriginalImageRange.Width();
  size_t kernelRadius = static_cast<size_t>( 1 + 2 * kernelSigma );

  // We now make sure kernel radius is large enough to cover any gaps in the original image histogram.
  // This is to avoid infinite KL divergence values
  size_t runLengthZeroes = 1;
  for ( size_t i = 0; i < Self::NumberOfHistogramBins; ++i )
    {
    if ( (*this->m_OriginalImageHistogram)[i] == 0 )
      {
      ++runLengthZeroes;
      kernelRadius = std::max( kernelRadius, runLengthZeroes );
      }
    else
      {
      runLengthZeroes = 0;
      }
    }

  // Create Gaussian kernel using the previously determined sigma and cutoff radius.
  this->m_OriginalImageIntensityNoiseKernel.resize( kernelRadius );
  if ( kernelRadius > 1 )
    {
    const HistogramType::BinType normFactor = 1.0/(sqrt(2*M_PI) * kernelSigma);
    for ( size_t i = 0; i < kernelRadius; ++i )
      {
      this->m_OriginalImageIntensityNoiseKernel[i] = static_cast<HistogramType::BinType>( normFactor * exp( -MathUtil::Square( 1.0 * i / kernelSigma ) / 2 ) );
      }
    }
  else
    {
    this->m_OriginalImageIntensityNoiseKernel[0] = static_cast<HistogramType::BinType>( 1 );
    }

  // Finally, get original image histogram again, this time using the previously determined kernel.
  originalData->GetEntropy( *this->m_OriginalImageHistogram, &this->m_OriginalImageIntensityNoiseKernel[0], this->m_OriginalImageIntensityNoiseKernel.size() );
}

void
VolumeInjectionReconstruction
::ComputeTransformationsToPassImages( const int registrationMetric )
{
  this->m_TransformationsToPassImages.clear();
  
  UniformVolume::SmartPtr registerToImage = this->m_ReferenceImage ? this->m_ReferenceImage : this->m_OriginalPassImages[0];
  for ( int pass = 0; pass < this->m_NumberOfPasses; ++pass )
    {
    if ( registerToImage == this->m_OriginalPassImages[pass] )
      {
      // set identity transformation if current subvolume is used as the reference.
      this->m_TransformationsToPassImages.push_back( AffineXform::SmartPtr( new AffineXform ) );
      }
    else
      {
      // run the registration
      AffineRegistration ar;
      ar.SetVolume_1( registerToImage );
      ar.SetVolume_2( this->m_OriginalPassImages[pass] );
      
      ar.AddNumberDOFs( 6 );
      
      ar.SetInitialAlignCenters( false );
      ar.SetNoSwitch( true );
      
      ar.SetMetric( registrationMetric );
      ar.SetExploration( 4 * this->m_CorrectedImage->GetMaxDelta() );
      ar.SetAccuracy( .1 * this->m_CorrectedImage->GetMinDelta() );
      ar.SetSampling( 2 * this->m_CorrectedImage->GetMaxDelta() ); 
      
      ar.Register();
      
      this->m_TransformationsToPassImages.push_back( ar.GetTransformation() );
      }
    }
}

void
VolumeInjectionReconstruction
::VolumeInjectionAnisotropic( const Types::Coordinate kernelSigmaFactor, const Types::Coordinate kernelRadiusFactor )
{
  const Types::Coordinate minusOneOverTwoSigmaSquare = -1 / (2 * kernelSigmaFactor*kernelSigmaFactor);
		    
  UniformVolume::SmartPtr& correctedImage = this->m_CorrectedImage;
  TypedArray::SmartPtr correctedImageData = correctedImage->GetData();
  const Types::GridIndexType correctedImageNumPixels = correctedImage->GetNumberOfPixels();
  const Types::Coordinate correctedDelta[3] = { correctedImage->m_Delta[0], correctedImage->m_Delta[1], correctedImage->m_Delta[2] };

  this->m_NeighorhoodMaxPixelValues.setbounds( 1, correctedImageNumPixels );
  this->m_NeighorhoodMinPixelValues.setbounds( 1, correctedImageNumPixels );
  for ( Types::GridIndexType i = 1; i <= correctedImageNumPixels; ++i )
    {
    this->m_NeighorhoodMaxPixelValues(i) = this->m_OriginalImageRange.m_LowerBound;
    this->m_NeighorhoodMinPixelValues(i) = this->m_OriginalImageRange.m_UpperBound;
    }

  Progress::Begin( 0, correctedImageNumPixels, 1e5, "Anisotropic Volume Injection" );

// The following is currently broken due to Apple bug:
//  http://forums.macrumors.com/showthread.php?t=952857 
//  http://lists.apple.com/archives/perfoptimization-dev/2009/Sep/msg00043.html
//#ifdef CMTK_USE_GCD
//  const cmtk::Threads::Stride stride( correctedImageNumPixels );
//  dispatch_apply( stride.NBlocks(), dispatch_get_global_queue(0, 0), ^(size_t b)
//		  { for ( size_t correctedPx = stride.From( b ); correctedPx < stride.To( b ); ++correctedPx )
//#else
#pragma omp parallel for schedule(dynamic)
  for ( Types::GridIndexType correctedPx = 0; correctedPx < static_cast<Types::GridIndexType>( correctedImageNumPixels ); ++correctedPx )
//#endif
    {
      if ( (correctedPx % ((Types::GridIndexType)1e5)) == 0 )
      Progress::SetProgress( correctedPx );

    ap::real_value_type sum = 0;
    ap::real_value_type weight = 0;

    const UniformVolume::CoordinateVectorType vCorrected = correctedImage->GetGridLocation( correctedPx );
    
    for ( int pass = 0; pass < this->m_NumberOfPasses; ++pass )
      {
      const ap::real_value_type passImageWeight = this->m_PassWeights[pass];
      
      if ( passImageWeight > 0 )
	{
	const UniformVolume* passImage = this->m_OriginalPassImages[pass];
	const DataGrid::IndexType& passImageDims = passImage->GetDims();
	const Xform* passImageXform = this->m_TransformationsToPassImages[pass];

	const  UniformVolume::CoordinateVectorType vPass = passImageXform->Apply( vCorrected );

	Types::GridIndexType passGridPosition[3];
	passImage->GetVoxelIndexNoBounds( vPass, passGridPosition );

	Types::GridIndexType passGridFrom[3], passGridTo[3];
	for ( int n = 0; n < 3; ++n )
	  {
	  passGridFrom[n] = std::max<Types::GridIndexType>( passGridPosition[n] - static_cast<Types::GridIndexType>( kernelRadiusFactor ), 0 );
	  passGridTo[n] = std::min<Types::GridIndexType>( passGridPosition[n] + static_cast<Types::GridIndexType>( kernelRadiusFactor ) + 1, passImageDims[n] );
	  }

	UniformVolume::CoordinateVectorType u, delta;
	for ( Types::GridIndexType k = passGridFrom[2]; k < passGridTo[2]; ++k )
	  {
	  u[2] = passImage->GetPlaneCoord( AXIS_Z, k );
	  delta[2] = (u[2] - vPass[2]) / correctedDelta[2];
	      
	  for ( Types::GridIndexType j = passGridFrom[1]; j < passGridTo[1]; ++j )
	    {
	    u[1] = passImage->GetPlaneCoord( AXIS_Y, j );
	    delta[1] = (u[1] - vPass[1]) / correctedDelta[1];
	    
	    for ( Types::GridIndexType i = passGridFrom[0]; i < passGridTo[0]; ++i )
	      {                
	      u[0] = passImage->GetPlaneCoord( AXIS_X, i );
	      delta[0] = (u[0] - vPass[0]) / correctedDelta[0];
	      
	      Types::DataItem passImageData;
	      if ( passImage->GetDataAt( passImageData, i, j, k ) ) 
		{
		const Types::Coordinate mahalanobis = delta.RootSumOfSquares();
		if ( mahalanobis <= kernelRadiusFactor )
		  {
		  const ap::real_value_type kernelWeightPixel = passImageWeight * exp( mahalanobis*mahalanobis * minusOneOverTwoSigmaSquare );
		  
		  sum += passImageData * kernelWeightPixel;
		  weight += kernelWeightPixel;
		  
		  if ( passImageData < this->m_NeighorhoodMinPixelValues(correctedPx+1) )
		    this->m_NeighorhoodMinPixelValues(correctedPx+1) = passImageData;
		  if ( passImageData > this->m_NeighorhoodMaxPixelValues(correctedPx+1) )
		    this->m_NeighorhoodMaxPixelValues(correctedPx+1) = passImageData;
		  }
		}
	      }
	    }
	  }
	}
      }
    if ( weight > 0 )
      correctedImageData->Set( static_cast<Types::DataItem>( sum / weight ), correctedPx );
    else
      correctedImageData->SetPaddingAt( correctedPx );
    }
//#ifdef CMTK_USE_GCD
//		  });
//#endif

  Progress::Done();
}

void
VolumeInjectionReconstruction
::VolumeInjectionIsotropic( const Types::Coordinate kernelSigma, const Types::Coordinate kernelRadius )
{
  const Types::GridIndexType correctedImageNumPixels = this->m_CorrectedImage->GetNumberOfPixels();
  const DataGrid::IndexType& splattedImageDims = this->m_CorrectedImage->GetDims();

  this->m_CorrectedImage->GetData()->ClearArray();
  
  this->m_NeighorhoodMaxPixelValues.setbounds( 1, correctedImageNumPixels );
  this->m_NeighorhoodMinPixelValues.setbounds( 1, correctedImageNumPixels );
  for ( Types::GridIndexType i = 1; i <= correctedImageNumPixels; ++i )
    {
    this->m_NeighorhoodMaxPixelValues(i) = this->m_OriginalImageRange.m_LowerBound;
    this->m_NeighorhoodMinPixelValues(i) = this->m_OriginalImageRange.m_UpperBound;
    }

  const Types::GridIndexType kernelRadiusIndex[3] = 
  {
    1 + static_cast<Types::GridIndexType>( kernelRadius / this->m_CorrectedImage->m_Delta[0] ),
    1 + static_cast<Types::GridIndexType>( kernelRadius / this->m_CorrectedImage->m_Delta[1] ),
    1 + static_cast<Types::GridIndexType>( kernelRadius / this->m_CorrectedImage->m_Delta[2] )
  };

  const Types::Coordinate kernelRadiusSquare = kernelRadius * kernelRadius;
  const Types::Coordinate minusOneOverTwoSigmaSquare = -1 / (2 * kernelSigma*kernelSigma);

  std::vector<ap::real_value_type> kernelWeights( correctedImageNumPixels );
  std::fill( kernelWeights.begin(), kernelWeights.end(), 0 );
      
  std::vector<ap::real_value_type> splattedImage( correctedImageNumPixels );
  std::fill( splattedImage.begin(), splattedImage.end(), 0 );  
  
  Progress::Begin( 0, this->m_NumberOfPasses, 1, "Isotropic Volume Injection" );

  for ( int pass = 0; pass < this->m_NumberOfPasses; ++pass )
    {
    Progress::SetProgress( pass );

    const ap::real_value_type passImageWeight = this->m_PassWeights[pass];

    if ( passImageWeight > 0 )
      {
      const UniformVolume* passImage = this->m_OriginalPassImages[pass];
      AffineXform::SmartPtr affinePassImageXform = AffineXform::SmartPtr::DynamicCastFrom( this->m_TransformationsToPassImages[pass] );
      const AffineXform* passImageXformInverse = (affinePassImageXform) ? affinePassImageXform->GetInverse() : static_cast<const AffineXform*>( NULL );
      
      const Types::GridIndexType passImageNumPixels = passImage->GetNumberOfPixels();
      for ( Types::GridIndexType offset = 0; offset < passImageNumPixels; ++offset )
	{      
	Types::DataItem passImageData;
	if ( passImage->GetDataAt( passImageData, offset ) ) 
	  {
	  Types::GridIndexType x, y, z;
	  passImage->GetIndexFromOffset( offset, x, y, z );
	  
	  UniformVolume::CoordinateVectorType v = passImage->GetGridLocation( x, y, z );
	  if ( passImageXformInverse )
	    {
	    v = passImageXformInverse->Apply( v );
	    }
	  else
	    {
	    if ( ! this->m_TransformationsToPassImages[pass]->ApplyInverse( v, v ) )
	      {
	      StdErr << "ERROR: failed to apply inverse transformation in VolumeInjectionReconstruction::VolumeInjectionIsotropic\n";
	      throw ExitException( 1 );
	      }
	    }
	  
	  Types::GridIndexType targetGridPosition[3];
	  if ( this->m_CorrectedImage->FindVoxel( v, targetGridPosition ) )
	    {
	    // check if neighbours are outside - if yes, set new neighbour ranges
	    Types::GridIndexType targetGridFrom[3], targetGridTo[3];
	    for ( int n = 0; n < 3; ++n )
	      {
	      targetGridFrom[n] = std::max<Types::GridIndexType>( targetGridPosition[n] - kernelRadiusIndex[n], 0 );
	      targetGridTo[n] = std::min<Types::GridIndexType>( targetGridPosition[n] + kernelRadiusIndex[n] + 1, splattedImageDims[n] );
	      }
	    
	    UniformVolume::CoordinateVectorType u;
	    for ( Types::GridIndexType k = targetGridFrom[2]; k < targetGridTo[2]; ++k )
	      {
	      u[2] = this->m_CorrectedImage->GetPlaneCoord( AXIS_Z, k );
	      
	      for ( Types::GridIndexType j = targetGridFrom[1]; j < targetGridTo[1]; ++j )
		{
		u[1] = this->m_CorrectedImage->GetPlaneCoord( AXIS_Y, j );
		
		Types::GridIndexType splattedImageOffset = this->m_CorrectedImage->GetOffsetFromIndex( targetGridFrom[0], j, k );
		for ( Types::GridIndexType i = targetGridFrom[0]; i < targetGridTo[0]; ++i, ++splattedImageOffset )
		  {                
		  u[0] = this->m_CorrectedImage->GetPlaneCoord( AXIS_X, i );
		  
		  const Types::Coordinate distanceSquare = ( u-v ).SumOfSquares();
		  if ( distanceSquare <= kernelRadiusSquare )
		    {
		    const ap::real_value_type kernelWeightPixel = passImageWeight * exp( distanceSquare * minusOneOverTwoSigmaSquare );
		    
		    splattedImage[splattedImageOffset] += passImageData * kernelWeightPixel;
		    kernelWeights[splattedImageOffset] += kernelWeightPixel;
		    
		    if ( passImageData < this->m_NeighorhoodMinPixelValues(splattedImageOffset+1) )
		      this->m_NeighorhoodMinPixelValues(splattedImageOffset+1) = passImageData;
		    if ( passImageData > this->m_NeighorhoodMaxPixelValues(splattedImageOffset+1) )
		      this->m_NeighorhoodMaxPixelValues(splattedImageOffset+1) = passImageData;
		    }
		  }
		}
	      }
	    }
	  }
	}      
      }
    }
  
#pragma omp parallel for
  for ( Types::GridIndexType idx = 0; idx < static_cast<Types::GridIndexType>( correctedImageNumPixels ); ++idx )
    {
    const Types::DataItem kernelWeightPixel = static_cast<Types::DataItem>( kernelWeights[idx] );
    if ( kernelWeightPixel > 0 ) // check if pixel is a neighbour
      {
      this->m_CorrectedImage->SetDataAt( static_cast<Types::DataItem>( splattedImage[idx] / kernelWeightPixel ), idx ); // Set normalized data on grid
      }
    }
  
  Progress::Done();
}

UniformVolume::SmartPtr&
VolumeInjectionReconstruction
::GetCorrectedImage()
{
  return this->m_CorrectedImage;
}

void
VolumeInjectionReconstruction
::SetReferenceImage( UniformVolume::SmartPtr& referenceImage )
{
  this->m_ReferenceImage = referenceImage;
}

ap::real_value_type
VolumeInjectionReconstruction
::GetOriginalToCorrectedImageKLD( const ap::real_1d_array& x )
{
  this->m_CorrectedImageHistogram->Reset();
  for ( int i = x.getlowbound(); i <= x.gethighbound(); ++i )
    this->m_CorrectedImageHistogram->AddWeightedSymmetricKernel
	( this->m_CorrectedImageHistogram->ValueToBin( static_cast<Types::DataItem>( x(i) ) ), this->m_OriginalImageIntensityNoiseKernel.size(), &this->m_OriginalImageIntensityNoiseKernel[0] );
  const ap::real_value_type kld = this->m_CorrectedImageHistogram->GetKullbackLeiblerDivergence( *this->m_OriginalImageHistogram );

  return kld;
}

ap::real_value_type
VolumeInjectionReconstruction
::ComputeCorrectedImageLaplacianNorm( const ap::real_1d_array& correctedImagePixels )
{
  const UniformVolume* correctedImage = this->m_CorrectedImage;
  const Types::GridIndexType correctedImageNumPixels = correctedImage->GetNumberOfPixels();
  this->m_CorrectedImageLaplacians.resize( correctedImageNumPixels );

  const DataGrid::IndexType& correctedImageDims = correctedImage->GetDims();
  const Types::GridIndexType nextI = 1;
  const Types::GridIndexType nextJ = nextI * correctedImageDims[0];
  const Types::GridIndexType nextK = nextJ * correctedImageDims[1];

  ap::real_value_type lnorm = 0;
#pragma omp parallel for reduction(+:lnorm)
  for ( Types::GridIndexType idx = 1; idx <= static_cast<Types::GridIndexType>( correctedImageNumPixels ); ++idx )
    {
    Types::GridIndexType x, y, z;
    correctedImage->GetIndexFromOffset( idx-1, x, y, z );
    
    const Types::GridIndexType xm = (x>0) ? idx-nextI : idx+nextI;
    const Types::GridIndexType ym = (y>0) ? idx-nextJ : idx+nextJ;
    const Types::GridIndexType zm = (z>0) ? idx-nextK : idx+nextK;
    
    const Types::GridIndexType xp = (x+1 < correctedImageDims[0]) ? idx+nextI : idx-nextI;
    const Types::GridIndexType yp = (y+1 < correctedImageDims[1]) ? idx+nextJ : idx-nextJ;
    const Types::GridIndexType zp = (z+1 < correctedImageDims[2]) ? idx+nextK : idx-nextK;
    
    const ap::real_value_type l = 
      correctedImagePixels( xm ) + correctedImagePixels( xp ) +
      correctedImagePixels( ym ) + correctedImagePixels( yp ) +
      correctedImagePixels( zm ) + correctedImagePixels( zp ) - 
      6 * correctedImagePixels( idx );
    
    this->m_CorrectedImageLaplacians[idx-1] = l;
    lnorm += l*l;
    }

  if ( correctedImageNumPixels )
    lnorm /= correctedImageNumPixels;
  
  return lnorm;
}

void
VolumeInjectionReconstruction
::AddLaplacianGradientImage( ap::real_1d_array& g, const ap::real_1d_array&, const ap::real_value_type weight ) const
{
  const UniformVolume* correctedImage = this->m_CorrectedImage;
  const Types::GridIndexType correctedImageNumPixels = correctedImage->GetNumberOfPixels();
  const DataGrid::IndexType& correctedImageDims = correctedImage->GetDims();

  const Types::GridIndexType nextI = 1;
  const Types::GridIndexType nextJ = nextI * correctedImageDims[0];
  const Types::GridIndexType nextK = nextJ * correctedImageDims[1];
  
#pragma omp parallel for
  for ( Types::GridIndexType idx = 0; idx < static_cast<Types::GridIndexType>( correctedImageNumPixels ); ++idx )
    {
    Types::GridIndexType x, y, z;
    correctedImage->GetIndexFromOffset( idx, x, y, z );
    
    const Types::GridIndexType xm = (x>0) ? idx-nextI : idx+nextI;
    const Types::GridIndexType ym = (y>0) ? idx-nextJ : idx+nextJ;
    const Types::GridIndexType zm = (z>0) ? idx-nextK : idx+nextK;

    const Types::GridIndexType xp = (x+1 < correctedImageDims[0]) ? idx+nextI : idx-nextI;
    const Types::GridIndexType yp = (y+1 < correctedImageDims[1]) ? idx+nextJ : idx-nextJ;
    const Types::GridIndexType zp = (z+1 < correctedImageDims[2]) ? idx+nextK : idx-nextK;

    g(idx+1) += (2 * weight / correctedImageNumPixels) *
      ( this->m_CorrectedImageLaplacians[xm] + this->m_CorrectedImageLaplacians[xp] +
	this->m_CorrectedImageLaplacians[ym] + this->m_CorrectedImageLaplacians[yp] +
	this->m_CorrectedImageLaplacians[zm] + this->m_CorrectedImageLaplacians[zp] - 
	6 * this->m_CorrectedImageLaplacians[idx] );
    }
}

} // namespace cmtk
