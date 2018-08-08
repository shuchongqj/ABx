name = "Smith"
level = 20
modelIndex = 5     -- Smith body model
sex = 2            -- Male
creatureState = 1  -- Idle
prof1Index = 1     -- Warrior
prof2Index = 0     -- None

function onInit()
  return true
end

function onUpdate(timeElapsed)

end

-- self was selected by creature
function onSelected(creature)
  print(creature:GetName() .. " selected me, the " .. self:GetName() .. " :D")
end

-- creature collides with self
function onCollide(creature)
  -- Testing Octree query
  local objects = self:QueryObjects(2.0)
  for i, v in ipairs(objects) do
--    print(i, v, v:GetName(), v:AsCreature():GetLevel()) 
  end
end
