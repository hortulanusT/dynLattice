
/**
 * @file CSVDataPrinter.cpp
 * @author Til Gärtner
 * @brief Implementation of CSV data output module.
 *
 */

#include <jem/base/Array.h>
#include <jem/base/ClassTemplate.h>
#include <jem/base/System.h>
#include <jem/mp/GatherPrinter.h>
#include <jem/util/ArrayBuffer.h>
#include <jive/util/Globdat.h>
#include <jive/util/ItemMap.h>
#include <jive/util/ItemSet.h>
#include <jive/util/TableException.h>
#include "misc/CSVDataPrinter.h"

using jive::idx_t;
using jive::StringVector;
using jive::util::Globdat;

JEM_DEFINE_CLASS(CSVDataPrinter);

//=======================================================================
//   class CSVDataPrinter
//=======================================================================

//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------

CSVDataPrinter::CSVDataPrinter(const String &name) :

                                                     Super(name)

{
  mode_ = EMPTY;
}

CSVDataPrinter::~CSVDataPrinter()
{
}

//-----------------------------------------------------------------------
//   printTable
//-----------------------------------------------------------------------

void CSVDataPrinter::printTable

    (Output &out,
     const String &label,
     const Table &table,
     const Properties &globdat)

{
  if (mode_ == EMPTY)
  {
    mode_ = TABLES;
    writePrefix_(out, globdat);
    startTables_(out, label, table, globdat);
  }
  JEM_PRECHECK2(mode_ == TABLES, "Cannot Output Tables and Vectors in the same csv, try using seperate modules!");

  writePrefix_(out, globdat, label);

  double val;
  IdxVector rows(table.rowCount());
  table.getRowItems()->getItemMap()->getItemIDs(rows, (IdxVector)jem::iarray(table.rowCount()));

  for (idx_t row : rows)
    for (idx_t column = 0; column < table.columnCount(); column++)
      if (table.findValue(val, row, column))
        print(out, ",", val);

  print(out, "\n");
}

//-----------------------------------------------------------------------
//   printVector
//-----------------------------------------------------------------------

void CSVDataPrinter::printVector

    (Output &out,
     const String &label,
     const Vector &vec,
     const DofSpace &dofs,
     const Properties &globdat)

{
  if (mode_ == EMPTY)
  {
    mode_ = VECTORS;
    writePrefix_(out, globdat);
    startVectors_(out, label, vec, dofs, globdat);
  }
  JEM_PRECHECK2(mode_ == VECTORS, "Cannot Output Tables and Vectors in the same csv, try using seperate modules!");

  writePrefix_(out, globdat, label);
  writeVector_(out, vec);
}

//-----------------------------------------------------------------------
//   printSparseVector
//-----------------------------------------------------------------------

void CSVDataPrinter::printSparseVector

    (Output &out,
     const String &label,
     const Vector &vec,
     const IdxVector &idofs,
     const DofSpace &dofs,
     const Properties &globdat)

{
  if (mode_ == EMPTY)
  {
    mode_ = SPARSEVECTORS;
    writePrefix_(out, globdat);
    startVectors_(out, label, vec, dofs, globdat, idofs);
  }
  JEM_PRECHECK2(mode_ == SPARSEVECTORS, "Cannot Output Tables and Vectors in the same csv, try using seperate modules!");

  writePrefix_(out, globdat, label);
  writeVector_(out, (Vector)vec[idofs]);
}

//-----------------------------------------------------------------------
//   startTables_
//-----------------------------------------------------------------------

void CSVDataPrinter::startTables_

    (Output &out,
     const String &label,
     const Table &table,
     const Properties &globdat)

{
  double val;
  IdxVector rows(table.rowCount());
  table.getRowItems()->getItemMap()->getItemIDs(rows, (IdxVector)jem::iarray(table.rowCount()));

  for (idx_t row : rows)
    for (idx_t column = 0; column < table.columnCount(); column++)
      if (table.findValue(val, row, column))
        print(out, ",", table.getColumnName(column), "[", row, "]");

  print(out, "\n");
}

//-----------------------------------------------------------------------
//   startVectors_
//-----------------------------------------------------------------------

void CSVDataPrinter::startVectors_

    (Output &out,
     const String &label,
     const Vector &vec,
     const DofSpace &dofs,
     const Properties &globdat,
     const IdxVector &idofs)

{
  IdxVector reportDofs = idofs.size() ? idofs : (IdxVector)jem::iarray(dofs.dofCount());

  for (idx_t dof : reportDofs)
    print(out, ",", dofs.getDofName(dof));

  print(out, "\n");
}

//-----------------------------------------------------------------------
//   writePrefix_
//-----------------------------------------------------------------------

void CSVDataPrinter::writePrefix_

    (Output &out,
     const Properties &globdat,
     const String &label)

{
  idx_t i;
  double t;

  if (label.size())
  {
    globdat.get(i, Globdat::TIME_STEP);
    if (globdat.find(t, Globdat::TIME))
      print(out, t, ",");
    else
      print(out, i, ",");
    print(out, label);
  }
  else
  {
    if (globdat.find(t, Globdat::TIME))
      print(out, "time", ",");
    else
      print(out, "step", ",");
    print(out, "label");
  }
}

//-----------------------------------------------------------------------
//   writeVector_
//-----------------------------------------------------------------------

void CSVDataPrinter::writeVector_

    (Output &out,
     const Vector &vec)

{
  for (double val : vec)
    print(out, ",", val);

  print(out, "\n");
}
