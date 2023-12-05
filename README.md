# Lama Bytecode Interpreter

Lama Bytecode Interpreter is a project that provides an interpreter for the bytecode generated by the Lama programming language compiler. It allows you to execute programs compiled from the Lama language.

## Building the Interpreter

To build the interpreter, follow these steps:

1. Place the project inside the "Lama" directory.
2. Navigate to the project directory and run `make all` to build the interpreter.
3. If you need to clean the project, run `make clean`.

## Running Tests

To run regression tests, execute the bash script `run_tests.sh`. This script runs regression tests to ensure the correct functioning of the interpreter.

To run performance tests, execute the bash script `performance_test.sh`. This script runs performance test to compare implemented interpreter performance with the existing one.


## Usage

After successful compilation, the bytecode interpreter can be used to execute bytecode (.bc) files generated by the Lama compiler.

### Command for Execution

```bash
./build/lama-interpreter <bytecode_file>
```
Replace <bytecode_file> with the name of the bytecode file you want to interpret.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
