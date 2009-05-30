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

#include <cmtkUniformVolume.h>

#include <cmtkVolumeIO.h>

#include <stdio.h>

bool Verbose = false;

int Dims[3] = { 256, 256, 256 };

const char* 
SetDims( const char* arg )
{
  sscanf( arg, "%d,%d,%d", &Dims[0], &Dims[1], &Dims[2] );
  return NULL;
}

float Delta[3] = { 1.0, 1.0, 1.0 };

const char* 
SetDeltas( const char* arg )
{
  sscanf( arg, "%f,%f,%f", &Delta[0], &Delta[1], &Delta[2] );
  return NULL;
}

cmtk::ScalarDataType DataType = cmtk::TYPE_USHORT;
cmtk::Types::DataItem Background = 0;

const char* InputFileName = NULL;
const char* OutputFileName = "phantom.hdr";

int
main( const int argc, const char* argv[] )
{
  try 
    {
    cmtk::CommandLine cl( argc, argv );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_TITLE, "Generate phantom image" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_DESCR, "Generate 3D digital phantom images using a selection of drawing commands" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_SYNTX, "[options] command0 [command1 ...]\n"
		       "\t sphere x,y,z radius value\n"
		       "\t box x0,y0,z0 x1,y1,z1 value\n");

    typedef cmtk::CommandLine::Key Key;
    cl.AddSwitch( Key( 'v', "verbose" ), &Verbose, true, "Verbose mode" );

    cl.AddCallback( Key( 'D', "dims" ), SetDims, "Set dimensions in voxels" );
    cl.AddCallback( Key( 'V', "voxel" ), SetDeltas, "Set voxel size in [mm]" );

    cl.AddSwitch( Key( 'c', "char" ), &DataType, cmtk::TYPE_CHAR, "8 bits, signed" );
    cl.AddSwitch( Key( 'b', "byte" ), &DataType, cmtk::TYPE_BYTE, "8 bits, unsigned" );
    cl.AddSwitch( Key( 's', "short" ), &DataType, cmtk::TYPE_SHORT, "16 bits, signed" );
    cl.AddSwitch( Key( 'u', "ushort" ), &DataType, cmtk::TYPE_USHORT, "16 bits, unsigned" );
    cl.AddSwitch( Key( 'i', "int" ), &DataType, cmtk::TYPE_INT, "32 bits signed" );
    cl.AddSwitch( Key( 'f', "float" ), &DataType, cmtk::TYPE_FLOAT, "32 bits floating point" );
    cl.AddSwitch( Key( 'd', "double" ), &DataType, cmtk::TYPE_DOUBLE, "64 bits floating point\n" );

    cl.AddOption( Key( 'B', "bg" ), &Background, "Background value" );

    cl.AddOption( Key( 'I', "import" ), &InputFileName, "Import image" );
    cl.AddOption( Key( 'o', "outfile" ), &OutputFileName, "File name for output image" );
    
    cl.Parse();

    cmtk::UniformVolume::SmartPtr volume;
    if ( InputFileName )
      {
      volume = cmtk::UniformVolume::SmartPtr( cmtk::VolumeIO::ReadOriented( InputFileName, Verbose ) );
      }
    else
      {
      const cmtk::Types::Coordinate size[3] = { Delta[0] * (Dims[0]-1),  Delta[1] * (Dims[1]-1),  Delta[2] * (Dims[2]-1) };
      volume = cmtk::UniformVolume::SmartPtr( new cmtk::UniformVolume( Dims, size ) );
      volume->m_MetaInformation[CMTK_META_SPACE] = volume->m_MetaInformation[CMTK_META_SPACE_ORIGINAL] = cmtk::AnatomicalOrientation::ORIENTATION_STANDARD;
      cmtk::TypedArray::SmartPtr data( cmtk::TypedArray::Create( DataType, volume->GetNumberOfPixels() ) );
      volume->SetData( data );
      data->Fill( Background );
      }
    
    const char* nextCmd = cl.GetNextOptional();
    while (  nextCmd )
      {
      if ( ! strcmp( nextCmd, "sphere" ) )
	{
	const char* center = cl.GetNextOptional();
	const char* radius = cl.GetNextOptional();
	const char* value = cl.GetNextOptional();

	float cc[3];
	if ( sscanf( center, "%f,%f,%f", &cc[0], &cc[1], &cc[2] ) != 3 )
	  {
	  cmtk::StdErr << "Parameter 'center' of 'sphere' command must be x,y,z\n";
	  return 1;
	  }
	cmtk::Vector3D c( cc );
	volume->DrawSphere( c, atof( radius ), atof( value ) );
	}

      if ( ! strcmp( nextCmd, "box" ) )
	{
	const char* fromCorner = cl.GetNextOptional();
	const char* toCorner = cl.GetNextOptional();
	const char* value = cl.GetNextOptional();
	
	cmtk::IntROI3D box;
	if ( sscanf( fromCorner, "%d,%d,%d", &box.From[0], &box.From[1], &box.From[2] ) != 3 )
	  {
	  cmtk::StdErr << "Parameter 'corner0' of 'box' command must be three integers x,y,z\n";
	  return 1;
	  }
	if ( sscanf( toCorner, "%d,%d,%d", &box.To[0], &box.To[1], &box.To[2] ) != 3 )
	  {
	  cmtk::StdErr << "Parameter 'corner1' of 'box' command must be three integers x,y,z\n";
	  return 1;
	  }
	volume->DrawBox( box, atof( value ) );
	}
      nextCmd = cl.GetNextOptional();
      }
    
    cmtk::VolumeIO::Write( volume, OutputFileName, Verbose );
    }
  catch ( cmtk::CommandLine::Exception e ) 
    {
    cmtk::StdErr << e;
    return 1;
    }
  
  return 0;
}

