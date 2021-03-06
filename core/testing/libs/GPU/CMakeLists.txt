##
##  Copyright 1997-2009 Torsten Rohlfing
##
##  Copyright 2004-2010 SRI International
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

# ==========================================
# Setup binary test drivers

SET(CMTK_LIBRARIES "cmtkGPU;cmtkBase;cmtkNumerics;cmtkSystem")
SET(DRIVERS libGPUTests)

FOREACH(D ${DRIVERS})
  CUDA_ADD_EXECUTABLE(${D} ${D}.cxx)
  TARGET_LINK_LIBRARIES(${D} ${CMTK_LIBRARIES} ${CMTK_SYSTEM_LIBRARIES})
ENDFOREACH(D ${DRIVERS})

IF(CMTK_USE_CUDA)

  # ==========================================
  # Tests for "cmtk::DeviceHistogram" class
  SET(Tests ${Tests} 
    DeviceHistogramEntropy
    DeviceHistogramPopulate
    DeviceHistogramPopulateLog)
  
  # ==========================================
  # Tests for "cmtk::DeviceMemory" class
  SET(Tests ${Tests} 
    DeviceMemory)
  
  # ==========================================
  # Tests for "cmtk::UniformVolume" class
  SET(Tests ${Tests} 
    DeviceUniformVolume)
  
ENDIF(CMTK_USE_CUDA)

FOREACH(T ${Tests})
  ADD_TEST(${T} ${EXECUTABLE_OUTPUT_PATH}/libGPUTests ${T})
ENDFOREACH(T Tests)
