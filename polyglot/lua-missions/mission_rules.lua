-- AstroDefender polyglot module: Lua mission_scripting
-- ASTRO_POLYGLOT_CONTRACT_VERSION: 1.0.0
-- Reads shared state and returns mission modifiers for the runtime host.
local M = {}
M.contractVersion = "1.0.0"
M.logicalWidth = 960
M.logicalHeight = 720

function M.viewport(screenWidth, screenHeight)
  local scale = math.min(screenWidth / M.logicalWidth, screenHeight / M.logicalHeight)
  local width = math.floor(M.logicalWidth * scale + 0.5)
  local height = math.floor(M.logicalHeight * scale + 0.5)
  return { x = math.floor((screenWidth - width) / 2), y = math.floor((screenHeight - height) / 2), width = width, height = height, scale = scale }
end

function M.waveModifier(state)
  local level = state and state.level or 1
  return { enemySpeedMultiplier = 1.0 + (level - 1) * 0.08, bonusScoreMultiplier = level >= 5 and 1.5 or 1.0 }
end

return M
