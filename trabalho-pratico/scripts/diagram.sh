#!/bin/sh

# This script generates a dependency diagram for the application. Unlike in the
# report, there is no distinction between data and function modules. This script
# serves merely as an helper for drawing those diagrams manually without
# mistakes.
#

# Copyright 2023 Humberto Gomes, José Lopes, José Matos
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

. "$(dirname "$0")/utils.sh"

assert_installed_command dot
assert_installed_command tred

{
echo "digraph {"
echo "node[fontname = \"monospace\" shape=rect];"

# Exclude main and test. Those are not modules.
sources="$(find src -type f -name '*.c' | sed '/main.c$/d ; /test.c$/d ;' )"

for source in $sources; do
	source_name="$(basename "$source" | sed 's/.c$//')"
	header_path="$(echo "$source" | sed 's/^src\//include\// ; s/.c$/.h/')"

	# List include dependencies. This method is not as accurate as when this is
	# done by compilers, but this doesn't make a difference in our project.
	source_dependencies="$(grep '^#include "' "$source" | \
		sed 's/^#include "// ; s/.h"$//')"
	header_dependencies="$(grep '^#include "' "$header_path" | \
		sed 's/^#include "// ; s/.h"$//')"

	for dependency in $source_dependencies $header_dependencies; do
		dependency_name="$(basename "$dependency" | sed 's/.h$//')"
		if [ "$source_name" != "$dependency_name" ]; then
			echo "$source_name -> $dependency_name"
		fi
	done
done
echo "}"
} | tred | dot -Txlib
