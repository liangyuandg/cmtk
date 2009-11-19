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

#ifndef __cmtkVoxelMatchingMeanSquaredDifference_h_included_
#define __cmtkVoxelMatchingMeanSquaredDifference_h_included_

#include <cmtkconfig.h>

#include <cmtkVoxelMatchingMetric.h>

#include <cmtkUniformVolume.h>
#include <cmtkTypedArray.h>
#include <cmtkMathUtil.h>
#include <cmtkSmartPtr.h>

namespace
cmtk
{

/** \addtogroup Registration */
//@{
#ifdef _MSC_VER
#pragma warning (disable:4521)
#endif
/** Mean squared difference metric.
 */
class VoxelMatchingMeanSquaredDifference :
  /// Inherit generic voxel metric with internal short data.
  public VoxelMatchingMetricShort
{
public:
  /// This type.
  typedef VoxelMatchingMeanSquaredDifference Self;

  /// Smart pointer.
  typedef SmartPointer<Self> SmartPtr;

  /** Constructor.
   * For reference and model volume, InitDataset is called.
   *@param refVolume The reference (fixed) volume.
   *@param modVolume The model (transformed) volume.
   */
  VoxelMatchingMeanSquaredDifference
  ( const UniformVolume* refVolume, const UniformVolume* fltVolume );

  /// UNDOCUMENTED
  VoxelMatchingMeanSquaredDifference( const Self& other );
  
  /// UNDOCUMENTED
  VoxelMatchingMeanSquaredDifference( Self& other, const bool copyData = false );
  
  /// UNDOCUMENTED
  void Copy( const VoxelMatchingMeanSquaredDifference& other,
	     const bool copyData = false ) 
  {
    this->CopyUnsafe( other, copyData );
  }

  /// UNDOCUMENTED
  void CopyUnsafe( const VoxelMatchingMeanSquaredDifference& other,
		   const bool copyData = false );
  
  /** Add a pair of values to the metric.
   */
  template<class T> void Increment( const T a, const T b )
  {
    if ( (a == this->DataX.padding()) || (b == this->DataY.padding()) ) return;
    ++Samples;
    Sum -= MathUtil::Square( a - b );
  }

  /** Remove a pair of values from the metric.
   */
  template<class T> void Decrement( const T a, const T b )
  {
    if ( (a == this->DataX.padding()) || (b == this->DataY.padding()) ) return;
    --Samples;
    Sum += MathUtil::Square( a - b );
  }

  /// Reset internal variables for next computation.
  void Reset () 
  {
    Sum = 0;
    Samples = 0;
  }

  /// UNDOCUMENTED
  Self::ReturnType Get() const 
  {
	  return static_cast<Self::ReturnType>( Sum / Samples );
  }

  /// UNDOCUMENTED
  void AddSampleToBins ( const short sampleX, const short sampleY )
  {
    if ( (sampleX == this->DataX.padding()) || (sampleY == this->DataY.padding()) ) return;
    ++Samples;
    Sum -= MathUtil::Square( sampleX - sampleY );
  }

  /// UNDOCUMENTED
  void RemoveSampleFromBins ( const short sampleX, const short sampleY )
  {
    if ( (sampleX == this->DataX.padding()) || (sampleY == this->DataY.padding()) ) return;
    --Samples;
    Sum += MathUtil::Square( sampleX - sampleY );
  }

  /**
   * Don't let the Sum += ... confuse you!! Sign is already negative from
   * storing in other metric.
   */
  void AddJointHistogram ( const Self& other )
  {
    Sum += other.Sum;
    Samples += other.Samples;
  }

  /**
   * Don't let the Sum -= ... confuse you!! Sign is already negative from
   * storing in other metric.
   */
  void RemoveJointHistogram ( const Self& other )
  {
    Sum -= other.Sum;
    assert( Sum <= 0 );
    Samples -= other.Samples;
    assert( Samples >= 0 );
  }

private:
  /// UNDOCUMENTED
  double Sum;

  /// UNDOCUMENTED
  int Samples;
};

//@}

} // namespace cmtk

#endif // #ifndef __cmtkVoxelMatchingMeanSquaredDifference_h_included_
