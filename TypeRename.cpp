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
bool can_change = false;

FileID main_file;

// all the types we want to replace - I have added a space so the whitespace is controlled
// by the pair below and not the source file
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

// a vector to store changed locations
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

class VarVisitor : public RecursiveASTVisitor<VarVisitor> {
  private:
    ASTContext *astContext; // used for getting additional AST info

  public:
    explicit VarVisitor(CompilerInstance *CI)
        : astContext(&(CI->getASTContext())) { // initialize private members
        SourceManager &SM = astContext->getSourceManager();
        rewriter.setSourceMgr(SM, astContext->getLangOpts());
        main_file = SM.getMainFileID();

    }

    virtual bool VisitFunctionDecl(FunctionDecl *func) {
        FileID curr_file = astContext->getSourceManager().getFileID(func->getLocation());
        if (!can_change && curr_file == main_file) 
            can_change = true;
        if (!can_change)
            return true;
        string retType = func->getReturnType().getAsString();
        string funcName = func->getNameInfo().getName().getAsString();
        string arg;
        int params = func->getNumParams();

        // SourceLocation func_start = func->getLocStart();
        SourceLocation param_start;
        SourceLocation param_end;
        for (auto elem : type_replace) {
            if (retType == elem.first) {
                auto begin_loc = func->getLocStart();
                // if starting location is found - we have replaced already - move along!
                if (find(varaible_locations.begin(), varaible_locations.end(), begin_loc) != varaible_locations.end())
                    continue;
                varaible_locations.push_back(begin_loc);
                // getLocStart gives us the ( location, it would seem
                auto bracket_loc = func->getNameInfo().getLocStart();
                // get the name length
                int name_length = func->getNameInfo().getAsString().length();
                // gets the range of the total parameter - including type and argument
                auto range_token = CharSourceRange::getTokenRange(begin_loc, bracket_loc);
                // get the stringPtr from the range and convert to string
                string s = string(Lexer::getSourceText(range_token, rewriter.getSourceMgr(), rewriter.getLangOpts()));
                // replace the text with the text sent
                rewriter.ReplaceText(begin_loc, s.length() - name_length, elem.second);
            }
        }
        // if there are function parameters, find each one and replace if necessary
        if (params) {
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

        string var_type = var->getType().getAsString();

        // visit each variable declaration and replace if necessary
        for (auto elem : type_replace) {
            if (var_type == elem.first) {
                replace_text(var, elem.second, false);
            }
        }
        return true;
    }

};



class VarASTConsumer : public ASTConsumer {
  private:
    VarVisitor *visitor; // doesn't have to be private

  public:
    // override the constructor in order to pass CI
    explicit VarASTConsumer(CompilerInstance *CI)
        : visitor(new VarVisitor(CI)) // initialize the visitor
    { }

    // override this to call our VarVisitor on the entire source file
    virtual void HandleTranslationUnit(ASTContext &Context) {
        /* we can use ASTContext to get the TranslationUnitDecl, which is
             a single Decl that collectively represents the entire source file */
        visitor->TraverseDecl(Context.getTranslationUnitDecl());
        rewriter.overwriteChangedFiles();
    }

};



class VarFrontendAction : public ASTFrontendAction {
  public:
    virtual std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) {
        return std::unique_ptr<ASTConsumer>(new VarASTConsumer(&CI)); // pass CI pointer to ASTConsumer
    }
};


static llvm::cl::OptionCategory MyToolCategory("type-rename");

int main(int argc, const char **argv) {
    // parse the command-line args passed to your code
    CommonOptionsParser op(argc, argv, MyToolCategory);
    // create a new Clang Tool instance (a LibTooling environment)
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());

    // run the Clang Tool, creating a new FrontendAction (explained below)
    Tool.run(newFrontendActionFactory<VarFrontendAction>().get());

    // rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(errs());
    return EXIT_SUCCESS;
}
