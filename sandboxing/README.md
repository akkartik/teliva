This directory includes some working notes to audit the entire Teliva codebase
for side-effects that should be gated/sandboxed.

Founding principle for this approach: Side-effects come from the OS. There can
be no effects visible outside a Unix process (regardless of language) if it
doesn't invoke any OS syscalls.

## Top down

Things to secure:
* files opened (for read/write) on file system

* destinations opened (for read/write) on network
  * `inet_tryconnect` // `socket_connect`
  * `inet_tryaccept` // `socket_accept`

It seems more difficult to control what is written to a file or socket once
it's opened. For starters let's just focus on the interfaces that convert a
string path or url to a file descriptor.

Scenarios:
  * (1) app reads system files
  * (1) app sends data to a remote server
  * (1) app should _never_ be allowed to open Teliva's system files:
      - `teliva_editor_state`
      - app-specific sandboxing policies
  * (2) app can read from a remote server but not write (POST)
  * app gains access to a remote server for a legitimate purpose, reads
    sensitive data from the local system file for legitimate purpose. Now
    there's nothing preventing it from exfiltrating the sensitive data to the
    remote server.
    - (2) solution: make it obvious in the UI that granting both permissions
      allows an app to do anything. Educate people to separate apps that read
      sensitive data from apps that access remote servers.
    - (2) solution: map phases within an app to distinct permission sets
  * (3) app wants access to system() or exec()

Difficulty levels
  1. I have some sense of how to enforce this.
  2. Seems vaguely doable.
  3. Seems unlikely to be doable.

UX:
  * distinguish what Teliva can do, what the app can do, and Teliva's ability
    to police the app.
  * easily visualize Teliva's ability to police an app.
    - maybe show a lock in halves; left half = file system, right half =
      network. One half unlocked = orange. Both unlocked = red.

## Bottom up

* `includes`: all `#include`s throughout the codebase. I assume that C the
  language itself can't invoke any syscalls without at least triggering
  warnings from the compiler.
  ```
  cd src
  grep '#include' * */* > ../sandboxing/includes
  ```
* `system_includes`: all `#include <...>`s throughout the codebase. I assume
  side-effects require going outside the codebase. `#include`s could smuggle
  out of the codebase using relative paths (`../`) but I assume it's easy to
  protect against this using code review.
  ```
  grep '<' sandboxing/includes > sandboxing/system_includes
  ```
* `unique_system_includes`: deduped
  ```
  sed 's/.*<\|>.*//g' sandboxing/system_includes |sort |uniq > sandboxing/unique_system_includes
  ```
