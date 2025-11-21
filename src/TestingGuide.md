# Overview
For ease of development, this project makes use of test programs that run one or more tests against the target source files to verify expected functionality. Each test is a single source file with all necessary dependencies (often, basic C libs and all project header files).

# Creating a Test
1.) Make a new C file (myfile.c) with all necessary dependencies. When in doubt, just include all project header files along with any standard libraries.
2.) Update the test compilation script (called "compile\_tests.sh" at the time of writing) with the appropriate gcc command. Verify the following:
    a.) The test file itself is the first compiling argument, with all source file dependencies proceeding it.
    b.) ALL source file dependencies are included.
    c.) An output name is specified via '-o'. It is standard to simply name the compiled output after the source file (i.e. TEST\_Render.c compiles to TEST\_Render).
