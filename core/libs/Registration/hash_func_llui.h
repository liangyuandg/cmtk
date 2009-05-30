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

#ifndef __hash_func_llui_h_included_
#define __hash_func_llui_h_included_

#if defined(__GNUC__) && ! defined(__INTEL_COMPILER)
namespace __gnu_cxx
{

  template<>
    struct hash<unsigned long long>
    {
      size_t
      operator()(unsigned long long __x) const
      { return (__x & 0xffff) ^ (__x >> 32 ); }
    };

}
#endif // defined(__GNUC__) && ! defined(__INTEL_COMPILER)

#endif
