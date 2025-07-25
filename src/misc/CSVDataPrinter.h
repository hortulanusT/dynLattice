/**
 * @file CSVDataPrinter.h
 * @author Til Gärtner
 * @brief CSV data output module for tables and vectors.
 */
#pragma once

#include <jive/app/DataPrinter.h>
#include <jive/util/DofSpace.h>
#include <jive/util/Table.h>

using jive::IdxVector;
using jive::Properties;
using jive::String;
using jive::Vector;
using jive::app::DataPrinter;
using jive::util::DofSpace;
using jive::util::Table;

/**
 * @brief CSV data printer for simulation output.
 *
 * Extends the standard DataPrinter to output data in CSV format with timestamps.
 * Supports three output modes: tables, vectors, and sparse vectors. Once a mode
 * is set for an instance, it cannot be changed during the simulation.
 *
 * @section Usage
 * The printer automatically detects output mode from first print call:
 * - printTable() sets TABLES mode
 * - printVector() or printSparseVector() sets VECTORS/SPARSEVECTORS mode
 *
 * @author Til Gärtner
 * @see [jive::app::DataPrinter](https://jive-manual.dynaflow.com/classjive_1_1app_1_1DataPrinter.html)
 */
class CSVDataPrinter : public DataPrinter
{
public:
  /// Output mode enumeration for CSV printer state
  enum mode
  {
    EMPTY,        ///< No output mode set yet
    TABLES,       ///< Table output mode active
    VECTORS,      ///< Vector output mode active
    SPARSEVECTORS ///< Sparse vector output mode active
  };

  JEM_DECLARE_CLASS(CSVDataPrinter, DataPrinter);

  /**
   * @brief Constructs a new CSVDataPrinter object.
   *
   * @param[in] name Name of the data printer instance for identification
   */
  explicit CSVDataPrinter

      (const String &name = "CSVDataPrinter");

  /**
   * @brief Print table data in CSV format with timestamps.
   *
   * Sets printer mode to TABLES on first call. All subsequent calls must be
   * to printTable() or an exception will be thrown.
   *
   * @param[in] out Output stream for CSV data
   * @param[in] label Data label for identification
   * @param[in] table Table containing data to print
   * @param[in] globdat Global data properties with simulation context
   */
  virtual void printTable

      (Output &out,
       const String &label,
       const Table &table,
       const Properties &globdat) override;

  /**
   * @brief Print vector data in CSV format with DOF information.
   *
   * Sets printer mode to VECTORS on first call. Cannot be mixed with table output.
   *
   * @param[in] out Output stream for CSV data
   * @param[in] label Data label for identification
   * @param[in] vec Vector containing data to print
   * @param[in] dofs Degree of freedom space for context
   * @param[in] globdat Global data properties with simulation context
   */
  virtual void printVector

      (Output &out,
       const String &label,
       const Vector &vec,
       const DofSpace &dofs,
       const Properties &globdat) override;

  /**
   * @brief Print sparse vector data in CSV format.
   *
   * Similar to printVector but handles sparse data with specified DOF indices.
   * Sets printer mode to SPARSEVECTORS on first call.
   *
   * @param[in] out Output stream for CSV data
   * @param[in] label Data label for identification
   * @param[in] vec Vector containing sparse data to print
   * @param[in] idofs DOF indices for sparse vector entries
   * @param[in] dofs Degree of freedom space for context
   * @param[in] globdat Global data properties with simulation context
   */
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
  /**
   * @brief Initialize table output format and write headers
   * @param[in] out Output stream for CSV data
   * @param[in] label Data label for identification
   * @param[in] table Table containing data structure
   * @param[in] globdat Global data properties with simulation context
   */
  void startTables_

      (Output &out,
       const String &label,
       const Table &table,
       const Properties &globdat);

  /**
   * @brief Initialize vector output format and write headers
   * @param[in] out Output stream for CSV data
   * @param[in] label Data label for identification
   * @param[in] vec Vector containing data to print
   * @param[in] dofs Degree of freedom space for context
   * @param[in] globdat Global data properties with simulation context
   * @param[in] idofs DOF indices for sparse vectors (optional)
   */
  void startVectors_

      (Output &out,
       const String &label,
       const Vector &vec,
       const DofSpace &dofs,
       const Properties &globdat,
       const IdxVector &idofs = {});

  /**
   * @brief Write CSV prefix with timestamp information
   * @param[in] out Output stream for CSV data
   * @param[in] globdat Global data properties with simulation context
   * @param[in] label Data label for identification (optional)
   */
  void writePrefix_

      (Output &out,
       const Properties &globdat,
       const String &label = "");

  /**
   * @brief Write vector data values to output stream
   * @param[in] out Output stream for CSV data
   * @param[in] vec Vector containing data values to write
   */
  void writeVector_

      (Output &out,
       const Vector &vec);

private:
  mode mode_; ///< Current output mode state
};
