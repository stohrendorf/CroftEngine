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
    GlfwKey.LeftShift: Action.MoveSlow,
    GlfwGamepadButton.RightBumper: Action.MoveSlow,
    GlfwKey.LeftControl: Action.Action,
    GlfwGamepadButton.A: Action.Action,
    GlfwKey.R: Action.Holster,
    GlfwGamepadButton.Y: Action.Holster,
    GlfwKey.Space: Action.Jump,
    GlfwGamepadButton.X: Action.Jump,
    GlfwKey.X: Action.Roll,
    GlfwGamepadButton.B: Action.Roll,
    GlfwKey.Kp0: Action.FreeLook,
    GlfwGamepadButton.LeftBumper: Action.FreeLook,
    GlfwKey.Escape: Action.Menu,
    GlfwGamepadButton.Start: Action.Menu,
    GlfwKey.F11: Action.Debug,
    GlfwKey.Num1: Action.DrawPistols,
    GlfwKey.Num2: Action.DrawShotgun,
    GlfwKey.Num3: Action.DrawUzis,
    GlfwKey.Num4: Action.DrawMagnums,
    GlfwKey.Num5: Action.ConsumeSmallMedipack,
    GlfwKey.Num6: Action.ConsumeLargeMedipack,
    GlfwKey.F5: Action.Save,
    GlfwKey.F6: Action.Load,
    GlfwKey.A: Action.Left,
    GlfwGamepadButton.DPadLeft: Action.Left,
    GlfwKey.D: Action.Right,
    GlfwGamepadButton.DPadRight: Action.Right,
    GlfwKey.W: Action.Forward,
    GlfwGamepadButton.DPadUp: Action.Forward,
    GlfwKey.S: Action.Backward,
    GlfwGamepadButton.DPadDown: Action.Backward,
    GlfwKey.Q: Action.StepLeft,
    GlfwKey.E: Action.StepRight,
    GlfwKey.F12: Action.Screenshot,
    GlfwKey.F10: Action.CheatDive,  # only available in debug builds
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
