docker build --platform=linux/amd64 -t nox-x86_64  -f ubuntu.Dockerfile .
docker build --platform=linux/arm64 -t nox-aarch64 -f ubuntu.Dockerfile .

# The following is needed to run lldb in docker:
# --cap-add=SYS_PTRACE --security-opt seccomp=unconfined

echo "\n ---- Running x86_64 ----"
docker run --platform=linux/amd64 -it nox-x86_64
echo "\n ---- Running aarch64 ----"
docker run --platform=linux/arm64 -it nox-aarch64
