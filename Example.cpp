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

pair<string, string> type_replace[5] = {{"unsigned int", "uint16_t"}, {"unsigned long", "uint32_t"}, {"int", "int16_t"}, {"long", "int32_t"}, {"double", "float"}};
vector<SourceLocation> locs;

template<typename T>
void replace_text(T input_decl, string r_text) {
    auto in_start = input_decl->getLocStart();
    if (find(locs.begin(), locs.end(), in_start) != locs.end()) {
        errs() << "skipping " << "\n";
        return;
    }
    locs.push_back(in_start);
    auto in_end = input_decl->getLocEnd();
    // gets the range of the total parameter - including type and argument
    auto range = CharSourceRange::getTokenRange(in_start, in_end);
    // get the stringPtr from the range and convert to string
    string s = string(Lexer::getSourceText(range, rewriter.getSourceMgr(), rewriter.getLangOpts()));
    // offset gives us the length of the argument, 1 is for the space
    int offset = Lexer::MeasureTokenLength(in_end, rewriter.getSourceMgr(), rewriter.getLangOpts()) + 1;
    // replace the text with the text sent
    rewriter.ReplaceText(in_start, s.length() - offset, r_text);
}

class ExampleVisitor : public RecursiveASTVisitor<ExampleVisitor> {
  private:
    ASTContext *astContext; // used for getting additional AST info

  public:
    explicit ExampleVisitor(CompilerInstance *CI)
        : astContext(&(CI->getASTContext())) { // initialize private members
        rewriter.setSourceMgr(astContext->getSourceManager(), astContext->getLangOpts());
    }

    virtual bool VisitFunctionDecl(FunctionDecl *func) {
        numFunctions++;
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
                // getLocStart gives us the ( location, it would seem
                auto bracket_loc = func->getNameInfo().getLocStart();
                // get the name length, add 1 for the space
                int name_length = func->getNameInfo().getAsString().length() + 1;
                // rewriter.ReplaceText(func->getLocStart(), retType.length(), "int16_t");
                // gets the range of the total parameter - including type and argument
                auto range_token = CharSourceRange::getTokenRange(begin_loc, bracket_loc);
                // get the stringPtr from the range and convert to string
                string s = string(Lexer::getSourceText(range_token, rewriter.getSourceMgr(), rewriter.getLangOpts()));
                // replace the text with the text sent
                rewriter.ReplaceText(begin_loc, s.length() - name_length, elem.second);
                // errs() << "** Rewrote function: " << funcName << "\n";
            }
        }
        if (params) {
            cout << "params in " << funcName << endl;
            for (int i = 0; i < params; ++i) {
                auto param = func->parameters()[i];
                arg = param->getType().getAsString();
                for (auto elem : type_replace) {
                    if (arg == elem.first) {
                        replace_text(param, elem.second);
                    }
                }
                cout << arg << endl;
            }
        }

        return true;
    }

    virtual bool VisitVarDecl(VarDecl *var) {

        // auto var_loc = var->getLocation();
        string var_type = var->getType().getAsString();


        for (auto elem : type_replace) {
            vector<int> var_lines;
            if (var_type == elem.first) {
                auto var_loc = var->getLocStart();
                // int line_nm = rewriter.getSourceMgr().getPresumedLoc(var_loc).getLine();
                // if the line_number exists, go to next iteration
                if (find(locs.begin(), locs.end(), var_loc) != locs.end()) {
                    errs() << "skipping " << "\n";
                    continue;
                }
                locs.push_back(var_loc);
                auto name_loc = var->getLocation(); // location of name
                int name_length = var_type.length();
                auto range_token = CharSourceRange::getTokenRange(var_loc, name_loc);
                string s = string(Lexer::getSourceText(range_token, rewriter.getSourceMgr(), rewriter.getLangOpts()));

                int offset = Lexer::MeasureTokenLength(name_loc, rewriter.getSourceMgr(), rewriter.getLangOpts()) + 1;
                rewriter.ReplaceText(var_loc, s.length() - offset, elem.second);
                errs() << "s is: " << s << " length is: " << s.length() << "\n";
                // need to do something, calling replace_text doesn't work except for func args
            }
        }

        return true;
    }

    // virtual bool VisitStmt(Stmt *st) {
    //     if (ReturnStmt *ret = dyn_cast<ReturnStmt>(st)) {
    //         rewriter.ReplaceText(ret->getRetValue()->getLocStart(), 6, "val");
    //         errs() << "** Rewrote ReturnStmt\n";
    //     }
    //     if (CallExpr *call = dyn_cast<CallExpr>(st)) {
    //         rewriter.ReplaceText(call->getLocStart(), 7, "add5");
    //         errs() << "** Rewrote function call\n";
    //     }
    //     return true;
    // }

    /*
        virtual bool VisitReturnStmt(ReturnStmt *ret) {
            rewriter.ReplaceText(ret->getRetValue()->getLocStart(), 6, "val");
            errs() << "** Rewrote ReturnStmt\n";
            return true;
        }

        virtual bool VisitCallExpr(CallExpr *call) {
            rewriter.ReplaceText(call->getLocStart(), 7, "add5");
            errs() << "** Rewrote function call\n";
            return true;
        }
    */
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


static llvm::cl::OptionCategory MyToolCategory("");

int main(int argc, const char **argv) {
    // parse the command-line args passed to your code
    CommonOptionsParser op(argc, argv, MyToolCategory);
    // create a new Clang Tool instance (a LibTooling environment)
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());

    // run the Clang Tool, creating a new FrontendAction (explained below)
    int result = Tool.run(newFrontendActionFactory<ExampleFrontendAction>().get());

    errs() << "\nFound " << numFunctions << " functions.\n\n";
    // print out the rewritten source code ("rewriter" is a global var.)
    rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(errs());
    return result;
}
