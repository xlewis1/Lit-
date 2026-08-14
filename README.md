# 🔥 Lit (lit)
The custom version control system built from the metal up.

lit is a lightweight, blazing-fast version control system built completely from scratch in C. Instead of relying on existing frameworks, lit implements its own object-hashing database (.lit/objects), branch management, state engines, and custom system architecture.

# ⚡ Why Lit?
Zero Bloat: Pure C systems programming designed for raw speed and direct memory control.
Granular Control: Custom file-tracking algorithms, snapshot compression, network streaming, and a sleek command-line interface.

# 🛠️ The Architecture
Unlike Git, which relies on complex packfiles and heavy abstraction layers, lit keeps things transparent and mechanical:
Local Object Database (.lit/objects/): Every snapshot is hashed, compressed, and stored safely locally.
Custom Engine (engine.c): Manages project states, networking, and metadata tracking directly from the command line.

# 🚀 Getting Started
1. Initialize a Repository
Start tracking your code natively with lit:

Bash
./lit start
2. Save a Snapshot
Commit your current progress into the local object database:

Bash
./lit save "Implemented core tracking engine"

# Command Reference
| ./lit start | Initialize an empty .lit repository workspace |
| ./lit status | Show working tree status and untracked files |
| ./lit save <msg> | Save a snapshot of your files with a custom message |
| ./lit history | Display the commit history tree with timestamps |
| ./lit ignite | Pull and fast-forward updates from a remote/peer |
| ./lit push <ip> | Push your local commits and objects over network socket |
| ./lit branch | Manage or switch active project branches |
| ./lit dilute | Clean or dilute working directory cache |
| ./lit nuke | Nuke or delete repository objects/workspace |
| ./lit dalek | EXTERMINATE sensitive files (.env, keys, tokens) from workspace |
| ./lit help | Print the help screen and logo |


# 💡 The Future
lit is more than a utility—it's an experiment in rewriting the foundational tools of software engineering from scratch.
