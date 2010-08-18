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

#include "Registration/cmtkXformListEntry.h"

cmtk::XformListEntry::XformListEntry
( const Xform::SmartConstPtr& xform, const bool inverse, const Types::Coordinate globalScale )
  : m_Xform( xform ), 
    InverseAffineXform( NULL ), 
    m_WarpXform( NULL ),
    Inverse( inverse ), 
    GlobalScale( globalScale )
{
  if ( this->m_Xform ) 
    {
    this->m_WarpXform = dynamic_cast<const WarpXform*>( this->m_Xform.GetConstPtr() );
    
    AffineXform::SmartConstPtr affineXform( AffineXform::SmartConstPtr::DynamicCastFrom( this->m_Xform ) );
    if ( affineXform ) 
      {
      this->InverseAffineXform = affineXform->MakeInverse();
      }
    }
}

cmtk::XformListEntry::~XformListEntry()
{
  // we got the inverse affine from AffineXform::MakeInverse, so we
  // need to get rid of it explicitly.
  delete this->InverseAffineXform;
}

cmtk::XformListEntry::SmartPtr 
cmtk::XformListEntry::CopyAsAffine() const
{
  if ( this->m_WarpXform )
    {
    return Self::SmartPtr( new Self( this->m_WarpXform->m_InitialAffineXform, this->Inverse, this->GlobalScale ) );
    }
  else
    {
    return Self::SmartPtr( new Self( this->m_Xform, this->Inverse, this->GlobalScale ) );
    }
}
