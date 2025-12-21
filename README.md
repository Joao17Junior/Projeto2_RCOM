# RCOM Lab 2 - FTP Download Client

A command-line FTP client implementation in C that downloads files from FTP servers using the File Transfer Protocol (RFC 959).

## Features

- 🔐 Supports both anonymous and authenticated FTP connections
- 📁 Automatic file download from FTP servers
- 🌐 DNS resolution for hostnames
- 🔄 Passive mode (PASV) for data transfer
- 📝 URL parsing for `ftp://` protocol

## Requirements

- GCC compiler
- Make
- Linux/Unix environment (or WSL on Windows)
- Network connectivity

## Installation

Clone the repository and build the project:

```bash
make clean && make
```

## Usage

### Basic Usage

Download a file using an FTP URL:

```bash
./download ftp://ftp.example.com/path/to/file.txt
```

### With Authentication

```bash
./download ftp://username:password@ftp.example.com/path/to/file.txt
```

### Anonymous Login

If no credentials are provided, the client uses anonymous login:

```bash
./download ftp://ftp.netlab.fe.up.pt/pub/ubuntu-24.04.2-desktop-amd64.iso
```

### Available Make Commands

To see all available test commands:

```bash
make help
```

## URL Format

The client accepts URLs in the following formats:

- `ftp://<host>/<url-path>` - Anonymous login
- `ftp://<user>:<password>@<host>/<url-path>` - Authenticated login

## Project Structure

```
.
├── include/
│   └── download.h          # Header files and definitions
├── src/
│   ├── download.c          # Main FTP client implementation
│   ├── clientTCP.c         # TCP socket utilities
│   └── getip.c             # DNS resolution utilities
├── network/                # Network configuration experiments
├── references/             # RFC documentation
├── Makefile                # Build configuration
└── README.md               # This file
```

## How It Works

1. **URL Parsing**: Extracts host, credentials, and file path from the FTP URL
2. **DNS Resolution**: Converts hostname to IP address
3. **Control Connection**: Establishes TCP connection on port 21
4. **Authentication**: Logs in with provided or anonymous credentials
5. **Passive Mode**: Requests passive mode for data transfer
6. **Data Connection**: Opens secondary connection for file transfer
7. **Download**: Retrieves and saves the file locally

## Authors

João Júnior  - up202306719

Miguel Neri  - up202006475

## License

Educational project for RCOM course at FEUP.


