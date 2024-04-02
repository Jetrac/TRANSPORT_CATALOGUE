#include "json_builder.h"

#include <utility>

namespace json {

    Builder::Builder() {
        nodes_stack_.emplace_back(&root_);
    }

    Node Builder::Build() {
        if (!nodes_stack_.empty()) {
            throw std::logic_error("Build error");
        }

        return root_;
    }

//---
    KeyItemContext Builder::Key(const std::string &key) {
        if (!nodes_stack_.back()->IsMap()) {
            throw std::logic_error("Key error");
        }

        nodes_stack_.emplace_back(&const_cast<Dict &>(nodes_stack_.back()->AsMap())[key]);

        return *this;
    }

    KeyItemContext ItemContext::Key(const std::string &key) {
        return builder_.Key(key);
    }

//---

    Builder &Builder::Value(const Node &value) {
        if (!nodes_stack_.back()->IsNull() && !nodes_stack_.back()->IsArray()) {
            throw std::logic_error("Value error");
        }

        if (nodes_stack_.back()->IsArray()) {
            const_cast<Array &>(nodes_stack_.back()->AsArray()).emplace_back(value);
        } else {
            *nodes_stack_.back() = value;
            nodes_stack_.pop_back();
        }

        return *this;
    }

    Builder &ItemContext::Value(const Node &value) {
        return builder_.Value(value);
    }

    ValueItemContext KeyItemContext::Value(const Node &value) {
        return ItemContext::Value(value);
    }

    ArrayItemContext ArrayItemContext::Value(const Node &value) {
        return ItemContext::Value(value);
    }

//---

    DictItemContext Builder::StartDict() {
        if (!nodes_stack_.back()->IsNull() && !nodes_stack_.back()->IsArray()) {
            throw std::logic_error("StartDict error");
        }

        AddElement(Dict());

        return *this;
    }

    DictItemContext ItemContext::StartDict() {
        return builder_.StartDict();
    }

//---

    ArrayItemContext Builder::StartArray() {
        if (!nodes_stack_.back()->IsNull() && !nodes_stack_.back()->IsArray()) {
            throw std::logic_error("StartArray error");
        }

        AddElement(Array());

        return *this;
    }

    ArrayItemContext ItemContext::StartArray() {
        return builder_.StartArray();
    }

//---
    Builder &Builder::EndDict() {
        if (!nodes_stack_.back()->IsMap()) {
            throw std::logic_error("EndDict error");
        }
        nodes_stack_.pop_back();

        return *this;
    }

    Builder &ItemContext::EndDict() {
        return builder_.EndDict();
    }

//---

    Builder &Builder::EndArray() {
        if (!nodes_stack_.back()->IsArray()) {
            throw std::logic_error("EndArray error");
        }

        nodes_stack_.pop_back();
        return *this;
    }

    Builder &ItemContext::EndArray() {
        return builder_.EndArray();
    }

//---
}
