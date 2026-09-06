/**
 * @file codeValidator.cpp
 * @brief Code Validator for SSCC barcodes.
 *
 * The application reads a configuration file to obtain the GS1-128 organization ID and validates example
 * codes as well as user-provided / scanned codes against the SSCC format according to the GS1-128 standard.
 */
#include <iostream>
#include <fstream>
#include <filesystem>
#include <optional>
#include <vector>
#include <string>
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
optional<string> loadOrganizationID();  // can return nullopt if the organization ID cannot be loaded
void validateAndPrintResult(const string& code, const string& organizationId);
void validateExamples(const string& organizationId);
int runUnitTests();
bool isCheckDigitValid(const string& code);
int calculateCheckDigit(const string& code);
bool containsOnlyDigits(const string& str);

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
    if (argc == 2 && string(argv[1]) == "--test") {
        return runUnitTests();
    }

    const optional<string> organizationId = loadOrganizationID();
    if (!organizationId.has_value()) {
        return 1;   // exit with error
    }
    cout << "Loaded organization ID: " << organizationId.value() << endl;

    validateExamples(organizationId.value());
    printUsage(organizationId.value());

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
 * organization ID is not found or invalid, an error message is printed and nullopt is returned.
 *
 * @return The loaded organization ID, or nullopt on failure.
 */
optional<string> loadOrganizationID() {
    if (!std::filesystem::exists(CONFIGURATION_FILE)) {
        cerr << "Error: Configuration file not found: " << CONFIGURATION_FILE << endl;
        return nullopt;
    }

    std::filesystem::path configPath{CONFIGURATION_FILE};
    ifstream configFile{configPath};
    if (!configFile.is_open()) {
        cerr << "Error: Could not open configuration file: " << CONFIGURATION_FILE << endl;
        return nullopt;
    }

    string organizationId;
    string line;
    while (getline(configFile, line)) {
        if (line.find(ORGANIZATION_KEY) == 0) {
            organizationId = line.substr(ORGANIZATION_KEY.length());
            break;
        }
    }

    if (organizationId.empty()) {
        cerr << "Error: Organization ID not found in configuration file." << endl;
        return nullopt;
    }

    // only digits are allowed
    if (!containsOnlyDigits(organizationId)) {
        cerr << "Error: Organization ID must contain digits only." << endl;
        return nullopt;
    }

    return organizationId;
}

bool containsOnlyDigits(const string& str) {
    if (str.empty()) {
        return false;
    }

    for (char c : str) {
        if (c < '0' || c > '9') {
            return false;
        }
    }

    return true;
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
        return result;  // code is empty - do not check anything else
    }

    const size_t inputLength = input.length();

    if (inputLength != CODE_LENGTH) {
        result.valid = false;
        result.errors.push_back("Invalid code length. Expected " + to_string(CODE_LENGTH) + " digits.");
    }

    // only digits are allowed
    const bool nonDigitFound = !containsOnlyDigits(input);
    if (nonDigitFound) {
        result.valid = false;
        result.errors.push_back("Code contains non-digit characters.");
    }

    // Application Identifier must be "00" for SSCC
    const string applicationIdentifier = input.substr(0, 2);
    if (applicationIdentifier != "00") {
        result.valid = false;
        result.errors.push_back("Invalid Application Identifier. Expected '00', got: " + applicationIdentifier);
    }

    // company prefix starts at index 3 (after AI and the extension digit)
    if (input.length() >= 3 + organizationId.length()) {
        const string enteredOrganizationId = input.substr(3, organizationId.length());
        if (enteredOrganizationId != organizationId) {
            result.valid = false;
            result.errors.push_back("Invalid Organization ID. Expected: " + organizationId + ", got: " + enteredOrganizationId);
        }
    }

    // only check the check digit if the length is correct and all characters are digits - otherwise the check makes no sense
    if ((inputLength == CODE_LENGTH && !nonDigitFound) &&
        (!isCheckDigitValid(input))) {
            result.valid = false;
            result.errors.push_back("Invalid check digit.");
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
    const ValidationResult result = validateSscc(code, organizationId);

    cout << "Validating code " << TEXT_BOLD << code << COLOR_RESET << " - ";
    if (result.valid) {
        cout << COLOR_GREEN << "VALID" << COLOR_RESET << endl << endl;
    } else {
        cout << COLOR_RED << "INVALID" << COLOR_RESET << endl;
        for (string error : result.errors) {
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
    string input;

    while (true) {
        cout << "Enter a code to validate or 'q' to quit: ";

        if (!(cin >> input)) { // read until EOF or error
            cout << endl << "Exiting." << endl;
            return;
        } else if (input == "q" || input == "Q") {
            cout << "Exiting." << endl;
            return;
        } else {
            validateAndPrintResult(input, organizationId);
        }
    }
}

bool isCheckDigitValid(const string& code) {
    if (code.length() != CODE_LENGTH) {
        return false;
    }

    // take the code without first two (AI) and last digit (check digit)
    const string codeWithoutCheckDigit = code.substr(2, CODE_LENGTH - 3);

    const int expectedCheckDigit = calculateCheckDigit(codeWithoutCheckDigit);
    const int actualCheckDigit = code.back() - '0'; // Convert char to int

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

int expect(bool condition, string description) {
    if (!condition) {
        cerr << "FAILED: " << description << endl;
        return 1;
    }
    return 0;
}

/**
 * Run unit tests for the SSCC validator. Using own testing logic since assert fails when first test fails, and we want to run all tests and report all failures.
 *
 * @return The process exit code: 0 if all tests passed, 1 otherwise.
 */
int runUnitTests() {
    const string organizationId = "34260311";

    int failedTestCount = 0;

    failedTestCount += expect(validateSscc(exampleCodes[0], organizationId).valid, exampleCodes[0] + " should be valid");
    failedTestCount += expect(!validateSscc(exampleCodes[1], organizationId).valid, exampleCodes[1] + " should be invalid (multiple errors)");
    failedTestCount += expect(!validateSscc(exampleCodes[2], organizationId).valid, exampleCodes[2] + " should be invalid (bad check digit)");
    failedTestCount += expect(validateSscc(exampleCodes[3], organizationId).valid, exampleCodes[3] + " should be valid");
    failedTestCount += expect(!validateSscc(exampleCodes[4], organizationId).valid, exampleCodes[4] + " should be invalid (too short)");
    failedTestCount += expect(!validateSscc(exampleCodes[5], organizationId).valid, exampleCodes[5] + " should be invalid (wrong company prefix)");

    failedTestCount += expect(!validateSscc("", organizationId).valid, "empty code should be invalid");
    string code = "0003426031113077659A";
    failedTestCount += expect(!validateSscc(code, organizationId).valid, code + " should be invalid (non-digit character)");

    code = "000342603111307765940";
    failedTestCount += expect(!validateSscc(code, organizationId).valid, code + " should be invalid (too long)");

    code = "0003426031113";
    failedTestCount += expect(!validateSscc(code, organizationId).valid, code + " should be invalid (too short)");

    // change organization ID - previously valid codes should now be invalid
    const string anotherOrganizationId = "1234567";
    failedTestCount += expect(!validateSscc(exampleCodes[0], anotherOrganizationId).valid, exampleCodes[0] + " should be invalid for a different organization ID");
    failedTestCount += expect(!validateSscc(exampleCodes[3], anotherOrganizationId).valid, exampleCodes[3] + " should be invalid for a different organization ID");

    if (failedTestCount == 0) {
        cout << "All tests passed." << endl;
        return 0;
    }
    cout << failedTestCount << " test(s) failed." << endl;
    return 1;
}
