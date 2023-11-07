set -e
docker build --platform=linux/amd64 -t nox-x86_64  -f docker/ubuntu.Dockerfile .
docker build --platform=linux/arm64 -t nox-aarch64 -f docker/ubuntu.Dockerfile .

echo "\n ---- Running x86_64 ----"
docker run --platform=linux/amd64 -it nox-x86_64
echo "\n ---- Running aarch64 ----"
docker run --platform=linux/arm64 -it nox-aarch64
