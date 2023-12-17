//
//  components.cpp
//  MiaCrate++
//
//  Created by 咔咔 on 2023/12/7.
//

#include <Mochi/components.hpp>

namespace Mochi {

    // MARK: -

    IContentVisitor::Ref IContentVisitor::Create(IContentVisitor::Signature action) {
        class Instance : public IContentVisitor {
        private:
            IContentVisitor::Signature _delegate;
        public:
            Instance(IContentVisitor::Signature action) : _delegate(action) {
                
            }
            
            void Accept(IContent::Ref content, IStyle::Ref style) override {
                _delegate(content, style);
            }
        };
        
        return ::__MC_NAMESPACE::CreateRef<Instance>(action);
    }

    // MARK: -

    TextColor::Ref TextColor::RegisterBuiltin(char code, std::string name, Color color) {
        TextColor::Ref result = ::__MC_NAMESPACE::CreateRef<TextColor>(code, name, color);
        _byChar.insert({code, result});
        _byName.insert({name, result});
        return result;
    }

    TextColor::TextColor(char code, std::string name, Color color) : _code(code), _name(name), _color(color) {
        std::stringstream str;
        str << ColorChar << code;
        str >> _toString;
        
        _ordinal = _count++;
    }

    const std::string TextColor::ColorChar = "§";
    TextColor::ByCharMap TextColor::_byChar = TextColor::ByCharMap();
    TextColor::ByNameMap TextColor::_byName = TextColor::ByNameMap();
    int TextColor::_count = 0;

    #define __MC_DEFINE_COLOR(id, code, name, color) \
    const TextColor::Ref TextColor :: id = TextColor::RegisterBuiltin( code , #name, Color( color ));
    __MC_DEFINE_COLORS
    #undef __MC_DEFINE_COLOR

    // MARK: -

    TextColor::Ref BasicColoredStyle::GetColor() {
        return _color;
    }

    IColoredStyle::Ref BasicColoredStyle::WithColor(TextColor::Ref color) {
        _color = color;
        return GetRef<IStyle>(this);
    }

    IStyle::Ref BasicColoredStyle::ApplyTo(IStyle::Ref other) {
        auto o = ::__MC_NAMESPACE::AssertSubType<IColoredStyle>(other);
        
        auto self = GetRef<IStyle>(this);
        if (self == _empty) return o;
        if (other == _empty) return self;
        
        auto color = _color;
        if (color == nullptr) {
            color = o->GetColor();
        }
        
        return CreateRef<BasicColoredStyle>()->WithColor(color);
    }

    void BasicColoredStyle::SerializeInto(Json::Value obj) {
        
    }

    IStyle::Ref BasicColoredStyle::Clear() {
        return GetRef<IStyle>(this); // shared_from_this();
    }

    BasicColoredStyle::Ref BasicColoredStyle::_empty = CreateRef<BasicColoredStyle>();
    BasicColoredStyle::Ref BasicColoredStyle::Empty() { return _empty; }

    // MARK: -

    // MARK: TextContentTypes::_types
    TextContentTypes::Registry TextContentTypes::_types = TextContentTypes::Registry();

    // MARK: -

    LiteralContent::LiteralContent(std::string text)
    : text(text) {}

    IContentType::Ref LiteralContent::GetType() {
        return TextContentTypes::Literal();
    }

    IContent::Ref LiteralContent::Clone() {
        return CreateRef<LiteralContent>(text);
    }

    void LiteralContent::InsertPayload(Json::Value target) {
        GetType()->InsertPayload(target, shared_from_this());
    }

    void LiteralContent::Visit(IContentVisitor::Ref visitor,
                            IStyle::Ref style) {
        visitor->Accept(GetRef<IContent>(this), style);
    }

    void LiteralContent::VisitLiteral(std::shared_ptr<IContentVisitor> visitor,
        std::shared_ptr<IStyle> style) {
        visitor->Accept(GetRef<IContent>(this), style);
    }

    // MARK: -

    std::shared_ptr<IContent> LiteralContentType::CreateContent(Json::Value payload) {
        auto text = payload["text"].asString();
        return CreateRef<LiteralContent>(text);
    }

    void LiteralContentType::InsertPayload(Json::Value target, IContent::Ref content) {
        ::__MC_NAMESPACE::ThrowNotImplemented();
    }

    std::shared_ptr<LiteralContentType> TextContentTypes::e_Literal = TextContentTypes::Register("text", std::make_shared<LiteralContentType>());
    std::shared_ptr<LiteralContentType> TextContentTypes::Literal() {
        return e_Literal;
    }

    // MARK: -

    class GenericMutableComponent : public IMutableComponent {
    private:
        IContent::Ref _content;
        IStyle::Ref _style;
        std::list<IComponent::Ref> _siblings;
        
    public:
        GenericMutableComponent(IContent::Ref content,
                                IStyle::Ref style):
        _content(content), _style(style), _siblings() {}
        
        IContent::Ref GetContent() override {
            return _content;
        }
        
        IStyle::Ref GetStyle() override {
            return _style;
        }
        
        void SetStyle(IStyle::Ref style) override {
            _style = style;
        }
        
        std::list<IComponent::Ref> GetSiblings() override {
            return _siblings;
        }
        
        IMutableComponent::Ref Clone() override {
            auto result = CreateRef<GenericMutableComponent>(_content, _style);
            for (auto sibling : _siblings) {
                auto clone = sibling->Clone();
                result->GetSiblings().push_back(clone);
            }
            
            return result;
        }
        
        void Visit(IContentVisitor::Ref visitor, IStyle::Ref style) override {
            style = _style->ApplyTo(style);
            _content->Visit(visitor, style);
            
            for (auto sibling : _siblings) {
                sibling->Visit(visitor, style);
            }
        }
        
        void VisitLiteral(IContentVisitor::Ref visitor, IStyle::Ref style) override {
            style = _style->ApplyTo(style);
            _content->VisitLiteral(visitor, style);
            
            for (auto sibling : _siblings) {
                sibling->VisitLiteral(visitor, style);
            }
        }
    };

    // MARK: -

    IComponent::Ref Component::FromJson(Json::Value obj,
                                        Component::JsonStyleParseFn parseStyle) {
        ::__MC_NAMESPACE::ThrowNotImplemented();
    }

    IComponent::Ref Component::FromJson(Json::Value obj) {
        ::__MC_NAMESPACE::ThrowNotImplemented();
        // return FromJson(obj, );
    }

    IComponent::Ref Component::Literal(std::string text) {
        return std::make_shared<GenericMutableComponent>(std::make_shared<LiteralContent>(text),
                                                        std::make_shared<BasicColoredStyle>());
    }

}
