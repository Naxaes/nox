

# Running the docker container
Make sure you're in the project root directory and run the following command:
```shell
docker build --platform=linux/amd64 -t nox-x86_64  -f ubuntu.Dockerfile .
docker build --platform=linux/arm64 -t nox-aarch64 -f ubuntu.Dockerfile .
docker run -it nox-x86_64
docker run -it nox-aarch64
```


## Multi-stage build
```shell
docker build --platform=linux/amd64 -t nox-x86_64  -f ubuntu.Dockerfile --target build .
docker build --platform=linux/arm64 -t nox-aarch64 -f ubuntu.Dockerfile --target build .
```
