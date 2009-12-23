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
#include <cmtkTimers.h>

#include <cmtkUniformVolume.h>
#include <cmtkVolumeIO.h>

#include <cmtkStudyList.h>
#include <cmtkClassStreamStudyList.h>

#include <math.h>
#include <cmtkMathFunctionWrappers.h>

#include <stdlib.h>

#include <iostream>
#include <fstream>
#include <set>
#include <map>

#ifdef HAVE_IEEEFP_H
#  include <ieeefp.h>
#endif

#ifdef CMTK_SINGLE_COMMAND_BINARY
namespace cmtk
{
namespace apps
{
namespace convert
{
#endif

bool Verbose = false;

/// Image operation base class.
class ImageOperation
{
public:
  /// This class.
  typedef ImageOperation Self;

  /// Smart pointer.
  typedef cmtk::SmartPointer<Self> SmartPtr;
  
  /// Apply this operation to an image in place.
  virtual cmtk::UniformVolume& Apply( cmtk::UniformVolume& volume )
  {
    return volume;
  }
};

/// List of image operations.
std::list<ImageOperation::SmartPtr> ImageOperationList;

/// Image operation: flip.
class ImageOperationConvertType
/// Inherit from image operation base class.
  : public ImageOperation
{
public:
  /// Constructor:
  ImageOperationConvertType( const cmtk::ScalarDataType newType ) : m_NewType( newType ) {}
  
  /// Apply this operation to an image in place.
  virtual cmtk::UniformVolume& Apply( cmtk::UniformVolume& volume )
  {    
    switch ( this->m_NewType ) 
      {
      case cmtk::TYPE_CHAR:
      case cmtk::TYPE_BYTE:
      case cmtk::TYPE_SHORT:
      case cmtk::TYPE_USHORT:
      case cmtk::TYPE_INT:
      case cmtk::TYPE_FLOAT:
      case cmtk::TYPE_DOUBLE:
	if ( this->m_NewType != volume.GetData()->GetType() ) 
	  {
	  if ( Verbose )
	    cmtk::StdErr << "Converting to new data type.\n";
	  
	  volume.SetData( cmtk::TypedArray::SmartPtr( volume.GetData()->Convert( this->m_NewType ) ) );
	  }
	break;
      default:
	break;
      }
    return volume;
  }

  /// Create object to convert to "char" data.
  static const char* NewChar()
  {
    ImageOperationList.push_back( SmartPtr( new ImageOperationConvertType( cmtk::TYPE_CHAR ) ) );
    return NULL;
  }

  /// Create object to convert to "byte" data.
  static const char* NewByte()
  {
    ImageOperationList.push_back( SmartPtr( new ImageOperationConvertType( cmtk::TYPE_BYTE ) ) );
    return NULL;
  }

  /// Create object to convert to "short" data.
  static const char* NewShort()
  {
    ImageOperationList.push_back( SmartPtr( new ImageOperationConvertType( cmtk::TYPE_SHORT ) ) );
    return NULL;
  }

  /// Create object to convert to "unsigned short" data.
  static const char* NewUShort()
  {
    ImageOperationList.push_back( SmartPtr( new ImageOperationConvertType( cmtk::TYPE_USHORT ) ) );
    return NULL;
  }

  /// Create object to convert to "int" data.
  static const char* NewInt()
  {
    ImageOperationList.push_back( SmartPtr( new ImageOperationConvertType( cmtk::TYPE_INT ) ) );
    return NULL;
  }

  /// Create object to convert to "float" data.
  static const char* NewFloat()
  {
    ImageOperationList.push_back( SmartPtr( new ImageOperationConvertType( cmtk::TYPE_FLOAT ) ) );
    return NULL;
  }

  /// Create object to convert to "double" data.
  static const char* NewDouble()
  {
    ImageOperationList.push_back( SmartPtr( new ImageOperationConvertType( cmtk::TYPE_DOUBLE ) ) );
    return NULL;
  }
  
private:
  /// New data type.
  cmtk::ScalarDataType m_NewType;
};

/// Image operation: flip.
class ImageOperationFlip
  /// Inherit from image operation base class.
  : public ImageOperation
{
public:
  /// Constructor:
  ImageOperationFlip( const int normalAxis ) : m_NormalAxis( normalAxis ) {}

  /// Apply this operation to an image in place.
  virtual cmtk::UniformVolume& Apply( cmtk::UniformVolume& volume )
  {
    volume.ApplyMirrorPlane( this->m_NormalAxis );
    return volume;
  }

  /// Create x flip object.
  static const char* NewX()
  {
    ImageOperationList.push_back( SmartPtr( new ImageOperationFlip( cmtk::AXIS_X ) ) );
    return NULL;
  }

  /// Create y flip object.
  static const char* NewY()
  {
    ImageOperationList.push_back( SmartPtr( new ImageOperationFlip( cmtk::AXIS_Y ) ) );
    return NULL;
  }

  /// Create y flip object.
  static const char* NewZ()
  {
    ImageOperationList.push_back( SmartPtr( new ImageOperationFlip( cmtk::AXIS_Z ) ) );
    return NULL;
  }

private:
  /// The normal axis of the flip.
  int m_NormalAxis;
};

/// Apply mask image.
class ImageOperationApplyMask
/// Inherit generic image operation.
  : public ImageOperation
{
public:
  /// Constructor.
  ImageOperationApplyMask( const cmtk::UniformVolume::SmartPtr& maskVolume ) : m_MaskVolume( maskVolume ) {}

  /// Apply this operation to an image in place.
  virtual cmtk::UniformVolume& Apply( cmtk::UniformVolume& volume )
  {
    const std::string maskOrientation = this->m_MaskVolume->m_MetaInformation[CMTK_META_IMAGE_ORIENTATION];
    const std::string workingOrientation = volume.m_MetaInformation[CMTK_META_IMAGE_ORIENTATION];
    if ( maskOrientation != workingOrientation )
      {
      if ( Verbose )
	{
	cmtk::StdErr << "INFO: reorienting mask from '" << maskOrientation << "' to '" << workingOrientation << "' to match working image.\n";
	}
      this->m_MaskVolume = cmtk::UniformVolume::SmartPtr( this->m_MaskVolume->GetReoriented( workingOrientation.c_str() ) );
      }
    
    for ( int dim = 0; dim < 3; ++dim )
      {
      if ( this->m_MaskVolume->m_Dims[dim] != volume.m_Dims[dim] )
	{
	cmtk::StdErr << "ERROR: mask volume dimensions do not match working volume dimensions.\n";
	exit( 1 );
	}
      }
    
    const cmtk::TypedArray* maskData = this->m_MaskVolume->GetData();
    cmtk::TypedArray::SmartPtr& volumeData = volume.GetData();

    const size_t nPixels = volume.GetNumberOfPixels();
    for ( size_t i = 0; i < nPixels; ++i )
      if ( maskData->IsPaddingOrZeroAt( i ) ) 
	volumeData->SetPaddingAt( i );

    return volume;
  }

  /// Create new mask operation.
  static const char* New( const char* maskFileName )
  {
    ImageOperationList.push_back( SmartPtr( new ImageOperationApplyMask( ReadMaskFile( maskFileName ) ) ) );
    return NULL;
  }

  /// Create new inverse mask operation.
  static const char* NewInverse( const char* maskFileName )
  {
    ImageOperationList.push_back( SmartPtr( new ImageOperationApplyMask( ReadMaskFile( maskFileName, true /*inverse*/ ) ) ) );
    return NULL;
  }

private:
  /// The mask volume.
  cmtk::UniformVolume::SmartPtr m_MaskVolume;

  /// Read the actual mask file.
  static cmtk::UniformVolume::SmartPtr ReadMaskFile( const char* maskFileName, const bool inverse = false )
  {
    cmtk::UniformVolume::SmartPtr maskVolume( cmtk::VolumeIO::ReadOriented( maskFileName, Verbose ) );
    if ( !maskVolume || !maskVolume->GetData() ) 
      {
      cmtk::StdErr << "ERROR: could not read mask from file " << maskFileName << "\nProgram will terminate now, just to be safe.\n";
      exit( 1 );
      }
    
    // binarize mask to 1/0, convert to char, and also consider "inverse" flag in the process.
    cmtk::TypedArray::SmartPtr& maskData = maskVolume->GetData();
    const size_t nPixels = maskData->GetDataSize();
    for ( size_t n = 0; n < nPixels; ++n )
      {
      if ( maskData->IsPaddingOrZeroAt( n ) != inverse ) 
	maskData->Set( n, 1 );
      else
	maskData->Set( n, 0 );
      }
    maskVolume->SetData( cmtk::TypedArray::SmartPtr( maskData->Convert( cmtk::TYPE_BYTE ) ) );
    
    return maskVolume;
  }
};

int
main( int argc, char* argv[] )
{
  cmtk::Types::DataItem paddingDataValue = 0.0;
  bool paddingDataFlag = false;

  const char* imagePathIn = NULL;
  const char* imagePathOut = NULL;

  try 
    {
    cmtk::CommandLine cl( argc, argv );  
    cl.SetProgramInfo( cmtk::CommandLine::PRG_TITLE, "Convert between image formats" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_SYNTX, "[options] infile outfile" );

    typedef cmtk::CommandLine::Key Key;
    cl.AddSwitch( Key( 'v', "verbose" ), &Verbose, true, "Verbose mode" );

    cl.AddOption( Key( 'N', "set-padding" ), &paddingDataValue, "Set Padding data for input image.", &paddingDataFlag );
    
    cl.AddCallback( Key( 'c', "char" ), &ImageOperationConvertType::NewChar, "8 bits, signed integer" );
    cl.AddCallback( Key( 'b', "byte" ), &ImageOperationConvertType::NewByte, "8 bits, unsigned integer" );
    cl.AddCallback( Key( 's', "short" ), &ImageOperationConvertType::NewShort, "16 bits, signed integer" );
    cl.AddCallback( Key( 'u', "ushort" ), &ImageOperationConvertType::NewUShort, "16 bits, unsigned integer" );
    cl.AddCallback( Key( 'i', "int" ), &ImageOperationConvertType::NewInt, "32 bits signed integer" );
    cl.AddCallback( Key( 'f', "float" ), &ImageOperationConvertType::NewFloat, "32 bits floating point" );
    cl.AddCallback( Key( 'd', "double" ), &ImageOperationConvertType::NewDouble, "64 bits floating point\n" );
    
    cl.AddCallback( Key( "flip-x" ), &ImageOperationFlip::NewX, "Flip (mirror) along x-direction" );
    cl.AddCallback( Key( "flip-y" ), &ImageOperationFlip::NewY, "Flip (mirror) along y-direction" );
    cl.AddCallback( Key( "flip-z" ), &ImageOperationFlip::NewZ, "Flip (mirror) along z-direction" );

    cl.AddCallback( Key( 'M', "mask" ), &ImageOperationApplyMask::New, "Binary mask file name: eliminate all image pixels where mask is 0." );
    cl.AddCallback( Key( "mask-inverse" ), &ImageOperationApplyMask::NewInverse, "Inverse binary mask file name eliminate all image pixels where mask is NOT 0." );

    if ( ! cl.Parse() ) return 1;
    
    cl.AddParameter( &imagePathIn, "InputImage", "Input image path" )->SetProperties( cmtk::CommandLine::PROPS_IMAGE );
    cl.AddParameter( &imagePathOut, "OutputImage", "Output image path" )->SetProperties( cmtk::CommandLine::PROPS_IMAGE | cmtk::CommandLine::PROPS_OUTPUT );
    }
  catch ( cmtk::CommandLine::Exception ex ) 
    {
    cmtk::StdErr << ex << "\n";
    return false;
    }
  
  cmtk::UniformVolume::SmartPtr volume( cmtk::VolumeIO::ReadOriented( imagePathIn, Verbose ) );
  if ( ! volume ) 
    {
    cmtk::StdErr << "ERROR: could not read image " << imagePathIn << "\n";
    exit( 1 );
    }
  else
    {
    cmtk::TypedArray::SmartPtr volumeData = volume->GetData();
    if ( ! volumeData ) 
      {
      cmtk::StdErr << "ERROR: image seems to contain no data.\n";
      exit( 1 );
      }
    }
  
  if ( paddingDataFlag ) 
    {
    volume->GetData()->SetPaddingValue( paddingDataValue );
    }
  
  for ( std::list<ImageOperation::SmartPtr>::iterator opIt = ImageOperationList.begin(); opIt != ImageOperationList.end(); ++opIt )
    {
    (*opIt)->Apply( *volume );
    }

  if ( Verbose )
    {
    cmtk::StdErr << "Writing to file " << imagePathOut << "\n";
    }
  
  cmtk::VolumeIO::Write( volume, imagePathOut, Verbose );
  return 0;
}
#ifdef CMTK_SINGLE_COMMAND_BINARY
} // namespace convert
} // namespace apps
} // namespace cmtk
#endif
