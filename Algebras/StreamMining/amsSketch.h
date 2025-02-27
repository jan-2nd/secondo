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

*/

#ifndef AMSSKETCH_H
#define AMSSKETCH_H

#include "NestedList.h"
#include "ListUtils.h"
#include "AlgebraTypes.h"

# define LONG_PRIME 4294967311l

namespace eschbach {
    class amsSketch
{
  public:
  amsSketch(const double epsilon, const double delta);
  amsSketch(const amsSketch& rhs);
  ~amsSketch() {}


  //Getter und Setter
  bool getDefined();
  size_t getWidth();
  size_t getDepth();
  double getEpsilon();
  double getDelta();
  size_t getTotalCount();
  int getElement(int counterNumber, int elementIndex);
  void updateElement(int counterNumber, int elementIndex, int value);
  long getConstantTwA(int index);
  long getConstantTwB(int index);
  long getConstantFwA(int index);
  long getConstantFwB(int index);  
  long getConstantFwC(int index);
  long getConstantFwD(int index);


  std::vector<std::vector<long>> getConstantsTw();
  void setConstantsTw(int counterNumber, long a, long b);
  std::vector<std::vector<long>> getConstantsFw();
  void setConstantsFw(int counterNumber, long a, long b, long c, long d);
  std::vector<std::vector<int>> getMatrix();

  //Auxiliary Functions
  void initialize(const double epsilon, const double delta);
  //Generates the 2-wise constants for every rows 2-wise hashfunction
  void generateConstants(int index);
  //Generates the 4-wise constants for every rows 4-wise hashfunction
  void generateFwConstants(int index);
  //Increases or decreases an elements counter
  void changeWeight(size_t value);
  //Retrieves the estimation of the selfjoin size
  int estimateInnerProduct();

  void swap(int* a, int* b);
  int partition(int arr[], int l, int r);
  int randomPartition(int arr[], int l, int r);
  void medianDecider(int arr[], int l, int r, int k, int& a, int& b);
  int findMedian(int arr[], int size);

  //Support Functions
  static Word     In( const ListExpr typeInfo, const ListExpr instance,
                      const int errorPos, ListExpr& errorInfo, bool& correct );
  
  static ListExpr Out( ListExpr typeInfo, Word value );

  //Storage Record
  static Word     Create( const ListExpr typeInfo );

  static void     Delete( const ListExpr typeInfo, Word& w );

  static bool     Open(SmiRecord& valueRecord, size_t& offset, 
                         const ListExpr typeInfo, Word& value);
  
  static void     Close( const ListExpr typeInfo, Word& w );

  static bool     Save(SmiRecord & valueRecord , size_t & offset,
                       const ListExpr typeInfo , Word & value);
  
  static Word     Clone( const ListExpr typeInfo, const Word& w );

  static bool     KindCheck( ListExpr type, ListExpr& errorInfo );

  static int      SizeOfObj();

  static ListExpr Property();


   static const std::string BasicType() {
    return "amssketch";
  }

  static const bool checkType (const ListExpr list) {
    return listutils::isSymbol(list, BasicType());
  }

  private:
    amsSketch() {}
    friend struct ConstructorFunctions<amsSketch>;
    bool defined;
    //Errorbound
    double eps;
    //Inverse Probability of the answers being
    //within the errorbound
    double delta; 
    //Width of the Matrix
    size_t width;
    //Depth of the Matrix
    size_t depth;
    //Amount of elements seen
    size_t totalCount;
    //2d structure for the counters
    std::vector<std::vector<int>> matrix;
    //Vector for saving the 2-wise hashConstants calculated
    //for each row of the matrix
    std::vector<std::vector<long>> twConstants;
    //Vector for saving the 4-wise hashConstants calculated
    //for each row of the matrix
    std::vector<std::vector<long>> fwConstants;
  };
}

#endif
