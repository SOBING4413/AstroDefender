-- Lua + LÖVE: scale the game canvas proportionally and center it.
local M = {}
M.logicalWidth = 960
M.logicalHeight = 720

function M.viewport(screenWidth, screenHeight)
  local scale = math.min(screenWidth / M.logicalWidth, screenHeight / M.logicalHeight)
  local width = math.floor(M.logicalWidth * scale + 0.5)
  local height = math.floor(M.logicalHeight * scale + 0.5)
  return { x = math.floor((screenWidth - width) / 2), y = math.floor((screenHeight - height) / 2), width = width, height = height, scale = scale }
end

function M.present(canvas)
  local v = M.viewport(love.graphics.getWidth(), love.graphics.getHeight())
  love.graphics.clear(0, 4 / 255, 18 / 255, 1)
  love.graphics.draw(canvas, v.x, v.y, 0, v.scale, v.scale)
end

return M
