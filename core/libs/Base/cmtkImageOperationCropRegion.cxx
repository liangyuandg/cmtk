/*
//
//  Copyright 2010 SRI International
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

#include <cmtkImageOperationCropRegion.h>

cmtk::UniformVolume::SmartPtr 
cmtk::ImageOperationCropRegion
::Apply( cmtk::UniformVolume::SmartPtr& volume )
{
    volume->SetCropRegion( this->m_Region, this->m_Region+3 );
    return cmtk::UniformVolume::SmartPtr( volume->GetCroppedVolume() );    
  }
  
void
cmtk::ImageOperationCropRegion
::New( const char* arg )
{
  int region[6];
  const bool okay = (6 == sscanf( arg, "%d,%d,%d,%d,%d,%d", &region[0], &region[1], &region[2], &region[3], &region[4], &region[5] ) );
  if ( ! okay )
    {
    throw "Expected six comma-separated integer values.";
    }
  
  ImageOperation::m_ImageOperationList.push_back( SmartPtr( new ImageOperationCropRegion( region ) ) );
}
