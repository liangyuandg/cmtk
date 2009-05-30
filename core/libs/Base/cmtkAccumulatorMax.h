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

#ifndef __cmtkAccumulatorMax_h_included_
#define __cmtkAccumulatorMax_h_included_

#include <cmtkconfig.h>

namespace
cmtk
{

/** \addtogroup Base */
//@{
namespace
Accumulators
{
/// Maximum-value accumulator.
template<class TDataType>
class Max
{
public:
  /// Default constructor.
  Max()
  {
    this->m_Maximum = 0;
    this->m_Valid = false;
  }

  /// Reset.
  void Reset()
  {
    this->m_Valid = false;
  }

  /// Add a value.
  void AddValue( const TDataType& value )
  {
    if ( !this->m_Valid || value > this->m_Maximum )
      {
      this->m_Maximum = value;
      this->m_Valid = true;
      }
  }

  /// Retrive accumulated result.
  const TDataType GetResult() const
  {
    return this->m_Maximum;
  }

private:
  /// Flag whether there is a valid accumulated value.
  bool m_Valid;

  /// Current maximum.
  TDataType m_Maximum;
};
} // namespace Accumulators
} // namespace cmtk

#endif // #ifdef __cmtkAccumulatorMax_h_included_
