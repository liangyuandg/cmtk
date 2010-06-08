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
//  $Revision: 1652 $
//
//  $LastChangedDate: 2010-05-14 14:45:52 -0700 (Fri, 14 May 2010) $
//
//  $LastChangedBy: torstenrohlfing $
//
*/

#include <cmtkconfig.h>

#include <cmtkCommandLine.h>
#include <cmtkConsole.h>

#include <cmtkXformIO.h>

#include <cmtkXform.h>
#include <cmtkVector3D.h>

#include <iostream>
#include <sstream>

int
main( const int argc, const char* argv[] )
{
  const char* XformPath = NULL;
  bool Verbose = false;
  bool Inverse = false;

  try
    {
    cmtk::CommandLine cl( argc, argv );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_TITLE, "Apply coordinate transformation to point coordinates in VTK file (standard input) and write equivalent file with transformed points to standard output." );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_SYNTX, "[options] transformation" );      

    typedef cmtk::CommandLine::Key Key;
    cl.AddSwitch( Key( "inverse" ), &Inverse, true, "Apply inverse of given transformation" );
    cl.AddSwitch( Key( 'v', "verbose" ), &Verbose, true, "Print each point to STDERR (as well as stdout)" );

    cl.Parse();

    XformPath = cl.GetNext();
    }
  catch ( cmtk::CommandLine::Exception ex )
    {
    cmtk::StdErr << ex << "\n";
    exit( 1 );
    }
  
  cmtk::Xform::SmartPtr xform = cmtk::XformIO::Read( XformPath, Verbose );
  if ( !xform )
    {
    exit( 1 );
    }
  
  // First, read everything up to and including the "POINTS" line and write everything to output unchanged
  std::string line;
  while ( !std::cin.eof() )
    {
    std::getline( std::cin, line );
    std::cout << line << std::endl;
    
    if ( ! line.compare( 0, 6, "POINTS" ) )
      break;
    }
  
  // If we're not at EOF, then "line" must be "POINTS <npoints> ..."
  if ( ! std::cin.eof() )
    {
    // Parse number of points out of line
    std::stringstream sstream( line.substr( 7 ) );
    size_t npoints;
    sstream >> npoints;
    
    // Repeat npoints times
    cmtk::Xform::SpaceVectorType xyz;
    for ( size_t n = 0; n<npoints; ++n )
      {
      // Read original point coordinates from file
      std::cin >> xyz[0] >> xyz[1] >> xyz[2];
      
      // Apply transformation (or inverse)
      if ( Inverse )
	xform->ApplyInverseInPlace( xyz );
      else
	xform->ApplyInPlace( xyz );
      
      // Write transformed point to output
      std::cout << xyz[0] << " " << xyz[1] << " " << xyz[2] << std::endl;
      }
    }

  // Everything else remains unchanged, so copy from input to output.
  while ( !std::cin.eof() )
    {
    std::getline( std::cin, line );
    std::cout << line << std::endl;
    }

  // if we got here, the program probably ran
  return 0;
}

