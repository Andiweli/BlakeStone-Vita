#!/usr/bin/env bash

set -Eeuo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
VITA_TEST="${VITA_TEST:-OFF}"
VITA_SHOW_WARNINGS="${VITA_SHOW_WARNINGS:-OFF}"
WSL_CLOCK_SKEW_DELAY="${WSL_CLOCK_SKEW_DELAY:-6}"
CLEAN_BUILD=1

usage() {
  cat <<'EOF'
Usage: ./build_vita.sh [--debug] [--warnings] [--incremental]

By default the script performs a completely clean build. This prevents object
files from earlier patched versions from being reused.

Options:
  --debug        Build with CMAKE_BUILD_TYPE=Debug and VITA_TEST=ON.
  --warnings     Show compiler warnings from the original sources.
  --incremental  Keep the existing build directory.
  -h, --help     Show this help text.

Environment variables:
  VITASDK              Required path to the installed VitaSDK.
  BUILD_DIR            Build directory (default: ./build).
  BUILD_TYPE           CMake build type (default: Release).
  VITA_TEST            Vita test mode, ON or OFF (default: OFF).
  VITA_SHOW_WARNINGS   Show warnings, ON or OFF (default: OFF).
  JOBS                 Number of parallel build jobs.
  WSL_CLOCK_SKEW_DELAY Seconds to wait on /mnt drives (default: 6).
EOF
}

while (($# > 0)); do
  case "$1" in
    --debug)
      BUILD_TYPE="Debug"
      VITA_TEST="ON"
      ;;
    --warnings)
      VITA_SHOW_WARNINGS="ON"
      ;;
    --incremental)
      CLEAN_BUILD=0
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
  shift
done

if [[ -z "${VITASDK:-}" ]]; then
  echo "Error: VITASDK is not set." >&2
  echo "Example: export VITASDK=/usr/local/vitasdk" >&2
  exit 1
fi

if [[ ! -f "${VITASDK}/share/vita.toolchain.cmake" ]]; then
  echo "Error: VitaSDK toolchain not found at ${VITASDK}/share/vita.toolchain.cmake" >&2
  exit 1
fi

if [[ ! -f "${VITASDK}/share/vita.cmake" ]]; then
  echo "Error: VitaSDK CMake helpers not found at ${VITASDK}/share/vita.cmake" >&2
  exit 1
fi

export PATH="${VITASDK}/bin:${PATH}"

for command_name in cmake arm-vita-eabi-g++; do
  if ! command -v "${command_name}" >/dev/null 2>&1; then
    echo "Error: ${command_name} is not available in PATH." >&2
    exit 1
  fi
done

if [[ -z "${JOBS:-}" ]]; then
  if command -v nproc >/dev/null 2>&1; then
    JOBS="$(nproc)"
  elif command -v getconf >/dev/null 2>&1; then
    JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"
  else
    JOBS=4
  fi
fi

if ! [[ "${JOBS}" =~ ^[1-9][0-9]*$ ]]; then
  echo "Error: JOBS must be a positive integer." >&2
  exit 1
fi

if [[ "${CLEAN_BUILD}" == "1" ]]; then
  echo "Removing previous build directory: ${BUILD_DIR}"
  rm -rf "${BUILD_DIR}"
fi

mkdir -p "${BUILD_DIR}"

printf 'Configuring original Blake Stone Vita source\n'
printf '  VitaSDK:    %s\n' "${VITASDK}"
printf '  Build dir:  %s\n' "${BUILD_DIR}"
printf '  Build type: %s\n' "${BUILD_TYPE}"
printf '  Vita test:  %s\n' "${VITA_TEST}"
printf '  Warnings:   %s\n' "${VITA_SHOW_WARNINGS}"
printf '  Jobs:       %s\n' "${JOBS}"

cmake -S "${ROOT_DIR}/src/vita" -B "${BUILD_DIR}" \
  -DCMAKE_TOOLCHAIN_FILE="${VITASDK}/share/vita.toolchain.cmake" \
  -DVITASDK="${VITASDK}" \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DVITA_TEST="${VITA_TEST}" \
  -DVITA_SHOW_WARNINGS="${VITA_SHOW_WARNINGS}" \
  -DCMAKE_WARN_DEPRECATED=OFF \
  -DCMAKE_SUPPRESS_DEVELOPER_WARNINGS=ON

# NTFS-mounted WSL directories may expose newly generated files a few seconds
# in the future. Wait before Make reads the generated timestamps.
if [[ "${BUILD_DIR}" == /mnt/* ]]; then
  printf 'Waiting %s seconds for WSL timestamp synchronization...\n' "${WSL_CLOCK_SKEW_DELAY}"
  sleep "${WSL_CLOCK_SKEW_DELAY}"
fi

cmake --build "${BUILD_DIR}" --parallel "${JOBS}"

VPK_PATH="${BUILD_DIR}/bstone.vpk"
SELF_PATH="${BUILD_DIR}/bstone.self"

if [[ ! -f "${VPK_PATH}" ]]; then
  echo "Error: Build completed without producing ${VPK_PATH}" >&2
  exit 1
fi

printf '\nBuild successful.\n'
printf '  VPK:  %s\n' "${VPK_PATH}"
if [[ -f "${SELF_PATH}" ]]; then
  printf '  SELF: %s\n' "${SELF_PATH}"
fi
