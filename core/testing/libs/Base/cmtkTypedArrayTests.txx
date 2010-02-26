/*
//
//  Copyright 2004-2010 SRI International
//  Copyright 1997-2009 Torsten Rohlfing
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

#include <cmath>
#include <algorithm>

#include <cmtkTypedArray.h>
#include <cmtkTypedArrayFunctionHistogramMatching.h>

// test "TypedArray::MatchHistogramToReference" function
int
testTypedArrayMatchHistogram1()
{
  float data1[] = { 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3 };
  cmtk::TypedArray::SmartPtr array1( cmtk::TypedArray::Create( cmtk::TYPE_FLOAT, data1, sizeof( data1 ) / sizeof( *data1 ), false /*freeArray*/ ) );

  float data2[] = { 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3 };
  cmtk::TypedArray::SmartPtr array2( cmtk::TypedArray::Create( cmtk::TYPE_FLOAT, data2, sizeof( data2 ) / sizeof( *data2 ), false /*freeArray*/ ) );
  array2->ApplyFunction( cmtk::TypedArrayFunctionHistogramMatching( *array2, *array1 ) );

  float maxDeviation = 0;
  for ( size_t i = 0; i<array2->GetDataSize(); ++i )
    {
    cmtk::Types::DataItem value;
    array2->Get( value, i );
    maxDeviation = std::max<float>( maxDeviation, fabs(value-data1[i]) );
    }

  if ( maxDeviation > 0.01 )
    return 1;
  else
    return 0;
}

// test "TypedArray::MatchHistogramToReference" function
int
testTypedArrayMatchHistogram2()
{
  float data1[] = { 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3 };
  cmtk::TypedArray::SmartPtr array1( cmtk::TypedArray::Create( cmtk::TYPE_FLOAT, data1, sizeof( data1 ) / sizeof( *data1 ), false /*freeArray*/ ) );

  float data2[] = { 0.1, 0.1, 0.5, 0.5, 2.5, 2.5, 3.0, 3.0 };
  float base2[] = { 0.0, 0.0, 1.0, 1.0, 2.0, 2.0, 3.0, 3.0 };
  cmtk::TypedArray::SmartPtr array2( cmtk::TypedArray::Create( cmtk::TYPE_FLOAT, data2, sizeof( data2 ) / sizeof( *data2 ), false /*freeArray*/ ) );
  array2->ApplyFunction( cmtk::TypedArrayFunctionHistogramMatching( *array2, *array1 ) );

  for ( size_t i = 0; i<array2->GetDataSize(); ++i )
    {
    cmtk::Types::DataItem value;
    array2->Get( value, i );
    }
  
  float maxDeviation = 0;
  for ( size_t i = 0; i<array2->GetDataSize(); ++i )
    {
    cmtk::Types::DataItem value;
    array2->Get( value, i );
    maxDeviation = std::max<float>( maxDeviation, fabs(value-base2[i]) );
    }
  
  if ( maxDeviation > 0.01 )
    return 1;
  else
    return 0;
}
