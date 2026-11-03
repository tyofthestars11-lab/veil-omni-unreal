VEIL JARVIS SETUP: an AI that drives Unreal on your PC (no copy-paste)
Source: Epic's Unreal MCP docs (UE 5.8, Experimental) + Epic's Claude Code plugin.

0. BACK UP FIRST
   Copy D:\VEIL-OMNI to D:\VEIL-OMNI-backup. The AI can change or delete assets.

1. TURN IT ON IN UNREAL (needs UE 5.8)
   Edit > Plugins > search "Unreal MCP" > Enabled
   Edit > Plugins > search "All Toolsets" > Enabled
   Restart the editor.
   Edit > Editor Preferences > General > Model Context Protocol > Auto Start Server = ON
   Open the console (backtick key ` ) and type:
   ModelContextProtocol.GenerateClientConfig All
   That writes .mcp.json into D:\VEIL-OMNI.

2. PICK THE BRAIN (one)
   A) Claude Code (paid Claude plan). Epic made a plugin for it.
      PowerShell:  irm https://claude.ai/install.ps1 | iex
   B) Gemini CLI (has a free tier). Install Node.js LTS first, then:
      PowerShell:  npm install -g @google/gemini-cli

3. CONNECT (Unreal editor open with VEIL.uproject)
   PowerShell:
   cd D:\VEIL-OMNI
   claude        (or: gemini)
   Claude Code only, once:
   /plugin install unreal-engine-skills-for-claude-code@claude-plugins-official
   Then type /mcp and check that unreal-mcp shows.

4. FIRST WORDS TO IT
   "List all actors in the current level."
   If it answers with real actor names, JARVIS is live.

5. THEN THE GAME
   "Open /Game/Maps/VEILDistrict. Build the Hub, The Golden Midst, at 0,0:
    stone plaza, gold phi rune pillars on a golden-angle ring
    (angle = i x 137.5077640500378 deg, radius = spacing x sqrt(i)), no random,
    reflecting pool, blue crystal clusters at the edge. Save the level and take a screenshot."

SAFETY
- Don't run it with --dangerously-skip-permissions. Approve each step.
- Keep wallet keys and secrets out of D:\VEIL-OMNI.
- Save and back up before every big session.

- Hark
