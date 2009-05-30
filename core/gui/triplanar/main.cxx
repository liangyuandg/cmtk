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

#include <cmtkQtTriplanarViewer.h>
#include <cmtkQtProgress.h>

#include <qapplication.h>
#include <QPlastiqueStyle>

#include <string.h>

#include <list>
#include <string>

#include <cmtkCommandLine.h>
#include <cmtkStudy.h>

int main( int argc, char* argv[] )
{
  QApplication app( argc, argv );
  app.setStyle( new QPlastiqueStyle );

  cmtk::QtTriplanarViewer* viewer = new cmtk::QtTriplanarViewer;
  if ( viewer ) 
    {
    cmtk::QtProgress progressInstance( viewer );
    progressInstance.SetProgressWidgetMode( cmtk::QtProgress::PROGRESS_DIALOG );
    cmtk::Progress::SetProgressInstance( &progressInstance );
    
    if ( (argc > 1) && !strcmp( argv[1], "--exec" ) )
      {
      viewer->hide();
      viewer->ExecuteBatchMode( argc-2, argv+2 );
      }
    else
      {
      for ( int i = 1; i < argc; ++i ) 
	{
	viewer->slotAddStudy( argv[i] );
	}
      viewer->show();
      app.setMainWidget( viewer );
      
      return app.exec();
      }
    }
  
  return 0;
}
