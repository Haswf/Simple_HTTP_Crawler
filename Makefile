all:
	export CXX=$(which gcc)
	cmake .
	cmake --build ./