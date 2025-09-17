# olshell

The world's worst shell.

I'm building olshell to be my daily driven shell, since I haven't yet found a decent shell for windows and just decided to make my own.

## Features
- [Builtins](docs/Builtins.MD)
- [Scripting](docs/Scripting.MD)
- [Pipes and redirection](docs/Pipeline.MD)
- History
- Tab Completion (kinda wokrs)
- Aliases

## Build & Setup

1. **Clone the repo:**
   ```bash
   git clone https://github.com/Oltsu-code/olsh.git
   cd olsh
   ```

2. **Build:**
   ```bash
    mkdir build && cd build
    cmake ..
    cmake --build .
   ```
   
3. **Run:**
   ```bash
   ./olshell
   ```

## Contributing

Please follow [these instructions](CONTRIBUTING.md) for contributing.

## License
This project is licensed under the
PolyForm Noncommercial License 1.0.0 — free for noncommercial use only. Attribution required: preserve LICENSE and NOTICE, and credit the original “olshell” project in forks/derivatives (see NOTICE for wording). For commercial licensing, contact the authors.
