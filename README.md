# About

XShell is a simple custom terminal shell built with functionality for managing a to-do list directly from the shell prompt. It allows users to add and remove tasks, customize the terminal prompt, and change colors easily through a configuration file.

## Features

- **To-Do List Management**:
    - View your tasks by typing `todo`.
    - Add tasks with the `-a` flag: `todo -a some_task`.
    - Remove tasks by name or ID using the `-r` flag: `todo -r some_task` or `todo -r ID`.

- **Customizable Prompt**:
    - Customize the prompt format and colors using the `~/.xshellrc` configuration file.
    - Change colors and prompt text.

## Installation

   ```bash
   git clone https://github.com/malyok99/XShell.git
   cd XShell
   gcc -o xshell *.c #if you added some code
   ./xshell
