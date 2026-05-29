#!/usr/bin/env bash
set -euo pipefail

gcc -std=c11 -Wall -Wextra -O0 -g ./*.c -o kata_tests
./kata_tests
