/**
 * @file CSVDataPrinter.h
 * @author Til Gärtner
 * @brief Expands the standard Data Printer with the capabiltiy to write CSV data
 *
 *
 */
#pragma once

#include <jem/base/Array.h>
#include <jem/base/System.h>
#include <jem/mp/GatherPrinter.h>
#include <jem/util/ArrayBuffer.h>
#include <jive/app/DataPrinter.h>
#include <jive/util/DofSpace.h>
#include <jive/util/Globdat.h>
#include <jive/util/ItemMap.h>
#include <jive/util/ItemSet.h>
#include <jive/util/Table.h>
#include <jive/util/TableException.h>

using jive::idx_t;
using jive::IdxVector;
using jive::Properties;
using jive::String;
using jive::StringVector;
using jive::Vector;
using jive::app::DataPrinter;
using jive::util::DofSpace;
using jive::util::Globdat;
using jive::util::Table;

class CSVDataPrinter : public DataPrinter
{
public:
  enum mode
  {
    EMPTY,
    TABLES,
    VECTORS,
    SPARSEVECTORS
  };

  JEM_DECLARE_CLASS(CSVDataPrinter, DataPrinter);

  explicit CSVDataPrinter

      (const String &name = "CSVDataPrinter");

  virtual void printTable

      (Output &out,
       const String &label,
       const Table &table,
       const Properties &globdat) override;

  virtual void printVector

      (Output &out,
       const String &label,
       const Vector &vec,
       const DofSpace &dofs,
       const Properties &globdat) override;

  virtual void printSparseVector

      (Output &out,
       const String &label,
       const Vector &vec,
       const IdxVector &idofs,
       const DofSpace &dofs,
       const Properties &globdat) override;

protected:
  virtual ~CSVDataPrinter();

private:
  void startTables_

      (Output &out,
       const String &label,
       const Table &table,
       const Properties &globdat);

  void startVectors_

      (Output &out,
       const String &label,
       const Vector &vec,
       const DofSpace &dofs,
       const Properties &globdat,
       const IdxVector &idofs = {});

  void writePrefix_

      (Output &out,
       const Properties &globdat,
       const String &label = "");

  void writeVector_

      (Output &out,
       const Vector &vec);

private:
  mode mode_;
};
