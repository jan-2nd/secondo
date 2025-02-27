/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] Header File of the ConnCodeMatrix

June-November, 2011. Thomas Uchdorf

[TOC]

1 Overview

This header file essentially contains the definition of the class
~ConnCodeMatrix~.

2 Defines and includes

*/
// [...]
#ifndef __CONN_CODE_MATRIX_H__
#define __CONN_CODE_MATRIX_H__

// --- Including header-files
//...

class ConnCodeMatrix {

public:

   // --- Constructors
   // Default-Constructor
   ConnCodeMatrix ();

   // Destructor
   ~ConnCodeMatrix ();

   // --- Methods
   void computeConnCode ();

   int getConnCode () const;   

   void setValues (const int (&values) [4][4]);
   
   // Transforming the matrix (Incrementing in vertical direction actually
   // means down and decrementing in vertical direction means up. So, the
   // 3rd row and the 4th row have to be swapped. The same applies to the
   // according columns.)
   // ------------------------------------------
   // |        |A_{up}|A_{down}|B_{up}|B_{down}|
   // ------------------------------------------
   // |A_{up}  |      |        |      |        |
   // ------------------------------------------
   // |A_{down}|      |        |      |        |
   // ------------------------------------------
   // |B_{up}  |      |        |      |        |
   // ------------------------------------------
   // |B_{down}|      |        |      |        |
   // ------------------------------------------
   //
   //                          |
   //                          |
   //                          V
   //
   // ------------------------------------------
   // |        |A_{up}|A_{down}|B_{down}|B_{up}|
   // ------------------------------------------
   // |A_{up}  |      |        |        |      |
   // ------------------------------------------
   // |A_{down}|      |        |        |      |
   // ------------------------------------------
   // |B_{down}|      |        |        |      |
   // ------------------------------------------
   // |B_{up}  |      |        |        |      |
   // ------------------------------------------ 
   void transform ();

   void shrinkTo3x3Mat (int missing);

   void print () const;

private:

   // --- Methods
   void initialize ();

   // --- Members
   int m_connCode;

   int **m_values;
};

#endif /* __CONN_CODE_MATRIX_H__ */
