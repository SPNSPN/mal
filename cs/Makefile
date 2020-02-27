CSC := '/mnt/c/Windows/Microsoft.NET/Framework64/v4.0.30319/csc.exe'
LIB := 'C:\\Windows\\Microsoft.NET\\Framework64\\v4.0.30319,C:\\Windows\Microsoft.NET\\assembly\\GAC_MSIL'

Main.exe: Main.cs Interpreter.cs
	$(CSC) /lib:$(LIB) Main.cs Interpreter.cs

.PHONY: clean
clean:
	rm Main.exe
