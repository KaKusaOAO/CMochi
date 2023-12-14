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
__MC_DEFINE_REF_TYPE(IContent)

class IContentType {
public:
    virtual IContentRef CreateContent(Json::Value payload) = 0;
    virtual void InsertPayload(Json::Value target, IContentRef content) = 0;
};
__MC_DEFINE_REF_TYPE(IContentType)

class IStyle : public std::enable_shared_from_this<IStyle> {
    using Ref = std::shared_ptr<IStyle>;
    
public:
    virtual void SerializeInto(Json::Value obj) = 0;
    virtual Ref ApplyTo(Ref other) = 0;
    virtual Ref Clear() = 0;
};
__MC_DEFINE_REF_TYPE(IStyle)

class IContentVisitor;
__MC_DEFINE_REF_TYPE(IContentVisitor)

class IContent : public std::enable_shared_from_this<IContent> {
public:
    virtual IContentTypeRef GetType() = 0;
    virtual IContentRef Clone() = 0;
    virtual void InsertPayload(Json::Value target) = 0;
    virtual void Visit(IContentVisitorRef visitor, IStyleRef style) = 0;
    virtual void VisitLiteral(IContentVisitorRef visitor, IStyleRef style) = 0;
};

class IContentVisitor {
    using Signature = std::function<void(IContentRef, IStyleRef)>;
    
public:
    virtual void Accept(IContentRef content, IStyleRef style) = 0;
    static std::shared_ptr<IContentVisitor> Create(Signature action);
};

class IMutableComponent;
__MC_DEFINE_REF_TYPE(IMutableComponent)

class IComponent {
    using Ref = __MC_REF_TYPE(IComponent);
    
public:
    virtual IContentRef GetContent() = 0;
    virtual IStyleRef GetStyle() = 0;
    virtual std::list<Ref> GetSiblings() = 0;
    virtual IMutableComponentRef Clone() = 0;
    virtual void Visit(IContentVisitorRef visitor, IStyleRef style) = 0;
    virtual void VisitLiteral(IContentVisitorRef visitor, IStyleRef style) = 0;
};

__MC_DEFINE_REF_TYPE(IComponent)

class IMutableComponent : public IComponent {
public:
    virtual void SetStyle(IStyleRef style) = 0;
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

class TextColor : public std::enable_shared_from_this<TextColor> {
    using Ref = std::shared_ptr<TextColor>;
    using ByCharMap = std::map<char, Ref>;
    using ByNameMap = std::map<std::string, Ref>;
    
private:
    char _code;
    std::string _name;
    std::string _toString;
    int _ordinal;
    Color _color;
    
    static std::map<char,        Ref> _byChar;
    static std::map<std::string, Ref> _byName;
    static int _count;
    
    static Ref RegisterBuiltin(char code, std::string name, Color color);
public:
    const static std::string ColorChar;

    TextColor(char code, std::string name, Color color);
    
#define __MC_DEFINE_COLOR(id, code, name, color) \
    const static Ref id ;
__MC_DEFINE_COLORS
#undef __MC_DEFINE_COLOR
};

__MC_DEFINE_REF_TYPE(TextColor)

class IColoredStyle : public IStyle {
    using Ref = std::shared_ptr<IColoredStyle>;
public:
    virtual TextColorRef GetColor() = 0;
    virtual Ref WithColor(TextColorRef color) = 0;
};

__MC_DEFINE_REF_TYPE(IColoredStyle)

class BasicColoredStyle : public IColoredStyle {
    using Ref = __MC_REF_TYPE(BasicColoredStyle);
    
private:
    static Ref _empty;
    TextColorRef _color;
    
public:
    static Ref Empty();
    
    TextColorRef GetColor() override;
    IColoredStyleRef WithColor(TextColorRef color) override;
    IStyleRef ApplyTo(IStyleRef other) override;
    void SerializeInto(Json::Value obj) override;
    IStyleRef Clear() override;
};

__MC_DEFINE_REF_TYPE(BasicColoredStyle)

class LiteralContentType;
__MC_DEFINE_REF_TYPE(LiteralContentType)

class TextContentTypes {
public:
    using Registry = std::map<std::string, IContentTypeRef>;

private:
    static std::map<std::string, IContentTypeRef> _types;
    static LiteralContentTypeRef e_Literal;
    
public:
    template<class T>
    static __MC_REF_TYPE(T) Register(std::string key, __MC_REF_TYPE(T) type) {
        _types[key] = type;
        return type;
    }
    
    static LiteralContentTypeRef Literal();
};

class LiteralContent : public IContent {
public:
    std::string text;
    
    LiteralContent(std::string text);
    IContentTypeRef GetType() override;
    IContentRef Clone() override;
    void InsertPayload(Json::Value target) override;
    void Visit(std::shared_ptr<IContentVisitor> visitor,
               std::shared_ptr<IStyle> style) override;
    void VisitLiteral(std::shared_ptr<IContentVisitor> visitor,
                      std::shared_ptr<IStyle> style) override;
};

class LiteralContentType : public IContentType {
public:
    std::shared_ptr<IContent> CreateContent(Json::Value payload) override;
    void InsertPayload(Json::Value target, IContentRef content) override;
};

namespace Component {

std::shared_ptr<IComponent> FromJson(Json::Value obj,
                                     std::function<IStyle*(Json::Value)> parseStyle);
std::shared_ptr<IComponent> FromJson(Json::Value obj);
std::shared_ptr<IComponent> Literal(std::string text);

};

}

#endif /* components_hpp */
#endif
