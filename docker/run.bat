@echo off
setlocal

:: Build the x86_64 Docker image
docker build --platform=linux/amd64 -t nox-x86_64 -f docker\ubuntu.Dockerfile .

:: Build the aarch64 Docker image
docker build --platform=linux/arm64 -t nox-aarch64 -f docker\ubuntu.Dockerfile .

:: Run the x86_64 container
docker run --platform=linux/amd64 -it nox-x86_64

:: Run the aarch64 container
docker run --platform=linux/arm64 -it nox-aarch64

endlocal
