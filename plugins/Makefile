CXX = g++
CXXFLAGS = -Wall -std=c++11 -fPIC

LIB_DIR = .
SRC_DIR = src
INCLUDE = -I ../include -I ../externals/EasyBMP/include/
OBJ_DIR = obj

.DEFAULT_GOAL := all


.PHONY: all
all: $(OBJ_DIR)/*.o $(LIB_DIR)/*.so

$(OBJ_DIR)/*.o:
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(SRC_DIR)/autocontrast.cpp -c -o $(OBJ_DIR)/autocontrast.o
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(SRC_DIR)/gray_world.cpp -c -o $(OBJ_DIR)/gray_world.o
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(SRC_DIR)/median.cpp -c -o $(OBJ_DIR)/median.o
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(SRC_DIR)/unsharp.cpp -c -o $(OBJ_DIR)/unsharp.o
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(SRC_DIR)/resize.cpp -c -o $(OBJ_DIR)/resize.o

$(LIB_DIR)/*.so:
	$(CXX) -shared -std=c++11 $(OBJ_DIR)/autocontrast.o -o $(LIB_DIR)/lib_autocontrast.so
	$(CXX) -shared -std=c++11 $(OBJ_DIR)/gray_world.o -o $(LIB_DIR)/lib_gray_world.so
	$(CXX) -shared -std=c++11 $(OBJ_DIR)/median.o -o $(LIB_DIR)/lib_median.so
	$(CXX) -shared -std=c++11 $(OBJ_DIR)/unsharp.o -o $(LIB_DIR)/lib_unsharp.so
	$(CXX) -shared -std=c++11 $(OBJ_DIR)/resize.o -o $(LIB_DIR)/lib_resize.so

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR)
	rm -f lib_gray_world.so lib_autocontrast.so lib_median.so lib_unsharp.so lib_resize.so
	mkdir -p $(OBJ_DIR)
	

