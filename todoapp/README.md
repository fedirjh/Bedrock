# TODO Application

A complete TODO application built using the libstuff framework, demonstrating HTTP handling, SQLite database operations, JSON parsing, and cross-platform compatibility.

## Features

- **REST API** with full CRUD operations
- **SQLite Database** for persistent storage
- **JSON Support** for request/response handling
- **Query Filtering** by completion status and priority
- **Health Check** endpoint for monitoring
- **Graceful Shutdown** with signal handling
- **Cross-platform** compatible (Linux/macOS)

## API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| `GET` | `/health` | Health check and statistics |
| `GET` | `/todos` | List all TODO items |
| `GET` | `/todos?completed=true` | List completed TODOs |
| `GET` | `/todos?priority=high` | List high priority TODOs |
| `GET` | `/todos/{id}` | Get specific TODO item |
| `POST` | `/todos` | Create new TODO item |
| `PUT` | `/todos/{id}` | Update existing TODO item |
| `DELETE` | `/todos/{id}` | Delete TODO item |

## TODO Item Structure

```json
{
  "id": 1,
  "title": "Learn libstuff framework",
  "description": "Study the libstuff documentation and examples",
  "completed": false,
  "priority": "medium",
  "created_at": "2025-07-11 09:31:23",
  "updated_at": "2025-07-11 09:31:23"
}
```

### Priority Values
- `low` - Low priority tasks
- `medium` - Medium priority tasks (default)
- `high` - High priority tasks

## Building

### Prerequisites

1. **Build libstuff** first from the parent directory:
   ```bash
   cd ..
   make clean
   make libstuff.a
   ```

2. **Required libraries**:
   - libstuff (from parent directory)
   - mbedtls
   - pcre2
   - Standard C++ libraries

### Build Commands

```bash
# Build the application
make clean && make

# Build with debug symbols
make debug

# Show help
make help
```

## Usage

### Starting the Server

```bash
# Default settings (port 8080, database: todos.db)
./todoapp

# Custom port and database
./todoapp --port=3000 --db=/path/to/my_todos.db

# Show help
./todoapp --help
```

### Command Line Options

- `--port=PORT` - Port to listen on (default: 8080)
- `--db=PATH` - Database file path (default: todos.db)
- `--help` - Show help message

## API Usage Examples

### Health Check
```bash
curl http://localhost:8080/health
```

### Create a TODO
```bash
curl -X POST http://localhost:8080/todos \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Learn libstuff framework",
    "description": "Study the documentation and build example app",
    "priority": "high"
  }'
```

### List All TODOs
```bash
curl http://localhost:8080/todos
```

### List Completed TODOs
```bash
curl http://localhost:8080/todos?completed=true
```

### List High Priority TODOs
```bash
curl http://localhost:8080/todos?priority=high
```

### Get Specific TODO
```bash
curl http://localhost:8080/todos/1
```

### Update a TODO
```bash
curl -X PUT http://localhost:8080/todos/1 \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Learn libstuff framework",
    "description": "Completed the tutorial and built TODO app",
    "completed": true,
    "priority": "high"
  }'
```

### Delete a TODO
```bash
curl -X DELETE http://localhost:8080/todos/1
```

## Database Schema

The application creates a SQLite database with the following schema:

```sql
CREATE TABLE todos (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    description TEXT DEFAULT '',
    completed BOOLEAN DEFAULT 0,
    priority TEXT DEFAULT 'medium',
    created_at TEXT NOT NULL,
    updated_at TEXT NOT NULL
);

-- Indexes for performance
CREATE INDEX idx_todos_completed ON todos(completed);
CREATE INDEX idx_todos_priority ON todos(priority);
CREATE INDEX idx_todos_created_at ON todos(created_at);
```

## libstuff Components Used

This application demonstrates the following libstuff features:

- **SData** - HTTP request/response handling
- **STable** - Name/value pair management
- **STime** - Timestamp generation and formatting
- **SLog** - Logging with SINFO, SWARN, SERROR macros
- **SParseJSONObject()** / **SComposeJSONObject()** - JSON parsing
- **SQuery()** - SQLite database operations
- **SQ()** - SQL query parameter escaping
- **SInitializeSignals()** - Signal handling for graceful shutdown
- **SParseCommandLine()** - Command-line argument parsing

## Error Handling

The application includes comprehensive error handling:

- **HTTP Status Codes** - Proper REST API status codes (200, 201, 400, 404, 500, etc.)
- **JSON Error Responses** - Structured error messages
- **Input Validation** - Validates TODO data and parameters
- **Database Error Handling** - Graceful handling of SQLite errors
- **Logging** - Detailed logging of operations and errors

## Testing

### Quick Test
```bash
# Run automated test
make test
```

### Manual Testing
Use the example scripts in the `examples/` directory:

```bash
# Make scripts executable
chmod +x examples/api_examples.sh

# Run example API calls
./examples/api_examples.sh
```

## Architecture

The application consists of four main components:

1. **TodoItem** (`src/TodoItem.h/cpp`) - Data model for TODO items
2. **TodoDatabase** (`src/TodoDatabase.h/cpp`) - SQLite database operations
3. **TodoServer** (`src/TodoServer.h/cpp`) - HTTP server and API handling
4. **main** (`src/main.cpp`) - Application entry point and configuration

## Configuration

The application supports configuration through:

- **Command-line arguments** - Port and database path
- **Environment variables** - Can be easily extended
- **Default values** - Sensible defaults for quick start

## Development

### Directory Structure
```
todoapp/
├── src/
│   ├── main.cpp           # Application entry point
│   ├── TodoServer.cpp     # HTTP server implementation
│   ├── TodoServer.h       # Server class definition
│   ├── TodoDatabase.cpp   # Database operations
│   ├── TodoDatabase.h     # Database class definition
│   ├── TodoItem.cpp       # TODO item model
│   └── TodoItem.h         # TODO item class definition
├── examples/
│   └── api_examples.sh    # Sample API calls
├── Makefile               # Build configuration
└── README.md              # This documentation
```

### Extending the Application

The application is designed to be easily extensible:

- Add new fields to `TodoItem` class
- Extend database schema in `TodoDatabase::createTables()`
- Add new API endpoints in `TodoServer`
- Add query filters and sorting options

## Troubleshooting

### Build Issues

1. **libstuff.a not found**:
   ```bash
   cd .. && make libstuff.a
   ```

2. **Missing dependencies**:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libpcre2-dev

   # macOS
   brew install pcre2
   ```

### Runtime Issues

1. **Port already in use**:
   ```bash
   ./todoapp --port=8081
   ```

2. **Database permissions**:
   ```bash
   chmod 666 todos.db
   ```

3. **Check logs** - The application uses libstuff logging extensively

## License

This TODO application is provided as an example of libstuff framework usage.

---

*This application serves as a comprehensive example of building real-world applications with the libstuff framework, demonstrating best practices for HTTP handling, database operations, JSON processing, and system integration.*