##
##  Copyright 1997-2010 Torsten Rohlfing
##
##  Copyright 2004-2014 SRI International
##
##  Copyright 2015 Google, Inc.
##
##  This file is part of the Computational Morphometry Toolkit.
##
##  http://www.nitrc.org/projects/cmtk/
##
##  The Computational Morphometry Toolkit is free software: you can
##  redistribute it and/or modify it under the terms of the GNU General Public
##  License as published by the Free Software Foundation, either version 3 of
##  the License, or (at your option) any later version.
##
##  The Computational Morphometry Toolkit is distributed in the hope that it
##  will be useful, but WITHOUT ANY WARRANTY; without even the implied
##  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License along
##  with the Computational Morphometry Toolkit.  If not, see
##  <http://www.gnu.org/licenses/>.
##
##  $Revision$
##
##  $LastChangedDate$
##
##  $LastChangedBy$
##

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  SET(CPACK_GENERATOR "ZIP")
ELSE(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    SET(CPACK_GENERATOR "TGZ;RPM")
  ELSE(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    SET(CPACK_GENERATOR "TGZ")
  ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

# Optionally override system name and CPU type
IF(NOT CMTK_SYSTEM_NAME)
  SET(CMTK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
ENDIF(NOT CMTK_SYSTEM_NAME)

IF(NOT CMTK_SYSTEM_PROCESSOR)
  SET(CMTK_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR})
ENDIF(NOT CMTK_SYSTEM_PROCESSOR)

SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.txt")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CMTK -- The Computational Morphometry Toolkit")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR "${CMTK_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${CMTK_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${CMTK_VERSION_PATCH}")
SET(CPACK_PACKAGE_FILE_NAME "CMTK-${CMTK_VERSION_MAJOR}.${CMTK_VERSION_MINOR}.${CMTK_VERSION_PATCH}-${CMTK_SYSTEM_NAME}-${CMTK_SYSTEM_PROCESSOR}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMTK-${CMTK_VERSION_MAJOR}.${CMTK_VERSION_MINOR}")

SET(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
SET(CPACK_RPM_PACKAGE_LICENSE "GPL v3")
SET(CPACK_RPM_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
SET(CPACK_RPM_COMPONENT_INSTALL ON)
SET(CPACK_RPM_PACKAGE_URL "http://nitrc.org/projects/cmtk" )
##SET(CPACK_RPM_CHANGELOG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG" ) ## Our Changelog is in wrong format

SET(CPACK_TGZ_COMPONENT_INSTALL ON)

# mandatory package
IF(CMTK_USE_QT)
  SET(CPACK_RPM_PACKAGE_REQUIRES "qt >= 4.7.0")
ENDIF(CMTK_USE_QT)

# mandatory package with optional bundled replacement
IF(NOT CMTK_BUILD_MXML)
  SET(CPACK_RPM_PACKAGE_REQUIRES "mxml >= 2.5")
ENDIF(NOT CMTK_BUILD_MXML)

# optional package with optional bundled replacement
IF(CMTK_USE_DCMTK AND NOT CMTK_BUILD_DCMTK)
  SET(CPACK_RPM_PACKAGE_REQUIRES "dcmtk >= 3.5.4")
ENDIF(CMTK_USE_DCMTK AND NOT CMTK_BUILD_DCMTK)

# optional package with optional bundled replacement
IF(CMTK_USE_SQLITE AND NOT CMTK_BUILD_SQLITE)
  SET(CPACK_RPM_PACKAGE_REQUIRES "sqlite >= 3.7.4")
ENDIF(CMTK_USE_SQLITE AND NOT CMTK_BUILD_SQLITE)

# optional package
IF(CMTK_USE_BZIP2)
  SET(CPACK_RPM_PACKAGE_REQUIRES "bzip2-devel")
ENDIF(CMTK_USE_BZIP2)

# optional package
IF(CMTK_USE_LZMA)
  SET(CPACK_RPM_PACKAGE_REQUIRES "lzma-devel")
ENDIF(CMTK_USE_LZMA)

# optional package
IF(CMTK_USE_FFTW_FOUND)
  SET(CPACK_RPM_PACKAGE_REQUIRES "fftw-devel >= 3.3")

  IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    STRING(REPLACE ".lib" ".dll" SYSTEM_FFTW_LIBRARIES ${CMTK_FFTW_LIBRARIES})
    STRING(REPLACE "/" "\\\\" SYSTEM_FFTW_LIBRARIES ${SYSTEM_FFTW_LIBRARIES})
    LIST(APPEND CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${SYSTEM_FFTW_LIBRARIES})
  ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
ENDIF(CMTK_USE_FFTW_FOUND)

set(CPACK_COMPONENTS_ALL tools gui libraries headers documentation)
INSTALL(FILES ${CPACK_RESOURCE_FILE_LICENSE} ${CPACK_PACKAGE_DESCRIPTION_FILE} DESTINATION ${CMTK_INSTALL_DATA_DIR}/doc/ COMPONENT documentation)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Licenses DESTINATION ${CMTK_INSTALL_DATA_DIR}/doc/ COMPONENT documentation)

INCLUDE(CPack)

CPACK_ADD_COMPONENT(tools
  DISPLAY_NAME "Command Line Tools"
  GROUP core)

CPACK_ADD_COMPONENT(gui
  DISPLAY_NAME "Graphical User Interface Applications"
  GROUP core)

CPACK_ADD_COMPONENT(libraries
  DISPLAY_NAME "Link Libraries"
  GROUP devel
  DISABLED)

CPACK_ADD_COMPONENT(headers
  DISPLAY_NAME "C/C++ Header Files"
  GROUP devel
  DISABLED)

CPACK_ADD_COMPONENT(runtime
  DISPLAY_NAME "Runtime Components"
  GROUP core)

CPACK_ADD_COMPONENT(documentation
  DISPLAY_NAME "CMTK Documentation"
  GROUP core)

CPACK_ADD_COMPONENT_GROUP(devel
  DISPLAY_NAME "Development Components"
  DEPENDS core)

CPACK_ADD_COMPONENT_GROUP(core
  DISPLAY_NAME "Core Components")

IF(BUILD_SHARED_LIBS)
  SET(CPACK_COMPONENT_TOOLS_DEPENDS libraries)
  SET(CPACK_COMPONENT_GUI_DEPENDS libraries)
ENDIF(BUILD_SHARED_LIBS)

INCLUDE(InstallRequiredSystemLibraries)
