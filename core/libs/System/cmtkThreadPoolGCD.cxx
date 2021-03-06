/*
//
//  Copyright 1997-2009 Torsten Rohlfing
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

#include "cmtkThreadPoolGCD.h"

#include <System/cmtkThreads.h>
#include <System/cmtkConsole.h>

namespace
cmtk
{

ThreadPoolGCD::ThreadPoolGCD( const size_t nThreads )
{
  if ( ! nThreads )
    this->m_NumberOfThreads = cmtk::Threads::GetNumberOfThreads();
  else
    this->m_NumberOfThreads = nThreads;
  
  this->m_Queues.resize( this->m_NumberOfThreads );
  for ( size_t i = 0; i < this->m_NumberOfThreads; ++i )
    {
    this->m_Queues[i] = dispatch_queue_create( "ThreadPoolGCD", 0 );
    }
}

ThreadPoolGCD::~ThreadPoolGCD()
{
  for ( size_t i = 0; i < this->m_NumberOfThreads; ++i )
    {
    dispatch_release( this->m_Queues[i] );
    }
}

void
ThreadPoolGCD::Dispatch( Self::TaskFunction taskFunction, std::vector<void*>& taskParameters, const size_t numberOfTasksOverride )
{
  const size_t numberOfTasks = numberOfTasksOverride ? numberOfTasksOverride : taskParameters.size();
  if ( ! numberOfTasks )
    {
    StdErr << "ERROR: trying to run zero tasks on thread pool. Did you forget to resize the parameter vector?\n";
    exit( 1 );
    }

  const std::vector<void*>& vParams = taskParameters;

  const size_t nQueues = this->m_Queues.size();
  for ( size_t taskIdx = 0; taskIdx < numberOfTasks; )
    {
    for ( size_t queueIdx = 0; queueIdx < nQueues; ++queueIdx, ++taskIdx )
      {
      if ( taskIdx < numberOfTasks )
	{
        dispatch_async( this->m_Queues[queueIdx], 
			^{
			  taskFunction( (void*)( vParams[taskIdx] ), taskIdx, numberOfTasks, queueIdx, nQueues );
			} );
	}
      }
    }

  // wait for all queues to finish
  for ( size_t queueIdx = 0; queueIdx < nQueues; ++queueIdx )
    {
    dispatch_sync( this->m_Queues[queueIdx], ^{} );
    }
}

ThreadPoolGCD& 
ThreadPoolGCD::GetGlobalThreadPool()
{
  static ThreadPoolGCD globalThreadPoolGCD;
  return globalThreadPoolGCD;
}

}
