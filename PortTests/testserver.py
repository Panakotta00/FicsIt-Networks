#!/usr/bin/env python3
"""Minimaler self-contained Test-Server fuer die FIN Port-Tests.

- Serviert die .lua-Dateien aus diesem Ordner (GET /modtest.lua etc.).
- Sammelt Test-Ergebnisse:  POST /log   Body {"source": "...", "lines": [{"line": "..."}]}
- Zeigt Ergebnisse:         GET /results        (Klartext, letzter Lauf hervorgehoben)
                            GET /results.json   (JSON)

Bindet 127.0.0.1:8080. Planner-server.py vorher stoppen (Port-Konflikt).
Start:  python testserver.py
"""
import json
import os
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

HOST, PORT = "127.0.0.1", 8080
ROOT = os.path.dirname(os.path.abspath(__file__))
_results = []  # Liste von (ts, source, line)


class Handler(BaseHTTPRequestHandler):
    def log_message(self, *args):  # ruhiger Output
        pass

    def _send(self, code, body, ctype="text/plain; charset=utf-8"):
        data = body.encode("utf-8") if isinstance(body, str) else body
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        path = self.path.split("?")[0]
        if path == "/results":
            lines = ["[%s] %s" % (s, ln) for (_, s, ln) in _results]
            return self._send(200, "\n".join(lines) or "(noch keine Ergebnisse)")
        if path == "/results.json":
            return self._send(
                200,
                json.dumps([{"t": t, "source": s, "line": ln} for (t, s, ln) in _results]),
                "application/json",
            )
        # statische .lua-Dateien aus ROOT
        name = path.lstrip("/")
        if name.endswith(".lua") and "/" not in name and ".." not in name:
            fp = os.path.join(ROOT, name)
            if os.path.isfile(fp):
                with open(fp, "rb") as f:
                    return self._send(200, f.read(), "text/plain; charset=utf-8")
        return self._send(404, "not found: " + path)

    def do_POST(self):
        path = self.path.split("?")[0]
        length = int(self.headers.get("Content-Length", 0))
        raw = self.rfile.read(length) if length else b""
        if path == "/log":
            try:
                d = json.loads(raw.decode("utf-8"))
                src = str(d.get("source") or "unknown")
                for entry in d.get("lines") or []:
                    ln = str(entry.get("line", ""))
                    _results.append((time.time(), src, ln))
                    print("[%s] %s" % (src, ln))  # Live in der Konsole
                return self._send(200, "ok")
            except Exception as exc:  # noqa: BLE001
                return self._send(400, "bad json: " + str(exc))
        return self._send(404, "not found: " + path)


if __name__ == "__main__":
    print("FIN Port-Test-Server -> http://%s:%d" % (HOST, PORT))
    print("  serviert .lua aus:", ROOT)
    print("  Ergebnisse:        http://%s:%d/results" % (HOST, PORT))
    print("  (Strg+C zum Beenden)\n")
    ThreadingHTTPServer((HOST, PORT), Handler).serve_forever()
