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

FileID main_file;

const int num_types = 40;

std::vector<std::pair<std::string, std::string>> types_to_replace;

std::array<std::string, 3> prefixes = {"const ", "volatile ", "const volatile "};


std::array<std::string, 10> suffixes = {"*", "&", "**", "&&", "***", "*const", "&const", "**const", "&&const" "***const"};
// all the types we want to replace - I have added a space so the whitespace is controlled
// by the pair below and not the source file
std::pair<std::string, std::string> base_replace[5] = {
    {"unsigned int", "uint16_t "},
    {"unsigned long", "uint32_t "},
    {"int", "int16_t "},
    {"long", "int32_t "},
    {"double", "float "},
};



void fill_types() {
    for (auto elem : base_replace) {
        types_to_replace.push_back(elem);
        for (auto s : suffixes)
            types_to_replace.push_back(make_pair(elem.first + " " + s, elem.second + s + " "));
        for (auto pre : prefixes) {
            std::string pre_first = pre + elem.first;
            std::string pre_second = pre + elem.second;
            types_to_replace.push_back(make_pair(pre_first, pre_second));
            for (auto suf : suffixes) {
                std::string s_first = pre_first + " " + suf;
                std::string s_second = pre_second + suf + " ";
                types_to_replace.push_back(make_pair(s_first, s_second));
            }
        }
    }


}

void print_types() {
    cout << "\n\n" << "Printing types...." << endl;
    for (auto elem : types_to_replace) {
        cout << elem.first << "\t" << elem.second << endl;
    }
}

// a vector to store changed locations
vector<SourceLocation> variable_locations;


// Replace the variable type for different input decl types
template<typename T>
void replace_text(T input_decl, std::string r_text, bool end = true) {
    auto in_start = input_decl->getLocStart();
    // setting it to in_start enables the use of auto for different input decls we use
    auto in_end = in_start;

    // if we have already replaced it, skip
    if (find(variable_locations.begin(), variable_locations.end(), in_start) != variable_locations.end())
        return;
    variable_locations.push_back(in_start);
    // if we want the locEnd or getLocation function - depends on decl type
    if (end)
        in_end = input_decl->getLocEnd();
    else
        in_end = input_decl->getLocation();

    // gets the range of the total parameter - including type and argument
    auto range = CharSourceRange::getTokenRange(in_start, in_end);

    // get the stringPtr from the range and convert to std::string
    std::string s = std::string(Lexer::getSourceText(range, rewriter.getSourceMgr(), rewriter.getLangOpts()));

    // offset gives us the length
    int offset = Lexer::MeasureTokenLength(in_end, rewriter.getSourceMgr(), rewriter.getLangOpts());
    // replace the text with the text sent
    rewriter.ReplaceText(in_start, s.length() - offset, r_text);
}

class VarVisitor : public RecursiveASTVisitor<VarVisitor> {
  private:
    ASTContext *astContext; // used for getting additional AST info

    template<typename T>
    bool not_in_main(T v) {
        FileID curr_file = astContext->getSourceManager().getFileID(v->getLocStart());
        if (curr_file != main_file)
            return true;
        return false;
    }

  public:
    explicit VarVisitor(CompilerInstance *CI)
        : astContext(&(CI->getASTContext())) { // initialize private members
        SourceManager &SM = astContext->getSourceManager();
        rewriter.setSourceMgr(SM, astContext->getLangOpts());
        main_file = SM.getMainFileID();

    }

    virtual bool VisitFunctionDecl(FunctionDecl *func) {
        if (not_in_main(func))
            return true;
        std::string retType = func->getReturnType().getAsString();
        // errs() << "found function with returnType: " << retType << "\n";
        std::string funcName = func->getNameInfo().getName().getAsString();
        std::string arg;
        int params = func->getNumParams();

        // SourceLocation func_start = func->getLocStart();
        SourceLocation param_start;
        SourceLocation param_end;

        for (auto elem : types_to_replace) {
            if (retType == elem.first) {
                auto begin_loc = func->getLocStart();
                // if starting location is found - we have replaced already - move along!
                if (find(variable_locations.begin(), variable_locations.end(), begin_loc) != variable_locations.end())
                    continue;
                variable_locations.push_back(begin_loc);
                // getLocStart gives us the ( location, it would seem
                auto bracket_loc = func->getNameInfo().getLocStart();
                // get the name length
                int name_length = func->getNameInfo().getAsString().length();
                // gets the range of the total parameter - including type and argument
                auto range_token = CharSourceRange::getTokenRange(begin_loc, bracket_loc);
                // get the stringPtr from the range and convert to std::string
                std::string s = std::string(Lexer::getSourceText(range_token, rewriter.getSourceMgr(), rewriter.getLangOpts()));
                // replace the text with the text sent
                rewriter.ReplaceText(begin_loc, s.length() - name_length, elem.second);
            }
        }
        // if there are function parameters, find each one and replace if necessary
        if (params) {
            for (int i = 0; i < params; ++i) {
                auto param = func->parameters()[i];
                arg = param->getType().getAsString();
                for (auto elem : types_to_replace) {
                    if (arg == elem.first) {
                        replace_text(param, elem.second);
                    }
                }
            }
        }

        return true;
    }

    virtual bool VisitVarDecl(VarDecl *var) {
        if (not_in_main(var))
            return true;

        std::string var_type = var->getType().getAsString();
        bool is_static = var->isStaticLocal();
        const Type *type = var->getType().getTypePtr();
        bool is_array = type->isArrayType();



        if (is_array) {
            const ArrayType *arr = type->getAsArrayTypeUnsafe();
            var_type = arr->getElementType().getAsString();
        }

        // visit each variable declaration and replace if necessary
        for (auto elem : types_to_replace) {
            if (var_type == elem.first) {
                replace_text(var, elem.second, false);
            }
        }
        return true;
    }

    virtual bool VisitFieldDecl(FieldDecl *field) {
        if (not_in_main(field))
            return true;

        std::string var_type = field->getType().getAsString();
        // bool is_static = field->isStaticLocal();
        const Type *type = field->getType().getTypePtr();
        bool is_array = type->isArrayType();



        if (is_array) {
            const ArrayType *arr = type->getAsArrayTypeUnsafe();
            var_type = arr->getElementType().getAsString();
        }

        // visit each variable declaration and replace if necessary
        for (auto elem : types_to_replace) {
            if (var_type == elem.first) {
                replace_text(field, elem.second, false);
            }
        }
        return true;
    }

    // this works for statc_cast<int> dynamic_cast etc etc.
    virtual bool VisitCXXNamedCastExpr(CXXNamedCastExpr *cast) {
        if (not_in_main(cast))
            return true;
        auto loc = cast->getAngleBrackets();
        auto in_start = cast->getLocStart();
        // setting it to in_start enables the use of auto for different input decls we use
        auto in_end = cast->getLocEnd();
        auto loc_begin = loc.getBegin();
        auto loc_end = loc.getEnd();
        SourceLocation var_start_loc = loc.getBegin().getLocWithOffset(1);
        SourceLocation var_end_loc = loc.getEnd().getLocWithOffset(-1);

        // // if we have already replaced it, skip
        if (find(variable_locations.begin(), variable_locations.end(), var_start_loc) != variable_locations.end())
            return true;
        variable_locations.push_back(var_start_loc);
        // if we want the locEnd or getLocation function - depends on decl type

        // gets the range of the total parameter - including type and argument
        auto range = CharSourceRange::getTokenRange(var_start_loc, var_end_loc);

        // get the stringPtr from the range and convert to std::string
        std::string type_name = std::string(Lexer::getSourceText(range, rewriter.getSourceMgr(), rewriter.getLangOpts()));
        for (auto type : types_to_replace) {
            if (type_name == type.first) {
                rewriter.ReplaceText(var_start_loc, type_name.length(), type.second);

            }
        }
        return true;
    }

    virtual bool VisitCStyleCastExpr(CStyleCastExpr *cast) {
        if (not_in_main(cast))
            return true;
        string type_name = cast->getTypeAsWritten().getAsString();
        for (auto t : types_to_replace) {
            if (type_name == t.first) {
                SourceLocation loc_start = cast->getTypeInfoAsWritten()->getTypeLoc().getLocStart();
                SourceLocation loc_end = cast->getTypeInfoAsWritten()->getTypeLoc().getLocStart();

                auto range = CharSourceRange::getTokenRange(loc_start, loc_end);
                if (find(variable_locations.begin(), variable_locations.end(), loc_start) != variable_locations.end())
                    continue;
                variable_locations.push_back(loc_start);
                rewriter.ReplaceText(loc_start, type_name.length(), t.second);
                // get the stringPtr from the range and convert to std::string


            }

        }
        return true;
    }

    virtual bool VisitCXXConstructExpr(CXXConstructExpr *expr) {
        if (not_in_main(expr)) {
            return true;
        }
        int num_args = expr->getNumArgs();
        auto type = expr->getType().getAsString();
        if (num_args) {
            // for(auto i = expr->arg_begin();i != expr->arg_end(); ++i) {
            //     arg = *i->getType().getAsString();
            //     for (auto elem : types_to_replace) {
            //         if (arg == elem.first) {
            //             //replace_text(param, elem.second);
            //         }
            //     }
            // }

        }
        return true;
    }

    // virtual bool VisitParmVarDecl(ParmVarDecl *parm) {
    //     if (not_in_main(parm)) {
    //         return true;
    //     }
    //     auto type = parm->getType().getAsString();

    //     return true;
    // }

    // virtual bool VisitDeclRefExpr(DeclRefExpr *expr) {
    //     if (not_in_main(expr))
    //         return true;

    //     return true;

    // }

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
        // rewriter.overwriteChangedFiles();
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
    fill_types();
    // print_types();
    // parse the command-line args passed to your code
    CommonOptionsParser op(argc, argv, MyToolCategory);
    // create a new Clang Tool instance (a LibTooling environment)
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());

    // run the Clang Tool, creating a new FrontendAction (explained below)
    Tool.run(newFrontendActionFactory<VarFrontendAction>().get());

    rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(errs());
    return EXIT_SUCCESS;
}
