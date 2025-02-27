/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

December 2005, Victor Almeida deleted the deprecated algebra levels (~executable~,
~descriptive~, and ~hybrid~). Only the executable level remains.

April 2006, M. Spiekermann. Display function for type text changed since its output
format was changed. Moreover, the word wrapping of text atoms was improved.

August 2009, M. Spiekermann. Many code changes happen in order to make implementing
display functions easier: 1) There is no need to add new static functions to the DisplayTTY.h
file, 2) Errors are now returned using exceptions 3) In case an error is caught, the generic
display functions will be used as fallback. 4) To do: Removing the algebra- and type-id from the
key in the internal map; the name of the datatype should be sufficient.


\def\CC{C\raise.22ex\hbox{{\footnotesize +}}\raise.22ex\hbox{\footnotesize +}\xs
pace}
\centerline{\LARGE \bf  DisplayTTY}

\centerline{Friedhelm Becker , Mai1998}

\begin{center}
\footnotesize
\tableofcontents
\end{center}

1 Overview

There must be exactly one TTY display function of type DisplayFunction
for every type constructor provided by any of the algebra modules which
are loaded by *AlgebraManager2*. The first parameter is the type
expression in numeric nested list format describing the value which is
going to be displayed by the display function. The second parameter is
this value in nested list format.

1.2 Includes and defines

maxLineLength gives the maximum length of an input line (an command)
which will be read.

*/


#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <math.h>
#include <stdexcept>
#include <iterator>
#include <map>
#include <algorithm>
#include <iomanip>


#include "../include/DisplayTTY.h"
#include "../include/NestedList.h"
#include "../include/Base64.h"
#include "../include/Symbols.h"

#include "../Algebras/Raster2/DisplaySType.h"
#include "../Algebras/Raster2/Displaymstype.h"
#include "../Algebras/Raster2/Displayistype.h"
#include "../Algebras/Raster2/Displaygrid2.h"
#include "../Algebras/Raster2/Displaygrid3.h"

/*
Auxiliary global variables and functions

*/

#define LINELENGTH 80

using namespace std;

int DisplayTTY::maxIndent = 0;

const string stdErrMsg = "Incorrect Data Format!";

/*
1.3 Managing display functions

The map *displayFunctions* holds all existing display functions. It
is indexed by a string created from the algebraId and the typeId of
the corresponding type constructor.

*/
NestedList*       DisplayFunction::nl = 0;

double
DisplayFunction::GetNumeric(ListExpr value, bool &err){
  return DisplayTTY::GetInstance().GetNumeric(value, err);
}

void
DisplayFunction::CallDisplayFunction( const string& name,
                                      ListExpr type,
                                      ListExpr value ) {
   DisplayTTY::GetInstance().CallDisplayFunction( name, type,value );
}

void
DisplayFunction::DisplayResult( ListExpr type, ListExpr value )
{
  DisplayTTY::GetInstance().DisplayResult(type, value);
}

 int DisplayFunction::MaxAttributLength( ListExpr type )
  {
    int max=0, len=0;
    string s="";
    while (!nl->IsEmpty( type ))
    {
      s = nl->ToString( nl->First( nl->First( type ) ) );
      len = s.length();
      if ( len > max )
      {
        max = len;
      }
      type = nl->Rest( type );
    }
    return (max);
  }


NestedList*            DisplayTTY::nl = 0;
DisplayTTY*            DisplayTTY::dtty = 0;
//DisplayTTY::DisplayMap DisplayTTY::displayFunctions;


DisplayTTY::~DisplayTTY()
{
 DisplayMap::iterator it = displayFunctions.begin();
 for (; it != displayFunctions.end(); it++){
     delete it->second;
     it->second=0;
 }
}


/*
The function *CallDisplayFunction* uses its first argument *idPair*
--- consisting of the two-elem-list $<$algebraId, typeId$>$ --- to
find the right display function in the array *displayFunctions*. The
arguments *typeArg* and *valueArg* are simply passed to this display
function.

*/

void DisplayTTY::CallDisplayFunction( const string& name,
                                      ListExpr type,
                                      ListExpr value ) {
  
  DisplayMap::iterator dfPos = displayFunctions.find(name );
  if ( dfPos != displayFunctions.end() ) {
    try {
      (dfPos->second)->Display( type, value );
    } catch ( const exception& e) {
      cerr << "Error in display function: " << e.what() << endl << endl;
      DisplayGeneric( type, value );
    }
  } else {
    DisplayGeneric( type, value );
  }
}

/*
The procedure *InsertDisplayFunction* inserts the procedure given as
second argument into the array displayFunctions at the index which is
determined by the type constructor name given as first argument.

*/

void DisplayTTY::Insert( const string& name, DisplayFunction* df )
{
  displayFunctions[name] = df;
}

/*
1.4 Auxiliary Functions

*/

void
DisplayTTY::DisplayGeneric( ListExpr type, ListExpr value )
{
  cout << "Generic display function used!" << endl;
  cout << "Type : ";
  nl->WriteListExpr( type, cout );
  cout << endl << "Value: ";
  nl->WriteListExpr( value, cout );
}


string DisplayTTY::getStr(ListExpr le){

  int at = nl->AtomType(le);
  switch(at){
    case StringType : return nl->StringValue(le);
    case TextType : return nl->Text2String(le);
    case SymbolType : return nl->SymbolValue(le);
    default : return nl->ToString(le);
  };
}


void
DisplayTTY::DisplayDescriptionLines( ListExpr value, int  maxNameLen)
{

  string errMsg = string("Error: Unknown List Format. ") +
      "Expecting (name (labels) (entries)) but got \n" +
      nl->ToString(value) + "\n";

  if ( !nl->HasLength(value,3)) {
    cout << errMsg << endl;
    return;
  }

  ListExpr nameSymbol = nl->First(value);
  ListExpr labels = nl->Second(value); 
  ListExpr entries = nl->Third(value) ;

  if (    (nl->AtomType(nameSymbol) != SymbolType)
       || (nl->AtomType(labels)!=NoAtom) 
       || (nl->AtomType(entries)!=NoAtom)) {
    cout << errMsg;
    return;
  }

  cout << endl;
  string name = nl->SymbolValue(nameSymbol);
  string blanks = "";
  blanks.assign( maxNameLen-4 , ' ' );
  cout << blanks << "Name: " << name << endl;

  while ( !nl->IsEmpty(labels) && !nl->IsEmpty(entries)) {
      string s = getStr(nl->First(labels));
      blanks.assign( maxNameLen - s.length() , ' ' );
      string printstr = blanks + s + ": ";
      string e = getStr(nl->First(entries));
      printstr += e;
      cout << wordWrap((size_t)0, (size_t)(maxNameLen+2),
                       (size_t)LINELENGTH, printstr) << endl;
      labels = nl->Rest(labels);
      entries = nl->Rest(entries);
    }
}

bool
DisplayTTY::DisplayResult( ListExpr type, ListExpr value )
{
  string  name("");
  if(nl->AtomType(type)==SymbolType){
     CallDisplayFunction(nl->SymbolValue(type), type, value);
  } else {
     if(!nl->HasMinLength(type,2)){
        return false;
     } 
     ListExpr mt = nl->First(type);
     if(nl->AtomType(mt)!=SymbolType){
        return false;
     }
     CallDisplayFunction(nl->SymbolValue(mt), type, value);
  }
  cout << endl;
  return true;
}



/*
2 User Defined Display Functions

Display functions of the DisplayTTY module are used to  transform a
nested list value into a pretty printed output in text format. Display
functions which are called with a value of compound type usually call
recursively the Display functions of the subtypes, passing the subtype
and subvalue, respectively.

*/


struct DisplayRelation : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value )
  {
    type = nl->Second( type );
    CallDisplayFunction(nl->SymbolValue(nl->First(type)), type, value );
  }
};


struct DisplayTuples : DisplayFunction {

  DisplayTuples() : DisplayFunction() {}

  virtual void Display( ListExpr type, ListExpr value ) {
    int maxAttribNameLen = MaxAttributLength( nl->Second( type ) );
    while (!nl->IsEmpty( value ))
    {
      DisplayTuple( nl->Second( type ), nl->First( value ), maxAttribNameLen );
      value = nl->Rest( value );
      cout << endl;
    }
  }

  void DisplayTuple( ListExpr type, ListExpr value, const int maxNameLen ) {

    while (!nl->IsEmpty( value )) { // for each attribute
      cout << endl;
      ListExpr namedAttrType = nl->First(type);
      ListExpr attrValue = nl->First(value);
      string attrName = nl->SymbolValue(nl->First(namedAttrType));     
      string attr = string( maxNameLen-attrName.length() , ' ' ) + attrName 
                    + string(" : ");
      cout << attr;
      DisplayTTY::maxIndent = attr.length();

      ListExpr attrType = nl->Second(namedAttrType);

      string attrtypename="";
      if(nl->AtomType(attrType)==SymbolType){
        attrtypename = nl->SymbolValue(attrType);
      } else {
        attrtypename = nl->SymbolValue(nl->First(attrType));
      }  

      CallDisplayFunction(attrtypename, attrType, attrValue);
 
      DisplayTTY::maxIndent = 0;
      type    = nl->Rest( type );
      value   = nl->Rest( value );
    }
  }
};



struct DisplayNestedRelation : DisplayFunction
{
  virtual void Display( ListExpr type, ListExpr value )
  {
    type = nl->Second( type );
    CallDisplayFunction(nl->SymbolValue(nl->First(type)), type, value );
  }
};


struct DisplayAttributeRelation : DisplayFunction
{
  DisplayAttributeRelation() : DisplayFunction() {}

  virtual void Display( ListExpr type, ListExpr value)
  {
    int select = nl->IntValue(nl->First(value));
    value = nl->Rest(value);
    if (select == 0)//means that this arel is an attribute of a nested relation
    {
      int maxAttribNameLen = MaxAttributLength(nl->Second(nl->Second(type)));
      int ind;
      if (maxAttribNameLen >= (DisplayTTY::maxIndent - 3))
        ind = 4;
      else
        ind = DisplayTTY::maxIndent - 3 - maxAttribNameLen + 4;
      cout << endl;
      while (!nl->IsEmpty( value ))
      {
        DisplayArelTuple( nl->Second( nl->Second( type ) ), nl->First( value ),
                      maxAttribNameLen, ind );
        value = nl->Rest( value );
        if (!nl->IsEmpty(value))
          cout << endl;
      }
      DisplayTTY::maxIndent = 0;
    }
    else
    {
      long tid;
      cout << endl << "Saved TupleIds: " << endl << endl;
      int i = 1;
      while (!nl->IsEmpty(value))
      {
        tid = nl->IntValue(nl->First(value)); 
        cout << "TupleId no. " << i << ": " << tid << endl;
        value = nl->Rest(value); 
        i++;
      }
    }
  }

  void DisplayArelTuple( ListExpr type, 
                         ListExpr value, 
                         const int maxNameLen,
                         int ind ) {

    while (!nl->IsEmpty( value ))
    {
      string s = nl->ToString( nl->First( nl->First( type ) ) ); // attrname
      int i = maxNameLen - s.length();
      if ( i < 0) {
        i = 0;
      }
      string attr = string(ind , ' ') + s +
                  string( i , ' ') + string(" : ");
      cout << attr;
      DisplayTTY::maxIndent = attr.length();
      string tn = "";
      ListExpr attrType = nl->Second(nl->First(type));
      if(nl->AtomType(attrType)==SymbolType){
         tn = nl->SymbolValue(attrType);
      } else {
         tn = nl->SymbolValue(nl->First(attrType));
      }
      CallDisplayFunction(tn, attrType, nl->First(value));
      cout << endl;
      type    = nl->Rest( type );
      value   = nl->Rest( value );
    }
  }
};

struct DisplayARel : DisplayFunction
{
    const static int c_indentBy = 4;

    virtual void Display(ListExpr type, ListExpr value)
    {
      DisplayRelation(type, value);
    }

    static void DisplayRelation(ListExpr type, ListExpr value,
        int indentation = 0)
    {
      ListExpr attrDescs = nl->Second(nl->Second(type));
      if (indentation == 0)
      {
        indentation = GetBaseIndentation(attrDescs);
      }
      if (!nl->IsAtom(value))
      {
        cout << endl; // Print Subvalues in new line and increase indentation
        while (!nl->IsEmpty(value))
        {
          const ListExpr &currentTuple = nl->First(value);
          DisplayTuple(attrDescs, currentTuple, indentation);
          value = nl->Rest(value);
        }
      }
      else
      {
        cout << nl->ToString(value) << endl;
      }
    }

  private:

    static void DisplayTuple(ListExpr attrDescs, ListExpr value,
        const int indentation)
    {
      while (!nl->IsEmpty(attrDescs))
      {
        const ListExpr &currentAttrDesc = nl->First(attrDescs);
        const string &attrName = nl->SymbolValue(nl->First(currentAttrDesc));
        const ListExpr &currentAttrType = nl->Second(currentAttrDesc);
        const string &attrShortType = GetShortType(currentAttrType);
        cout << string(indentation - attrName.length(), ' ') << attrName
            << " : ";
        if ((attrShortType == "arel2") || (attrShortType == "nrel2"))
        {
          DisplayRelation(currentAttrType, nl->First(value),
              indentation + c_indentBy);
          //Print an empty line after a complete (sub)relation.
          //On higher levels this will sum up to multiple empty lines,
          //the "bigger" the relation the bigger the space.
        }
        else
        {
          DisplayTTY::GetInstance().CallDisplayFunction(attrShortType,
              currentAttrType, nl->First(value));
          cout << endl;
        }
        attrDescs = nl->Rest(attrDescs);
        value = nl->Rest(value);
      }
      //Print an empty line after each tuple to group attributes of one tuple
      //together visually
      cout << endl;
    }

    static int GetBaseIndentation(ListExpr attrDescs)
    {
      int result = 0;
      //listForeach should be available here...
      while (!nl->IsEmpty(attrDescs))
      {
        ListExpr currentAttrDesc = nl->First(attrDescs);
        const ListExpr &attrType = nl->Second(currentAttrDesc);
        string shortType = GetShortType(attrType);
        int len = 0;
        const string &attrName = nl->SymbolValue(
            nl->First(currentAttrDesc));
        len = attrName.length();
        result = (len > result) ? len : result;
        if ((shortType == "arel2") || (shortType == "nrel2"))
        {
          len = GetBaseIndentation(nl->Second(nl->Second(attrType)))-c_indentBy;
          result = (len > result) ? len : result;
        }
        attrDescs = nl->Rest(attrDescs);
      }
      return result;
    }

    static string GetShortType(const ListExpr attrType)
    {
      return
          nl->IsAtom(attrType) ?
              nl->SymbolValue(attrType) : nl->SymbolValue(nl->First(attrType));
    }

};

struct DisplayNRel : DisplayFunction
{
  virtual void Display( ListExpr type, ListExpr value )
  {
    DisplayARel::DisplayRelation(type, value);
  }
};

struct DisplayInquiry: DisplayFunction{

  virtual void Display(ListExpr type, ListExpr value){

    if(!nl->HasLength(value,2)){
       cout << "invalid format for inquiry" << endl;
       cout << nl->ToString(value) << endl;
    }
    
    ListExpr InquiryType = nl->First(value);
    string TypeName = nl->SymbolValue(InquiryType);

    ListExpr v = nl->Second(value);
    if(TypeName=="databases") {
      cout << endl << "--------------------" << endl;
      cout << "Database(s)" << endl;
      cout << "--------------------" << endl;
      if(nl->ListLength(v)==0)
         cout << "none" << endl;
      while(!nl->IsEmpty(v)){
         cout << "  " << nl->SymbolValue(nl->First(v)) << endl;
         v = nl->Rest(v);
      }
      return;
    }
    if(TypeName=="algebras") {
      cout << endl << "--------------------" << endl;
      cout << "Algebra(s) " << endl;
      cout << "--------------------" << endl;
      if(nl->ListLength(v)==0)
        cout << "none" << endl;
      while(!nl->IsEmpty(v)) {
        cout << "  " << nl->SymbolValue(nl->First(v)) << endl;
        v = nl->Rest(v);
      }
      return;
    }
    if(TypeName=="types") {
       nl->WriteListExpr(v,cout);
       return;
    }
    if(TypeName=="objects") {
      ListExpr tmp = nl->Rest(v); // ignore the OBJECTS
      cout << " complete list " << endl;
      nl->WriteListExpr(v,cout);
      cout << endl << "---------------" << endl;
      if(! nl->IsEmpty(tmp)) {
        cout << " short list " << endl << endl;
        while(!nl->IsEmpty(tmp)) {
          cout << "  * " << nl->SymbolValue(nl->Second(nl->First(tmp)));
          cout << endl;
          tmp = nl->Rest(tmp);
        }
      }
      return;
    }
    if(TypeName=="constructors" || TypeName=="operators") {
      cout << endl << "--------------------" << endl;
      if(TypeName=="constructors")
         cout <<"Type Constructor(s)\n";
      else
         cout << "Operator(s)" << endl;
      cout << "--------------------" << endl;
      if(nl->IsEmpty(v)) {
         cout <<"  none " << endl;
      } else {
        ListExpr headerlist = v;
        int MaxLength = 0;
        int currentlength;


        try{

        while(!nl->IsEmpty(headerlist)) {
          ListExpr tmp = (nl->Second(nl->First(headerlist)));
          while(!nl->IsEmpty(tmp)) {
            currentlength = (nl->StringValue(nl->First(tmp))).length();
            tmp = nl->Rest(tmp);
            if(currentlength>MaxLength)
              MaxLength = currentlength;
          }
          headerlist = nl->Rest(headerlist);
        }

        while (!nl->IsEmpty( v )) {
          DisplayTTY::DisplayDescriptionLines( nl->First(v), MaxLength );
          v   = nl->Rest( v );
        }

        } catch(...){
            cerr << "Exception " << endl;
        }

      }



      return;
    }


    if(TypeName=="algebra") {
      string AlgebraName = nl->SymbolValue(nl->First(v));
      cout << endl << "-----------------------------------" << endl;
      cout << "Algebra : " << AlgebraName << endl;
      cout << "-----------------------------------" << endl;
      ListExpr Cs = nl->First(nl->Second(v));
      ListExpr Ops = nl->Second(nl->Second(v));
      // determine the headerlength
      ListExpr tmp1 = Cs;
      int maxLength=0;
      int len;
      while(!nl->IsEmpty(tmp1)) {
         ListExpr tmp2 = nl->Second(nl->First(tmp1));
         while(!nl->IsEmpty(tmp2)) {
           len = (nl->StringValue(nl->First(tmp2))).length();
           if(len>maxLength)
              maxLength=len;
           tmp2 = nl->Rest(tmp2);
         }
         tmp1 = nl->Rest(tmp1);
      }
      tmp1 = Ops;
      while(!nl->IsEmpty(tmp1)) {
         ListExpr tmp2 = nl->Second(nl->First(tmp1));
         while(!nl->IsEmpty(tmp2)) {
            len = (nl->StringValue(nl->First(tmp2))).length();
            if(len>maxLength)
               maxLength=len;
            tmp2 = nl->Rest(tmp2);
         }
         tmp1 = nl->Rest(tmp1);
      }

      cout << endl << "-------------------------" << endl;
      cout << "  "<< "Type Constructor(s)" << endl;
      cout << "-------------------------" << endl;
      if(nl->ListLength(Cs)==0)
        cout << "  none" << endl;
      while(!nl->IsEmpty(Cs)) {
        DisplayTTY::DisplayDescriptionLines(nl->First(Cs),maxLength);
        Cs = nl->Rest(Cs);
      }

      cout << endl << "-------------------------" << endl;
      cout << "  " << "Operator(s)" << endl;
      cout << "-------------------------" << endl;
      if(nl->ListLength(Ops)==0)
        cout << "  none" << endl;
      while(!nl->IsEmpty(Ops)) {
        DisplayTTY::DisplayDescriptionLines(nl->First(Ops),maxLength);
        Ops = nl->Rest(Ops);
      }
      return;
  }
  cout << "unknow inquiry type" << endl;
  nl->WriteListExpr(value,cout);
  }
};



struct DisplayInt : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value )
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else
    {
      cout << nl->IntValue( value );
    }
  }

};

struct DisplayLongInt : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value )
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else if(nl->AtomType(value)==IntType)
    {
      cout << nl->IntValue( value );
    } else if(nl->HasLength(value,2) ){
       ListExpr a1 = nl->First(value);
       ListExpr a2 = nl->Second(value);
       if( (nl->AtomType(a1)==IntType) &&
           (nl->AtomType(a2)==IntType)){
          int64_t v1 = nl->IntValue(a1);
          int64_t v2 = (uint32_t)nl->IntValue(a2);
          int64_t v = (v1<<32) | v2;
          cout << v;
       } else {
         cout << nl->ToString(value);
       }
    } else {
      cout << nl->ToString(value);
    }
  }

};


struct DisplayRational : DisplayFunction {

  virtual void Display( ListExpr type,  ListExpr value )
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
      return;
    }
    if(nl->HasLength(value,3)){ // sign included
       cout << nl->ToString(nl->First(value));
       value = nl->Rest(value);
    }
    cout << getUint64(nl->First(value));
    uint64_t d = getUint64(nl->Second(value));
    if(d!=1){
       cout << " / " << d;
    }
  }

  static uint64_t getUint64(ListExpr args){
     if(nl->AtomType(args)==IntType){
        uint32_t i = nl->IntValue(args);
        return i;
     } else if(nl->HasLength(args,2)){
         ListExpr i1 = nl->First(args);
         ListExpr i2 = nl->Second(args);
         if(nl->AtomType(i1)!=IntType || nl->AtomType(i2)!=IntType){
           cerr << "Invalid representation found "
                << __PRETTY_FUNCTION__ << endl;
           return 0;
         }
         uint32_t v1 = nl->IntValue(i1);
         uint32_t t2 = nl->IntValue(i2);
         uint64_t res = v1;
         res = (res << 32) | t2;
         return res;
     }
     cerr << "Invalid representation found " << __PRETTY_FUNCTION__ << endl;
     return 0;
  }


};


struct DisplayDRM : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value )
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else
    { // an INT value
      int v = nl->IntValue(value);
      int pos=1;
      for(int i=0;i<9;i++){
         if(i>0) cout << " ";
         cout << ((v&pos)>0?1:0);
         pos = pos << 1;
      }
    }
  }
};




struct DisplayReal : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value )
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else
    {
      cout.unsetf(ios_base::floatfield);
      double d = nl->RealValue( value );
      int p = min( static_cast<int>(ceil(log10(d))) + 10, 16 );
      cout << setprecision(p) << d;

      if ( RTFlag::isActive("TTY:Real:16digits") ) {
        cout << " (16digits = " << setprecision(16) << d << ")";
      }
    }
  }
};

struct DisplayBoolean : DisplayFunction {

  virtual void Display( ListExpr list, ListExpr value )
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else
    {
      if ( nl->BoolValue( value ) )
      {
        cout << "TRUE";
      }
      else
      {
        cout << "FALSE";
      }
    }
  }
};

struct DisplayString : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value )
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else
    {
      cout << nl->StringValue( value );
    }
  }
};


struct DisplayText : DisplayFunction {

  virtual void Display( ListExpr type,  ListExpr value )
  {

    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else
    {
      string printstr="";
      nl->Text2String(value, printstr);
      cout << wordWrap((size_t)0,  (size_t)DisplayTTY::maxIndent,
                       (size_t)(LINELENGTH - DisplayTTY::maxIndent),
                       printstr);
    }
  }
};

struct DisplayFun : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value )
  {
    cout << "Function type: ";
    nl->WriteListExpr( type, cout );
    cout << endl << "Function value: ";
    nl->WriteListExpr( value, cout );
    cout << endl;
  }
};

struct DisplayDate : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value)
  {
    if (nl->IsAtom(value) && nl->AtomType(value)==StringType)
      cout <<nl->StringValue(value);
    else
      throw runtime_error(stdErrMsg);
  }
};


struct DisplayBinfile : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value)
  {
    cout << "binary file";
  }

};

double DisplayTTY::GetNumeric(ListExpr value, bool &err)
{
  if(nl->AtomType(value)==IntType){
    err=false;
    return nl->IntValue(value);
  }
  if(nl->AtomType(value)==RealType){
    err=false;
    return nl->RealValue(value);
  }
  if(nl->AtomType(value)==NoAtom){
    int len = nl->ListLength(value);
    if(len!=5 && len!=6){
      err=true;
      return 0;
    }
    ListExpr F = nl->First(value);
    if(nl->AtomType(F)!=SymbolType){
      err=true;
      return 0;
    }
    if(nl->SymbolValue(F)!="rat"){
      err=true;
      return 0;
    }
    value = nl->Rest(value);
    double sign = 1.0;
    if(nl->ListLength(value)==5){  // with sign
      ListExpr SignList = nl->First(value);
      if(nl->AtomType(SignList)!=SymbolType){
        err=true;
        return 0;
      }
      string SignString = nl->SymbolValue(SignList);
      if(SignString=="-")
        sign = -1.0;
      else if(SignString=="+")
        sign = 1.0;
      else{
        err=true;
        return 0;
      }
      value= nl->Rest(value);
    }
    if(nl->AtomType(nl->First(value))==IntType &&
       nl->AtomType(nl->Second(value))==IntType &&
       nl->AtomType(nl->Third(value))==SymbolType &&
       nl->SymbolValue(nl->Third(value))=="/" &&
       nl->AtomType(nl->Fourth(value))==IntType){
      err=false;
      double intpart = nl->IntValue(nl->First(value));
      double numDecimal = nl->IntValue(nl->Second(value));
      double denomDecimal = nl->IntValue(nl->Fourth(value));
      if(denomDecimal==0){
        err=true;
        return 0;
      }
      double res1 = intpart*denomDecimal + numDecimal/denomDecimal;
      return sign*res1;
       } else{
         err = true;
         return 0;
       }
  }
  err=true;
  return 0;
}


struct DisplayXPoint : DisplayFunction {

  virtual void Display( ListExpr type,  ListExpr value)
  {
    if(nl->ListLength(value)!=2)
      throw runtime_error(stdErrMsg);
    else{
      bool err;
      double x = GetNumeric(nl->First(value),err);
      if(err){
        throw runtime_error(stdErrMsg);
      }
      double y = GetNumeric(nl->Second(value),err);
      if(err){
        throw runtime_error(stdErrMsg);
      }
      cout << "xpoint (" << x << "," << y << ")";
    }
  }
};

struct DisplayPoint : DisplayFunction {

virtual void Display( ListExpr type, ListExpr value)
{
  if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
      nl->SymbolValue( value ) == Symbol::UNDEFINED() )
  {
    cout << Symbol::UNDEFINED();
  }
  else if(nl->ListLength(value)!=2)
    throw runtime_error(stdErrMsg);
  else{
    bool err;
    double x = GetNumeric(nl->First(value),err);
    if(err){
      throw runtime_error(stdErrMsg);
    }
    double y = GetNumeric(nl->Second(value),err);
    if(err){
      throw runtime_error(stdErrMsg);
    }
    cout << "point: (" << x << "," << y << ")";
  }
}
};


struct DisplayTBTree : DisplayFunction {

virtual void Display( ListExpr type,  ListExpr value)
{
  if(nl->AtomType(value)!=TextType){
     throw runtime_error(" invalid representation of a tbtree ");
  } else {
    cout << nl->ToString(value);
  }
}
};


struct DisplayRect : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value)
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else if( nl->ListLength(value) != 4 )
      throw runtime_error(stdErrMsg);
    else
    {
      bool realValue;
      double coordValue[4];
      unsigned i = 0;
      ListExpr restList, firstVal;

      restList = value;
      do
      {
        firstVal = nl->First(restList);
        realValue = nl->AtomType( firstVal ) == RealType;
        if (realValue)
        {
          restList = nl->Rest(restList);
          coordValue[i] = nl->RealValue(firstVal);
          i++;
        }
      } while (i < 4 && realValue);

      if (realValue)
      {
        cout << "rect: ( (";
        for( unsigned i = 0; i < 2; i++ )
        {
          cout << coordValue[2*i];
          if( i < 2 - 1 )
            cout << ",";
        }
        cout << ") - (";
        for( unsigned i = 0; i < 2; i++ )
        {
          cout << coordValue[2*i+1];
          if( i < 2 - 1 )
            cout << ",";
        }
        cout << ") )";
      }
      else
      {
        throw runtime_error(stdErrMsg);
      }
    }
  }
};

/*
DisplayRect3

*/


struct DisplayRect3 : DisplayFunction {

  virtual void Display( ListExpr type,  ListExpr value)
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else if( nl->ListLength(value) != 6 )
      throw runtime_error(stdErrMsg);
    else
    {
      bool realValue;
      double coordValue[6];
      unsigned i = 0;
      ListExpr restList, firstVal;

      restList = value;
      do
      {
        firstVal = nl->First(restList);
        realValue = nl->AtomType( firstVal ) == RealType;
        if (realValue)
        {
          restList = nl->Rest(restList);
          coordValue[i] = nl->RealValue(firstVal);
          i++;
        }
      } while (i < 6 && realValue);

      if (realValue)
      {
        cout << "rect: ( (";
        for( unsigned i = 0; i < 3; i++ )
        {
          cout << coordValue[2*i];
          if( i < 3 - 1 )
            cout << ",";
        }
        cout << ") - (";
        for( unsigned i = 0; i < 3; i++ )
        {
          cout << coordValue[2*i+1];
          if( i < 3 - 1 )
            cout << ",";
        }
        cout << ") )";
      }
      else
      {
        throw runtime_error(stdErrMsg);
      }
    }
  }
};

/*
DisplayRect4

*/


struct DisplayRect4 : DisplayFunction {

  virtual void Display( ListExpr type,  ListExpr value)
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else if( nl->ListLength(value) != 8 )
      throw runtime_error(stdErrMsg);
    else
    {
      bool realValue;
      double coordValue[8];
      unsigned i = 0;
      ListExpr restList, firstVal;

      restList = value;
      do
      {
        firstVal = nl->First(restList);
        realValue = nl->AtomType( firstVal ) == RealType;
        if (realValue)
        {
          restList = nl->Rest(restList);
          coordValue[i] = nl->RealValue(firstVal);
          i++;
        }
      } while (i < 8 && realValue);

      if (realValue)
      {
        cout << "rect: ( (";
        for( unsigned i = 0; i < 4; i++ )
        {
          cout << coordValue[2*i];
          if( i < 4 - 1 )
            cout << ",";
        }
        cout << ") - (";
        for( unsigned i = 0; i < 4; i++ )
        {
          cout << coordValue[2*i+1];
          if( i < 4 - 1 )
            cout << ",";
        }
        cout << ") )";
      }
      else
      {
        throw runtime_error(stdErrMsg);
      }
    }
  }
};

/*
DisplayRect8

*/


struct DisplayRect8 : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value)
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else if( nl->ListLength(value) != 16 ) {
      throw runtime_error(stdErrMsg);
    }
    else
    {
      bool realValue;
      double coordValue[16];
      unsigned i = 0;
      ListExpr restList, firstVal;

      restList = value;
      do
      {
        firstVal = nl->First(restList);
        realValue = nl->AtomType( firstVal ) == RealType;
        if (realValue)
        {
          restList = nl->Rest(restList);
          coordValue[i] = nl->RealValue(firstVal);
          i++;
        }
      } while (i < 16 && realValue);

      if (realValue)
      {
        cout << "rect: ( (";
        for( unsigned i = 0; i < 8; i++ )
        {
          cout << coordValue[2*i];
          if( i < 8 - 1 )
            cout << ",";
        }
        cout << ") - (";
        for( unsigned i = 0; i < 8; i++ )
        {
          cout << coordValue[2*i+1];
          if( i < 8 - 1 )
            cout << ",";
        }
        cout << ") )";
      }
      else
      {
        throw runtime_error(stdErrMsg);
      }
    }
  }
};

/*
DisplayMP3

*/

struct DisplayMP3 : DisplayFunction {

  virtual void Display( ListExpr type,  ListExpr value)
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
      cout << Symbol::UNDEFINED();
    else
      cout << "mp3 file";
  }
};

/*
DisplayID3

*/
struct DisplayID3 : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value)
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else
    {
      cout << "ID3-Tag"<<endl << endl;
      cout << "Title   : " << nl->StringValue (nl->First (value)) <<endl;
      cout << "Author  : " << nl->StringValue (nl->Second (value)) << endl;
      cout << "Album   : " << nl->StringValue (nl->Third (value)) << endl;
      cout << "Year    : " << nl->IntValue (nl->Fourth (value)) << endl;
      cout << "Comment : " << nl->StringValue (nl->Fifth (value)) << endl;

      if (nl->ListLength(value) == 6)
      {
        cout << "Genre   : " << nl->StringValue (nl->Sixth (value)) << endl;
      }
      else
      {
        cout << "Track   : " << nl->IntValue (nl->Sixth (value)) << endl;
        cout << "Genre   : ";
        cout << nl->StringValue (nl->Sixth (nl->Rest (value))) << endl;
      }
    }
  }
};

/*
DisplayLyrics

*/

struct DisplayLyrics : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value)
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else
    {
      cout << "Lyrics"<<endl<<endl;
      int no = nl->ListLength (value) / 2;
      for (int i=1; i<=no; i++)
      {
        cout << "[" << nl->IntValue ( nl->First (value)) / 60 << ":";
        cout << nl->IntValue ( nl->First (value)) % 60 << "] ";
        cout << nl->StringValue (nl->Second (value));
        value = nl->Rest (nl->Rest (value));
      }
    }
  }
};

/*
DisplayMidi

*/
struct DisplayMidi  : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value)
  {
    int size = nl->IntValue(nl->Second(value));
    int noOfTracks = nl->IntValue(nl->Third(value));
    cout << "Midi: " << size << "bytes, ";
    cout << noOfTracks << " tracks";
  }
};

/*
DisplayArray

*/
struct DisplayArray : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value)
  {
    if(    (nl->AtomType(value) == SymbolType )
        && (nl->SymbolValue(value)==Symbol::UNDEFINED())){
       cout << Symbol::UNDEFINED(); 
    } else if(nl->ListLength(value)==0) {
      cout << "an empty array";
    }else{
      ListExpr AType = nl->Second(type);
      int No = 0;
      cout << "*************** BEGIN ARRAY ***************" << endl;
      ListExpr t = AType;
      while(nl->AtomType(t)!=SymbolType){
         t = nl->First(t);
      }
      while( !nl->IsEmpty(value)){
        cout << "--------------- Field No: ";
        cout << No++ << " ---------------" << endl;
        CallDisplayFunction( nl->SymbolValue(t), AType,nl->First(value));
        cout << endl;
        value = nl->Rest(value);
      }
      cout << "***************  END ARRAY  ***************";

    }
  }
};

/*
DisplayDArray

*/
struct DisplayDArray_old : DisplayFunction {

  virtual void Display( ListExpr type,  ListExpr value)
  {
    if(nl->ListLength(value) == 1)
      cout << "an empty darray";
    else{
      ListExpr AType = nl->Second(type);
      ListExpr t = AType;
      while(nl->AtomType(t)!=SymbolType){
        t = nl->First(t);
      }

      int No = 0;
      bool skipFirst = true;
      cout << "*************** BEGIN DARRAY ***************" << endl;
      if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() ) {
          cout << Symbol::UNDEFINED() << endl;
      } else {
          while( !nl->IsEmpty(value)){
            if (!skipFirst)
              {
                cout << "------------ DArray Index: ";
                cout << No++ << " ---------------" << endl;
                CallDisplayFunction( nl->SymbolValue(t), 
                                     AType,
                                     nl->First(value) );
                cout << endl;
              }
            else
              {
                cout << "---------------  Workers:  ---------------" << endl;
                ListExpr workers = nl->First(value);
                while (!nl -> IsEmpty(workers))
                  {
                    ListExpr curworker = nl -> First(workers);
                    cout <<  nl -> ToString(curworker);

                    cout << endl;
                    workers = nl -> Rest(workers);
                  }
                cout << endl;
                skipFirst = false;
              }
            value = nl->Rest(value);
          }
        }
      cout << "***************  END DARRAY  ***************";

    }
  }
};

/*
DisplayInstant

*/
struct DisplayInstant : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value )
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else if(nl->AtomType(value) ==RealType){
      cout << nl->RealValue(value);
    } else {
      cout << nl->StringValue( value );
    }
  }
};

/*
DisplayDuration

*/
struct DisplayDuration : DisplayFunction {

  virtual void Display( ListExpr type,  ListExpr value )
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else
    {
      bool written = false;
      if(nl->ListLength(value)==2){
        if( (nl->AtomType(nl->First(value))==IntType) &&
             (nl->AtomType(nl->Second(value))==IntType)){
          int dv  = nl->IntValue(nl->First(value));
          int msv = nl->IntValue(nl->Second(value));
          written = true;
          if( (dv==0) && (msv==0)){
            cout << "0ms";
          }else{
            if(dv!=0){
              cout << dv << " days";
              if(msv>0)
                cout <<" + ";
            }
            if(msv > 0) {
              int h = msv / 3600000;
              if(h!=0){
                 cout << h << " h ";
              }
              msv = msv % 3600000;
              int m = msv / 60000;
              if(h!=0 || m!=0){
                 cout << m << " m ";
              }
              msv = msv % 60000;
              int s = msv / 1000;
              if(h!=0 || m!=0 || s!=0){
                 cout << s << " s ";
              }
              msv = msv%1000;
              cout << msv <<" ms";
            }
          }

             }
      }
      if(!written){
        cout << "unknown list format for duration; using nested list output:";
        nl->WriteListExpr(value);
      }
    }
  }
};

/*
DisplayTid

*/
struct DisplayTid : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value )
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else
    {
      cout << nl->IntValue( value );
    }
  }
};

/*
DisplayHtml

*/
struct DisplayHtml : DisplayFunction {

  virtual void Display( ListExpr type,  ListExpr value)
  {
    if(nl->ListLength(value)!=3)
      throw runtime_error(stdErrMsg);
    else{
      string out;
      nl->WriteToString(out, type);
      cout << "Type: " << out << endl;

      cout << "Date of last modification: ";
      ListExpr First = nl->First(value);      //DateTime
      ListExpr Second = nl->Second(value);    //Text (FLOB)
      ListExpr Third = nl->Third(value);      //URL
      DisplayResult( nl->First(First), nl->Second(First) );
      DisplayResult( nl->First(Third), nl->Second(Third) );
      Base64 b;
      string text = nl->Text2String(Second);
      int sizeDecoded = b.sizeDecoded( text.size() );
      char *bytes = (char *)malloc( sizeDecoded );
      int result = b.decode( text, bytes );
      assert( result <= sizeDecoded );
      bytes[result] = 0;
      text = bytes;
      free( bytes );
      cout << "-------------Start of html content:" << endl;
      cout << text << endl;
      cout << "-------------End of html content:" << endl;
    }
  }
};
/*
DisplayPage

*/
struct DisplayPage : DisplayFunction {

  virtual void Display( ListExpr type,  ListExpr value)
  {
    if(nl->ListLength(value)<1)
      throw runtime_error(stdErrMsg);
    else{
      string out;
      nl->WriteToString(out, type);
      cout << "Type: " << out << endl;

      ListExpr First = nl->First(value);              //html
      nl->WriteToString(out, First);
      int nrOfEmb = nl->ListLength(value) - 1;
      DisplayResult( nl->First(First), nl->Second(First) );

      First = nl->Rest(value);
       //now lists of (url text)
      cout << "----Start of embedded objects" << endl;
      for( int ii=0; ii < nrOfEmb; ii++)
      {
        ListExpr emblist = nl->First(First);
        First = nl->Rest(First);

        if ( nl->ListLength( emblist ) == 3)
        {
          cout << endl << ii+1 << ". embedded object:" << endl;
          ListExpr embFirst = nl->First(emblist);         //url
          DisplayResult( nl->First(embFirst), nl->Second(embFirst) );
          cout << "The content is binary coded, no display here" << endl;
          string mime = nl->StringValue(nl->Third(emblist));
          cout << "Mimetype: " << mime << endl;
        }
        else
        {
          throw runtime_error(stdErrMsg);
        }
      }
      cout << "----End of embedded objects" << endl;
    }
  }
};

/*
DisplayUrl

*/
struct DisplayUrl : DisplayFunction {

  virtual void Display( ListExpr type,  ListExpr value)
  {
    if(nl->ListLength(value)!=3)
      throw runtime_error(stdErrMsg);
    else{
      string out;
      nl->WriteToString(out, type);
      cout << "Type: " << out << endl;

      ListExpr First = nl->First(value);              //protocol
      ListExpr Second = nl->Second(value);    //host
      ListExpr Third = nl->Third(value);              //path
      string prot = nl->StringValue(First);
      string host = nl->Text2String(Second);
      string path = nl->Text2String(Third);
      cout << "- Protocol: " << prot << endl;
      cout << "- Host: " << host << endl;
      cout << "- Path: " << path << endl;
    }
  }
};

/*
DisplayVertex

*/
struct DisplayVertex : DisplayFunction {

  virtual void Display( ListExpr type,  ListExpr value)
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else if(nl->ListLength(value)!=2)
      throw runtime_error(stdErrMsg);
    else{
      bool err=false;
      int key = (int) GetNumeric(nl->First(value),err);
      if(err){
        throw runtime_error(stdErrMsg);
      }
      if( nl->IsAtom( nl->Second(value) )  &&
          nl->SymbolValue(  nl->Second(value) ) == Symbol::UNDEFINED() )
      {
        stringstream info;
        info << "vertex " << key << ": no point defined" << endl;
        throw runtime_error(info.str());
      }
      double x = GetNumeric(nl->First(nl->Second(value)),err);
      if(err){
        throw runtime_error(stdErrMsg);
      }
      double y = GetNumeric(nl->Second(nl->Second(value)),err);
      if(err){
        throw runtime_error(stdErrMsg);
      }
      cout << "vertex "<<key<<": (" << x << "," << y << ")";
    }
  }
};
/*
DisplayEdge

*/
struct DisplayEdge : DisplayFunction {

  virtual void Display( ListExpr type,  ListExpr value)
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else if(nl->ListLength(value)!=3)
      throw runtime_error(stdErrMsg);
    else{
      bool err=false;
      int key1 = (int) GetNumeric(nl->First(value),err);
      if(err){
        throw runtime_error(stdErrMsg);
      }

      int key2 = (int) GetNumeric(nl->Second(value),err);
      if(err){
        throw runtime_error(stdErrMsg);
      }
      double cost = GetNumeric(nl->Third(value),err);
      if(err){
        throw runtime_error(stdErrMsg);
      }
      cout << "edge "<<key1<<"---" << cost << "---->" << key2;
    }
  }
};
/*
DisplayPath

*/
struct DisplayPath : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value)
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else if(nl->ListLength(value)!=1&&(nl->ListLength(value)-1)%2!=0)
      throw runtime_error(stdErrMsg);
    else{
      cout <<"*******************BEGIN PATH***************************"<<endl;
      bool err=false;
      int key = (int) GetNumeric(nl->First(nl->First(value)),err);
      if(err){
        throw runtime_error(stdErrMsg);
      }
      if( nl->IsAtom( nl->Second(nl->First(value)) )  &&
          nl->SymbolValue(nl->Second(nl->First(value)))==Symbol::UNDEFINED() )
      {
        cout << "vertex "<<key<<": no point defined";
      }
      else {
        double x = GetNumeric(nl->First(nl->Second(nl->First(value))),err);
        if(err){
          throw runtime_error(stdErrMsg);
        }
        double y = GetNumeric(nl->Second(nl->Second(nl->First(value))),err);
        if(err){
          throw runtime_error(stdErrMsg);
        }
        cout << "vertex "<<key<<": (" << x << "," << y << ")"<<endl;
      }
      value=nl->Rest(value);
      while (!nl->IsEmpty( value ))
      {

        double cost =  GetNumeric(nl->First(value),err);
        if(err){
          throw runtime_error(stdErrMsg);
        }
        cout <<"---"<<cost<<"--->"<<endl;

        int key = (int) GetNumeric(nl->First(nl->Second(value)),err);
        if(err){
          throw runtime_error(stdErrMsg);
        }
        if( nl->IsAtom( nl->Second(nl->Second(value)) )  &&
            nl->SymbolValue(  nl->Second(nl->Second(value)) )
                                                       == Symbol::UNDEFINED() )
        {
          cout << "vertex "<<key<<": no point defined";
        }
        else
        {
          double x = GetNumeric(nl->First(nl->Second(nl->Second(value))),err);
          if(err){
            throw runtime_error(stdErrMsg);
          }
          double y = GetNumeric(nl->Second(nl->Second(nl->Second(value))),err);
          if(err){
            throw runtime_error(stdErrMsg);
          }
          cout << "vertex "<<key<<": (" << x << "," << y << ")"<<endl;
        }
        value = nl->Rest(nl->Rest( value ));
      }
      cout <<"********************END PATH****************************"<<endl;
    }
  }
};

/*
DisplayGraph

*/
struct DisplayGraph : DisplayFunction {

  virtual void Display( ListExpr type,  ListExpr value)
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else if(nl->ListLength(value)!=2)
      cout << "Incorrect Data Format ";
    else{
      ListExpr vertices=nl->First(value);
      ListExpr edges=nl->Second(value);
      cout <<"*******************BEGIN GRAPH***************************"<<endl;
      bool err=false;
      cout <<"********************VERTICES*****************************"<<endl;
      while (!nl->IsEmpty( vertices ))
      {

        int key = (int) GetNumeric(nl->First(nl->First(vertices)),err);
        if(err){
          throw runtime_error(stdErrMsg);
        }
        if( nl->IsAtom( nl->Second(nl->First(vertices)) )  &&
            nl->SymbolValue(nl->Second(nl->First(vertices)))
                                                     ==Symbol::UNDEFINED() )
        {
          cout << key<<": no point defined"<<endl;;

        }
        else
        {
          double x =
             GetNumeric(nl->First(nl->Second(nl->First(vertices))),err);
          if(err){
            throw runtime_error(stdErrMsg);
          }
          double y =
             GetNumeric(nl->Second(nl->Second(nl->First(vertices))),err);
          if(err){
            throw runtime_error(stdErrMsg);
          }
          cout << key<<": (" << x << "," << y << ")"<<endl;
        }
        vertices = nl->Rest(vertices);

      }
      cout <<"**********************EDGES******************************"<<endl;
      while (!nl->IsEmpty( edges ))
      {
        if(nl->ListLength(nl->First(edges))!=3)
          throw runtime_error(stdErrMsg);
        else{
          bool err=false;
          int key1 = (int)GetNumeric(nl->First(nl->First(edges)),err);
          if(err){
            throw runtime_error(stdErrMsg);
          }

          int key2 = (int)GetNumeric(nl->Second(nl->First(edges)),err);
          if(err){
            throw runtime_error(stdErrMsg);
          }
          double cost = GetNumeric(nl->Third(nl->First(edges)),err);
          if(err){
            throw runtime_error(stdErrMsg);
          }
          cout << key1<<"---" << cost << "---->" << key2<<endl;
        }
        edges = nl->Rest(edges);

      }
      cout <<"********************END GRAPH****************************"<<endl;
    }
  }
};



/*
DisplayHistogram2d

*/

struct DisplayHistogram2d : DisplayFunction {

  virtual void Display( ListExpr type,  ListExpr value)
  {
    cout << setprecision(16);
    bool err;
    if(   nl->AtomType(value)==SymbolType 
       && nl->IsEqual(value, Symbol::UNDEFINED()))  {
      cout << Symbol::UNDEFINED();
    }

    else if(!nl->HasLength(value,3) || 
                (nl->ListLength(nl->First(value))-1) 
             * (nl->ListLength(nl->Second(value))-1)
             != nl->ListLength(nl->Third(value))){
      err = true;
      throw runtime_error(stdErrMsg);
      nl->WriteListExpr(value, cout);
    } else {

      ListExpr rangesX = nl->First(value);
      ListExpr rangesY = nl->Second(value);
      ListExpr bins = nl->Third(value);


      // determine Maximum for scale
      double maxRangeY = 0;
      double minRange = 1e300;
      double last = GetNumeric(nl->First(rangesY), err);
      double d, dist;
      rangesY = nl->Rest(rangesY);
      while(!nl->IsEmpty(rangesY)) {
        ListExpr elem = nl->First(rangesY);
        d = GetNumeric(elem, err);
        dist = d - last;
        maxRangeY = maxRangeY > dist ? maxRangeY : dist;
        minRange = minRange < dist ? minRange : dist;
        last = d;
        rangesY = nl->Rest(rangesY);
      }
      rangesY = nl->Second(value); 

      // Maximum ermitteln, zur Skalierung
      double maxBin = 1;
      while (!nl->IsEmpty(bins) ) {
        ListExpr elem = nl->First(bins);
        d = GetNumeric(elem, err);
        maxBin = maxBin > d ? maxBin : d;
        bins = nl->Rest(bins);
      }
      bins = nl->Third(value);

      cout << endl;
      cout << "HISTOGRAM2D:" << endl;
      cout<< endl;

      double _rangeX = GetNumeric(nl->First(rangesX), err);
      cout << "************* RangeX: "<< _rangeX;
      cout << " **************"<< endl;
      rangesX = nl->Rest(rangesX); //paints first rangeX

      int height;
      int width;

      while ( !nl->IsEmpty(rangesX)) {
        double rangeY = GetNumeric(nl->First(rangesY), err);
        cout << "y: " << rangeY << endl;
        rangesY = nl->Rest(rangesY); //paints first rangeY

        double rangeX = GetNumeric(nl->First(rangesX), err);

        while ( !nl->IsEmpty(rangesY)) {
          double lastRange = rangeY;
          rangeY = GetNumeric(nl->First(rangesY), err);
          double bin = GetNumeric(nl->First(bins), err);

          // only scale, if bin greater than 80
          if (maxBin <= 80)
          {
            height = static_cast<int>(bin+1);
          }
          else
          {
            height =
              static_cast<int>(
                  (bin+1)/maxBin*80.0/(rangeY-lastRange)*minRange );
          }
          string space = "";

          for (int i = 0; i < height; ++i)
          {
            cout << "_";
            space += " ";
          }

          cout << endl;

          string binStr = "";

          stringstream ss(stringstream::in | stringstream::out);
          ss << bin;
          ss >> binStr;

          int numLen = binStr.length();

          width = static_cast<int>((rangeY-lastRange)*7.0/maxRangeY);

          for (int i = 0; i < width - 1; i++)
          {
            if (i == 0)
              if (numLen < static_cast<int>(space.length()))
                cout << bin << space.substr(numLen) << "|" << endl;
            else
              cout << space << "|" << bin << endl;
            else
              cout << space << "|" << endl;
          }

          if (width < 2)
            if (numLen < static_cast<int>(space.length()))
              cout << bin << space.substr(numLen) << "|" << endl;
          else
            cout << space << "|" << bin << endl;
          else
            cout << space << "|" << endl;

          for (int i = 0; i < height; ++i)
          {
            cout << "_";
          }
          cout << "|" << (bin/(rangeY-lastRange)) << endl;
          cout << "y: "<< rangeY << endl;

          rangesY = nl->Rest(rangesY);
          bins = nl->Rest(bins);
        } // while ( !(rangesY.isEmpty()))

        rangesY = nl->Second(value);
        cout << "*********** RangeX: " << rangeX;
        cout << " ***************" << endl;
        rangesX = nl->Rest(rangesX);
      } // while ( !(rangesX.isEmpty()))
    }
    cout << endl;
  }
};


/*
DisplayHistogram1d

*/

struct DisplayHistogram1d : DisplayFunction {

  virtual void Display( ListExpr type,  ListExpr value )
  {
    cout << setprecision(16);
    bool err;
    if (    nl->AtomType(value)==SymbolType 
         && nl->IsEqual(value, Symbol::UNDEFINED())){ 
      cout << Symbol::UNDEFINED();
    } else if ( !nl->HasLength(value,2) ||
                nl->ListLength(nl->First(value)) 
               != (nl->ListLength(nl->Second(value)) + 1)) {
      err = true;
      throw runtime_error(stdErrMsg);
      nl->WriteListExpr(value, cout);
    } else {
      ListExpr ranges = nl->First(value);
      ListExpr bins = nl->Second(value);

      // Maximum ermitteln, zur Skalierung
      double maxRange = 0;
      double minRange = 1e300;
      double last = GetNumeric(nl->First(ranges), err);
      double d, dist;
      ranges = nl->Rest(ranges);
      while(!nl->IsEmpty(ranges)) {
        ListExpr elem = nl->First(ranges);
        d = GetNumeric(elem, err);
        dist = d - last;
        maxRange = maxRange > dist ? maxRange : dist;
        minRange = minRange < dist ? minRange : dist;
        last = d;
        ranges = nl->Rest(ranges); 
      }
      ranges = nl->First(value);

      // Maximum ermitteln, zur Skalierung
      double maxBin = 1;
      //double d;
      while(!nl->IsEmpty(bins)) {
        ListExpr elem = nl->First(bins);
        d = GetNumeric(elem, err);
        maxBin = maxBin > d ? maxBin : d;
        bins = nl->Rest(bins);
      }
      bins = nl->Second(value);

      cout << endl;
      cout << "HISTOGRAM:" << endl;
      cout<< endl;
      double range = GetNumeric(nl->First(ranges), err);
      cout << range << endl;
      ranges = nl->Rest(ranges);
      int height;
      int width;
      while ( !nl->IsEmpty(ranges)) {
        double lastRange = range;
        range = GetNumeric(nl->First(ranges), err);
        double bin = GetNumeric(nl->First(bins), err);

       // only scale, if bin greater than 80
        if (maxBin <= 80)
        {
          height = static_cast<int>(bin+1);
        }
        else {
          height = static_cast<int>
              ((bin+1)/maxBin*80.0/(range-lastRange)*minRange);
        }
        string space = "";

        for (int i = 0; i < height;++i)
        {
          cout << "_";
          space += " ";
        }

        cout << endl;

        string binStr = "";

        stringstream ss(stringstream::in | stringstream::out);
        ss << bin;
        ss >> binStr;

        int numLen = binStr.length();

        width =
            static_cast<int>((range-lastRange)*7.0/maxRange);

        for (int i = 0; i < width - 1;i++)
        {
          if (i == 0)
            if (numLen < static_cast<int>(space.length()))
              cout << bin << space.substr(numLen) << "|" << endl;
          else
            cout << space << "|" << bin << endl;
          else
            cout << space << "|" << endl;
        }

        if (width < 2)
          if (numLen < static_cast<int>(space.length()))
            cout << bin << space.substr(numLen) << "|" << endl;
        else
          cout << space << "|" << bin << endl;
        else
          cout << space << "|" << endl;

        for (int i = 0; i < height;++i)
        {
          cout << "_";
        }
        cout << "|" << bin/(range-lastRange) << endl;
        cout << range << endl;

        ranges = nl->Rest(ranges);
        bins = nl->Rest(bins);
      }

    }
    cout << endl;
  }
};




/*
DisplayPosition

*/
struct DisplayPosition : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value )
  {
    string agents[ 64 ];

    if ( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
         nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else if ( nl->ListLength( value ) != 2 )
      throw runtime_error(stdErrMsg);
    else
    {
      ListExpr rowList, agent;
      ListExpr moveNoExpr = nl->First( value );
      ListExpr agentList = nl->Second( value );

      int moveNumber;
      if ( nl->IsAtom( moveNoExpr ) && nl->AtomType( moveNoExpr ) == IntType )
        moveNumber = nl->IntValue( moveNoExpr );
      else
      {
        throw runtime_error(stdErrMsg);
      }

      if ( nl->ListLength( agentList ) != 8 )
        throw runtime_error(stdErrMsg);
      else
      {
        int row = 7;
        while ( !nl->IsEmpty( agentList ) )
        {
          int file = 0;
          rowList = nl->First( agentList );
          agentList = nl->Rest( agentList );

          if ( nl->ListLength( rowList ) != 8 )
            throw runtime_error(stdErrMsg);
          else
          {
            while ( !nl->IsEmpty( rowList ) )
            {
              agent = nl->First( rowList );
              rowList = nl->Rest( rowList );
              if ( nl->IsAtom( agent )
                   && nl->AtomType( agent ) == StringType )
              {
                agents[ ( row * 8 ) + file ] = nl->StringValue( agent );
              }
              else
              {
                throw runtime_error(stdErrMsg);
              }
              file++;
            }
            row--;
          }
        }

        cout << "Position: (after move number "
            << moveNumber << ")" << endl << endl;
        cout << "        a b c d e f g h" << endl;
        cout << "       ----------------" << endl;
        for ( int row = 7; row >= 0; row-- )
        {
          cout << "      " << row + 1 << "|";
          for ( int file = 0; file < 8; file++ )
          {
            cout << agents[ ( row * 8 ) + file ] << " ";
          }
          cout << endl;
        }
        cout << endl;
      }
    }
  }
};

/*
DisplayMove

*/

struct DisplayMove : DisplayFunction {

  virtual void Display( ListExpr type,  ListExpr value )
  {
    if ( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
         nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
      cout << Symbol::UNDEFINED();
    }
    else if ( nl->ListLength( value ) != 8 )
      throw runtime_error(stdErrMsg);
    else
    {
      int moveNumber;
      string agent, captured, startfile, endfile;
      int startrow, endrow;
      bool check;
      ListExpr current = nl->First( value );
      ListExpr rest = nl->Rest( value );

      // read moveNumber
      if ( nl->IsAtom( current ) && nl->AtomType( current ) == IntType )
        moveNumber = nl->IntValue( current );
      else
      {
        throw runtime_error(stdErrMsg);
      }

      // read agent
      current = nl->First( rest );
      rest = nl->Rest( rest );
      if ( nl->IsAtom( current ) && nl->AtomType( current ) == StringType )
        agent = nl->StringValue( current );
      else
      {
        throw runtime_error(stdErrMsg);
      }

      // read captured
      current = nl->First( rest );
      rest = nl->Rest( rest );
      if ( nl->IsAtom( current ) && nl->AtomType( current ) == StringType )
        captured = nl->StringValue( current );
      else
      {
        throw runtime_error(stdErrMsg);
      }

      // read startfile
      current = nl->First( rest );
      rest = nl->Rest( rest );
      if ( nl->IsAtom( current ) && nl->AtomType( current ) == StringType )
        startfile = nl->StringValue( current );
      else
      {
        throw runtime_error(stdErrMsg);
      }
      // read startrow
      current = nl->First( rest );
      rest = nl->Rest( rest );
      if ( nl->IsAtom( current ) && nl->AtomType( current ) == IntType )
        startrow = nl->IntValue( current );
      else
      {
        throw runtime_error(stdErrMsg);
      }

      // read endfile
      current = nl->First( rest );
      rest = nl->Rest( rest );
      if ( nl->IsAtom( current ) && nl->AtomType( current ) == StringType )
        endfile = nl->StringValue( current );
      else
      {
        throw runtime_error(stdErrMsg);
      }

      // read endrow
      current = nl->First( rest );
      rest = nl->Rest( rest );
      if ( nl->IsAtom( current ) && nl->AtomType( current ) == IntType )
        endrow = nl->IntValue( current );
      else
      {
        throw runtime_error(stdErrMsg);
      }

      // read check
      current = nl->First( rest );
      rest = nl->Rest( rest );
      if ( nl->IsAtom( current ) && nl->AtomType( current ) == BoolType )
        check = nl->BoolValue( current );
      else
      {
        throw runtime_error(stdErrMsg);
      }

      cout << "Move number: " << moveNumber;
      if ( check )
        cout << " (Check!)" << endl;
      else
        cout << endl;

      cout << agent << " moves from "
          << startfile << startrow << " to " << endfile << endrow;

      if ( ( captured != "none" ) && ( captured != "None" ) )
        cout << " and captures " << captured << endl;
      else
        cout << endl;


      cout << endl;
    }
  }

};

/*
Chess Algebra 08/09


*/

struct DisplayMoveB  : DisplayFunction {

  virtual void Display( ListExpr, ListExpr value )
  {
      cout << nl->ToString( value );
  }
};


struct DisplayPositionB  : DisplayFunction {

  virtual void Display( ListExpr, ListExpr value )
  {
    struct mapper
    {
        map<string, char> m_;
        mapper()
        {
            typedef pair<string, char> T;
            T v[16] = { T("none", ' '), T("pawn", 'p'), T("Pawn", 'P'),
            T("knight", 'n'), T("Knight", 'N'), T("bishop", 'b'),
            T("Bishop", 'B'), T("rook", 'r'), T("Rook", 'R'),
            T("queen", 'q'), T("Queen", 'Q'), T("king", 'k'),
            T("King", 'K') };
            for( int i = 0; i < 16; ++i )
                m_.insert( v[i] );
        }
    };
    static mapper mapper_;

    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
        cout << Symbol::UNDEFINED();
        return;
    }

    try {
        if ( !nl->IsAtom( nl->First(value) )
            || nl->AtomType( nl->First(value) )!= IntType )
            throw runtime_error( "Expect int as first arg" );
        cout << "\nMove number: " << nl->IntValue( nl->First(value) ) << ", ";

        if ( nl->IsAtom( nl->Second(value) ) )
            throw runtime_error( "Expect list as second arg" );
        ListExpr fields = nl->Second(value);

        if ( !nl->IsAtom( nl->Third(value) )
            || nl->AtomType( nl->Third(value) )!= IntType )
            throw runtime_error( "Expect int as third arg" );
        cout << "Turn: " << ( nl->IntValue( nl->Third(value) ) ?
            "white" : "black" ) << ", ";

        if ( !nl->IsAtom( nl->Fourth(value) )
            || nl->AtomType( nl->Fourth(value) )!= IntType )
            throw runtime_error( "Expect int as fourth arg" );
        int state = nl->IntValue( nl->Fourth(value) );
        cout << "Enpassant file: ";
        if ( state & 16 )
            cout << char( (state >> 5) + 'a') << "\n";
        else
            cout << "None\n";
        cout << "White castling[ long: "
           << ( (state & 8) ? "yes" : "no" )
           << ",  short: "
           << ( (state & 2) ? "yes" : "no" )
           << " ]\n";
        cout << "Black castling[ long: "
           << ( (state & 4) ? "yes" : "no" )
           << ",  short: "
           << ( (state & 1) ? "yes" : "no" )
           << " ]\n\n";

        cout << "    _________________\n";
        for ( int row = 8; row >= 1; --row )
        {
            if ( 8 != nl->ListLength( fields ) )
                throw runtime_error( "Expect 8-elem. list as second arg" );

            ListExpr r = nl->Nth( row, fields );
            if ( 8 != nl->ListLength( r ) )
                throw runtime_error( "Expect 8-elem. list as sublist arg" );
            cout << " " << row << " | ";
            for ( int file = 1; file <= 8; ++file )
            {
                ListExpr f = nl->Nth( file, r );
                if ( !nl->IsAtom( f ) || nl->AtomType( f )!= StringType )
                    throw runtime_error( "Expect string as agent arg" );
                string agent = nl->StringValue( f );
                map<string, char>::const_iterator it = mapper_.m_.find(agent);
                if ( it == mapper_.m_.end() )
                    cout  << 'u' << " ";
                else
                    cout  << it->second << " ";
            }
            cout << "|\n";
        }
        cout << "    -----------------\n     a b c d e f g h\n\n";
    }
    catch(const exception& e){

        stringstream info;
        info << "Incorrect Data Format: " << e.what() << "\n"
             << nl->ToString( value ) << endl;
        throw runtime_error(info.str());
    }
  }
};


struct DisplayField : DisplayFunction {

  virtual void Display( ListExpr,ListExpr value )
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
        cout << Symbol::UNDEFINED();
        return;
    }

    if ( ! nl->IsAtom( value ) || nl->AtomType( value ) != StringType )
    {
        stringstream msg;
        msg << "Incorrect Data Format: " << nl->ToString( value ) << endl;
        throw runtime_error(msg.str());
    }
    cout << nl->StringValue( value );
  }
};


struct DisplayPiece  : DisplayFunction {

  virtual void Display( ListExpr, ListExpr value )
  {
    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
        cout << Symbol::UNDEFINED();
        return;
    }

    if ( ! nl->IsAtom( value ) || nl->AtomType( value ) != StringType )
    {
        stringstream msg;
        msg << "Incorrect Data Format: " << nl->ToString( value ) << endl;
        throw runtime_error(msg.str());
    }
    cout << "Piece: " << nl->StringValue( value );
  }
};


struct DisplayMaterial : DisplayFunction {

  virtual void Display( ListExpr, ListExpr value )
  {
    stringstream msg;

    if( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType &&
        nl->SymbolValue( value ) == Symbol::UNDEFINED() )
    {
        cout << Symbol::UNDEFINED();
        return;
    }

    if ( 12 != nl->ListLength( value ) )
    {
        msg << "Incorrect Data Format, length != 12: "
            << nl->ToString( value ) << endl;
        throw runtime_error(msg.str());
    }

    int mat[12];
    for( int i = 0; i < 12; ++i )
    {
        ListExpr e = nl->Nth( i + 1, value );
        if ( nl->IsAtom( e ) && nl->AtomType( e ) == IntType )
            mat[i] = nl->IntValue( e );
        else
        {
            msg << "Element " << i << " is not an int. "
                << nl->ToString( value ) << endl;
            throw runtime_error(msg.str());
        }
    }
    cout << "Material: [ ";
    copy( mat, mat + 11, ostream_iterator< int >( cout, ", " ) );
    cout << mat[11] << " ]\n";
  }
};


/*
DisplayVector

*/
struct DisplayVector : DisplayFunction {
  virtual void Display( ListExpr type, ListExpr value)
  {
    if(nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType
       && nl->SymbolValue( value ) == Symbol::UNDEFINED()) {
      cout << "Undefined vector";
    } else{
      ListExpr AType = nl->Second(type);
      ListExpr tn = AType;
      while(nl->AtomType(tn)!=SymbolType){
        tn = nl->First(tn);
      }
      int No = 0;
      cout << "*************** BEGIN VECTOR ***************" << endl;
      while( !nl->IsEmpty(value)){
        cout << "--------------- Elem No: ";
        cout << No++ << " ---------------" << endl;
        CallDisplayFunction(nl->SymbolValue(tn), AType, nl->First(value));
        cout << endl;
        value = nl->Rest(value);
      }
      cout << "***************  END VECTOR  ***************";
    }
  }
};

/*
DisplaySet

*/
struct DisplaySet : DisplayFunction {

  virtual void Display( ListExpr type,  ListExpr value)
  {

    if(nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType
       && nl->SymbolValue( value ) == Symbol::UNDEFINED()) {
      cout << "Undefined set";
    } else{
      ListExpr AType = nl->Second(type);
      ListExpr tn = AType;
      while(nl->AtomType(tn)!=SymbolType){
        tn = nl->First(tn);
      }
      int No = 0;
      cout << "*************** BEGIN SET ***************" << endl;
      while( !nl->IsEmpty(value)){
        cout << "--------------- Elem No: ";
        cout << No++ << " ---------------" << endl;
        CallDisplayFunction(nl->SymbolValue(tn), AType, nl->First(value));
        cout << endl;
        value = nl->Rest(value);
      }
      cout << "***************  END SET  ***************";

       }
  }
};

/*
DisplaySet

*/
struct DisplayMultiset : DisplayFunction {

  virtual void Display( ListExpr type,  ListExpr value)
  {

    if(nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType
       && nl->SymbolValue( value ) == Symbol::UNDEFINED()) {
      cout << "Undefined multiset";
    } else{
      ListExpr AType = nl->Second(type);
      ListExpr tn = AType;
      while(nl->AtomType(tn)!=SymbolType){
        tn = nl->First(tn);
      }
      int No = 0;
      cout << "*************** BEGIN MULTISET ***************" << endl;
      while( !nl->IsEmpty(value)){
        cout << "--------------- Elem No: ";
        cout << No++ << " ---------------" << endl;
        CallDisplayFunction(nl->SymbolValue(tn), AType, 
                            nl->First(nl->First(value)));
        int times = nl->IntValue(nl->Second(nl->First(value)));
        cout << " (contained " << times << " times)."<< endl;
        value = nl->Rest(value);
      }
      cout << "***************  END MULTISET  ***************";
  }
}
};



struct DisplayCellgrid2D : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value )
  {
    if(nl->IsEqual(value,Symbol::UNDEFINED())){
        cout << Symbol::UNDEFINED();
    } else {
      cout << "[ x0 = " << nl->RealValue(nl->First(value))
           << ", y0 = " << nl->RealValue(nl->Second(value))
           << ", wx = " << nl->RealValue(nl->Third(value))
           << ", wy = " << nl->RealValue(nl->Fourth(value))
           << ", nx = " << nl->IntValue(nl->Fifth(value))
           << "]";
    }

  }

};

struct DisplayCellgrid3D : DisplayFunction {

  virtual void Display( ListExpr type, ListExpr value )
  {
    if(nl->IsEqual(value,Symbol::UNDEFINED())){
        cout << Symbol::UNDEFINED();
    } else {
      cout << "[ x0 = " << nl->RealValue(nl->First(value))
           << ", y0 = " << nl->RealValue(nl->Second(value))
           << ", z0 = " << nl->RealValue(nl->Third(value))
           << ", wx = " << nl->RealValue(nl->Fourth(value))
           << ", wy = " << nl->RealValue(nl->Fifth(value))
           << ", wz = " << nl->RealValue(nl->Sixth(value))
           << ", nx = " << nl->IntValue(nl->Nth(7, value))
           << ", ny = " << nl->IntValue(nl->Nth(8, value))
           << "]";
    }
  }

};

/*
Displayfunctions for JNetAlgebra

*/
struct DisplayJDirection : DisplayFunction {
  virtual void Display(ListExpr type,  ListExpr value)
  {
    cout << nl->ToString(nl->First(value)) << " direction";
  }
};

struct DisplayRouteLocation : DisplayFunction {
  virtual void Display(ListExpr type, ListExpr value)
  {
    if (nl->IsEqual(value, Symbol::UNDEFINED()))
      cout << Symbol::UNDEFINED() << endl;
    else
    {
      ListExpr subtype = nl->TheEmptyList();
      nl->ReadFromString("jdirection", subtype);
      cout<< "road " << nl->IntValue(nl->First(value))
          << " position " << nl->RealValue(nl->Second(value))
          << " reachable from ";
      DisplayTTY::GetInstance().DisplayResult(subtype, nl->Third(value));
    }
  }
};

struct DisplayJRouteInterval : DisplayFunction {
  virtual void Display(ListExpr type,  ListExpr value)
  {
    if (nl->IsEqual(value, Symbol::UNDEFINED()))
      cout << Symbol::UNDEFINED() << endl;
    else
    {
      ListExpr subtype = nl->TheEmptyList();
      nl->ReadFromString("jdirection", subtype);
      cout<< "road " << nl->IntValue(nl->First(value))
      << " from " << nl->RealValue(nl->Second(value))
      << " to " << nl->RealValue(nl->Third(value))
      << " direction ";
      DisplayTTY::GetInstance().DisplayResult(subtype, nl->Fourth(value));
    }
  }
};

struct DisplayJListInt : DisplayFunction {
  virtual void Display(ListExpr type, ListExpr value)
  {
    cout << "List of int: ";
    if (nl->IsEqual(value, Symbol::UNDEFINED()))
      cout << Symbol::UNDEFINED() << endl;
    else
    {
      cout << endl;
      ListExpr rest = value;
      ListExpr first = nl->TheEmptyList();
      while( !nl->IsEmpty( rest ) )
      {
        first = nl->First( rest );
        rest = nl->Rest( rest );
        if (nl->IntAtom(first))
          cout << nl->IntValue(first) << " ";
        else
          cout << Symbol::UNDEFINED() << endl;
      }
      cout << endl;
      cout << "End of list." << endl;
    }
  }
};


struct DisplayJListRLoc : DisplayFunction {
  virtual void Display(ListExpr type, ListExpr value)
  {
    cout << "List of route locations: ";
    if (nl->IsEqual(value, Symbol::UNDEFINED()))
      cout << Symbol::UNDEFINED() << endl;
    else
    {
      cout << endl;
      ListExpr subtype = nl->TheEmptyList();
      nl->ReadFromString("rloc", subtype);
      ListExpr rest = value;
      while( !nl->IsEmpty(rest))
      {
        DisplayTTY::GetInstance().DisplayResult(subtype, nl->First(rest));
        rest = nl->Rest(rest);
      }
      cout << "End of list." << endl;
    }
  }
};


struct DisplayJListRInt : DisplayFunction {
  virtual void Display(ListExpr type,  ListExpr value)
  {
    cout << "List of route intervals: ";
    if (nl->IsEqual(value, Symbol::UNDEFINED()))
      cout << Symbol::UNDEFINED() << endl;
    else
    {
      cout << endl;
      ListExpr subtype = nl->TheEmptyList();
      nl->ReadFromString("jrint", subtype);
      ListExpr rest = value;
      while( !nl->IsEmpty(rest))
      {
        DisplayTTY::GetInstance().DisplayResult(subtype, nl->First(rest));
        rest = nl->Rest(rest);
      }
      cout << "End of list." << endl;
    }
  }
};

struct DisplayNDG : DisplayFunction {
  virtual void Display(ListExpr type, ListExpr value)
  {
    if (nl->IsEqual(value, Symbol::UNDEFINED()))
      cout << Symbol::UNDEFINED() << endl;
    else
    {
      cout << "From junction  " << nl->IntValue(nl->First(value))
           << " to junction " << nl->IntValue(nl->Second(value))
           << " via section " << nl->IntValue(nl->Fourth(value))
           << " via junction " << nl->IntValue(nl->Third(value))
           << " network distance " << nl->RealValue(nl->Fifth(value))
           << endl;
    }
  }
};

struct DisplayJListNDG : DisplayFunction {
  virtual void Display(ListExpr type, ListExpr value)
  {
    cout << "List of NetDistanceGroups: ";
    if (nl->IsEqual(value, Symbol::UNDEFINED()))
      cout << Symbol::UNDEFINED() << endl;
    else
    {
      cout << endl;
      ListExpr subtype = nl->TheEmptyList();
      nl->ReadFromString("ndg", subtype);
      ListExpr rest = value;
      while( !nl->IsEmpty(rest))
      {
        DisplayTTY::GetInstance().DisplayResult(subtype, nl->First(rest));
        rest = nl->Rest(rest);
      }
      cout << "End of list." << endl;
    }
  }
};

struct DisplayJPoint : DisplayFunction {
  virtual void Display(ListExpr type, ListExpr value)
  {
    if (nl->IsEqual(value, Symbol::UNDEFINED()))
      cout << Symbol::UNDEFINED() << endl;
    else
    {
      cout << "In network " << nl->StringValue(nl->First(value));
      cout << " at ";

      ListExpr subtype = nl->TheEmptyList();
      nl->ReadFromString("rloc", subtype);
      DisplayTTY::GetInstance().DisplayResult(subtype, nl->Second(value));
    }
  }
};

struct DisplayJLine : DisplayFunction {
  virtual void Display(ListExpr type, ListExpr value)
  {
    if (nl->IsEqual(value, Symbol::UNDEFINED()))
      cout << Symbol::UNDEFINED() << endl;
    else
    {
      cout << "In network " << nl->StringValue(nl->First(value))
           << " at: " << endl;

      ListExpr subtype = nl->TheEmptyList();
      nl->ReadFromString("jrint", subtype);

      ListExpr rest = nl->Second(value);
      while (!nl->IsEmpty(rest))
      {
        DisplayTTY::GetInstance().DisplayResult(subtype, nl->First(rest));
        rest = nl->Rest(rest);
      }
    }
  }
};

struct DisplayJPoints : DisplayFunction{
  virtual void Display(ListExpr type,  ListExpr value)
  {
    if (nl->IsEqual(value, Symbol::UNDEFINED()))
      cout << Symbol::UNDEFINED() << endl;
    else
    {
      ListExpr nid = nl->First(value);

      cout << "Positions in network " << nl->ToString(nid)
           << ": " << endl;

      ListExpr subtype = nl->TheEmptyList();
      nl->ReadFromString("rloc", subtype);
      ListExpr rest = nl->Second(value);
      while (!nl->IsEmpty(rest))
      {
        DisplayTTY::GetInstance().DisplayResult(subtype, nl->First(rest));
        rest = nl->Rest(rest);
      }
    }
  }
};

struct DisplayJNetwork : DisplayFunction {
  virtual void Display(ListExpr type, ListExpr value)
  {
    ListExpr subtype = nl->TheEmptyList();
    cout << "JNetwork: ";

    if (nl->ListLength(value) == 1 && nl->IsAtom(value))
      cout << Symbol::UNDEFINED() << endl;
    else
    {
      ListExpr nid = nl->First(value);
      cout << nl->ToString(nid) << endl;
      cout << "tolerance: " << nl->RealValue(nl->Second(value)) << endl;
      cout << "Junctions: " << endl;
      ListExpr rest = nl->Third(value);
      ListExpr first = nl->TheEmptyList();

      while (!nl->IsEmpty(rest))
      {
        first = nl->First(rest);
        rest = nl->Rest(rest);
        cout << "junction with id " << nl->IntValue(nl->First(first))
             << " at position: " ;
        nl->ReadFromString("point", subtype);
        DisplayTTY::GetInstance().DisplayResult(subtype, nl->Second(first));

        nl->ReadFromString("listrloc", subtype);
        cout << "route positions of this junction: " << endl;
        DisplayTTY::GetInstance().DisplayResult(subtype, nl->Third(first));

        nl->ReadFromString("listint", subtype);
        cout << "Incoming sections of this junction: " << endl;
        DisplayTTY::GetInstance().DisplayResult(subtype, nl->Fourth(first));

        nl->ReadFromString("listint", subtype);
        cout << "Outgoing sections from this junction: " << endl;
        DisplayTTY::GetInstance().DisplayResult(subtype, nl->Fifth(first));
      }
      cout << endl;

      cout << "Sections: " << endl;
      rest = nl->Fourth(value);
      first = nl->TheEmptyList();
      while (!nl->IsEmpty(rest))
      {
        first = nl->First(rest);
        rest = nl->Rest(rest);

        cout << "Id: " << nl->IntValue(nl->First(first));
        cout << ", Length: " << nl->RealValue(nl->Seventh(first));
        cout << ", Speedlimit: " << nl->RealValue(nl->Sixth(first));
        ListExpr dir = nl->Fifth(first); 
        cout << ", Direction: " << nl->ToString(nl->First(dir)) << endl;
        cout << "Startjunction: " << nl->IntValue(nl->Third(first)) << endl;
        cout << "Targetjunction: " << nl->IntValue(nl->Fourth(first)) << endl;
        cout << "Represented route parts: " << endl;

        nl->ReadFromString("listjrint", subtype);
        DisplayTTY::GetInstance().DisplayResult(subtype, nl->Eigth(first));

        nl->ReadFromString("listint", subtype);
        cout << "Adjacent Sections Up: " << endl;
        DisplayTTY::GetInstance().DisplayResult(subtype, nl->Ninth(first));

        cout << "Adjacent Sections Down: " << endl;
        DisplayTTY::GetInstance().DisplayResult(subtype, nl->Tenth(first));

        cout << "Reverse adjacent sections Up: " << endl;
        DisplayTTY::GetInstance().DisplayResult(subtype, nl->Eleventh(first));

        cout << "Reverse adjacent Sections Down: " << endl;
        DisplayTTY::GetInstance().DisplayResult(subtype, nl->Twelfth(first));

        cout << "Curve starts at ";
        if (nl->BoolValue(nl->Second(nl->Second(first))))
          cout << "smaller ";
        else
          cout << "bigger ";
        cout << "endpoint." << endl;

        ListExpr restElem = nl->First(nl->Second(first));
        ListExpr firstElem = nl->TheEmptyList();
        while(!nl->IsEmpty(restElem))
        {
          firstElem = nl->First(restElem);
          restElem = nl->Rest(restElem);

          cout << "(" << nl->RealValue(nl->First(firstElem)) << ", "
               << nl->RealValue(nl->Second(firstElem)) << ")"
               << "-> (" << nl->RealValue(nl->Third(firstElem)) << ", "
               << nl->RealValue(nl->Fourth(firstElem)) << ")"
               << endl;
        }
        cout << endl;
      }
      cout << endl;

      cout << "Routes: " << endl;
      rest = nl->Fifth(value);
      first = nl->TheEmptyList();
      while (!nl->IsEmpty(rest))
      {
        first = nl->First(rest);
        rest = nl->Rest(rest);
        cout << "id: " << nl->IntValue(nl->First(first)) << endl;
        cout << "length: " << nl->RealValue(nl->Fourth(first)) << endl;
        nl->ReadFromString("listint", subtype);
        cout << "Junctions on Route: " << endl;
        DisplayTTY::GetInstance().DisplayResult(subtype, nl->Second(first));

        cout << "Sections on Route: " << endl;
        DisplayTTY::GetInstance().DisplayResult(subtype, nl->Third(first));
      }
      cout << endl;
    }
  }
};

struct DisplayIJPoint : DisplayFunction{
  virtual void Display(ListExpr type, ListExpr value)
  {
    if (nl->IsEqual(value, Symbol::UNDEFINED()))
      cout << Symbol::UNDEFINED() << endl;
    else
    {
      ListExpr subtype = nl->TheEmptyList();
      nl->ReadFromString("instant", subtype);
      DisplayTTY::GetInstance().DisplayResult(subtype, nl->First(value));
      nl->ReadFromString("jpoint", subtype);
      DisplayTTY::GetInstance().DisplayResult(subtype, nl->Second(value));
    }
  }
};

struct DisplayUJPoint : DisplayFunction{
  virtual void Display(ListExpr type, ListExpr value)
  {
    if (nl->IsEqual(value, Symbol::UNDEFINED()))
      cout << Symbol::UNDEFINED() << endl;
    else
    {
      ListExpr netL =nl->First(value);
      cout << "Network: " << nl->ToString(netL) << " " << endl;
      ListExpr subtype = nl->TheEmptyList();
      nl->ReadFromString("junit", subtype);
      DisplayTTY::GetInstance().DisplayResult(subtype, nl->Second(value));
    }
  }
};

struct DisplayJUnit : DisplayFunction{
  virtual void Display(ListExpr type, ListExpr value)
  {
    if (nl->IsEqual(value, Symbol::UNDEFINED()))
      cout << Symbol::UNDEFINED() << endl;
    else
    {
      ListExpr instL = nl->First(value);
      ListExpr subtype = nl->TheEmptyList();
      nl->ReadFromString("instant", subtype);
      DisplayTTY::GetInstance().DisplayResult(subtype, nl->First(instL));
      DisplayTTY::GetInstance().DisplayResult(subtype, nl->Second(instL));
      nl->ReadFromString("bool", subtype);
      DisplayTTY::GetInstance().DisplayResult(subtype, nl->Third(instL));
      DisplayTTY::GetInstance().DisplayResult(subtype, nl->Fourth(instL));
      nl->ReadFromString("jrint", subtype);
      DisplayTTY::GetInstance().DisplayResult(subtype, nl->Second(value));
    }
  }
};

struct DisplayMJPoint : DisplayFunction{
  virtual void Display(ListExpr type,  ListExpr value)
  {
    if (nl->IsEqual(value, Symbol::UNDEFINED()))
      cout << Symbol::UNDEFINED() << endl;
    else
    {
      ListExpr netL = nl->First(value);
      cout << "Network: " << nl->ToString(netL) << " " << endl;
      ListExpr subtype = nl->TheEmptyList();
      nl->ReadFromString("junit", subtype);
      ListExpr unitList = nl->Second(value);
      while (!nl->IsEmpty(unitList))
      {
        DisplayTTY::GetInstance().DisplayResult(subtype, nl->First(unitList));
        unitList = nl->Rest(unitList);
      }
    }
  }
};

struct DisplayRegEx : DisplayFunction{
  virtual void Display(ListExpr type, ListExpr value){
     if(nl->IsEqual(value,Symbol::UNDEFINED())){
       cout << Symbol::UNDEFINED() << endl;
       return;
     }
     if(!nl->HasLength(value,3) && !nl->HasLength(value,4)){
       cout << "Invalid regex representation" << endl;
       return;
     }
     if(nl->HasLength(value,4)){
       ListExpr srcList = nl->Fourth(value);
       if(nl->AtomType(srcList) == TextType) {
         cout << "Source : " << nl->Text2String(srcList) << endl;
       }
     }


     int numStates = nl->IntValue(nl->First(value));
     ListExpr transitions = nl->Second(value);
     ListExpr finalStates = nl->Third(value);
     vector<bool> final;
     for(int i=0;i<numStates;i++){
        final.push_back(false);
     }
     while(!nl->IsEmpty(finalStates)){
        int n = nl->IntValue(nl->First(finalStates));
        finalStates = nl->Rest(finalStates);
        final[n] = true;
     }
     set<int>* table[numStates][numStates];
     memset(table, 0, numStates*numStates*sizeof(void*));

     while(!nl->IsEmpty(transitions)){
         ListExpr transition = nl->First(transitions);
         transitions = nl->Rest(transitions);
         int source = nl->IntValue(nl->First(transition));
         int value = nl->IntValue(nl->Second(transition));
         int target = nl->IntValue(nl->Third(transition));
         if(!table[source][target]){
           table[source][target] = new set<int>();
         }
         (table[source][target])->insert(value);
     }
     for(int s=0;s<numStates;s++){
        for(int t=0;t<numStates;t++){
           if(table[s][t]){
              printtransition(s,t,*table[s][t],final[s],final[t]);
              delete table[s][t];
           }
        }
     }
  }
  private:
     void printtransition(const int src, const int dest, const set<int>& d,
                          const bool srcfinal, const bool destFinal){

          // TODO: abbreviate d
          set<int>::iterator it;
          int last = -2;
          vector<int> range;
          stringstream ss;

          if(d.size()<250) {
            for(it = d.begin(); it!=d.end(); it++){
               int next = *it;
               if(next!=last+1) { // start new range
                 if(range.size()>4){
                    ss << "[" << getStr(range[0]) << "-"
                       << getStr(range[range.size()-1]) << "]";
                 } else {
                   for(size_t i=0;i<range.size();i++){
                     ss << getStr(range[i]);
                   }
                 }
                 range.clear();
               }
               range.push_back(next);
               last = next;
            }
           for(size_t i=0;i<range.size();i++){
             ss << getStr(range[i]);
           }
          } else {
            if(d.size()==256){
              ss << "'all'";
            } else {
              // the most chars are not affected by this rule
              ss << "[^";
              it = d.begin();
              int pos = 0;
              while(pos<256){
                 if(it==d.end()){
                    ss << getStr(pos);
                    pos++;
                 } else {
                    int dpos = *it;
                    if(pos<dpos){
                      ss << getStr(pos);
                      pos++;
                    } else if(pos>dpos){
                      // shound never be the case
                      it++;
                    } else { // dpos == pos
                      pos++;
                      it++;
                    }
                 }
              }
              ss << "]";
            }
          }

          cout << (srcfinal?"*":" ") << src << " - "
               << ss.str() << " -> " << dest << (destFinal?"*":" ") << endl;
     }

     bool isPrintable(int c){
       return (c>32) && (c<127);
     }

     string getStr(int c){
        stringstream h;
        if(isPrintable(c)){
          h << (char)c;
          return h.str();
        }
        if(c==9){
          return "\\t";
        }
        if(c==10){
          return "\\n";

        }
        if(c==13){
           return "\\r";
        }

        h << "(" << c << ")";
        return h.str();
     }
};



/*
Display Hadoop file list

*/
struct DisplayFileList : DisplayFunction {
  virtual void Display(ListExpr type, ListExpr value)
  {
    if (nl->IsEqual(value, Symbol::UNDEFINED())){
      cout << Symbol::UNDEFINED();
    }
    else {
      string objName = nl->StringValue(nl->First(value));
      ListExpr nodeList = nl->Second(value);
      ListExpr locList = nl->Third(value);
      size_t dupTimes = nl->IntValue(nl->Sixth(value));
      bool isDistributed = nl->BoolValue(nl->Nth(7, value));
      int dataKind = nl->IntValue(nl->Nth(8, value));
      //NEW
      ListExpr UEMapQuery = nl->Nth(9, value);

      cout << "Name : " << objName << endl;
      cout << "Type : " << nl->ToString(type) << endl;

      int nlLen = nl->ListLength(nodeList);
      cout << "Cluster with total " << nlLen << " nodes.\n";
      cout << " - Master: \n  0.  "
          << nl->StringValue(nl->Second(nl->First(nodeList))) << ":"
          << nl->IntValue(nl->Fourth(nl->First(nodeList)))
          << endl;
      cout << " - Slaves: \n";
      ListExpr rest = nl->Rest(nodeList);
      int idx = 1;
      while (!nl->IsEmpty(rest))
      {
        cout <<"  " << (idx++) << ".  "
            << nl->StringValue(nl->Second(nl->First(rest))) << ":"
            << nl->IntValue(nl->Fourth(nl->First(rest)))
            << endl;
        rest = nl->Rest(rest);
      }

      if (!nl->IsEmpty(locList))
      {
        cout << "rowNo.\tcolumnNo.\tlocNode\tdupTimes" << endl;
        size_t rowNum = 1;
        while (!nl->IsEmpty(locList))
        {
          ListExpr aRow = nl->First(locList);

          if (!nl->IsEmpty(aRow)){
            size_t locNode = nl->IntValue(nl->First(aRow));

            ListExpr cfs = nl->Second(aRow);
            string dataLoc = nl->Text2String(nl->Third(aRow));

            cout << rowNum << ".";
            if (nl->IsEmpty(cfs)){
              cout << endl;
            }
            else{
              while (!nl->IsEmpty(cfs))
              {
                ListExpr aCF = nl->First(cfs);
                cout << "\t_" << nl->IntValue(aCF)
                    << "\t" << locNode << ":'" << dataLoc << "'"
                    << "\t" << dupTimes << endl;
                cfs = nl->Rest(cfs);
              }
            }
          }
          locList = nl->Rest(locList);
          rowNum++;
        }
      }

      cout << "Data distribute status: " <<
          (isDistributed ? "Done" :"Unknown" ) << endl;

      string kinds[4] = {"UNDEF", "DLO", "DLF"};
      cout << "Data kind: " << kinds[dataKind] << endl;

      //NEW
      cout << "UnExecuted Map Query: ";
      if (nl->IsEmpty(UEMapQuery))
        cout << "\"\"" << endl;
      else
      {
        cout << endl << nl->ToString(UEMapQuery) << endl;
      }

    }
  }
};

/*
Display LUBool

*/
struct DisplayLUType : DisplayFunction {

    virtual void Display (ListExpr type, ListExpr value)
    {
        if (nl->IsAtom(value)
            && nl->AtomType(value) == SymbolType
            && nl->SymbolValue(value) == "undef")
            {
                cout << "UNDEFINED";
            }
        else
        {
            ListExpr first = nl->First(value);
            ListExpr second = nl->Second(value);
            double Start = nl->RealValue(nl->First(first));
            double End = nl->RealValue(nl->Second(first));
            bool lc = nl->BoolValue(nl->Third(first));
            bool rc = nl->BoolValue(nl->Fourth(first));
            if (nl->IsAtom(second))
            {
                cout << nl->ToString(type)
                     << " -> interval: "
                     << (lc ? "[ " : "( ")
                     << std::setprecision(3)
                     << std::fixed
                     << Start
                     << " , "
                     << End
                     << (rc ? " ]" : " )")
                     << " value: "
                     << nl->ToString(value);
            }
            else
            {
                double m = nl->RealValue(nl->First(second));
                double n = nl->RealValue(nl->Second(second));

                cout << "lureal -> interval: "
                     << (lc ? "[ " : "( ")
                     << std::setprecision(3)
                     << std::fixed
                     << Start
                     << " , "
                     << End
                     << (rc ? " ]" : " )")
                     << " function: "
                     << "f(x) = " << m << " x + " << n;
            }
        }
    }
};

struct DisplayLType : DisplayFunction
{
virtual void Display(ListExpr type,  ListExpr value)
    {
        int No = 1;
        string unittype;
        if (nl->SymbolValue(type) == "lbool")
            unittype = "lubool";
        if (nl->SymbolValue(type) == "lint")
            unittype = "luint";
        if (nl->SymbolValue(type) == "lstring")
            unittype = "lustring";
        if (nl->SymbolValue(type) == "lreal")
            unittype = "lureal";

        cout << "*****************************BEGIN "
             << nl->ToString(type)
             << "********************************************** \n \n";
        if (nl->IsAtom(value) && nl->AtomType(value) == SymbolType &&
            (nl->SymbolValue(value) == "undef" ||
             nl->SymbolValue(value) == "undefined")) {
          cout << "   UNDEFINED" << endl;
        }
        else {
          ListExpr subtype = nl->TheEmptyList();
          nl->ReadFromString(unittype, subtype);
          if (!nl->IsEmpty(value))
          {
              ListExpr rest = value;
              while (!nl->IsEmpty(rest))
              {
                  cout << No++ << ": ";
                  DisplayTTY::GetInstance().DisplayResult(
                      subtype, nl->First(rest));
                  rest = nl->Rest(rest);
              }
          }
          else {
            cout << "Empty LType!";
          }
        }
        cout << endl << "*******************************END "
             << nl->ToString(type)
             << "**********************************************"
             << endl;
    }
};

/*
Display Label

*/
struct DisplayLabel : DisplayFunction {
  
virtual void Display(ListExpr type, ListExpr value) {
  if (nl->IsAtom(value) && nl->AtomType(value) == SymbolType &&
      nl->SymbolValue(value) == "undef") {
    cout << "UNDEFINED";
  }
  else {
    cout << nl->ToString(value);
  }
}
  
};

/*
Display Labels

*/
struct DisplayLabels : DisplayFunction {
  
virtual void Display(ListExpr type, ListExpr value) {
  if (nl->IsAtom(value) && nl->AtomType(value) == SymbolType &&
      nl->SymbolValue(value) == "undef") {
    cout << "UNDEFINED";
  }
  else {
    cout << nl->ToString(value);
  }
}
  
};

/*
Display Place

*/
struct DisplayPlace : DisplayFunction {
  
virtual void Display(ListExpr type, ListExpr value) {
  if (nl->IsAtom(value) && nl->AtomType(value) == SymbolType &&
      nl->SymbolValue(value) == "undef") {
    cout << "UNDEFINED";
  }
  else {
    cout << nl->ToString(value);
  }
}
  
};

/*
Display Places

*/
struct DisplayPlaces : DisplayFunction {
  
virtual void Display(ListExpr type, ListExpr value) {
  if (nl->IsAtom(value) && nl->AtomType(value) == SymbolType &&
      nl->SymbolValue(value) == "undef") {
    cout << "UNDEFINED";
  }
  else {
    cout << nl->ToString(value);
  }
}
  
};

/*
Display MPointer

*/
struct DisplayMPointer : DisplayFunction{
  virtual void Display(ListExpr type, ListExpr value){
     // just call the display function of the embedded mem 
     DisplayResult(nl->Second(type), value);     
  }
};

struct DisplayMem : DisplayFunction{
  virtual void Display(ListExpr type, ListExpr value){
     // just call the display function of the embedded object
     DisplayResult(nl->Second(type), value);     
  }
};

/*
4 Initialization

After implementing a new subclass of base ~DisplayFunction~ the new type
must be added below.

*/

void
DisplayTTY::Initialize()
{
  DisplayTTY& d = DisplayTTY::GetInstance();

  d.Insert("inquiry", new DisplayInquiry());
  d.Insert( "int",     new DisplayInt() );
  d.Insert( "longint", new DisplayLongInt() );
  d.Insert( "rational", new DisplayRational() );
  d.Insert( "real",    new DisplayReal() );
  d.Insert( "bool",    new DisplayBoolean() );
  d.Insert( "string",  new DisplayString() );
  d.Insert( "rel",     new DisplayRelation() );
  d.Insert( "mem",     new DisplayMem());
  d.Insert( "mpointer",new DisplayMPointer());
  d.Insert( "orel",    new DisplayRelation() );
  d.Insert( "trel",    new DisplayRelation() );
  d.Insert( "mrel",    new DisplayRelation() );
  d.Insert( "nrel",    new DisplayNestedRelation() );
  d.Insert( "arel",    new DisplayAttributeRelation() );
  d.Insert( "nrel2",   new DisplayNRel() ); //nr2a::ARel::BasicType()
  d.Insert( "arel2",   new DisplayARel() ); //nr2a::NRel::BasicType()
  d.Insert( "tuple",   new DisplayTuples() );
  d.Insert( "mtuple",  new DisplayTuples() );
  d.Insert( "map",     new DisplayFun() );
  d.Insert( "date",    new DisplayDate() );
  d.Insert( "text",    new DisplayText() );
  d.Insert( "xpoint",  new DisplayXPoint() );
  d.Insert( "rect",    new DisplayRect() );
  d.Insert( "rect3",   new DisplayRect3() );
  d.Insert( "rect4",   new DisplayRect4() );
  d.Insert( "rect8",   new DisplayRect8() );
  d.Insert( "array",   new DisplayArray() );
  d.Insert( "darray_old",  new DisplayDArray_old() );
  d.Insert( "point",   new DisplayPoint() );
  d.Insert( "tbtree",  new DisplayTBTree() );
  d.Insert( "binfile", new DisplayBinfile() );
  d.Insert( "mp3",     new DisplayMP3() );
  d.Insert( "id3",     new DisplayID3() );
  d.Insert( "lyrics",  new DisplayLyrics() );
  d.Insert( "midi",    new DisplayMidi() );
  d.Insert( "instant", new DisplayInstant() );
  d.Insert( "duration",new DisplayDuration() );
  d.Insert( "tid",     new DisplayTid() );
  d.Insert( "html",    new DisplayHtml() );
  d.Insert( "page",    new DisplayPage() );
  d.Insert( "url",     new DisplayUrl() );
  d.Insert( "vertex",  new DisplayVertex() );
  d.Insert( "edge",  new DisplayEdge() );
  d.Insert( "path",  new DisplayPath() );
  d.Insert( "graph", new DisplayGraph() );

  d.Insert( "histogram1d", new DisplayHistogram1d() );
  d.Insert( "histogram2d", new DisplayHistogram2d() );

  d.Insert( "cellgrid2d", new DisplayCellgrid2D() );
  d.Insert( "cellgrid3d", new DisplayCellgrid3D() );

  d.Insert( "flist", new DisplayFileList() );

  d.Insert( "drm", new DisplayDRM());


  // Chess Algebra 07/08
#ifndef ChessB
  d.Insert( "position", new DisplayPosition() );
  d.Insert( "move",     new DisplayMove() );
#else
  // Chess Algebra 08/09
  d.Insert( "position",  new DisplayPositionB() );
  d.Insert( "chessmove", new DisplayMoveB() );
  d.Insert( "material",  new DisplayMaterial() );
  d.Insert( "piece",     new DisplayPiece() );
  d.Insert( "field",     new DisplayField() );
#endif

  // CollectionAlgebra
  d.Insert( "vector",    new DisplayVector() );
  d.Insert( "set",       new DisplaySet() );
  d.Insert( "multiset",  new DisplayMultiset() );

  // JNetAlgebra
  d.Insert( "jdirection", new DisplayJDirection() );
  d.Insert( "rloc", new DisplayRouteLocation());
  d.Insert( "jrint", new DisplayJRouteInterval());
  d.Insert( "ndg", new DisplayNDG());
  d.Insert( "junit", new DisplayJUnit());
  d.Insert( "listint", new DisplayJListInt());
  d.Insert( "listrloc", new DisplayJListRLoc());
  d.Insert( "listjrint", new DisplayJListRInt());
  d.Insert( "listndg", new DisplayJListNDG());
  d.Insert( "jnet", new DisplayJNetwork());
  d.Insert( "jpoint", new DisplayJPoint());
  d.Insert( "jline", new DisplayJLine());
  d.Insert( "jpath", new DisplayJLine());
  d.Insert( "jpoints", new DisplayJPoints());
  d.Insert( "ijpoint", new DisplayIJPoint());
  d.Insert( "ujpoint", new DisplayUJPoint());
  d.Insert( "mjpoint", new DisplayMJPoint());
  d.Insert( "regex", new DisplayRegEx());
  d.Insert( "regex2", new DisplayRegEx());

  // Raster2Algebra
  d.Insert("sbool", new raster2::DisplaySType());
  d.Insert("sint", new raster2::DisplaySType());
  d.Insert("sreal", new raster2::DisplaySType());
  d.Insert("sstring", new raster2::DisplaySType());
  d.Insert("msbool", new raster2::Displaymstype());
  d.Insert("msint", new raster2::Displaymstype());
  d.Insert("msreal", new raster2::Displaymstype());
  d.Insert("msstring", new raster2::Displaymstype());
  d.Insert("isbool", new raster2::Displayistype());
  d.Insert("isint", new raster2::Displayistype());
  d.Insert("isreal", new raster2::Displayistype());
  d.Insert("isstring", new raster2::Displayistype());
  d.Insert("grid2", new raster2::Displaygrid2());
  d.Insert("grid3", new raster2::Displaygrid3());

  d.Insert("lubool", new DisplayLUType);
  d.Insert("luint", new DisplayLUType);
  d.Insert("lustring", new DisplayLUType);
  d.Insert("lureal", new DisplayLUType);
  d.Insert("lbool", new DisplayLType);
  d.Insert("lint", new DisplayLType);
  d.Insert("lstring", new DisplayLType);
  d.Insert("lreal", new DisplayLType);

  // SymbolicTrajectoryAlgebra
  d.Insert("label", new DisplayLabel);
  d.Insert("labels", new DisplayLabels);
  d.Insert("place", new DisplayPlace);
  d.Insert("places", new DisplayPlaces);
 
  


}

/*
Removes the existing instance

*/
 void DisplayTTY::Finish(){
   if(dtty){
     delete dtty;
     dtty = 0;
   }
 }



