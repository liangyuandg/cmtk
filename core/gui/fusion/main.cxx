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

#include <cmtkconfig.h>

#include <cmtkQtSimpleFusionApp.h>

#include <cmtkPipelineObject.h>

#include <cmtkStudyList.h>
#include <cmtkClassStreamStudyList.h>

int
main( int argc, char *argv[] )
{
#ifdef DEBUG
  cmtk::PipelineObject::SetDebugMode( true );
#endif
  cmtk::QtSimpleFusionApp fusionApp( argc, argv );

  if ( argc > 1 )
    {
    cmtk::StdErr << "ERROR: this application does not accept any non-Qt command line options\n";
    exit( 2 );
    }

  return fusionApp.exec();
}
