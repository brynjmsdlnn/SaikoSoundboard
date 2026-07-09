---
name: git-squash
description: Squash and restructure a range of Git commits into logical, conventionally-formatted commits using a non-interactive detached HEAD approach.
---

# Git Squashing Skill

This skill explains how to squash a range of commits into clean, logical, and conventionally-formatted commits. It uses a non-interactive detached HEAD approach to avoid editor prompts and command-line quoting issues.

## Guidelines & Heuristics

### 1. Grouping Commits
Instead of squashing everything into a single massive commit or leaving noisy WIP commits, group commits into logical units of work.
- **"What was accomplished?" vs. "What did I work on today?"**: Each final commit should represent a complete semantic feature, refactor, or fix.
- **Revertibility Heuristic**: Ask yourself: *"If this commit introduced a bug, would I ever want to revert only this logical unit?"* If yes, keep it as its own commit. If not (e.g., a style tweak supporting a feature), squash it into the main commit.

### 2. Commit Message Structure
Follow the Conventional Commits specification:
- **Title**: `<type>(<scope>): <brief imperative summary>`
- **Body**: A short paragraph explaining the reason for the change, the problem it solves, and the behavior it enables.
- **Layer Breakdown**: Bulleted lists grouped by project-specific layer (e.g., **Audio engine**, **QML UI layer**, **Data models**, **Orchestration layer**).
- **Grammar**: In bullet points, use base-form verbs and avoid third-person singular verb endings (e.g., write `Add`, `Move`, `Update`, `Register` rather than `Adds`, `Moves`, `Updates`, `Registers`).

---

## Step-by-Step Squashing Protocol

To perform the squash without opening interactive rebase editors or dealing with complex escaping issues, follow this detached HEAD procedure:

### Step 1: Identify the Commits
Locate the range of commits to squash and find the **parent commit SHA** (the commit immediately preceding the range).
```bash
git log --oneline -15
```

### Step 2: Detach HEAD at the Parent Commit
Check out the parent commit so we can build the new commits from that starting point:
```bash
git checkout <parent-commit-sha>
```

### Step 3: Apply and Commit Each Group
For each logical group of commits (starting from the oldest group):

1. **Stage the changes**: Cherry-pick all commits in the group in chronological order without committing them:
   ```bash
   git cherry-pick --no-commit <commit-sha-1> <commit-sha-2> ... <commit-sha-n>
   ```
2. **Draft the commit message**: Save the structured commit message to a temporary file (e.g. `temp_msg.txt`) to avoid command-line newline and escaping issues.
3. **Commit the staged changes**:
   ```bash
   git commit -F temp_msg.txt
   ```
4. **Clean up**: Delete the temporary file.

### Step 4: Reset the Branch Pointer
Once all groups are committed:
1. Switch back to your active branch:
   ```bash
   git checkout <branch-name>
   ```
2. Hard reset the branch to point to the newly created HEAD:
   ```bash
   git reset --hard <final-squashed-commit-sha>
   ```
3. Verify the final clean history:
   ```bash
   git log --oneline -10
   ```
