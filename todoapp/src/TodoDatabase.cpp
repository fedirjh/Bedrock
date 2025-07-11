#include "TodoDatabase.h"
#include <libstuff/SQResult.h>

// Constructor
TodoDatabase::TodoDatabase(const string& dbPath) : _dbPath(dbPath), _db(nullptr), _isOpen(false) {
}

// Destructor
TodoDatabase::~TodoDatabase() {
    if (_db) {
        sqlite3_close(_db);
        _db = nullptr;
    }
}

// Initialize database
bool TodoDatabase::initialize() {
    if (_isOpen) {
        return true;
    }

    int rc = sqlite3_open(_dbPath.c_str(), &_db);
    if (rc != SQLITE_OK) {
        SERROR("Cannot open database: " << sqlite3_errmsg(_db));
        return false;
    }

    _isOpen = true;
    SINFO("Database opened successfully: " << _dbPath);

    // Create tables if they don't exist
    return createTables();
}

// Check if database is open
bool TodoDatabase::isOpen() const {
    return _isOpen && _db != nullptr;
}

// Create database tables
bool TodoDatabase::createTables() {
    string createTableSQL = 
        "CREATE TABLE IF NOT EXISTS todos ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "title TEXT NOT NULL, "
        "description TEXT DEFAULT '', "
        "completed BOOLEAN DEFAULT 0, "
        "priority TEXT DEFAULT 'medium', "
        "created_at TEXT NOT NULL, "
        "updated_at TEXT NOT NULL"
        ");";

    if (!executeQuery(createTableSQL)) {
        SERROR("Failed to create todos table");
        return false;
    }

    // Create indexes for better performance
    string createIndexSQL = 
        "CREATE INDEX IF NOT EXISTS idx_todos_completed ON todos(completed); "
        "CREATE INDEX IF NOT EXISTS idx_todos_priority ON todos(priority); "
        "CREATE INDEX IF NOT EXISTS idx_todos_created_at ON todos(created_at);";

    if (!executeQuery(createIndexSQL)) {
        SWARN("Failed to create indexes (non-critical)");
    }

    SINFO("Database tables and indexes created successfully");
    return true;
}

// Execute a query without results
bool TodoDatabase::executeQuery(const string& sql) {
    if (!isOpen()) {
        SERROR("Database is not open");
        return false;
    }

    int rc = SQuery(_db, "TodoDatabase", sql);
    return rc == SQLITE_OK;
}

// Create a new TODO item
bool TodoDatabase::createTodo(TodoItem& item) {
    if (!isOpen()) {
        SERROR("Database is not open");
        return false;
    }

    if (!item.isValid()) {
        SERROR("Invalid TODO item");
        return false;
    }

    string sql = "INSERT INTO todos (title, description, completed, priority, created_at, updated_at) "
                 "VALUES (" + SQ(item.getTitle()) + ", " + SQ(item.getDescription()) + ", " +
                 SQ(item.isCompleted() ? 1 : 0) + ", " + SQ(item.getPriority()) + ", " +
                 SQ(item.getCreatedAt()) + ", " + SQ(item.getUpdatedAt()) + ");";

    SQResult result;
    int rc = SQuery(_db, "TodoDatabase", sql, result);
    
    if (rc == SQLITE_OK) {
        // Get the last inserted ID
        int64_t newId = sqlite3_last_insert_rowid(_db);
        item.setId(newId);
        SINFO("Created TODO item with ID: " << newId);
        return true;
    } else {
        SERROR("Failed to create TODO item");
        return false;
    }
}

// Get a TODO item by ID
TodoItem TodoDatabase::getTodo(int64_t id) {
    TodoItem item;
    
    if (!isOpen()) {
        SERROR("Database is not open");
        return item;
    }

    string sql = "SELECT id, title, description, completed, priority, created_at, updated_at "
                 "FROM todos WHERE id = " + SQ(id) + ";";

    SQResult result;
    int rc = SQuery(_db, "TodoDatabase", sql, result);
    
    if (rc == SQLITE_OK && result.size() > 0) {
        return resultToTodoItem(result, 0);
    }
    
    return item; // Return empty item if not found
}

// Get all TODO items
vector<TodoItem> TodoDatabase::getAllTodos() {
    vector<TodoItem> todos;
    
    if (!isOpen()) {
        SERROR("Database is not open");
        return todos;
    }

    string sql = "SELECT id, title, description, completed, priority, created_at, updated_at "
                 "FROM todos ORDER BY created_at DESC;";

    SQResult result;
    int rc = SQuery(_db, "TodoDatabase", sql, result);
    
    if (rc == SQLITE_OK) {
        for (size_t i = 0; i < result.size(); ++i) {
            todos.push_back(resultToTodoItem(result, i));
        }
    }
    
    return todos;
}

// Get TODO items by completion status
vector<TodoItem> TodoDatabase::getTodosByStatus(bool completed) {
    vector<TodoItem> todos;
    
    if (!isOpen()) {
        SERROR("Database is not open");
        return todos;
    }

    string sql = "SELECT id, title, description, completed, priority, created_at, updated_at "
                 "FROM todos WHERE completed = " + SQ(completed ? 1 : 0) + 
                 " ORDER BY created_at DESC;";

    SQResult result;
    int rc = SQuery(_db, "TodoDatabase", sql, result);
    
    if (rc == SQLITE_OK) {
        for (size_t i = 0; i < result.size(); ++i) {
            todos.push_back(resultToTodoItem(result, i));
        }
    }
    
    return todos;
}

// Get TODO items by priority
vector<TodoItem> TodoDatabase::getTodosByPriority(const string& priority) {
    vector<TodoItem> todos;
    
    if (!isOpen() || !TodoItem::isValidPriority(priority)) {
        return todos;
    }

    string sql = "SELECT id, title, description, completed, priority, created_at, updated_at "
                 "FROM todos WHERE priority = " + SQ(priority) + " ORDER BY created_at DESC;";

    SQResult result;
    int rc = SQuery(_db, "TodoDatabase", sql, result);
    
    if (rc == SQLITE_OK) {
        for (size_t i = 0; i < result.size(); ++i) {
            todos.push_back(resultToTodoItem(result, i));
        }
    }
    
    return todos;
}

// Update a TODO item
bool TodoDatabase::updateTodo(const TodoItem& item) {
    if (!isOpen()) {
        SERROR("Database is not open");
        return false;
    }

    if (!item.isValid() || item.getId() <= 0) {
        SERROR("Invalid TODO item for update");
        return false;
    }

    string sql = "UPDATE todos SET "
                 "title = " + SQ(item.getTitle()) + ", "
                 "description = " + SQ(item.getDescription()) + ", "
                 "completed = " + SQ(item.isCompleted() ? 1 : 0) + ", "
                 "priority = " + SQ(item.getPriority()) + ", "
                 "updated_at = " + SQ(item.getUpdatedAt()) + " "
                 "WHERE id = " + SQ(item.getId()) + ";";

    SQResult result;
    int rc = SQuery(_db, "TodoDatabase", sql, result);
    
    if (rc == SQLITE_OK) {
        SINFO("Updated TODO item with ID: " << item.getId());
        return true;
    } else {
        SERROR("Failed to update TODO item");
        return false;
    }
}

// Delete a TODO item
bool TodoDatabase::deleteTodo(int64_t id) {
    if (!isOpen()) {
        SERROR("Database is not open");
        return false;
    }

    string sql = "DELETE FROM todos WHERE id = " + SQ(id) + ";";

    SQResult result;
    int rc = SQuery(_db, "TodoDatabase", sql, result);
    
    if (rc == SQLITE_OK) {
        SINFO("Deleted TODO item with ID: " << id);
        return true;
    } else {
        SERROR("Failed to delete TODO item");
        return false;
    }
}

// Get total count of todos
int TodoDatabase::getTotalCount() {
    if (!isOpen()) {
        return 0;
    }

    string sql = "SELECT COUNT(*) as total FROM todos;";
    SQResult result;
    int rc = SQuery(_db, "TodoDatabase", sql, result);
    
    if (rc == SQLITE_OK && result.size() > 0) {
        return SToInt(result[0][0]);
    }
    
    return 0;
}

// Get count of completed todos
int TodoDatabase::getCompletedCount() {
    if (!isOpen()) {
        return 0;
    }

    string sql = "SELECT COUNT(*) as completed FROM todos WHERE completed = 1;";
    SQResult result;
    int rc = SQuery(_db, "TodoDatabase", sql, result);
    
    if (rc == SQLITE_OK && result.size() > 0) {
        return SToInt(result[0][0]);
    }
    
    return 0;
}

// Get count of incomplete todos
int TodoDatabase::getIncompleteCount() {
    if (!isOpen()) {
        return 0;
    }

    string sql = "SELECT COUNT(*) as incomplete FROM todos WHERE completed = 0;";
    SQResult result;
    int rc = SQuery(_db, "TodoDatabase", sql, result);
    
    if (rc == SQLITE_OK && result.size() > 0) {
        return SToInt(result[0][0]);
    }
    
    return 0;
}

// Convert SQResult row to TodoItem
TodoItem TodoDatabase::resultToTodoItem(const SQResult& result, int row) {
    if (row >= (int)result.size()) {
        return TodoItem(); // Return empty item
    }

    STable data;
    data["id"] = result[row][0];
    data["title"] = result[row][1];
    data["description"] = result[row][2];
    data["completed"] = result[row][3];
    data["priority"] = result[row][4];
    data["created_at"] = result[row][5];
    data["updated_at"] = result[row][6];

    return TodoItem::fromSTable(data);
}