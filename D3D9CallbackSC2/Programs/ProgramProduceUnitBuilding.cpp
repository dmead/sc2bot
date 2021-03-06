#include "Main.h"

void ProgramProduceUnitBuilding::Init(UnitEntry *Entry)
{
    _CurState = ProgProduceUnitBuildingSelectWorkers;
    _Entry = Entry;
    _AttemptsMade = 0;
    Name = String("Building<") + Entry->Name + String(">");
}

ProgramResultType ProgramProduceUnitBuilding::ExecuteStep()
{
    FrameHUDManager &FrameHUD = g_Context->Managers.FrameHUD;
    const String ActionButtonName = GetRaceTypeString(_Entry->Race) + String("Build") + GetComplexityTypeString(_Entry->Complexity) + String("Structure");
    if(_CurState == ProgProduceUnitBuildingSelectWorkers)
    {
        LogThreadEvent("Selecting Workers", LogStep);
        g_Context->Managers.KeyboardMouse.SendKey(KEY_0 + g_Context->Managers.ControlGroup.FindControlGroupIndex(ControlGroupWorkerMineral));
        _CurState = ProgProduceUnitBuildingMoveToBuildingSite;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgProduceUnitBuildingMoveToBuildingSite)
    {
        ActionButtonStateType State;
        if(!FrameHUD.ActionButtonPresent(ActionButtonName, State))
        {
            LogThreadEvent("BuildStructure not present", LogError);
            return ProgramResultFail;
        }
        if(State != ActionButtonStateNormal)
        {
            LogThreadEvent("BuildStructure is in an invalid state", LogError);
            return ProgramResultFail;
        }
        const BaseInfo* TargetSite = g_Context->Managers.Knowledge.FindBuildSite();
        if(TargetSite == NULL)
        {
            LogThreadEvent("TargetSite not found", LogError);
            return ProgramResultFail;
        }
        Vec2i ScreenCoord = g_Context->Constants.MinimapCoordToScreenCoord(TargetSite->BuildLocation);
        if(_Entry->Name == "Hatchery" || _Entry->Name == "SporeCrawler" || _Entry->Name == "SpineCrawler")
        {
            ScreenCoord = g_Context->Constants.MinimapCoordToScreenCoord(TargetSite->BaseLocation);
        }
        LogThreadEvent(String("MoveToBuildingSite: ") + ScreenCoord.CommaSeparatedString(), LogStep);
        g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
        _CurState = ProgProduceUnitBuildingSelectBuildStructure;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgProduceUnitBuildingSelectBuildStructure)
    {
        ActionButtonStateType State;
        if(!FrameHUD.ActionButtonPresent(ActionButtonName, State))
        {
            LogThreadEvent("BuildStructure not present", LogError);
            return ProgramResultFail;
        }
        if(State != ActionButtonStateNormal)
        {
            LogThreadEvent("BuildStructure is in an invalid state", LogError);
            return ProgramResultFail;
        }
        if(_Entry->Complexity == ComplexityBasic)
        {
            g_Context->Managers.KeyboardMouse.SendKey(KEY_B);
        }
        else
        {
            g_Context->Managers.KeyboardMouse.SendKey(KEY_V);
        }
        LogThreadEvent("Selecting BuildStructure", LogStep);
        _CurState = ProgProduceUnitBuildingSelectBuilding;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgProduceUnitBuildingSelectBuilding)
    {
        ActionButtonStateType State;
        if(!FrameHUD.ActionButtonPresent(_Entry->Name, State))
        {
            LogThreadEvent(_Entry->Name + String(" not present"), LogError);
            return ProgramResultFail;
        }
        if(State != ActionButtonStateNormal)
        {
            LogThreadEvent(_Entry->Name + String(" is in an invalid state"), LogError);
            return ProgramResultFail;
        }
        g_Context->Managers.KeyboardMouse.SendKey(KEY_A + (_Entry->Hotkey - 'a'));
        LogThreadEvent(String("Selecting building: ") + String(_Entry->Hotkey), LogStep);
        _CurState = ProgProduceUnitBuildingFindBuildingPlacement;
        _TimeoutTime0 = GameTime() + 2.5;
    }
    if(_CurState == ProgProduceUnitBuildingFindBuildingPlacement)
    {
        ActionButtonStateType State;
        if(FrameHUD.ActionButtonPresent(ActionButtonName, State))
        {
            if(_AttemptsMade == 0)
            {
                LogThreadEvent("BuildStructure present but no attempts made", LogError);
                return ProgramResultFail;
            }
            _TimeoutTime1 = GameTime() + 1.5;
            _CurState = ProgProduceUnitBuildingWaitForSuccess;
            return ProgramResultStillExecuting;
        }
        if(!FrameHUD.ActionButtonPresent("Cancel", State))
        {
            LogThreadEvent("Cancel not present", LogError);
            return ProgramResultFail;
        }
        if(GameTime() > _TimeoutTime0)
        {
            LogThreadEvent("Timed out searching for building location", LogError);
            return ProgramResultFail;
        }
        Vec2i ScreenCoord = g_Context->Constants.RandomClickablePoint(0.15f);
        LogThreadEvent(String("RandomBuildingPoint: ") + ScreenCoord.CommaSeparatedString(), LogStep);
        g_Context->Managers.KeyboardMouse.Click(ScreenCoord, MouseButtonLeft, ModifierNone);
        _AttemptsMade++;
        return ProgramResultStillExecuting;
    }
    if(_CurState == ProgProduceUnitBuildingWaitForSuccess)
    {
        if(GameTime() > _TimeoutTime1)
        {
            LogThreadEvent("Timed out waiting for building success", LogError);
            return ProgramResultFail;
        }
        ActionButtonStateType State;
        if(FrameHUD.ActionButtonPresent(ActionButtonName, State))
        {
            if(State == ActionButtonStateSelected)
            {
                LogThreadEvent("BuildStructure selected, success", LogDone);
                g_Context->ReportAction(_Entry->Name, RGBColor::Green);
                g_Context->Managers.Knowledge.RecordUnitInProduction(_Entry, GameTime());
                return ProgramResultSuccess;
            }
            else
            {
                LogThreadEvent("BuildStructure is in an invalid state", LogStep);
                return ProgramResultStillExecuting;
            }
        }
    }
    HANDLE_CRITICAL_FAILURE;
}
