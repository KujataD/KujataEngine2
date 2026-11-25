@echo off
rem Review uncommitted changes with Claude. Claude gets read-only tools.
rem Keep this file ASCII only: cmd.exe misreads UTF-8 Japanese in batch files.
chcp 65001 > nul
cd /d "%~dp0.."

set "HAS_CHANGES="
git diff --quiet HEAD || set "HAS_CHANGES=1"
for /f "delims=" %%f in ('git ls-files --others --exclude-standard') do set "HAS_CHANGES=1"
if not defined HAS_CHANGES (echo No changes to review. & exit /b)

rem git diff skips new files, so add the ones not yet staged.
(
	git diff HEAD
	for /f "delims=" %%f in ('git ls-files --others --exclude-standard') do git diff --no-index -- /dev/null "%%f"
) | claude -p "Review this diff and list likely bugs. Answer in Japanese. Do not edit any files." --model haiku --tools "Read,Grep,Glob" --max-turns 5
