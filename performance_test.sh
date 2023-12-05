#!/bin/bash

lama_compiler="lamac"
performance_test_dir="../performance/"

if [ ! -d "./logs" ]; then
    mkdir "./logs"
fi

# Running the performance test
echo "Running the performance test Sort.lama in $performance_test_dir"

performance_src_file="$performance_test_dir/Sort.lama"
performance_binary_file="Sort.bc"
performance_actual_file="$performance_test_dir/Sort.log"
performance_input_file="$performance_test_dir/Sort.input"

$lama_compiler -b "$performance_src_file"

start_time_interpreter=$(date +%s.%3N)
./build/lama-interpreter "$performance_binary_file" /dev/null > "$performance_actual_file"
return_code_interpreter=$?
end_time_interpreter=$(date +%s.%3N)
execution_time_interpreter=$(echo "scale=3; ($end_time_interpreter - $start_time_interpreter) * 1000" | bc)

if [ $return_code_interpreter -ne 0 ]; then
    echo "Error: return code of interpreter is $return_code_interpreter"
else
    echo "Performance test using interpreter: OK"
    echo "Execution time using interpreter: $execution_time_interpreter milliseconds"
fi



touch "$performance_input_file"
start_time_lamac=$(date +%s.%3N)
$lama_compiler -i "$performance_src_file" < "$performance_input_file"
return_code_lamac=$?
end_time_lamac=$(date +%s.%3N)
rm "$performance_input_file"
execution_time_lamac=$(echo "scale=3; ($end_time_lamac - $start_time_lamac) * 1000" | bc)

if [ $return_code_lamac -ne 0 ]; then
    echo "Error: return code of lamac is $return_code_lamac"
else
    echo "Performance test using lamac with -i flag: OK"
    echo "Execution time using lamac with -i flag: $execution_time_lamac milliseconds"
fi

rm "$performance_binary_file"