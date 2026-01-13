CC = gcc
CXX = g++
CFLAGS = -Wall -g -I../OldCompiler/Util -I../OldCompiler/Parser -I../OldCompiler/Output -I../OldCompiler/Lexer
CXXFLAGS = -std=c++17 -Wall -g $(CFLAGS) -I./AST -I./Semantics -I./Bridge

OLD_DIR = ../OldCompiler
OLD_UTIL = $(OLD_DIR)/Util
OLD_OUT = $(OLD_DIR)/Output

# Objetos C (Antigos + Legacy Support)
C_OBJS = $(OLD_UTIL)/Util.o \
         $(OLD_UTIL)/Logger.o \
         $(OLD_OUT)/lex.yy.o \
         $(OLD_OUT)/Parser.tab.o \
         Bridge/LegacySupport.o

CPP_OBJS = Bridge/ASTBuilder.o main.o

TARGET = RocketC_Refactor
TEST_FILE = TestSemantics.c

all: $(TARGET)

$(TARGET): $(CPP_OBJS) $(C_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

run: $(TARGET)
	./$(TARGET) $(TEST_FILE)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) *.o Bridge/*.o Semantics/*.o