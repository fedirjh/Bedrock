#include "TodoItem.h"
#include <set>

// Default constructor
TodoItem::TodoItem() : _id(0), _completed(false), _priority("medium") {
    setCreatedAt();
    _updatedAt = _createdAt;
}

// Parameterized constructor
TodoItem::TodoItem(int64_t id, const string& title, const string& description, 
                   bool completed, const string& priority)
    : _id(id), _title(title), _description(description), _completed(completed), _priority(priority) {
    if (!isValidPriority(_priority)) {
        _priority = "medium";
    }
    setCreatedAt();
    _updatedAt = _createdAt;
}

// Create TodoItem from STable
TodoItem TodoItem::fromSTable(const STable& data) {
    TodoItem item;
    
    if (SContains(data, "id")) {
        item._id = SToInt64(data.at("id"));
    }
    if (SContains(data, "title")) {
        item._title = data.at("title");
    }
    if (SContains(data, "description")) {
        item._description = data.at("description");
    }
    if (SContains(data, "completed")) {
        item._completed = (data.at("completed") == "true" || data.at("completed") == "1");
    }
    if (SContains(data, "priority")) {
        item._priority = data.at("priority");
        if (!isValidPriority(item._priority)) {
            item._priority = "medium";
        }
    }
    if (SContains(data, "created_at")) {
        item._createdAt = data.at("created_at");
    } else {
        item.setCreatedAt();
    }
    if (SContains(data, "updated_at")) {
        item._updatedAt = data.at("updated_at");
    } else {
        item._updatedAt = item._createdAt;
    }
    
    return item;
}

// Create TodoItem from JSON
TodoItem TodoItem::fromJSON(const string& json) {
    STable data = SParseJSONObject(json);
    return fromSTable(data);
}

// Convert to STable
STable TodoItem::toSTable() const {
    STable data;
    data["id"] = SToStr(_id);
    data["title"] = _title;
    data["description"] = _description;
    data["completed"] = _completed ? "true" : "false";
    data["priority"] = _priority;
    data["created_at"] = _createdAt;
    data["updated_at"] = _updatedAt;
    return data;
}

// Convert to JSON
string TodoItem::toJSON() const {
    STable data = toSTable();
    
    // Create a properly formatted JSON object with correct types
    string json = "{";
    json += "\"id\":" + SToStr(_id) + ",";
    json += "\"title\":" + SToJSON(_title) + ",";
    json += "\"description\":" + SToJSON(_description) + ",";
    json += "\"completed\":" + (_completed ? "true" : "false") + ",";
    json += "\"priority\":" + SToJSON(_priority) + ",";
    json += "\"created_at\":" + SToJSON(_createdAt) + ",";
    json += "\"updated_at\":" + SToJSON(_updatedAt);
    json += "}";
    
    return json;
}

// Validation
bool TodoItem::isValid() const {
    return !_title.empty() && isValidPriority(_priority);
}

bool TodoItem::isValidPriority(const string& priority) {
    static const set<string> validPriorities = {"low", "medium", "high"};
    return validPriorities.find(priority) != validPriorities.end();
}

// Update timestamp
void TodoItem::updateTimestamp() {
    _updatedAt = SComposeTime("%Y-%m-%d %H:%M:%S", STimeNow());
}

// Set creation timestamp
void TodoItem::setCreatedAt() {
    _createdAt = SComposeTime("%Y-%m-%d %H:%M:%S", STimeNow());
}