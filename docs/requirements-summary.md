# Requirements Summary

The final application should be a lightweight console monitoring agent.

- Language: C++17 or C++20.
- Build system: CMake.
- Target platform: Windows with Win32 API, Linux with X11/environment APIs, or
  a cross-platform solution.
- Collect metrics every 5 seconds in a background worker.
- Capture foreground process name and window title.
- Detect whether physical mouse or keyboard activity happened during the last
  collection interval.
- Pass collected records through a thread-safe queue.
- Do not let network delays block the collector.
- Send JSON batches every 30 seconds or once 10 records are queued.
- POST to a local demo endpoint, for example `http://localhost:8080`.
- If sending fails, keep data in memory up to 100 records and retry later.
- Handle Ctrl+C / SIGINT / SIGTERM gracefully.
- On shutdown, flush unsent metrics to `backup.json`.
- Provide README documentation with build and run instructions.
