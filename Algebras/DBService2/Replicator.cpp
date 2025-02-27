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
#include "Algebras/Distributed2/FileRelations.h"

#include "Algebras/DBService2/CommunicationClient.hpp"
#include "Algebras/DBService2/DebugOutput.hpp"
#include "Algebras/DBService2/ReplicationServer.hpp"
#include "Algebras/DBService2/ReplicationUtils.hpp"
#include "Algebras/DBService2/Replicator.hpp"
#include "Algebras/DBService2/SecondoUtilsLocal.hpp"

using namespace std;

namespace DBService {

Replicator::Replicator(
        std::string& dbName,
        std::string& relName,
        ListExpr type)
: runner(0), databaseName(dbName), relationName(relName), relType(type)
{
    printFunction("Replicator::Replicator", std::cout);
}

Replicator::~Replicator()
{
    printFunction("Replicator::~Replicator", std::cout);
}

void Replicator::createReplica(
        const std::string databaseName,
        const std::string relationName,
        const ListExpr relType,
        const bool async)
{
    printFunction("Replicator::createReplica", std::cout);
    print("databaseName", databaseName, std::cout);
    print("relationName", relationName, std::cout);
    print("async", string(async ? "TRUE" : "FALSE"), std::cout);

    if(async)
    {
        const string fileName = ReplicationUtils::getFileName(
                databaseName,
                relationName);

        SecondoCatalog* catalog = SecondoSystem::GetCatalog();
        if(!catalog->IsObjectName(relationName))
        {
            print("not an object", std::cout);
            return;
        }

        Word value;
        bool defined;
        do
        {
            print("object not yet defined", std::cout);
            catalog->GetObject(relationName, value, defined);
        }while(!defined);

        Relation* rel = (Relation*)value.addr;

        if(!BinRelWriter::writeRelationToFile(rel, relType,
                fileName))
        {
            print("Could not write relation to file", std::cout);
            return;
        }
    }

    print("Giving starting signal for replication", std::cout);
    string dbServiceHost;
    string dbServiceCommPort;
    SecondoUtilsLocal::lookupDBServiceLocation(
            dbServiceHost, dbServiceCommPort);
    CommunicationClient dbServiceMasterClient(dbServiceHost,
            atoi(dbServiceCommPort.c_str()),
            0);
    dbServiceMasterClient.giveStartingSignalForReplication(
            databaseName,
            relationName);
}

void Replicator::run(const bool async)
{
    printFunction("Replicator::run", std::cout);
    if(runner){
       runner->join();
       delete runner;
    }
    runner = new boost::thread(boost::bind(
            &Replicator::createReplica,
            this,
            databaseName,
            relationName,
            relType,
            async));
}

} /* namespace DBService */
