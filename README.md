# Custom C++ Unix Shell

> A robust, POSIX-compliant command-line interpreter built from scratch in C++, featuring advanced I/O handling, process management, and interactive terminal features via GNU Readline integration.

This project is a custom implementation of a Unix shell, mimicking the core functionalities of widely used shells like `bash` or `zsh`. It serves as a deep dive into low-level system programming, exploring how an operating system interacts with user commands, manages processes, and handles data streams.

---

## Features

Based on the completed project extentions, this shell supports a wide range of functionalities:

### Core Capabilities
* **Command Execution:** Finds and executes external programs located in the system's `$PATH`.
* **Built-in Commands:** Custom implementations for essential shell operations:
    * `cd`: Change directory (supports relative paths, absolute paths, and `~` for home).
    * `pwd`: Print current working directory.
    * `echo`: Display text to standard output.
    * `type`: Reveal information about command type (builtin vs. executable path).
    * `exit`: Terminate the shell session gracefully.

### Advanced I/O & Process Management
* ** Pipelines (`|`):**
    * Supports chaining multiple commands together, where the standard output of one becomes the standard input of the next (e.g., `ls -l | grep ".cpp" | wc -l`).
* **Redirection:**
    * **Output (`>` and `>>`):** Redirect standard output to a file (overwrite or append).
    * **Error (`2>` and `2>>`):** Redirect standard error to a file.
    * Handles complex combinations like redirecting stdout to a file while piping the result elsewhere.

### Interactive User Experience (GNU Readline)
* **Quoting Mechanisms:**
    * Handles single quotes (`'`) to preserve literal string values.
    * Handles double quotes (`"`) allowing for specific escape sequences.
    * Supports backslash escaping (`\`) for characters like spaces within arguments.
* **Tab Autocompletion:**
    * Intelligently suggests executables found in the `$PATH` when the Tab key is pressed.
* **History Navigation:**
    * Use Up/Down arrow keys to scroll through previously executed commands.
    * Full line editing capabilities (cursors, backspace, etc.) provided by the Readline library.

### History Persistence
* **Session History:** Keeps track of commands typed during the current session via the `history` builtin.
* **Persistent Storage:**
    * Loads history from a file specified by the `HISTFILE` environment variable on startup.
    * Saves history back to the file on exit.
    * Supports manual history management via `history -r` (read), `history -w` (write), and `history -a` (append).

---

## How It Works Internally

This project relies heavily on standard Unix system calls and C++ systems programming concepts:

1.  **The Read-Eval-Print Loop (REPL):** The main function runs an infinite loop that prompts the user for input using GNU Readline, parses that input, executes it, and waits for the next command.
2.  **Input Parsing:** A custom tokenizer (`quotes_splitter`) handles the complexity of splitting a command string into arguments while respecting various quoting rules and escape characters.
3.  **Process Creation (`fork` & `exec`):**
    * When an external command is entered, the shell calls `fork()` to create a child process (a duplicate of itself).
    * The child process then uses `execvp()` to replace its memory image with the requested executable program.
    * The parent shell process uses `waitpid()` to pause until the child process finishes execution.
4.  **Pipelines (`pipe` & `dup2`):**
    * For commands separated by `|`, the shell creates unidirectional data channels using the `pipe()` system call.
    * It forks multiple child processes.
    * Inside each child, it uses `dup2()` to manipulate file descriptors, connecting the `stdout` of the left command to the write-end of the pipe, and the `stdin` of the right command to the read-end of the pipe.
5.  **Redirection:** Before executing a command, the shell scans for redirection tokens (`>`, `2>`). It opens the specified files using `open()` and uses `dup2()` to replace the standard file descriptors (STDOUT_FILENO or STDERR_FILENO) with the file descriptors of the opened files.

---

## Getting Started

### Prerequisites

To compile and run this shell, you need a C++ compiler (like `g++`) and the **GNU Readline development headers and library** installed on your system.

* **Ubuntu/Debian based:**
    ```bash
    sudo apt update
    sudo apt install build-essential libreadline-dev
    ```
* **Fedora/RHEL based:**
    ```bash
    sudo dnf install readline-devel gcc-c++
    ```
* **macOS (via Homebrew):**
    ```bash
    brew install readline
    # Note: You might need to link readline during compilation explicitly on macOS if brew doesn't link it automatically.
    ```

### Compilation

Clone the repository and use the following command to compile the source code. You **must** link the readline library using the `-lreadline` flag.

```bash
g++ main.cpp -o myshell -lreadline
