#pragma once

namespace throwback {

enum class PowerAction { ShutDown, Restart, Sleep, SignOut, Lock };

// Runs the action right away, like the start menu's power button does.
void RunPowerAction(PowerAction action);

// Shows Windows' own "Shut Down Windows" dialog, as the classic start
// menus' Shut Down... item did.
void ShowShutDownDialog();

}  // namespace throwback
