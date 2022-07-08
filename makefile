EXE=a.exe

all: $(EXE)

$(EXE): main.cpp makefile
	cl main.cpp /I"C:\Program Files\Pleora Technologies Inc\eBUS SDK\\Includes" /std:c++20 /EHsc    \
	/link /OUT:$(EXE)  /LIBPATH:"C:\Program Files\Pleora Technologies Inc\eBUS SDK\\Libraries"  

run: $(EXE)
	./$(EXE)
