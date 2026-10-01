@echo off

cd /d "%~dp0"

echo.
echo ========================================
echo        UPDATING QUIZ LIST
echo ========================================
echo.

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
"$files = Get-ChildItem '.\quizzes\*.txt' | Sort-Object Name; ^
$items = @(); ^
foreach ($file in $files) { ^
    $name = [System.IO.Path]::GetFileNameWithoutExtension($file.Name); ^
    $name = $name -replace '[_-]', ' '; ^
    $name = (Get-Culture).TextInfo.ToTitleCase($name.ToLower()); ^
    $items += [PSCustomObject]@{ file = $file.Name; name = $name } ^
}; ^
$items | ConvertTo-Json -Depth 2 | Set-Content '.\web\quiz-list.json' -Encoding UTF8"

echo.
echo Quiz list updated!
echo.

type web\quiz-list.json

echo.
echo ========================================
echo        DONE
echo ========================================
echo.

pause