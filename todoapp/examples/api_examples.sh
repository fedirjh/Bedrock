#!/bin/bash

# TODO API Examples
# Demonstrates all the available API endpoints

set -e  # Exit on any error

BASE_URL="http://localhost:8080"
CONTENT_TYPE="Content-Type: application/json"

echo "=== TODO Application API Examples ==="
echo ""

# Function to make HTTP requests with error handling
make_request() {
    local method=$1
    local endpoint=$2
    local data=$3
    local description=$4
    
    echo ">>> $description"
    echo "    $method $endpoint"
    if [ -n "$data" ]; then
        echo "    Data: $data"
    fi
    echo ""
    
    if [ -n "$data" ]; then
        curl -s -X "$method" "$BASE_URL$endpoint" \
             -H "$CONTENT_TYPE" \
             -d "$data" | jq . 2>/dev/null || curl -s -X "$method" "$BASE_URL$endpoint" -H "$CONTENT_TYPE" -d "$data"
    else
        curl -s -X "$method" "$BASE_URL$endpoint" | jq . 2>/dev/null || curl -s -X "$method" "$BASE_URL$endpoint"
    fi
    
    echo ""
    echo "----------------------------------------"
    echo ""
}

# Check if server is running
echo "Checking if TODO server is running..."
if ! curl -s "$BASE_URL/health" > /dev/null; then
    echo "ERROR: TODO server is not running on $BASE_URL"
    echo "Please start the server first:"
    echo "  ./todoapp --port=8080 --db=todos.db"
    echo ""
    exit 1
fi

echo "✓ Server is running"
echo ""

# 1. Health Check
make_request "GET" "/health" "" "Health Check - Get server status and statistics"

# 2. Create some TODO items
make_request "POST" "/todos" '{
    "title": "Learn libstuff framework",
    "description": "Study the libstuff documentation and build example applications",
    "priority": "high"
}' "Create TODO #1 - High priority learning task"

make_request "POST" "/todos" '{
    "title": "Write unit tests",
    "description": "Add comprehensive unit tests for the TODO application",
    "priority": "medium"
}' "Create TODO #2 - Medium priority development task"

make_request "POST" "/todos" '{
    "title": "Deploy to production",
    "description": "Set up production environment and deploy the application",
    "priority": "low",
    "completed": false
}' "Create TODO #3 - Low priority deployment task"

make_request "POST" "/todos" '{
    "title": "Code review",
    "description": "Review the TODO application code with team",
    "priority": "high"
}' "Create TODO #4 - High priority review task"

# 3. List all TODOs
make_request "GET" "/todos" "" "List All TODOs - Get complete list"

# 4. Get specific TODO
make_request "GET" "/todos/1" "" "Get Specific TODO - Retrieve TODO with ID 1"

# 5. Update a TODO (mark as completed)
make_request "PUT" "/todos/1" '{
    "title": "Learn libstuff framework",
    "description": "Study the libstuff documentation and build example applications - COMPLETED!",
    "completed": true,
    "priority": "high"
}' "Update TODO #1 - Mark learning task as completed"

# 6. List completed TODOs
make_request "GET" "/todos?completed=true" "" "List Completed TODOs - Filter by completion status"

# 7. List incomplete TODOs
make_request "GET" "/todos?completed=false" "" "List Incomplete TODOs - Filter by incomplete status"

# 8. List high priority TODOs
make_request "GET" "/todos?priority=high" "" "List High Priority TODOs - Filter by priority"

# 9. List medium priority TODOs
make_request "GET" "/todos?priority=medium" "" "List Medium Priority TODOs - Filter by priority"

# 10. Update another TODO
make_request "PUT" "/todos/2" '{
    "title": "Write comprehensive unit tests",
    "description": "Add unit tests covering all API endpoints and database operations",
    "completed": true,
    "priority": "medium"
}' "Update TODO #2 - Mark testing task as completed"

# 11. Delete a TODO
make_request "DELETE" "/todos/3" "" "Delete TODO #3 - Remove deployment task"

# 12. Verify deletion by listing all TODOs
make_request "GET" "/todos" "" "List All TODOs - Verify deletion"

# 13. Try to get deleted TODO (should return 404)
echo ">>> Get Deleted TODO - Should return 404 Not Found"
echo "    GET /todos/3"
echo ""
curl -s -X "GET" "$BASE_URL/todos/3" | jq . 2>/dev/null || curl -s -X "GET" "$BASE_URL/todos/3"
echo ""
echo "----------------------------------------"
echo ""

# 14. Create TODO with invalid priority (should return 400)
echo ">>> Create TODO with Invalid Priority - Should return 400 Bad Request"
echo "    POST /todos"
echo "    Data: {\"title\":\"Invalid\",\"priority\":\"invalid\"}"
echo ""
curl -s -X "POST" "$BASE_URL/todos" \
     -H "$CONTENT_TYPE" \
     -d '{"title":"Invalid TODO","priority":"invalid"}' | jq . 2>/dev/null || \
curl -s -X "POST" "$BASE_URL/todos" -H "$CONTENT_TYPE" -d '{"title":"Invalid TODO","priority":"invalid"}'
echo ""
echo "----------------------------------------"
echo ""

# 15. Final health check to see updated statistics
make_request "GET" "/health" "" "Final Health Check - View updated statistics"

echo "=== API Examples Complete ==="
echo ""
echo "Summary of demonstrated features:"
echo "✓ Health check endpoint"
echo "✓ Create TODO items with various priorities"
echo "✓ List all TODOs"
echo "✓ Get specific TODO by ID"
echo "✓ Update TODO items"
echo "✓ Mark TODOs as completed"
echo "✓ Filter TODOs by completion status"
echo "✓ Filter TODOs by priority"
echo "✓ Delete TODO items"
echo "✓ Error handling (404, 400)"
echo "✓ JSON request/response handling"
echo ""
echo "The TODO application is fully functional and demonstrates"
echo "all the capabilities of the libstuff framework!"