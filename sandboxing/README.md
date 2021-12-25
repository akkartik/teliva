This directory includes some working notes to audit the entire Teliva codebase
for side-effects that should be gated/sandboxed.

Founding principle for this approach: Side-effects come from the OS. There can
be no effects visible outside a Unix process (regardless of language) if it
doesn't invoke any OS syscalls.

## Top down

Things to secure:
* files opened (for read/write) on file system
* what gets written to files on file system
* destinations opened (for read/write) on network
  * `inet_tryconnect` // `socket_connect`
  * `inet_tryaccept` // `socket_accept`
* what gets written to network
  * `socket_send`, `socket_sendto`
  * `socket_recv`, `socket_recvfrom`

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
