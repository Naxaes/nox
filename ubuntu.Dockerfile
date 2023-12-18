FROM ubuntu:latest AS build

# Set the working directory
WORKDIR /app

# Update the package list and install software as root
USER root

# Update apps on the base image
RUN apt-get -y update && apt-get install -y

# Install the Clang compiler
RUN apt-get -y install clang

# Install the llvm for diassemble tools
RUN apt-get -y install gdb

# Install the Clang compiler
RUN apt-get -y install cmake

# Install vim so we can edit files if we log in to the container.
RUN apt-get -y install vim

# Make sure we have the pthread library (required for Google Test)
RUN apt-get -y install libpthread-stubs0-dev

# Install the lldb for debugging in the Docker container
RUN apt-get -y install lldb

# Create a non-root user
RUN adduser nox

# Make the build directory
RUN mkdir -p build

# Copy our tests
COPY tests tests

# Copy your C program source code and any required files
COPY CMakeLists.txt .

# Copy your C program source code and any required files
COPY src src

# Copy our example nox programs
COPY examples examples

# Move into the build directory
WORKDIR /app/build

# Compile the C program
RUN cmake ..
RUN cmake --build .

WORKDIR /app

# Run the C program
#CMD ["./build/nox", "run", "examples/adding.nox"]
#CMD ["./build/nox", "run", "examples/arithmetic.nox"]
#CMD ["./build/nox", "run", "examples/assignment.nox"]
#CMD ["./build/nox", "run", "examples/check-error.nox"]
#CMD ["./build/nox", "run", "examples/conditions.nox"]
#CMD ["./build/nox", "run", "examples/fun-call.nox"]
#CMD ["./build/nox", "run", "examples/fun-decl.nox"]
#CMD ["./build/nox", "run", "examples/identifier.nox"]
#CMD ["./build/nox", "run", "examples/if-else-stmt.nox"]
#CMD ["./build/nox", "run", "examples/if-stmt.nox"]
#CMD ["./build/nox", "run", "examples/lex-error.nox"]
#CMD ["./build/nox", "run", "examples/parse-error.nox"]
#CMD ["./build/nox", "run", "examples/program.nox"]
#CMD ["./build/nox", "run", "examples/single_digit.nox"]
#CMD ["./build/nox", "run", "examples/string.nox"]
#CMD ["./build/nox", "run", "examples/structs.nox"]
CMD ["./build/nox", "run", "examples/while-loop.nox"]
