# SSCC Code Validator

A C++17 console application that checks scanned barcode data for TSX organization.

The program validates 6 example codes and supports interactive input of codes. It prints validation results for each code to the console.

## Requirements

- A C++ compiler with C++17 support.
- A terminal supporting ANSI colors for colored output.

## Configuration

`application.properties` file in the program **current working directory** containing `organizationId`:

```properties
organizationId=34260311
```
If the configuration file cannot be opened or the ID is missing or empty, the program prints an error and exits.

## Build and run

Run the following commands from the directory containing `codeValidator.cpp` and `application.properties`.

Compile:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic codeValidator.cpp -o codeValidator
```

Run:

```bash
./codeValidator
```

Alternatively, run the prepared `run.sh` script that executes both of the above commands:
```bash
./run.sh
```

## Run tests

Compile the program: 
```bash
g++ -std=c++17 -Wall -Wextra -pedantic codeValidator.cpp -o codeValidator
```

Run the tests:
```bash
./codeValidator --test
```

## Usage

1. On startup, the program checks the 6 hardcoded example codes and displays the results of the validation.
2. Enter or scan an additional code when prompted and press Enter key to validate the code.

   The input numeric string must include the Application Identifier `00`, without spaces or parentheses. For example:

   ```text
   00034260311130776594
   ```

   The expected input is 20 digits: the two-digit Application Identifier followed by the 18-digit SSCC. FNC1 must not be included.
3. Repeat step 2 as needed.
4. Type `q` or `Q` and press Enter to exit.