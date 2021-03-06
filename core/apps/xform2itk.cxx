/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2011, 2013 SRI International
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

#include <System/cmtkConsole.h>
#include <System/cmtkCommandLine.h>
#include <System/cmtkMountPoints.h>

#include <Base/cmtkMetaInformationObject.h>
#include <Base/cmtkAffineXform.h>
#include <Base/cmtkAnatomicalOrientationBase.h>
#include <Base/cmtkTransformChangeToSpaceAffine.h>

#include <IO/cmtkVolumeIO.h>
#include <IO/cmtkXformIO.h>
#include <IO/cmtkAffineXformITKIO.h>

int 
doMain( const int argc, const char* argv[] )
{
  std::string inputPath;
  std::string outputPath;

  bool invertInputXform = false;

  std::string fixedImagePath;
  std::string movingImagePath;

  std::string fixedImageSpace = cmtk::AnatomicalOrientationBase::SPACE_ITK;
  std::string movingImageSpace = cmtk::AnatomicalOrientationBase::SPACE_ITK;
  
  try 
    {
    cmtk::CommandLine cl;
    cl.SetProgramInfo( cmtk::CommandLine::PRG_TITLE, "Convert affine transformations to ITK format." );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_DESCR, "This tool converts coordinate transformations from CMTK format to ITK format and, in the process, also correct for differences in image coordinate conventions" );

    typedef cmtk::CommandLine::Key Key;
    cl.BeginGroup( "Input", "Input parameters" )->SetProperties( cmtk::CommandLine::PROPS_NOXML );
    cl.AddOption( Key( "fixed-space" ), &fixedImageSpace, "Change fixed image coordinate space (e.g., 'RAS', 'LPS', ...). This defaults to ITK standard space." );
    cl.AddOption( Key( "moving-space" ), &movingImageSpace, "Change moving image coordinate space (e.g., 'RAS', 'LPS', ...). This defaults to ITK standard space." );
    cl.AddSwitch( Key( "invert-input" ), &invertInputXform, true, "Invert input transformation before conversion" );
    cl.EndGroup();
    
    cl.BeginGroup( "Output", "Output parameters" )->SetProperties( cmtk::CommandLine::PROPS_NOXML );
    cl.AddOption( Key( "fixed-image" ), &fixedImagePath, "Override transformation's fixed/reference image (if any) with this one." );
    cl.AddOption( Key( "moving-image" ), &movingImagePath, "Override transformation's moving/floating image (if any) with this one." );
    cl.EndGroup();
    
    cl.AddParameter( &inputPath, "InputPath", "CMTK input transformation path" )->SetProperties( cmtk::CommandLine::PROPS_XFORM );
    cl.AddParameter( &outputPath, "OutputPath", "ITK output transformation path" )->SetProperties( cmtk::CommandLine::PROPS_XFORM | cmtk::CommandLine::PROPS_OUTPUT );
    
    cl.Parse( argc, argv );

    }
  catch ( const cmtk::CommandLine::Exception& e ) 
    {
    cmtk::StdErr << e << "\n";
    throw cmtk::ExitException( 1 );
    }

  cmtk::AffineXform::SmartConstPtr xform = cmtk::AffineXform::SmartConstPtr::DynamicCastFrom( cmtk::XformIO::Read( inputPath ) );
  if ( !xform )
    {
    cmtk::StdErr << "ERROR: could not read transformation from '" << inputPath << "'\n";
    throw cmtk::ExitException( 1 );
    }

  if ( invertInputXform )
    {
    try
      {
      xform = xform->GetInverse();
      }
    catch ( const cmtk::AffineXform::MatrixType::SingularMatrixException& )
      {
      cmtk::StdErr << "ERROR: affine transformation with singular matrix cannot be inverted\n";
      throw cmtk::ExitException( 1 );
      }
    }

  if ( fixedImagePath.empty() )
    {
    if ( !xform->MetaKeyExists( cmtk::META_XFORM_FIXED_IMAGE_PATH ) )
      {
      cmtk::StdErr << "ERROR: could not deduce fixed image path from transformation; must be explicitly provided on the command line instead\n";
      throw cmtk::ExitException( 1 );
      }
    fixedImagePath = xform->GetMetaInfo( cmtk::META_XFORM_FIXED_IMAGE_PATH );
    }

  if ( movingImagePath.empty() )
    {
    if ( !xform->MetaKeyExists( cmtk::META_XFORM_MOVING_IMAGE_PATH ) )
      {
      cmtk::StdErr << "ERROR: could not deduce moving image path from transformation; must be explicitly provided on the command line instead\n";
      throw cmtk::ExitException( 1 );
      }
    movingImagePath = xform->GetMetaInfo( cmtk::META_XFORM_MOVING_IMAGE_PATH );
    }

  cmtk::UniformVolume::SmartPtr fixedImage = cmtk::VolumeIO::ReadGridOriented( cmtk::MountPoints::Translate( fixedImagePath ) );
  if ( ! fixedImage )
    {
    cmtk::StdErr << "ERROR: could not read fixed image '" << fixedImagePath << "'\n";
    throw cmtk::ExitException( 1 );
    }

  if ( !fixedImageSpace.empty() )
    {
    fixedImage->SetMetaInfo( cmtk::META_SPACE_ORIGINAL, fixedImageSpace );
    }

  cmtk::UniformVolume::SmartPtr movingImage = cmtk::VolumeIO::ReadGridOriented( cmtk::MountPoints::Translate( movingImagePath ) );
  if ( ! movingImage )
    {
    cmtk::StdErr << "ERROR: could not read moving image '" << movingImagePath << "'\n";
    throw cmtk::ExitException( 1 );
    }

  if ( !movingImageSpace.empty() )
    {
    movingImage->SetMetaInfo( cmtk::META_SPACE_ORIGINAL, movingImageSpace );
    }

  cmtk::TransformChangeToSpaceAffine toNative( *(xform), *(fixedImage), *(movingImage) );
  cmtk::AffineXformITKIO::Write( outputPath, toNative.GetTransformation() );
  
  return 0;
}

#include "cmtkSafeMain"
