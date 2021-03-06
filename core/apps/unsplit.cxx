/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2013 SRI International
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
#include <System/cmtkConsole.h>
#include <System/cmtkDebugOutput.h>

#include <Base/cmtkUniformVolume.h>
#include <IO/cmtkVolumeIO.h>

#include <vector>
#include <list>

int
doMain( const int argc, const char* argv[] )
{
  std::list<const char*> inputFilePaths;
  const char* outputFilePath = NULL;

  int axis = 2;
  cmtk::Types::Coordinate forceSpacing = 0;

  try
    {
    cmtk::CommandLine cl;
    cl.SetProgramInfo( cmtk::CommandLine::PRG_TITLE, "Unsplit images" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_DESCR, "Join separate image stacks into a single interleaved image volume" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_SYNTX, "unsplit [options] inImage0 inImage1 ..." );

    typedef cmtk::CommandLine::Key Key;
    cl.BeginGroup( "Stacking", "Stacking Options" );
    cmtk::CommandLine::EnumGroup<int>::SmartPtr
      interleaveGroup = cl.AddEnum( "stacking", &axis, "Define slice axis for stacking." );
    interleaveGroup->AddSwitch( Key( 'a', "axial" ), (int)cmtk::AXIS_Z, "Interleaved axial images" );
    interleaveGroup->AddSwitch( Key( 's', "sagittal" ),(int)cmtk::AXIS_X, "Interleaved sagittal images" );
    interleaveGroup->AddSwitch( Key( 'c', "coronal" ), (int)cmtk::AXIS_Y, "Interleaved coronal images" );
    interleaveGroup->AddSwitch( Key( 'x', "interleave-x" ), (int)cmtk::AXIS_X, "Interleaved along x axis" );
    interleaveGroup->AddSwitch( Key( 'y', "interleave-y" ), (int)cmtk::AXIS_Y, "Interleaved along y axis" );
    interleaveGroup->AddSwitch( Key( 'z', "interleave-z" ), (int)cmtk::AXIS_Z, "Interleaved along z axis" );

    cl.AddOption( Key( "spacing" ), &forceSpacing, "If non-zero, force slice spacing in the output image to given value. This is required when stacking single-slice images." );
    cl.EndGroup();

    cl.AddOption( Key( 'o', "output" ), &outputFilePath, "Path for output image." );

    cl.Parse( argc, argv );

    const char *next = cl.GetNext();
    while ( next )
      {
      inputFilePaths.push_back( next );
      next = cl.GetNextOptional();
      }
    }
  catch ( const cmtk::CommandLine::Exception& e )
    {
    cmtk::StdErr << e << "\n";
    return 1;
    }

  cmtk::UniformVolume::IndexType stackDims;
  cmtk::Types::Coordinate stackDelta[3] = { 1,1,1 };
  std::vector<cmtk::UniformVolume::SmartPtr> volumes;
  for ( std::list<const char*>::const_iterator it = inputFilePaths.begin(); it != inputFilePaths.end(); ++it )
    {
    cmtk::UniformVolume::SmartPtr volume( cmtk::VolumeIO::ReadOriented( *it ) );
    if ( ! volume || ! volume->GetData() )
      {
      cmtk::StdErr << "ERROR: Could not read image " << *it << "\n";
      return 1;
      }

    if ( !volumes.empty() )
      {
      // check image dimensions
      for ( int dim = 0; dim < 3; ++dim )
	{
	if ( dim == axis )
	  {
	  if ( (volume->m_Dims[dim] != volumes[0]->m_Dims[dim]) && (volume->m_Dims[dim]+1 != volumes[0]->m_Dims[dim]) )
	    {
	    cmtk::StdErr << "ERROR: interleaving dimension of image " << *it << " must be same as, or one smaller than first image's\n";
	    throw cmtk::ExitException( 1 );
	    }
	  stackDims[dim] += volume->m_Dims[dim];
	  }
	else
	  {
	  if ( volume->m_Dims[dim] != volumes[0]->m_Dims[dim] )
	    {
	    cmtk::StdErr << "ERROR: in-plane dimensions of image " << *it << " do not match first image's\n";
	    throw cmtk::ExitException( 1 );
	    }
	  }
	}
      }
    else
      // ! volumes.size() -> first image
      {
      // set dims and deltas; will modify later
      for ( int dim = 0; dim < 3; ++dim )
	{
	stackDims[dim] = volume->m_Dims[dim];
	stackDelta[dim] = volume->m_Delta[dim];
	}
      }
    volumes.push_back( volume );
    }

  if ( forceSpacing )
    stackDelta[axis] = forceSpacing;
  else
    stackDelta[axis] = volumes[0]->m_Delta[axis] / volumes.size();
  
  cmtk::DebugOutput( 1 ) << "Stacked image will have dimensions " << stackDims[0] << "x" << stackDims[1] << "x" << stackDims[2] << "\n"
			 << "Stacked image will have pixel size " << stackDelta[0] << "x" << stackDelta[1] << "x" << stackDelta[2] << "\n";

  cmtk::UniformVolume::SmartPtr stacked( new cmtk::UniformVolume( stackDims, stackDelta[0], stackDelta[1], stackDelta[2] ) );
  stacked->CreateDataArray( volumes[0]->GetData()->GetType() );

  int toSlice = 0;
  for ( int fromSlice = 0; fromSlice < volumes[0]->m_Dims[axis]; ++fromSlice )
    {
    for ( size_t fromVolume = 0; fromVolume < volumes.size(); ++fromVolume, ++toSlice )
      {
      cmtk::ScalarImage::SmartPtr slice( volumes[fromVolume]->GetOrthoSlice( axis, fromSlice ) );
      stacked->SetOrthoSlice( axis, toSlice, slice );
      }
    }

  // get origin and orientation from first input image
  cmtk::AffineXform::MatrixType xformMatrix = volumes[0]->GetImageToPhysicalMatrix();
  // and copy to output
  stacked->m_IndexToPhysicalMatrix *= xformMatrix;
  for ( std::map<int,cmtk::AffineXform::MatrixType>::iterator it = stacked->m_AlternativeIndexToPhysicalMatrices.begin(); it != stacked->m_AlternativeIndexToPhysicalMatrices.end(); ++it )
    {
    it->second *= xformMatrix;
    }
  stacked->CopyMetaInfo( *(volumes[0]) );
  
  if ( outputFilePath )
    {
    cmtk::VolumeIO::Write( *stacked, outputFilePath );
    }

  return 0;
}

#include "cmtkSafeMain"
