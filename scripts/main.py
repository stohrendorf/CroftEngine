# language_override:  "en_US.utf8"
from engine import TR1ItemId

from scripts.tr1.object_infos import object_infos
from scripts.tr1.audio import tracks
from scripts.tr1.level_sequence import level_sequence, title_menu, lara_home
from hid import GlfwKey, GlfwGamepadButton, Action

if False:
    cheats = {
        "godMode": True,
        "allAmmoCheat": True,
        "inventory": {
            TR1ItemId.Key1: 10,
            TR1ItemId.Key2: 10,
            TR1ItemId.Key3: 10,
            TR1ItemId.Key4: 10,
            TR1ItemId.Puzzle1: 10,
            TR1ItemId.Puzzle2: 10,
            TR1ItemId.Puzzle3: 10,
            TR1ItemId.Puzzle4: 10,
        }
    }
else:
    cheats = {}

input_mapping = {
    Action.MoveSlow: [GlfwKey.LeftShift, GlfwGamepadButton.RightBumper],
    Action.Action: [GlfwKey.LeftControl, GlfwGamepadButton.A],
    Action.Holster: [GlfwKey.R, GlfwGamepadButton.Y],
    Action.Jump: [GlfwKey.Space, GlfwGamepadButton.X],
    Action.Roll: [GlfwKey.X, GlfwGamepadButton.B],
    Action.FreeLook: [GlfwKey.Kp0, GlfwGamepadButton.LeftBumper],
    Action.Menu: [GlfwKey.Escape, GlfwGamepadButton.Start],
    Action.Debug: [GlfwKey.F11],
    Action.DrawPistols: [GlfwKey.Num1],
    Action.DrawShotgun: [GlfwKey.Num2],
    Action.DrawUzis: [GlfwKey.Num3],
    Action.DrawMagnums: [GlfwKey.Num4],
    Action.ConsumeSmallMedipack: [GlfwKey.Num5],
    Action.ConsumeLargeMedipack: [GlfwKey.Num6],
    Action.Save: [GlfwKey.F5],
    Action.Load: [GlfwKey.F6],
    Action.Left: [GlfwKey.A, GlfwGamepadButton.DPadLeft],
    Action.Right: [GlfwKey.D, GlfwGamepadButton.DPadRight],
    Action.Forward: [GlfwKey.W, GlfwGamepadButton.DPadUp],
    Action.Backward: [GlfwKey.S, GlfwGamepadButton.DPadDown],
    Action.StepLeft: [GlfwKey.Q],
    Action.StepRight: [GlfwKey.E],
    Action.Screenshot: [GlfwKey.F12],
    Action.CheatDive: [GlfwKey.F10],  # only available in debug builds
}


def getGlidosPack():
    return None
    # return "assets/trx/1SilverlokAllVers/silverlok/"
    # return "assets/trx/JC levels 1-12/Textures/JC/"
    # return "assets/trx/JC levels 13-15/Textures/JC/"


def getObjectInfo(id):
    return object_infos[TR1ItemId(id)]


def getTrackInfo(id):
    return tracks[id]


print("Yay! Main script loaded.")
