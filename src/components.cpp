//
//  components.cpp
//  MiaCrate++
//
//  Created by 咔咔 on 2023/12/7.
//

#include "components.hpp"

namespace Mochi {

// MARK: -

IContentVisitorRef IContentVisitor::Create(IContentVisitor::Signature action) {
    class Instance : public IContentVisitor {
    private:
        IContentVisitor::Signature _delegate;
    public:
        Instance(IContentVisitor::Signature action) : _delegate(action) {
            
        }
        
        void Accept(IContentRef content, IStyleRef style) override {
            _delegate(content, style);
        }
    };
    
    return std::make_shared<Instance>(action);
}

// MARK: -

std::shared_ptr<TextColor> TextColor::RegisterBuiltin(char code, std::string name, Color color) {
    std::shared_ptr<TextColor> result = std::make_shared<TextColor>(code, name, color);
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
auto TextColor::_byChar = std::map<char,        std::shared_ptr<TextColor>>();
auto TextColor::_byName = std::map<std::string, std::shared_ptr<TextColor>>();
int TextColor::_count = 0;

#define __MC_DEFINE_COLOR(id, code, name, color) \
const auto TextColor :: id = TextColor::RegisterBuiltin( code , #name, Color( color ));
__MC_DEFINE_COLORS
#undef __MC_DEFINE_COLOR

// MARK: -

TextColorRef BasicColoredStyle::GetColor() {
    return _color;
}

IColoredStyleRef BasicColoredStyle::WithColor(TextColorRef color) {
    _color = color;
    return std::dynamic_pointer_cast<IColoredStyle>(shared_from_this());
}

IStyleRef BasicColoredStyle::ApplyTo(IStyleRef other) {
    auto o = Mochi::AssertSubType<IColoredStyle>(other);
    
    auto self = shared_from_this();
    if (self == _empty) return o;
    if (other == _empty) return self;
    
    auto color = _color;
    if (color == nullptr) {
        color = o->GetColor();
    }
    
    return std::make_shared<BasicColoredStyle>()->WithColor(color);
}

void BasicColoredStyle::SerializeInto(Json::Value obj) {
    
}

IStyleRef BasicColoredStyle::Clear() {
    return shared_from_this();
}

BasicColoredStyleRef BasicColoredStyle::_empty = std::make_shared<BasicColoredStyle>();
BasicColoredStyleRef BasicColoredStyle::Empty() { return _empty; }

// MARK: -

// MARK: TextContentTypes::_types
auto TextContentTypes::_types = std::map<std::string, std::shared_ptr<IContentType>>();

// MARK: -

LiteralContent::LiteralContent(std::string text)
: text(text) {}

std::shared_ptr<IContentType> LiteralContent::GetType() {
    return TextContentTypes::Literal();
}

IContentRef LiteralContent::Clone() {
    return std::make_shared<LiteralContent>(text);
}

void LiteralContent::InsertPayload(Json::Value target) {
    GetType()->InsertPayload(target, shared_from_this());
}

void LiteralContent::Visit(std::shared_ptr<IContentVisitor> visitor,
                           std::shared_ptr<IStyle> style) {
    visitor->Accept(shared_from_this(), style);
}

void LiteralContent::VisitLiteral(std::shared_ptr<IContentVisitor> visitor,
                                  std::shared_ptr<IStyle> style) {
    visitor->Accept(shared_from_this(), style);
}

// MARK: -

std::shared_ptr<IContent> LiteralContentType::CreateContent(Json::Value payload) {
    auto text = payload["text"].asString();
    return std::make_shared<LiteralContent>(text);
}

void LiteralContentType::InsertPayload(Json::Value target, IContentRef content) {
    Mochi::ThrowNotImplemented();
}

auto TextContentTypes::e_Literal = TextContentTypes::Register("text", std::make_shared<LiteralContentType>());

std::shared_ptr<LiteralContentType> TextContentTypes::Literal() {
    return e_Literal;
}

// MARK: -

class GenericMutableComponent : public IMutableComponent {
private:
    IContentRef _content;
    IStyleRef _style;
    std::list<IComponentRef> _siblings;
    
public:
    GenericMutableComponent(IContentRef content,
                            IStyleRef style):
    _content(content), _style(style), _siblings() {}
    
    std::shared_ptr<IContent> GetContent() override {
        return _content;
    }
    
    std::shared_ptr<IStyle> GetStyle() override {
        return _style;
    }
    
    void SetStyle(std::shared_ptr<IStyle> style) override {
        _style = style;
    }
    
    std::list<std::shared_ptr<IComponent>> GetSiblings() override {
        return _siblings;
    }
    
    std::shared_ptr<IMutableComponent> Clone() override {
        auto result = std::make_shared<GenericMutableComponent>(_content, _style);
        for (auto sibling : _siblings) {
            auto clone = sibling->Clone();
            result->GetSiblings().push_back(clone);
        }
        
        return result;
    }
    
    void Visit(std::shared_ptr<IContentVisitor> visitor, std::shared_ptr<IStyle> style) override {
        style = _style->ApplyTo(style);
        _content->Visit(visitor, style);
        
        for (auto sibling : _siblings) {
            sibling->Visit(visitor, style);
        }
    }
    
    void VisitLiteral(std::shared_ptr<IContentVisitor> visitor, std::shared_ptr<IStyle> style) override {
        style = _style->ApplyTo(style);
        _content->VisitLiteral(visitor, style);
        
        for (auto sibling : _siblings) {
            sibling->VisitLiteral(visitor, style);
        }
    }
};

// MARK: -

IComponentRef Component::FromJson(Json::Value obj,
                                  std::function<IStyle*(Json::Value)> parseStyle) {
    Mochi::ThrowNotImplemented();
}

IComponentRef Component::FromJson(Json::Value obj) {
    Mochi::ThrowNotImplemented();
    // return FromJson(obj, );
}

IComponentRef Component::Literal(std::string text) {
    return std::make_shared<GenericMutableComponent>(std::make_shared<LiteralContent>(text),
                                                     std::make_shared<BasicColoredStyle>());
}

}
