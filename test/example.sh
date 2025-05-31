#!/bin/bash

set -e || exit $?
set -o pipefail
_test/example | diff _bin/embed -

