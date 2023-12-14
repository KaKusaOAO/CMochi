//
//  components.hpp
//  MiaCrate++
//
//  Created by 咔咔 on 2023/12/6.
//

#if defined(__cplusplus)
#ifndef __MC_COMPONENTS_HPP_HEADER_GUARD
#define __MC_COMPONENTS_HPP_HEADER_GUARD

#include <list>
#include <json/json.h>
#include <Mochi/common.hpp>
#include <Mochi/core.hpp>
#include <sstream>

namespace Mochi {

class IContent;

class IContentType {
public:
    virtual IContent& CreateContent(Json::Value payload) = 0;
    virtual void InsertPayload(Json::Value target, IContent& content) = 0;
};

class IStyle {
public:
    virtual void SerializeInto(Json::Value obj) = 0;
    virtual IStyle& ApplyTo(const IStyle& other) = 0;
    virtual IStyle& Clear() = 0;
};

class IContentVisitor;
class IContent {
public:
    virtual IContentType& GetType() = 0;
    virtual IContent& Clone() = 0;
    virtual void InsertPayload(Json::Value target) = 0;
    virtual void Visit(const IContentVisitor& visitor, const IStyle& style) = 0;
    virtual void VisitLiteral(const IContentVisitor& visitor, const IStyle& style) = 0;
};

class IContentVisitor {
public:
    using Signature = std::function<void(const IContent&, const IStyle&)>;
    
    virtual void Accept(const IContent& content, const IStyle& style) const = 0;
    static IContentVisitor& Create(Signature action);
};

class IMutableComponent;

class IComponent {
public:
    virtual IContent& GetContent() = 0;
    virtual IStyle& GetStyle() = 0;
    virtual std::list<std::reference_wrapper<IComponent>> GetSiblings() = 0;
    virtual IMutableComponent& Clone() = 0;
    virtual void Visit(const IContentVisitor& visitor, const IStyle& style) = 0;
    virtual void VisitLiteral(const IContentVisitor& visitor, const IStyle& style) = 0;
};

class IMutableComponent : public IComponent {
public:
    virtual void SetStyle(IStyle& style) = 0;
};

#define __MC_DEFINE_COLORS \
    __MC_DEFINE_COLOR(Black,      '0', black,       0x0     ) \
    __MC_DEFINE_COLOR(DarkBlue,   '1', dark_blue,   0xaa    ) \
    __MC_DEFINE_COLOR(DarkGreen,  '2', dark_green,  0xaa00  ) \
    __MC_DEFINE_COLOR(DarkAqua,   '3', dark_aqua,   0xaaaa  ) \
    __MC_DEFINE_COLOR(DarkRed,    '4', dark_red,    0xaa0000) \
    __MC_DEFINE_COLOR(DarkPurple, '5', dark_purple, 0xaa00aa) \
    __MC_DEFINE_COLOR(Gold,       '6', gold,        0xffaa00) \
    __MC_DEFINE_COLOR(Gray,       '7', gray,        0xaaaaaa) \
    __MC_DEFINE_COLOR(DarkGray,   '8', dark_gray,   0x555555) \
    __MC_DEFINE_COLOR(Blue,       '9', blue,        0x5555ff) \
    __MC_DEFINE_COLOR(Green,      'a', green,       0x55ff55) \
    __MC_DEFINE_COLOR(Aqua,       'b', aqua,        0x55ffff) \
    __MC_DEFINE_COLOR(Red,        'c', red,         0xff5555)

class TextColor  {
    using ByCharMap = std::map<char,        std::reference_wrapper<TextColor>>;
    using ByNameMap = std::map<std::string, std::reference_wrapper<TextColor>>;
    
private:
    char _code;
    std::string _name;
    std::string _toString;
    int _ordinal;
    Color _color;
    
    static ByCharMap _byChar;
    static ByNameMap _byName;
    static int _count;
    
    static TextColor& RegisterBuiltin(char code, std::string name, Color color);
public:
    const static std::string ColorChar;

    TextColor(char code, std::string name, Color color);

    bool operator==(const TextColor& other) const;
    
#define __MC_DEFINE_COLOR(id, code, name, color) \
    const static TextColor& id ;
__MC_DEFINE_COLORS
#undef __MC_DEFINE_COLOR
};

class IColoredStyle : public IStyle {
public:
    virtual std::optional<std::reference_wrapper<TextColor>> GetColor() const = 0;
    virtual IColoredStyle& WithColor(std::optional<std::reference_wrapper<TextColor>> color) const = 0;
    virtual IColoredStyle& Clear() override = 0;

    bool operator==(const IColoredStyle& other) const;
};

class BasicColoredStyle : public IColoredStyle {
private:
    static BasicColoredStyle& _empty;
    std::optional<std::reference_wrapper<TextColor>> _color;
    
public:
    static BasicColoredStyle& Empty();

    BasicColoredStyle();
    BasicColoredStyle(TextColor& color);
    BasicColoredStyle(std::optional<std::reference_wrapper<TextColor>> color);
    
    std::optional<std::reference_wrapper<TextColor>> GetColor() const override;
    BasicColoredStyle& WithColor(std::optional<std::reference_wrapper<TextColor>> color) const override;
    BasicColoredStyle& ApplyTo(const IStyle& other) override;
    void SerializeInto(Json::Value obj) override;
    BasicColoredStyle& Clear() override;
};

class LiteralContentType;

class TextContentTypes {
public:
    using Registry = std::map<std::string, std::reference_wrapper<IContentType>>;
    
    template<class T>
    static T& Register(std::string key, T& type) {
        _types[key] = type;
        return type;
    }
    
    static LiteralContentType& Literal();

private:
    static Registry _types;
    static LiteralContentType& e_Literal;
};

class LiteralContent : public IContent {
public:
    std::string text;
    
    LiteralContent(std::string text);
    IContentType& GetType() override;
    LiteralContent& Clone() override;
    void InsertPayload(Json::Value target) override;
    void Visit(const IContentVisitor& visitor, const IStyle& style) override;
    void VisitLiteral(const IContentVisitor& visitor, const IStyle& style) override;
};

class LiteralContentType : public IContentType {
public:
    LiteralContent& CreateContent(Json::Value payload) override;
    void InsertPayload(Json::Value target, IContent& content) override;
};

namespace Component {

Ref<IComponent> FromJson(Json::Value obj,
                         std::function<IStyle*(Json::Value)> parseStyle);
Ref<IComponent> FromJson(Json::Value obj);
Ref<IComponent> Literal(std::string text);

};

}

#endif /* components_hpp */
#endif
