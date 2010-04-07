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

#include <cmtkVolumeFromFile.h>

#include <cmtkCompressedStream.h>

namespace
cmtk
{

UniformVolume*
VolumeFromFile::ReadVanderbilt( const char *path )
{
  FILE *fp = NULL;
  if ( ! (fp = fopen( path, "r" )) ) return NULL;
  
  int dims[3] = { 1, 1, 1 };
  double calib[3] = { 0, 0, 0 };

  char orientation[] = "RAS";

  // parse header file for image dimensios etc.
  char line[96], key[32], value[64];
  while ( !feof( fp ) ) 
    {
    fgets( line, 96, fp );
    if ( 2 == sscanf( line, "%32[a-zA-Z ]:= %64[0-9.: ]", key, value ) ) 
      {
      if ( ! strcmp( key, "Columns " ) ) 
	{
	dims[0] = atoi( value );
	} 
      else if ( ! strcmp( key, "Rows " ) ) 
	{
	dims[1] = atoi( value );
	} 
      else if ( ! strcmp( key, "Slices " ) ) 
	{
	dims[2] = atoi( value );
	} 
      else if ( ! strcmp( key, "Pixel size " ) ) 
	{
	sscanf( value, "%lf : %lf", calib, calib+1 );
	} 
      else if ( ! strcmp( key, "Slice thickness " ) ) 
	{
	calib[2] = static_cast<Types::Coordinate>( atof( value ) );
	}
      }
    else
      {
      char axes[3];
      if ( 3 == sscanf( line, "Patient orientation := %c : %c : %c", &axes[0], &axes[1], &axes[2] ) )
	{
	const char *const translation = "PbcdeSgIijkRmnoAqLstuvwxyz";
	for ( int i = 0; i < 3; ++i )
	  {
	  orientation[i] = translation[axes[i]-'A'];
	  }
	}
      }
    }
  fclose( fp );
  
  // convert pixel dimensions to volume dimensions.
  Types::Coordinate size[3];
  for ( int i = 0; i < 3; ++i )
    size[i] = static_cast<Types::Coordinate>( (dims[i] - 1) * calib[i] );

  // create volume, for the time being with empty data array.
  UniformVolume *volume = new UniformVolume( dims, size );
  volume->m_MetaInformation[META_IMAGE_ORIENTATION] = volume->m_MetaInformation[META_IMAGE_ORIENTATION_ORIGINAL] = orientation;

  // generate image filename from header file path.
  char imageFilename[PATH_MAX], *lastSlash;
  strcpy( imageFilename, path );
  if ( (lastSlash = strrchr( imageFilename, '/' ) ) )
    ++lastSlash;
  else
    lastSlash = imageFilename;
  strcpy( lastSlash, "image.bin" );

  // open image file and read data.
  CompressedStream imageStream( imageFilename );
  TypedArray* data = TypedArray::Create( TYPE_SHORT, dims[0] * dims[1] * dims[2] );
  imageStream.Read( data->GetDataPtr(), data->GetItemSize(), data->GetDataSize() );
  
#ifndef WORDS_BIGENDIAN
  // change endianness from Sun to whatever we're currently on.
  data->ChangeEndianness();
#endif
  
  // set volume data array with what we just read.
  volume->SetData( TypedArray::SmartPtr( data ) );
  
  return volume;
}

} // namespace cmtk
