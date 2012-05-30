/*
//
//  Copyright 2012 SRI International
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

#ifndef __cmtkDetectedPhantomMagphanEMR051_h_included_
#define __cmtkDetectedPhantomMagphanEMR051_h_included_

#include <cmtkconfig.h>

#include <System/cmtkSmartPtr.h>
#include <System/cmtkSmartConstPtr.h>

#include <Base/cmtkTypes.h>
#include <Base/cmtkLandmarkPair.h>
#include <Base/cmtkAffineXform.h>

#include <list>
#include <string>

namespace
cmtk
{

/** \addtogroup Base */
//@{

/// Description of a detected Magphan EMR051 structural imaging phantom (a.k.a. ADNI Phantom) in an actual image.
class DetectedPhantomMagphanEMR051
{
public:
  /// This class.
  typedef DetectedPhantomMagphanEMR051 Self;
  
  /// Smart pointer to this class.
  typedef SmartPointer<Self> SmartPtr;

  /// Smart pointer to a constant object of this class.
  typedef SmartConstPointer<Self> SmartConstPtr;

  /// Constructor.
  DetectedPhantomMagphanEMR051( const AffineXform& linearFitXform /*!< Fitted linear (including shear and scale) transformation */ ) : m_LinearFitXform( linearFitXform ) {}
    
    
  /// Add expected and actual location of a detected phantom landmark.
  void AddLandmarkPair( const std::string& name /*!< Name of this sphere. */,
			const LandmarkPair::SpaceVectorType& expected /*!< Expected landmark position in physical image coordinates based on linear (not necessarily rigid) fit. */, 
			const LandmarkPair::SpaceVectorType& actual /*!< Actual, detected landmark position in physical image coordinates in the image. */, 
			const Types::Coordinate residual /*!< Residual of landmark fit. */,
			const bool reliable /*!< If true, this landmark is considered reliable, i.e., its expected position by phantom manufacturing is sufficiently precise to be used as a landmark. */ )
  {
    this->m_LandmarkPairs.push_back( LandmarkPair( name, expected, actual, residual, reliable ) );
  }

  /** Apply a transformation to all landmarks.
   * The purpose of this function is mainly to bring all landmark images into a new image space, e.g.,
   * to transform them from phantom image physical space to unwarp image standard space.
   */
  void ApplyXformToLandmarks( const Xform& xform )
  {
    for ( std::list<LandmarkPair>::iterator it = this->m_LandmarkPairs.begin(); it != this->m_LandmarkPairs.end(); ++it )
      {
      xform.ApplyInPlace( it->m_Location );
      xform.ApplyInPlace( it->m_TargetLocation );
      }
  }

  /// Get list of landmark pairs.
  const std::list<LandmarkPair>& LandmarkPairsList() const
  {
    return this->m_LandmarkPairs;
  }
      
  /// Estimated image signal-to-noise ratio..
  Types::DataItem m_EstimatedSNR;

  /// Estimated image contrast-to-noise ratio.
  FixedVector<4,Types::DataItem> m_EstimatedCNR;

  /// Estimated linear transformation fitted to landmarks.
  AffineXform m_LinearFitXform;

  /// Vector of landmark pairs.
  std::list<LandmarkPair> m_LandmarkPairs;
};

//@}

} // namespace cmtk

#endif // #ifndef __cmtkDetectedPhantomMagphanEMR051_h_included_