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

#include <cmtkMathUtil.h>

#include <cmtkMatrix.h>
#include <cmtkConsole.h>

#include <math.h>
#include <algorithm>
#include <vector>

#include <sevd.h>
#include <spddet.h>
#include <svd.h>

namespace
cmtk
{

/** \addtogroup Base */
//@{

namespace
MathUtil
{

template<class T>
void
ComputeEigensystem
( const Matrix2D<T>& matrix, Matrix2D<T>& eigenvectors,  Array<T>& eigenvalues )
{
  const size_t n = matrix.GetNumberOfColumns();

  /*  Convert Matrix2D to ap::real_2d_array
   *  and Array to ap::real_1d_array
   */
  ap::real_2d_array apMatrix;
  apMatrix.setbounds(0, (int)matrix.GetNumberOfRows(), 0, (int)matrix.GetNumberOfColumns());
  for (size_t j = 0; j < n; j++)
    for (size_t i = 0; i < n; i++)
      apMatrix(i,j) = (double)(1.0 * matrix[i][j]);

  ap::real_1d_array apEigenvalues;
  apEigenvalues.setbounds(0, eigenvalues.GetDim());
  for (size_t i = 0; i < eigenvalues.GetDim(); i++)
        apEigenvalues(i) = (double)(1.0 * eigenvalues[i]);


  ap::real_2d_array apEigenvectors;
  apEigenvectors.setbounds(0, (int)matrix.GetNumberOfRows(), 0, (int)matrix.GetNumberOfColumns());

  /*  Run AlgLib eigensystem computation
   */
  if ( ! smatrixevd(apMatrix, (int)n, 1, true, apEigenvalues, apEigenvectors) )
    {
    StdErr << "WARNING: Something went wrong in smatrixevd\n";
    }

  /*  Convert ap::real_1d_array and ap::real_2d_array
   *  back to our data types.
   */

  for (size_t j = 0; j < n; j++)
    for (size_t i = 0; i < n; i++)
        eigenvectors[i][j] = static_cast<T>( apEigenvectors(i,j) );

  for (size_t i = 0; i < eigenvalues.GetDim(); i++)
    eigenvalues[i] = static_cast<T>( apEigenvalues(i) );
}

template void ComputeEigensystem<float>( const Matrix2D<float>& matrix, Matrix2D<float>& eigensystem, Array<float>& eigenvalues );
template void ComputeEigensystem<double>( const Matrix2D<double>& matrix, Matrix2D<double>& eigensystem, Array<double>& eigenvalues );

template<class T>
void
ComputeEigenvalues
( const Matrix2D<T>& matrix, Array<T>& eigenvalues )
{
  const size_t n = matrix.GetNumberOfColumns();

  /*  Convert Matrix2D to ap::real_2d_array
   *  and Array to ap::real_1d_array
   */
  ap::real_2d_array apMatrix;
  apMatrix.setbounds(0, (int)matrix.GetNumberOfRows(), 0, (int)matrix.GetNumberOfColumns());
  for (size_t j = 0; j < n; j++)
    for (size_t i = 0; i < n; i++)
      apMatrix(i,j) = (double)(1.0 * matrix[i][j]);

  ap::real_1d_array apEigenvalues;
  apEigenvalues.setbounds(0, eigenvalues.GetDim());
  for (size_t i = 0; i < eigenvalues.GetDim(); i++)
        apEigenvalues(i) = (double)(1.0 * eigenvalues[i]);

  ap::real_2d_array nullArray;

  /*  Run AlgLib eigenvalues computation
   */
  if ( !smatrixevd(apMatrix, (int)n, 0, true, apEigenvalues, nullArray) )
    {
    StdErr << "WARNING: Something went wrong in smatrixevd\n";
    }

  /*  Convert ap::real_1d_array and ap::real_2d_array
   *  back to our data types.
   */

  for (size_t i = 0; i < n; i++)
    for (size_t j = 0; j < n; j++)
        matrix[i][j] = static_cast<T>( apMatrix(i,j) );

  for (size_t i = 0; i < eigenvalues.GetDim(); i++)
    eigenvalues[i] = static_cast<T>( apEigenvalues(i) );
}

template void ComputeEigenvalues<float>( const Matrix2D<float>& matrix, Array<float>& eigenvalues );
template void ComputeEigenvalues<double>( const Matrix2D<double>& matrix, Array<double>& eigenvalues );

void 
SVD( double **U, size_t m, size_t n, double *W, double **V )
{
  ap::real_2d_array apA;
  apA.setbounds(0, m-1, 0, n-1);
  for (size_t j = 0; j < n; j++)
    for (size_t i = 0; i < m; i++)
      apA(i,j) = (double) U[i+1][j+1];

  ap::real_1d_array w;
  ap::real_2d_array u;
  ap::real_2d_array vt;
  w.setbounds( 0, n-1 );
  u.setbounds( 0, m-1, 0, n-1 );
  vt.setbounds( 0, m-1, 0, n-1 );

  rmatrixsvd( apA, m, n, 
              true /* U needed */, 
              true /* V needed */, 
              1 /*medium-level memory usage */, 
              w, u, vt);

  /* Put u in U */
  for (size_t j = 0; j < n; j++)
    for (size_t i = 0; i < m; i++)
      U[i+1][j+1] = u(i,j);
  
  /* Put w in W */
  for (size_t i = 0; i < n; i++)
    W[i+1] = w(i);
  
  /* Un-transpose vt and put it in V */
  for (size_t j = 0; j < n; j++)
    for (size_t i = 0; i < n; i++)
      V[i+1][j+1] = vt(j,i);

}


/////////////////////////////////////////////////////////////////////
// HELPERS
/////////////////////////////////////////////////////////////////////

template<class T> 
T
CholeskyDeterminant
(const Matrix2D<T>& matrix, int n)
{    
  ap::real_2d_array apMatrix;
  apMatrix.setbounds(0, n-1, 0, n-1);
  for (int j = 0; j < n; j++)
    for (int i = 0; i < n; i++)
      apMatrix(i,j) = (double)(1.0 * matrix[i][j]);
  spdmatrixcholesky( apMatrix, n, false );
  T determinant = (T) spdmatrixcholeskydet( apMatrix, n );
  return determinant;
}

template double CholeskyDeterminant<double>(const Matrix2D<double>& matrix, int n);
template float CholeskyDeterminant<float>(const Matrix2D<float>& matrix, int n);

} // namespace MathUtil
} // namespace cmtk

