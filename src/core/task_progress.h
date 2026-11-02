#pragma once

namespace throwback {

// A window's progress as ITaskbarList3 reports it, drawn on its button.
enum class ProgressState { None, Indeterminate, Normal, Error, Paused };

// Maps TBPFLAG bits (as sent with SetProgressState) to a state.
ProgressState ProgressStateFromFlags(unsigned flags);

// The share of the button to fill: SetProgressValue arrives scaled to
// 0..65534 by the shell. Values out of range are clamped.
float ProgressFraction(long long value);

}  // namespace throwback
