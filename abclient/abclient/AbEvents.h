#pragma once

#include "Config.h"

using namespace Urho3D;

/// User defined events
namespace AbEvents
{

URHO3D_EVENT(E_SCREENSHOTTAKEN, ScreenshotTaken)
{
    URHO3D_PARAM(P_FILENAME, Filename);        // String
}

URHO3D_EVENT(E_SERVERJOINED, ServerJoined)
{
    URHO3D_PARAM(P_SERVERID, ServerId);        // String
}

URHO3D_EVENT(E_SERVERLEFT, ServerLeft)
{
    URHO3D_PARAM(P_SERVERID, ServerId);        // String
}

URHO3D_EVENT(E_GOTSERVICES, GotServices)
{
}

URHO3D_EVENT(E_CHANGINGINSTANCE, ChangingInstance)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_SERVERUUID, ServerUuid);                // String
    URHO3D_PARAM(P_MAPUUID, MapUuid);                      // String
    URHO3D_PARAM(P_INSTANCEUUID, InstanceUuid);            // String
}

URHO3D_EVENT(E_SETLEVEL, SetLevel)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_NAME, Name);                  // String
    URHO3D_PARAM(P_MAPUUID, MapUuid);            // String
    URHO3D_PARAM(P_INSTANCEUUID, InstanceUuid);  // String
    URHO3D_PARAM(P_TYPE, Type);                  // uint8_t
    URHO3D_PARAM(P_PARTYSIZE, PartySize);        // uint8_t
}

URHO3D_EVENT(E_LEVELREADY, LevelReady)
{
    URHO3D_PARAM(P_NAME, Name);        // String
}

URHO3D_EVENT(E_OBJECTSPAWN, ObjectSpawn)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_EXISTING, Existing);      // bool
    URHO3D_PARAM(P_OBJECTID, ObjectId);
    URHO3D_PARAM(P_POSITION, Position);
    URHO3D_PARAM(P_ROTATION, Rotation);
    URHO3D_PARAM(P_UNDESTROYABLE, Undestroyable);  // bool
    URHO3D_PARAM(P_SCALE, Scale);
    URHO3D_PARAM(P_STATE, State);            // AB::GameProtocol::CreatureState
    URHO3D_PARAM(P_SPEEDFACTOR, SpeedFactor);  // float
    URHO3D_PARAM(P_GROUPID, GroupId);  // uint32_t
    URHO3D_PARAM(P_GROUPPOS, GroupPos);  // uint32_t
    URHO3D_PARAM(P_DATA, Data);
}

URHO3D_EVENT(E_OBJECTDESPAWN, ObjectDespawn)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_OBJECTID, ObjectId);
}

URHO3D_EVENT(E_OBJECTPOSUPDATE, ObjectPosUpdate)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_OBJECTID, ObjectId);
    URHO3D_PARAM(P_POSITION, Position);
}

URHO3D_EVENT(E_OBJECTROTUPDATE, ObjectRotUpdate)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_OBJECTID, ObjectId);
    URHO3D_PARAM(P_ROTATION, Rotation);
    URHO3D_PARAM(P_MANUAL, Manual);
}

URHO3D_EVENT(E_OBJECTSTATEUPDATE, ObjectStateUpdate)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_OBJECTID, ObjectId);
    URHO3D_PARAM(P_STATE, State);
}

URHO3D_EVENT(E_OBJECTSPEEDUPDATE, ObjectSpeedUpdate)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_OBJECTID, ObjectId);
    URHO3D_PARAM(P_SPEEDFACTOR, SpeedFactor);      // float
}

URHO3D_EVENT(E_OBJECTSELECTED, ObjectSelected)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_SOURCEID, SourceId);
    URHO3D_PARAM(P_TARGETID, TargetId);
}

URHO3D_EVENT(E_SKILLFAILURE, SkillFailure)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_OBJECTID, ObjectId);
    URHO3D_PARAM(P_SKILLINDEX, SkillIndex);
    URHO3D_PARAM(P_ERROR, Error);                      // uint8_t
    URHO3D_PARAM(P_ERRORMSG, ErrorMsg);                // String
}

URHO3D_EVENT(E_OBJECTUSESKILL, ObjectUseSkill)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_OBJECTID, ObjectId);
    URHO3D_PARAM(P_SKILLINDEX, SkillIndex);
    URHO3D_PARAM(P_ENERGY, Energy);
    URHO3D_PARAM(P_ADRENALINE, Adrenaline);
    URHO3D_PARAM(P_ACTIVATION, Activation);
    URHO3D_PARAM(P_OVERCAST, Overcast);
}

URHO3D_EVENT(E_OBJECTENDUSESKILL, ObjectEndUseSkill)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_OBJECTID, ObjectId);
    URHO3D_PARAM(P_SKILLINDEX, SkillIndex);
    URHO3D_PARAM(P_RECHARGE, Recharge);
}

URHO3D_EVENT(E_OBJECTEFFECTADDED, ObjectEffectAdded)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_OBJECTID, ObjectId);
    URHO3D_PARAM(P_EFFECTINDEX, EffectIndex);
    URHO3D_PARAM(P_TICKS, Ticks);
}

URHO3D_EVENT(E_OBJECTEFFECTREMOVED, ObjectEffectRemoved)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_OBJECTID, ObjectId);
    URHO3D_PARAM(P_EFFECTINDEX, EffectIndex);
}

URHO3D_EVENT(E_OBJECTRESOURCECHANGED, ObjectResourceChanged)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_OBJECTID, ObjectId);
    URHO3D_PARAM(P_RESTYPE, ResType);              // uint32_t
    URHO3D_PARAM(P_VALUE, Value);                  // int32_t
}

URHO3D_EVENT(E_SERVERMESSAGE, ServerMessage)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_MESSAGETYPE, MessageType);
    URHO3D_PARAM(P_SENDER, Sender);
    URHO3D_PARAM(P_DATA, Data);
}

URHO3D_EVENT(E_CHATMESSAGE, ChatMessage)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_MESSAGETYPE, MessageType);
    URHO3D_PARAM(P_SENDERID, SenderId);
    URHO3D_PARAM(P_SENDER, Sender);
    URHO3D_PARAM(P_DATA, Data);
}

URHO3D_EVENT(E_MAILINBOX, MailInbox)
{
}

URHO3D_EVENT(E_MAILREAD, MailRead)
{
}

URHO3D_EVENT(E_PARTYINVITED, PartyInvited)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_SOURCEID, SourceId);     // unit32_t
    URHO3D_PARAM(P_TARGETID, TargetId);     // unit32_t
    URHO3D_PARAM(P_PARTYID, PartyId);       // unit32_t
}

URHO3D_EVENT(E_PARTYREMOVED, PartyRemoved)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_SOURCEID, SourceId);     // unit32_t
    URHO3D_PARAM(P_TARGETID, TargetId);     // unit32_t
    URHO3D_PARAM(P_PARTYID, PartyId);       // unit32_t
}

URHO3D_EVENT(E_PARTYADDED, PartyAdded)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_PLAYERID, PlayerId);     // unit32_t
    URHO3D_PARAM(P_LEADERID, LeaderId);     // unit32_t
    URHO3D_PARAM(P_PARTYID, PartyId);       // unit32_t
}

URHO3D_EVENT(E_PARTYINVITEREMOVED, PartyInviteRemoved)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_SOURCEID, SourceId);     // unit32_t
    URHO3D_PARAM(P_TARGETID, TargetId);     // unit32_t
    URHO3D_PARAM(P_PARTYID, PartyId);       // unit32_t
}

URHO3D_EVENT(E_PARTYINFOMEMBERS, PartyInfoMembers)
{
    URHO3D_PARAM(P_PARTYID, PartyId);       // unit32_t
    URHO3D_PARAM(P_MEMBERS, Members);       // VariantVector
}

URHO3D_EVENT(E_NEWMAIL, NewMail)
{
    URHO3D_PARAM(P_COUNT, Count);           // int
}

URHO3D_EVENT(E_ACTORNAMECLICKED, ActorNameClicked)
{
    URHO3D_PARAM(P_SOURCEID, SourceId);     // unit32_t
}

// Shortcuts
URHO3D_EVENT(E_SC_TOGGLEMAP, ToggleMap)
{
}

URHO3D_EVENT(E_SC_MOVEFORWARD, MoveForward)
{
}

URHO3D_EVENT(E_SC_MOVEBACKWARD, MoveBackward)
{
}

URHO3D_EVENT(E_SC_MOVELEFT, MoveLeft)
{
}

URHO3D_EVENT(E_SC_MOVERIGHT, MoveRight)
{
}

URHO3D_EVENT(E_SC_TURNLEFT, TurnLeft)
{
}

URHO3D_EVENT(E_SC_TURNRIGHT, TurnRight)
{
}

URHO3D_EVENT(E_SC_KEEPRUNNING, KeepRunning)
{
}

URHO3D_EVENT(E_SC_REVERSECAMERA, ReverseCamera)
{
}

URHO3D_EVENT(E_SC_HIGHLIGHTOBJECTS, HighlightObjects)
{
}

URHO3D_EVENT(E_SC_MOUSELOOK, MouseLook)
{
}

URHO3D_EVENT(E_SC_TOGGLEPARTYWINDOW, TogglePartyWindow)
{
}

URHO3D_EVENT(E_SC_DEFAULTACTION, DefaultAction)
{
}

URHO3D_EVENT(E_SC_TAKESCREENSHOT, TakeScreenshot)
{
}

URHO3D_EVENT(E_SC_TOGGLEMAILWINDOW, ToggleMailWindow)
{
}

URHO3D_EVENT(E_SC_TOGGLENEWMAILWINDOW, ToggleNewMailWindow)
{
}

URHO3D_EVENT(E_SC_TOGGLEMISSIONMAPWINDOW, ToggleMissionMapWindow)
{
}

URHO3D_EVENT(E_SC_TOGGLEFRIENDLISTWINDOW, ToggleFriendListWindow)
{
}

URHO3D_EVENT(E_SC_REPLYMAIL, ReplyMail)
{
    URHO3D_PARAM(P_RECIPIENT, Recipient);     // String
    URHO3D_PARAM(P_SUBJECT, Subject);         // String
}

URHO3D_EVENT(E_SC_LOGOUT, Logout)
{
}

URHO3D_EVENT(E_SC_SELECTCHARACTER, SelectCharacter)
{
}

URHO3D_EVENT(E_SC_SHOWCREDITS, ShowCredits)
{
}

URHO3D_EVENT(E_SC_TOGGLEOPTIONS, ToggleOptions)
{
}

URHO3D_EVENT(E_SC_TOGGLEMUTEAUDIO, ToggleMuteAudio)
{
}

URHO3D_EVENT(E_SC_EXITPROGRAM, ExitProgram)
{
}

URHO3D_EVENT(E_SC_CHATGENERAL, ChatGeneral)
{
}

URHO3D_EVENT(E_SC_CHATGUILD, ChatGuild)
{
}

URHO3D_EVENT(E_SC_CHATPARTY, ChatParty)
{
}

URHO3D_EVENT(E_SC_CHATTRADE, ChatTrade)
{
}

URHO3D_EVENT(E_SC_CHATWHISPER, ChatWhisper)
{
}

URHO3D_EVENT(E_SC_TOGGLECHATWINDOW, ToggleChatWindow)
{
}

URHO3D_EVENT(E_SC_HIDEUI, HideUI)
{
}

URHO3D_EVENT(E_SC_SELECTSELF, SelectSelf)
{
}

URHO3D_EVENT(E_SC_SELECTNEXTFOE, SelectNextFoe)
{
}

URHO3D_EVENT(E_SC_SELECTPREVFOE, SelectPrevFoe)
{
}

URHO3D_EVENT(E_SC_SELECTNEXTALLY, SelectNextAlly)
{
}

URHO3D_EVENT(E_SC_SELECTPREVALLY, SelectPrevAlly)
{
}

URHO3D_EVENT(E_SC_USESKILL1, UseSkill1)
{
}

URHO3D_EVENT(E_SC_USESKILL2, UseSkill2)
{
}

URHO3D_EVENT(E_SC_USESKILL3, UseSkill3)
{
}

URHO3D_EVENT(E_SC_USESKILL4, UseSkill4)
{
}

URHO3D_EVENT(E_SC_USESKILL5, UseSkill5)
{
}

URHO3D_EVENT(E_SC_USESKILL6, UseSkill6)
{
}

URHO3D_EVENT(E_SC_USESKILL7, UseSkill7)
{
}

URHO3D_EVENT(E_SC_USESKILL8, UseSkill8)
{
}

URHO3D_EVENT(E_SC_WEAPONSET1, WeaponSet1)
{
}

URHO3D_EVENT(E_SC_WEAPONSET2, WeaponSet2)
{
}

URHO3D_EVENT(E_SC_WEAPONSET3, WeaponSet3)
{
}

URHO3D_EVENT(E_SC_WEAPONSET4, WeaponSet4)
{
}

URHO3D_EVENT(E_SC_CANCEL, Cancel)
{
}

// Audio
URHO3D_EVENT(E_AUDIOPLAY, AudioPlay)
{
    URHO3D_PARAM(P_NAME, Name);     // String
    URHO3D_PARAM(P_TYPE, Type);     // String
}

URHO3D_EVENT(E_AUDIOSTOP, AudioStop)
{
    URHO3D_PARAM(P_NAME, Name);     // String
    URHO3D_PARAM(P_TYPE, Type);     // String
}

URHO3D_EVENT(E_AUDIOSTOPALL, AudioStopAll)
{
}

URHO3D_EVENT(E_AUDIOPLAYMAPMUSIC, AudioPlayMapMusic)
{
    URHO3D_PARAM(P_MAPUUID, MapUuid);     // String
}

/// Play style from current play list
URHO3D_EVENT(E_AUDIOPLAYMUSICSTYLE, AudioPlayMusicStyle)
{
    URHO3D_PARAM(P_STYLE, Style);     // uintt32_t
}

#ifdef DEBUG_HUD
URHO3D_EVENT(E_SC_TOGGLEDEBUGHUD, ToggleDebugHUD)
{
}

URHO3D_EVENT(E_SC_TOGGLECONSOLE, ToggleConsole)
{
}
#endif

}
