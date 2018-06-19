#pragma once

#include "BaseLevel.h"
#include "GameObject.h"
#include "ChatWindow.h"
#include "GameMenu.h"
#include <stdint.h>
#include "PropStream.h"
#include "Actor.h"
#include "PingDot.h"
#include "TargetWindow.h"
#include "MailWindow.h"
#include "MapWindow.h"
#include "PartyWindow.h"

/// All World maps, Outposts, Combat, Exploreable...
/// These all have the Game UI.
class WorldLevel : public BaseLevel
{
    URHO3D_OBJECT(WorldLevel, BaseLevel);
public:
    WorldLevel(Context* context);
    void CreatePlayer(uint32_t id, const Vector3& position, const Vector3& scale, const Quaternion& direction);
    Actor* CreateActor(uint32_t id, const Vector3& position, const Vector3& scale, const Quaternion& direction);
    void SelectObject(uint32_t objectId);
    SharedPtr<GameObject> GetObjectById(uint32_t id)
    {
        if (objects_.Contains(id))
            return objects_[id];
        return SharedPtr<GameObject>();
    }
protected:
    SharedPtr<ChatWindow> chatWindow_;
    SharedPtr<PingDot> pingDot_;
    SharedPtr<GameMenu> gameMenu_;
    SharedPtr<TargetWindow> targetWindow_;
    SharedPtr<MailWindow> mailWindow_;
    SharedPtr<MapWindow> mapWindow_;
    SharedPtr<PartyWindow> partyWindow_;
    String mapUuid_;
    String mapName_;
    /// All objects in the scene
    HashMap<uint32_t, SharedPtr<GameObject>> objects_;
    /// Urho3D NodeIDs -> AB Object IDs given from the server
    HashMap<uint32_t, uint32_t> nodeIds_;
    WeakPtr<GameObject> hoveredObject_;
    void ShowMap();
    void HideMap();
    void ToggleMap();
    void CreateUI() override;
    void SubscribeToEvents() override;
    void Update(StringHash eventType, VariantMap& eventData) override;
    void SetupViewport() override;
    virtual void CreateScene();
private:
    IntVector2 mouseDownPos_;
    bool rmbDown_;
    SharedPtr<GameObject> GetObjectFromNode(Node* node)
    {
        unsigned id = node->GetID();
        uint32_t objectId = nodeIds_[id];
        if (objectId == 0 && node->GetParent())
        {
            id = node->GetParent()->GetID();
            objectId = nodeIds_[id];
        }
        if (objectId != 0)
            return objects_[objectId];
        return SharedPtr<GameObject>();
    }
    Text* CreateServerDropdownItem(const String& text, const String& value);
    SharedPtr<GameObject> GetObjectAt(const IntVector2& pos);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandleMouseDown(StringHash eventType, VariantMap& eventData);
    void HandleMouseUp(StringHash eventType, VariantMap& eventData);
    void HandleMouseWheel(StringHash eventType, VariantMap& eventData);
    void HandleMouseMove(StringHash eventType, VariantMap& eventData);
    void HandleObjectSpawn(StringHash eventType, VariantMap& eventData);
    void HandleObjectDespawn(StringHash eventType, VariantMap& eventData);
    void HandleObjectPosUpdate(StringHash eventType, VariantMap& eventData);
    void HandleObjectRotUpdate(StringHash eventType, VariantMap& eventData);
    void HandleObjectStateUpdate(StringHash eventType, VariantMap& eventData);
    void HandleObjectSelected(StringHash eventType, VariantMap& eventData);
    void HandleMenuLogout(StringHash eventType, VariantMap& eventData);
    void HandleMenuSelectChar(StringHash eventType, VariantMap& eventData);
    void HandleMenuMail(StringHash eventType, VariantMap& eventData);
    void HandleMenuPartyWindow(StringHash eventType, VariantMap& eventData);
    void HandleTargetWindowUnselectObject(StringHash eventType, VariantMap& eventData);

    void SpawnObject(int64_t updateTick, uint32_t id, bool existing, const Vector3& position, const Vector3& scale,
        const Quaternion& rot, PropReadStream& data);
};

