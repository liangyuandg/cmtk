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

#include <cmtkImagePairSimilarityMeasureNCC.h>

namespace
cmtk
{

/** \addtogroup Registration */
//@{

ImagePairSimilarityMeasureNCC
::ImagePairSimilarityMeasureNCC( const UniformVolume::SmartPtr& refVolume, const UniformVolume::SmartPtr& fltVolume ) :
  ImagePairSimilarityMeasure( refVolume, fltVolume )
{}

ImagePairSimilarityMeasureNCC
::ImagePairSimilarityMeasureNCC( Self& other, const bool copyData ) :
  ImagePairSimilarityMeasure( other, copyData )
{
  SumX = other.SumX;
  SumY = other.SumY;
  SumXY = other.SumXY;
  SumSqX = other.SumSqX;
  SumSqY = other.SumSqY;
  Samples = other.Samples;
}

ImagePairSimilarityMeasureNCC
::ImagePairSimilarityMeasureNCC( const Self& other ) :
  ImagePairSimilarityMeasure( other )
{
  SumX = other.SumX;
  SumY = other.SumY;
  SumXY = other.SumXY;
  SumSqX = other.SumSqX;
  SumSqY = other.SumSqY;
  Samples = other.Samples;
}

void
ImagePairSimilarityMeasureNCC
::CopyUnsafe( const ImagePairSimilarityMeasureNCC& other, const bool )
{
  SumX = other.SumX;
  SumY = other.SumY;
  SumXY = other.SumXY;
  SumSqX = other.SumSqX;
  SumSqY = other.SumSqY;
  Samples = other.Samples;
}

ImagePairSimilarityMeasureNCC::ReturnType
ImagePairSimilarityMeasureNCC
::Get() const
{
  const double muX = SumX / Samples;
  const double muY = SumY / Samples;

  const double p = SumXY - muY * SumX - muX * SumY + Samples * muX * muY;
  const double qX = SumSqX - 2.0 * muX * SumX + Samples * muX * muX;
  const double qY = SumSqY - 2.0 * muY * SumY + Samples * muY * muY;
  
  return static_cast<Self::ReturnType>( p / sqrt( qX * qY ) );
}

} // namespace cmtk
