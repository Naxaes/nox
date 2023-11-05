# Setting up GitHub Actions

Github actions allow us to run scripts on GitHub's servers. This is useful for running tests, building our project, and deploying our project. We'll use it to be sure it works on different platforms before merging a pull request into our main branch.

## Creating a workflow

Create a file named `.github/workflows/build.yml`.

First give it a name. I'll name it "Build and Run".
```yaml
name: Build and Run
```

Then specify when you want this script to run. We want it to run when making a `pull_request` to `main`.
```yaml
on:
  pull_request:
    branches: [ main ]
```

We'll also want CMake to build in release mode. We can specify this by setting the `CMAKE_BUILD_TYPE` environment variable in the `env` section.
```yaml
env:
  CMAKE_BUILD_TYPE: Release
```


Then specify the jobs you want to run. We want to run the `build` job.
```yaml
jobs:
  build:
```

Under the `build` job, we can start specifying different attributes for it, like `name`, what it `runs-on` and the steps to take. Since we want it to run on different platforms, we can use a matrix strategy to define variables for different jobs to run in parallel. These variables can then be referenced elsewhere in our build script.

Let's start defining our strategy matrix. Each job will have a name, its os, and how to run the executable. We'll also specify `fail-fast: false` so that all jobs will run even if one of them fails.
```yaml
strategy:
  fail-fast: false
  matrix:
    config:
      - {
        name: "Windows x86-64",
        os: windows-latest,
        run: build\Release\nox.exe,
      }
      - {
        name: "Ubuntu x86_64",
        os: ubuntu-latest,
        run: ./build/nox,
      }
      - {
        name: "MacOSX x86_64",
        os: macos-latest,
        run: ./build/nox,
      }
```
All platforms currently supported are x86_64. In the future, we might see if we can use Docker in our jobs to support different architectures.

Now we can start define metadata for our job. We can give it a name, and specify what platform it should run on by using our matrix variables.
```yaml
name: ${{ matrix.config.name }}
runs-on: ${{ matrix.config.os }}
```

Then we need to specify the steps a job should take.
```yaml
steps:
  # Fetch our repository.
  - uses: actions/checkout@v2

  # Create our build directory.
  - name: Create build directory
    run: mkdir build

  # Change to our build directory and run cmake.
  - name: Configure
    working-directory: ./build
    run: cmake ..

  # Build the project.
  - name: Build
    working-directory: ./build
    run: cmake --build .

  # Change to our project root run the executable.
  - name: Run
    working-directory: .
    run: ${{ matrix.config.run }}
```


## Full script
```yaml
name: Build and Run

on:
  pull_request:
    branches: [ main ]

env:
  CMAKE_BUILD_TYPE: Release

jobs:
  build:
    strategy:
      fail-fast: false
      matrix:
        config:
          - {
            name: "Windows x86-64",
            os: windows-latest,
            run: build\Release\nox.exe,
          }
          - {
            name: "Ubuntu x86_64",
            os: ubuntu-latest,
            run: ./build/nox,
          }
          - {
            name: "MacOSX x86_64",
            os: macos-latest,
            run: ./build/nox,
          }
    name: ${{ matrix.config.name }}
    runs-on: ${{ matrix.config.os }}
    steps:
      # Fetch our repository.
      - uses: actions/checkout@v2

      # Create our build directory.
      - name: Create build directory
        run: mkdir build

      # Change to our build directory and run cmake.
      - name: Configure
        working-directory: ./build
        run: cmake ..

      # Build the project.
      - name: Build
        working-directory: ./build
        run: cmake --build .

      # Change to our project root run the executable.
      - name: Run
        working-directory: .
        run: ${{ matrix.config.run }}
```