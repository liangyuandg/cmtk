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

#include <cmtkCommandLine.h>
#include <cmtkConsole.h>

#include <cmtkClassStream.h>
#include <cmtkClassStreamAffineXform.h>

#include <cmtkUniformVolume.h>
#include <cmtkVolumeIO.h>

bool Verbose = false;

const char* InputFilePath = NULL;
const char* OutputFilePath = NULL;
const char* OutputXformPath = NULL;

int Axis = cmtk::AXIS_Z;
int Factor = 2;
bool Padded = false;

int
main( const int argc, const char* argv[] )
{
  try
    {
    cmtk::CommandLine cl( argc, argv );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_TITLE, "Split images" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_DESCR, "Split volume image into sub-images, i.e., to separate interleaved images into passes" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_SYNTX, "[options] inImage outImage" );

    typedef cmtk::CommandLine::Key Key;
    cl.AddSwitch( Key( 'v', "verbose" ), &Verbose, true, "Verbose operation" );

    cl.AddSwitch( Key( 'a', "axial" ), &Axis, (int)cmtk::AXIS_Z, "Interleaved axial images [default]" );
    cl.AddSwitch( Key( 's', "sagittal" ), &Axis, (int)cmtk::AXIS_X, "Interleaved sagittal images" );
    cl.AddSwitch( Key( 'c', "coronal" ), &Axis, (int)cmtk::AXIS_Y, "Interleaved coronal images" );

    cl.AddSwitch( Key( 'x' ), &Axis, (int)cmtk::AXIS_X, "Interleaved along x axis" );
    cl.AddSwitch( Key( 'y' ), &Axis, (int)cmtk::AXIS_Y, "Interleaved along y axis" );
    cl.AddSwitch( Key( 'z' ), &Axis, (int)cmtk::AXIS_Z, "Interleaved along z axis [default]" );

    cl.AddOption( Key( 'f', "factor" ), &Factor, "Interleave factor [default: 2]" );
    cl.AddSwitch( Key( 'p', "padded" ), &Padded, true, "Padded output, i.e., fill in removed slices [default: off]" );

    cl.AddOption( Key( "output-xform-path" ), &OutputXformPath, "Optional path template (fprintf-style) for output affine transformation that maps input image coordinates to each output image." );

    cl.Parse();

    InputFilePath = cl.GetNext();
    OutputFilePath = cl.GetNext();
    }
  catch ( cmtk::CommandLine::Exception e )
    {
    cmtk::StdErr << e << "\n";
    return 1;
    }

  cmtk::UniformVolume::SmartPtr volume( cmtk::VolumeIO::ReadOriented( InputFilePath, Verbose ) );
  if ( ! volume || ! volume->GetData() )
    {
    cmtk::StdErr << "ERROR: Could not read image " << InputFilePath << "\n";
    return 1;
    }

  for ( int i = 0; i < Factor; ++i )
    {
    cmtk::UniformVolume::SmartPtr subvolume( Padded ? volume->GetInterleavedPaddedSubVolume( Axis, Factor, i ) : volume->GetInterleavedSubVolume( Axis, Factor, i ) );

    char path[PATH_MAX];
    if ( snprintf( path, PATH_MAX, OutputFilePath, i ) > PATH_MAX )
      {
      cmtk::StdErr << "ERROR: output path exceeds maximum path length\n";
      }
    else
      {
      cmtk::VolumeIO::Write( subvolume, path, Verbose );
      }

    if ( OutputXformPath )
      {
      if ( snprintf( path, PATH_MAX, OutputXformPath, i ) > PATH_MAX )
	{
	cmtk::StdErr << "ERROR: output path exceeds maximum path length\n";
	}
      cmtk::AffineXform xform;
      cmtk::Types::Coordinate xlate[3] = {0,0,0};
      xlate[Axis] = -i * volume->Delta[Axis];
      xform.SetXlate( xlate );

      cmtk::ClassStream stream( path, cmtk::ClassStream::WRITE );
      if ( stream.IsValid() )
	{
	stream << xform;
	stream.Close();
	}
      }
    }
  
  return 0;
}

