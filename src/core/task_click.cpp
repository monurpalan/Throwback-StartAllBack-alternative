#include "core/task_click.h"

namespace throwback {

TaskClickAction DecideTaskClick(const TaskClickState& state) {
  if (state.minimized) return TaskClickAction::Restore;
  if (!state.wasActive) return TaskClickAction::Activate;
  return state.canMinimize ? TaskClickAction::Minimize : TaskClickAction::None;
}

}  // namespace throwback
