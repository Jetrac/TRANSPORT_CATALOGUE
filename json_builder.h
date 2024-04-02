#include "json.h"

namespace json {
    class ItemContext;

    class KeyItemContext;
    class ValueItemContext;
    class DictItemContext;
    class ArrayItemContext;

    class Builder {
    public:
        Builder();

        KeyItemContext Key(const std::string &key);

        Builder &Value(const Node &value);

        DictItemContext StartDict();

        ArrayItemContext StartArray();

        Builder &EndDict();

        Builder &EndArray();

        Node Build();

    private:
        Node root_;
        std::vector<Node *> nodes_stack_;

        template<typename Element>
        void AddElement(Element element);
    };

    template<typename Element>
    void Builder::AddElement(Element element) {
        if (nodes_stack_.back()->IsArray()) {
            const_cast<Array &>(nodes_stack_.back()->AsArray()).emplace_back(element);
            nodes_stack_.emplace_back(&const_cast<Array &>(nodes_stack_.back()->AsArray()).back());
        } else {
            *nodes_stack_.back() = element;
        }
    }

    class ItemContext {
    public:
        ItemContext(Builder &builder) : builder_(builder) {};

        KeyItemContext Key(const std::string &key);

        Builder &Value(const Node &value);

        DictItemContext StartDict();

        ArrayItemContext StartArray();

        Builder &EndDict();

        Builder &EndArray();

    private:
        Builder &builder_;
    };

    class KeyItemContext : public ItemContext {
    public:
        KeyItemContext(Builder &builder) : ItemContext(builder) {};

        KeyItemContext Key(std::string key);

        ValueItemContext Value(const Node& value);

        Builder &EndDict();

        Builder &EndArray();
    };

    class ValueItemContext : public ItemContext {
    public:
        ValueItemContext(Builder &builder) : ItemContext(builder) {};

        Builder &Value(Node value);

        DictItemContext StartDict();

        ArrayItemContext StartArray();

        Builder &EndArray();
    };

    class DictItemContext : public ItemContext {
    public:
        DictItemContext(Builder &builder) : ItemContext(builder) {};

        Builder &Value(Node value);

        DictItemContext StartDict();

        ArrayItemContext StartArray();

        Builder &EndArray();
    };

    class ArrayItemContext : public ItemContext {
    public:
        ArrayItemContext(Builder &builder) : ItemContext(builder) {};

        KeyItemContext Key(std::string key);

        ArrayItemContext Value(const Node& value);

        Builder &EndDict();
    };

} // namespace json
