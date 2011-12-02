/*
//
//  Copyright 1997-2010 Torsten Rohlfing
//
//  Copyright 2004-2011 SRI International
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

#include <System/cmtkCommandLine.h>
#include <System/cmtkExitException.h>
#include <System/cmtkConsole.h>

#include <Base/cmtkUniformVolume.h>
#include <Base/cmtkVector3D.h>
#include <Base/cmtkMetaInformationObject.h>

#include <IO/cmtkFileFormat.h>
#include <IO/cmtkVolumeIO.h>

#include <stdio.h>

const char* ReadOrientation = NULL;

bool MachineReadable = false;

int
doMain( int argc, const char *argv[] )
{
  try
    {
    cmtk::CommandLine cl;
    cl.SetProgramInfo( cmtk::CommandLine::PRG_TITLE, "Describe image file formats and parameters" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_DESCR, "This tool prints a detailed description of the input image(s)" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_SYNTX, "describe [options] imageFile0 [imageFile1 ...]" );

    typedef cmtk::CommandLine::Key Key;
    cl.AddSwitch( Key( 'm', "machine-readable" ), &MachineReadable, true, "Print output in format that is easy to parse automatically." );
    cl.AddSwitch( Key( "read-ras" ), &ReadOrientation, "RAS", "Read image in RAS orientation" );

    cl.Parse( argc, const_cast<const char**>( argv ) );

    for ( const char* next = cl.GetNext(); next; next = cl.GetNextOptional() ) 
      {
      cmtk::UniformVolume::SmartPtr volume;
      if ( ReadOrientation )
	volume = cmtk::VolumeIO::ReadOriented( next, ReadOrientation );
      else
 	volume = cmtk::VolumeIO::Read( next );
      
      if ( ! volume ) 
	{
	fprintf( stderr, "Could not read volume data for %s.\n", next );
	continue;
	}
      
      const char* orientOriginal = volume->GetMetaInfo( cmtk::META_IMAGE_ORIENTATION_ORIGINAL ).c_str();
      const cmtk::TypedArray* dataArray = volume->GetData();
      
      if ( MachineReadable )
	{
	fprintf( stdout, "FNAME\t%s\n", next );            
	fprintf( stdout, "FORMAT\t%s\n", volume->GetMetaInfo( cmtk::META_FILEFORMAT_ORIGINAL ).c_str() );

	if ( volume->MetaKeyExists( cmtk::META_IMAGE_DESCRIPTION ) )
	  fprintf( stdout, "DESCRIP\t\"%s\"\n", volume->GetMetaInfo( cmtk::META_IMAGE_DESCRIPTION ).c_str() );
	
	fprintf( stdout, "XDIM\t%d\nYDIM\t%d\nZDIM\t%d\n", volume->GetDims()[cmtk::AXIS_X], volume->GetDims()[cmtk::AXIS_Y], volume->GetDims()[cmtk::AXIS_Z] );
	fprintf( stdout, "ORIENT\t%s\n", orientOriginal ? orientOriginal : "UNKNOWN" );
      
	fprintf( stdout, "GRID\tUniform\nXPIX\t%f\nYPIX\t%f\nZPIX\t%f\nXFOV\t%f\nYFOV\t%f\nZFOV\t%f\n",
		 volume->m_Delta[0], volume->m_Delta[1], volume->m_Delta[2],
		 volume->Size[0], volume->Size[1], volume->Size[2] );

	fprintf( stdout, "XORIGIN\t%f\nYORIGIN\t%f\nZORIGIN\t%f\n", volume->m_Offset[0], volume->m_Offset[1], volume->m_Offset[2] );
	if ( volume->MetaKeyExists(cmtk::META_SPACE_UNITS_STRING ) )
	  fprintf( stdout, "UNITS\t%s\n", volume->GetMetaInfo( cmtk::META_SPACE_UNITS_STRING ).c_str() );
	
	const cmtk::AffineXform::MatrixType a2p = volume->GetImageToPhysicalMatrix();
	fprintf( stdout, "I2PMAT0\t%f\t%f\t%f\t%f\nI2PMAT1\t%f\t%f\t%f\t%f\nI2PMAT2\t%f\t%f\t%f\t%f\nI2PMAT3\t%f\t%f\t%f\t%f\n", 
		 a2p[0][0], a2p[1][0], a2p[2][0], a2p[3][0], 
		 a2p[0][1], a2p[1][1], a2p[2][1], a2p[3][1], 
		 a2p[0][2], a2p[1][2], a2p[2][2], a2p[3][2],
		 a2p[0][3], a2p[1][3], a2p[2][3], a2p[3][3] );
	
	if ( dataArray ) 
	  {
	  fprintf( stdout, "DTYPE\t%s\n", cmtk::DataTypeName[ dataArray->GetType() ] );
	  if ( dataArray->GetDataSize() )
	    {
	    const cmtk::Types::DataItemRange range = dataArray->GetRange();
	    fprintf( stdout, "MINDATA\t%f\nMAXDATA\t%f\n", static_cast<float>( range.m_LowerBound ), static_cast<float>( range.m_UpperBound ) );
	    }
	  }
	}
      else
	{
	fprintf( stdout, "File: %s\n", next );            
	fprintf( stdout, "File format: %s\n", volume->GetMetaInfo( cmtk::META_FILEFORMAT_ORIGINAL ).c_str() );

	if ( volume->MetaKeyExists( cmtk::META_IMAGE_DESCRIPTION ) )
	  fprintf( stdout, "Description: \"%s\"\n", volume->GetMetaInfo( cmtk::META_IMAGE_DESCRIPTION ).c_str() );
	
	fprintf( stdout, "%d x %d x %d voxels\n", volume->GetDims()[cmtk::AXIS_X], volume->GetDims()[cmtk::AXIS_Y], volume->GetDims()[cmtk::AXIS_Z] );
	
	fprintf( stdout, "Original image orientation: %s\n", orientOriginal ? orientOriginal : "UNKNOWN" );
      
	const char* spaceUnits = "";
	if ( volume->MetaKeyExists(cmtk::META_SPACE_UNITS_STRING ) )
	  spaceUnits = volume->GetMetaInfo( cmtk::META_SPACE_UNITS_STRING ).c_str();
	
	fprintf( stdout, "Uniform volume\n%f x %f x %f [%s] voxel size\n%f x %f x %f [%s] volume size\n",
		 volume->m_Delta[0], volume->m_Delta[1], volume->m_Delta[2], spaceUnits,
		 volume->Size[0], volume->Size[1], volume->Size[2], spaceUnits );
	
	fprintf( stdout, "Volume origin (%f,%f,%f)\n", volume->m_Offset[0], volume->m_Offset[1], volume->m_Offset[2] );

	const cmtk::AffineXform::MatrixType a2p = volume->GetImageToPhysicalMatrix();
	fprintf( stdout, "\nImage-to-physical matrix:\n  %f %f %f %f \n %f %f %f %f \n %f %f %f %f \n %f %f %f %f \n\n", 
		 a2p[0][0], a2p[1][0], a2p[2][0], a2p[3][0], 
		 a2p[0][1], a2p[1][1], a2p[2][1], a2p[3][1], 
		 a2p[0][2], a2p[1][2], a2p[2][2], a2p[3][2],
		 a2p[0][3], a2p[1][3], a2p[2][3], a2p[3][3] );
      
	if ( dataArray ) 
	  {
	  cmtk::StdOut.printf( "Data type %s", cmtk::DataTypeName[ dataArray->GetType() ] );
	  if ( dataArray->GetDataSize() )
	    {
	    const cmtk::Types::DataItemRange range = dataArray->GetRange();
	    cmtk::StdOut.printf( ", range [%f .. %f]\n", static_cast<float>( range.m_LowerBound ), static_cast<float>( range.m_UpperBound ) );
	    }
	  else
	    {
	    cmtk::StdOut << "\n";
	    }
	  } 
	else
	  {
	  cmtk::StdOut << "Image does not contain valid data.\n";
	  }
	}
      }
    }
  catch ( const cmtk::CommandLine::Exception& e )
    {
    cmtk::StdErr << e << "\n";
    throw cmtk::ExitException( 1 );
    }

  return 0;
}

#include "cmtkSafeMain"

