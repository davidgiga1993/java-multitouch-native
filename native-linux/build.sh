#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

OUTPUT_DIR="${1:-build}"
mkdir -p "$OUTPUT_DIR"

if docker buildx version >/dev/null 2>&1; then
	docker buildx build --target export --output "type=local,dest=${OUTPUT_DIR}" .
else
	docker build --target export -t mt-linux-export .
	CONTAINER_ID=$(docker create mt-linux-export)
	docker cp "${CONTAINER_ID}:/libMtLinux64.so" "${OUTPUT_DIR}/libMtLinux64.so"
	docker rm "${CONTAINER_ID}" > /dev/null
fi

echo "Built ${OUTPUT_DIR}/libMtLinux64.so"
