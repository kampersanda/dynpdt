#!/bin/bash

echo_and_do() {
  echo "$1"
  eval "$1"
}

file_name=sample.txt
num_keys=1000000

for dic_type in {1..5}
do
  echo "=================================================="
  echo_and_do "./build/bench $dic_type $file_name = $num_keys 0.8 16 6"
done
