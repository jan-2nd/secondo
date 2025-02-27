/*
---- 
This file is part of SECONDO.

Copyright (C) 2019, University in Hagen, Department of Computer Science, 
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

//paragraph    [10]    title:           [{\Large \bf ] [}]
//paragraph    [21]    table1column:    [\begin{quote}\begin{tabular}{l}]     [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:   [\begin{quote}\begin{tabular}{ll}]    [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:   [\begin{quote}\begin{tabular}{lll}]   [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:   [\begin{quote}\begin{tabular}{llll}]  [\end{tabular}\end{quote}]
//[--------]    [\hline]
//characters    [1]    verbatim:   [$]    [$]
//characters    [2]    formula:    [$]    [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]
//[Contents] [\tableofcontents]

1 Header File: ExecutionContextTypes

September 2019, Fischer Thomas

1.1 Overview

The ~ExecutionContextEntity~ class represents a single entitiy of exection 
context. It is a composition of elements unique for this context.

1.2 Imports

*/

#ifndef SECONDO_PARTHREAD_ENTITY_H
#define SECONDO_PARTHREAD_ENTITY_H

#include "ExecutionContextTypes.h"
#include "../ConcurrentTupleBuffer/ConcurrentTupleBuffer.h"

namespace parthread
{

class ExecutionContextEntity
{
public:
    ExecutionContextEntity(const int entityIdx,const OpTree partialTree)
        : EntityIdx(entityIdx), LastSendMessage(INIT), Writer(NULL),
          PartialTree(partialTree){};

/*
1.3 Member

*/
    int EntityIdx;
    int LastSendMessage;
    ConcurrentTupleBufferWriter *Writer;
    OpTree PartialTree;
/*

  * ~EntitiyIdx~: is an identifier for the entity used for debug purposes

  * ~LastSendMessage~: the last message send to the partial tree of this entity
 
  * ~Writer~: writer to the tuple buffer of the context. It is necessary that 
    the same writer is related to the entity during the whole execution. For 
    hash-partitioning it is necessary to trigger the correct entity in case of 
    filled buffers.

  * ~PartialTree~: a copy of the partial tree enclosed by the context

*/
};

} // namespace parthread
#endif
