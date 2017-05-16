#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/Lexer.h"

#include <iostream>
#include <iterator>
#include <algorithm>

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

Rewriter rewriter;
int numFunctions = 0;
bool can_change = false;

FileID main_file;

// all the types we want to replace - I have added a space to stop "int *hi = 2;"
pair<string, string> type_replace[10] = {{"unsigned int", "uint16_t "},
    {"unsigned long", "uint32_t "},
    {"int", "int16_t "},
    {"long", "int32_t "},
    {"double", "float "},
    {"unsigned int *", "uint16_t * "},
    {"int *", "int16_t * "},
    {"unsigned long *", "uint32_t * "},
    {"long *", "int32_t * "},
    {"double *", "float * "},
};
vector<SourceLocation> varaible_locations;


// Replace the variable type for different input decl types
template<typename T>
void replace_text(T input_decl, string r_text, bool end = true) {
    auto in_start = input_decl->getLocStart();
    // setting it to in_start enables the use of auto for different input decls we use
    auto in_end = in_start;

    // if we have already replaced it, skip
    if (find(varaible_locations.begin(), varaible_locations.end(), in_start) != varaible_locations.end())
        return;
    varaible_locations.push_back(in_start);
    // if we want the locEnd or getLocation function - depends on decl type
    if (end)
        in_end = input_decl->getLocEnd();
    else
        in_end = input_decl->getLocation();

    // gets the range of the total parameter - including type and argument
    auto range = CharSourceRange::getTokenRange(in_start, in_end);

    // get the stringPtr from the range and convert to string
    string s = string(Lexer::getSourceText(range, rewriter.getSourceMgr(), rewriter.getLangOpts()));

    // offset gives us the length of the argument, 1 is for the space
    int offset = Lexer::MeasureTokenLength(in_end, rewriter.getSourceMgr(), rewriter.getLangOpts());
    // replace the text with the text sent
    rewriter.ReplaceText(in_start, s.length() - offset, r_text);
}

class ExampleVisitor : public RecursiveASTVisitor<ExampleVisitor> {
  private:
    ASTContext *astContext; // used for getting additional AST info

  public:
    explicit ExampleVisitor(CompilerInstance *CI)
        : astContext(&(CI->getASTContext())) { // initialize private members
        SourceManager &SM = astContext->getSourceManager();
        rewriter.setSourceMgr(SM, astContext->getLangOpts());
        main_file = SM.getMainFileID();
        
    }

    virtual bool VisitFunctionDecl(FunctionDecl *func) {
        numFunctions++;
        string retType = func->getReturnType().getAsString();
        string funcName = func->getNameInfo().getName().getAsString();
        string arg;
        int params = func->getNumParams();
        FileID curr_file = astContext->getSourceManager().getFileID(func->getLocation());
        if (!can_change && curr_file == main_file) {
            can_change = true;
        }
        // SourceLocation func_start = func->getLocStart();
        SourceLocation param_start;
        SourceLocation param_end;
        for (auto elem : type_replace) {
            if (retType == elem.first && can_change) {
                auto begin_loc = func->getLocStart();
                // if starting location is found - we have replaced already - move along!
                if (find(varaible_locations.begin(), varaible_locations.end(), begin_loc) != varaible_locations.end())
                    continue;
                varaible_locations.push_back(begin_loc);
                // getLocStart gives us the ( location, it would seem
                auto bracket_loc = func->getNameInfo().getLocStart();
                // get the name length, add 1 for the space
                int name_length = func->getNameInfo().getAsString().length();
                // rewriter.ReplaceText(func->getLocStart(), retType.length(), "int16_t");
                // gets the range of the total parameter - including type and argument
                auto range_token = CharSourceRange::getTokenRange(begin_loc, bracket_loc);
                // get the stringPtr from the range and convert to string
                string s = string(Lexer::getSourceText(range_token, rewriter.getSourceMgr(), rewriter.getLangOpts()));
                // replace the text with the text sent
                rewriter.ReplaceText(begin_loc, s.length() - name_length, elem.second);
            }
        }
        if (params && can_change) {
            for (int i = 0; i < params; ++i) {
                auto param = func->parameters()[i];
                arg = param->getType().getAsString();
                for (auto elem : type_replace) {
                    if (arg == elem.first) {
                        replace_text(param, elem.second);
                    }
                }
            }
        }

        return true;
    }

    virtual bool VisitVarDecl(VarDecl *var) {
        if (!can_change)
            return true;

        // auto var_loc = var->getLocation();
        string var_type = var->getType().getAsString();


        for (auto elem : type_replace) {
            vector<int> var_lines;
            if (var_type == elem.first) {
                replace_text(var, elem.second, false);
                // need to do something, calling replace_text doesn't work except for func args
            }
        }

        return true;
    }

};



class ExampleASTConsumer : public ASTConsumer {
  private:
    ExampleVisitor *visitor; // doesn't have to be private

  public:
    // override the constructor in order to pass CI
    explicit ExampleASTConsumer(CompilerInstance *CI)
        : visitor(new ExampleVisitor(CI)) // initialize the visitor
    { }

    // override this to call our ExampleVisitor on the entire source file
    virtual void HandleTranslationUnit(ASTContext &Context) {
        /* we can use ASTContext to get the TranslationUnitDecl, which is
             a single Decl that collectively represents the entire source file */
        visitor->TraverseDecl(Context.getTranslationUnitDecl());
        rewriter.overwriteChangedFiles();
    }

    /*
        // override this to call our ExampleVisitor on each top-level Decl
        virtual bool HandleTopLevelDecl(DeclGroupRef DG) {
            // a DeclGroupRef may have multiple Decls, so we iterate through each one
            for (DeclGroupRef::iterator i = DG.begin(), e = DG.end(); i != e; i++) {
                Decl *D = *i;
                visitor->TraverseDecl(D); // recursively visit each AST node in Decl "D"
            }
            return true;
        }
    */
};



class ExampleFrontendAction : public ASTFrontendAction {
  public:
    virtual std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) {
        return std::unique_ptr<ASTConsumer>(new ExampleASTConsumer(&CI)); // pass CI pointer to ASTConsumer
    }
};


static llvm::cl::OptionCategory MyToolCategory("type-rename");

int main(int argc, const char **argv) {
    // parse the command-line args passed to your code
    CommonOptionsParser op(argc, argv, MyToolCategory);
    // create a new Clang Tool instance (a LibTooling environment)
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());

    // run the Clang Tool, creating a new FrontendAction (explained below)
    Tool.run(newFrontendActionFactory<ExampleFrontendAction>().get());

    // rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(errs());
    return EXIT_SUCCESS;
}
