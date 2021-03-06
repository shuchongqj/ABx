//
// Copyright (c) 2008-2015 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/Text.h>

namespace Urho3D
{

class Font;
class BorderImage;

/// Multi-line text editor %UI element.
class  MultiLineEdit : public BorderImage
{
    URHO3D_OBJECT(MultiLineEdit, BorderImage)

public:
    /// Construct.
    MultiLineEdit(Context* context);
    /// Destruct.
    virtual ~MultiLineEdit() override;
    /// Register object factory.
    static void RegisterObject(Context* context);

    /// Apply attribute changes that can not be applied immediately.
    void ApplyAttributes() override;
    /// Perform UI element update.
    void Update(float timeStep) override;
    /// React to mouse click begin.
    void OnClickBegin
    (const IntVector2& position, const IntVector2& screenPosition, MouseButton button, MouseButtonFlags buttons, QualifierFlags qualifiers, Cursor* cursor) override;
    /// React to mouse doubleclick.
    void OnDoubleClick
    (const IntVector2& position, const IntVector2& screenPosition, MouseButton button, MouseButtonFlags buttons, QualifierFlags qualifiers, Cursor* cursor) override;
    /// React to mouse drag begin.
    void
        OnDragBegin(const IntVector2& position, const IntVector2& screenPosition, MouseButtonFlags buttons, QualifierFlags qualifiers, Cursor* cursor) override;
    /// React to mouse drag motion.
    void OnDragMove
    (const IntVector2& position, const IntVector2& screenPosition, const IntVector2& deltaPos, MouseButtonFlags buttons, QualifierFlags qualifiers,
        Cursor* cursor) override;
    void OnKey(Key key, MouseButtonFlags buttons, QualifierFlags qualifiers) override;
    /// React to text input event.
    void OnTextInput(const String& text) override;

    /// Set text.
    void SetText(const String& text);
    /// Set cursor position.
    void SetCursorPosition(unsigned position);
    /// Set cursor blink rate. 0 disables blinking.
    void SetCursorBlinkRate(float rate);
    /// Set maximum text length. 0 for unlimited.
    void SetMaxLength(unsigned length);
    /// Set echo character for password entry and such. 0 (default) shows the actual text.
    void SetEchoCharacter(unsigned c);
    /// Set whether can move cursor with arrows or mouse, default true.
    void SetCursorMovable(bool enable);
    /// Set whether selections are allowed, default true.
    void SetTextSelectable(bool enable);
    /// Set whether copy-paste operations are allowed, default true.
    void SetTextCopyable(bool enable);
    /// get number of lines in text
    int GetNumLines();
    /// Set maximum number of lines user can input
    void SetMaxNumLines(int maxNumber);
    /// Enable Multiline to on.
    void SetMultiLine(bool enable);
    /// Set wordwrap. In wordwrap mode the text element will respect its current width. Otherwise it resizes itself freely.
    void SetWordwrap(bool enable);
    /// Return wordwrap mode.
    bool GetWordwrap() const;
    void SetEnableLinebreak(bool enable) { enabledLinebreak_ = enable; }
    bool GetEnableLinebreak() const { return enabledLinebreak_; }
    /// Set font size.
    void SetFontSize(int size);
    /// Set font color.
    void SetFontColor(Color color);
    /// Return text.
    const String& GetText() const
    {
        return line_;
    }

    /// Return cursor position.
    unsigned GetCursorPosition() const
    {
        return cursorPosition_;
    }

    /// Return cursor blink rate.
    float GetCursorBlinkRate() const
    {
        return cursorBlinkRate_;
    }

    /// Return maximum text length.
    unsigned GetMaxLength() const
    {
        return maxLength_;
    }

    /// Return echo character.
    unsigned GetEchoCharacter() const
    {
        return echoCharacter_;
    }

    /// Return whether can move cursor with arrows or mouse.
    bool IsCursorMovable() const
    {
        return cursorMovable_;
    }

    /// Return whether selections are allowed.
    bool IsTextSelectable() const
    {
        return textSelectable_;
    }

    /// Return whether copy-paste operations are allowed.
    bool IsTextCopyable() const
    {
        return textCopyable_;
    }

    /// Return text element.
    Text* GetTextElement() const
    {
        return text_;
    }

    /// Return cursor element.
    BorderImage* GetCursor() const
    {
        return cursor_;
    }

    void CopySelection();
    bool CutSelection();
    bool Paste();
    bool DeleteSelection();
    void SelectAll();
protected:
    /// Filter implicit attributes in serialization process.
    bool FilterImplicitAttributes(XMLElement& dest) const override;
    /// Update displayed text.
    void UpdateText();
    /// Update cursor position and restart cursor blinking.
    void UpdateCursor();
    /// Return char index corresponding to position within element, or M_MAX_UNSIGNED if not found.
    unsigned GetCharIndex(const IntVector2& position);
    /// Is number of lines limited
    bool hasMaxLines;
    /// maximum number of lines that can be inputted (only applicable if hasMaxLines is true).
    int maxLines;
    /// Text element.
    SharedPtr<Text> text_;
    /// Cursor element.
    SharedPtr<BorderImage> cursor_;
    /// Text line.
    String line_;
    /// Last used text font.
    Font* lastFont_{ nullptr };
    /// Last used text size.
    int lastFontSize_{ 0 };
    /// Text edit cursor position.
    unsigned cursorPosition_{ 0 };
    /// Drag begin cursor position.
    unsigned dragBeginCursor_{ M_MAX_UNSIGNED };
    /// Cursor blink rate.
    float cursorBlinkRate_{ 1.0f };
    /// Cursor blink timer.
    float cursorBlinkTimer_{ 0.0f };
    /// Maximum text length.
    unsigned maxLength_{ 0 };
    /// Echo character.
    unsigned echoCharacter_{ 0 };
    /// Cursor movable flag.
    bool cursorMovable_{ true };
    /// Text selectable flag.
    bool textSelectable_{ true };
    /// Copy-paste enable flag.
    bool textCopyable_{ true };
    /// Ability to write several lines enabled flag.
    bool multiLine_{ true };
    /// Entering returns adds a new line
    bool enabledLinebreak_{ true };

private:
    void HandleMouseWheel(StringHash eventType, VariantMap& eventData);
    /// Handle being focused.
    void HandleFocused(StringHash eventType, VariantMap& eventData);
    /// Handle being defocused.
    void HandleDefocused(StringHash eventType, VariantMap& eventData);
    /// Handle the element layout having been updated.
    void HandleLayoutUpdated(StringHash eventType, VariantMap& eventData);
    bool HandleEnterKey();
};
}
