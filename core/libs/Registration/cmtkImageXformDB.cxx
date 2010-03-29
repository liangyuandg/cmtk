/*
//
//  Copyright 2010 SRI International
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

#include <cmtkImageXformDB.h>

#include <cmtkConsole.h>

#include <string>
#include <sstream>

#include <cstdlib>
#include <cassert>

cmtk::ImageXformDB
::ImageXformDB( const std::string& dbPath, const bool readOnly ) 
  : cmtk::SQLite( dbPath, readOnly )
{
  // create entity tables
  if ( ! this->TableExists( "images" ) )
    {
    this->Exec( "CREATE TABLE images(id INTEGER PRIMARY KEY, space INTEGER, path TEXT)" );
    }
  
  if ( ! this->TableExists( "xforms" ) )
    {
    this->Exec( "CREATE TABLE xforms(id INTEGER PRIMARY KEY, path TEXT, invertible INTEGER, spacefrom INTEGER, spaceto INTEGER)" );
    }
}

void
cmtk::ImageXformDB
::AddImage( const std::string& imagePath, const std::string& spacePath )
{
  if ( (spacePath == "") )
    {
    this->Exec( "INSERT INTO images (path) VALUES ('"+imagePath+"')" );
    this->Exec( "UPDATE images SET space=(SELECT id FROM images WHERE path='"+imagePath+"') WHERE path='"+imagePath+"'" );
    }
  else
    {
    PrimaryKeyType spaceKey = this->FindImageSpaceID( spacePath );
    if ( (spaceKey == Self::NOTFOUND) )
      {
      this->Exec( "INSERT INTO images (path) VALUES ('"+spacePath+"')" );
      this->Exec( "UPDATE images SET space=(SELECT space FROM images WHERE path='"+spacePath+"') WHERE path='"+spacePath+"'" );
      spaceKey = this->FindImageSpaceID( spacePath );
      }
    
    std::ostringstream sql;
    sql << "INSERT INTO images (space,path) VALUES ( " << spaceKey << ", '" << imagePath << "')";
    this->Exec( sql.str() );
    }
}

bool
cmtk::ImageXformDB
::AddXform
( const std::string& xformPath, const bool invertible, const std::string& imagePathSrc, const std::string& imagePathTrg )
{
  PrimaryKeyType spaceKeySrc = this->FindImageSpaceID( imagePathSrc );
  if ( spaceKeySrc == Self::NOTFOUND )
    {
    this->AddImage( imagePathSrc );
    spaceKeySrc = this->FindImageSpaceID( imagePathSrc );
    assert( spaceKeySrc != Self::NOTFOUND );
    }

  PrimaryKeyType spaceKeyTrg = this->FindImageSpaceID( imagePathTrg );
  if ( spaceKeyTrg == Self::NOTFOUND )
    {
    this->AddImage( imagePathTrg );
    spaceKeyTrg = this->FindImageSpaceID( imagePathTrg );
    assert( spaceKeyTrg != Self::NOTFOUND );
    }

  if ( spaceKeyTrg == spaceKeySrc )
    {
    StdErr << "WARNING - cmtk::ImageXformDB::AddXform - source and target image of transformation are in the same space; bailing out.\n";
    return false;
    }
  
  std::ostringstream sql;
  sql << "INSERT INTO xforms (path,invertible,spacefrom,spaceto) VALUES ( '" << xformPath << "', " << (invertible ? 1 : 0) << ", " << spaceKeySrc << ", " << spaceKeyTrg << ")";
  this->Exec( sql.str() );

  return true;
} 

cmtk::ImageXformDB::PrimaryKeyType 
cmtk::ImageXformDB
::FindImageSpaceID( const std::string& imagePath )
{
  if ( imagePath != "" )
    {
    const std::string sql = "SELECT space FROM images WHERE path='"+imagePath+"'";
    
    SQLite::TableType table;
    this->Query( sql, table );
    
    if ( table.size() && table[0].size() )
      return atoi( table[0][0].c_str() );
    }

  return Self::NOTFOUND;
}

bool
cmtk::ImageXformDB
::FindXform( const std::string& imagePathSrc, const std::string& imagePathTrg, std::string& xformPath, bool& inverse )
{
  const PrimaryKeyType spaceKeySrc = this->FindImageSpaceID( imagePathSrc );
  const PrimaryKeyType spaceKeyTrg = this->FindImageSpaceID( imagePathTrg );

  if ( (spaceKeySrc == Self::NOTFOUND) || (spaceKeyTrg == Self::NOTFOUND) )
    {
    // if either space is not in the database, then clearly no transformation can reach it
    return false;
    }

  if ( spaceKeySrc == spaceKeyTrg )
    {
    xformPath = "";
    inverse = false;
    return true;
    }

  std::ostringstream sql;
  sql << "SELECT path FROM xforms WHERE ( spacefrom=" << spaceKeySrc << " AND spaceto=" << spaceKeyTrg << " ) ORDER BY invertible";

  SQLite::TableType table;
  this->Query( sql.str(), table );
  if ( table.size() && table[0].size() )
    {
    inverse = false;
    xformPath = table[0][0];
    return true;
    }
  
  sql.str( "" );
  sql << "SELECT path FROM xforms WHERE ( spacefrom=" << spaceKeyTrg << " AND spaceto=" << spaceKeySrc << " ) ORDER BY invertible";
  
  this->Query( sql.str(), table );
  if ( table.size() && table[0].size() )
    {
    inverse = true;
    xformPath = table[0][0];
    return true;
    }

  return false;
}
