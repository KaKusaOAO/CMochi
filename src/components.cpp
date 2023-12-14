//
//  components.cpp
//  MiaCrate++
//
//  Created by 咔咔 on 2023/12/7.
//

#include <Mochi/components.hpp>

namespace Mochi {

// MARK: -

Ref<IContentVisitor> IContentVisitor::Create(IContentVisitor::Signature action) {
    class Instance : public IContentVisitor {
    private:
        IContentVisitor::Signature _delegate;
    public:
        Instance(IContentVisitor::Signature action) : _delegate(action) { }
        
        void Accept(const IContent& content, const IStyle& style) const override {
            _delegate(content, style);
        }
    };
    
    return CreateReference<Instance>(action);
}

// MARK: -

TextColor& TextColor::RegisterBuiltin(char code, std::string name, Color color) {
    TextColor result(code, name, color);
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

bool TextColor::operator==(const TextColor& other) const {
    return _color == other._color;
}

const std::string TextColor::ColorChar = "§";
TextColor::ByCharMap TextColor::_byChar = TextColor::ByCharMap();
TextColor::ByNameMap TextColor::_byName = TextColor::ByNameMap();
int TextColor::_count = 0;

#define __MC_DEFINE_COLOR(id, code, name, color) \
const TextColor& TextColor :: id = TextColor::RegisterBuiltin( code , #name, Color( color ));
__MC_DEFINE_COLORS
#undef __MC_DEFINE_COLOR

// MARK: -

bool IColoredStyle::operator==(const IColoredStyle& other) const {
    return GetColor() == other.GetColor();
}

// MARK: -

BasicColoredStyle::BasicColoredStyle() : _color() {}
BasicColoredStyle::BasicColoredStyle(TextColor& color) : _color({ color }) {}
BasicColoredStyle::BasicColoredStyle(std::optional<std::reference_wrapper<TextColor>> color) : _color(color) {}

std::optional<std::reference_wrapper<TextColor>> BasicColoredStyle::GetColor() const {
    return _color;
}

BasicColoredStyle& BasicColoredStyle::WithColor(std::optional<std::reference_wrapper<TextColor>> color) const {
    BasicColoredStyle other(color);
    return other.ApplyTo(*this);
}

BasicColoredStyle& BasicColoredStyle::ApplyTo(const IStyle& other) {
    auto& o = Mochi::AssertSubType<BasicColoredStyle>((IStyle&) other);
    
    auto& self = *this;
    if (self == _empty) return o;
    if (o == _empty) return self;
    
    auto color = _color;
    if (!color.has_value()) {
        color = o.GetColor();
    }
    
    return CreateReference<BasicColoredStyle>(color);
}

void BasicColoredStyle::SerializeInto(Json::Value obj) {
    
}

BasicColoredStyle& BasicColoredStyle::Clear() {
    return BasicColoredStyle::Empty();
}

BasicColoredStyle& BasicColoredStyle::_empty = CreateReference<BasicColoredStyle>();
BasicColoredStyle& BasicColoredStyle::Empty() { return _empty; }

// MARK: -

// MARK: TextContentTypes::_types
TextContentTypes::Registry TextContentTypes::_types = TextContentTypes::Registry();

// MARK: -

LiteralContent::LiteralContent(std::string text) : text(text) {}

IContentType& LiteralContent::GetType() {
    return TextContentTypes::Literal();
}

LiteralContent& LiteralContent::Clone() {
    LiteralContent copy = *this;
    return copy;
}

void LiteralContent::InsertPayload(Json::Value target) {
    GetType().InsertPayload(target, *this);
}

void LiteralContent::Visit(const IContentVisitor& visitor,
                           const IStyle& style) {
    visitor.Accept(*this, style);
}

void LiteralContent::VisitLiteral(const IContentVisitor& visitor,
                                  const IStyle& style) {
    visitor.Accept(*this, style);
}

// MARK: -

LiteralContent& LiteralContentType::CreateContent(Json::Value payload) {
    auto text = payload["text"].asString();
    return CreateReference<LiteralContent>(text);
}

void LiteralContentType::InsertPayload(Json::Value target, IContent& content) {
    Mochi::ThrowNotImplemented();
}

Ref<LiteralContentType> TextContentTypes::e_Literal = TextContentTypes::Register("text", CreateReference<LiteralContentType>());
Ref<LiteralContentType> TextContentTypes::Literal() {
    return e_Literal;
}

// MARK: -

class GenericMutableComponent : public IMutableComponent {
private:
    IContent& _content;
    IStyle& _style;
    std::list<std::reference_wrapper<IComponent>> _siblings;
    
public:
    GenericMutableComponent(IContent& content,
                            IStyle& style):
    _content(content), _style(style), _siblings() {}
    
    IContent& GetContent() override {
        return _content;
    }
    
    IStyle& GetStyle() override {
        return _style;
    }
    
    void SetStyle(IStyle& style) override {
        _style = style;
    }
    
    std::list<std::reference_wrapper<IComponent>> GetSiblings() override {
        return _siblings;
    }
    
    IMutableComponent& Clone() override {
        GenericMutableComponent result(_content, _style);
        for (auto& sibling : _siblings) {
            auto& clone = sibling.get().Clone();
            result.GetSiblings().push_back(clone);
        }
        
        return result;
    }
    
    void Visit(const IContentVisitor& visitor, const IStyle& style) override {
        auto& tmpStyle = _style.ApplyTo(style);
        _content.Visit(visitor, tmpStyle);
        
        for (auto& sibling : _siblings) {
            sibling.get().Visit(visitor, tmpStyle);
        }
    }
    
    void VisitLiteral(const IContentVisitor& visitor, const IStyle& style) override {
        auto& tmpStyle = _style.ApplyTo(style);
        _content.VisitLiteral(visitor, tmpStyle);
        
        for (auto& sibling : _siblings) {
            sibling.get().VisitLiteral(visitor, tmpStyle);
        }
    }
};

// MARK: -

Ref<IComponent> Component::FromJson(Json::Value obj,
                                    std::function<IStyle*(Json::Value)> parseStyle) {
    Mochi::ThrowNotImplemented();
}

Ref<IComponent> Component::FromJson(Json::Value obj) {
    Mochi::ThrowNotImplemented();
    // return FromJson(obj, );
}

Ref<IComponent> Component::Literal(std::string text) {
    return CreateReference<GenericMutableComponent>(CreateReference<LiteralContent>(text),
                                                    CreateReference<BasicColoredStyle>());
}

}
