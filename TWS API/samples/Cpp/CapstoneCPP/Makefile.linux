CXX=g++
CXXFLAGS=-pthread -Wall -Wno-switch -Wpedantic -Wno-unused-function -std=c++11
ROOT_DIR=../../../source/cppclient
BASE_SRC_DIR=${ROOT_DIR}/client
INCLUDES=-I${BASE_SRC_DIR} -I${ROOT_DIR}
SOURCE_DIR=${BASE_SRC_DIR}
SOURCE_LIB=libTwsSocketClient.so
LIB_DIR=$(SOURCE_DIR)/lib
LIB_NAME_A=libbid.a
LIB_NAME_SO=libbid.so
TARGET=TestCppClient

$(TARGET)Static:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(BASE_SRC_DIR)/*.cpp ./*.cpp $(LIB_DIR)/$(LIB_NAME_A) -o$(TARGET)Static

$(TARGET):
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(BASE_SRC_DIR)/*.cpp ./*.cpp -L$(LIB_DIR) -l:$(LIB_NAME_SO) -o$(TARGET)

$(TARGET)Dynamic:
	$(CXX) $(CXXFLAGS) $(INCLUDES)  ./*.cpp -L$(LIB_DIR) -l:$(LIB_NAME_SO) -L$(SOURCE_DIR) -l:$(SOURCE_LIB) -o$(TARGET)Dynamic

run$(TARGET)Static:
	./$(TARGET)Static

run$(TARGET):
	LD_LIBRARY_PATH=$(PWD)/$(LIB_DIR):$(LD_LIBRARY_PATH) ./$(TARGET)

run$(TARGET)Dynamic:
	LD_LIBRARY_PATH=$(PWD)/$(LIB_DIR):$(PWD)/$(SOURCE_DIR):$(LD_LIBRARY_PATH) ./$(TARGET)Dynamic

clean:
	rm -f $(TARGET) $(TARGET)Static $(TARGET)Dynamic *.o

