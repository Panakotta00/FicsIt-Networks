-- ============================================================================
-- FIN PORT-TEST  Runner + Suite   (Regressionstests fuer den 1.2/UE5.6-Port)
-- ============================================================================
-- Geladen via boot_modtest.lua auf einem dedizierten Test-Computer. Voellig
-- unabhaengig vom Planner. Meldet an POST /log (source=modtest), Server zeigt
-- sie unter /results.
--
-- KONVENTION: pro gefixtem Port-Bug ein Test, im Kommentar mit Katalog-# (siehe
-- README.md). Tests die Hardware brauchen (Maschine/Gleis am FIN-Netz) gehen via
-- skip() auf SKIP statt FAIL, mit Hinweis was anzuschliessen ist.
-- ============================================================================

local SERVER = "http://127.0.0.1:8080/"

local results = {}
local SkipMarker = {}

local function skip(reason) error({ [SkipMarker] = true, reason = reason or "skip" }) end

local function T(name, fn)
    local ok, res = pcall(fn)
    if ok then
        results[#results + 1] = { name = name, status = "PASS", detail = res ~= nil and tostring(res) or "" }
    elseif type(res) == "table" and res[SkipMarker] then
        results[#results + 1] = { name = name, status = "SKIP", detail = tostring(res.reason) }
    else
        results[#results + 1] = { name = name, status = "FAIL", detail = tostring(res) }
    end
end

local function assert_true(v, msg) if not v then error(msg or "erwartet wahr", 2) end end
local function assert_eq(got, want, msg)
    if got ~= want then error((msg or "assert_eq") .. " (got=" .. tostring(got) .. ", want=" .. tostring(want) .. ")", 2) end
end
local function assert_nonnil(v, msg) if v == nil then error(msg or "erwartet nicht-nil", 2) end end

-- Helfer: erste Komponente einer Klasse am Netzwerk (oder nil)
local function firstComp(cls)
    if not cls then return nil end
    local ids = component.findComponent(cls)
    if not ids or #ids == 0 then return nil end
    return component.proxy(ids[1])
end

-- ===========================================================================
-- SUITE
-- ===========================================================================

-- ---- Core / Lua-VM  (Kat#1 TString-Kollision: wenn Lua laeuft, ist sie gefixt) ----
T("core: lua-vm", function() assert_nonnil(_VERSION); return _VERSION end)
T("core: computer.magicTime", function() return computer.magicTime() end)
T("core: computer.millis", function() return computer.millis() end)
T("core: computer.getInstance", function() assert_nonnil(computer.getInstance()); return "ok" end)
T("core: event.pull(0)", function() event.pull(0); return "ok" end)

-- ---- Reflection (Herzstueck) ----
T("reflect: classes.FINComputerCase", function() assert_nonnil(classes.FINComputerCase); return "found" end)
T("reflect: classes.FINInternetCard", function() assert_nonnil(classes.FINInternetCard); return "found" end)
T("reflect: game-class Build_ConstructorMk1_C", function() assert_nonnil(classes.Build_ConstructorMk1_C); return "found" end)
T("reflect: structs vorhanden", function() assert_nonnil(structs); return "yes" end)
T("reflect: self:getType().name", function()
    local t = computer.getInstance():getType(); assert_nonnil(t.name); return t.name
end)

-- ---- Netzwerk / PCI ----
T("net: getPCIDevices(InternetCard)", function() return #computer.getPCIDevices(classes.FINInternetCard) end)
T("net: getPCIDevices(GPU)", function() return #computer.getPCIDevices(classes.FINComputerGPU) end)
T("net: component.findComponent('')", function() return #component.findComponent("") end)

-- ---- Kat#5: Recipe getIngredients (CDO-Methode statt static) ----
T("port: recipe getIngredients (Kat#5)", function()
    -- Reflection-Klasse einer bekannten Maschine holen, ihre Rezepte abfragen.
    local cls = classes.Recipe_IronIngot_C or classes.Recipe_IronPlate_C
    if not cls then skip("keine bekannte Recipe-Klasse gefunden (API evtl. anders)") end
    -- Hinweis: exakter Lua-Aufruf der reflektierten ClassFunc kann variieren;
    -- dieser Test wird verfeinert sobald wir die Recipe-Reflection live sehen.
    skip("Recipe-Reflection-Aufruf aus Lua noch zu verifizieren (Geruest)")
end)

-- ---- Kat#6: FactoryConnection .type == 0 (Conveyor); Enum in 1.2 entfernt ----
T("port: FactoryConnection .type (Kat#6)", function()
    local c = firstComp(classes.FGFactoryConnectionComponent)
    if not c then skip("keine FactoryConnection am Netzwerk (Maschine via Network Adapter anschliessen)") end
    local t = c.type
    assert_eq(t, 0, "FactoryConnection.type sollte 0 (Conveyor) sein")
    return "type=" .. tostring(t)
end)

-- ---- Kat#7: CentralStorage canUpload -> Limit>0 ----
T("port: CentralStorage canUpload (Kat#7)", function()
    local c = firstComp(classes.FGCentralStorageSubsystem)
    if not c then skip("CentralStorage nicht am Netzwerk") end
    skip("Test-Setup fuer canUpload(itemType) noch zu definieren (Geruest)")
end)

-- ---- Kat#8: Railroad Signal Block (friend-Zugriff) ----
T("port: railroad signal block (Kat#8)", function()
    local sig = firstComp(classes.FGBuildableRailroadSignal)
    if not sig then skip("kein Railroad-Signal am Netzwerk") end
    skip("Signal-Block-Reflection live verifizieren (Geruest)")
end)

-- ===========================================================================
-- REPORT  -> /log  (source=modtest)
-- ===========================================================================
local function esc(s)
    return tostring(s):gsub("\\", "\\\\"):gsub('"', '\\"'):gsub("\n", "\\n"):gsub("\r", "")
end

local nPass, nFail, nSkip = 0, 0, 0
local lines = {}
for _, r in ipairs(results) do
    if r.status == "PASS" then nPass = nPass + 1
    elseif r.status == "FAIL" then nFail = nFail + 1
    else nSkip = nSkip + 1 end
    local line = "[" .. r.status .. "] " .. r.name
    if r.detail ~= "" then line = line .. "  -> " .. r.detail end
    lines[#lines + 1] = '{"line":"' .. esc(line) .. '"}'
    print(line)
end
local summary = ("[SUMMARY] %d PASS / %d FAIL / %d SKIP  (FIN 1.2-Port)"):format(nPass, nFail, nSkip)
lines[#lines + 1] = '{"line":"' .. esc(summary) .. '"}'
print(summary)

local body = '{"source":"modtest","lines":[' .. table.concat(lines, ",") .. "]}"
local cards = computer.getPCIDevices(classes.FINInternetCard)
if cards and #cards > 0 then
    local ok, err = pcall(function()
        local code = cards[1]:request(SERVER .. "log", "POST", body):await()
        print("[modtest] POST /log -> http " .. tostring(code))
    end)
    if not ok then print("[modtest] POST fehlgeschlagen: " .. tostring(err)) end
else
    print("[modtest] keine Internet Card — Ergebnisse nur lokal")
end
