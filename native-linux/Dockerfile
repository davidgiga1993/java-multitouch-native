# Builds the Linux/X11 multitouch native library (libMtLinux64.so).
# Keeping the build inside a container means the .so is always linked against a
# known glibc/libX11/libXi ABI instead of whatever happens to be on the host.
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
		build-essential \
		cmake \
		libx11-dev \
		libxi-dev \
		openjdk-25-jdk \
	&& rm -rf /var/lib/apt/lists/*

ENV JAVA_HOME=/usr/lib/jvm/java-25-openjdk-amd64

WORKDIR /build
COPY . .

RUN cmake -S . -B out -DCMAKE_BUILD_TYPE=Release \
	&& cmake --build out --config Release -j"$(nproc)"

# Minimal export stage so `docker build --target export --output type=local,dest=<dir> .`
# copies out just the built .so, without needing a throwaway container.
FROM scratch AS export
COPY --from=builder /build/out/libMtLinux64.so /libMtLinux64.so
