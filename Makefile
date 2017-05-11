OPT = -std=c++11 -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS
INC = -I/usr/lib/llvm-3.8/include
EXE=Example

LIB = -fno-rtti -lclang \
    -lclangFrontend -lclangDriver -lclangTooling \
    -lclangDriver -lclangTooling -lclangFrontendTool \
    -lclangFrontend -lclangDriver -lclangSerialization \
    -lclangCodeGen -lclangParse -lclangSema \
    -lclangStaticAnalyzerFrontend -lclangStaticAnalyzerCheckers \
    -lclangStaticAnalyzerCore -lclangAnalysis \
    -lclangARCMigrate -lclangRewriteFrontend \
    -lclangEdit  -lclangAST -lclangRewrite \
    -lclangLex -lclangBasic  -lclangCodeGen -lclangSema \
    -lclangAnalysis  -lclangParse -lclangLex \
    -lclangASTMatchers -lclangIndex  \
    -lclangBasic  -lLLVMSupport  -lpthread

all: $(EXE) 


$(EXE): $(EXE).cpp
	clang++ -o $(EXE) $(EXE).cpp $(OPT) $(INC) -g $(LIB) \
    `llvm-config-3.8 --cxxflags --ldflags --libs all --system-libs`

clean:
	rm $(EXE) *.o *.dwo
