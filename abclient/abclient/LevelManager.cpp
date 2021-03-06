/**
 * Copyright 2017-2020 Stefan Ascher
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


#include "LevelManager.h"
#include "BaseLevel.h"
#include "WorldLevel.h"
#include "GameMenu.h"
#include "Actor.h"
#include "FwClient.h"

//#include <Urho3D/DebugNew.h>

LevelManager::LevelManager(Context* context) :
    Object(context),
    level_(nullptr),
    fadeWindow_(nullptr)
{
    // Listen to set level event
    SubscribeToEvent(Events::E_SETLEVEL, URHO3D_HANDLER(LevelManager, HandleSetLevelQueue));
}

LevelManager::~LevelManager()
{
    level_ = SharedPtr<Object>();
}

void LevelManager::HandleSetLevelQueue(StringHash, VariantMap& eventData)
{
    // Busy now
    if (levelQueue_.Size())
    {
        return;
    }
    // Push to queue
    levelQueue_.Push(eventData);
    using namespace Events::SetLevel;
    instanceUuid_ = String(eventData[P_INSTANCEUUID].GetString());

    // Subscribe HandleUpdate() function for processing update events
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(LevelManager, HandleUpdate));

    // Init fade status
    fadeStatus_ = 0;
}

void LevelManager::HandleUpdate(StringHash, VariantMap& eventData)
{
    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    // Move sprites, scale movement with time step
    fadeTime_ -= timeStep;

    switch (fadeStatus_)
    {
    case 0:
    {
        // Prepare to fade out
        if (!level_)
        {
            // No old level
            fadeStatus_++;
            break;
        }
        // Add a fade layer
        AddFadeLayer();
        fadeWindow_->SetOpacity(0.0f);
        fadeTime_ = MAX_FADE_TIME;
        fadeStatus_++;
        break;
    }
    case 1:
    {
        // Fade out
        if (!level_)
        {
            // No old level
            fadeStatus_++;
            break;
        }
        fadeWindow_->SetOpacity(1.0f - fadeTime_ / MAX_FADE_TIME);

        // Increase fade status
        if (fadeTime_ <= 0.0f)
        {
            fadeStatus_++;
        }
        break;
    }
    case 2:
    {
        // Release old level
        if (!level_)
        {
            // No old level
            fadeStatus_++;
            break;
        }
        // We can not create new level here, or it may cause errors, we have to create it at the next update point.
        level_ = SharedPtr<Object>();
        fadeStatus_++;
        break;
    }
    case 3:
    {
        // Create new level
        lastLevelName_ = levelName_;
        VariantMap& levelData = levelQueue_.Front();
        using namespace Events::SetLevel;
        levelName_ = levelData[P_NAME].GetString();
        mapUuid_ = levelData[P_MAPUUID].GetString();
        mapType_ = static_cast<AB::Entities::GameType>(levelData[P_TYPE].GetUInt());
        partySize_ = static_cast<uint8_t>(levelData[P_PARTYSIZE].GetUInt());
        level_ = context_->CreateObject(StringHash(levelName_));
        BaseLevel* baseLevel = GetCurrentLevel<BaseLevel>();
        if (baseLevel)
            baseLevel->debugGeometry_ = drawDebugGeometry_;

        // Add a fade layer
        if (!fadeWindow_)
            AddFadeLayer();
        fadeWindow_->SetOpacity(1.0f);
        fadeTime_ = MAX_FADE_TIME;
        fadeStatus_++;
        break;
    }
    case 4:
    {
        // Fade in
        fadeWindow_->SetOpacity(fadeTime_ / MAX_FADE_TIME);

        // Increase fade status
        if (fadeTime_ <= 0.0f)
        {
            fadeStatus_++;
        }
        break;
    }
    case 5:
    {
        // Finished
        // Remove fade layer
        fadeWindow_->Remove();
        fadeWindow_ = SharedPtr<Window>();

        // Unsubscribe update event
        UnsubscribeFromEvent(E_UPDATE);
        // Remove the task
        levelQueue_.PopFront();
        // Release all unused resources
        GetSubsystem<ResourceCache>()->ReleaseAllResources(false);
        break;
    }
    }
}

void LevelManager::AddFadeLayer()
{
    fadeWindow_ = new Window(context_);
    // Make the window a child of the root element, which fills the whole screen.
    GetSubsystem<UI>()->GetRoot()->AddChild(fadeWindow_);
    auto* graphics = GetSubsystem<Graphics>();
    fadeWindow_->SetSize(graphics->GetWidth(), graphics->GetHeight());
    fadeWindow_->SetLayout(LM_FREE);
    // Urho has three layouts: LM_FREE, LM_HORIZONTAL and LM_VERTICAL.
    // In LM_FREE the child elements of this window can be arranged freely.
    // In the other two they are arranged as a horizontal or vertical list.

    // Center this window in it's parent element.
    fadeWindow_->SetAlignment(HA_CENTER, VA_CENTER);
    // Black color
    fadeWindow_->SetColor(Color(0.0f, 0.0f, 0.0f, 1.0f));
    // Make it top most
    fadeWindow_->SetBringToBack(false);
    fadeWindow_->BringToFront();
}

const AB::Entities::Game* LevelManager::GetGame() const
{
    auto* client = GetSubsystem<FwClient>();
    return client->GetGame(mapUuid_);
}

void LevelManager::SetDrawDebugGeometry(bool draw)
{
    if (drawDebugGeometry_ != draw)
    {
        drawDebugGeometry_ = draw;
        BaseLevel* baseLevel = GetCurrentLevel<BaseLevel>();
        if (baseLevel)
            baseLevel->debugGeometry_ = drawDebugGeometry_;
    }
}

GameObject* LevelManager::GetObject(uint32_t objectId)
{
    WorldLevel* lvl = GetCurrentLevel<WorldLevel>();
    if (lvl)
        return lvl->GetObject<GameObject>(objectId);
    return nullptr;
}

Actor* LevelManager::GetActorByName(const String& name)
{
    WorldLevel* lvl = GetCurrentLevel<WorldLevel>();
    if (lvl)
        return lvl->GetActorByName(name);
    return nullptr;
}

Player* LevelManager::GetPlayer()
{
    WorldLevel* lvl = GetCurrentLevel<WorldLevel>();
    if (lvl)
        return lvl->GetPlayer();
    return nullptr;
}

Camera* LevelManager::GetCamera() const
{
    WorldLevel* lvl = GetCurrentLevel<WorldLevel>();
    if (lvl)
        return lvl->GetCamera();
    return nullptr;
}
