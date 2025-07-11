#pragma once

#include <libstuff/libstuff.h>
#include <string>

using namespace std;

// TODO item data model
class TodoItem {
public:
    // Constructors
    TodoItem();
    TodoItem(int64_t id, const string& title, const string& description = "", 
             bool completed = false, const string& priority = "medium");

    // Static factory methods
    static TodoItem fromSTable(const STable& data);
    static TodoItem fromJSON(const string& json);

    // Conversion methods
    STable toSTable() const;
    string toJSON() const;

    // Accessors
    int64_t getId() const { return _id; }
    const string& getTitle() const { return _title; }
    const string& getDescription() const { return _description; }
    bool isCompleted() const { return _completed; }
    const string& getPriority() const { return _priority; }
    const string& getCreatedAt() const { return _createdAt; }
    const string& getUpdatedAt() const { return _updatedAt; }

    // Mutators
    void setId(int64_t id) { _id = id; updateTimestamp(); }
    void setTitle(const string& title) { _title = title; updateTimestamp(); }
    void setDescription(const string& description) { _description = description; updateTimestamp(); }
    void setCompleted(bool completed) { _completed = completed; updateTimestamp(); }
    void setPriority(const string& priority) { _priority = priority; updateTimestamp(); }

    // Validation
    bool isValid() const;
    static bool isValidPriority(const string& priority);

private:
    int64_t _id;
    string _title;
    string _description;
    bool _completed;
    string _priority;
    string _createdAt;
    string _updatedAt;

    void updateTimestamp();
    void setCreatedAt();
};