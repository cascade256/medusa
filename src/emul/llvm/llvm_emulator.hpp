#ifndef _EMUL_LLVM_
#define _EMUL_LLVM_

#include <medusa/emulation.hpp>

#include <stack>
#include <tuple>

#include <llvm/Support/TargetSelect.h>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/DataLayout.h>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Target/TargetMachine.h>

#if defined(_WIN32) || defined(WIN32)
# ifdef emul_llvm_EXPORTS
#  define EMUL_LLVM_EXPORT __declspec(dllexport)
# else
#  define EMUL_LLVM_EXPORT __declspec(dllimport)
# endif
#else
# define EMUL_LLVM_EXPORT
#endif

MEDUSA_NAMESPACE_USE

  extern "C" EMUL_LLVM_EXPORT Emulator* GetEmulator(CpuInformation const* pCpuInfo, CpuContext* pCpuCtxt, MemoryContext *pMemCtxt);

// TODO: overload VariableContext to use llvm Alloca
class LlvmEmulator : public medusa::Emulator
{
public:
  LlvmEmulator(CpuInformation const* pCpuInfo, CpuContext* pCpuCtxt, MemoryContext *pMemCtxt);
  virtual ~LlvmEmulator(void);

  virtual std::string GetName(void) const { return "llvm"; }

  virtual bool Execute(Address const& rAddress, Expression const& rExpr);
  virtual bool Execute(Address const& rAddress, Expression::List const& rExprList);

private:
  typedef void (*BasicBlockCode)(u8* pCpuCtxt, u8* pCpuCtxtObj, u8* pMemCtxtObj);

  llvm::IRBuilder<>             m_Builder;
  static llvm::Module*          sm_pModule;
  static llvm::ExecutionEngine* sm_pExecutionEngine;
  static llvm::DataLayout*      sm_pDataLayout;

  // TODO: Implement InvalidCache to handle self-modifying code
  // TODO: Implement a method in CpuContext to get the current address (we can't always rely on CpuInformation::ProgramPointerRegister)
  typedef std::unordered_map<u64, llvm::BasicBlock*> BasicBlockCacheType;
  typedef std::unordered_map<u64, llvm::Function*>   FunctionCacheType;
  BasicBlockCacheType           m_BasicBlockCache;
  FunctionCacheType             m_FunctionCache;

  class LlvmVariableContext : public VariableContext
  {
  public:
    LlvmVariableContext(llvm::IRBuilder<>& rBuilder);

    virtual bool ReadVariable(std::string const& rVariableName, u64& rValue) const;
    virtual bool WriteVariable(std::string const& rVariableName, u64 Value, bool SignExtend = false);

    virtual bool AllocateVariable(u32 Type, std::string const& rVariableName);

    virtual std::string ToString(void) const;

  private:
    llvm::IRBuilder<>& m_rBuilder;
  };


  class LlvmExpressionVisitor : public ExpressionVisitor
  {
  public:
    LlvmExpressionVisitor(
      HookAddressHashMap const& Hooks,
      CpuContext* pCpuCtxt, MemoryContext* pMemCtxt, VariableContext* pVarCtxt,
      llvm::IRBuilder<>& rBulder, llvm::Value* pCpuCtxtParam, llvm::Value* pCpuCtxtObjParam, llvm::Value* pMemCtxtObjParam);
    virtual Expression* VisitBind(Expression::List const& rExprList);
    virtual Expression* VisitCondition(u32 Type, Expression const* pRefExpr, Expression const* pTestExpr);
    virtual Expression* VisitIfCondition(u32 Type, Expression const* pRefExpr, Expression const* pTestExpr, Expression const* pThenExpr);
    virtual Expression* VisitIfElseCondition(u32 Type, Expression const* pRefExpr, Expression const* pTestExpr, Expression const* pThenExpr, Expression const* pElseExpr);
    virtual Expression* VisitWhileCondition(u32 Type, Expression const* pRefExpr, Expression const* pTestExpr, Expression const* pBodyExpr);
    virtual Expression* VisitOperation(u32 Type, Expression const* pLeftExpr, Expression const* pRightExpr);
    virtual Expression* VisitConstant(u32 Type, u64 Value);
    virtual Expression* VisitIdentifier(u32 Id, CpuInformation const* pCpuInfo);
    virtual Expression* VisitMemory(u32 AccessSizeInBit, Expression const* pBaseExpr, Expression const* pOffsetExpr, bool Deref);
    virtual Expression* VisitVariable(u32 SizeInBit, std::string const& rName);

    void ClearValues(void);

  protected:
    llvm::Value* MakeInteger(u32 Bits, u64 Value) const;
    llvm::Value* MakePointer(u32 Bits, void* pPointer, s32 Offset = 0) const;
    llvm::Value* MakePointer(u32 Bits, llvm::Value* pPointerValue, s32 Offset = 0) const;

    HookAddressHashMap const& m_rHooks;
    CpuContext*               m_pCpuCtxt;
    MemoryContext*            m_pMemCtxt;
    VariableContext*          m_pVarCtxt;
    llvm::IRBuilder<>&        m_rBuilder;

    std::stack<std::tuple<llvm::Value*, llvm::Value*>>  m_ValueStack;
    std::map<std::string, llvm::Value*>                 m_Variables;
    llvm::Value*                                        m_pCpuCtxtParam;
    llvm::Value*                                        m_pCpuCtxtObjParam;
    llvm::Value*                                        m_pMemCtxtObjParam;
  };
};

#endif // !_EMUL_LLVM_