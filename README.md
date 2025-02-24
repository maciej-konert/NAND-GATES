# NAND Gates Library

This project implements a dynamically loaded C library for creating and simulating Boolean systems using NAND gates. The library provides a comprehensive API for constructing complex digital logic circuits based on the fundamental NAND operation.

## Project Overview

The NAND Gates library allows users to build arbitrarily complex Boolean circuits using only NAND gates as the fundamental building block. This approach aligns with the theoretical understanding that NAND gates are functionally complete, meaning any Boolean function can be implemented using only NAND operations.

## Task Description

Complete specifications for the project can be found in:
- `NANDGATES/task_en.pdf` (English version)
- `NANDGATES/task_pl.pdf` (Polish version)

## Project Structure

### Testing Framework
The `NANDGATES/tests` directory contains validation utilities:
- `memory_tests.c/.h`: Validate memory management and detect potential leaks
- `nand.h`: Header file defining the library's public interface
- `nand_example.c`: Example usage demonstrating the library's capabilities

### Implementation
The `NANDGATES/solution` directory contains the actual implementation:
- `nand.c`: Core implementation of all library functions
- `makefile`: Build configuration for compiling the shared library

## Building the Library

To compile the library, navigate to the solution directory and execute:

```bash
make libnand.so
```

This command will compile the source code and link it with the memory tests, producing a dynamically loadable shared library.

## Library Features

The library supports:
- Creation and management of NAND gate networks
- Input manipulation and output calculation
- Circuit evaluation based on provided input states
- Memory-efficient representation of complex Boolean systems
- Dynamic allocation and deallocation of resources

## Design Philosophy

The implementation prioritizes:
- Memory efficiency
- Performance optimization
- Clear API boundaries
- Flexibility in circuit construction

## Project Evaluation

The implementation received a strong evaluation:
- 13/14 points for passing automatic functional tests
- 5/6 points for code quality assessment
- Total score: 18/20

This score reflects the library's robust implementation, efficient memory management, and adherence to specified requirements while maintaining high code quality standards.
