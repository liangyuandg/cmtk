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

#ifndef __cmtkTransformChangeToSpaceAffine_h_included_
#define __cmtkTransformChangeToSpaceAffine_h_included_

#include <cmtkconfig.h>

#include <cmtkAffineXform.h>
#include <cmtkUniformVolume.h>

#include <string>

namespace
cmtk
{

/** \addtogroup Base */
//@{

/// Compute affine coordinate transformation between native spaces of reference and floating images.
class TransformChangeToSpaceAffine
{
public:
  /// Simplified constructor: compute transformation between images in new, common space.
  TransformChangeToSpaceAffine( const AffineXform& xform, //!< Transformation from reference to floating in their current spaces.
				const UniformVolume& reference, //!< Reference (fixed) image.
				const UniformVolume& floating //! Floating (moving) image.
    );

  /// Return transformation in native spaces.
  const AffineXform& GetTransformation() const
  {
    return this->m_NewXform;
  }

private:
  /// Transformation between native spaces.
  AffineXform m_NewXform;
};

//@}

} // namespace cmtk

#endif // #ifndef __cmtkTransformChangeToSpaceAffine_h_included_
