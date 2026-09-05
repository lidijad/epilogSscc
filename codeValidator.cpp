/**
 * @file codeValidator.cpp
 * @brief Code Validator for SSCC barcodes.
 * 
 * The application reads a configuration file to obtain the GS1-128 organization ID and validates example
 * codes as well as user-provided / scanned codes against the SSCC format according to the GS1-128 standard.
 * @todo Add unit test.
 */
#include <iostream>
#include <limits>
#include <fstream>
#include <vector>
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

ValidationResult validateSscc(const string& input);
void printUsage();
void loadOrganizationID();
void validateAndPrintResult(const string& code);
void validateExamples();

/** List of example SSCC codes for validation. */
const vector<string> examples = {
    "00034260311130776594",
    "02044260311130776512",
    "00034260311130776144",
    "00034260311130776570",
    "0003426031113077646",
    "00034260321130774636"
};

const short CODE_LENGTH = 20; // Maximum length of the code
const string CONFIGURATION_FILE = "application.properties"; // Configuration file name
string organizationId; // GS1-128 organization ID

// color codes for terminal output
const string COLOR_RED = "\033[1;31m";
const string COLOR_GREEN = "\033[1;32m";
const string COLOR_ORANGE = "\033[1;33m";
const string COLOR_RESET = "\033[0m";

int main() {
    loadOrganizationID();

    validateExamples();

    printUsage();

    return 0;
}

/**
 * Validate the example codes and print the validation results.
 */
void validateExamples() {
    cout << "Validating example codes..." << endl << endl;
    for (const string& example : examples) {
        cout << "Validating code: " << COLOR_ORANGE << example << COLOR_RESET << endl;
        validateAndPrintResult(example);
    }
}

/**
 * Load the organization ID from the configuration file. If the file cannot be opened or the
 * organization ID is not found, an error message is printed and the program exits.
 */
void loadOrganizationID() {
    ifstream configFile(CONFIGURATION_FILE);
    if (!configFile.is_open()) {
        cerr << "Error: Could not open configuration file: " << CONFIGURATION_FILE << endl;
        exit(1);
    }

    string line;

    while (getline(configFile, line)) {
        size_t pos = line.find("organizationId=");
        if (pos != string::npos) {
            organizationId = line.substr(pos + 15);
            break;
        }
    }

    if (organizationId.empty()) {
        cerr << "Error: Organization ID not found in configuration file." << endl;
        exit(1);
    }
}

/**
 * Validate a specific SSCC code.
 *
 * @param input The SSCC code to validate.
 * @return The validation result: status and errors.
 */
ValidationResult validateSscc(const string& input) {
    ValidationResult result{true, {}};

    if (input.empty()) {
        result.valid = false;
        result.errors.push_back("Code is empty.");
    }

    // only digits are allowed
    bool nonDigitFound = false;
    for (char c : input) {
        if (!isdigit(c)) {
            result.valid = false;
            nonDigitFound = true;
            break;
        }
    }
    if (nonDigitFound) {
        result.errors.push_back("Code contains non-digit characters.");
    }

    // length of the code should be exactly 20 digits
    if (input.length() != CODE_LENGTH) {
        result.valid = false;
        result.errors.push_back("Invalid code length. Expected " + to_string(CODE_LENGTH) + " digits.");
    }

    // company prefix starts at index 3
    if (input.length() >= 2 && input.substr(0, 2) != "00") {
        result.valid = false;
        result.errors.push_back("Invalid Application Identifier. Expected '00', got: " + input.substr(0, 2));
    }

    // AI takes first 2 characters, company prefix starts at index 3
    if (input.length() >= 3 + organizationId.length()) {
        string enteredOrganizationId = input.substr(3, organizationId.length());
        if (enteredOrganizationId != organizationId) {
            result.valid = false;
            result.errors.push_back("Invalid Organization ID. Expected: " + organizationId + ", got: " + enteredOrganizationId);
        }
    }

    // TODO validate control digit (last digit)

    return result;
}

/**
 * Validate a specific SSCC code and print the validation result.
 * 
 * @param code The SSCC code to validate.
 */
void validateAndPrintResult(const string& code) {

    ValidationResult result = validateSscc(code);

    if (result.valid) {
        cout << "The code is " << COLOR_GREEN << "VALID" << COLOR_RESET << endl << endl;
    } else {
        cout << "The code is INVALID" << endl;
        for (const string& error : result.errors) {
            cout << COLOR_RED << error << COLOR_RESET << endl;
        }
        cout << endl;
    }
}

/**
 * Print usage instructions.
 */
void printUsage() {
    // TODO fix Ctrl+D endless loop issue
    while (true) {
        cout << "Enter a code to validate or 'q' to quit: ";

        string input;
        cin >> input;

        if (input == "q" || input == "Q") {
            cout << "Exiting." << endl;
            return;
        }

        validateAndPrintResult(input);
    }
}