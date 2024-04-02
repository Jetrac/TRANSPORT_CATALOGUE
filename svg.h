#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <sstream>

using namespace std::literals;

namespace svg {
    class Rgb {
    public:
        Rgb() = default;
        Rgb(int red, int green, int blue) : red(red), green(green), blue(blue) {}

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    class Rgba {
    public:
        Rgba() = default;
        Rgba(int red, int green, int blue, double opacity) : red(red), green(green), blue(blue), opacity(opacity) {}

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant < std::monostate, std::string, svg::Rgb, svg::Rgba >;

    inline void PrintRoots(std::ostream& out, std::monostate) {
        out << "none"sv;
    }
    inline void PrintRoots(std::ostream& out, const std::string& color) {
        out << color;
    }
    inline void PrintRoots(std::ostream& out, svg::Rgb color) {
        out << "rgb("sv << static_cast<int>(color.red) << ","sv << static_cast<int>(color.green) << ","sv << static_cast<int>(color.blue) << ")"sv;
    }
    inline void PrintRoots(std::ostream& out, svg::Rgba color) {
        out << "rgba("sv << static_cast<int>(color.red) << ","sv << static_cast<int>(color.green) << ","sv << static_cast<int>(color.blue) << ","sv << color.opacity << ")"sv;
    }

    // Объявив в заголовочном файле константу со спецификатором inline,
    // мы сделаем так, что она будет одной на все единицы трансляции,
    // которые подключают этот заголовок.
    // В противном случае каждая единица трансляции будет использовать свою копию этой константы
    inline const Color NoneColor{"none"};

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    inline std::ostream& operator<< (std::ostream& os, StrokeLineCap line_cap) {
        switch (line_cap) {
            case StrokeLineCap::BUTT:
                return os << "butt"sv;
            case StrokeLineCap::ROUND:
                return os << "round"sv;
            case StrokeLineCap::SQUARE:
                return os << "square"sv;
        }
        return os;
    }

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    inline std::ostream& operator<< (std::ostream& os, StrokeLineJoin line_join) {
        switch (line_join) {
            case StrokeLineJoin::ARCS:
                return os << "arcs"sv;
            case StrokeLineJoin::BEVEL:
                return os << "bevel"sv;
            case StrokeLineJoin::MITER:
                return os << "miter"sv;
            case StrokeLineJoin::MITER_CLIP:
                return os << "miter-clip"sv;
            case StrokeLineJoin::ROUND:
                return os << "round"sv;
        }
        return os;
    }
//----------------------------------------------------------------------------------------------------------------------
    struct Point {
        Point() = default;

        Point(double x, double y) : x(x), y(y) {}

        double x = 0;
        double y = 0;
    };
//----------------------------------------------------------------------------------------------------------------------
/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
    struct RenderContext {
        RenderContext(std::ostream &out) : out(out) {}

        RenderContext(std::ostream &out, int indent_step, int indent = 0)
                : out(out), indent_step(indent_step), indent(indent) {}

        RenderContext Indented() const {
            return {out, indent_step, indent + indent_step};
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream &out;
        int indent_step = 0;
        int indent = 0;
    };
//----------------------------------------------------------------------------------------------------------------------
/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
    class Object {
    public:
        void Render(const RenderContext &context) const;

        virtual ~Object() = default;
    private:
        virtual void RenderObject(const RenderContext &context) const = 0;
    };
    // ---------- PathProps ---------------
    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ =line_join;
            return AsOwner();
        }
    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_ != std::nullopt) {
                std::ostringstream strm1;
                // Это универсальная лямбда-функция (generic lambda).
                // Внутри неё нужная функция PrintRoots будет выбрана за счёт перегрузки функций.
                visit([&strm1](auto value) {
                    PrintRoots(strm1, value);
                }, *fill_color_);
                out << " "sv << "fill"sv << "="sv << "\""sv << strm1.str() << "\""sv;
            }
            if (stroke_color_ != std::nullopt) {
                std::ostringstream strm2;
                // Это универсальная лямбда-функция (generic lambda).
                // Внутри неё нужная функция PrintRoots будет выбрана за счёт перегрузки функций.
                visit([&strm2](auto value) {
                    PrintRoots(strm2, value);
                }, *stroke_color_);
                out << " "sv << "stroke"sv << "="sv << "\""sv << strm2.str() << "\""sv;
            }
            if (stroke_width_ != std::nullopt) {
                out << " "sv << "stroke-width"sv << "="sv << "\""sv << *stroke_width_ << "\""sv;
            }
            if (line_cap_ != std::nullopt) {
                out << " "sv << "stroke-linecap"sv << "="sv << "\""sv << *line_cap_ << "\""sv;
            }
            if (line_join_ != std::nullopt) {
                out << " "sv << "stroke-linejoin"sv << "="sv << "\""sv << *line_join_ << "\""sv;
            }
        }
    private:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };


//----------------------------------------------------------------------------------------------------------------------
/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
    class Circle final : public Object, public PathProps<Circle>  {
    public:
        Circle &SetCenter(Point center);

        Circle &SetRadius(double radius);
    private:
        void RenderObject(const RenderContext &context) const override;

        Point center_;
        double radius_ = 1.0;
    };
//----------------------------------------------------------------------------------------------------------------------
/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline &AddPoint(Point point);
    private:
        void RenderObject(const RenderContext &context) const override;

        std::vector<Point> points_;
        /*
         * Прочие методы и данные, необходимые для реализации элемента <polyline>
         */
    };
//----------------------------------------------------------------------------------------------------------------------
/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
    class Text final : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text &SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text &SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text &SetFontSize(uint32_t font_size);

        // Задаёт название шрифта (атрибут font-family)
        Text &SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text &SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text &SetData(std::string data);

    private:
        void RenderObject(const RenderContext &context) const override;

        Point pos_ = {0,0};
        Point offset_ = {0,0};
        uint32_t font_size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
        // Прочие данные и методы, необходимые для реализации элемента <text>
    };
//----------------------------------------------------------------------------------------------------------------------
    class ObjectContainer {
    public:
        template <typename T>
        void Add(T obj);

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    protected:
        ~ObjectContainer() = default;
    };

    class Document : public ObjectContainer{
    public:
        /*
         Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
         Пример использования:
         Document doc;
         doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
        */
        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

        // Прочие методы и данные, необходимые для реализации класса Document
    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

    template <typename T>
    void ObjectContainer::Add(T obj) {
        AddPtr(std::make_unique<T>(std::move(obj)));
    }

//----------------------------------------------------------------------------------------------------------------------
    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;

        virtual ~Drawable() = default;
    };

//----------------------------------------------------------------------------------------------------------------------

} // namespace svg
