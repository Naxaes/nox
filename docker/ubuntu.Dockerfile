FROM ubuntu:latest AS build

# Set the working directory
WORKDIR /app

# Update the package list and install software as root
USER root

# Update apps on the base image
RUN apt-get -y update && apt-get install -y

# Install the Clang compiler
RUN apt-get -y install clang

# Install the Clang compiler
RUN apt-get -y install cmake

# Install vim so we can edit files if we log in to the container.
RUN apt-get -y install vim

# Create a non-root user
RUN adduser nox

# Make the build directory
RUN mkdir -p build

# Copy your C program source code and any required files
COPY ../CMakeLists.txt .

# Copy your C program source code and any required files
COPY ../src src

# Copy our example nox programs
COPY ../examples examples

WORKDIR /app/build

# Compile the C program
RUN cmake .. && make

WORKDIR /app

# Run the C program
CMD ["./build/nox"]
