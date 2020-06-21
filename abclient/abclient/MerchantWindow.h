/**
 * Copyright 2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "DialogWindow.h"
#include "AB/Entities/ConcreteItem.h"
#include "FwClient.h"
#include "TabGroup.h"
#include "ItemUIElement.h"
#include "Spinner.h"

class MerchantWindow final : public DialogWindow
{
    URHO3D_OBJECT(MerchantWindow, DialogWindow)
private:
    SharedPtr<TabGroup> tabgroup_;
    SharedPtr<ListView> sellItems_;
    SharedPtr<ListView> buyItems_;
    SharedPtr<LineEdit> countText_;
    SharedPtr<Spinner> countSpinner_;
    SharedPtr<Button> doItButton_;
    uint32_t npcId_{ 0 };
    void CreateUI();
    void SubscribeEvents();
    TabElement* CreateTab(TabGroup* tabs, const String& page);
    void CreatePageSell(TabElement* tabElement);
    void CreatePageBuy(TabElement* tabElement);
    void LoadWindow(Window* wnd, const String& fileName);
    void UpdateSellList();
    UIElement* GetSellItem(uint16_t pos);
    unsigned GetSellItemIndex(uint16_t pos);
    UISelectable* CreateItem(ListView& container, const ConcreteItem& iItem);
    void HandleMerchantItems(StringHash eventType, VariantMap& eventData);
    void HandleDoItClicked(StringHash eventType, VariantMap& eventData);
    void HandleGoodByeClicked(StringHash eventType, VariantMap& eventData);
    void HandleInventory(StringHash eventType, VariantMap& eventData);
    void HandleInventoryItemUpdate(StringHash eventType, VariantMap& eventData);
    void HandleInventoryItemRemove(StringHash eventType, VariantMap& eventData);
    void HandleTabSelected(StringHash eventType, VariantMap& eventData);
    void HandleSellItemSelected(StringHash eventType, VariantMap& eventData);
public:
    MerchantWindow(Context* context);
    ~MerchantWindow() override;
    void Initialize(uint32_t npcId) override;
    void Clear();
};

