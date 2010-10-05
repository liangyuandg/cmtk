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

#ifndef __cmtkQRDecomposition_h_included_
#define __cmtkQRDecomposition_h_included_

#include <cmtkconfig.h>

#include <Base/cmtkMatrix.h>
#include "Numerics/ap.h"

namespace
cmtk
{

/** \addtogroup Base */
//@{

/** Compute the QRDecomposition of a matrix
 */  
template<class TFloat>
class QRDecomposition
{
public:

  /// Matrix type
  typedef Matrix2D<TFloat> matrix2D;
  /// Matrix pointer
  typedef SmartPointer< matrix2D > matrixPtr;

  /// Constructor: compute QR decomposition of given matrix.
  QRDecomposition( const Matrix2D<TFloat>& matrix );

  /// Get the Q factor 
  matrixPtr GetQ();
  
  /// Get the R factor 
  matrixPtr GetR(); 

protected:
  /// Alglib compact QR representation
  ap::real_2d_array compactQR;

  /// Alglib tau array (generated by rmatrixqr, needed to compute Q)
  ap::real_1d_array tau;

  /// Q matrix.
  matrixPtr Q;
  
  /// R matrix.
  matrixPtr R;

private:
   
  /// Number of rows in the input matrix
  int m; 
  /// Number of columns in the input matrix
  int n;
  /// True iff the Q extraction has been performed
  bool extractedQ; 
  /// True iff the R extraction has been performed
  bool extractedR;

};

//@}

} // namespace cmtk

#include "cmtkQRDecomposition.txx"

#endif // #ifndef __cmtkQRDecomposition_h_included_
