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
#include <Mochi/foundation.hpp>
#include <sstream>

namespace __MC_NAMESPACE {

    class IContent;
    class IContentType {
    public:
        using Ref = Handle<IContentType>;

        virtual Handle<IContent> CreateContent(Json::Value payload) = 0;
        virtual void InsertPayload(Json::Value target, Handle<IContent> content) = 0;
    };

    class IStyle : public std::enable_shared_from_this<IStyle> {
    public:
        using Ref = Handle<IStyle>;
        
        virtual void SerializeInto(Json::Value obj) = 0;
        virtual Ref ApplyTo(Ref other) = 0;
        virtual Ref Clear() = 0;
    };

    class IContentVisitor;
    class IContent : public std::enable_shared_from_this<IContent> {
    public:
        using Ref = Handle<IContent>;

        virtual Handle<IContentType> GetType() = 0;
        virtual Ref Clone() = 0;
        virtual void InsertPayload(Json::Value target) = 0;
        virtual void Visit(Handle<IContentVisitor> visitor, IStyle::Ref style) = 0;
        virtual void VisitLiteral(Handle<IContentVisitor> visitor, IStyle::Ref style) = 0;
    };

    class IContentVisitor {
    public:
        using Ref       = Handle<IContentVisitor>;
        using Signature = std::function<void(IContent::Ref, IStyle::Ref)>;
        
        virtual void Accept(IContent::Ref content, IStyle::Ref style) = 0;
        static Ref Create(Signature action);
    };

    class IMutableComponent;

    class IComponent {
    public:
        using Ref = Handle<IComponent>;
        
        virtual IContent::Ref GetContent() = 0;
        virtual IStyle::Ref GetStyle() = 0;
        virtual std::list<Ref> GetSiblings() = 0;
        virtual Handle<IMutableComponent> Clone() = 0;
        virtual void Visit(IContentVisitor::Ref visitor, IStyle::Ref style) = 0;
        virtual void VisitLiteral(IContentVisitor::Ref visitor, IStyle::Ref style) = 0;
    };

    class IMutableComponent : public IComponent {
    public:
        using Ref = Handle<IMutableComponent>;
        virtual void SetStyle(IStyle::Ref style) = 0;
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
    public:
        using Ref       = Handle<TextColor>;
        const static std::string ColorChar;

        TextColor(char code, std::string name, Color color);
        
#define __MC_DEFINE_COLOR(id, code, name, color) \
    const static Ref id ;
    __MC_DEFINE_COLORS
#undef __MC_DEFINE_COLOR
        
    private:
        using ByCharMap = std::map<char,        Ref>;
        using ByNameMap = std::map<std::string, Ref>;

        char _code;
        std::string _name;
        std::string _toString;
        int _ordinal;
        Color _color;
        
        static ByCharMap _byChar;
        static ByNameMap _byName;
        static int _count;
        
        static Ref RegisterBuiltin(char code, std::string name, Color color);
    };


    class IColoredStyle : public IStyle {
    public:
        using Ref = Handle<IColoredStyle>;
        virtual TextColor::Ref GetColor() = 0;
        virtual Ref WithColor(TextColor::Ref color) = 0;
    };

    class BasicColoredStyle : public IColoredStyle {
    public:
        using Ref = Handle<BasicColoredStyle>;
        static Ref Empty();

        TextColor::Ref GetColor() override;
        IColoredStyle::Ref WithColor(TextColor::Ref color) override;
        IStyle::Ref ApplyTo(IStyle::Ref other) override;
        void SerializeInto(Json::Value obj) override;
        IStyle::Ref Clear() override;
        
    private:
        static Ref _empty;
        TextColor::Ref _color;
    };

    class LiteralContentType;

    class TextContentTypes {
    public:
        using Registry = std::map<std::string, IContentType::Ref>;

    private:
        static Registry _types;
        static Handle<LiteralContentType> e_Literal;
        
    public:
        template<class T>
        static Handle<T> Register(std::string key, Handle<T> type) {
            _types[key] = type;
            return type;
        }
        
        static Handle<LiteralContentType> Literal();
    };

    class LiteralContent : public IContent {
    public:
        using Ref = Handle<LiteralContent>;

        std::string text;
        
        LiteralContent(std::string text);
        IContentType::Ref GetType() override;
        IContent::Ref Clone() override;
        void InsertPayload(Json::Value target) override;
        void Visit(IContentVisitor::Ref visitor,
                IStyle::Ref style) override;
        void VisitLiteral(IContentVisitor::Ref visitor,
                        IStyle::Ref style) override;
    };

    class LiteralContentType : public IContentType {
    public:
        using Ref = Handle<LiteralContentType>;

        IContent::Ref CreateContent(Json::Value payload) override;
        void InsertPayload(Json::Value target, IContent::Ref content) override;
    };

    namespace Component {

        using JsonStyleParseFn = std::function<IStyle::Ref(Json::Value)>;

        IComponent::Ref FromJson(Json::Value obj,
                                JsonStyleParseFn parseStyle);
        IComponent::Ref FromJson(Json::Value obj);
        IComponent::Ref Literal(std::string text);

    };

}

#endif /* components_hpp */
#endif
