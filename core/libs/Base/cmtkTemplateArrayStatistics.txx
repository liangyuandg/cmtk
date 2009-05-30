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

namespace
cmtk
{

/** \addtogroup Base */
//@{

template<class T> size_t
TemplateArray<T>::GetStatistics
( Types::DataItem& mean, Types::DataItem& variance ) const 
{
  size_t Count = 0;
  Types::DataItem Sum = 0, SumOfSquares = 0;
  for ( size_t i = 0; i < DataSize; ++i ) 
    {
    if ( !this->PaddingFlag || (this->Data[i] != this->Padding) ) 
      {
      ++Count;
      Sum += this->Data[i];
      SumOfSquares += MathUtil::Square<Types::DataItem>( this->Data[i] );
      }
    }
  
  if ( Count ) 
    {
    mean = Sum / Count;
    variance = (SumOfSquares - 2*mean*Sum)/Count + MathUtil::Square(mean);
    } 
  else
    {
    variance = mean = 0;
    }
  
  return Count;
}

template<class T>
Histogram<unsigned int>*
TemplateArray<T>::GetHistogram( const unsigned int numberOfBins ) const
{
  Histogram<unsigned int>* histogram = new Histogram<unsigned int>( numberOfBins );

  T min, max;
  this->GetRangeTemplate( min, max );
  histogram->SetRange( min, max );
  
  for ( size_t idx = 0; idx < DataSize; ++idx )
    if ( !this->PaddingFlag || (this->Data[idx] != this->Padding) )
      histogram->Increment( histogram->ValueToBin( this->Data[idx] ) );

  return histogram;
}

template<class T>
Types::DataItem
TemplateArray<T>::EstimateGaussianNoiseSigma
( const size_t histogramBins ) const
{
  Histogram<unsigned int>::SmartPtr histogram( this->GetHistogram( histogramBins ) );
  
  // find first maximum
  size_t idx = 0;
  while ( (idx < (histogramBins-1)) && ( (*histogram)[idx] <= (*histogram)[idx+1] ) )
    {
    ++idx;
    }
  
  const Types::DataItem noiseMean = histogram->BinToValue( idx );

  // now find following minimum
  while ( (idx < (histogramBins-1)) && ( (*histogram)[idx] > (*histogram)[idx+1] ) )
    {
    ++idx;
    }
  
  // then, compute standard deviation of all values below that threshold from
  // first maximum.
  const Types::DataItem threshold = histogram->BinToValue( idx );

  Types::DataItem sdev = 0;
  size_t count = 0;
  for ( size_t i = 0; i < this->DataSize; ++i )
    {
    if ( !this->PaddingFlag || (this->Data[i] != this->Padding) )
      {
      if ( this->Data[i] <= threshold )
	{
        sdev += static_cast<Types::DataItem>( MathUtil::Square( this->Data[i] - noiseMean ) );
	++count;
	}
      }
    }

  if ( count )
    return static_cast<Types::DataItem>( sqrt( sdev/count ) );
  else
    return 0.0;
}

} // namespace cmtk
