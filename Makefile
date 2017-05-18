OPT = -std=c++11
INC = -I/usr/lib/llvm-3.8/include
EXE = type-rename
SRC = TypeRename.cpp
CXX ?= clang++

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


$(EXE): $(SRC)
	$(CXX) -o $(EXE) $(SRC) $(OPT) $(INC) -g $(LIB) \
    `llvm-config-3.8 --ldflags --libs all --system-libs`

clean:
	rm -f $(EXE) *.dwo
