@echo off
setlocal enabledelayedexpansion
echo Generating directory tree...

:: Create or overwrite the output file
echo Directory Tree (excluding .git and .vscode) > directory_tree.txt
echo. >> directory_tree.txt

:: Start listing from the current directory
call :list_dirs "." 0 >> directory_tree.txt

echo Directory tree saved to directory_tree.txt
pause
exit /b

:: Recursive function to list directories and files, ignoring .git and .vscode
:list_dirs
setlocal enabledelayedexpansion
for /d %%D in (%1\*) do (
    set "dirname=%%~nxD"
    if /I not "!dirname!"==".git" if /I not "!dirname!"==".vscode" (
        for /l %%I in (1,1,%2) do <nul set /p "=  "  :: Indentation
        echo|set /p =+-- [DIR] %%D
        call :list_dirs "%%D" %2+1
    )
)

:: List files in the current directory (excluding directories)
for %%F in (%1\*) do (
    if not exist "%%F\" (
        for /l %%I in (1,1,%2) do <nul set /p "=  "  :: Indentation
        echo|set /p =+-- %%~nxF
    )
)
exit /b
