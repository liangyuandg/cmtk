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
#include <Base/cmtkMathUtil.h>
#include <Base/cmtkMatrix.h>

#include <vector>

// test eigensystem decomposition
int
testMathUtilEigensystem()
{
  const double mdata[] = { 1, 0, 0, 0, 
			   0, -2, 0, 0,
			   0, 0, 0, -4,
			   0, 0, 3, 0 };
  
  cmtk::Matrix2D<double> matrix( 4, 4, mdata );
  
  cmtk::Matrix2D<double> eigensystem( 4, 4 );
  std::vector<double> eigenvalues( 4 );

  cmtk::MathUtil::ComputeEigensystem( matrix, eigensystem, eigenvalues );
  
  return 0;
}

// test eigenvalues computation
int
testMathUtilEigenvalues()
{
  const double mdata[] = { 1, 0, 0, 0, 
			   0, -2, 0, 0,
			   0, 0, 0, -4,
			   0, 0, 3, 0 };
  
  cmtk::Matrix2D<double> matrix( 4, 4, mdata );

  cmtk::Matrix2D<double> eigensystem( 4, 4 );
  std::vector<double> eigenvalues( 4 );
  
  cmtk::MathUtil::ComputeEigenvalues( matrix, eigenvalues );

  return 0;
}

