/**
 * @file codeValidator.cpp
 * @brief Code Validator for SSCC barcodes.
 * 
 * The application reads a configuration file to obtain the GS1-128 organization ID and validates example
 * codes as well as user-provided / scanned codes against the SSCC format according to the GS1-128 standard.
 */
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <string>
#include <cctype>
#include <cstdlib>
using namespace std;

/**
 * @struct ValidationResult
 * @brief The result of validating an SSCC code.
 * 
 * Contains a boolean indicating whether the code is valid and a vector of error messages if the code is invalid.
 */
struct ValidationResult {
    bool valid;
    vector<string> errors;
};

ValidationResult validateSscc(const string& input, const string& organizationId);
void printUsage(const string& organizationId);
string loadOrganizationID();
void validateAndPrintResult(const string& code, const string& organizationId);
void validateExamples(const string& organizationId);
void runUnitTests(const string& organizationId);
bool isCheckDigitValid(const string& code);
int calculateCheckDigit(const string& code);

/** List of example SSCC codes for validation. */
const vector<string> exampleCodes = {
    "00034260311130776594",
    "02044260311130776512",
    "00034260311130776144",
    "00034260311130776570",
    "0003426031113077646",
    "00034260321130774636"
};

const size_t CODE_LENGTH = 20; // expected length of the code
const string CONFIGURATION_FILE = "application.properties";
const string ORGANIZATION_KEY = "organizationId=";

// ANSI escape codes for terminal output
const string COLOR_RED = "\033[1;31m";
const string COLOR_GREEN = "\033[1;32m";
const string COLOR_RESET = "\033[0m";
const string TEXT_BOLD = "\033[1m";

int main(int argc, char* argv[]) {
    const string organizationId = loadOrganizationID();

    if (argc == 2 && string(argv[1]) == "--test") {
        runUnitTests(organizationId);
        return 0;
    }

    validateExamples(organizationId);

    printUsage(organizationId);

    return 0;
}

/**
 * Validate the example codes and print the validation results.
 * 
 * @param organizationId The organization ID to use for validation.
 */
void validateExamples(const string& organizationId) {
    cout << "Validating example codes..." << endl << endl;
    for (const string& example : exampleCodes) {
        validateAndPrintResult(example, organizationId);
    }
}

/**
 * Load the organization ID from the configuration file. If the file cannot be opened or the
 * organization ID is not found, an error message is printed and the program exits.
 * 
 * @return The loaded organization ID.
 */
string loadOrganizationID() {
    ifstream configFile(CONFIGURATION_FILE);
    if (!configFile.is_open()) {
        cerr << "Error: Could not open configuration file: " << CONFIGURATION_FILE << endl;
        exit(1);
    }

    string line;
    string organizationId;

    while (getline(configFile, line)) {
        if (line.rfind(ORGANIZATION_KEY, 0) == 0) {
            organizationId = line.substr(ORGANIZATION_KEY.length());
            break;
        }
    }

    if (organizationId.empty()) {
        cerr << "Error: Organization ID not found in configuration file." << endl;
        exit(1);
    }
    
    // only digits are allowed
    for (char c : organizationId) {
        if (!isdigit(static_cast<unsigned char>(c))) {
            cerr << "Error: Organization ID must contain digits only." << endl;
            exit(1);
        }
    }

    cout << "Loaded organization ID: " << organizationId << endl;
    return organizationId;
}

/**
 * Validate a specific SSCC code.
 *
 * @param input The SSCC code to validate.
 * @param organizationId The organization ID to use for validation.
 * @return The validation result: status and errors.
 */
ValidationResult validateSscc(const string& input, const string& organizationId) {
    ValidationResult result{true, {}};

    if (input.empty()) {
        result.valid = false;
        result.errors.push_back("Code is empty.");
    }

    const size_t inputLength = input.length();

    if (inputLength != CODE_LENGTH) {
        result.valid = false;
        result.errors.push_back("Invalid code length. Expected " + to_string(CODE_LENGTH) + " digits.");
    }

    // only digits are allowed
    bool nonDigitFound = false;
    for (char c : input) {
        if (!isdigit(static_cast<unsigned char>(c))) {
            result.valid = false;
            nonDigitFound = true;
            break;
        }
    }
    if (nonDigitFound) {
        result.errors.push_back("Code contains non-digit characters.");
    }

    // Application Identifier must be "00" for SSCC
    if (input.length() >= 2 && input.substr(0, 2) != "00") {
        result.valid = false;
        result.errors.push_back("Invalid Application Identifier. Expected '00', got: " + input.substr(0, 2));
    }

    // company prefix starts at index 3
    if (input.length() >= 3 + organizationId.length()) {
        string enteredOrganizationId = input.substr(3, organizationId.length());
        if (enteredOrganizationId != organizationId) {
            result.valid = false;
            result.errors.push_back("Invalid Organization ID. Expected: " + organizationId + ", got: " + enteredOrganizationId);
        }
    }

    if (inputLength == CODE_LENGTH && !nonDigitFound) { // only check the check digit if the length is correct and all characters are digits
        if (!isCheckDigitValid(input)) {
            result.valid = false;
            result.errors.push_back("Invalid check digit.");
        }
    }

    return result;
}

/**
 * Validate a specific SSCC code and print the validation result.
 * 
 * @param code The SSCC code to validate.
 * @param organizationId The organization ID to use for validation.
 */
void validateAndPrintResult(const string& code, const string& organizationId) {
    ValidationResult result = validateSscc(code, organizationId);

    string output = "Validating code: " + TEXT_BOLD + code + COLOR_RESET + " - ";
    if (result.valid) {
        cout << output << COLOR_GREEN << "VALID" << COLOR_RESET << endl << endl;
    } else {
        cout << output << COLOR_RED << "INVALID" << COLOR_RESET << endl;
        for (const string& error : result.errors) {
            cout << error << endl;
        }
        cout << endl;
    }
}

/**
 * Print usage instructions.
 * 
 * @param organizationId The organization ID to use for validation.
 */
void printUsage(const string& organizationId) {
    while (true) {
        cout << "Enter a code to validate or 'q' to quit: ";

        string input;
        if (!(cin >> input)) {
            cout << endl << "Exiting." << endl;
            return;
        }

        if (input == "q" || input == "Q") {
            cout << "Exiting." << endl;
            return;
        }

        validateAndPrintResult(input, organizationId);
    }
}

bool isCheckDigitValid(const string& code) {
    if (code.length() != CODE_LENGTH) {
        return false;
    }
    const string codeWithoutCheckDigit = code.substr(2, CODE_LENGTH - 3);

    int expectedCheckDigit = calculateCheckDigit(codeWithoutCheckDigit);
    int actualCheckDigit = code.back() - '0'; // Convert char to int

    return expectedCheckDigit == actualCheckDigit;
}

/**
 * Calculate the check digit for a given SSCC code.
 * 
 * @param code The SSCC code for which to calculate the check digit.
 * @return The calculated check digit using the Modulo 10 algorithm.
 */
int calculateCheckDigit(const string& code) {
    int sum = 0;
    for (size_t i = 0; i < code.length(); ++i) {
        const int digit = code[i] - '0'; // Convert char to int
        const int weight = (i % 2 == 0) ? 3 : 1; // Weighting factor: 3 for even positions, 1 for odd positions
        sum += digit * weight;
    }
    return (10 - (sum % 10)) % 10;
}

/**
 * Run unit tests for the SSCC validator.
 * 
 * @param organizationId The organization ID to use for validation.
 */
void runUnitTests(const string& organizationId) {
    // Examples
    assert(validateSscc(exampleCodes[0], organizationId).valid);
    assert(!validateSscc(exampleCodes[1], organizationId).valid);
    assert(!validateSscc(exampleCodes[2], organizationId).valid);
    assert(validateSscc(exampleCodes[3], organizationId).valid);
    assert(!validateSscc(exampleCodes[4], organizationId).valid);
    assert(!validateSscc(exampleCodes[5], organizationId).valid);

    // Additional invalid inputs.
    assert(!validateSscc("", organizationId).valid);
    assert(!validateSscc("0003426031113077659A", organizationId).valid);   // Contains a letter
    assert(!validateSscc("000342603111307765940", organizationId).valid);  // Too long
    assert(!validateSscc("0003426031113", organizationId).valid);  // Too short

     // change organization ID - previously valid codes should now be invalid
    const string anotherOrganizationId = "1234567";
    assert(!validateSscc(exampleCodes[0], anotherOrganizationId).valid);
    assert(!validateSscc(exampleCodes[3], anotherOrganizationId).valid);

    cout << "All tests passed." << endl;
}