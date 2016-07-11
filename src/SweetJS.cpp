#include <iostream>

#include "swift/Basic/LLVMInitialize.h"
#include "swift/Frontend/Frontend.h"
#include "swift/AST/AST.h"
#include "swift/Frontend/PrintingDiagnosticConsumer.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Signals.h"

#include "Translator.h"

static llvm::cl::opt<std::string>
inputFilename(llvm::cl::desc("input file"), llvm::cl::init("-"),
              llvm::cl::Positional);

static llvm::cl::opt<std::string>
OutputFilename("o", llvm::cl::desc("output filename"), llvm::cl::init("-"));


static llvm::cl::opt<bool>
EmitVerboseSIL("emit-verbose-sil",
               llvm::cl::desc("Emit locations during sil emission."));

static llvm::cl::opt<std::string>
FunctionName("func", llvm::cl::desc("Function name to extract."));

static llvm::cl::list<std::string>
importPaths("I", llvm::cl::desc("add a directory to the import search path"));

static llvm::cl::opt<std::string>
ModuleName("module-name", llvm::cl::desc("The name of the module if processing"
                                         " a module. Necessary for processing "
                                         "stdin."));

static llvm::cl::opt<std::string>
moduleCachePath("module-cache-path", llvm::cl::desc("Clang module cache path"));

static llvm::cl::opt<std::string>
resourceDir("resource-dir",
            llvm::cl::desc("The directory that holds the compiler resource files"));

static llvm::cl::opt<std::string>
sdkPath("sdk", llvm::cl::desc("The path to the SDK for use with the clang "
                              "importer."),
        llvm::cl::init(""));

static llvm::cl::opt<std::string>
triple("target", llvm::cl::desc("target triple"));

// This function isn't referenced outside its translation unit, but it
// can't use the "static" keyword because its address is used for
// getMainExecutable (since some platforms don't support taking the
// address of main, and some platforms can't implement getMainExecutable
// without being given the address of a function in the main executable).
void anchorForGetMainExecutable() {}

int main(int argc, char **argv) {
    INITIALIZE_LLVM(argc, argv);
    
    llvm::cl::ParseCommandLineOptions(argc, argv, "SweetJS\n");
    
    using namespace swift;
    CompilerInvocation invocation;
    
    auto mainExecutablePath = llvm::sys::fs::getMainExecutable(
        argv[0], reinterpret_cast<void *>(&anchorForGetMainExecutable));
    invocation.setMainExecutablePath(mainExecutablePath);
    
    invocation.setImportSearchPaths(importPaths);
    if (sdkPath.getNumOccurrences() == 0) {
        auto *sdkRoot = getenv("SDKROOT");
        if (sdkRoot != nullptr)
            sdkPath = sdkRoot;
    }
    if (!sdkPath.empty())
        invocation.setSDKPath(sdkPath);
    if (!triple.empty())
        invocation.setTargetTriple(triple);
    if (!resourceDir.empty())
        invocation.setRuntimeResourcePath(resourceDir);
    invocation.getClangImporterOptions().ModuleCachePath = moduleCachePath;
    invocation.getLangOptions().EnableAccessControl = false;
    invocation.getLangOptions().EnableObjCAttrRequiresFoundation = false;
    invocation.setModuleName("main");
    invocation.setInputKind(InputFileKind::IFK_Swift);
    
    auto fileBufOrErr = llvm::MemoryBuffer::getFileOrSTDIN(inputFilename);
    if (!fileBufOrErr) {
        std::cerr << "Error! Failed to open file: " << inputFilename << std::endl;
        exit(-1);
    }
    invocation.addInputBuffer(fileBufOrErr.get().get());
    
    CompilerInstance instance;
    PrintingDiagnosticConsumer printDiags;
    instance.addDiagnosticConsumer(&printDiags);
    
    if (instance.setup(invocation))
        return 1;
    instance.performSema();
    
    auto &astContext = instance.getASTContext();
    if (astContext.hadError())
        return 1;
    
    auto mainModuleIter = astContext.LoadedModules.begin();
    while (mainModuleIter != astContext.LoadedModules.end()) {
        if (mainModuleIter->first.str() == "main")
            break;
        ++mainModuleIter;
    }
    
    if (mainModuleIter == astContext.LoadedModules.end()) {
        std::cout << "main module not found" << std::endl;
        exit(1);
    }
    
    auto mainModule = mainModuleIter->second;
    mainModule->dump(llvm::outs());
    SmallVector<Decl*, 32> decls;
    mainModule->getTopLevelDecls(decls);
    for (auto decl : decls) {
        auto funcDecl = dyn_cast<FuncDecl>(decl);
        if (funcDecl == nullptr)
            continue;
        auto translate = TranslateDecl(llvm::outs(), llvm::errs(), 0);
        translate.visit(decl);
    }
    
    return 0;
}
