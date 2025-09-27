# TOTP Generator

A simple, secure, and cross-platform command-line utility for generating Time-based One-Time Passwords (TOTP). This tool is designed for developers, system administrators, and anyone who prefers managing their two-factor authentication codes from the terminal.

---

### **Security Notice**

**This project is a proof of concept and is not secure for storing real-world TOTP secrets.** The secret key is stored in plain text in a configuration file on your local machine. For any production or sensitive accounts, you should use a dedicated, encrypted password manager or authenticator application. This tool is intended for educational purposes or for use in trusted, low-security development environments.

---

## Features

*   **Standard TOTP Generation:** Generates 6-digit TOTP codes compatible with Google Authenticator, Authy, and other 2FA services.
*   **Local Storage:** The account secret is stored locally in a configuration file in your user's config directory, not in the cloud.
*   **Command-Line Interface:** All operations are performed via the command line, making it fast, scriptable, and lightweight.
*   **Watch Mode:** A "watch" feature displays the current code and continuously updates it on a single line until you quit.
*   **Cross-Platform:** Built with standard C++ and CMake, it is designed to compile and run on Linux, macOS, and Windows.

## Installation

### Prerequisites

*   A C++17 compliant compiler (GCC, Clang, MSVC)
*   CMake (version 3.11 or higher)
*   Git

### Building from Source

1.  **Clone the repository:**
    ```sh
    git clone https://github.com/chooisfox/totp-generator.git
    cd totp-generator
    ```

2.  **Configure the project with CMake:**
    ```sh
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    ```

3.  **Build the executable:**
    ```sh
    cmake --build build
    ```

The final executable will be located in the `build/` directory.

## Usage

The application is controlled via command-line options.

### Command-Line Options

| Short | Long        | Description                                                          | Argument             |
| :---- | :---------- | :------------------------------------------------------------------- | :------------------- |
| `-s`  | `--secret`  | **Set** a new TOTP secret for an account.                            | `<secret_key>`       |
| `-a`  | `--account` | Specify the **account name**. Must be used with `-s`.                | `<account_name>`     |
| `-w`  | `--watch`   | **Watch** and continuously update the TOTP code. Press 'q' to quit.  | (none)               |
| `-h`  | `--help`    | Prints the help menu and all available options.                      | (none)               |
| `-d`  | `--debug`   | Prints debug information.                                            | (none)               |

### Examples

#### 1. Setting an Account Secret

To store a new account, you must provide both the secret (the Base32 string from your service provider) and an account name.

```sh
totp-generator -a "MyService" -s "JBSWY3DPEHPK3PXP"
```

This command will save the secret for "MyService" to your configuration file and print the current TOTP code.

#### 2. Generating a Single Code

If an account is already configured, simply run the application with no arguments to get the current code.

```sh
totp-generator
```

**Output:**
```
Account: MyService
TOTP Code: 123456
```

#### 3. Watching a Code

To continuously monitor a code (e.g., while waiting for a login prompt to appear), use the watch flag.

```sh
totp-generator -w
```

**Output (updates on a single line):**
```
Starting watch mode for account: MyService. Press 'q' to quit.
Code: 123456  (updates in 21s)
```

#### 4. Setting a Secret and Immediately Watching

You can combine the `-s`, `-a`, and `-w` flags. This is useful for initial setup.

```sh
totp-generator -a "AnotherService" -s "GEZDGNBVGY3TQOJQ" -w
```

This command will first save the secret for "AnotherService" and then immediately start watch mode.

## Configuration

The application stores the last-used account name and secret in a `.toml` file located in the standard user configuration directory for your operating system:

*   **Linux:** `~/.config/totp-generator/totp-generator.toml`
*   **macOS:** `~/Library/Application Support/totp-generator/totp-generator.toml`
*   **Windows:** `%APPDATA%\totp-generator\totp-generator.toml`
