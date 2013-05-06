#ifndef _MEDUSA_ANALYZER_
#define _MEDUSA_ANALYZER_

#include "medusa/namespace.hpp"
#include "medusa/export.hpp"
#include "medusa/database.hpp"
#include "medusa/architecture.hpp"
#include "medusa/control_flow_graph.hpp"

//class ControlFlowGraph;
#include <fstream>

#include <boost/thread/mutex.hpp>
#include <boost/graph/graphviz.hpp>

MEDUSA_NAMESPACE_BEGIN

//! Analyzer handles all analysis operations.
class Medusa_EXPORT Analyzer
{
public:
  Analyzer(void)
  : m_FunctionPrefix("fcn_")
  , m_LabelPrefix("lbl_")
  , m_DataPrefix("dat_")
  , m_StringPrefix("str_")
  , m_ArchIdPool()
  , m_DefaultArchitectureTag(MEDUSA_ARCH_UNK)
  , m_UsedArchitectures()
  {}

  ~Analyzer(void) { m_UsedArchitectures.erase(std::begin(m_UsedArchitectures), std::end(m_UsedArchitectures)); }

  //! This method disassembles code by following the execution path.
  void DisassembleFollowingExecutionPath(Database& rDb, Address const& rEntrypoint, Architecture &rArch) const;

  //! This method finds and adds cross-references.
  void CreateXRefs(Database& rDb) const;

  //! This method finds string using specific patterns.
  void FindStrings(Database& rDb, Architecture& rArch) const;

  /*! This method computes the size of a function.
   * \param rDb contains all cells.
   * \param rFunctionAddress is the address of the function.
   * \param EndAddress is set by this method and contains the end of the function.
   * \param rFunctionLength is set by this method and contains the size of the function.
   * \param rInstructionCounter is set by this method and contains the number of instruction in the function.
   * \param LengthThreshold is the maximum size of this function, it this value is reached, this method returns false.
   * \return Returns true if the size of the function can be computed, otherwise it returns false.
   */
  bool ComputeFunctionLength(
    Database const& rDb,
    Address const& rFunctionAddress,
    Address& EndAddress,
    u16& rFunctionLength,
    u16& rInstructionCounter,
    u32 LengthThreshold) const;

  bool CreateFunction(Database& rDb, Address const& rAddr);

  bool BuildControlFlowGraph(Database& rDb, std::string const& rLblName, ControlFlowGraph& rCfg) const;
  bool BuildControlFlowGraph(Database& rDb, Address const& rAddr,        ControlFlowGraph& rCfg) const;

  bool RegisterArchitecture(Architecture::SharedPtr spArch);
  bool UnregisterArchitecture(Architecture::SharedPtr spArch);
  void ResetArchitecture(void);

  Architecture::SharedPtr GetArchitecture(Tag ArchTag) const;

  Cell* GetCell(Database const& rDatabase, BinaryStream const& rBinStrm, Address const& rAddr);
  Cell const* GetCell(Database const& rDatabase, BinaryStream const& rBinStrm, Address const& rAddr) const;

  MultiCell* GetMultiCell(Database const& rDatabase, BinaryStream const& rBinStrm, Address const& rAddr);
  MultiCell const* GetMultiCell(Database const& rDatabase, BinaryStream const& rBinStrm, Address const& rAddr) const;

  void DumpControlFlowGraph(std::string const& rFilename, ControlFlowGraph const& rCfg, Database const& rDatabase, BinaryStream const& rBinStrm) const
  {
    std::ofstream File(rFilename.c_str());
    boost::write_graphviz(File, rCfg.GetGraph(), PropWriter<ControlFlowGraph::Type>(rCfg.GetGraph(), *this, rDatabase, rBinStrm));
  }

private:
  // Workaround from http://stackoverflow.com/questions/9669109/print-a-constified-subgraph-with-write-graphviz
  template<typename Graph> struct PropWriter
  {
    PropWriter(Graph const& rCfg, Analyzer const& rAnlz, Database const& rDatabase, BinaryStream const& rBinStrm)
      : m_rCfg(rCfg), m_rAnlz(rAnlz), m_rDb(rDatabase), m_rBinStrm(rBinStrm) {}
    template<typename Vertex> void operator()(std::ostream & out, Vertex const& v) const
    {
      out << "[shape=box] [label=\"";
      for (auto itAddr = std::begin(m_rCfg[v].GetAddresses()); itAddr != std::end(m_rCfg[v].GetAddresses()); ++itAddr)
      {
        std::string LineString = "Unknown";
        auto pCell = m_rAnlz.GetCell(m_rDb, m_rBinStrm, *itAddr);
        if (pCell != nullptr)
          LineString = pCell->ToString();

        out << *itAddr << ": " << LineString << "\\n";
      }
      out << "\"]";
    }

    Graph        const& m_rCfg;
    Analyzer     const& m_rAnlz;
    Database     const& m_rDb;
    BinaryStream const& m_rBinStrm;
  };

  static bool DisassembleBasicBlock(
      Database const& rDb,
      Architecture& rArch,
      Address const& rAddr,
      std::list<Instruction*>& rBasicBlock);

  std::string          m_FunctionPrefix; //! Function prefix
  std::string          m_LabelPrefix;    //! Label prefix
  std::string          m_DataPrefix;     //! Data prefix
  std::string          m_StringPrefix;   //! String prefix
  mutable boost::mutex m_DisasmMutex;

  u32                  m_ArchIdPool;
  Tag                  m_DefaultArchitectureTag;
  Architecture::TagMap m_UsedArchitectures;

};

MEDUSA_NAMESPACE_END

#endif // !_MEDUSA_DISASSEMBLER_