@echo off
rem Review uncommitted changes with Claude. Claude gets read-only tools.
rem Keep this file ASCII only: cmd.exe misreads UTF-8 Japanese in batch files.
chcp 65001 > nul
cd /d "%~dp0.."

rem Keep non-ASCII file names readable in every git command below (needs git 2.31 or later).
set "GIT_CONFIG_COUNT=1"
set "GIT_CONFIG_KEY_0=core.quotepath"
set "GIT_CONFIG_VALUE_0=false"

rem Untracked files are sent only when they look like source and are small, so keys and huge files are not uploaded.
set "UNTRACKED=git ls-files --others --exclude-standard -- *.h *.hpp *.cpp *.hlsl *.hlsli *.props *.vcxproj *.ps1 *.bat *.md"
set "MAX_BYTES=200000"
set "DIFF_FILE=%TEMP%\kujata-review-%RANDOM%.diff"

set "HAS_CHANGES="
git diff --quiet HEAD || set "HAS_CHANGES=1"
for /f "delims=" %%f in ('%UNTRACKED%') do set "HAS_CHANGES=1"
if not defined HAS_CHANGES (echo No changes to review. & exit /b)

rem git diff skips new files, so add the ones not yet staged.
git diff HEAD > "%DIFF_FILE%"
for /f "delims=" %%f in ('%UNTRACKED%') do call :add_untracked "%%f"

claude -p "Review this diff and list likely bugs. Answer in Japanese. Do not edit any files." --model haiku --tools "Read,Grep,Glob" --max-turns 5 < "%DIFF_FILE%"
del "%DIFF_FILE%"
exit /b

:add_untracked
if %~z1 LSS %MAX_BYTES% (
	git diff --no-index -- /dev/null %1 >> "%DIFF_FILE%"
) else (
	echo Skipped large file: %~1 1>&2
)
exit /b
