/*
//
//  Copyright 2009 SRI International
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

#include <typeinfo>

template<class T>
void
cmtk::CommandLine::Vector<T>
::Evaluate( const size_t argc, const char* argv[], size_t& index )
{
  if ( index+1 < argc ) 
    {
    } 
  else
    {
    throw( Exception( "Vector command line option needs an argument.", index ) );
    }
}

template<class T>
mxml_node_t* 
cmtk::CommandLine::Vector<T>
::MakeXML(  mxml_node_t *const parent ) const 
{
  if ( ! (this->m_Properties & PROPS_NOXML) )
    {
    mxml_node_t *node = Item::Helper<T>::MakeXML( this, parent );
    mxmlElementSetAttr( node, "multiple", "true" );
    
    return node;
    }
  return NULL;
}
