javac -cp antlrworks-1.2.3.jar;jogl.jar;gluegen-rt.jar;. SimpleGLSLEffectCompiler.java
copy *.dll ..\bin\
..\..\apache-ant-1.8.0\bin\ant -buildfile runnableJARexport.xml

