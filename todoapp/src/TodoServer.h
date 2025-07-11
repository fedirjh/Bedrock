#pragma once

#include <libstuff/libstuff.h>
#include <libstuff/SData.h>
#include "TodoDatabase.h"
#include <memory>

using namespace std;

// HTTP server for TODO API
class TodoServer {
public:
    // Constructor
    TodoServer(const string& dbPath, int port = 8080);
    
    // Destructor
    ~TodoServer();

    // Server operations
    bool initialize();
    void run();
    void shutdown();

private:
    string _dbPath;
    int _port;
    unique_ptr<TodoDatabase> _database;
    bool _running;
    int _serverSocket;

    // Request handling
    void handleRequest(int clientSocket, const SData& request);
    SData processRequest(const SData& request);
    
    // API endpoints
    SData handleGetTodos(const SData& request);
    SData handleGetTodo(const SData& request, int64_t id);
    SData handleCreateTodo(const SData& request);
    SData handleUpdateTodo(const SData& request, int64_t id);
    SData handleDeleteTodo(const SData& request, int64_t id);
    SData handleHealthCheck(const SData& request);
    
    // Helper methods
    int64_t extractIdFromPath(const string& path);
    SData createResponse(int statusCode, const string& statusMessage, 
                         const string& content = "", const string& contentType = "application/json");
    SData createErrorResponse(int statusCode, const string& message);
    string createTodoListJSON(const vector<TodoItem>& todos);
    bool parseQueryParams(const string& path, STable& params);
    
    // Utility methods
    string getMethodAndPath(const string& methodLine, string& method, string& path);
    bool isValidHttpMethod(const string& method);
};