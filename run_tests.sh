#!/bin/bash

base_test_dir="../regression/"
test_dirs=("." "expressions" "deep-expressions")
lama_compiler="lamac"
logs_dir="./logs"
tests_total=0
tests_success=0

if [ ! -d "$logs_dir" ]; then
    mkdir "$logs_dir"
fi

for test_dir in "${test_dirs[@]}"; do
    cur_test_dir="$base_test_dir$test_dir"
    basic_tests=$(ls "$cur_test_dir" | grep '.lama' | sed 's/.lama//g' | sort)

    for test in $basic_tests; do
        echo "Interpreting ${test_dir}/${test}.lama:"
        ((tests_total++))

        src_file="$cur_test_dir/${test}.lama"
        binary_file="${test}.bc"
        input_file="$cur_test_dir/${test}.input"
        expected_file="$cur_test_dir/orig/${test}.log"
        actual_file="$logs_dir/${test}.log"

        $lama_compiler -b "$src_file"

        ./build/lama-interpreter "$binary_file" < "$input_file" > "$actual_file"
        return_code=$?
        if [ $return_code -ne 0 ]; then
            echo "Error: return code is $return_code"
        else
            diff "$expected_file" "$actual_file" > /dev/null
            if [ $? -ne 0 ]; then
                echo "Error! Output is different from expected"
            else
                ((tests_success++))
                echo "Ok"
            fi
        fi
    done
done

echo "Total tests: $tests_total, successful: $tests_success"