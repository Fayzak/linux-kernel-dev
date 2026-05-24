#!/bin/bash

MODULE="hello_world_module"
PARAM_PATH="/sys/module/${MODULE}/parameters"

# "Hello, World!"
#  H  e  l  l  o ,     W  o  r  l  d !
CHARS=(72 101 108 108 111 44 32 87 111 114 108 100 33)

for i in "${!CHARS[@]}"; do
    echo "$i" > "${PARAM_PATH}/idx"
    echo "${CHARS[$i]}" > "${PARAM_PATH}/ch_val"
done

echo "Result:"
cat "${PARAM_PATH}/my_str"