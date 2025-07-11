#pragma once

#include <libstuff/libstuff.h>
#include <libstuff/SQResult.h>
#include "TodoItem.h"
#include <vector>

using namespace std;

// Database class for TODO operations using SQLite
class TodoDatabase {
public:
    // Constructor
    TodoDatabase(const string& dbPath);
    
    // Destructor
    ~TodoDatabase();

    // Database operations
    bool initialize();
    bool isOpen() const;
    
    // CRUD operations
    bool createTodo(TodoItem& item);  // Sets the ID on the item
    TodoItem getTodo(int64_t id);
    vector<TodoItem> getAllTodos();
    vector<TodoItem> getTodosByStatus(bool completed);
    vector<TodoItem> getTodosByPriority(const string& priority);
    bool updateTodo(const TodoItem& item);
    bool deleteTodo(int64_t id);
    
    // Statistics
    int getTotalCount();
    int getCompletedCount();
    int getIncompleteCount();

private:
    string _dbPath;
    sqlite3* _db;
    bool _isOpen;

    // Helper methods
    bool createTables();
    TodoItem resultToTodoItem(const SQResult& result, int row);
    bool executeQuery(const string& sql);
};