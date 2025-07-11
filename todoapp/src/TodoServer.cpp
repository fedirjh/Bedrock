#include "TodoServer.h"
#include <libstuff/SData.h>
#include <libstuff/SFastBuffer.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <regex>

// Constructor
TodoServer::TodoServer(const string& dbPath, int port) 
    : _dbPath(dbPath), _port(port), _running(false), _serverSocket(-1) {
    _database = make_unique<TodoDatabase>(dbPath);
}

// Destructor
TodoServer::~TodoServer() {
    shutdown();
}

// Initialize server
bool TodoServer::initialize() {
    // Initialize database
    if (!_database->initialize()) {
        SERROR("Failed to initialize database");
        return false;
    }

    // Create server socket
    _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket < 0) {
        SERROR("Failed to create server socket");
        return false;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        SWARN("Failed to set SO_REUSEADDR");
    }

    // Bind socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(_port);

    if (bind(_serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        SERROR("Failed to bind socket to port " << _port);
        close(_serverSocket);
        return false;
    }

    // Listen for connections
    if (listen(_serverSocket, 10) < 0) {
        SERROR("Failed to listen on socket");
        close(_serverSocket);
        return false;
    }

    SINFO("TODO Server initialized on port " << _port);
    return true;
}

// Run server
void TodoServer::run() {
    if (!initialize()) {
        SERROR("Failed to initialize server");
        return;
    }

    _running = true;
    SINFO("TODO Server started on port " << _port);

    while (_running) {
        struct sockaddr_in clientAddress;
        socklen_t clientLen = sizeof(clientAddress);
        
        int clientSocket = accept(_serverSocket, (struct sockaddr*)&clientAddress, &clientLen);
        if (clientSocket < 0) {
            if (_running) {
                SERROR("Failed to accept client connection");
            }
            continue;
        }

        // Handle request in a separate thread
        thread clientThread([this, clientSocket]() {
            try {
                // Read request
                SFastBuffer buffer;
                char tempBuffer[4096];
                ssize_t bytesRead = recv(clientSocket, tempBuffer, sizeof(tempBuffer) - 1, 0);
                
                if (bytesRead > 0) {
                    tempBuffer[bytesRead] = '\0';
                    string requestStr(tempBuffer, bytesRead);
                    
                    SData request;
                    request.deserialize(requestStr);
                    
                    handleRequest(clientSocket, request);
                }
            } catch (const exception& e) {
                SERROR("Error handling client request: " << e.what());
            }
            
            close(clientSocket);
        });
        
        clientThread.detach();
    }
}

// Shutdown server
void TodoServer::shutdown() {
    _running = false;
    if (_serverSocket >= 0) {
        close(_serverSocket);
        _serverSocket = -1;
    }
    SINFO("TODO Server shut down");
}

// Handle client request
void TodoServer::handleRequest(int clientSocket, const SData& request) {
    try {
        SData response = processRequest(request);
        string responseStr = response.serialize();
        
        send(clientSocket, responseStr.c_str(), responseStr.length(), 0);
    } catch (const exception& e) {
        SERROR("Error processing request: " << e.what());
        SData errorResponse = createErrorResponse(500, "Internal Server Error");
        string responseStr = errorResponse.serialize();
        send(clientSocket, responseStr.c_str(), responseStr.length(), 0);
    }
}

// Process HTTP request
SData TodoServer::processRequest(const SData& request) {
    string method;
    string path;
    
    // Parse method line
    if (!SParseRequestMethodLine(request.methodLine, method, path)) {
        return createErrorResponse(400, "Bad Request");
    }

    SINFO("Processing " << method << " " << path);

    // Handle health check
    if (path == "/health") {
        return handleHealthCheck(request);
    }

    // Parse path and extract ID if present
    regex todoPathRegex(R"(/todos(?:/(\d+))?)");
    smatch matches;
    
    if (!regex_match(path, matches, todoPathRegex)) {
        return createErrorResponse(404, "Not Found");
    }

    // Extract ID if present
    int64_t id = 0;
    if (matches.size() > 1 && !matches[1].str().empty()) {
        id = SToInt64(matches[1].str());
    }

    // Route to appropriate handler
    if (method == "GET") {
        if (id > 0) {
            return handleGetTodo(request, id);
        } else {
            return handleGetTodos(request);
        }
    } else if (method == "POST" && id == 0) {
        return handleCreateTodo(request);
    } else if (method == "PUT" && id > 0) {
        return handleUpdateTodo(request, id);
    } else if (method == "DELETE" && id > 0) {
        return handleDeleteTodo(request, id);
    }

    return createErrorResponse(405, "Method Not Allowed");
}

// Handle GET /todos
SData TodoServer::handleGetTodos(const SData& request) {
    // Parse query parameters for filtering
    STable queryParams;
    string path;
    string method;
    SParseRequestMethodLine(request.methodLine, method, path);
    
    // Check for query parameters
    if (SContains(path, "?")) {
        string queryString = SAfter(path, "?");
        SParseURIPath("?" + queryString, path, queryParams);
    }

    vector<TodoItem> todos;
    
    // Filter by status if specified
    if (SContains(queryParams, "completed")) {
        bool completed = (queryParams["completed"] == "true");
        todos = _database->getTodosByStatus(completed);
    }
    // Filter by priority if specified
    else if (SContains(queryParams, "priority")) {
        string priority = queryParams["priority"];
        if (TodoItem::isValidPriority(priority)) {
            todos = _database->getTodosByPriority(priority);
        } else {
            return createErrorResponse(400, "Invalid priority value");
        }
    }
    // Get all todos
    else {
        todos = _database->getAllTodos();
    }

    string jsonContent = createTodoListJSON(todos);
    return createResponse(200, "OK", jsonContent);
}

// Handle GET /todos/{id}
SData TodoServer::handleGetTodo(const SData& request, int64_t id) {
    TodoItem todo = _database->getTodo(id);
    
    if (todo.getId() == 0) {
        return createErrorResponse(404, "Todo not found");
    }

    return createResponse(200, "OK", todo.toJSON());
}

// Handle POST /todos
SData TodoServer::handleCreateTodo(const SData& request) {
    try {
        TodoItem todo = TodoItem::fromJSON(request.content);
        
        if (!todo.isValid()) {
            return createErrorResponse(400, "Invalid todo data");
        }

        if (_database->createTodo(todo)) {
            return createResponse(201, "Created", todo.toJSON());
        } else {
            return createErrorResponse(500, "Failed to create todo");
        }
    } catch (const exception& e) {
        SERROR("Error creating todo: " << e.what());
        return createErrorResponse(400, "Invalid JSON");
    }
}

// Handle PUT /todos/{id}
SData TodoServer::handleUpdateTodo(const SData& request, int64_t id) {
    try {
        // Check if todo exists
        TodoItem existingTodo = _database->getTodo(id);
        if (existingTodo.getId() == 0) {
            return createErrorResponse(404, "Todo not found");
        }

        TodoItem todo = TodoItem::fromJSON(request.content);
        todo.setId(id); // Ensure ID matches URL
        
        if (!todo.isValid()) {
            return createErrorResponse(400, "Invalid todo data");
        }

        if (_database->updateTodo(todo)) {
            return createResponse(200, "OK", todo.toJSON());
        } else {
            return createErrorResponse(500, "Failed to update todo");
        }
    } catch (const exception& e) {
        SERROR("Error updating todo: " << e.what());
        return createErrorResponse(400, "Invalid JSON");
    }
}

// Handle DELETE /todos/{id}
SData TodoServer::handleDeleteTodo(const SData& request, int64_t id) {
    // Check if todo exists
    TodoItem existingTodo = _database->getTodo(id);
    if (existingTodo.getId() == 0) {
        return createErrorResponse(404, "Todo not found");
    }

    if (_database->deleteTodo(id)) {
        return createResponse(204, "No Content");
    } else {
        return createErrorResponse(500, "Failed to delete todo");
    }
}

// Handle GET /health
SData TodoServer::handleHealthCheck(const SData& request) {
    STable healthData;
    healthData["status"] = "healthy";
    healthData["service"] = "todo-api";
    healthData["database"] = _database->isOpen() ? "connected" : "disconnected";
    healthData["total_todos"] = SToStr(_database->getTotalCount());
    healthData["completed_todos"] = SToStr(_database->getCompletedCount());
    healthData["incomplete_todos"] = SToStr(_database->getIncompleteCount());
    
    string json = SComposeJSONObject(healthData);
    return createResponse(200, "OK", json);
}

// Create HTTP response
SData TodoServer::createResponse(int statusCode, const string& statusMessage, 
                                 const string& content, const string& contentType) {
    SData response;
    response.methodLine = "HTTP/1.1 " + SToStr(statusCode) + " " + statusMessage;
    response.nameValueMap["Content-Type"] = contentType;
    response.nameValueMap["Content-Length"] = SToStr(content.length());
    response.nameValueMap["Access-Control-Allow-Origin"] = "*";
    response.nameValueMap["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS";
    response.nameValueMap["Access-Control-Allow-Headers"] = "Content-Type";
    response.content = content;
    return response;
}

// Create error response
SData TodoServer::createErrorResponse(int statusCode, const string& message) {
    STable errorData;
    errorData["error"] = message;
    errorData["status"] = SToStr(statusCode);
    
    string json = SComposeJSONObject(errorData);
    return createResponse(statusCode, message, json);
}

// Create JSON array from todo list
string TodoServer::createTodoListJSON(const vector<TodoItem>& todos) {
    if (todos.empty()) {
        return "[]";
    }
    
    string json = "[";
    for (size_t i = 0; i < todos.size(); ++i) {
        json += todos[i].toJSON();
        if (i < todos.size() - 1) {
            json += ",";
        }
    }
    json += "]";
    
    return json;
}