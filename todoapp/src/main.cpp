#include <libstuff/libstuff.h>
#include "TodoServer.h"
#include <iostream>
#include <csignal>

using namespace std;

// Global server instance for signal handling
TodoServer* g_server = nullptr;

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    SINFO("Received signal " << signal << ", shutting down gracefully...");
    if (g_server) {
        g_server->shutdown();
    }
    exit(0);
}

// Print usage information
void printUsage(const char* programName) {
    cout << "Usage: " << programName << " [OPTIONS]\n"
         << "A TODO application using the libstuff framework\n\n"
         << "Options:\n"
         << "  --port=PORT         Port to listen on (default: 8080)\n"
         << "  --db=PATH          Database file path (default: todos.db)\n"
         << "  --help             Show this help message\n\n"
         << "Examples:\n"
         << "  " << programName << " --port=8080 --db=todos.db\n"
         << "  " << programName << " --port=3000 --db=/tmp/mytodos.db\n\n"
         << "API Endpoints:\n"
         << "  GET    /health              Health check\n"
         << "  GET    /todos               List all todos\n"
         << "  GET    /todos?completed=true List completed todos\n"
         << "  GET    /todos?priority=high  List high priority todos\n"
         << "  GET    /todos/{id}          Get specific todo\n"
         << "  POST   /todos               Create new todo\n"
         << "  PUT    /todos/{id}          Update existing todo\n"
         << "  DELETE /todos/{id}          Delete todo\n\n";
}

int main(int argc, char* argv[]) {
    // Initialize libstuff
    SInitialize("TodoApp");
    
    // Initialize signal handling
    SInitializeSignals();
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Parse command line arguments
    STable args = SParseCommandLine(argc, argv);
    
    // Check for help
    if (SContains(args, "help")) {
        printUsage(argv[0]);
        return 0;
    }

    // Get configuration from command line or use defaults
    int port = 8080;
    if (SContains(args, "port")) {
        port = SToInt(args["port"]);
        if (port <= 0 || port > 65535) {
            SERROR("Invalid port number: " << args["port"]);
            return 1;
        }
    }

    string dbPath = "todos.db";
    if (SContains(args, "db")) {
        dbPath = args["db"];
    }

    SINFO("Starting TODO Application");
    SINFO("Configuration:");
    SINFO("  Port: " << port);
    SINFO("  Database: " << dbPath);

    try {
        // Create and start server
        TodoServer server(dbPath, port);
        g_server = &server;

        SINFO("TODO Application started successfully");
        SINFO("Access the API at http://localhost:" << port);
        SINFO("Health check: http://localhost:" << port << "/health");
        SINFO("Press Ctrl+C to stop the server");

        // Run the server (this blocks)
        server.run();

    } catch (const exception& e) {
        SERROR("Fatal error: " << e.what());
        return 1;
    }

    SINFO("TODO Application terminated");
    return 0;
}