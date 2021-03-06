/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2012 SRI International
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

#include "cmtkTypedStreamStudylist.h"

#include <Base/cmtkTypes.h>

#include <System/cmtkConsole.h>
#include <System/cmtkStrUtility.h>
#include <System/cmtkConsole.h>
#include <System/cmtkCompressedStream.h>
#include <System/cmtkMountPoints.h>

#include <IO/cmtkTypedStreamInput.h>
#include <IO/cmtkClassStreamInput.h>
#include <IO/cmtkClassStreamAffineXform.h>

namespace
cmtk
{

/** \addtogroup IO */
//@{

TypedStreamStudylist::TypedStreamStudylist()
{
  this->Clear();
}

void TypedStreamStudylist::Clear()
{
  StudyPath[0] = StudyPath[1] = NULL;
  ReferenceStudyIndex = 0;
  this->m_AffineXform = AffineXform::SmartPtr( NULL );
  this->m_WarpXform = WarpXform::SmartPtr( NULL );
}

TypedStreamStudylist::~TypedStreamStudylist()
{
  if ( StudyPath[0] ) free( StudyPath[0] );
  if ( StudyPath[1] ) free( StudyPath[1] );
}

bool
TypedStreamStudylist::Read( const std::string& studylistpath )
{
  char archive[PATH_MAX];

  snprintf( archive, sizeof( archive ), "%s%cstudylist", MountPoints::Translate( studylistpath ).c_str(), (int)CMTK_PATH_SEPARATOR );
  ClassStreamInput classStream( archive );
  if ( ! classStream.IsValid() ) 
    {
    StdErr.printf( "Could not open studylist archive %s.\n", archive );
    return false;
    }
  
  if ( StudyPath[0] ) free( StudyPath[0] );
  classStream.Seek ( "source" );
  StudyPath[0] = classStream.ReadString( "studyname", "<unknown>" );
  
  if ( StudyPath[1] ) free( StudyPath[1] );
  classStream.Seek ( "source" );
  StudyPath[1] = classStream.ReadString( "studyname", "<unknown>" );
  classStream.Close();
  
  snprintf( archive, sizeof( archive ), "%s%cregistration", MountPoints::Translate(studylistpath).c_str(), (int)CMTK_PATH_SEPARATOR  );
  classStream.Open( archive );
  if ( ! classStream.IsValid() ) 
    {
    StdErr.printf( "Could not open studylist archive %s.\n", archive );
    return false;
    }
  
  classStream.Seek ( "registration" );
  char *referenceStudy = classStream.ReadString( "reference_study" );
  ReferenceStudyIndex = ( StrCmp( referenceStudy, StudyPath[0] ) ) ? 1 : 0;

  bool legacy = false;
  char *floatingStudy = classStream.ReadString( "floating_study" );
  if ( !floatingStudy )
    {
    classStream.Begin();
    floatingStudy = classStream.ReadString( "model_study" );
    if ( floatingStudy )
      {
      legacy = true;
      }
    else
      {
      StdErr.printf( "WARNING: Studylist %s/registration apparently has neither new 'floating_study' nor old 'model_study' entry\n", archive );
      }
    }
  
  classStream >> this->m_AffineXform;

  if ( referenceStudy )
    {
    this->m_AffineXform->SetMetaInfo( META_XFORM_FIXED_IMAGE_PATH, referenceStudy );
    }
  if ( floatingStudy )
    {
    this->m_AffineXform->SetMetaInfo( META_XFORM_MOVING_IMAGE_PATH, floatingStudy );
    }

  if ( legacy )
    {
    this->m_AffineXform = AffineXform::SmartPtr( this->m_AffineXform->MakeInverse() );
    }

  classStream.Get( this->m_WarpXform );
  if ( this->m_WarpXform )
    {
    if ( referenceStudy )
      {
      this->m_WarpXform->SetMetaInfo( META_XFORM_FIXED_IMAGE_PATH, referenceStudy );
      }
    if ( floatingStudy )
      {
      this->m_WarpXform->SetMetaInfo( META_XFORM_MOVING_IMAGE_PATH, floatingStudy );
      }
    }
  
  classStream.Close();
  return true;
}

} // namespace
