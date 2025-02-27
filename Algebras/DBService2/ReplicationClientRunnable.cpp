/*

1.1.1 Class Implementation

----
This file is part of SECONDO.

Copyright (C) 2017,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/
#include "Algebras/DBService2/DebugOutput.hpp"
#include "Algebras/DBService2/ReplicationClient.hpp"
#include "Algebras/DBService2/ReplicationClientRunnable.hpp"
#include "Algebras/DBService2/ReplicationUtils.hpp"

#include "boost/filesystem.hpp"

#include <loguru.hpp>

namespace fs = boost::filesystem;

using namespace std;

namespace DBService {

ReplicationClientRunnable::ReplicationClientRunnable(
        string targetHost,
        int targetTransferPort,
        string databaseName,
        string relationName)
: targetHost(targetHost), targetTransferPort(targetTransferPort),
  databaseName(databaseName), relationName(relationName),
  runner(0)
{
    print("ReplicationClientRunnable::ReplicationClientRunnable", std::cout);
    LOG_SCOPE_FUNCTION(INFO);
}

ReplicationClientRunnable::~ReplicationClientRunnable()
{
    print("ReplicationClientRunnable::~ReplicationClientRunnable", std::cout);
    LOG_SCOPE_FUNCTION(INFO);
}

void ReplicationClientRunnable::run()
{
    print("ReplicationClientRunnable::run", std::cout);
    LOG_SCOPE_FUNCTION(INFO);

    if(runner){
        runner->join();
        delete runner;
    }
    runner = new boost::thread(boost::bind(
            &ReplicationClientRunnable::create,
            this,
            targetHost,
            targetTransferPort,
            databaseName,
            relationName));
}

void ReplicationClientRunnable::create(
        string& targetHost,
        int targetTransferPort,
        string& databaseName,
        string& relationName)
{
    print("ReplicationClientRunnable::create", std::cout);
    LOG_SCOPE_FUNCTION(INFO);
    
    // loguru::set_thread_name("ReplicationClientRunnable");
    
    LOG_F(INFO, "%s", "getFilePathOnDBServiceWorker");
    const fs::path localPath =
        ReplicationUtils::getFilePathOnDBServiceWorker(
                    databaseName,
                    relationName);
                    
    const string remoteFilename =
        ReplicationUtils::getFileName(
                    databaseName,
                    relationName);


    LOG_F(INFO, "%s", "Creating the ReplicationClient...");
    ReplicationClient client(targetHost,
                             targetTransferPort,
                             localPath,
                             remoteFilename,
                             databaseName,
                             relationName);

    LOG_F(INFO, "%s", "ReceivingReplica...");
    client.receiveReplica();

    LOG_F(INFO, "%s", "Done.");
}

} /* namespace DBService */
