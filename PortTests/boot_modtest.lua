-- ============================================================================
-- FIN PORT-TEST  EEPROM-Bootloader
-- ============================================================================
-- Aufs EEPROM eines DEDIZIERTEN Test-Computers (Computer Case + CPU T1 Lua +
-- RAM T1 + Internet Card + Strom). Holt modtest.lua vom Test-Server (testserver.py
-- auf 127.0.0.1:8080) und fuehrt es aus. Online-only — Test ist immer frisch.
-- Bei Aenderung an modtest.lua: nur Computer-Neustart noetig.
-- ============================================================================

local URL   = "http://127.0.0.1:8080/modtest.lua"
local TRIES = 10
local DELAY = 0.5

local cards = computer.getPCIDevices(classes.FINInternetCard)
if not cards or #cards == 0 then
    print("[modtest-boot] FEHLER: keine Internet Card im Computer")
    return
end

local body
for attempt = 1, TRIES do
    local req = cards[1]:request(URL, "GET", "")
    if req then
        local code, b = req:await()
        if code == 200 and b then
            body = b
            if attempt > 1 then print("[modtest-boot] online OK nach " .. attempt .. " Versuchen") end
            break
        else
            print("[modtest-boot] try " .. attempt .. "/" .. TRIES .. " -> HTTP " .. tostring(code))
        end
    else
        print("[modtest-boot] try " .. attempt .. "/" .. TRIES .. " -> request nil (Card init?)")
    end
    if attempt < TRIES then pcall(function() event.pull(DELAY) end) end
end

if not body then
    computer.beep(0.2)
    print("[modtest-boot] FEHLER: modtest.lua nicht ladbar — laeuft testserver.py?")
    return
end

print("[modtest-boot] modtest.lua geladen (" .. #body .. " bytes) — starte Tests...")
local fn, err = load(body, "modtest.lua")
if fn then
    local ok, runErr = pcall(fn)
    if not ok then
        computer.beep(0.3)
        print("[modtest-boot] modtest.lua CRASH: " .. tostring(runErr))
    end
else
    computer.beep(0.3)
    print("[modtest-boot] load error: " .. tostring(err))
end
print("[modtest-boot] done")
